/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011 Nikolay & Stefano Giorgio

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

#ifndef MM_EX_TRANSDIALOG_H_
#define MM_EX_TRANSDIALOG_H_

#include "defs.h"
#include "customfielddialog.h"

#include "model/Model_Checking.h"
#include "model/Model_Payee.h"
#include "model/Model_Splittransaction.h"

#include <wx/spinbutt.h>

class wxDatePickerCtrl;
class mmTextCtrl;

class mmTransDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmTransDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmTransDialog() {}
    virtual ~mmTransDialog();

    mmTransDialog(
        wxWindow* parent
        , int account_id
        , int transaction_id
        , double current_balance
        , bool duplicate = false
        , int type = Model_Checking::WITHDRAWAL
    );

    bool Create(wxWindow* parent
        , wxWindowID id
        , const wxString& caption
        , const wxPoint& pos
        , const wxSize& size
        , long style
    );

    void SetDialogTitle(const wxString& title);
    int getAccountID() { return m_trx_data.ACCOUNTID; }
    int getToAccountID() { return m_trx_data.TOACCOUNTID; }
    int getTransactionID() { return m_trx_data.TRANSID; }

private:
    void CreateControls();
    void dataToControls();
    bool validateData();

    void OnSplitChecked(wxCommandEvent& event);
    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnQuit(wxCloseEvent& event);
    void OnStartupCustomFields();
    void OnMove(wxMoveEvent& event);
    void OnCategs(wxCommandEvent& event);
    void OnAttachments(wxCommandEvent& event);
    void OnCustomFields(wxCommandEvent& event);
    void OnAccountOrPayeeUpdated(wxCommandEvent& event);
    void OnDpcKillFocus(wxFocusEvent& event);
    void OnAutoTransNum(wxCommandEvent& event);
    void OnFrequentUsedNotes(wxCommandEvent& event);
    void onNoteSelected(wxCommandEvent& event);
    void OnTransTypeChanged(wxCommandEvent& event);
    void OnSpin(wxSpinEvent&);
    void OnDateChanged(wxDateEvent& event);
    void onFocusChange(wxChildFocusEvent& event);
    void onTextEntered(wxCommandEvent& event);
    void OnAdvanceChecked(wxCommandEvent& event);
    void activateSplitTransactionsDlg();
    void setTooltips();
    void setCategoryForPayee(const Model_Payee::Data *payee);

    mmTextCtrl *textNumber_;
    mmTextCtrl *textAmount_;
    mmTextCtrl *toTextAmount_;
    wxTextCtrl *textNotes_;
    wxButton* bCategory_;
    wxButton* bAttachments_;
    wxButton* bCustomFields_;
    wxComboBox* cbAccount_;
    wxComboBox* cbPayee_;
    wxCheckBox* cSplit_;
    wxCheckBox* cAdvanced_;
    wxButton* itemButtonCancel_;
    wxChoice* choiceStatus_;
    wxChoice* transaction_type_;
    wxDatePickerCtrl* dpc_;
    wxSpinButton *spinCtrl_;
    wxStaticText* itemStaticTextWeek_;
    wxStaticText* account_label_;
    wxStaticText* payee_label_;
    mmCustomFieldDialog* CustomFieldDialog_;

    bool m_transfer;
    bool m_new_trx;
    bool m_duplicate;
    bool categUpdated_;
    bool m_advanced;
    int m_account_id;
    double m_current_balance;

    int object_in_focus_;

    DB_Table_CHECKINGACCOUNT_V1::Data m_trx_data;
    std::vector<Split> local_splits;
    Model_Currency::Data *m_currency;
    Model_Currency::Data *m_to_currency;

    std::vector<wxString> frequentNotes_;

    bool skip_date_init_;
    bool skip_account_init_;
    bool skip_amount_init_;
    bool skip_payee_init_;
    bool skip_status_init_;
    bool skip_notes_init_;
    bool skip_category_init_;
    bool skip_tooltips_init_;

    enum
    {
        /* Transaction Dialog */
        ID_DIALOG_TRANS_TYPE = wxID_HIGHEST + 900,
        ID_DIALOG_TRANS_TEXTNUMBER,
        ID_DIALOG_TRANS_BUTTONDATE,
        ID_DIALOG_TRANS_TEXTNOTES,
        ID_DIALOG_TRANS_TEXTAMOUNT,
        ID_DIALOG_TRANS_TOTEXTAMOUNT,
        ID_DIALOG_TRANS_STATIC_PAYEE,
        ID_DIALOG_TRANS_BUTTONPAYEE,
        ID_DIALOG_TRANS_BUTTONTO,
        ID_DIALOG_TRANS_STATUS,
        ID_DIALOG_TRANS_ADVANCED_CHECKBOX,
        ID_DIALOG_TRANS_ADVANCED_FROM,
        ID_DIALOG_TRANS_ADVANCED_TO,
        ID_DIALOG_TRANS_SPLITCHECKBOX,
        ID_DIALOG_TRANS_BUTTONTRANSNUM,
        ID_DIALOG_TRANS_PAYEECOMBO,
        ID_DIALOG_TRANS_BUTTON_FREQENTNOTES,
        ID_DIALOG_TRANS_DATE_SPINNER,
        ID_DIALOG_TRANS_CUSTOMFIELDS,
    };
};

#endif
