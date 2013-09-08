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

#include "transdialog.h"
#include "constants.h"
#include "util.h"
#include "mmCurrencyFormatter.h"
#include "mmOption.h"
#include "paths.h"
#include "categdialog.h"
#include "splittransactionsdialog.h"
#include "validators.h"

#include <wx/valnum.h>

IMPLEMENT_DYNAMIC_CLASS( mmTransDialog, wxDialog )

BEGIN_EVENT_TABLE( mmTransDialog, wxDialog )
    EVT_BUTTON(wxID_OK, mmTransDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, mmTransDialog::OnCancel)
    EVT_CLOSE(mmTransDialog::OnQuit)
    EVT_BUTTON(ID_DIALOG_TRANS_BUTTONCATEGS, mmTransDialog::OnCategs)
    EVT_CHOICE(ID_DIALOG_TRANS_TYPE, mmTransDialog::OnTransTypeChanged)
    EVT_CHECKBOX(ID_DIALOG_TRANS_ADVANCED_CHECKBOX, mmTransDialog::OnAdvanceChecked)
    EVT_CHECKBOX(ID_DIALOG_TRANS_SPLITCHECKBOX, mmTransDialog::OnSplitChecked)
    EVT_CHILD_FOCUS(mmTransDialog::changeFocus)
    EVT_SPIN(wxID_ANY,mmTransDialog::OnSpin)
    EVT_DATE_CHANGED(ID_DIALOG_TRANS_BUTTONDATE, mmTransDialog::OnDateChanged)
    EVT_TIMER(wxID_ANY, mmTransDialog::ResetKeyStrikes)
END_EVENT_TABLE()

mmTransDialog::mmTransDialog(
    mmCoreDB* core,
    int accountID, mmBankTransaction* pBankTransaction, bool edit,
    wxWindow* parent, wxWindowID id,
    const wxString& caption, const wxPoint& pos,
    const wxSize& size, long style
) :
    core_(core),
    parent_(parent),
    pBankTransaction_(pBankTransaction),
    accountID_(accountID),
    referenceAccountID_(accountID),
    categUpdated_(false),
    edit_(edit),
    advancedToTransAmountSet_(false),
    edit_currency_rate(1.0),
    categID_(-1),
    subcategID_(-1),
    payeeID_(-1),
    toID_(-1),
    toTransAmount_(-1),
    transAmount_(-1),
    bBestChoice_(true)

{
    Create(parent, id, caption, pos, size, style);
}

mmTransDialog::~mmTransDialog()
{
    timer_->Stop();
}

bool mmTransDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption,
                           const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());

    mmSplitTransactionEntries* split(new mmSplitTransactionEntries());
    split_ = split;

    dataToControls();

    Centre();
    Fit();
    timer_ = new wxTimer(this, wxID_ANY);

    return TRUE;
}

void mmTransDialog::dataToControls()
{
    // Use last date used as per user option.
    wxDateTime trx_date = wxDateTime::Now();
    if (mmIniOptions::instance().transDateDefault_ != 0)
        trx_date = core_->bTransactionList_.getLastDate(accountID_);

    dpc_->SetValue(edit_ ? pBankTransaction_->date_: trx_date);
    //process date change event for set weekday name
    wxDateEvent dateEvent(dpc_, trx_date, wxEVT_DATE_CHANGED);
    GetEventHandler()->ProcessEvent(dateEvent);
    dpc_->SetFocus();

    wxString dispAmount;
    if (edit_)
    {
        wxString statusString = pBankTransaction_->status_;
        if (statusString == "") statusString = "N";
        choiceStatus_->SetSelection(wxString("NRVFD").Find(statusString));

        sTransaction_type_ = pBankTransaction_->transType_;

        categID_ = pBankTransaction_->categID_;
        categoryName_ = core_->categoryList_.GetCategoryName(categID_);
        subcategID_ = pBankTransaction_->subcategID_;
        subCategoryName_ = core_->categoryList_.GetSubCategoryName(categID_, subcategID_);

        accountID_ = pBankTransaction_->accountID_;
        toID_ = pBankTransaction_->toAccountID_;

        payeeID_ = pBankTransaction_->payeeID_;
        payee_name_ = core_->payeeList_.GetPayeeName(pBankTransaction_->payeeID_);

        textNotes_->SetValue(pBankTransaction_->notes_);
        textNumber_->SetValue(pBankTransaction_->transNum_);

        transAmount_ = pBankTransaction_->amt_;
        toTransAmount_ = pBankTransaction_->toAmt_;
        advancedToTransAmountSet_ = (transAmount_ != toTransAmount_);

        dispAmount = CurrencyFormatter::float2String(transAmount_);
        textAmount_->SetValue(dispAmount);
        dispAmount = CurrencyFormatter::float2String(toTransAmount_);
        toTextAmount_->SetValue(dispAmount);
    }
    else
    {
        choiceStatus_->SetSelection(mmIniOptions::instance().transStatusReconciled_);
        sTransaction_type_ = TRANS_TYPE_WITHDRAWAL_STR;

        notesColour_ = textNotes_->GetForegroundColour();
        textNotes_->SetForegroundColour(wxColour("GREY"));
        textNotes_->SetValue(notesTip_);
        int font_size = textNotes_->GetFont().GetPointSize();
        textNotes_->SetFont(wxFont(font_size, wxSWISS, wxNORMAL, wxNORMAL, FALSE, ""));
    }
    // backup the original currency rate first
    if (transAmount_ > 0.0)
        edit_currency_rate = toTransAmount_ / transAmount_;

    for(const auto& i : TRANSACTION_TYPE)
        transaction_type_->Append(wxGetTranslation(i), new wxStringClientData(i));
    transaction_type_->SetStringSelection(wxGetTranslation(sTransaction_type_));

    wxString categString = _("Select Category");

    updateControlsForTransType();
    if (edit_)
    {
        *split_ = *core_->bTransactionList_.getBankTransactionPtr(
            pBankTransaction_->transactionID())->splitEntries_;
    }
    else
    {
        if (mmIniOptions::instance().transCategorySelectionNone_> 0)
            if (categID_ > -1) categString = core_->categoryList_.GetFullCategoryString(categID_, subcategID_);

    }
    SetSplitState();

}

void mmTransDialog::OnTransTypeChanged(wxCommandEvent& /*event*/)
{
    wxString sType = sTransaction_type_;
    sTransaction_type_ = TRANS_TYPE_WITHDRAWAL_STR;
    wxStringClientData* type_obj = (wxStringClientData *)transaction_type_->GetClientObject(
        transaction_type_->GetSelection());
    if (type_obj) sTransaction_type_ = type_obj->GetData();
    if (sType != TRANS_TYPE_TRANSFER_STR && sTransaction_type_ == TRANS_TYPE_TRANSFER_STR)
    {
        payee_name_ = resetPayeeString();
        categID_ = -1;
    }

    updateControlsForTransType();
}

void mmTransDialog::updateControlsForTransType()
{
    bool transfer = sTransaction_type_ == TRANS_TYPE_TRANSFER_STR;
    if (!edit_)
    {
        if (mmIniOptions::instance().transPayeeSelectionNone_ > 0)
        {
            payeeID_ = core_->bTransactionList_.getLastUsedPayeeID(accountID_
                , sTransaction_type_, categID_, subcategID_);
            payee_name_ = core_->payeeList_.GetPayeeName(payeeID_);
        }

        wxString categString = resetCategoryString();
        if (mmIniOptions::instance().transCategorySelectionNone_ != 0)
        {
            categID_ = core_->bTransactionList_.getLastUsedCategoryID(accountID_
                , payeeID_, sTransaction_type_, subcategID_);
            categString = core_->categoryList_.GetFullCategoryString(categID_, subcategID_);
            categoryName_    = core_->categoryList_.GetCategoryName(categID_);
            subCategoryName_ = core_->categoryList_.GetSubCategoryName(categID_, subcategID_);
        }
        bCategory_->SetLabel(categString);
    }

    SetTransferControls(transfer);
}

void mmTransDialog::SetTransferControls(bool transfer)
{
    textAmount_->UnsetToolTip();
    toTextAmount_->UnsetToolTip();
    cbAccount_->UnsetToolTip();

    cbPayee_->SetEvtHandlerEnabled(false);

    cAdvanced_->SetValue(advancedToTransAmountSet_);
    cAdvanced_->Enable(transfer);

    wxString dataStr = payee_name_;
    cbPayee_->Clear();
    payee_name_.Clear();
    payeeID_ = -1;
    wxSortedArrayString data;
    int type_num = transaction_type_->GetSelection();

    newAccountID_ = accountID_;
    cbAccount_->Clear();
    data = core_->accountList_.getAccountsName();
    for (const auto &entry : data)
        cbAccount_ ->Append(entry);

    cbAccount_->SetStringSelection(core_->accountList_.GetAccountName(accountID_));
    cbAccount_->AutoComplete(data);

    if (transfer)
    {
        textAmount_->SetToolTip(amountTransferTip_);
        toTextAmount_->SetToolTip(_("Specify the transfer amount in the To Account"));
        if (cSplit_->IsChecked())
        {
            cSplit_->SetValue(false);
            split_->entries_.clear();
        }

        toTextAmount_->Enable(cAdvanced_->GetValue());

        if (toID_ > 0) dataStr = core_->accountList_.GetAccountName(toID_);
        data = core_->accountList_.getAccountsName();
        payee_label_->SetLabel(_("To"));
        cbPayee_->SetToolTip(_("Specify which account the transfer is going to"));
        account_label_->SetLabel(_("From"));
        cbAccount_->SetToolTip(_("Specify which account the transfer is going from"));
        cbAccount_->Enable(true);
    }
    else
    {
        textAmount_->SetToolTip(amountNormalTip_);

        if (type_num == DEF_WITHDRAWAL)
            cbPayee_->SetToolTip(_("Specify to whom the transaction is going to"));
        else
            cbPayee_->SetToolTip(_("Specify where the transaction is coming from"));

        payee_label_->SetLabel((type_num == DEF_WITHDRAWAL) ? _("Payee") : _("From"));

        cbAccount_->SetToolTip(_("Specify account for the transaction"));
        account_label_->SetLabel(_("Account"));
        cbAccount_->Enable(core_->accountList_.accounts_.size() > 1);

        data = core_->payeeList_.FilterPayees("");
        toTextAmount_->Enable(false);
        toTextAmount_->SetValue("");
        advancedToTransAmountSet_ = false;
        cAdvanced_->Enable(false);
        payeeID_ = core_->payeeList_.GetPayeeId(dataStr);
        payee_name_ = core_->payeeList_.GetPayeeName(payeeID_);
        dataStr = payee_name_;
    }

    for (const auto & entry : data)
        cbPayee_ ->Append(entry);
    cbPayee_->AutoComplete(data);

    if (!cbPayee_ -> SetStringSelection(dataStr))
        cbPayee_ -> SetValue(dataStr);
    SetSplitState();
    cbPayee_ -> SetEvtHandlerEnabled(true);
}

void mmTransDialog::CreateControls()
{
    int border = 5;
    wxSizerFlags flags, flagsExpand;
    flags.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, border);
    flagsExpand.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxEXPAND).Border(wxALL, border).Proportion(1);

    wxBoxSizer* box_sizer1 = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* box_sizer2 = new wxBoxSizer(wxVERTICAL);
    box_sizer1->Add(box_sizer2, flagsExpand);

    wxStaticBox* static_box = new wxStaticBox(this, wxID_ANY, _("Transaction Details"));
    wxStaticBoxSizer* box_sizer = new wxStaticBoxSizer(static_box, wxVERTICAL);
    box_sizer2->Add(box_sizer, flagsExpand);

    wxFlexGridSizer* flex_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    box_sizer->Add(flex_sizer, flags);

    // Date --------------------------------------------
    long date_style = wxDP_DROPDOWN|wxDP_SHOWCENTURY;

    dpc_ = new wxDatePickerCtrl( this, ID_DIALOG_TRANS_BUTTONDATE, wxDateTime::Now(),
        wxDefaultPosition, wxSize(110, -1), date_style);

    //Text field for day of the week
    itemStaticTextWeek_ = new wxStaticText(this, wxID_STATIC, "");
    // Display the day of the week

    spinCtrl_ = new wxSpinButton(this, wxID_STATIC,
        wxDefaultPosition, wxSize(18, wxSize(dpc_->GetSize()).GetHeight()),
        wxSP_VERTICAL|wxSP_ARROW_KEYS|wxSP_WRAP);
    spinCtrl_->SetRange (-32768, 32768);

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Date")), flags);
    wxBoxSizer* date_sizer = new wxBoxSizer(wxHORIZONTAL);
    flex_sizer->Add(date_sizer);
    date_sizer->Add(dpc_, flags);
    date_sizer->Add(spinCtrl_, flags);
    date_sizer->Add(itemStaticTextWeek_, flags);

    // Status --------------------------------------------
    choiceStatus_ = new wxChoice(this, ID_DIALOG_TRANS_STATUS,
        wxDefaultPosition, wxSize(110, -1));

    for(const auto& i : TRANSACTION_STATUS)
        choiceStatus_->Append(wxGetTranslation(i), new wxStringClientData(i));

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Status")), flags);
    flex_sizer->Add(choiceStatus_, flags);

    // Type --------------------------------------------
    transaction_type_ = new wxChoice(this, ID_DIALOG_TRANS_TYPE,
        wxDefaultPosition, wxSize(110, -1));

    cAdvanced_ = new wxCheckBox(this,
        ID_DIALOG_TRANS_ADVANCED_CHECKBOX, _("Advanced"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );

    wxBoxSizer* typeSizer = new wxBoxSizer(wxHORIZONTAL);

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Type")), flags);
    flex_sizer->Add(typeSizer);
    typeSizer->Add(transaction_type_, flags);
    typeSizer->Add(cAdvanced_, flags);

    // Amount Fields --------------------------------------------
    textAmount_ = new wxTextCtrl( this, ID_DIALOG_TRANS_TEXTAMOUNT, "",
        wxDefaultPosition, wxSize(110, -1),
        wxALIGN_RIGHT|wxTE_PROCESS_ENTER, mmCalcValidator());
    textAmount_->Connect(ID_DIALOG_TRANS_TEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER,
        wxCommandEventHandler(mmTransDialog::onTextEntered), NULL, this);

    toTextAmount_ = new wxTextCtrl( this, ID_DIALOG_TRANS_TOTEXTAMOUNT, "",
        wxDefaultPosition, wxSize(110, -1),
        wxALIGN_RIGHT|wxTE_PROCESS_ENTER, mmCalcValidator());
    toTextAmount_->Connect(ID_DIALOG_TRANS_TOTEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER,
        wxCommandEventHandler(mmTransDialog::onTextEntered), NULL, this);

    wxBoxSizer* amountSizer = new wxBoxSizer(wxHORIZONTAL);
    amountSizer->Add(textAmount_, flags);
    amountSizer->Add(toTextAmount_, flags);

    flex_sizer->Add(new wxStaticText( this, wxID_STATIC, _("Amount")), flags);
    flex_sizer->Add(amountSizer);

    // Account ---------------------------------------------
    cbAccount_ = new wxComboBox(this, wxID_ANY, "",
        wxDefaultPosition, wxSize(230, -1));
    cbAccount_->Connect(wxID_ANY, wxEVT_COMMAND_TEXT_UPDATED,
        wxCommandEventHandler(mmTransDialog::OnAccountUpdated), NULL, this);

    account_label_ = new wxStaticText(this, wxID_STATIC, _("Account"));
    flex_sizer->Add(account_label_, flags);
    flex_sizer->Add(cbAccount_, flags);

    // Payee ---------------------------------------------
    payee_label_ = new wxStaticText(this, wxID_STATIC, _("Payee"));

    /*Note: If you want to use EVT_TEXT_ENTER(id,func) to receive wxEVT_COMMAND_TEXT_ENTER events,
      you have to add the wxTE_PROCESS_ENTER window style flag.
      If you create a wxComboBox with the flag wxTE_PROCESS_ENTER, the tab key won't jump to the next control anymore.*/
    cbPayee_ = new wxComboBox(this, ID_DIALOG_TRANS_PAYEECOMBO, "",
        wxDefaultPosition, wxSize(230, -1));

    cbPayee_->Connect(ID_DIALOG_TRANS_PAYEECOMBO, wxEVT_COMMAND_TEXT_UPDATED,
        wxCommandEventHandler(mmTransDialog::OnPayeeUpdated), NULL, this);

    cbPayee_ -> SetEvtHandlerEnabled(!edit_);

    flex_sizer->Add(payee_label_, flags);
    flex_sizer->Add(cbPayee_, flags);

    // Split Category -------------------------------------------
    cSplit_ = new wxCheckBox(this, ID_DIALOG_TRANS_SPLITCHECKBOX,
        _("Split"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    cSplit_->SetValue(FALSE);

    flex_sizer->AddSpacer(20);  // Fill empty space.
    flex_sizer->Add(cSplit_, flags);

    // Category -------------------------------------------------
    bCategory_ = new wxButton(this, ID_DIALOG_TRANS_BUTTONCATEGS, ""
        , wxDefaultPosition, wxSize(230, -1));

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Category")), flags);
    flex_sizer->Add(bCategory_, flags);

    bCategory_->Connect(ID_DIALOG_TRANS_BUTTONCATEGS, wxEVT_KEY_DOWN
        , wxCharEventHandler(mmTransDialog::OnCategoryKey), NULL, this);

    // Number  ---------------------------------------------
    textNumber_ = new wxTextCtrl(this
        , ID_DIALOG_TRANS_TEXTNUMBER, "", wxDefaultPosition
        , wxDefaultSize, wxTE_PROCESS_ENTER);
    textNumber_->Connect(ID_DIALOG_TRANS_TEXTNUMBER, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmTransDialog::onTextEntered), NULL, this);

    bAuto_ = new wxButton(this
        , ID_DIALOG_TRANS_BUTTONTRANSNUM, "...", wxDefaultPosition
        , wxSize(cbPayee_->GetSize().GetY(), cbPayee_->GetSize().GetY()));
    bAuto_ -> Connect(ID_DIALOG_TRANS_BUTTONTRANSNUM,
        wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(mmTransDialog::OnAutoTransNum), NULL, this);

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Number")), flags);
    wxBoxSizer* number_sizer = new wxBoxSizer(wxHORIZONTAL);
    flex_sizer->Add(number_sizer, flagsExpand.Border(wxALL, 0));
    number_sizer->Add(textNumber_, flagsExpand.Border(wxALL, border));
    number_sizer->Add(bAuto_, flags);

    // Notes  ---------------------------------------------
/*
    wxNotebook* trx_notebook = new wxNotebook(this,
        wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE );
    wxPanel* notes_tab = new wxPanel(trx_notebook, wxID_ANY);
    trx_notebook->AddPage(notes_tab, _("Notes"));
    wxBoxSizer *notes_sizer = new wxBoxSizer(wxVERTICAL);
    notes_tab->SetSizer(notes_sizer);

    textNotes_ = new wxTextCtrl(notes_tab, ID_DIALOG_TRANS_TEXTNOTES, "",
        wxDefaultPosition, wxSize(290,120), wxTE_MULTILINE);

    box_sizer->Add(trx_notebook);
    notes_sizer->Add(textNotes_, flags);
*/

    notesTip_ = _("Notes");
    textNotes_ = new wxTextCtrl(this, ID_DIALOG_TRANS_TEXTNOTES, "",
        wxDefaultPosition, wxSize(-1,80), wxTE_MULTILINE);

    box_sizer->Add(textNotes_, flagsExpand.Border(wxLEFT|wxRIGHT|wxBOTTOM, 10));

    amountNormalTip_   = _("Specify the amount for this transaction");
    amountTransferTip_ = _("Specify the amount to be transfered");
    if (true) //TODO: Add parameter
    {
        dpc_->SetToolTip(_("Specify the date of the transaction"));
        spinCtrl_->SetToolTip(_("Retard or advance the date of the transaction"));
        choiceStatus_->SetToolTip(_("Specify the status for the transaction"));
        transaction_type_->SetToolTip(_("Specify the type of transactions to be created."));
        cAdvanced_->SetToolTip(_("Allows the setting of different amounts in the FROM and TO accounts."));
        textAmount_->SetToolTip(amountNormalTip_);
        cSplit_->SetToolTip(_("Use split Categories"));
        bCategory_->SetToolTip(_("Specify the category for this transaction"));
        textNumber_->SetToolTip(_("Specify any associated check number or transaction number"));
        bAuto_->SetToolTip(_("Populate Transaction #"));
        textNotes_->SetToolTip(_("Specify any text notes you want to add to this transaction."));
    }

    /**********************************************************************************************
     Button Panel with OK and Cancel Buttons
    ***********************************************************************************************/
    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    box_sizer1->Add(buttons_panel, flags.Center().Border(wxALL, 0));

    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    wxButton* itemButtonOK = new wxButton( buttons_panel, wxID_OK, _("&OK"));
    itemButtonCancel_ = new wxButton( buttons_panel, wxID_CANCEL, _("&Cancel"));

    buttons_sizer->Add(itemButtonOK, flags.Border(wxBOTTOM|wxRIGHT, 10));
    buttons_sizer->Add(itemButtonCancel_, flags);

    buttons_sizer->Realize();
    Center();
    this->SetSizer(box_sizer1);
}

void mmTransDialog::OnAccountUpdated(wxCommandEvent& /*event*/)
{
    wxString sAccountName = cbAccount_->GetValue();
    newAccountID_ = core_->accountList_.GetAccountId(sAccountName);
}

void mmTransDialog::OnPayeeUpdated(wxCommandEvent& event)
{
    payee_name_ = cbPayee_->GetValue();

    bool transfer_transaction = transaction_type_->GetStringSelection() == wxGetTranslation(TRANS_TYPE_TRANSFER_STR);
    if (!transfer_transaction)
    {
        payeeID_ = core_->payeeList_.GetPayeeId(payee_name_);

        // Only for new transactions: if user want to autofill last category used for payee.
        // If this is a Split Transaction, ignore displaying last category for payee
        if (payeeID_ != -1 && mmIniOptions::instance().transCategorySelectionNone_ == 1 && !edit_ && !categUpdated_ && split_->numEntries() == 0)
        {
            mmPayee* pPayee = core_->payeeList_.GetPayeeSharedPtr(payeeID_);
            // if payee has memory of last category used then display last category for payee
            if (pPayee->categoryId_ != -1)
            {
                categID_ = pPayee->categoryId_;
                subcategID_ = pPayee->subcategoryId_;
                bCategory_->SetLabel(core_->categoryList_.GetFullCategoryString(categID_, subcategID_));
                categoryName_ = core_->categoryList_.GetCategoryName(categID_);
                subCategoryName_ = core_->categoryList_.GetSubCategoryName(categID_, subcategID_);
            }
        }
    }
    else
    {
        toID_ = core_->accountList_.GetAccountId(payee_name_);
    }

    event.Skip();
}

void mmTransDialog::OnAutoTransNum(wxCommandEvent& /*event*/)
{
    wxString current_number = textNumber_->GetValue();
    wxDateTime transaction_date = dpc_->GetValue();
    wxArrayString number_strings = core_->bTransactionList_.getTransactionNumber(accountID_, transaction_date);
    int i = number_strings.GetCount();
    int s = 0;
    if (number_strings.Index(current_number) != wxNOT_FOUND)
         s = number_strings.Index(current_number);
    i = (s+1)%i;

    textNumber_->SetValue( number_strings[i] );
}

void mmTransDialog::OnSpin(wxSpinEvent& event)
{
    wxDateTime date = dpc_->GetValue();
    int value = event.GetPosition();

    date = date.Add(wxDateSpan::Days(value));
    dpc_->SetValue (date);
    spinCtrl_->SetValue(0);

    //process date change event for set weekday name
    wxDateEvent dateEvent(dpc_, date, wxEVT_DATE_CHANGED);
    GetEventHandler()->ProcessEvent(dateEvent);

    event.Skip();
}

void mmTransDialog::OnDateChanged(wxDateEvent& event)
{
    //get weekday name
    wxDateTime date = dpc_->GetValue();
    if (event.GetDate().IsValid())
        itemStaticTextWeek_->SetLabel(wxGetTranslation(date.GetWeekDayName(date.GetWeekDay())));
    event.Skip();
}

void mmTransDialog::OnAdvanceChecked(wxCommandEvent& /*event*/)
{
    advancedToTransAmountSet_ = cAdvanced_->IsChecked();

    wxString amountStr = textAmount_->GetValue().Trim();
    if (advancedToTransAmountSet_)
    {
        if (amountStr.IsEmpty())
        {
            amountStr = "1";
            transAmount_ = 1;
            toTransAmount_ = transAmount_;
            textAmount_->SetValue(amountStr);
        }

        CurrencyFormatter::formatCurrencyToDouble(amountStr, transAmount_);

        if (toID_ > 0) {
            double rateFrom = core_->accountList_.getAccountBaseCurrencyConvRate(accountID_);
            double rateTo = core_->accountList_.getAccountBaseCurrencyConvRate(toID_);
            double convToBaseFrom = rateFrom * transAmount_;
            toTransAmount_ = convToBaseFrom / rateTo;
        }
        else
        {
            toTextAmount_->SetValue("");
            toTransAmount_ = transAmount_;
        }
    }
    else
    {
        toTransAmount_ = transAmount_;
    }

    amountStr = CurrencyFormatter::float2String(toTransAmount_);
    toTextAmount_->SetValue(amountStr);

    SetTransferControls();
}

void mmTransDialog::OnCategoryKey(wxKeyEvent& event)
{
    bool skip = false;
    if ( !event.HasModifiers() )
    {
        //TODO: Get national (non Latin) keys
        wxString key = wxString() << event.GetUnicodeKey();
        categStrykes_ << key;
        if (key <= " ") skip = true;

        if (!timer_->IsRunning ())
            timer_->Start(INTERVAL, true);
        core_->categoryList_.GetCategoryLikeString(categStrykes_, categID_, subcategID_);

        //wxLogDebug(key + " | " + categStrykes_ + " | " + core_->categoryList_.GetFullCategoryString(categID_, subcategID_));
        bCategory_->SetLabel(core_->categoryList_.GetFullCategoryString(categID_, subcategID_));
        categoryName_ = core_->categoryList_.GetCategoryName(categID_);
        subCategoryName_ = core_->categoryList_.GetSubCategoryName(categID_, subcategID_);
    }
    else
        skip = true;

    if (skip) event.Skip();
}

void mmTransDialog::ResetKeyStrikes(wxTimerEvent& /*event*/)
{
    categStrykes_.clear();
}

void mmTransDialog::OnCategs(wxCommandEvent& /*event*/)
{
    if (cSplit_->IsChecked())
    {
        activateSplitTransactionsDlg();
    }
    else
    {
        mmCategDialog dlg(core_, parent_, true, false);
        dlg.setTreeSelection(categoryName_, subCategoryName_);
        if ( dlg.ShowModal() == wxID_OK )
        {
            categID_ = dlg.getCategId();
            subcategID_ = dlg.getSubCategId();
            categUpdated_ = true;

            categoryName_ = core_->categoryList_.GetCategoryName(categID_);
            subCategoryName_ = core_->categoryList_.GetSubCategoryName(categID_, subcategID_);
        }
    }
    SetSplitState();
}

wxString mmTransDialog::resetPayeeString(/*bool normal*/) //normal is deposits or withdrawls
{
    wxString payeeStr = "";

    payeeID_ = -1;
    wxArrayString filtd = core_->payeeList_.FilterPayees("");
    if (filtd.Count() == 1)
    {
        //only one payee present. Choose it
        payeeStr = filtd[0];
        payeeID_ = core_->payeeList_.GetPayeeId(payeeStr);
    }

    return payeeStr;
}

wxString mmTransDialog::resetCategoryString()
{
    categID_ = -1;
    subcategID_ = -1;

    return _("Select Category");
}

void mmTransDialog::OnOk(wxCommandEvent& /*event*/)
{
    wxString date1 = dpc_->GetValue().FormatISODate();

    wxString sAccountName = cbAccount_->GetValue();
    newAccountID_ = core_->accountList_.GetAccountId(sAccountName);

    if (newAccountID_ < 1)
    {
        mmShowErrorMessageInvalid(this, _("Account"));
        return;
    }

    bool bTransfer = (sTransaction_type_ == TRANS_TYPE_TRANSFER_STR);
    advancedToTransAmountSet_ = cAdvanced_->IsChecked();

    if (cSplit_->IsChecked())
    {
        transAmount_ = split_->getTotalSplits();
        if (transAmount_ < 0.0)
        {
            if (bTransfer) {
                if (transAmount_ < 0)
                    transAmount_ = - transAmount_;
            } else {
                mmShowErrorMessageInvalid(parent_, _("Amount"));
                return;
            }
        }

        if (split_->numEntries() == 0)
        {
            mmShowErrorMessageInvalid(this, _("Category"));
            return;
        }
    }
    else
    {
        wxString amountStr = textAmount_->GetValue().Trim();
        if (! CurrencyFormatter::formatCurrencyToDouble(amountStr, transAmount_) || (transAmount_ < 0.0))
        {
            textAmount_->SetBackgroundColour("RED");
            mmShowErrorMessageInvalid(parent_, _("Amount"));
            textAmount_->SetBackgroundColour(wxNullColour);
            textAmount_->SetFocus();
            return;
        }

        if (categID_ < 1 || !core_->categoryList_.CategoryExists(categoryName_))
        {
            mmShowErrorMessageInvalid(this, _("Category"));
            return;
        }
    }

    toTransAmount_ = transAmount_;
    if (bTransfer)
    {
        if (advancedToTransAmountSet_)
        {
            wxString amountStr = toTextAmount_->GetValue().Trim();
            if (amountStr.IsEmpty()
                || ! CurrencyFormatter::formatCurrencyToDouble(amountStr, toTransAmount_)
                || (toTransAmount_ < 0.0)
            )
            {
                toTextAmount_->SetBackgroundColour("RED");
                mmShowErrorMessageInvalid(parent_, _("Advanced Amount"));
                toTextAmount_->SetBackgroundColour(wxNullColour);
                toTextAmount_->SetFocus();
                return;
            }
        }
    }

    if (!bTransfer)
    {
        wxString payee_name = cbPayee_->GetValue();
        if (payee_name.IsEmpty())
        {
            mmShowErrorMessageInvalid(this, _("Payee"));
            return;
        }

        payeeID_ = core_->payeeList_.GetPayeeId(payee_name);

        if (payeeID_ < 0)
        {
            wxMessageDialog msgDlg( this
                , wxString::Format(_("Do you want to add new payee: \n%s?"), payee_name)
                , _("Confirm to add new payee")
                , wxYES_NO | wxYES_DEFAULT | wxICON_WARNING);
            if (msgDlg.ShowModal() == wxID_YES)
            {
                payeeID_ = core_->payeeList_.AddPayee(payee_name);
            }
            else
                return;
        }
    }

    int toAccountID = -1;

    if (bTransfer)
    {
        if (toID_ < 1 || toID_ == newAccountID_)
        {
            mmShowErrorMessageInvalid(this, _("To Account"));
            cbPayee_->SetFocus();
            return;
        }

        toAccountID = toID_;
        payeeID_ = -1;
    }
    else
    {
        // save the category used for this payee to allow automatic category fill at user request.
        mmPayee* pPayee = core_->payeeList_.GetPayeeSharedPtr(payeeID_);
        pPayee->categoryId_ = categID_;
        pPayee->subcategoryId_ = subcategID_;
        core_->payeeList_.UpdatePayee(payeeID_, "");
    }

    wxString transNum = textNumber_->GetValue();

    textNotes_->SetFocus();
    wxString notes = textNotes_->GetValue();

    wxString status;
    wxStringClientData* status_obj = (wxStringClientData *)choiceStatus_->GetClientObject(choiceStatus_->GetSelection());
    if (status_obj) status = status_obj->GetData().Left(1);
    status.Replace("N", "");

    mmBankTransaction* pTransaction;
    if (!edit_)
    {
        mmBankTransaction* pTemp(new mmBankTransaction(core_));
        pTransaction = pTemp;
    }
    else
    {
        pTransaction = core_->bTransactionList_.getBankTransactionPtr(
            pBankTransaction_->transactionID());
    }

    mmCurrency* pCurrencyPtr = core_->accountList_.getCurrencySharedPtr(newAccountID_);
    wxASSERT(pCurrencyPtr);

    pTransaction->accountID_ = newAccountID_;
    pTransaction->toAccountID_ = toAccountID;
    pTransaction->payeeID_ = payeeID_;
    pTransaction->payeeStr_ = core_->payeeList_.GetPayeeName(payeeID_);
    pTransaction->transType_ = sTransaction_type_;
    pTransaction->amt_ = transAmount_;
    pTransaction->status_ = status;
    pTransaction->transNum_ = transNum;
    pTransaction->notes_ = notes;
    pTransaction->categID_ = categID_;
    pTransaction->subcategID_ = subcategID_;
    pTransaction->date_ = dpc_->GetValue();
    pTransaction->toAmt_ = toTransAmount_;

    *pTransaction->splitEntries_ = *split_;

    if (!edit_)
    {
        transID_ = core_->bTransactionList_.addTransaction(pTransaction);
    }
    else
    {
        core_->bTransactionList_.UpdateTransaction(pTransaction);
        transID_ = pTransaction->transactionID();
    }

    EndModal(wxID_OK);
}

void mmTransDialog::SetSplitState()
{
    int entries = split_->numEntries();
    wxString categString;
    if (split_->numEntries() > 0)
        categString = _("Split Category");
    else
    {
        if (categID_ < 0)
            categString = edit_ ? pBankTransaction_->fullCatStr_ : wxString(_("Select Category"));
        else
            categString = core_->categoryList_.GetFullCategoryString(categID_, subcategID_);
    }

    bCategory_->SetLabel(categString);
    cSplit_->SetValue(entries > 0);
    cSplit_->Enable(sTransaction_type_ != TRANS_TYPE_TRANSFER_STR);

    textAmount_->Enable(entries < 1);
}

void mmTransDialog::OnSplitChecked(wxCommandEvent& /*event*/)
{
    /* Reset Category */
    //split_ = std::shared_ptr<mmSplitTransactionEntries>(new mmSplitTransactionEntries());
    if (cSplit_->IsChecked())
    {
        activateSplitTransactionsDlg();
    }
    else
    {
        if (split_->numEntries() != 1)
        {
            transAmount_ = 0;
        }
        else
        {
            categID_    = split_->entries_[0]->categID_;
            subcategID_ = split_->entries_[0]->subCategID_;
            transAmount_  = split_->entries_[0]->splitAmount_;

            if (transAmount_ < 0 )
            {
                transAmount_ = - transAmount_;
                transaction_type_->SetStringSelection(wxGetTranslation(TRANS_TYPE_WITHDRAWAL_STR));
            }
            split_->removeSplitByIndex(0);
        }
        wxString dispAmount = CurrencyFormatter::float2String(transAmount_);
        textAmount_->SetValue(dispAmount);
    }
    SetSplitState();
}

//----------------------------------------------------------------------------

void mmTransDialog::changeFocus(wxChildFocusEvent& event)
{
    wxWindow *w = event.GetWindow();
    if ( w )
        object_in_focus_ = w->GetId();

    if (!edit_ && textNotes_->GetValue() == notesTip_ && object_in_focus_ == ID_DIALOG_TRANS_TEXTNOTES)
    {
        textNotes_->SetValue("");
        textNotes_->SetForegroundColour(notesColour_);
    }
    event.Skip();
}

void mmTransDialog::OnCancel(wxCommandEvent& /*event*/)
{
    if (object_in_focus_ == bCategory_->GetId()) return;
    if (object_in_focus_ == textNotes_->GetId()) return;

    if (object_in_focus_ == cbPayee_->GetId())
    {
        if (!cbPayee_->GetValue().IsEmpty()) {
            cbPayee_->SetValue("");
            return;
        }
        else {
            itemButtonCancel_->SetFocus();
            return;
        }
    }

    if (object_in_focus_ == textAmount_->GetId())
    {
        if (!textAmount_->IsEmpty()) {
            textAmount_->SetValue("");
            return;
        }
        else {
            itemButtonCancel_->SetFocus();
            return;
        }
    }

    if ((int)object_in_focus_ == (int)toTextAmount_->GetId())
    {
        if (!toTextAmount_->IsEmpty()) {
            toTextAmount_->SetValue("");
            return;
        }
        else {
            itemButtonCancel_->SetFocus();
            return;
        }
    }

    if ((int)object_in_focus_ == (int)textNumber_->GetId())
    {
        if (!textNumber_->IsEmpty())
        {
            textNumber_->SetValue("");
            return;
        }
        else
        {
            itemButtonCancel_->SetFocus();
            return;
        }
    }

    EndModal(wxID_CANCEL);
}

void mmTransDialog::OnQuit(wxCloseEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void mmTransDialog::onTextEntered(wxCommandEvent& event)
{
    wxString sAmount = "";

    if (object_in_focus_ == textAmount_->GetId())
    {
        if (mmCalculator(textAmount_->GetValue(), sAmount))
            textAmount_->SetValue(sAmount);
        textAmount_->SetInsertionPoint(textAmount_->GetValue().Len());
    }
    else if (object_in_focus_ == toTextAmount_->GetId())
    {
        if (mmCalculator(toTextAmount_->GetValue(), sAmount))
            toTextAmount_->SetValue(sAmount);
        toTextAmount_->SetInsertionPoint(toTextAmount_->GetValue().Len());
    }
    else if (object_in_focus_ == textNumber_->GetId())
    {
        textNotes_->SetFocus();
    }

    event.Skip();
}

void mmTransDialog::activateSplitTransactionsDlg()
{
    bool bDeposit = sTransaction_type_ == TRANS_TYPE_DEPOSIT_STR;
    mmSplitTransactionEntry* pSplitEntry(new mmSplitTransactionEntry);
    if (categID_ > -1)
    {
        wxString sAmount = textAmount_->GetValue();
        if (! CurrencyFormatter::formatCurrencyToDouble(sAmount, transAmount_))
            transAmount_ = 0;
        pSplitEntry->splitAmount_  = bDeposit ? transAmount_ : transAmount_;
        pSplitEntry->categID_      = categID_;
        pSplitEntry->subCategID_   = subcategID_;
        split_->addSplit(pSplitEntry);
    }
    categID_ = -1;
    subcategID_ = -1;

    SplitTransactionDialog dlg(core_, split_, transaction_type_->GetSelection(), this);
    if (dlg.ShowModal() == wxID_OK)
    {
        double amount = split_->getTotalSplits();
        if (transaction_type_->GetSelection() == DEF_TRANSFER && amount < 0)
            amount = - amount;
        wxString dispAmount = CurrencyFormatter::float2String(amount);
        textAmount_->SetValue(dispAmount);
    }
}

void mmTransDialog::onChoiceTransChar(wxKeyEvent& event)
{
    int i = transaction_type_->GetSelection();
    if (event.GetKeyCode()==WXK_DOWN)
    {
        if (i < (core_->accountList_.getNumBankAccounts() > 1 ? DEF_TRANSFER : DEF_DEPOSIT))
        {
            transaction_type_->SetSelection(++i);
        }
    }
    else if (event.GetKeyCode()==WXK_UP)
    {
        if (i > DEF_WITHDRAWAL)
        {
            transaction_type_->SetSelection(--i);
        }
    }
    else
    {
        event.Skip();
    }
    updateControlsForTransType();
}

void mmTransDialog::SetDialogToDuplicateTransaction()
{
    // we want the dialog to treat the transaction as a new transaction.
    edit_ = false;
    this->SetTitle(_("Duplicate Transaction"));

    // we need to create a new pointer for Split transactions.
    mmSplitTransactionEntries* splitTransEntries(new mmSplitTransactionEntries());
    core_->bTransactionList_.getBankTransactionPtr(pBankTransaction_->transactionID())->getSplitTransactions(splitTransEntries);
    split_->entries_ = splitTransEntries->entries_;
}
