/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ********************************************************/

#include "mmex.h"
#include "constants.h"
#include "mmframe.h"
#include "mmSimpleDialogs.h"
#include "paths.h"
#include "platfdep.h"
#include "util.h"

#include "model/Model_Setting.h"
#include "model/Model_Usage.h"

#include <wx/cmdline.h>
#include <wx/fs_arc.h>
#include <wx/fs_filter.h>
#include <wx/fs_mem.h>
#include <wx/mstream.h>
#include "../resources/money.xpm"
 //----------------------------------------------------------------------------
wxIMPLEMENT_APP(mmGUIApp);
//----------------------------------------------------------------------------

static const wxCmdLineEntryDesc g_cmdLineDesc[] =
{
    { wxCMD_LINE_SWITCH, "h", "help", "", wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
    { wxCMD_LINE_SWITCH, "s", "silent", "Do not show warning messages at startup.", wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL },
    { wxCMD_LINE_OPTION, "i", "mmexini",   "where <str> is a path to mmexini.db3"
        "\n\nTo open a determined database(.mmb) file from a shortcut or command line, set the path to the database file as a parameter."
        "\n\nThe file with the application settings mmexini.db3 can be used separately."
        " Otherwise, it is taken from the home directory or the root folder of the application."},
    { wxCMD_LINE_PARAM, nullptr, nullptr, wxT_2("database file"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
    { wxCMD_LINE_NONE }
};

//----------------------------------------------------------------------------

mmGUIApp::mmGUIApp() : m_frame(nullptr)
, m_setting_db(nullptr)
, m_optParam1(wxEmptyString)
, m_optParam2(wxEmptyString)
, m_optParamSilent(false)
, m_lang(wxLANGUAGE_UNKNOWN)
, m_locale(wxLANGUAGE_DEFAULT)
{
#if wxUSE_ON_FATAL_EXCEPTION
    // catch fatal exceptions
    wxHandleFatalExceptions(true);
#endif
}

wxLanguage mmGUIApp::getGUILanguage() const
{
    return this->m_lang;
}

wxLocale& mmGUIApp::getLocale()
{
    return this->m_locale;
}

bool mmGUIApp::setGUILanguage(wxLanguage lang)
{
    if (lang == this->m_lang && lang != wxLANGUAGE_UNKNOWN) {
        return false;
    }
    wxTranslations *trans = new wxTranslations;
    trans->SetLanguage(lang);
    trans->AddStdCatalog();
    if (trans->AddCatalog("mmex", wxLANGUAGE_ENGLISH_US) || lang == wxLANGUAGE_ENGLISH_US || lang == wxLANGUAGE_DEFAULT)
    {
        wxTranslations::Set(trans);
        this->m_lang = lang;
        Option::instance().setLanguage(lang);
        return true;
    }
    else
    {
        wxArrayString lang_files = trans->GetAvailableTranslations("mmex");
        lang_files.Add("en_US");
        wxArrayString lang_names;
        for (const auto & file : lang_files)
        {
            const wxLanguageInfo* info = wxLocale::FindLanguageInfo(file);
            if (info) {
                lang_names.Add(info->Description);
            }
        }
        lang_names.Sort();

        wxString languages_list;
        for (const auto & name : lang_names)
        {
            languages_list += name + ", ";
        }
        languages_list.RemoveLast(2);

        wxString msg;
        if (lang != wxLANGUAGE_UNKNOWN)
        {
            wxString best;
#if wxCHECK_VERSION(3, 1, 2) && !wxCHECK_VERSION(3, 1, 3)
            // workaround for https://github.com/wxWidgets/wxWidgets/pull/1082
            wxArrayString all = trans->GetAcceptableTranslations("mmex");
            best = all.IsEmpty() ? "" : all[0];
#else
            best = trans->GetBestTranslation("mmex");
#endif
            if (best.IsEmpty()) {
                best = wxLocale::GetLanguageName(wxLocale::GetSystemLanguage());
                msg = wxString::Format(_("Cannot load a translation for the default language of your system (%s)."),
                    best);
            }
        }

        msg += "\n\n";
        if (lang == wxLANGUAGE_UNKNOWN) {
            msg += wxString::Format(_("Please use the Switch Application Language option in View menu to select one of the following available languages:\n\n%s"), languages_list);
            m_lang = wxLANGUAGE_DEFAULT;
            Option::instance().setLanguage(m_lang);
        }

        mmErrorDialogs::MessageWarning(NULL, msg, _("Language change"));
        wxDELETE(trans);
        return false;
    }
}


void mmGUIApp::OnInitCmdLine(wxCmdLineParser& parser)
{
    parser.SetDesc(g_cmdLineDesc);
}

bool mmGUIApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
    wxString ini_file;
    if (parser.Found("i", &ini_file))
        m_optParam2 = ini_file;

    if (parser.GetParamCount() > 0)
        m_optParam1 = parser.GetParam(0);

    if (parser.FoundSwitch("s")) {
        m_optParamSilent = true;
    }

    return true;
}

//----------------------------------------------------------------------------
/*
    See also: wxStackWalker, wxDebugReportUpload.
*/
void mmGUIApp::ReportFatalException(wxDebugReport::Context ctx)
{
    // TODO email it or upload it
    wxDebugReportCompress report;

    if (!report.IsOk())
    {
        return wxSafeShowMessage(mmex::getProgramName()
            , _("Fatal error occured.\nApplication will be terminated."));
    }

    report.AddAll(ctx);

    wxDebugReportPreviewStd preview;

    if (preview.Show(report) && report.Process()) {
        report.Reset();
    }
}
/*
    This method allows catching the exceptions thrown by any event handler.
*/
void mmGUIApp::HandleEvent(wxEvtHandler *handler, wxEventFunction func, wxEvent& event) const
{
    try
    {
        wxApp::HandleEvent(handler, func, event);
    }
    catch (const wxSQLite3Exception &e)
    {
        wxLogError(e.GetMessage());
    }
    catch (const std::exception &e)
    {
        wxLogError("%s", e.what());
    }
}

int mmGUIApp::FilterEvent(wxEvent &event)
{
    int ret = wxApp::FilterEvent(event);

    if (event.GetEventType() == wxEVT_SHOW)
    {
        wxWindow *win = static_cast<wxWindow*>(event.GetEventObject());

        if (win && win->IsTopLevel() && win != this->m_frame) // wxDialog & wxFrame http://docs.wxwidgets.org/trunk/classwx_top_level_window.html
        {
            Model_Usage::instance().pageview(win);
        }
    }

    return ret;
}

//----------------------------------------------------------------------------

void mmGUIApp::OnFatalException()
{
    ReportFatalException(wxDebugReport::Context_Exception);
}
//----------------------------------------------------------------------------

bool OnInitImpl(mmGUIApp* app)
{
    app->SetAppName(mmex::GetAppName());

    app->SetSettingDB(new wxSQLite3Database());
    wxString file_path = mmex::getPathUser(mmex::SETTINGS);
    if (!app->GetIniParam().empty()) {
        if (wxFileName::FileExists(app->GetIniParam()))
        {
            file_path = app->GetIniParam();
        }
    }
    app->GetSettingDB()->Open(file_path);
    Model_Setting::instance(app->GetSettingDB());

    Model_Setting::instance().ShrinkUsageTable();
    Model_Usage::instance(app->GetSettingDB());

    /* Load general MMEX Custom Settings */
    Option::instance().LoadOptions(false);

    /* initialize GUI with best language */
    wxTranslations *trans = new wxTranslations;
    trans->SetLanguage(wxLANGUAGE_DEFAULT);
    trans->AddStdCatalog();
    trans->AddCatalog("mmex", wxLANGUAGE_ENGLISH_US);
    wxTranslations::Set(trans);

    // Resource files
#ifndef __WXGTK__

    wxFileSystem::AddHandler(new wxMemoryFSHandler);

    //Copy files from resources to VFS
    const wxString res_dir = mmex::GetResourceDir().GetPathWithSep();
    wxArrayString files_array;
    wxDir::GetAllFiles(res_dir, &files_array);
    for (const auto& source_file : files_array)
    {
        wxString data;
        if (wxFileName::FileExists(source_file))
        {
            const auto file_name = wxFileName(source_file).GetFullName();
            const auto file_etx = wxFileName(file_name).GetExt();
            if (file_etx == "mo") continue;

            wxFileInputStream input(source_file);
            wxMemoryOutputStream memOut(nullptr);
            input.Read(memOut);
            wxStreamBuffer* buffer = memOut.GetOutputStreamBuffer();
            wxLogDebug("File: %s has been copied to VFS", file_name);
            wxMemoryFSHandler::AddFile(file_name, buffer->GetBufferStart()
                , buffer->GetBufferSize());
        }
    }

#else

    const wxString resDir = mmex::GetResourceDir().GetPathWithSep();
    const wxString tempDir = mmex::getTempFolder();
    wxFileName::Mkdir(tempDir, 511, wxPATH_MKDIR_FULL);
    wxArrayString filesArray;
    wxDir::GetAllFiles(resDir, &filesArray);
    for (const auto& sourceFile : filesArray)
    {
        const wxString repFile = tempDir + wxFileName(sourceFile).GetFullName();
        if (::wxFileExists(sourceFile))
        {
            if (!::wxFileExists(repFile)
                || wxFileName(sourceFile).GetModificationTime() > wxFileName(repFile).GetModificationTime())
            {
                if (!::wxCopyFile(sourceFile, repFile))
                    wxLogError("Could not copy %s !", sourceFile);
                else
                    wxLogDebug("Coping file:\n %s \nto\n %s", sourceFile, repFile);
            }
        }
    }

#endif


#if defined (__WXMSW__)
    // https://msdn.microsoft.com/en-us/library/ee330730(v=vs.85).aspx
    // https://kevinragsdale.net/windows-10-and-the-web-browser-control/
    wxRegKey Key(wxRegKey::HKCU, R"(Software\Microsoft\Internet Explorer\Main\FeatureControl\FEATURE_BROWSER_EMULATION)");
    if (!Key.Create(true) && Key.SetValue(wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetFullName(), 11001))
        wxASSERT(false);
#endif

    /* Initialize CURL */
    curl_global_init(CURL_GLOBAL_ALL);

    /* Initialize Image Handlers */
    wxInitAllImageHandlers();

    /* set preffered GUI language */
    app->setGUILanguage(Option::instance().getLanguageID());

    //Get System screen size
#ifdef _MSC_VER
    int sys_screen_x = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int sys_screen_y = GetSystemMetrics(SM_CYVIRTUALSCREEN);
#else
    int sys_screen_x = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
    int sys_screen_y = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);
#endif

    /* Load Dimensions of Window */
    int valx = Model_Setting::instance().GetIntSetting("ORIGINX", 50);
    int valy = Model_Setting::instance().GetIntSetting("ORIGINY", 50);
    int valw = Model_Setting::instance().GetIntSetting("SIZEW", sys_screen_x / 4 * 3);
    int valh = Model_Setting::instance().GetIntSetting("SIZEH", sys_screen_y / 4 * 3);

    //BUGFIX: #214 MMEX Window is "off screen"
    if (valx >= sys_screen_x) valx = sys_screen_x - valw;
    if (valy >= sys_screen_y) valy = sys_screen_y - valh;

    app->m_frame = new mmGUIFrame(app, mmex::getProgramName(), wxPoint(valx, valy), wxSize(valw, valh));

    bool ok = app->m_frame->Show();

    /* Was App Maximized? */
    bool isMax = Model_Setting::instance().GetBoolSetting("ISMAXIMIZED", true);
    if (isMax) app->m_frame->Maximize(true);

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned FALSE here, the
    // application would exit immediately.

    return ok;
}

//----------------------------------------------------------------------------
bool mmGUIApp::OnInit()
{
    bool ok = false;

    try
    {
        ok = wxApp::OnInit() && OnInitImpl(this);
    }
    catch (const wxSQLite3Exception &e)
    {
        wxLogError(e.GetMessage());
    }
    catch (const std::exception &e)
    {
        wxLogError("%s", e.what());
    }

    if (!ok) {
        wxSharedPtr<wxSQLite3Database> db;
        db = GetSettingDB();
        if (db) {
            db->Close();
        }
    }

    return ok;
}

int mmGUIApp::OnExit()
{
    wxLogDebug("OnExit()");
    Model_Usage::Data* usage = Model_Usage::instance().create();
    usage->USAGEDATE = wxDate::Today().FormatISODate();

    wxString rj = Model_Usage::instance().To_JSON_String();
    wxLogDebug("===== mmGUIApp::OnExit ===========================");
    wxLogDebug("RapidJson\n%s", rj);

    usage->JSONCONTENT = rj;
    Model_Usage::instance().save(usage);

    if (m_setting_db) {
        delete m_setting_db;
    }

    /* CURL Cleanup */
    curl_global_cleanup();

    //Delete mmex temp folder for current user
    wxFileName::Rmdir(mmex::getTempFolder(), wxPATH_RMDIR_RECURSIVE);

    return 0;
}
