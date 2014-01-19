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

#ifndef _MM_EX_TRANSDIALOG_H_
#define _MM_EX_TRANSDIALOG_H_

#include "defs.h"
#include "model/Model_Splittransaction.h"
#include "model/Model_Checking.h"

#include <wx/spinbutt.h>

class wxDatePickerCtrl;
class mmTextCtrl;

class mmTransDialog : public wxDialog
{
    DECLARE_DYNAMIC_CLASS( mmTransDialog )
    DECLARE_EVENT_TABLE()

public:
    mmTransDialog() {}
    virtual ~mmTransDialog();

    mmTransDialog(
        wxWindow* parent
        , int account_id
        , int transaction_id
    );

    bool Create( wxWindow* parent
        , wxWindowID id
        , const wxString& caption
        , const wxPoint& pos
        , const wxSize& size
        , long style
    );

    void SetDialogTitle(const wxString& title);
    int getAccountID() { return accountID_; }
    int getToAccountID() { return transaction_->TOACCOUNTID; }
    int getTransactionID() { return transaction_->TRANSID; }

private:
    void CreateControls();
    void dataToControls();
    bool validateData();

    void OnSplitChecked(wxCommandEvent& event);
    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnQuit(wxCloseEvent& event);
    void OnCategs(wxCommandEvent& event);
    void OnAccountOrPayeeUpdated(wxCommandEvent& event);
    void OnDpcKillFocus(wxFocusEvent& event);
    void OnAutoTransNum(wxCommandEvent& event);
    void OnTransTypeChanged(wxCommandEvent& event);
    void OnSpin(wxSpinEvent&);
    void OnDateChanged(wxDateEvent& event);
    void onFocusChange(wxChildFocusEvent& event);
    void onTextEntered(wxCommandEvent& event);
    void OnAdvanceChecked(wxCommandEvent& event);
    void activateSplitTransactionsDlg();
    void setTooltips();

    mmTextCtrl *textNumber_;
    mmTextCtrl *textAmount_;
    mmTextCtrl *toTextAmount_;
    mmTextCtrl *textNotes_;
    wxButton* bCategory_;
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

    bool m_transfer;
    bool categUpdated_;
    bool advancedToTransAmountSet_;

    // store the original currency rate for transaction editing
    double  edit_currency_rate;
    wxString amountNormalTip_;
    wxString amountTransferTip_;
    wxString notesTip_;
    wxColour notesColour_;
    int object_in_focus_;
    wxString resetPayeeString(/*bool normal = true*/);

    Model_Checking::Data * transaction_;
    Model_Splittransaction::Data_Set m_local_splits;
    int accountID_; //The transaction account ID //TODO: may be Model_Account ?
    int referenceAccountID_; // used for transfer transactions
    int transaction_id_; //The transaction ID. NULL if new transaction

    bool skip_account_init_;
    bool skip_amount_init_;
    bool skip_payee_init_;
    bool skip_status_init_;
    bool skip_notes_init_;
    bool skip_category_init_;

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
    };
};

#endif
