/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011 Stefano Giorgio

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

class relocatePayeeDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(relocatePayeeDialog);
    wxDECLARE_EVENT_TABLE();

public:
    relocatePayeeDialog();
    relocatePayeeDialog(wxWindow* parent, int source_payee_id = -1);

    int updatedPayeesCount();

private:
    bool Create(wxWindow* parent
        , wxWindowID id
        , const wxString& caption
        , const wxPoint& pos
        , const wxSize& size
        , long style);

    void CreateControls();
    void IsOkOk();
    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);

    void OnPayeeChanged(wxCommandEvent& event);

    int sourcePayeeID_;
    int destPayeeID_;

    wxComboBox* cbSourcePayee_;
    wxComboBox* cbDestPayee_;
    int m_changed_records;
    wxStaticText* m_info;
    wxCheckBox* cbDelete_;
};

#endif
