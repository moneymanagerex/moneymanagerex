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

#define SYMBOL_TRANSDIALOG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_TRANSDIALOG_IDNAME ID_DIALOG_TRANS
#define SYMBOL_TRANSDIALOG_SIZE wxSize(500, 300)
#define SYMBOL_TRANSDIALOG_POSITION wxDefaultPosition

#include "guiid.h"
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
    int getAccountID() {return newAccountID_;}
    int getToAccountID() { return transaction_->TOACCOUNTID; }
    int getTransactionID() { return transaction_->TRANSID; }

private:

    void CreateControls();
    void OnSplitChecked(wxCommandEvent& event);
    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnQuit(wxCloseEvent& event);
    void OnCategs(wxCommandEvent& event);
    void OnPayeeUpdated(wxCommandEvent& event);
    void OnAccountUpdated(wxCommandEvent& event);
    void OnDpcKillFocus(wxFocusEvent& event);
    void OnAutoTransNum(wxCommandEvent& event);
    void OnTransTypeChanged(wxCommandEvent& event);
    void OnSpin(wxSpinEvent&);
    void OnDateChanged(wxDateEvent& event);
    void dataToControls();
    void updateControlsForTransType();
    void changeFocus(wxChildFocusEvent& event);
    void onTextEntered(wxCommandEvent& event);
    void OnAdvanceChecked(wxCommandEvent& event);
    void updateControlsForTransType2(bool transfer = true);
    void activateSplitTransactionsDlg();
    void SetSplitState();
    bool validateData();

    wxWindow* parent_;
    mmTextCtrl *textNumber_;
    mmTextCtrl *textAmount_;
    mmTextCtrl *toTextAmount_;
    mmTextCtrl *textNotes_;
    mmTextCtrl* textCategory_;
    wxButton* bCategory_;
    wxComboBox* cbAccount_;
    wxComboBox* cbPayee_;
    wxCheckBox* cSplit_;
    wxCheckBox* cAdvanced_;
    wxButton* bAuto_;
    wxButton* itemButtonCancel_;
    wxChoice* choiceStatus_;
    wxChoice* transaction_type_;
    wxDatePickerCtrl* dpc_;
    wxSpinButton *spinCtrl_;
    wxStaticText* itemStaticTextWeek_;
    wxStaticText* account_label_;
    wxStaticText* payee_label_;

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
    wxString resetCategoryString();

    Model_Checking::Data * transaction_;
    Model_Splittransaction::Data_Set* m_splits;
    Model_Splittransaction::Data_Set m_local_splits;
    int accountID_; //The transaction account ID
    int newAccountID_;
    int referenceAccountID_; // used for transfer transactions
    int transaction_id_; //The transaction ID. NULL if new transaction

};

#endif
