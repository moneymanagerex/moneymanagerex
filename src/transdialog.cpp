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
#include "util.h"
#include "mmOption.h"
#include "paths.h"
#include "categdialog.h"
#include "splittransactionsdialog.h"
#include "mmCalculator.h"
#include "validators.h"
#include "model/Model_Payee.h"
#include "model/Model_Account.h"
#include "model/Model_Category.h"
#include "model/Model_Subcategory.h"
#include <wx/valnum.h>

IMPLEMENT_DYNAMIC_CLASS( mmTransDialog, wxDialog )

BEGIN_EVENT_TABLE( mmTransDialog, wxDialog )
    EVT_BUTTON(wxID_OK, mmTransDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, mmTransDialog::OnCancel)
    EVT_BUTTON(wxID_VIEW_DETAILS, mmTransDialog::OnCategs)
    EVT_CLOSE(mmTransDialog::OnQuit)
    EVT_CHOICE(wxID_ANY, mmTransDialog::OnTransTypeChanged)
    EVT_CHECKBOX(ID_DIALOG_TRANS_ADVANCED_CHECKBOX, mmTransDialog::OnAdvanceChecked)
    EVT_CHECKBOX(wxID_FORWARD, mmTransDialog::OnSplitChecked)
    EVT_CHILD_FOCUS(mmTransDialog::changeFocus)
    EVT_SPIN(wxID_ANY,mmTransDialog::OnSpin)
    EVT_DATE_CHANGED(ID_DIALOG_TRANS_BUTTONDATE, mmTransDialog::OnDateChanged)
END_EVENT_TABLE()

mmTransDialog::mmTransDialog(wxWindow* parent
    , int account_id
    , int transaction_id
) :
      parent_(parent)
    , accountID_(account_id)
    , referenceAccountID_(account_id)
    , transaction_id_(transaction_id)
    , categUpdated_(false)
    , advancedToTransAmountSet_(false)
    , edit_currency_rate(1.0)
    , skip_account_init_(false)
    , skip_payee_init_(false)
    , skip_status_init_(false)
    , skip_notes_init_(false)
    , skip_category_init_(false)

{
    if (transaction_id_)
    {
        transaction_ = Model_Checking::instance().get(transaction_id_);
        for (const auto& item : Model_Checking::splittransaction(transaction_)) m_local_splits.push_back(item);
    }
    else
    {
        wxDateTime trx_date = wxDateTime::Today();
        if (mmIniOptions::instance().transDateDefault_ != 0)
        {
            Model_Account::Data *account = Model_Account::instance().get(accountID_);
            if (account) trx_date = Model_Account::last_date(Model_Account::instance().get(accountID_));
        }

        transaction_ = Model_Checking::instance().create();

        transaction_->STATUS = Model_Checking::toShortStatus(Model_Checking::all_status()[mmIniOptions::instance().transStatusReconciled_]);
        transaction_->ACCOUNTID = accountID_;
        transaction_->TRANSDATE = trx_date.FormatISODate();
        transaction_->TRANSCODE = Model_Checking::all_type()[Model_Checking::WITHDRAWAL];
        transaction_->CATEGID = -1;
        transaction_->SUBCATEGID = -1;
        if (mmIniOptions::instance().transPayeeSelectionNone_)
        {
            Model_Checking::Data_Set transactions = Model_Checking::instance().all(Model_Checking::COL_TRANSDATE, false);
            for (const auto &trx : transactions)
            {
                if (trx.ACCOUNTID != transaction_->ACCOUNTID) continue;
                if (Model_Checking::type(trx) == Model_Checking::TRANSFER) continue;
                transaction_->PAYEEID = trx.PAYEEID;
                Model_Payee::Data * payee = Model_Payee::instance().get(trx.PAYEEID);

                if (payee && mmIniOptions::instance().transCategorySelectionNone_)
                {
                    transaction_->CATEGID = payee->CATEGID;
                    transaction_->SUBCATEGID = payee->SUBCATEGID;
                }
                break;
            }
        }
    }

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

    advancedToTransAmountSet_ = (transaction_->TRANSAMOUNT != transaction_->TOTRANSAMOUNT);
    dataToControls();

    Centre();
    Fit();

    return TRUE;
}

void mmTransDialog::dataToControls()
{
    //Dialog title
    if (transaction_id_)
        SetDialogTitle(_("Edit Transaction"));
    else
        SetDialogTitle(_("New Transaction"));

    bool transfer = Model_Checking::type(transaction_) == Model_Checking::TRANSFER;

    //Date
    const wxDateTime trx_date = Model_Checking::TRANSDATE(transaction_);
    dpc_->SetValue(trx_date);
    //process date change event for set weekday name
    wxDateEvent dateEvent(dpc_, trx_date, wxEVT_DATE_CHANGED);
    GetEventHandler()->ProcessEvent(dateEvent);

    //Status
    if (!skip_status_init_)
    {
        choiceStatus_->SetSelection(Model_Checking::status(transaction_));
        skip_status_init_ = true;
	}

    //Type
    transaction_type_->SetSelection(Model_Checking::type(transaction_));

    //Advanced
    cAdvanced_->Enable(transfer);
    cAdvanced_->SetValue(advancedToTransAmountSet_);

    //Amounts
    if (transaction_->TRANSAMOUNT)
        textAmount_->SetValue(transaction_->TRANSAMOUNT, Model_Account::instance().get(accountID_));
    if (transaction_->TOTRANSAMOUNT && cAdvanced_->IsChecked())
        toTextAmount_->SetValue(transaction_->TOTRANSAMOUNT, Model_Account::instance().get(accountID_));
    if (!transfer)
        toTextAmount_->SetValue("");

    toTextAmount_->Enable(cAdvanced_->IsChecked());
    textAmount_->UnsetToolTip();
    toTextAmount_->UnsetToolTip();

    // backup the original currency rate first
    if (transaction_->TRANSAMOUNT > 0.0)
        edit_currency_rate = transaction_->TOTRANSAMOUNT / transaction_->TRANSAMOUNT;

    //Account
    if (!skip_account_init_)
    {
        cbAccount_->SetEvtHandlerEnabled(false);
        cbAccount_->Clear();
        newAccountID_ = accountID_;
        Model_Account::Data_Set accounts = Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME);
        for (const auto &account : accounts)
        {
            if (Model_Account::type(account) == Model_Account::INVESTMENT) continue;
            cbAccount_->Append(account.ACCOUNTNAME);
            if (account.ACCOUNTID == transaction_->ACCOUNTID) cbAccount_->SetStringSelection(account.ACCOUNTNAME);
        }
        cbAccount_->AutoComplete(Model_Account::instance().all_checking_account_names());
        accountID_ = transaction_->ACCOUNTID;
        if (accounts.size() == 1)
        {
            cbAccount_->SetValue(accounts.begin()->ACCOUNTNAME);
            cbAccount_->Enable(false);
        }
        cbAccount_->SetEvtHandlerEnabled(true);
        skip_account_init_ = true;
    }

    //Payee or To Account
    if (!skip_payee_init_)
    {
        cbPayee_->SetEvtHandlerEnabled(false);

        cbPayee_->Clear();
        cbAccount_->UnsetToolTip();
        cbPayee_->UnsetToolTip();
        wxString payee_tooltip = "";
        if (!transfer)
        {
            textAmount_->SetToolTip(amountNormalTip_);

            if (transaction_->TRANSCODE == Model_Checking::all_type()[Model_Checking::WITHDRAWAL])
            {
                payee_tooltip = _("Specify to whom the transaction is going to");
                payee_label_->SetLabel(_("Payee"));
            }
            else
            {
                payee_tooltip = _("Specify where the transaction is coming from");
                payee_label_->SetLabel(_("From"));
            }

            cbAccount_->SetToolTip(_("Specify account for the transaction"));
            account_label_->SetLabel(_("Account"));
            transaction_->TOACCOUNTID = -1;

            for (const auto & entry : Model_Payee::instance().all_payee_names())
                cbPayee_->Append(entry);
            cbPayee_->AutoComplete(Model_Payee::instance().all_payee_names());
            Model_Payee::Data* payee = Model_Payee::instance().get(transaction_->PAYEEID);
            if (payee)
                cbPayee_->SetStringSelection(payee->PAYEENAME);
        }
        else
        {
            textAmount_->SetToolTip(amountTransferTip_);
            toTextAmount_->SetToolTip(_("Specify the transfer amount in the To Account"));
            if (cSplit_->IsChecked())
            {
                cSplit_->SetValue(false);
                m_local_splits.clear();
            }

            for (const auto & entry : Model_Account::instance().all_checking_account_names())
                cbPayee_->Append(entry);
            cbPayee_->AutoComplete(Model_Account::instance().all_checking_account_names());

            Model_Account::Data *account = Model_Account::instance().get(transaction_->TOACCOUNTID);
            if (account)
                cbPayee_->SetStringSelection(account->ACCOUNTNAME);

            payee_label_->SetLabel(_("To"));
            payee_tooltip = _("Specify which account the transfer is going to");
            transaction_->PAYEEID = -1;
            account_label_->SetLabel(_("From"));
            cbAccount_->SetToolTip(_("Specify which account the transfer is going from"));
            cbAccount_->Enable(true);
        }
        cbPayee_->SetToolTip(payee_tooltip);
        skip_payee_init_ = true;
        cbPayee_->SetEvtHandlerEnabled(true);
    }

    //SetSplitState();
    if (!skip_category_init_)
    {
        bool has_split = !this->m_local_splits.empty();
        wxString fullCategoryName;
        bCategory_->UnsetToolTip();
        if (has_split)
        {
            fullCategoryName = _("Categories");
            bCategory_->SetToolTip(_("Specify categories for this transaction"));
            double total = Model_Splittransaction::instance().get_total(m_local_splits);
            textAmount_->SetValue(total);
        }
        else
        {
            Model_Category::Data *category = Model_Category::instance().get(transaction_->CATEGID);
            Model_Subcategory::Data *subcategory = (Model_Subcategory::instance().get(transaction_->SUBCATEGID));
            fullCategoryName = Model_Category::full_name(category, subcategory);
            if (fullCategoryName.IsEmpty()) fullCategoryName = _("Select Category");
            bCategory_->SetToolTip(_("Specify the category for this transaction"));
        }

        bCategory_->SetLabel(fullCategoryName);
        textAmount_->Enable(!has_split);
        cSplit_->SetValue(has_split);
        cSplit_->Enable(Model_Checking::type(transaction_) != Model_Checking::TRANSFER);
        skip_category_init_ = true;
    }

    //Notes & Transaction Number
    if (!skip_notes_init_)
    {
        textNumber_->SetValue(transaction_->TRANSACTIONNUMBER);
        textNotes_->SetValue(transaction_->NOTES);
        if (transaction_->NOTES.IsEmpty() && !transaction_id_)
        {
            notesColour_ = textNotes_->GetForegroundColour();
            textNotes_->SetForegroundColour(wxColour("GREY"));
            textNotes_->SetValue(notesTip_);
            int font_size = textNotes_->GetFont().GetPointSize();
            textNotes_->SetFont(wxFont(font_size, wxSWISS, wxNORMAL, wxNORMAL, FALSE, ""));
        }
        skip_notes_init_ = true;
    }
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

    for(const auto& i : Model_Checking::all_status())
        choiceStatus_->Append(wxGetTranslation(i), new wxStringClientData(i));

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Status")), flags);
    flex_sizer->Add(choiceStatus_, flags);

    // Type --------------------------------------------
    transaction_type_ = new wxChoice(this, wxID_ANY,
        wxDefaultPosition, wxSize(110, -1));

    for (const auto& i : Model_Checking::all_type())
    {
        if (i != Model_Checking::all_type()[Model_Checking::TRANSFER] || Model_Account::instance().all().size() > 1)
            transaction_type_->Append(wxGetTranslation(i), new wxStringClientData(i));
    }

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

    toTextAmount_ = new mmTextCtrl( this, ID_DIALOG_TRANS_TOTEXTAMOUNT, "",
        wxDefaultPosition, wxSize(110, -1),
        wxALIGN_RIGHT|wxTE_PROCESS_ENTER, mmCalcValidator());

    wxBoxSizer* amountSizer = new wxBoxSizer(wxHORIZONTAL);
    amountSizer->Add(textAmount_, flags);
    amountSizer->Add(toTextAmount_, flags);

    flex_sizer->Add(new wxStaticText( this, wxID_STATIC, _("Amount")), flags);
    flex_sizer->Add(amountSizer);

    // Account ---------------------------------------------
    cbAccount_ = new wxComboBox(this, wxID_ANY, "",
        wxDefaultPosition, wxSize(230, -1));

    account_label_ = new wxStaticText(this, wxID_STATIC, _("Account"));
    flex_sizer->Add(account_label_, flags);
    flex_sizer->Add(cbAccount_, flags);

    // Payee ---------------------------------------------
    payee_label_ = new wxStaticText(this, wxID_STATIC, _("Payee"));

    /*Note: If you want to use EVT_TEXT_ENTER(id,func) to receive wxEVT_COMMAND_TEXT_ENTER events,
      you have to add the wxTE_PROCESS_ENTER window style flag.
      If you create a wxComboBox with the flag wxTE_PROCESS_ENTER, the tab key won't jump to the next control anymore.*/
    cbPayee_ = new wxComboBox(this, ID_DIALOG_TRANS_PAYEECOMBO, ""
        , wxDefaultPosition, wxSize(230, -1));

    flex_sizer->Add(payee_label_, flags);
    flex_sizer->Add(cbPayee_, flags);

    // Split Category -------------------------------------------
    cSplit_ = new wxCheckBox(this, wxID_FORWARD
        , _("Split"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    cSplit_->SetValue(FALSE);

    flex_sizer->AddSpacer(20);  // Fill empty space.
    flex_sizer->Add(cSplit_, flags);

    // Category -------------------------------------------------
    bCategory_ = new wxButton(this, wxID_VIEW_DETAILS, ""
        , wxDefaultPosition, wxSize(230, -1));

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Category")), flags);
    flex_sizer->Add(bCategory_, flags);

    // Number  ---------------------------------------------
    textNumber_ = new mmTextCtrl(this
        , ID_DIALOG_TRANS_TEXTNUMBER, "", wxDefaultPosition
        , wxDefaultSize, wxTE_PROCESS_ENTER);

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
    textNotes_ = new mmTextCtrl(this, ID_DIALOG_TRANS_TEXTNOTES, ""
        , wxDefaultPosition, wxSize(-1,80), wxTE_MULTILINE);

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

    wxButton* itemButtonOK = new wxButton( buttons_panel, wxID_OK, _("&OK "));
    itemButtonCancel_ = new wxButton( buttons_panel, wxID_CANCEL, _("&Cancel "));

    buttons_sizer->Add(itemButtonOK, flags.Border(wxBOTTOM|wxRIGHT, 10));
    buttons_sizer->Add(itemButtonCancel_, flags);

    buttons_sizer->Realize();
    Center();
    this->SetSizer(box_sizer1);

    cbAccount_->Connect(wxID_ANY, wxEVT_COMMAND_TEXT_UPDATED,
        wxCommandEventHandler(mmTransDialog::OnAccountUpdated), NULL, this);
    cbPayee_->Connect(ID_DIALOG_TRANS_PAYEECOMBO, wxEVT_COMMAND_TEXT_UPDATED,
        wxCommandEventHandler(mmTransDialog::OnPayeeUpdated), NULL, this);
    cbPayee_->Connect(ID_DIALOG_TRANS_PAYEECOMBO, wxEVT_COMMAND_COMBOBOX_CLOSEUP,
        wxCommandEventHandler(mmTransDialog::OnPayeeUpdated), NULL, this);
    textAmount_->Connect(ID_DIALOG_TRANS_TEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER,
        wxCommandEventHandler(mmTransDialog::onTextEntered), NULL, this);
    toTextAmount_->Connect(ID_DIALOG_TRANS_TOTEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER,
        wxCommandEventHandler(mmTransDialog::onTextEntered), NULL, this);
    textNumber_->Connect(ID_DIALOG_TRANS_TEXTNUMBER, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmTransDialog::onTextEntered), NULL, this);

#ifdef __WXGTK__ // Workaround for bug http://trac.wxwidgets.org/ticket/11630
    dpc_->Connect(ID_DIALOG_TRANS_BUTTONDATE, wxEVT_KILL_FOCUS
        , wxFocusEventHandler(mmTransDialog::OnDpcKillFocus), NULL, this);
#endif

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

bool mmTransDialog::validateData()
{
    Model_Account::Data* account = Model_Account::instance().get(cbAccount_->GetValue());
    if (account && Model_Account::type(account) != Model_Account::INVESTMENT)
        newAccountID_ = account->ACCOUNTID;
    else
    {
        mmShowErrorMessageInvalid(this, _("Account"));
        return false;
    }

    bool bTransfer = (Model_Checking::type(transaction_) == Model_Checking::TRANSFER);
    advancedToTransAmountSet_ = cAdvanced_->IsChecked();

    if (cSplit_->IsChecked())
    {
        transaction_->TRANSAMOUNT = Model_Splittransaction::instance().get_total(m_local_splits);
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

        if (m_local_splits.empty())
        {
            mmShowErrorMessageInvalid(this, _("Category"));
            return false;
        }
    }
    else
    {
        wxString amountStr = textAmount_->GetValue().Trim();
        if (!Model_Currency::fromString(amountStr, transaction_->TRANSAMOUNT
            , Model_Account::currency(account)))
        {
            textAmount_->SetBackgroundColour("RED");
            mmShowErrorMessageInvalid(parent_, _("Amount"));
            textAmount_->SetBackgroundColour(wxNullColour);
            textAmount_->SetFocus();
            return false;
        }

        Model_Category::Data *category = Model_Category::instance().get(transaction_->CATEGID);
        Model_Subcategory::Data *subcategory = Model_Subcategory::instance().get(transaction_->SUBCATEGID);
        if (!category || !(subcategory || transaction_->SUBCATEGID < 0))
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
                || !amountStr.ToDouble(&transaction_->TOTRANSAMOUNT)
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
                Model_Payee::instance().save(payee);
            }
            else
                return false;
        }
        transaction_->PAYEEID = payee->PAYEEID;
        transaction_->TOACCOUNTID = -1;

        payee->CATEGID = transaction_->CATEGID;
        payee->SUBCATEGID = transaction_->SUBCATEGID;
        Model_Payee::instance().save(payee);
    }
    else
    {
        Model_Account::Data *to_account = Model_Account::instance().get(transaction_->TOACCOUNTID);
        if (!account || transaction_->TOACCOUNTID == newAccountID_ || Model_Account::type(to_account) == Model_Account::INVESTMENT)
        {
            mmShowErrorMessageInvalid(this, _("To Account"));
            cbPayee_->SetFocus();
            return false;
        }

        transaction_->PAYEEID = -1;
    }
    return true;
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

    if (textNotes_->GetValue() == notesTip_ && object_in_focus_ == ID_DIALOG_TRANS_TEXTNOTES)
    {
        textNotes_->SetValue("");
        textNotes_->SetForegroundColour(notesColour_);
    }
    event.Skip();
}

void mmTransDialog::activateSplitTransactionsDlg()
{
    bool bDeposit = transaction_->TRANSCODE == Model_Checking::all_type()[Model_Checking::DEPOSIT];

    if (transaction_->CATEGID > -1)
    {
        Model_Splittransaction::Data *split = Model_Splittransaction::instance().create();
        wxString sAmount = textAmount_->GetValue();
        if (!sAmount.ToDouble(&transaction_->TRANSAMOUNT))
            transaction_->TRANSAMOUNT = 0;
        split->SPLITTRANSAMOUNT = bDeposit ? transaction_->TRANSAMOUNT : transaction_->TRANSAMOUNT;
        split->CATEGID = transaction_->CATEGID;
        split->SUBCATEGID = transaction_->SUBCATEGID;
        m_local_splits.push_back(*split);
    }
    transaction_->CATEGID = -1;
    transaction_->SUBCATEGID = -1;
    
    SplitTransactionDialog dlg(&m_local_splits, this, transaction_type_->GetSelection());
    if (dlg.ShowModal() == wxID_OK)
    {
        double amount = Model_Splittransaction::instance().get_total(m_local_splits);
        if (transaction_type_->GetSelection() == DEF_TRANSFER && amount < 0)
            amount = - amount;
        Model_Account::Data* account = Model_Account::instance().get(cbAccount_->GetValue());
        wxString dispAmount = Model_Currency::toString(amount, Model_Account::currency(account));
        textAmount_->SetValue(dispAmount);
        textAmount_->Enable(false);
    }
}

void mmTransDialog::SetDialogTitle(const wxString& title)
{
    this->SetTitle(title);
}

//** --------------=Event handlers=----------------- **//
void mmTransDialog::OnDateChanged(wxDateEvent& event)
{
    //get weekday name
    wxDateTime date = dpc_->GetValue();
    if (event.GetDate().IsValid())
        itemStaticTextWeek_->SetLabel(wxGetTranslation(date.GetWeekDayName(date.GetWeekDay())));
    event.Skip();
}

void mmTransDialog::OnSpin(wxSpinEvent& event)
{
    wxDateTime date = dpc_->GetValue();
    int value = event.GetPosition();

    date = date.Add(wxDateSpan::Days(value));
    dpc_->SetValue(date);
    spinCtrl_->SetValue(0);

    //process date change event for set weekday name
    wxDateEvent dateEvent(dpc_, date, wxEVT_DATE_CHANGED);
    GetEventHandler()->ProcessEvent(dateEvent);

    event.Skip();
}

void mmTransDialog::OnTransTypeChanged(wxCommandEvent& event)
{
    const wxString old_type = transaction_->TRANSCODE;
    wxStringClientData *client_obj = (wxStringClientData *) event.GetClientObject();
    if (client_obj) transaction_->TRANSCODE = client_obj->GetData();
    if (old_type != transaction_->TRANSCODE)
    {
        skip_payee_init_ = false;
        dataToControls();
    }
}


void mmTransDialog::OnAccountUpdated(wxCommandEvent& /*event*/)
{
    const Model_Account::Data* account = Model_Account::instance().get(cbAccount_->GetValue());
    if (account) newAccountID_ = account->ACCOUNTID;
    wxLogDebug("%i", newAccountID_);
}

void mmTransDialog::OnPayeeUpdated(wxCommandEvent& event)
{
    bool transfer_transaction = transaction_type_->GetSelection() == Model_Checking::TRANSFER;
    if (!transfer_transaction)
    {
        const Model_Payee::Data *payee = Model_Payee::instance().get(cbPayee_->GetValue());
        if (payee) transaction_->PAYEEID = payee->PAYEEID;

        // Only for new transactions: if user want to autofill last category used for payee.
        // If this is a Split Transaction, ignore displaying last category for payee
        if (transaction_->PAYEEID != -1 && mmIniOptions::instance().transCategorySelectionNone_ == 1
            && !categUpdated_ && m_local_splits.empty())
        {
            Model_Payee::Data* payee = Model_Payee::instance().get(transaction_->PAYEEID);
            // if payee has memory of last category used then display last category for payee
            if (payee && payee->CATEGID != -1)
            {
                Model_Category::Data *category = Model_Category::instance().get(payee->CATEGID);
                Model_Subcategory::Data *subcategory = (payee->SUBCATEGID != -1 ? Model_Subcategory::instance().get(payee->SUBCATEGID) : 0);
                wxString fullCategoryName = Model_Category::full_name(category, subcategory);

                transaction_->CATEGID = payee->CATEGID;
                transaction_->SUBCATEGID = payee->SUBCATEGID;
                bCategory_->SetLabel(fullCategoryName);
                wxLogDebug("Category: %s", bCategory_->GetLabel());

            }
        }
    }
    else
    {
        const Model_Account::Data* account = Model_Account::instance().get(cbPayee_->GetValue());
        if (account) transaction_->TOACCOUNTID = account->ACCOUNTID;
    }

    event.Skip();
}

void mmTransDialog::OnSplitChecked(wxCommandEvent& /*event*/)
{
    if (cSplit_->IsChecked())
    {
        activateSplitTransactionsDlg();
    }
    else
    {
        if (m_local_splits.size() == 1)
        {
            transaction_->CATEGID = m_local_splits.begin()->CATEGID;
            transaction_->SUBCATEGID = m_local_splits.begin()->SUBCATEGID;
            transaction_->TRANSAMOUNT = m_local_splits.begin()->SPLITTRANSAMOUNT;

            if (transaction_->TRANSAMOUNT < 0)
            {
                transaction_->TRANSAMOUNT = -transaction_->TRANSAMOUNT;
                transaction_type_->SetSelection(Model_Checking::WITHDRAWAL);
            }
            m_local_splits.clear();
        }
        else if (m_local_splits.empty())
        {
            transaction_->TRANSAMOUNT = 0;
        }
        else
            //Delete split items first (data protection)
            cSplit_->SetValue(true);
    }
    skip_category_init_ = false;
    dataToControls();
}

void mmTransDialog::OnAutoTransNum(wxCommandEvent& /*event*/)
{
    //TODO:
    wxString number = textNumber_->GetValue();
    double next_number = 1;
    if (number.ToDouble(&next_number))
    {
        next_number++;
        number = wxString::Format("%i", static_cast<int>(next_number));
    }

    if (number.IsEmpty()) number = "1";
    textNumber_->SetValue(number);
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

        Model_Currency::fromString(amountStr, transaction_->TRANSAMOUNT, 0);

        if (transaction_->TOACCOUNTID > 0)
        {
            const Model_Account::Data* from_account = Model_Account::instance().get(accountID_);
            const Model_Account::Data* to_account = Model_Account::instance().get(transaction_->TOACCOUNTID);

            const Model_Currency::Data* from_currency = Model_Account::currency(from_account);
            const Model_Currency::Data* to_currency = Model_Account::currency(to_account);
            double rateFrom = from_currency->BASECONVRATE;
            double rateTo = to_currency->BASECONVRATE;
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

    toTextAmount_->SetValue(transaction_->TOTRANSAMOUNT);

    dataToControls();
}

void mmTransDialog::OnCategs(wxCommandEvent& /*event*/)
{
    if (cSplit_->IsChecked())
    {
        activateSplitTransactionsDlg();
        dataToControls();
    }
    else
    {
        mmCategDialog dlg(parent_, true, false);
        dlg.setTreeSelection(transaction_->CATEGID, transaction_->SUBCATEGID);
        if (dlg.ShowModal() == wxID_OK)
        {
            transaction_->CATEGID = dlg.getCategId();
            transaction_->SUBCATEGID = dlg.getSubCategId();
            bCategory_->SetLabel(dlg.getFullCategName());
            categUpdated_ = true;
        }
    }
}

void mmTransDialog::onTextEntered(wxCommandEvent& WXUNUSED(event))
{
    wxString sAmount = "";

    Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();
    Model_Account::Data *account = Model_Account::instance().get(accountID_);
    if (account) currency = Model_Account::currency(account);

    mmCalculator calc;
    if (object_in_focus_ == textAmount_->GetId())
    {
        sAmount = wxString() << Model_Currency::fromString(textAmount_->GetValue(), currency);
        if (calc.is_ok(sAmount))
            textAmount_->SetValue(Model_Currency::toString(calc.get_result(), currency));
        textAmount_->SetInsertionPoint(textAmount_->GetValue().Len());
    }
    else if (object_in_focus_ == toTextAmount_->GetId())
    {
        sAmount = wxString() << Model_Currency::fromString(toTextAmount_->GetValue(), currency);
        if (calc.is_ok(sAmount))
            toTextAmount_->SetValue(Model_Currency::toString(calc.get_result(), currency));
        toTextAmount_->SetInsertionPoint(toTextAmount_->GetValue().Len());
    }
    else if (object_in_focus_ == textNumber_->GetId())
    {
        textNotes_->SetFocus();
    }
}

void mmTransDialog::OnOk(wxCommandEvent& event)
{
    if (!validateData()) return;

    transaction_->STATUS = "";
    wxStringClientData* status_obj = (wxStringClientData *)choiceStatus_->GetClientObject(choiceStatus_->GetSelection());
    if (status_obj) transaction_->STATUS = Model_Checking::toShortStatus(status_obj->GetData());

    textNotes_->SetFocus();
    transaction_->NOTES = textNotes_->GetValue();
    transaction_->TRANSACTIONNUMBER = textNumber_->GetValue();

    transaction_->ACCOUNTID = newAccountID_;
    transaction_->TOACCOUNTID = transaction_->TOACCOUNTID;
    transaction_->TRANSDATE = dpc_->GetValue().FormatISODate();

    if (!m_local_splits.empty())
    {
        this->transaction_->TRANSAMOUNT = Model_Splittransaction::instance().get_total(m_local_splits);
        this->transaction_->CATEGID = -1;
        this->transaction_->SUBCATEGID = -1;
    }

    transaction_id_ = Model_Checking::instance().save(transaction_);
    for (auto &item : m_local_splits) item.TRANSID = transaction_->TRANSID;
    Model_Splittransaction::instance().save(m_local_splits);

    wxLogDebug(transaction_->to_json());
    EndModal(wxID_OK);
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

    if ((int) object_in_focus_ == (int) toTextAmount_->GetId())
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

    if ((int) object_in_focus_ == (int) textNumber_->GetId())
    {
        if (!textNumber_->IsEmpty())
            textNumber_->SetValue("");
        else
            itemButtonCancel_->SetFocus();
        return;
    }

    EndModal(wxID_CANCEL);
}

void mmTransDialog::OnQuit(wxCloseEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}
