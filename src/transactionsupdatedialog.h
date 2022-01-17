/*******************************************************
 Copyright (C) 2017 - 2021 Nikolay Akimov
 Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)

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

#include "defs.h"
#include "mmcombobox.h"
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
        , wxWindowID id
        , const wxString& caption
        , const wxPoint& pos
        , const wxSize& size
        , long style);

    void CreateControls();
    void SetEventHandlers();
    void OnOk(wxCommandEvent& event);
    void OnCheckboxClick(wxCommandEvent& event);
    void onFocusChange(wxChildFocusEvent& event);
    void OnCategChange(wxCommandEvent& event);
    void OnPayeeUpdated(wxCommandEvent& event);
    void OnAccountUpdated(wxCommandEvent& event);
    void SetPayeeTransferControls();
    void OnTransTypeChanged(wxCommandEvent& event);

private:
    wxCheckBox* m_payee_checkbox;
    wxComboBox* m_payee;
    wxCheckBox* m_transferAcc_checkbox;
    wxComboBox* m_transferAcc;    
    wxCheckBox* m_date_checkbox;
    wxDatePickerCtrl* m_dpc;
    wxCheckBox* m_status_checkbox;
    wxChoice* m_status_choice;
    wxCheckBox* m_categ_checkbox;
    wxButton* m_categ_btn;
    wxCheckBox* m_type_checkbox;
    wxChoice* m_type_choice;
    wxCheckBox* m_amount_checkbox;
    mmTextCtrl* m_amount_ctrl;
    wxCheckBox* m_notes_checkbox;
    wxCheckBox* m_append_checkbox;
    wxTextCtrl* m_notes_ctrl;
    std::vector<int> m_transaction_id;
    Model_Currency::Data* m_currency;
    int m_categ_id;
    int m_subcateg_id;
    bool m_hasTransfers, m_hasNonTransfers, m_hasSplits;

    enum
    {
        /* Transaction Dialog */
        ID_PAYEE = wxID_HIGHEST + 900,
        ID_TRANS_TYPE,
        ID_TRANS_ACC
    };
};

#endif
