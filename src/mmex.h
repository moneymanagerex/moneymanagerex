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
    virtual int FilterEvent(wxEvent& event);

    wxLanguage getGUILanguage() const;
    bool setGUILanguage(wxLanguage lang);
    wxLocale& getLocale();
    virtual void OnInitCmdLine(wxCmdLineParser& parser);
    virtual bool OnCmdLineParsed(wxCmdLineParser& parser);
    mmGUIFrame* m_frame;
    const wxString GetOptParam() const;
    const wxString GetIniParam() const;
    wxSQLite3Database* GetSettingDB() const;
    void SetSettingDB(wxSQLite3Database* db);

private:
    wxString m_optParam1;
    wxString m_optParam2;
    wxSQLite3Database* m_setting_db;
    void ReportFatalException(wxDebugReport::Context);
    bool OnInit();
    int OnExit();
    void OnFatalException(); // called when a crash occurs in this application
    void HandleEvent(wxEvtHandler *handler, wxEventFunction func, wxEvent& event) const;
    wxLanguage m_lang; // GUI translation language displayed
    wxLocale m_locale;

};

inline wxSQLite3Database* mmGUIApp::GetSettingDB() const { return m_setting_db; }
inline const wxString mmGUIApp::GetOptParam() const { return m_optParam1; }
inline const wxString mmGUIApp::GetIniParam() const { return m_optParam2; }
inline void mmGUIApp::SetSettingDB(wxSQLite3Database* db) { m_setting_db = db; }

//----------------------------------------------------------------------------
wxDECLARE_APP(mmGUIApp);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
#endif // MM_EX_MMEX_H_
//----------------------------------------------------------------------------
