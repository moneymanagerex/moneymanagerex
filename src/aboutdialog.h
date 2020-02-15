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
    mmAboutDialog() {}
    mmAboutDialog(wxWindow* parent, int TabToOpen, const wxString &name = "mmAboutDialog");

private:
    bool Create(wxWindow* parent
        , wxWindowID id
        , const wxString& caption
        , const wxPoint& pos
        , const wxSize& size
        , long style
        , int TabToOpen
        , const wxString &name
        );
    void CreateControls(int TabToOpen);
    void InitControls();
    void OnVersionHistory(wxCommandEvent& event);
    void OnContributerList(wxCommandEvent& event);
    void OnLinkClicked(wxHtmlLinkEvent& event);

    wxHtmlWindow* about_text_;
    wxHtmlWindow* authors_text_;
    wxHtmlWindow* sponsors_text_;
    wxHtmlWindow* license_text_;
    wxHtmlWindow* privacy_text_;

};

#endif // MM_EX_ABOUTDIALOG_H_

