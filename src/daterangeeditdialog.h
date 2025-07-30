/*******************************************************
 Copyright (C) 2025 Klaus Wich

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

#include "daterange2.h"
#include <wx/dataview.h>


class mmDateRangeEditDialog: public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmFilterTransactionsDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmDateRangeEditDialog();
    mmDateRangeEditDialog(wxWindow* parent, wxString* name, wxString* range);

    enum
    {
        CTRL_ANY = 0,
        TXT_CTRL_NAME = 0,
        TXT_CTRL_RANGE
    };

private:
    std::vector<DateRange2::Spec>* m_date_ranges_ptr;
    int m_selected_row;
    wxString* m_name_ptr;
    wxString* m_range_ptr;
    wxColour m_defBColor;

    wxTextCtrl* m_name_edit;
    wxTextCtrl* m_range_edit;
    wxStaticText* m_status;
    wxStaticText* m_to_st;

    wxStaticText* m_from_st;
    wxChoice* m_count1;
    wxChoice* m_period1;
    wxChoice* m_count2;
    wxChoice* m_period2;

    void CreateControls();
    bool checkRange();
    void showSelectControls(bool status);

    void OnOk(wxCommandEvent&);
    void OnRange(wxCommandEvent&);
    void OnUpdateRangeFromControls(wxCommandEvent&);
    void updateControlsFromRange(wxString range);
};
