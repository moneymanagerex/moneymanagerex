/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011-2017 Nikolay Akimov
 Copyright (C) 2011-2017 Stefano Giorgio [stef145g]
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

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

#include "mmcustomdata.h"
#include "defs.h"
#include "mmSimpleDialogs.h"

#include "Model_Checking.h"
#include "Model_Payee.h"
#include "Model_CustomFieldData.h"
#include "Model_Splittransaction.h"

class mmTextCtrl;
class wxString;
class mmCustomData;

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
        , const wxString& name = "Transactions Dialog"
    );

    bool Create(wxWindow* parent
        , wxWindowID id = wxID_ANY
        , const wxString& caption = "Transactions Dialog"
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxDefaultSize
        , long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX
        , const wxString& name = "Transactions Dialog"
    );

    void SetDialogTitle(const wxString& title);
    int GetAccountID() { return m_trx_data.ACCOUNTID; }
    int GetToAccountID() { return m_trx_data.TOACCOUNTID; }
    int GetTransactionID() { return m_trx_data.TRANSID; }

private:
    wxSharedPtr<mmCustomData> m_custom_fields;
    void CreateControls();
    void dataToControls();
    bool ValidateData();
    void SetEventHandlers();

    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnMoreFields(wxCommandEvent& event);
    void OnQuit(wxCloseEvent& event);
    void OnCategs(wxCommandEvent& event);
    void OnAttachments(wxCommandEvent& event);
    void OnComboKey(wxKeyEvent& event);
    void OnDpcKillFocus(wxFocusEvent& event);
    void OnAutoTransNum(wxCommandEvent& event);
    void OnFrequentUsedNotes(wxCommandEvent& event);
    void OnNoteSelected(wxCommandEvent& event);
    void OnTransTypeChanged(wxCommandEvent& event);
    void OnPayeeChanged(wxCommandEvent& event);
    void OnFocusChange(wxChildFocusEvent& event);
    void OnTextEntered(wxCommandEvent& event);
    void OnAdvanceChecked(wxCommandEvent& event);
    void SetTooltips();
    void SetCategoryForPayee(const Model_Payee::Data *payee = nullptr);
private:
    mmTextCtrl* textNumber_;
    mmTextCtrl* m_textAmount;
    mmTextCtrl* toTextAmount_;
    wxTextCtrl* textNotes_;
    wxButton* bAttachments_;
    mmColorButton* bColours_;
    wxStaticText* account_label_;
    wxStaticText* categ_label_;
    mmComboBoxAccount* cbAccount_;
    wxStaticText* to_acc_label_;
    mmComboBoxAccount* cbToAccount_;
    wxStaticText* payee_label_;
    mmComboBoxPayee* cbPayee_;
    mmComboBoxCategory* cbCategory_;
    wxBitmapButton* bSplit_;
    wxCheckBox* cAdvanced_;
    wxButton* m_button_cancel;
    wxChoice* choiceStatus_;
    wxChoice* transaction_type_;
    mmDatePickerCtrl* dpc_;

    bool m_transfer;
    bool m_new_trx;
    bool m_duplicate;
    bool m_advanced;
    double m_current_balance;

    int object_in_focus_;
    int m_account_id;
    wxString m_status;

    DB_Table_CHECKINGACCOUNT_V1::Data m_trx_data;
    std::vector<Split> m_local_splits;

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
        ID_DIALOG_TRANS_TYPE = wxID_HIGHEST + 897,
        ID_DIALOG_TRANS_TEXTNUMBER,
        ID_DIALOG_TRANS_BUTTONDATE,
        ID_DIALOG_TRANS_TEXTNOTES,
        mmID_TEXTAMOUNT, //6900
        mmID_TOTEXTAMOUNT,
        ID_DIALOG_TRANS_STATIC_PAYEE,
        ID_DIALOG_TRANS_BUTTONPAYEE,
        ID_DIALOG_TRANS_BUTTONTO,
        ID_DIALOG_TRANS_STATUS,
        ID_DIALOG_TRANS_ADVANCED_CHECKBOX,
        ID_DIALOG_TRANS_ADVANCED_FROM,
        ID_DIALOG_TRANS_ADVANCED_TO,
        ID_DIALOG_TRANS_CATEGLABEL2,
        ID_DIALOG_TRANS_BUTTONTRANSNUM,
        mmID_TOACCOUNT_LABEL,
        mmID_TOACCOUNTNAME,
        mmID_PAYEE_LABEL,
        mmID_PAYEE,
        mmID_CATEGORY,
        mmID_CATEGORY_SPLIT,
        mmID_ACCOUNTNAME,
        ID_DIALOG_TRANS_BUTTON_FREQENTNOTES,
        ID_DIALOG_TRANS_CUSTOMFIELDS,
        ID_CUSTOMFIELD,
    };


};

#endif
