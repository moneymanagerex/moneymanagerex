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
#include "constants.h"
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
    EVT_BUTTON(wxID_OK, mmTransDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, mmTransDialog::OnCancel)
    EVT_BUTTON(wxID_VIEW_DETAILS, mmTransDialog::OnCategs)
    EVT_CLOSE(mmTransDialog::OnQuit)
    EVT_CHOICE(ID_DIALOG_TRANS_TYPE, mmTransDialog::OnTransTypeChanged)
    EVT_CHECKBOX(ID_DIALOG_TRANS_ADVANCED_CHECKBOX, mmTransDialog::OnAdvanceChecked)
    EVT_CHECKBOX(wxID_FORWARD, mmTransDialog::OnSplitChecked)
    EVT_CHILD_FOCUS(mmTransDialog::onFocusChange)
    EVT_SPIN(wxID_ANY,mmTransDialog::OnSpin)
    EVT_DATE_CHANGED(ID_DIALOG_TRANS_BUTTONDATE, mmTransDialog::OnDateChanged)
    EVT_COMBOBOX(wxID_ANY, mmTransDialog::OnAccountOrPayeeUpdated)
END_EVENT_TABLE()

mmTransDialog::mmTransDialog(wxWindow* parent
    , int account_id
    , int transaction_id
) :
      accountID_(account_id)
    , referenceAccountID_(account_id)
    , transaction_id_(transaction_id)
    , categUpdated_(false)
    , m_transfer(false)
    , advancedToTransAmountSet_(false)
    , edit_currency_rate(1.0)
    , skip_account_init_(false)
    , skip_payee_init_(false)
    , skip_status_init_(false)
    , skip_notes_init_(false)
    , skip_category_init_(false)
    , skip_amount_init_(false)
{
    if (transaction_id_)
    {
        transaction_ = Model_Checking::instance().get(transaction_id_);
        for (const auto& item : Model_Checking::splittransaction(transaction_)) m_local_splits.push_back(item);
        m_transfer = Model_Checking::type(transaction_) == Model_Checking::TRANSFER;
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
                if (m_transfer) continue;
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

    Create(parent
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
    cAdvanced_->Enable(m_transfer);
    cAdvanced_->SetValue(advancedToTransAmountSet_ && m_transfer);

    //Amounts
    if (!skip_amount_init_)
    {
        if (transaction_->TRANSAMOUNT)
            textAmount_->SetValue(transaction_->TRANSAMOUNT, Model_Account::instance().get(accountID_));
        skip_amount_init_ = true;
    }

    if (transaction_->TOTRANSAMOUNT && advancedToTransAmountSet_)
        toTextAmount_->SetValue(transaction_->TOTRANSAMOUNT, Model_Account::instance().get(accountID_));
    if (!m_transfer)
        toTextAmount_->SetValue("");

    toTextAmount_->Enable(cAdvanced_->IsChecked() && m_transfer);

    // backup the original currency rate first
    if (transaction_->TRANSAMOUNT > 0.0)
        edit_currency_rate = transaction_->TOTRANSAMOUNT / transaction_->TRANSAMOUNT;

    //Account
    if (!skip_account_init_)
    {
        cbAccount_->SetEvtHandlerEnabled(false);
        cbAccount_->Clear();
        Model_Account::Data_Set accounts = Model_Account::instance().all(Model_Account::COL_ACCOUNTNAME);

        for (const auto &account : accounts)
        {
            if (Model_Account::type(account) == Model_Account::INVESTMENT) continue;
            cbAccount_->Append(account.ACCOUNTNAME);
            if (account.ACCOUNTID == transaction_->ACCOUNTID) cbAccount_->ChangeValue(account.ACCOUNTNAME);
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
        if (!m_transfer)
        {
            if (transaction_->TRANSCODE == Model_Checking::all_type()[Model_Checking::WITHDRAWAL])
                payee_label_->SetLabel(_("Payee"));
            else
                payee_label_->SetLabel(_("From"));

            account_label_->SetLabel(_("Account"));
            transaction_->TOACCOUNTID = -1;
            for (const auto & entry : Model_Payee::instance().all_payee_names())
                cbPayee_->Append(entry);
            cbPayee_->AutoComplete(Model_Payee::instance().all_payee_names());
            Model_Payee::Data* payee = Model_Payee::instance().get(transaction_->PAYEEID);
            if (payee)
                cbPayee_->ChangeValue(payee->PAYEENAME);
        }
        else //transfer
        {
            if (cSplit_->IsChecked())
            {
                cSplit_->SetValue(false);
                m_local_splits.clear();
            }

            Model_Category::Data_Set categs = Model_Category::instance().find(Model_Category::CATEGNAME(wxGetTranslation("Transfer")));
            if (!categs.empty())
            {
                transaction_->SUBCATEGID = -1;
                transaction_->CATEGID = categs.begin()->CATEGID;
                bCategory_->SetLabel(Model_Category::full_name(transaction_->CATEGID, -1));
            }

            for (const auto & entry : Model_Account::instance().all_checking_account_names())
                cbPayee_->Append(entry);
            Model_Account::Data *account = Model_Account::instance().get(transaction_->TOACCOUNTID);
            if (account)
                cbPayee_->ChangeValue(account->ACCOUNTNAME);

            cbPayee_->AutoComplete(Model_Account::instance().all_checking_account_names());

            payee_label_->SetLabel(_("To"));
            transaction_->PAYEEID = -1;
            account_label_->SetLabel(_("From"));
            cbAccount_->Enable(true);
        }
        skip_payee_init_ = true;
        cbPayee_->SetEvtHandlerEnabled(true);
    }

    if (!skip_category_init_)
    {
        bool has_split = !this->m_local_splits.empty();
        wxString fullCategoryName;
        bCategory_->UnsetToolTip();
        if (has_split)
        {
            fullCategoryName = _("Categories");
            double total = Model_Splittransaction::instance().get_total(m_local_splits);
            textAmount_->SetValue(total);
        }
        else
        {
            Model_Category::Data *category = Model_Category::instance().get(transaction_->CATEGID);
            Model_Subcategory::Data *subcategory = (Model_Subcategory::instance().get(transaction_->SUBCATEGID));
            fullCategoryName = Model_Category::full_name(category, subcategory);
            if (fullCategoryName.IsEmpty()) fullCategoryName = _("Select Category");
        }

        bCategory_->SetLabel(fullCategoryName);
        cSplit_->SetValue(has_split);
        skip_category_init_ = true;
    }
    textAmount_->Enable(m_local_splits.empty());
    cSplit_->Enable(!m_transfer);

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
            textNotes_->SetFont(this->GetFont());
        }
        skip_notes_init_ = true;
    }
    setTooltips();
}

void mmTransDialog::CreateControls()
{
    wxBoxSizer* box_sizer1 = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* box_sizer2 = new wxBoxSizer(wxVERTICAL);
    box_sizer1->Add(box_sizer2, g_flagsExpand);

    wxStaticBox* static_box = new wxStaticBox(this, wxID_ANY, _("Transaction Details"));
    wxStaticBoxSizer* box_sizer = new wxStaticBoxSizer(static_box, wxVERTICAL);
    box_sizer2->Add(box_sizer, g_flagsExpand);

    wxFlexGridSizer* flex_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    box_sizer->Add(flex_sizer, g_flags);

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

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Date")), g_flags);
    wxBoxSizer* date_sizer = new wxBoxSizer(wxHORIZONTAL);
    flex_sizer->Add(date_sizer);
    date_sizer->Add(dpc_, g_flags);
    date_sizer->Add(spinCtrl_, g_flags);
    date_sizer->Add(itemStaticTextWeek_, g_flags);

    // Status --------------------------------------------
    choiceStatus_ = new wxChoice(this, ID_DIALOG_TRANS_STATUS,
        wxDefaultPosition, wxSize(110, -1));

    for(const auto& i : Model_Checking::all_status())
        choiceStatus_->Append(wxGetTranslation(i), new wxStringClientData(i));

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Status")), g_flags);
    flex_sizer->Add(choiceStatus_, g_flags);

    // Type --------------------------------------------
    transaction_type_ = new wxChoice(this, ID_DIALOG_TRANS_TYPE,
        wxDefaultPosition, wxSize(110, -1));

    for (const auto& i : Model_Checking::all_type())
    {
        if (i != Model_Checking::all_type()[Model_Checking::TRANSFER] || Model_Account::instance().all().size() > 1)
            transaction_type_->Append(wxGetTranslation(i), new wxStringClientData(i));
    }

    cAdvanced_ = new wxCheckBox(this
        , ID_DIALOG_TRANS_ADVANCED_CHECKBOX, _("&Advanced")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );

    wxBoxSizer* typeSizer = new wxBoxSizer(wxHORIZONTAL);

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Type")), g_flags);
    flex_sizer->Add(typeSizer);
    typeSizer->Add(transaction_type_, g_flags);
    typeSizer->Add(cAdvanced_, g_flags);

    // Amount Fields --------------------------------------------
    textAmount_ = new mmTextCtrl( this, ID_DIALOG_TRANS_TEXTAMOUNT, "",
        wxDefaultPosition, wxSize(110, -1),
        wxALIGN_RIGHT|wxTE_PROCESS_ENTER, mmCalcValidator());

    toTextAmount_ = new mmTextCtrl( this, ID_DIALOG_TRANS_TOTEXTAMOUNT, "",
        wxDefaultPosition, wxSize(110, -1),
        wxALIGN_RIGHT|wxTE_PROCESS_ENTER, mmCalcValidator());

    wxBoxSizer* amountSizer = new wxBoxSizer(wxHORIZONTAL);
    amountSizer->Add(textAmount_, g_flags);
    amountSizer->Add(toTextAmount_, g_flags);

    flex_sizer->Add(new wxStaticText( this, wxID_STATIC, _("Amount")), g_flags);
    flex_sizer->Add(amountSizer);

    // Account ---------------------------------------------
    cbAccount_ = new wxComboBox(this, wxID_ANY, "",
        wxDefaultPosition, wxSize(230, -1));

    account_label_ = new wxStaticText(this, wxID_STATIC, _("Account"));
    flex_sizer->Add(account_label_, g_flags);
    flex_sizer->Add(cbAccount_, g_flags);

    // Payee ---------------------------------------------
    payee_label_ = new wxStaticText(this, wxID_STATIC, _("Payee"));

    /*Note: If you want to use EVT_TEXT_ENTER(id,func) to receive wxEVT_COMMAND_TEXT_ENTER events,
      you have to add the wxTE_PROCESS_ENTER window style flag.
      If you create a wxComboBox with the flag wxTE_PROCESS_ENTER, the tab key won't jump to the next control anymore.*/
    cbPayee_ = new wxComboBox(this, ID_DIALOG_TRANS_PAYEECOMBO, ""
        , wxDefaultPosition, wxSize(230, -1));

    flex_sizer->Add(payee_label_, g_flags);
    flex_sizer->Add(cbPayee_, g_flags);

    // Split Category -------------------------------------------
    cSplit_ = new wxCheckBox(this, wxID_FORWARD
        , _("Split"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    cSplit_->SetValue(FALSE);

    flex_sizer->AddSpacer(20);  // Fill empty space.
    flex_sizer->Add(cSplit_, g_flags);

    // Category -------------------------------------------------
    bCategory_ = new wxButton(this, wxID_VIEW_DETAILS, ""
        , wxDefaultPosition, wxSize(230, -1));

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Category")), g_flags);
    flex_sizer->Add(bCategory_, g_flags);

    // Number  ---------------------------------------------
    textNumber_ = new mmTextCtrl(this
        , ID_DIALOG_TRANS_TEXTNUMBER, "", wxDefaultPosition
        , wxDefaultSize, wxTE_PROCESS_ENTER);

    wxButton* bAuto = new wxButton(this
        , ID_DIALOG_TRANS_BUTTONTRANSNUM, "...", wxDefaultPosition
        , wxSize(cbPayee_->GetSize().GetY(), cbPayee_->GetSize().GetY()));
    bAuto->Connect(ID_DIALOG_TRANS_BUTTONTRANSNUM,
        wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(mmTransDialog::OnAutoTransNum), NULL, this);
    bAuto->SetToolTip(_("Populate Transaction #"));

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Number")), g_flags);
    wxBoxSizer* number_sizer = new wxBoxSizer(wxHORIZONTAL);
    flex_sizer->Add(number_sizer, wxSizerFlags(g_flagsExpand).Border(wxALL, 0));
    number_sizer->Add(textNumber_, g_flagsExpand);
    number_sizer->Add(bAuto, g_flags);

    notesTip_ = _("Notes");
    textNotes_ = new mmTextCtrl(this, ID_DIALOG_TRANS_TEXTNOTES, ""
        , wxDefaultPosition, wxSize(-1,80), wxTE_MULTILINE);

    box_sizer->Add(textNotes_, wxSizerFlags(g_flagsExpand).Border(wxLEFT | wxRIGHT | wxBOTTOM, 10));

    /**********************************************************************************************
     Button Panel with OK and Cancel Buttons
    ***********************************************************************************************/
    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    box_sizer1->Add(buttons_panel, wxSizerFlags(g_flags).Center().Border(wxALL, 0));

    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    wxButton* itemButtonOK = new wxButton(buttons_panel, wxID_OK, _("&OK "));
    itemButtonCancel_ = new wxButton(buttons_panel, wxID_CANCEL, _("&Cancel "));

    buttons_sizer->Add(itemButtonOK, wxSizerFlags(g_flags).Border(wxBOTTOM | wxRIGHT, 10));
    buttons_sizer->Add(itemButtonCancel_, wxSizerFlags(g_flags).Border(wxBOTTOM | wxRIGHT, 10));

    itemButtonCancel_->SetFocus();

    buttons_sizer->Realize();
    Center();
    this->SetSizer(box_sizer1);

    cbPayee_->Connect(ID_DIALOG_TRANS_PAYEECOMBO, wxEVT_COMMAND_TEXT_UPDATED,
        wxCommandEventHandler(mmTransDialog::OnAccountOrPayeeUpdated), NULL, this);
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
    {
        accountID_ = account->ACCOUNTID;
    }
    else
    {
        mmMessageAccountInvalid(cbAccount_);
        return false;
    }

    bool bTransfer = (Model_Checking::type(transaction_) == Model_Checking::TRANSFER);

    transaction_->TOTRANSAMOUNT = transaction_->TRANSAMOUNT;
    if (!bTransfer)
    {
        wxString payee_name = cbPayee_->GetValue();
        if (payee_name.IsEmpty())
        {
            mmMessagePayeeInvalid(cbPayee_);
            return false;
        }

        // Get payee string from populated list to address issues with case compare differences between autocomplete and payee list
        int payee_loc = cbPayee_->FindString(payee_name);
        if (payee_loc != wxNOT_FOUND)
            payee_name = cbPayee_->GetString(payee_loc);

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
    else //transfer
    {
        Model_Account::Data *to_account = Model_Account::instance().get(cbPayee_->GetValue());
        if (!to_account || to_account->ACCOUNTID == accountID_ || Model_Account::type(to_account) == Model_Account::INVESTMENT)
        {
            mmMessageAccountInvalid(cbPayee_, true);
            return false;
        }
        transaction_->TOACCOUNTID = to_account->ACCOUNTID;
        if (advancedToTransAmountSet_)
        {
            if (!toTextAmount_->checkValue(transaction_->TOTRANSAMOUNT))
                return false;
        }

        transaction_->PAYEEID = -1;
    }

    if (cSplit_->IsChecked())
    {
        transaction_->TRANSAMOUNT = Model_Splittransaction::instance().get_total(m_local_splits);
        if (transaction_->TRANSAMOUNT < 0.0)
        {
            if (bTransfer) {
                if (transaction_->TRANSAMOUNT < 0)
                    transaction_->TRANSAMOUNT = -transaction_->TRANSAMOUNT;
            }
            else {
                mmShowErrorMessageInvalid(this, _("Amount"));
                return false;
            }
        }

        if (m_local_splits.empty())
        {
            mmMessageCategoryInvalid(bCategory_);
            return false;
        }
    }
    else //non split
    {
        if (!textAmount_->checkValue(transaction_->TRANSAMOUNT))
        {
            return false;
        }

        Model_Category::Data *category = Model_Category::instance().get(transaction_->CATEGID);
        Model_Subcategory::Data *subcategory = Model_Subcategory::instance().get(transaction_->SUBCATEGID);
        if (!category || !(subcategory || transaction_->SUBCATEGID < 0))
        {
            mmMessageCategoryInvalid(bCategory_);
            return false;
        }
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

void mmTransDialog::onFocusChange(wxChildFocusEvent& event)
{
    wxWindow *w = event.GetWindow();
    if ( w )
        object_in_focus_ = w->GetId();

    if (textNotes_->GetValue() == notesTip_ && object_in_focus_ == ID_DIALOG_TRANS_TEXTNOTES)
    {
        textNotes_->SetValue("");
        textNotes_->SetForegroundColour(notesColour_);
    }

    const Model_Currency::Data* currency = Model_Currency::GetBaseCurrency();
    const Model_Account::Data* account = Model_Account::instance().get(cbAccount_->GetValue());
    if (account)
    {
        currency = Model_Account::currency(account);
        accountID_ = account->ACCOUNTID;
        cbAccount_->SetValue(account->ACCOUNTNAME);
    }
    textAmount_->GetDouble(transaction_->TRANSAMOUNT);
    if (transaction_->TRANSAMOUNT) textAmount_->SetValue(transaction_->TRANSAMOUNT, currency);

    if (advancedToTransAmountSet_)
    {
        toTextAmount_->GetDouble(transaction_->TOTRANSAMOUNT);
        if (transaction_->TOTRANSAMOUNT) toTextAmount_->SetValue(transaction_->TOTRANSAMOUNT, currency);
    }

    if (m_transfer)
    {
    }
    else
    {
        Model_Payee::Data * payee = Model_Payee::instance().get(transaction_->PAYEEID);
        if (payee) cbPayee_->ChangeValue(payee->PAYEENAME);
    }

    event.Skip();
}

void mmTransDialog::activateSplitTransactionsDlg()
{
    bool bDeposit = transaction_->TRANSCODE == Model_Checking::all_type()[Model_Checking::DEPOSIT];

    if (transaction_->CATEGID > -1)
    {
        if (!textAmount_->GetDouble(transaction_->TRANSAMOUNT))
            transaction_->TRANSAMOUNT = 0;

        Model_Splittransaction::Data *split = Model_Splittransaction::instance().create();
        split->SPLITTRANSAMOUNT = bDeposit ? transaction_->TRANSAMOUNT : transaction_->TRANSAMOUNT;
        split->CATEGID = transaction_->CATEGID;
        split->SUBCATEGID = transaction_->SUBCATEGID;
        m_local_splits.push_back(*split);
    }
    transaction_->CATEGID = -1;
    transaction_->SUBCATEGID = -1;
    
    SplitTransactionDialog dlg(this, &m_local_splits, transaction_type_->GetSelection(), accountID_);
    dlg.ShowModal();
    transaction_->TRANSAMOUNT = Model_Splittransaction::instance().get_total(m_local_splits);
    skip_category_init_ = false;
}

void mmTransDialog::SetDialogTitle(const wxString& title)
{
    this->SetTitle(title);
}

//** --------------=Event handlers=------------------ **//
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
        m_transfer = Model_Checking::type(transaction_) == Model_Checking::TRANSFER;
        skip_payee_init_ = false;
        dataToControls();
    }
}


void mmTransDialog::OnAccountOrPayeeUpdated(wxCommandEvent& event)
{
    transaction_->PAYEEID = -1;
    if (!m_transfer && event.GetId() == ID_DIALOG_TRANS_PAYEECOMBO)
    {
        const Model_Payee::Data *payee = Model_Payee::instance().get(event.GetString());
        if (payee) transaction_->PAYEEID = payee->PAYEEID;

        // Only for new transactions: if user want to autofill last category used for payee.
        // If this is a Split Transaction, ignore displaying last category for payee
        if (payee && mmIniOptions::instance().transCategorySelectionNone_ != 0
            && !categUpdated_ && m_local_splits.empty() && transaction_id_ == 0)
        {
            // if payee has memory of last category used then display last category for payee
            Model_Category::Data *category = Model_Category::instance().get(payee->CATEGID);
            if (category)
            {
                Model_Subcategory::Data *subcategory = (payee->SUBCATEGID != -1 ? Model_Subcategory::instance().get(payee->SUBCATEGID) : 0);
                wxString fullCategoryName = Model_Category::full_name(category, subcategory);

                transaction_->CATEGID = payee->CATEGID;
                transaction_->SUBCATEGID = payee->SUBCATEGID;
                bCategory_->SetLabel(fullCategoryName);
                wxLogDebug("Category: %s", bCategory_->GetLabel());
            }
        }
    }
    wxChildFocusEvent evt;
    onFocusChange(evt);
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

    if (advancedToTransAmountSet_)
    {
        if (textAmount_->GetValue().Trim().IsEmpty())
        {
            transaction_->TRANSAMOUNT = 1;
            transaction_->TOTRANSAMOUNT = transaction_->TRANSAMOUNT;
            textAmount_->SetValue(1);
        }

        const Model_Account::Data* to_account = Model_Account::instance().get(cbPayee_->GetValue());
        if (to_account)
        {
            const Model_Account::Data* from_account = Model_Account::instance().get(cbAccount_->GetValue());
            const Model_Currency::Data* from_currency = Model_Currency::GetBaseCurrency();
            if (from_account) from_currency = Model_Account::currency(from_account);
            double rateFrom = from_currency->BASECONVRATE;
            const Model_Currency::Data* to_currency = Model_Account::currency(to_account);
            double rateTo = to_currency->BASECONVRATE;
            textAmount_->GetDouble(transaction_->TRANSAMOUNT);
            double toAmount = rateFrom * transaction_->TRANSAMOUNT / rateTo;
            toTextAmount_->SetValue(toAmount, to_account);
            transaction_->TOTRANSAMOUNT = toAmount;
        }
        else
        {
            transaction_->TOTRANSAMOUNT = transaction_->TRANSAMOUNT;
            toTextAmount_->SetValue(textAmount_->GetValue());
        }
    }
    else
    {
        transaction_->TOTRANSAMOUNT = transaction_->TRANSAMOUNT;
        toTextAmount_->SetValue("");
    }

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
        mmCategDialog dlg(this, true, false);
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
    Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();
    Model_Account::Data *account = Model_Account::instance().get(accountID_);
    if (account) currency = Model_Account::currency(account);

    if (object_in_focus_ == textAmount_->GetId())
    {
        textAmount_->Calculate(currency);
    }
    else if (object_in_focus_ == toTextAmount_->GetId())
    {
        toTextAmount_->Calculate(currency);
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

    transaction_->ACCOUNTID = accountID_;
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
#ifndef __WXMAC__
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
#endif

    EndModal(wxID_CANCEL);
}

void mmTransDialog::setTooltips()
{
    textAmount_->UnsetToolTip();
    toTextAmount_->UnsetToolTip();
    cbAccount_->UnsetToolTip();
    cbPayee_->UnsetToolTip();
    bCategory_->UnsetToolTip();
    
    if (m_transfer)
    {
        cbAccount_->SetToolTip(_("Specify account the money is taken from"));
        cbPayee_->SetToolTip(_("Specify account the money is moved to"));
        textAmount_->SetToolTip(_("Specify the transfer amount in the From Account."));
        
        if (advancedToTransAmountSet_)
        {
            toTextAmount_->SetToolTip(_("Specify the transfer amount in the To Account"));
        }
    }
    else
    {
        textAmount_->SetToolTip(_("Specify the amount for this transaction"));
        cbAccount_->SetToolTip(_("Specify account for the transaction"));
        if (Model_Checking::WITHDRAWAL == Model_Checking::type(transaction_))
            cbPayee_->SetToolTip(_("Specify to whom the transaction is going to"));
        else
            cbPayee_->SetToolTip(_("Specify where the transaction is coming from"));
    }

    if (this->m_local_splits.empty())
    {
        bCategory_->SetToolTip(_("Specify the category for this transaction"));
    }
    else
    {
        bCategory_->SetToolTip(_("Specify categories for this transaction"));
    }
    
    //Permanent
    dpc_->SetToolTip(_("Specify the date of the transaction"));
    spinCtrl_->SetToolTip(_("Retard or advance the date of the transaction"));
    choiceStatus_->SetToolTip(_("Specify the status for the transaction"));
    transaction_type_->SetToolTip(_("Specify the type of transactions to be created."));
    cSplit_->SetToolTip(_("Use split Categories"));
    textNumber_->SetToolTip(_("Specify any associated check number or transaction number"));
    textNotes_->SetToolTip(_("Specify any text notes you want to add to this transaction."));
    cAdvanced_->SetToolTip(_("Allows the setting of different amounts in the FROM and TO accounts."));

}

void mmTransDialog::OnQuit(wxCloseEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}
