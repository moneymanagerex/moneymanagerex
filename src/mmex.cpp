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
#include <wx/cmdline.h>

#include "mmex.h"
#include "mmframe.h"
#include "util.h"
#include "constants.h"
#include "paths.h"
#include "platfdep.h"

#include "model/Model_Setting.h"


#include <wx/fs_mem.h>
//----------------------------------------------------------------------------
/* Include XPM Support */
#include "../resources/money.xpm"

//----------------------------------------------------------------------------
IMPLEMENT_APP(mmGUIApp)
//----------------------------------------------------------------------------

static const wxCmdLineEntryDesc g_cmdLineDesc [] =
{
    { wxCMD_LINE_PARAM,  NULL, NULL, _("database file"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL },
    { wxCMD_LINE_NONE }
};

//----------------------------------------------------------------------------

mmGUIApp::mmGUIApp(): m_frame(0), m_setting_db(0), m_optParam("")
{
#if wxUSE_ON_FATAL_EXCEPTION
    // catch fatal exceptions
    wxHandleFatalExceptions(true);
#endif
}

wxLocale& mmGUIApp::getLocale()
{
    return this->m_locale;
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
    wxDebugReportCompress rep;

    if (!rep.IsOk())
    {
        wxSafeShowMessage(mmex::getProgramName(), _("Fatal error occured.\nApplication will be terminated."));
        return;
    }

    rep.AddAll(ctx);

    wxDebugReportPreviewStd preview;

    if (preview.Show(rep) && rep.Process()) {
        rep.Reset();
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
//----------------------------------------------------------------------------

void mmGUIApp::OnFatalException()
{
    reportFatalException(wxDebugReport::Context_Exception);
}
//----------------------------------------------------------------------------

bool OnInitImpl(mmGUIApp* app)
{
    app->SetAppName(mmex::GetAppName());

    /* Setting Locale causes unexpected problems, so default to English Locale */
    app->getLocale().Init(wxLANGUAGE_ENGLISH);

    /* Initialize Image Handlers */
    wxImage::AddHandler(new wxICOHandler());
    wxImage::AddHandler(new wxJPEGHandler());
    wxImage::AddHandler(new wxPNGHandler());

    /* Initialize File System Handlers */
    wxFileSystem::AddHandler(new wxMemoryFSHandler);

    app->m_setting_db = new wxSQLite3Database();
    app->m_setting_db->Open(mmex::getPathUser(mmex::SETTINGS));
    Model_Setting::instance(app->m_setting_db);

    /* Force setting MMEX language parameter if it has not been set. */
    mmSelectLanguage(0, !Model_Setting::instance().ContainsSetting(LANGUAGE_PARAMETER));

    /* Load Colors from Database */
    mmLoadColorsFromDatabase();

    /* Load MMEX Custom Settings */
    mmIniOptions::instance().loadOptions();

    /* Was App Maximized? */
    bool isMax = Model_Setting::instance().GetBoolSetting("ISMAXIMIZED", false);

    //Get System screen size
    int sys_screen_x = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
    int sys_screen_y = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);

    /* Load Dimensions of Window */
    int valx = Model_Setting::instance().GetIntSetting("ORIGINX", 50);
    int valy = Model_Setting::instance().GetIntSetting("ORIGINY", 50);
    int valw = Model_Setting::instance().GetIntSetting("SIZEW", sys_screen_x/4*3);
    int valh = Model_Setting::instance().GetIntSetting("SIZEH", sys_screen_y/4*3);

    //BUGFIX: #214 MMEX Window is "off screen"
    if (valx >= sys_screen_x ) valx = sys_screen_x - valw;
    if (valy >= sys_screen_y ) valy = sys_screen_y - valh;

    app->m_frame = new mmGUIFrame(mmex::getProgramName(), wxPoint(valx, valy), wxSize(valw, valh));

    new wxSplashScreen(wxBitmap(money_xpm),
        wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_TIMEOUT,
        4000, app->m_frame, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxSIMPLE_BORDER | wxSTAY_ON_TOP);

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
    if (m_setting_db) delete m_setting_db;

    return 0;
}
