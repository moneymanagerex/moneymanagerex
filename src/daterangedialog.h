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


class mmDateRangeDialog: public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmFilterTransactionsDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmDateRangeDialog();
    mmDateRangeDialog(wxWindow* parent, std::vector<DateRange2::Spec>* dateRangesPtr, int* subMenuBeginPtr);

    enum
    {
        BTN_BASE = 0,
        BTN_UP_TOP,
        BTN_UP,
        BTN_EDIT,
        BTN_NEW,
        BTN_DOWN,
        BTN_DOWN_BOTTOM,
        BTN_DELETE,
        BTN_DEFAULT
    };

private:
    std::vector<DateRange2::Spec>* m_dateRangesPtr;
    int* m_subMenuBeginPtr;
    int m_subMenuBegin;
    int m_selected_row;
    bool m_hasChanged = false;
    wxString m_subMenuHeader = "==== " + _t("More date ranges…");

    wxDataViewListCtrl* m_dateRangesLb = nullptr;
    wxButton* m_up_top = nullptr;
    wxBitmapButton* m_up = nullptr;
    wxButton* m_edit = nullptr;
    wxBitmapButton* m_down = nullptr;
    wxButton* m_down_bottom = nullptr;
    wxButton* m_delete = nullptr;

    void CreateControls();
    void fillControls();
    void updateButtonState(bool setselected = true);

    void OnTop(wxCommandEvent&);
    void OnUp(wxCommandEvent&);
    void OnEdit(wxCommandEvent&);
    void OnDown(wxCommandEvent&);
    void OnBottom(wxCommandEvent&);
    void OnNew(wxCommandEvent&);
    void OnOk(wxCommandEvent&);
    void OnCancel(wxCommandEvent&);
    void OnDelete(wxCommandEvent&);
    void OnDefault(wxCommandEvent&);
    void OnDateRangeSelected(wxDataViewEvent& event);

    // -- Support functions ---
    wxVector<wxVariant> getItemData(int row);
    void exchangeItemData(int row1, int row2);
    void insertItemToLb(int pos, wxString name, wxString range);
};
