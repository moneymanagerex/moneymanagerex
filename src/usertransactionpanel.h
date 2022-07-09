/*******************************************************
Copyright (C) 2014 Nikolay Akimov
Copyright (C) 2015 Stefano Giorgio

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

#include "constants.h"
#include "defs.h"
#include "mmSimpleDialogs.h"
#include "mmTextCtrl.h"
#include "model/Model_Translink.h"

class Model_Currency;


class UserTransactionPanel : public wxPanel
{
    wxDECLARE_EVENT_TABLE();

public:
    UserTransactionPanel();

    UserTransactionPanel(wxWindow *parent
        , Model_Checking::Data* checking_entry
        , wxWindowID win_id = wxID_ANY
        , const wxPoint &pos = wxDefaultPosition
        , const wxSize &size = wxDefaultSize
        , long style = wxTAB_TRAVERSAL
        , const wxString &name = "UserTransactionPanel");

    ~UserTransactionPanel();

    int SaveChecking();
    bool ValidCheckingAccountEntry();
    wxDateTime TransactionDate();
    void TransactionDate(const wxDateTime& trans_date);

    void SetTransactionValue(const double& trans_value, bool fixed_value = false);
    void SetTransactionNumber(const wxString& trans_number);
    void SetTransactionAccount(const wxString& trans_account);

    int TransactionType();
    Model_Translink::CHECKING_TYPE CheckingType();
    void CheckingType(Model_Translink::CHECKING_TYPE ct);

private:
    Model_Checking::Data* m_checking_entry;
    int m_transaction_id;

    int m_account_id;
    int m_payee_id;
    int m_category_id;
    int m_subcategory_id;

private:
    void Create();
    void DataToControls();
    void SetLastPayeeAndCategory(const int account_id);

    void OnTransAccountButton(wxCommandEvent& WXUNUSED(event));
    void OnTransPayeeButton(wxCommandEvent& WXUNUSED(event));
    void OnTransCategoryButton(wxCommandEvent& WXUNUSED(event));

    void OnEnteredText(wxCommandEvent& event);
    void OnFrequentNotes(wxCommandEvent& event);
    void onSelectedNote(wxCommandEvent& event);
    void OnAttachments(wxCommandEvent& WXUNUSED(event));

private:
    mmDatePickerCtrl* m_date_selector;

    wxButton* m_account;
    wxChoice* m_status_selector;
    wxChoice* m_type_selector;
    mmTextCtrl* m_entered_amount;
    wxCheckBox* m_transfer;
    wxButton* m_trans_currency;
    wxButton* m_payee;
    wxButton* m_category;
    wxTextCtrl* m_entered_number;
    wxTextCtrl* m_entered_notes;
    std::vector<wxString> m_frequent_notes;
    wxBitmapButton* m_attachment;
 
    enum
    {
        ID_TRANS_DATE_SELECTOR = wxID_HIGHEST + 10,
        ID_TRANS_DATE_CONTROLLER,
        ID_TRANS_STATUS_SELECTOR,
        ID_TRANS_ACCOUNT_BUTTON_TEXT,
        ID_TRANS_ACCOUNT_BUTTON,
        ID_TRANS_PAYEE_BUTTON_TEXT,
        ID_TRANS_PAYEE_BUTTON,
        ID_TRANS_CATEGORY_BUTTON,
        ID_TRANS_ENTERED_NUMBER,
        ID_TRANS_ENTERED_AMOUNT,
        ID_TRANS_ENTERED_NOTES,
        ID_TRANS_FREQUENT_NOTES,
        ID_TRANS_TRANSFER,
        ID_TRANS_CURRENCY_BUTTON,
    };
};
