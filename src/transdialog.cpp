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
#include "mmtextctrl.h"
#include "constants.h"
#include "util.h"
#include "mmCurrencyFormatter.h"
#include "mmOption.h"
#include "paths.h"
#include "categdialog.h"
#include "splittransactionsdialog.h"
#include "validators.h"
#include "model/Model_Payee.h"
#include "model/Model_Account.h"
#include "model/Model_Category.h"
#include "model/Model_Subcategory.h"
#include <wx/valnum.h>

IMPLEMENT_DYNAMIC_CLASS( mmTransDialog, wxDialog )

BEGIN_EVENT_TABLE( mmTransDialog, wxDialog )
    EVT_BUTTON(mmID_OK, mmTransDialog::OnOk)
    EVT_BUTTON(mmID_CANCEL, mmTransDialog::OnCancel)
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
    Model_Checking::Data *transaction
    , Model_Splittransaction::Data_Set& split
    , wxWindow* parent
    , mmCoreDB* core
    , mmBankTransaction* pBankTransaction
    , bool edit
) :
    transaction_(transaction)
    , splt_(split)
    , core_(core)
    , parent_(parent)
    , pBankTransaction_(pBankTransaction)
    , edit_(edit)
    , accountID_(transaction->ACCOUNTID)
    , referenceAccountID_(transaction->ACCOUNTID)
    , categUpdated_(false)
    , advancedToTransAmountSet_(false)
    , edit_currency_rate(1.0)
    , bBestChoice_(true)

{
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;

    Create(parent_
        , wxID_ANY
        , ""
        , wxDefaultPosition
        , wxSize(500, 400)
        , style);
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
    wxDateTime trx_date = mmGetStorageStringAsDate(transaction_->TRANSDATE);

    dpc_->SetValue(trx_date);
    //process date change event for set weekday name
    wxDateEvent dateEvent(dpc_, trx_date, wxEVT_DATE_CHANGED);
    GetEventHandler()->ProcessEvent(dateEvent);
    dpc_->SetFocus();

    wxString dispAmount;
    if (edit_)
    {
        wxString statusString = transaction_->STATUS;
        if (statusString == "") statusString = "N";
        choiceStatus_->SetSelection(wxString("NRVFD").Find(statusString));

        accountID_ = transaction_->ACCOUNTID; //pBankTransaction_->accountID_;

        textNotes_->SetValue(transaction_->NOTES);
        textNumber_->SetValue(transaction_->TRANSACTIONNUMBER);

        advancedToTransAmountSet_ = (transaction_->TRANSAMOUNT != transaction_->TOTRANSAMOUNT);

        dispAmount = CurrencyFormatter::float2String(transaction_->TRANSAMOUNT);
        textAmount_->SetValue(dispAmount);
        dispAmount = CurrencyFormatter::float2String(transaction_->TOTRANSAMOUNT);
        toTextAmount_->SetValue(dispAmount);
    }
    else
    {
        choiceStatus_->SetSelection(mmIniOptions::instance().transStatusReconciled_);
        transaction_->TRANSCODE = TRANS_TYPE_WITHDRAWAL_STR;

        notesColour_ = textNotes_->GetForegroundColour();
        textNotes_->SetForegroundColour(wxColour("GREY"));
        textNotes_->SetValue(notesTip_);
        int font_size = textNotes_->GetFont().GetPointSize();
        textNotes_->SetFont(wxFont(font_size, wxSWISS, wxNORMAL, wxNORMAL, FALSE, ""));
    }
    // backup the original currency rate first
    if (transaction_->TRANSAMOUNT > 0.0)
        edit_currency_rate = transaction_->TOTRANSAMOUNT / transaction_->TRANSAMOUNT;

    for(const auto& i : TRANSACTION_TYPE)
        transaction_type_->Append(wxGetTranslation(i), new wxStringClientData(i));
    transaction_type_->SetStringSelection(wxGetTranslation(transaction_->TRANSCODE));

    wxString categString = _("Select Category");

    updateControlsForTransType();
    if (edit_)
    {
        *split_ = *core_->bTransactionList_
            .getBankTransactionPtr(transaction_->TRANSID)->splitEntries_;
    }

    SetSplitState();

}

void mmTransDialog::OnTransTypeChanged(wxCommandEvent& /*event*/)
{
    wxString sType = transaction_->TRANSCODE;
    transaction_->TRANSCODE = TRANS_TYPE_WITHDRAWAL_STR;
    wxStringClientData* type_obj = (wxStringClientData *)transaction_type_->GetClientObject(
        transaction_type_->GetSelection());
    if (type_obj) transaction_->TRANSCODE = type_obj->GetData();
    if (sType != TRANS_TYPE_TRANSFER_STR && transaction_->TRANSCODE == TRANS_TYPE_TRANSFER_STR)
    {
        resetPayeeString();
        transaction_->CATEGID = -1;
        transaction_->SUBCATEGID = -1;
    }

    updateControlsForTransType();
}

void mmTransDialog::updateControlsForTransType()
{
    bool transfer = transaction_->TRANSCODE == TRANS_TYPE_TRANSFER_STR;
    if (!edit_)
    {
        if (mmIniOptions::instance().transPayeeSelectionNone_ > 0)
        {
            transaction_->PAYEEID = core_->bTransactionList_.getLastUsedPayeeID(accountID_
                , transaction_->TRANSCODE, transaction_->CATEGID, transaction_->SUBCATEGID);
        }

        wxString categString = resetCategoryString();
        if (mmIniOptions::instance().transCategorySelectionNone_ != 0)
        {
            transaction_->CATEGID = core_->bTransactionList_.getLastUsedCategoryID(accountID_
                , transaction_->PAYEEID, transaction_->TRANSCODE, transaction_->SUBCATEGID);
            categString = core_->categoryList_.GetFullCategoryString(transaction_->CATEGID, transaction_->SUBCATEGID);
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

    wxString dataStr = "";
    cbPayee_->Clear();
    wxSortedArrayString data;
    int type_num = transaction_type_->GetSelection();

    newAccountID_ = accountID_;
    cbAccount_->Clear();

    Model_Account::Data_Set accounts = Model_Account::instance().all();
    for (const auto &account : accounts)
    {
        data.Add(account.ACCOUNTNAME);
        cbAccount_ ->Append(account.ACCOUNTNAME);
    }
    cbAccount_->AutoComplete(data);

    Model_Account::Data *account = Model_Account::instance().get(accountID_);
    if (account) cbAccount_->SetStringSelection(account->ACCOUNTNAME);

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

        if (transaction_->TOACCOUNTID > 0)
        {
            Model_Account::Data *account = Model_Account::instance().get(transaction_->TOACCOUNTID);
            if (account) dataStr = account->ACCOUNTNAME;
        }

        payee_label_->SetLabel(_("To"));
        cbPayee_->SetToolTip(_("Specify which account the transfer is going to"));
        transaction_->PAYEEID = -1;
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
        cbAccount_->Enable(!accounts.empty());
        transaction_->TOACCOUNTID = -1;

        data = Model_Payee::instance().all_payee_names();
        toTextAmount_->Enable(false);
        toTextAmount_->SetValue("");
        advancedToTransAmountSet_ = false;
        cAdvanced_->Enable(false);

        Model_Payee::Data* payee = Model_Payee::instance().get(transaction_->PAYEEID);
        if (payee)
        {
            dataStr = payee->PAYEENAME;
        }
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

    dpc_ = new wxDatePickerCtrl( this, ID_DIALOG_TRANS_BUTTONDATE, wxDateTime::Now()
        , wxDefaultPosition, wxSize(110, -1), date_style);
#ifdef __WXGTK__ // Workaround for bug http://trac.wxwidgets.org/ticket/11630
    dpc_->Connect(ID_DIALOG_TRANS_BUTTONDATE, wxEVT_KILL_FOCUS
        , wxFocusEventHandler(mmTransDialog::OnDpcKillFocus), NULL, this);
#endif

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
    textAmount_ = new mmTextCtrl( this, ID_DIALOG_TRANS_TEXTAMOUNT, "",
        wxDefaultPosition, wxSize(110, -1),
        wxALIGN_RIGHT|wxTE_PROCESS_ENTER, mmCalcValidator());
    textAmount_->Connect(ID_DIALOG_TRANS_TEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER,
        wxCommandEventHandler(mmTransDialog::onTextEntered), NULL, this);

    toTextAmount_ = new mmTextCtrl( this, ID_DIALOG_TRANS_TOTEXTAMOUNT, "",
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
    cbPayee_->Connect(ID_DIALOG_TRANS_PAYEECOMBO, wxEVT_COMMAND_COMBOBOX_CLOSEUP,
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
    textNumber_ = new mmTextCtrl(this
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

    notesTip_ = _("Notes");
    textNotes_ = new mmTextCtrl(this, ID_DIALOG_TRANS_TEXTNOTES, "",
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

    wxButton* itemButtonOK = new wxButton( buttons_panel, mmID_OK, _("&OK"));
    itemButtonCancel_ = new wxButton( buttons_panel, mmID_CANCEL, _("&Cancel"));

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

    bool transfer_transaction = transaction_type_->GetStringSelection() == wxGetTranslation(TRANS_TYPE_TRANSFER_STR);
    if (!transfer_transaction)
    {
        const Model_Payee::Data *payee = Model_Payee::instance().get(cbPayee_->GetValue());
        if (payee) transaction_->PAYEEID = payee->PAYEEID;

        // Only for new transactions: if user want to autofill last category used for payee.
        // If this is a Split Transaction, ignore displaying last category for payee
        if (transaction_->PAYEEID != -1 && mmIniOptions::instance().transCategorySelectionNone_ == 1
            && !edit_ && !categUpdated_ && split_->numEntries() == 0)
        {
            Model_Payee::Data* payee = Model_Payee::instance().get(transaction_->PAYEEID);
            // if payee has memory of last category used then display last category for payee
            if (payee && payee->CATEGID != -1)
            {
                //TODO: move it to separate function
                Model_Category::Data *category = Model_Category::instance().get(payee->CATEGID);
                Model_Subcategory::Data *subcategory = Model_Subcategory::instance().get(payee->SUBCATEGID);
                wxString categoryName = "", subCategoryName = "";
                if (category) categoryName = category->CATEGNAME;
                if (subcategory) subCategoryName = subcategory->SUBCATEGNAME;
                wxString fullCategoryName = categoryName + (subCategoryName.IsEmpty() ? "" : ":" + subCategoryName);

                transaction_->CATEGID = payee->CATEGID;
                transaction_->SUBCATEGID = payee->SUBCATEGID;
                bCategory_->SetLabel(fullCategoryName);
                wxLogDebug("Category: %s", bCategory_->GetLabel());

            }
        }
    }
    else
    {
        transaction_->TOACCOUNTID = core_->accountList_.GetAccountId(cbPayee_->GetValue());
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
            transaction_->TRANSAMOUNT = 1;
            transaction_->TOTRANSAMOUNT = transaction_->TRANSAMOUNT;
            textAmount_->SetValue(amountStr);
        }

        CurrencyFormatter::formatCurrencyToDouble(amountStr, transaction_->TRANSAMOUNT);

        if (transaction_->TOACCOUNTID > 0) {
            double rateFrom = core_->accountList_.getAccountBaseCurrencyConvRate(accountID_);
            double rateTo = core_->accountList_.getAccountBaseCurrencyConvRate(transaction_->TOACCOUNTID);
            double convToBaseFrom = rateFrom * transaction_->TRANSAMOUNT;
            transaction_->TOTRANSAMOUNT = convToBaseFrom / rateTo;
        }
        else
        {
            toTextAmount_->SetValue("");
            transaction_->TOTRANSAMOUNT = transaction_->TRANSAMOUNT;
        }
    }
    else
    {
        transaction_->TOTRANSAMOUNT = transaction_->TRANSAMOUNT;
    }

    amountStr = CurrencyFormatter::float2String(transaction_->TOTRANSAMOUNT);
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
        core_->categoryList_.GetCategoryLikeString(categStrykes_, transaction_->CATEGID, transaction_->SUBCATEGID);

        //wxLogDebug(key + " | " + categStrykes_ + " | " + core_->categoryList_.GetFullCategoryString(categID_, subcategID_));
        //TODO: move it to separate function
        Model_Category::Data *category = Model_Category::instance().get(transaction_->CATEGID);
        Model_Subcategory::Data *subcategory = Model_Subcategory::instance().get(transaction_->SUBCATEGID);
        wxString categoryName = "", subCategoryName = "";
        if (category) categoryName = category->CATEGNAME;
        if (subcategory) subCategoryName = subcategory->SUBCATEGNAME;
        wxString fullCategoryName = categoryName + (subCategoryName.IsEmpty() ? "" : ":" + subCategoryName);

        bCategory_->SetLabel(fullCategoryName);

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
        dlg.setTreeSelection(transaction_->CATEGID, transaction_->SUBCATEGID);
        if ( dlg.ShowModal() == wxID_OK )
        {
            transaction_->CATEGID = dlg.getCategId();
            transaction_->SUBCATEGID = dlg.getSubCategId();
            categUpdated_ = true;
        }
    }
    SetSplitState();
}

wxString mmTransDialog::resetPayeeString(/*bool normal*/) //normal is deposits or withdrawls
{
    wxString payeeStr = "";

    transaction_->PAYEEID = -1;
    Model_Payee::Data_Set filtd = Model_Payee::instance().FilterPayees("");
    if (filtd.size() == 1)
    {
        //only one payee present. Choose it
        payeeStr = filtd[0].PAYEENAME;
        transaction_->PAYEEID = filtd[0].PAYEEID;
    }

    return payeeStr;
}

wxString mmTransDialog::resetCategoryString()
{
    transaction_->CATEGID = -1;
    transaction_->SUBCATEGID = -1;

    return _("Select Category");
}

bool mmTransDialog::validateData()
{
    Model_Account::Data* account = Model_Account::instance().get(cbAccount_->GetValue());
    if (account)
        newAccountID_ = account->ACCOUNTID;
    else
    {
        mmShowErrorMessageInvalid(this, _("Account"));
        return false;
    }

    bool bTransfer = (transaction_->TRANSCODE == TRANS_TYPE_TRANSFER_STR);
    advancedToTransAmountSet_ = cAdvanced_->IsChecked();

    if (cSplit_->IsChecked())
    {
        transaction_->TRANSAMOUNT = split_->getTotalSplits();
        if (transaction_->TRANSAMOUNT < 0.0)
        {
            if (bTransfer) {
                if (transaction_->TRANSAMOUNT < 0)
                    transaction_->TRANSAMOUNT = - transaction_->TRANSAMOUNT;
            } else {
                mmShowErrorMessageInvalid(parent_, _("Amount"));
                return false;
            }
        }

        if (split_->numEntries() == 0)
        {
            mmShowErrorMessageInvalid(this, _("Category"));
            return false;
        }
    }
    else
    {
        wxString amountStr = textAmount_->GetValue().Trim();
        if (! CurrencyFormatter::formatCurrencyToDouble(amountStr, transaction_->TRANSAMOUNT) || (transaction_->TRANSAMOUNT < 0.0))
        {
            textAmount_->SetBackgroundColour("RED");
            mmShowErrorMessageInvalid(parent_, _("Amount"));
            textAmount_->SetBackgroundColour(wxNullColour);
            textAmount_->SetFocus();
            return false;
        }

        Model_Category::Data *category = Model_Category::instance().get(transaction_->CATEGID);
        Model_Subcategory::Data *subcategory = Model_Subcategory::instance().get(transaction_->SUBCATEGID);
        if (!category || !subcategory)
        {
            mmShowErrorMessageInvalid(this, _("Category"));
            return false;
        }
    }

    transaction_->TOTRANSAMOUNT = transaction_->TRANSAMOUNT;
    if (bTransfer)
    {
        if (advancedToTransAmountSet_)
        {
            wxString amountStr = toTextAmount_->GetValue().Trim();
            if (amountStr.IsEmpty()
                || ! CurrencyFormatter::formatCurrencyToDouble(amountStr, transaction_->TOTRANSAMOUNT)
                || (transaction_->TOTRANSAMOUNT < 0.0)
            )
            {
                toTextAmount_->SetBackgroundColour("RED");
                mmShowErrorMessageInvalid(parent_, _("Advanced Amount"));
                toTextAmount_->SetBackgroundColour(wxNullColour);
                toTextAmount_->SetFocus();
                return false;
            }
        }
    }

    if (!bTransfer)
    {
        wxString payee_name = cbPayee_->GetValue();
        if (payee_name.IsEmpty())
        {
            mmShowErrorMessageInvalid(this, _("Payee"));
            return false;
        }

        Model_Payee::Data* payee = Model_Payee::instance().get(payee_name);
        if (!payee)
        {
            wxMessageDialog msgDlg( this
                , wxString::Format(_("Do you want to add new payee: \n%s?"), payee_name)
                , _("Confirm to add new payee")
                , wxYES_NO | wxYES_DEFAULT | wxICON_WARNING);
            if (msgDlg.ShowModal() == wxID_YES)
            {
                payee = Model_Payee::instance().create();
                payee->PAYEENAME = payee_name;
                transaction_->PAYEEID = Model_Payee::instance().save(payee);
            }
            else
                return false;
        }
    }

    transaction_->TOACCOUNTID = -1;

    if (bTransfer)
    {
        if (transaction_->TOACCOUNTID < 1 || transaction_->TOACCOUNTID == newAccountID_)
        {
            mmShowErrorMessageInvalid(this, _("To Account"));
            cbPayee_->SetFocus();
            return false;
        }

        transaction_->PAYEEID = -1;
    }
    else
    {
        Model_Payee::Data* payee = Model_Payee::instance().get(transaction_->PAYEEID);
        payee->CATEGID = transaction_->CATEGID;
        payee->SUBCATEGID = transaction_->SUBCATEGID;
        Model_Payee::instance().save(payee);
    }
    return true;
}
void mmTransDialog::OnOk(wxCommandEvent& /*event*/)
{
    if (!validateData()) return;

    textNotes_->SetFocus();
    transaction_->NOTES = textNotes_->GetValue();

    wxStringClientData* status_obj = (wxStringClientData *)choiceStatus_->GetClientObject(choiceStatus_->GetSelection());
    if (status_obj) transaction_->STATUS = status_obj->GetData().Left(1);
    transaction_->STATUS.Replace("N", "");

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

    transaction_->NOTES = textNotes_->GetValue();
    transaction_->TRANSACTIONNUMBER = textNumber_->GetValue();

    pTransaction->accountID_ = newAccountID_;
    pTransaction->toAccountID_ = transaction_->TOACCOUNTID;
    pTransaction->payeeID_ = transaction_->PAYEEID;
    Model_Payee::Data* payee = Model_Payee::instance().get(transaction_->PAYEEID);
    if (payee)
        pTransaction->payeeStr_ = payee->PAYEENAME;
    pTransaction->transType_ = transaction_->TRANSCODE;
    pTransaction->amt_ = transaction_->TRANSAMOUNT;
    pTransaction->status_ = transaction_->STATUS;
    pTransaction->transNum_ = textNumber_->GetValue();
    pTransaction->notes_ = transaction_->NOTES;
    pTransaction->categID_ = transaction_->CATEGID;
    pTransaction->subcategID_ = transaction_->SUBCATEGID;
    pTransaction->date_ = dpc_->GetValue();
    pTransaction->toAmt_ = transaction_->TOTRANSAMOUNT;

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
    wxString fullCategoryName;
    if (split_->numEntries() > 0)
        fullCategoryName = _("Split Category");
    else
    {
            //TODO: move it to separate function
            Model_Category::Data *category = Model_Category::instance().get(transaction_->CATEGID);
            Model_Subcategory::Data *subcategory = Model_Subcategory::instance().get(transaction_->SUBCATEGID);
            wxString categoryName = "", subCategoryName = "";
            if (category) categoryName = category->CATEGNAME;
            if (subcategory) subCategoryName = subcategory->SUBCATEGNAME;
            fullCategoryName = categoryName + (subCategoryName.IsEmpty() ? "" : ":" + subCategoryName);
            if (fullCategoryName.IsEmpty()) fullCategoryName = _("Select Category");
    }

    bCategory_->SetLabel(fullCategoryName);
    cSplit_->SetValue(entries > 0);
    cSplit_->Enable(transaction_->TRANSCODE != TRANS_TYPE_TRANSFER_STR);

    textAmount_->Enable(entries < 1);
}

void mmTransDialog::OnSplitChecked(wxCommandEvent& /*event*/)
{
    /* Reset Category */
    //split_ = wxSharedPtr<mmSplitTransactionEntries>(new mmSplitTransactionEntries());
    if (cSplit_->IsChecked())
    {
        activateSplitTransactionsDlg();
    }
    else
    {
        if (split_->numEntries() != 1)
        {
            transaction_->TRANSAMOUNT = 0;
        }
        else
        {
            transaction_->CATEGID    = split_->entries_[0]->categID_;
            transaction_->SUBCATEGID = split_->entries_[0]->subCategID_;
            transaction_->TRANSAMOUNT  = split_->entries_[0]->splitAmount_;

            if (transaction_->TRANSAMOUNT < 0 )
            {
                transaction_->TRANSAMOUNT = - transaction_->TRANSAMOUNT;
                transaction_type_->SetStringSelection(wxGetTranslation(TRANS_TYPE_WITHDRAWAL_STR));
            }
            split_->removeSplitByIndex(0);
        }
        wxString dispAmount = CurrencyFormatter::float2String(transaction_->TRANSAMOUNT);
        textAmount_->SetValue(dispAmount);
    }
    SetSplitState();
}

//----------------------------------------------------------------------------
// Workaround for bug http://trac.wxwidgets.org/ticket/11630
void mmTransDialog::OnDpcKillFocus(wxFocusEvent& event)
{
    if (wxGetKeyState(WXK_TAB) && wxGetKeyState(WXK_SHIFT))
        itemButtonCancel_->SetFocus();
    else if (wxGetKeyState(WXK_TAB))
        choiceStatus_->SetFocus();
    else if (wxGetKeyState(WXK_UP))
    {
        wxCommandEvent evt(wxEVT_SPIN, wxID_ANY);
        evt.SetInt(1);
        this->GetEventHandler()->AddPendingEvent(evt);
    }
    else if (wxGetKeyState(WXK_DOWN))
    {
        wxCommandEvent evt(wxEVT_SPIN, wxID_ANY);
        evt.SetInt(-1);
        this->GetEventHandler()->AddPendingEvent(evt);
    }
    else
        event.Skip();

}

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
    bool bDeposit = transaction_->TRANSCODE == TRANS_TYPE_DEPOSIT_STR;
    mmSplitTransactionEntry* pSplitEntry(new mmSplitTransactionEntry);
    if (transaction_->CATEGID > -1)
    {
        wxString sAmount = textAmount_->GetValue();
        if (! CurrencyFormatter::formatCurrencyToDouble(sAmount, transaction_->TRANSAMOUNT))
            transaction_->TRANSAMOUNT = 0;
        pSplitEntry->splitAmount_  = bDeposit ? transaction_->TRANSAMOUNT : transaction_->TRANSAMOUNT;
        pSplitEntry->categID_      = transaction_->CATEGID;
        pSplitEntry->subCategID_   = transaction_->SUBCATEGID;
        split_->addSplit(pSplitEntry);
    }
    transaction_->CATEGID = -1;
    transaction_->SUBCATEGID = -1;

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

    // we need to create a new pointer for Split transactions.
    mmSplitTransactionEntries* splitTransEntries(new mmSplitTransactionEntries());
    core_->bTransactionList_.getBankTransactionPtr(pBankTransaction_->transactionID())->getSplitTransactions(splitTransEntries);
    split_->entries_ = splitTransEntries->entries_;
}

void mmTransDialog::SetDialogTitle(const wxString& title)
{
    this->SetTitle(title);
}
