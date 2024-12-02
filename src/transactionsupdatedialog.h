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
    transactionsUpdateDialog(wxWindow* parent, std::vector<int64>& transaction_id);

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
    void OnTransTypeChanged(wxCommandEvent&);
    void OnMoreFields(wxCommandEvent& event);

private:
    wxCheckBox* m_payee_checkbox = nullptr;
    mmComboBoxPayee* cbPayee_ = nullptr;
    wxCheckBox* m_transferAcc_checkbox = nullptr;
    mmComboBoxAccount* cbAccount_ = nullptr;
    wxCheckBox* m_date_checkbox = nullptr;
    mmDatePickerCtrl* m_dpc = nullptr;
    wxCheckBox* m_time_checkbox = nullptr;
    wxTimePickerCtrl* m_time_ctrl = nullptr;
    wxCheckBox* m_status_checkbox = nullptr;
    wxChoice* m_status_choice = nullptr;
    wxCheckBox* m_categ_checkbox = nullptr;
    mmComboBoxCategory* cbCategory_ = nullptr;
    mmColorButton* bColours_ = nullptr;
    wxCheckBox* m_color_checkbox = nullptr;
    wxCheckBox* m_type_checkbox = nullptr;
    wxCheckBox* tag_checkbox_ = nullptr;
    wxCheckBox* tag_append_checkbox_ = nullptr;
    mmTagTextCtrl* tagTextCtrl_ = nullptr;
    wxChoice* m_type_choice = nullptr;
    wxCheckBox* m_amount_checkbox = nullptr;
    mmTextCtrl* m_amount_ctrl = nullptr;
    wxCheckBox* m_notes_checkbox = nullptr;
    wxCheckBox* m_append_checkbox = nullptr;
    wxTextCtrl* m_notes_ctrl = nullptr;
    std::vector<int64> m_transaction_id;
    Model_Currency::Data* m_currency = nullptr;
    bool m_hasTransfers = false, m_hasNonTransfers = false, m_hasSplits = false;
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
