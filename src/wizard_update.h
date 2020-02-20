/*******************************************************
 Copyright (C) 2014 Gabriele-V

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
#pragma once

#include <wx/wizard.h>
#include <wx/frame.h>
#include "option.h"

class mmUpdateWizard : public wxWizard
{
public:
    mmUpdateWizard(wxWindow* parent);

private:
    void Create(wxWindow* parent
        , wxWindowID id
        , const wxString& caption
        , const wxPoint& pos
        , const wxSize& size
        , long style
        , const wxString& name = "mmUpdateWizard"
    );
    void CreateControls();
    void checkUpdates();
    wxHtmlWindow* update_text_;
    void LinkClicked(wxHtmlLinkEvent& WXUNUSED(event));
    void OnCancel(wxCommandEvent& /*event*/);

    wxDECLARE_EVENT_TABLE();
};
//----------------------------------------------------------------------------
