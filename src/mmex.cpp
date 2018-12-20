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
#ifdef MMEX_WEBSERVER
#include "webserver.h"
#endif
#include "Model_Setting.h"
#include "Model_Usage.h"
#include "Model_Report.h"

#include <wx/cmdline.h>

//----------------------------------------------------------------------------
wxIMPLEMENT_APP(mmGUIApp);
//----------------------------------------------------------------------------

static const wxCmdLineEntryDesc g_cmdLineDesc [] =
{
    { wxCMD_LINE_PARAM, nullptr, nullptr, wxT_2("database file"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
    { wxCMD_LINE_NONE, nullptr, nullptr, nullptr, wxCMD_LINE_VAL_NONE, 0 }
};

//----------------------------------------------------------------------------

mmGUIApp::mmGUIApp(): m_frame(0)
    , m_setting_db(0)
    , m_optParam(wxEmptyString)
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

bool mmGUIApp::setGUILanguage(wxLanguage lang)
{
    if (lang == wxLANGUAGE_UNKNOWN) lang=wxLANGUAGE_DEFAULT;
    if (lang == this->m_lang) return false;
    wxTranslations *trans = new wxTranslations;
    trans->SetLanguage(lang);
    trans->AddStdCatalog();
    if (!trans->AddCatalog("mmex", wxLANGUAGE_ENGLISH_US) && lang != wxLANGUAGE_ENGLISH_US)
    {
        if (lang==wxLANGUAGE_DEFAULT)
            mmErrorDialogs::MessageWarning(NULL
                ,_("Can not load translation for default language")
                ,_("Language change"));
        else
            mmErrorDialogs::MessageWarning(NULL
                ,wxString::Format(_("Can not load translation for selected language %s"),
                    wxLocale::GetLanguageCanonicalName(lang))
                ,_("Language change"));
        wxDELETE(trans);
        return false;
    }
    wxTranslations::Set(trans);
    this->m_lang=lang;
    Option::instance().Language(lang);
    return true;
}

void mmGUIApp::OnInitCmdLine(wxCmdLineParser& parser)
{
    parser.SetDesc (g_cmdLineDesc);
}

bool mmGUIApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
    if(parser.GetParamCount() > 0)
        m_optParam = parser.GetParam(0);
    return true;
}

//----------------------------------------------------------------------------
/*
    See also: wxStackWalker, wxDebugReportUpload.
*/
void mmGUIApp::reportFatalException(wxDebugReport::Context ctx)
{
    // TODO email it or upload it
    wxDebugReportCompress report;

    if (!report.IsOk())
    {
        return wxSafeShowMessage(mmex::getProgramName()
            , _("Fatal error occurred.\nApplication will be terminated."));
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
        wxWindow *win = (wxWindow*)event.GetEventObject();

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
    reportFatalException(wxDebugReport::Context_Exception);
}
//----------------------------------------------------------------------------

bool OnInitImpl(mmGUIApp* app)
{
    app->SetAppName(mmex::GetAppName());

    /* initialize GUI with best language */
    wxTranslations *trans = new wxTranslations;
    trans->SetLanguage(wxLANGUAGE_DEFAULT);
    trans->AddStdCatalog();
    trans->AddCatalog("mmex", wxLANGUAGE_ENGLISH_US);
    wxTranslations::Set(trans);

    Model_Report::prepareTempFolder();
    Model_Report::WindowsUpdateRegistry();

    /* Initialize CURL */
    curl_global_init(CURL_GLOBAL_ALL);

    /* Initialize Image Handlers */
    wxInitAllImageHandlers();

    app->m_setting_db = new wxSQLite3Database();
    app->m_setting_db->Open(mmex::getPathUser(mmex::SETTINGS));
    Model_Setting::instance(app->m_setting_db);
    Model_Setting::instance().ShrinkUsageTable();

    Model_Usage::instance(app->m_setting_db);

    /* Load general MMEX Custom Settings */
    Option::instance().LoadOptions(false);

    /* set preffered GUI language */
    app->setGUILanguage(Option::instance().Language());

    /* Was App Maximized? */
    bool isMax = Model_Setting::instance().GetBoolSetting("ISMAXIMIZED", true);

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
    int valw = Model_Setting::instance().GetIntSetting("SIZEW", sys_screen_x/4*3);
    int valh = Model_Setting::instance().GetIntSetting("SIZEH", sys_screen_y/4*3);

    //BUGFIX: #214 MMEX Window is "off screen"
    if (valx >= sys_screen_x ) valx = sys_screen_x - valw;
    if (valy >= sys_screen_y ) valy = sys_screen_y - valh;

    app->m_frame = new mmGUIFrame(app, mmex::getProgramName(), wxPoint(valx, valy), wxSize(valw, valh));

#ifdef MMEX_WEBSERVER
    Mongoose_Service::instance().open();
#endif

    bool ok = app->m_frame->Show();
    if (isMax) app->m_frame->Maximize(true);

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned FALSE here, the
    // application would exit immediately.

    return ok;
}

//----------------------------------------------------------------------------
bool mmGUIApp::OnInit()
{
    m_checker = new wxSingleInstanceChecker;
    if (m_checker->IsAnotherRunning())
    {
        wxMessageBox(_(
            "MMEX is already running...\n\n"
            "Multiple instances are no longer supported."), _("MMEX Instance Check"));
        delete m_checker;
        return false;
    }

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

    //Document rapidjson;
    //rapidjson.Parse(rj);
    //wxLogDebug("===== mmGUIApp::OnExit ======== DOM Check ========");
    //wxLogDebug("RapidJson\n%s", JSON_PrettyFormated(rapidjson));

    usage->JSONCONTENT = rj;
    Model_Usage::instance().save(usage);

    if (m_setting_db) delete m_setting_db;

#ifdef MMEX_WEBSERVER
    Mongoose_Service::instance().stop();
#endif

    /* CURL Cleanup */
    curl_global_cleanup();

    //Delete mmex temp folder for current user
    wxFileName::Rmdir(mmex::getTempFolder(), wxPATH_RMDIR_RECURSIVE);

    delete m_checker;
    return 0;
}
