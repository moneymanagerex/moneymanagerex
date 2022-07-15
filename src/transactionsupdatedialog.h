/*******************************************************
 Copyright (C) 2017 - 2022 Nikolay Akimov
 Copyright (C) 2021-2022 Mark Whalley (mark@ipx.co.uk)

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

#ifndef MM_EX_TRANSACTIONSUPDATE_H_
#define MM_EX_TRANSACTIONSUPDATE_H_

#include "mmSimpleDialogs.h"
#include "defs.h"
#include "mmcombobox.h"
#include "mmcustomdata.h"
class mmTextCtrl;

class transactionsUpdateDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(transactionsUpdateDialog);
    wxDECLARE_EVENT_TABLE();

public:
    transactionsUpdateDialog();
    ~transactionsUpdateDialog();
    transactionsUpdateDialog(wxWindow* parent, std::vector<int>& transaction_id);

private:
    bool Create(wxWindow* parent
        , wxWindowID id = wxID_ANY
        , const wxString& caption = wxTRANSLATE("Multi Transactions Update")
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxSize(500, 300)
        , long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX);

    void CreateControls();
    void OnOk(wxCommandEvent& event);
    void OnCheckboxClick(wxCommandEvent& event);
    void OnComboKey(wxKeyEvent& event);
    void onFocusChange(wxChildFocusEvent& event);
    void SetPayeeTransferControls();
    void OnTransTypeChanged(wxCommandEvent& event);
    void OnMoreFields(wxCommandEvent& event);

private:
    wxCheckBox* m_payee_checkbox;
    mmComboBoxPayee* cbPayee_;
    wxCheckBox* m_transferAcc_checkbox;
    mmComboBoxAccount* cbAccount_;    
    wxCheckBox* m_date_checkbox;
    mmDatePickerCtrl* m_dpc;
    wxCheckBox* m_status_checkbox;
    wxChoice* m_status_choice;
    wxCheckBox* m_categ_checkbox;
    mmComboBoxCategory* cbCategory_;
    mmColorButton* bColours_;
    wxCheckBox* m_color_checkbox;
    wxCheckBox* m_type_checkbox;
    wxChoice* m_type_choice;
    wxCheckBox* m_amount_checkbox;
    mmTextCtrl* m_amount_ctrl;
    wxCheckBox* m_notes_checkbox;
    wxCheckBox* m_append_checkbox;
    wxTextCtrl* m_notes_ctrl;
    std::vector<int> m_transaction_id;
    Model_Currency::Data* m_currency;
    bool m_hasTransfers, m_hasNonTransfers, m_hasSplits;
    wxSharedPtr<mmCustomData> m_custom_fields;

    enum
    {
        /* Transaction Dialog */
        ID_PAYEE = wxID_HIGHEST + 997,
        ID_TRANS_TYPE,
        ID_TRANS_ACC,
        ID_BTN_CUSTOMFIELDS,
        ID_CUSTOMFIELDS,
    };
};

#endif
