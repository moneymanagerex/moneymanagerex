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
#include "fusedtransaction.h"

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

    mmTransDialog(wxWindow* parent
        , int64 account_id
        , Fused_Transaction::IdB fused_id
        , bool duplicate = false
        , int type = Model_Checking::TYPE_ID_WITHDRAWAL);

    bool Create(wxWindow* parent
        , wxWindowID id = wxID_ANY
        , const wxString& caption = _t("Transactions Dialog")
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxDefaultSize
        , long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX
        , const wxString& name = "Transactions Dialog"
    );

    void SetDialogTitle(const wxString& title);
    int64 GetAccountID() { return m_fused_data.ACCOUNTID; }
    int64 GetToAccountID() { return m_fused_data.TOACCOUNTID; }
    int64 GetTransactionID() { return m_fused_data.TRANSID; }

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
    void OnCalculator(wxCommandEvent& event);
    void SetTooltips();
    void SetCategoryForPayee(const Model_Payee::Data *payee = nullptr);
private:
    wxTextCtrl* textNumber_ = nullptr;
    mmTextCtrl* m_textAmount = nullptr;
    mmTextCtrl* toTextAmount_ = nullptr;
    wxTextCtrl* textNotes_ = nullptr;
    wxButton* bAttachments_ = nullptr;
    mmColorButton* bColours_ = nullptr;
    wxStaticText* account_label_ = nullptr;
    wxStaticText* categ_label_ = nullptr;
    mmComboBoxAccount* cbAccount_ = nullptr;
    wxStaticText* to_acc_label_ = nullptr;
    mmComboBoxAccount* cbToAccount_ = nullptr;
    wxStaticText* payee_label_ = nullptr;
    mmComboBoxPayee* cbPayee_ = nullptr;
    mmComboBoxCategory* cbCategory_ = nullptr;
    wxBitmapButton* bSplit_ = nullptr;
    wxBitmapButton* bAuto = nullptr;
    wxCheckBox* cAdvanced_ = nullptr;
    wxButton* m_button_cancel = nullptr;
    wxChoice* choiceStatus_ = nullptr;
    wxChoice* transaction_type_ = nullptr;
    mmDatePickerCtrl* dpc_ = nullptr;
    mmTagTextCtrl* tagTextCtrl_ = nullptr;
    wxButton* bCalc_ = nullptr;
    mmCalculatorPopup* calcPopup_ = nullptr;
    mmTextCtrl* calcTarget_ = nullptr;

    enum MODE { MODE_NEW = 0, MODE_DUP, MODE_EDIT };
    MODE m_mode = MODE_EDIT;
    bool m_transfer = false;
    bool m_advanced = false;

    int object_in_focus_ = wxID_ANY;
    int64 m_account_id = -1;
    wxString m_status;

    Fused_Transaction::Data m_fused_data;
    std::vector<Split> m_local_splits;

    std::vector<wxString> frequentNotes_;

    bool skip_date_init_ = false;
    bool skip_account_init_ = false;
    bool skip_amount_init_ = false;
    bool skip_payee_init_ = false;
    bool skip_status_init_ = false;
    bool skip_notes_init_ = false;
    bool skip_category_init_ = false;
    bool skip_tag_init_ = false;
    bool skip_tooltips_init_ = false;
    wxSize min_size_;

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
        mmID_CATEGORY_SPLIT,
        mmID_ACCOUNTNAME,
        ID_DIALOG_TRANS_BUTTON_FREQENTNOTES,
        ID_DIALOG_TRANS_CUSTOMFIELDS,
        ID_CUSTOMFIELD,
        ID_DIALOG_TRANS_TAGS
    };
};

#endif
