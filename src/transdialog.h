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
#define SYMBOL_TRANSDIALOG_TITLE _("New/Edit Transaction")
#define SYMBOL_TRANSDIALOG_IDNAME ID_DIALOG_TRANS
#define SYMBOL_TRANSDIALOG_SIZE wxSize(500, 300)
#define SYMBOL_TRANSDIALOG_POSITION wxDefaultPosition

#include "guiid.h"
#include "mmcoredb.h"
#include <wx/spinbutt.h>

class wxDatePickerCtrl;

class mmTransDialog : public wxDialog
{
    DECLARE_DYNAMIC_CLASS( mmTransDialog )
    DECLARE_EVENT_TABLE()

public:
    mmTransDialog() {}
    virtual ~mmTransDialog();

    mmTransDialog( mmCoreDB* core,
        int accountID, mmBankTransaction* pBankTransaction,
        bool edit,
        wxWindow* parent,
        wxWindowID id = SYMBOL_TRANSDIALOG_IDNAME,
        const wxString& caption = SYMBOL_TRANSDIALOG_TITLE,
        const wxPoint& pos = SYMBOL_TRANSDIALOG_POSITION,
        const wxSize& size = SYMBOL_TRANSDIALOG_SIZE,
        long style = SYMBOL_TRANSDIALOG_STYLE );

    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_TRANSDIALOG_IDNAME,
        const wxString& caption = SYMBOL_TRANSDIALOG_TITLE,
        const wxPoint& pos = SYMBOL_TRANSDIALOG_POSITION,
        const wxSize& size = SYMBOL_TRANSDIALOG_SIZE,
        long style = SYMBOL_TRANSDIALOG_STYLE );

    void SetDialogToDuplicateTransaction();
    int getTransID() {return transID_;}
    int getAccountID() {return newAccountID_;}
    int getToAccountID() {return toID_;}

private:

    int transID_;
    mmCoreDB* core_;
    mmBankTransaction* pBankTransaction_;
    wxWindow* parent_;
    int accountID_;
    int newAccountID_;
    int referenceAccountID_;    // used for transfer transactions

    void CreateControls();
    void OnSplitChecked(wxCommandEvent& event);
    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnQuit(wxCloseEvent& event);
    void OnCategs(wxCommandEvent& event);
    void OnCategoryKey(wxKeyEvent& event);
    void OnPayee(wxCommandEvent& event);
    void OnPayeeUpdated(wxCommandEvent& event);
    void OnAccountUpdated(wxCommandEvent& event);
    void OnAutoTransNum(wxCommandEvent& event);
    void OnTransTypeChanged(wxCommandEvent& event);
    void OnSpin(wxSpinEvent&);
    void OnDateChanged(wxDateEvent& event);
    void dataToControls();
    void updateControlsForTransType();
    void changeFocus(wxChildFocusEvent& event);
    void onTextEntered(wxCommandEvent& event);
    void onChoiceTransChar(wxKeyEvent& event);
    void OnAdvanceChecked(wxCommandEvent& event);
    void SetTransferControls(bool transfer = true);
    void activateSplitTransactionsDlg();
    void SetSplitState();

    wxTextCtrl *textNumber_;
    wxTextCtrl *textAmount_;
    wxTextCtrl *toTextAmount_;
    wxTextCtrl *textNotes_;
    wxTextCtrl* textCategory_;
    wxButton* bCategory_;
    wxBitmapButton* bPayee_;
    wxComboBox* cbAccount_;
    wxComboBox* cbPayee_;
    wxCheckBox* cSplit_;
    wxCheckBox* cAdvanced_;
    wxButton* bAuto_;
    bool payeeUnknown_;
    bool categUpdated_;
    bool bBestChoice_;
    wxButton* itemButtonCancel_;

    wxString categoryName_;
    wxString categStrykes_;
    wxString subCategoryName_;
    wxString sTransaction_type_;

    wxChoice* choiceStatus_;
    wxChoice* transaction_type_;
    bool edit_;

    wxDatePickerCtrl* dpc_;
    wxSpinButton *spinCtrl_;
    wxStaticText* itemStaticTextWeek_;
    wxStaticText* account_label_;
    wxStaticText* payee_label_;
    int categID_;
    int subcategID_;
    int payeeID_;
    int toID_;
    double toTransAmount_;
    double transAmount_;
    bool advancedToTransAmountSet_;
    std::shared_ptr<mmSplitTransactionEntries> split_;

    // store the original currency rate for transaction editing
    double  edit_currency_rate;

    wxString payee_name_;
    wxString amountNormalTip_;
    wxString amountTransferTip_;
    wxString notesTip_;
    wxColour notesColour_;

    wxArrayString freqnotes_;
    int oject_in_focus_;

    wxString resetPayeeString(/*bool normal = true*/);
    wxString resetCategoryString();

    wxTimer* timer_;
    static const int INTERVAL = 1000;
    void ResetKeyStrikes(wxTimerEvent& /*event*/);
};

#endif
