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

//----------------------------------------------------------------------------
#ifndef MM_EX_MMEX_H_
#define MM_EX_MMEX_H_
//----------------------------------------------------------------------------
#include <wx/app.h>
#include <wx/debugrpt.h>
#include <wx/snglinst.h>

//----------------------------------------------------------------------------
class mmGUIFrame;
class wxSQLite3Database;
//----------------------------------------------------------------------------

class mmGUIApp : public wxApp
{
public:
    mmGUIApp();

    wxLanguage getGUILanguage() const;
    bool setGUILanguage(wxLanguage lang);
    virtual void OnInitCmdLine(wxCmdLineParser& parser);
    virtual bool OnCmdLineParsed(wxCmdLineParser& parser);
    mmGUIFrame* m_frame;
    wxSQLite3Database* m_setting_db;
    wxString m_optParam;

private:
    void reportFatalException(wxDebugReport::Context);
    bool OnInit();
    int OnExit();
    void OnFatalException(); // called when a crash occurs in this application
    void HandleEvent(wxEvtHandler *handler, wxEventFunction func, wxEvent& event) const;
    wxSingleInstanceChecker* m_checker;

    wxLanguage m_lang; // GUI translation language displayed
    wxLocale m_locale;
public:
    virtual int FilterEvent(wxEvent& event);
};

//----------------------------------------------------------------------------
wxDECLARE_APP(mmGUIApp);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
#endif // MM_EX_MMEX_H_
//----------------------------------------------------------------------------
