/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011 Stefano Giorgio
 Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2016, 2020 - 2022 Nikolay Akimov

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

#ifndef MM_EX_RELOCATEPAYEEDIALOG_H_
#define MM_EX_RELOCATEPAYEEDIALOG_H_

#include "defs.h"
#include "mmSimpleDialogs.h"

class relocatePayeeDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(relocatePayeeDialog);

public:
    relocatePayeeDialog();
    relocatePayeeDialog(wxWindow* parent, int source_payee_id = -1);

    int updatedPayeesCount() const;

private:
    bool Create(wxWindow* parent
        , wxWindowID id = wxID_ANY
        , const wxString& caption = _("Relocate Payee Dialog")
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxDefaultSize
        , long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX);

    void CreateControls();
    void IsOkOk();
    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnTextUpdated(wxCommandEvent& event);

    void OnFocusChange(wxChildFocusEvent& event);

    int sourcePayeeID_;
    int destPayeeID_;

    mmComboBoxUsedPayee* cbSourcePayee_;
    mmComboBoxPayee* cbDestPayee_;
    int m_changed_records;
    wxStaticText* m_info;
    wxCheckBox* cbDeleteSourcePayee_;
    wxDECLARE_EVENT_TABLE();
};

inline int relocatePayeeDialog::updatedPayeesCount() const { return m_changed_records; }

#endif
