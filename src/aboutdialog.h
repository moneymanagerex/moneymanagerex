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

#ifndef MM_EX_ABOUTDIALOG_H_
#define MM_EX_ABOUTDIALOG_H_

#include <wx/dialog.h>
#include <wx/notebook.h>
#include "defs.h"

class wxHtmlWindow;

class mmAboutDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmAboutDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmAboutDialog();
    ~mmAboutDialog();
    mmAboutDialog(wxWindow* parent, int tabToOpenNo);

private:
    bool createWindow(wxWindow* parent
        , const wxString& caption
        , int tabToOpenNo
        , wxWindowID id = wxID_ANY
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxDefaultSize
        , long style = wxCAPTION | wxRESIZE_BORDER | wxCLOSE_BOX
        , const wxString &name = "mmAboutDialog"
        );
    void createControls(int tabToOpenNo);
    void initControls();
    void handleLink(wxHtmlLinkEvent& event);

    wxCheckBox* m_send_data = nullptr;
    wxHtmlWindow* aboutText_ = nullptr;
    wxHtmlWindow* authorsText_ = nullptr;
    wxHtmlWindow* sponsorsText_ = nullptr;
    wxHtmlWindow* licenseText_ = nullptr;
    wxHtmlWindow* privacyText_ = nullptr;

};

#endif // MM_EX_ABOUTDIALOG_H_
