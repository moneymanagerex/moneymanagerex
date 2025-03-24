/*******************************************************
Copyright (C) 2014 Nikolay Akimov
Copyright (C) 2015 Stefano Giorgio
Copyright (C) 2022  Mark Whalley (mark@ipx.co.uk)

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

#include "usertransactionpanel.h"
#include "option.h"
#include "validators.h"
#include "images_list.h"
#include "util.h"

#include "mmSimpleDialogs.h"
#include "payeedialog.h"
#include "categdialog.h"
#include "attachmentdialog.h"
#include "splittransactionsdialog.h"

#include "model/allmodel.h"

/*******************************************************/
wxBEGIN_EVENT_TABLE(UserTransactionPanel, wxPanel)
EVT_BUTTON(ID_TRANS_ACCOUNT_BUTTON, UserTransactionPanel::OnTransAccountButton)
EVT_BUTTON(ID_TRANS_PAYEE_BUTTON, UserTransactionPanel::OnTransPayeeButton)
EVT_BUTTON(mmID_CATEGORY_SPLIT, UserTransactionPanel::OnCategs)
EVT_COMBOBOX(ID_TRANS_CATEGORY_COMBOBOX, UserTransactionPanel::OnTransCategoryCombobox)
EVT_MENU(wxID_ANY, UserTransactionPanel::onSelectedNote)
EVT_BUTTON(wxID_FILE, UserTransactionPanel::OnAttachments)
wxEND_EVENT_TABLE()
/*******************************************************/

UserTransactionPanel::UserTransactionPanel()
{
}

UserTransactionPanel::UserTransactionPanel(wxWindow *parent
    , Model_Checking::Data* checking_entry
    , bool enable_revalue
    , wxWindowID win_id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name)
    : m_checking_entry(checking_entry)
    , m_enable_revalue(enable_revalue)
{
    if (m_checking_entry)
    {
        for (const auto& split: Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(m_checking_entry->TRANSID)))
        {
            wxArrayInt64 tags;
            for (const auto& tag : Model_Taglink::instance().find(Model_Taglink::REFTYPE(Model_Attachment::REFTYPE_NAME_TRANSACTIONSPLIT), Model_Taglink::REFID(split.SPLITTRANSID)))
                tags.push_back(tag.TAGID);
            m_local_splits.push_back({split.CATEGID, split.SPLITTRANSAMOUNT, tags, split.NOTES});
        }
    }

    Create(parent, win_id, pos, size, style, name);
    DataToControls();
    Model_Usage::instance().pageview(this);
}

UserTransactionPanel::~UserTransactionPanel()
{
}

bool UserTransactionPanel::Create(wxWindow* parent
    , wxWindowID win_id
    , const wxPoint &pos
    , const wxSize &size
    , long style
    , const wxString &name)
{
    wxPanel::Create(parent, win_id, pos, size, style, name);

    const wxSize std_size(230, -1);
    const wxSize std_half_size(110, -1);

    wxBoxSizer* main_panel_sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(main_panel_sizer);

    wxFlexGridSizer* transPanelSizer = new wxFlexGridSizer(0, 2, 0, 0);
    main_panel_sizer->Add(transPanelSizer);

    // Trans Date --------------------------------------------
    m_date_selector = new mmDatePickerCtrl(this, ID_TRANS_DATE_SELECTOR);
    mmToolTip(m_date_selector, _t("Specify the date of the transaction"));

    wxBoxSizer* date_sizer = new wxBoxSizer(wxHORIZONTAL);
    date_sizer->Add(m_date_selector->mmGetLayout());

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _t("Date")), g_flagsH);
    transPanelSizer->Add(date_sizer);

    // Account ------------------------------------------------
    transPanelSizer->Add(new wxStaticText(this, ID_TRANS_ACCOUNT_BUTTON_TEXT, _t("Account")), g_flagsH);
    m_account = new wxButton(this, ID_TRANS_ACCOUNT_BUTTON, _t("Select Account")
        , wxDefaultPosition, std_size);
    mmToolTip(m_account, _t("Specify the associated Account that will contain this transaction"));
    transPanelSizer->Add(m_account, g_flagsH);

    // Type --------------------------------------------
    m_type_selector = new wxChoice(this, ID_TRANS_TYPE, wxDefaultPosition, std_half_size);
    for (int i = 0; i < Model_Checking::TYPE_ID_size; ++i)
    {
        if (i != Model_Checking::TYPE_ID_TRANSFER || this->m_enable_revalue)
        {
            wxString type = Model_Checking::trade_type_name(i);
            m_type_selector->Append(wxGetTranslation(type), new wxStringClientData(type));
        }
    }

    m_type_selector->SetSelection(Model_Checking::TYPE_ID_WITHDRAWAL);
    mmToolTip(m_type_selector, _t("Withdraw funds from or deposit funds to this Account."));

    m_transfer = new wxCheckBox(this, ID_TRANS_TRANSFER, _t("&Transfer")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_transfer->Hide();
    CheckingType(Model_Translink::AS_INCOME_EXPENSE);
    mmToolTip(m_transfer, _t("Funds transfer from/to this account. Uncheck to set as Expense/Income."));

    wxBoxSizer* type_sizer = new wxBoxSizer(wxHORIZONTAL);
    type_sizer->Add(m_type_selector, g_flagsH);
    type_sizer->Add(m_transfer, g_flagsH);

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _t("Type")), g_flagsH);
    transPanelSizer->Add(type_sizer);

    // Amount ------------------------------------------------
    wxStaticText* entered_amount_text = new wxStaticText(this, wxID_STATIC, _t("Amount"));
    m_entered_amount = new mmTextCtrl(this, ID_TRANS_ENTERED_AMOUNT, ""
        , wxDefaultPosition, std_half_size, wxALIGN_RIGHT | wxTE_PROCESS_ENTER
        , mmCalcValidator());
    mmToolTip(m_entered_amount, _t("Specify the amount for this transaction"));

    Model_Currency::Data* currency = Model_Currency::GetBaseCurrency();
    if (m_account_id > 0)
    {
        currency = Model_Account::currency(Model_Account::instance().get(m_account_id));
    }
    m_trans_currency = new wxButton(this, ID_TRANS_CURRENCY_BUTTON, currency->CURRENCY_SYMBOL
        , wxDefaultPosition, std_half_size);
    mmToolTip(m_trans_currency, _t("Currency used for this transaction."));

    wxBoxSizer* entered_amount_sizer = new wxBoxSizer(wxHORIZONTAL);
    entered_amount_sizer->Add(m_entered_amount, g_flagsH);
    entered_amount_sizer->Add(m_trans_currency, g_flagsH);

    transPanelSizer->Add(entered_amount_text, g_flagsH);
    transPanelSizer->Add(entered_amount_sizer, wxSizerFlags(g_flagsExpand).Border(0));

    // Status --------------------------------------------
    m_status_selector = new wxChoice(this, ID_TRANS_STATUS_SELECTOR
        , wxDefaultPosition, std_half_size);

    for (int i = 0; i < Model_Checking::STATUS_ID_size; ++i)
    {
        wxString status = Model_Checking::status_name(i);
        m_status_selector->Append(wxGetTranslation(status), new wxStringClientData(status));
    }

    m_status_selector->SetSelection(Model_Checking::STATUS_ID_RECONCILED);
    mmToolTip(m_status_selector, _t("Specify the status for this transaction"));

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _t("Status")), g_flagsH);
    transPanelSizer->Add(m_status_selector, g_flagsH);

    // Payee ------------------------------------------------
    wxStaticText* payee_button_text = new wxStaticText(this, ID_TRANS_PAYEE_BUTTON_TEXT, _t("Payee"));
    m_payee = new wxButton(this, ID_TRANS_PAYEE_BUTTON, _t("Select Payee"), wxDefaultPosition, std_size, 0);
    mmToolTip(m_payee, _t("Specify a person, Company or Organisation for this transaction."));
    transPanelSizer->Add(payee_button_text, g_flagsH);
    transPanelSizer->Add(m_payee, g_flagsH);

    // Category ---------------------------------------------
    wxStaticText* category_button_text = new wxStaticText(this, wxID_STATIC, _t("Category"));
    transPanelSizer->Add(category_button_text, g_flagsH);

    m_category = new mmComboBoxCategory(this, ID_TRANS_CATEGORY_COMBOBOX, std_half_size, m_category_id, true);
    mmToolTip(m_category, _t("Specify the category for this transaction"));

    wxBitmapButton* bSplit_ = new wxBitmapButton(this, mmID_CATEGORY_SPLIT, mmBitmapBundle(png::NEW_TRX, mmBitmapButtonSize));
    bSplit_->Hide(); // TODO
    wxBoxSizer* split_sizer = new wxBoxSizer(wxHORIZONTAL);
    split_sizer->Add(m_category, g_flagsH);
    split_sizer->Add(bSplit_, g_flagsH);

    transPanelSizer->Add(split_sizer, wxSizerFlags(g_flagsExpand).Border(0));

    // Number ---------------------------------------------
    m_entered_number = new wxTextCtrl(this, ID_TRANS_ENTERED_NUMBER, ""
        , wxDefaultPosition, std_size);
    mmToolTip(m_entered_number, _t("Specify a transaction code or associated check details"));

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _t("Number")), g_flagsH);
    transPanelSizer->Add(m_entered_number, g_flagsH);

    // Notes ---------------------------------------------
    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _t("Notes")), g_flagsH);

    // Attachment ---------------------------------------------
    m_attachment = new wxBitmapButton(this, wxID_FILE, mmBitmapBundle(png::CLIP, mmBitmapButtonSize));
    mmToolTip(m_attachment, _t("Manage transaction attachments"));
    //TODO: m_attachment Enable/disable
    //m_attachment->Enable(false);

    // Frequent Notes ---------------------------------------------
    frequent_notes = new wxButton(this, ID_TRANS_FREQUENT_NOTES, "..."
        , wxDefaultPosition, wxSize(m_attachment->GetSize().GetX(), -1));
    mmToolTip(frequent_notes, _t("Select one of the frequently used notes"));
    frequent_notes->Connect(ID_TRANS_FREQUENT_NOTES
        , wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(UserTransactionPanel::OnFrequentNotes), nullptr, this);

    wxBoxSizer* right_align_sizer = new wxBoxSizer(wxHORIZONTAL);
    right_align_sizer->Add(m_attachment, g_flagsH);
    right_align_sizer->Add(frequent_notes, g_flagsH);

    m_entered_notes = new wxTextCtrl(this, ID_TRANS_ENTERED_NOTES, "", wxDefaultPosition, wxSize(220, 96), wxTE_MULTILINE);
    mmToolTip(m_entered_notes, _t("Specify any text notes you want to add to this transaction."));

    transPanelSizer->Add(right_align_sizer, wxSizerFlags(g_flagsH).Align(wxALIGN_RIGHT).Border(wxALL, 0));
    main_panel_sizer->Add(m_entered_notes, wxSizerFlags(g_flagsExpand).Border(wxTOP, 5));

    return true;
}

void UserTransactionPanel::DataToControls()
{
    if (!m_checking_entry) return;

    wxDateTime trans_date;
    trans_date.ParseDateTime(m_checking_entry->TRANSDATE) || trans_date.ParseDate(m_checking_entry->TRANSDATE);
    TransactionDate(trans_date);

    m_transaction_id = m_checking_entry->TRANSID;
    m_account_id = m_checking_entry->ACCOUNTID;
    m_account->SetLabelText(Model_Account::get_account_name(m_account_id));
    m_type_selector->SetSelection(Model_Checking::type_id(m_checking_entry->TRANSCODE));

    if (m_account_id > 0)
    {
        Model_Currency::Data* currency = Model_Account::currency(Model_Account::instance().get(m_account_id));
        m_trans_currency->SetLabelText(currency->CURRENCY_SYMBOL);
        m_entered_amount->SetCurrency(currency);
    }

    SetTransactionValue(m_checking_entry->TRANSAMOUNT);
    m_status_selector->SetSelection(Model_Checking::status_id(m_checking_entry->STATUS));

    m_payee_id = m_checking_entry->PAYEEID;
    m_payee->SetLabelText(Model_Payee::get_payee_name(m_payee_id));

    m_category_id = m_checking_entry->CATEGID;
    m_category->SetLabelText(Model_Category::full_name(m_category_id));

    m_entered_number->SetValue(m_checking_entry->TRANSACTIONNUMBER);
    m_entered_notes->SetValue(m_checking_entry->NOTES);

    bool has_split = !(m_local_splits.size() <= 1);
    if (has_split)
    {
        m_category->Enable(!has_split);
        m_category->SetLabelText(_t("Split Transaction"));

        SetTransactionValue(Model_Splittransaction::get_total(m_local_splits));
    }

    if (!m_checking_entry->DELETEDTIME.IsEmpty())
    {
        m_date_selector->Enable(false);
        m_account->Enable(false);
        m_type_selector->Enable(false);
        m_status_selector->Enable(false);
        m_transfer->Enable(false);
        m_entered_amount->Enable(false);
        m_trans_currency->Enable(false);
        m_payee->Enable(false);
        m_category->Enable(false);
        m_entered_number->Enable(false);
        m_attachment->Enable(false);
        m_entered_notes->Enable(false);
        frequent_notes->Enable(false);
    }
}

void UserTransactionPanel::SetLastPayeeAndCategory(const int64 account_id)
{
    if (Option::instance().getTransPayeeNone() == Option::LASTUSED)
    {
        Model_Checking::Data_Set trans_list = Model_Checking::instance().find(Model_Checking::ACCOUNTID(account_id), Model_Checking::TRANSCODE(Model_Checking::TYPE_ID_TRANSFER, NOT_EQUAL));
        if (!trans_list.empty())
        {
            int last_trans_pos = trans_list.size() - 1;

            Model_Payee::Data* last_payee = Model_Payee::instance().get(trans_list.at(last_trans_pos).PAYEEID);
            if (last_payee)
            {
                m_payee->SetLabelText(last_payee->PAYEENAME);
                m_payee_id = last_payee->PAYEEID;
                if ((Option::instance().getTransCategoryNone() == Option::LASTUSED)
                    && (!Model_Category::is_hidden(last_payee->CATEGID)))
                {
                    m_category_id = last_payee->CATEGID;
                    m_category->SetLabelText(Model_Category::full_name(last_payee->CATEGID));
                }
            }
        }
    }
}

void UserTransactionPanel::OnTransAccountButton(wxCommandEvent& WXUNUSED(event))
{
    const auto& accounts = Model_Account::instance().all_checking_account_names();
    mmSingleChoiceDialog scd(this
        , _t("Select the required account")
        , _t("Account Selection")
        , accounts);

    if (scd.ShowModal() == wxID_OK)
    {
        SetTransactionAccount(scd.GetStringSelection());
    }
}

void UserTransactionPanel::OnTransPayeeButton(wxCommandEvent& WXUNUSED(event))
{
    mmPayeeDialog dlg(this, true);
    if (dlg.ShowModal() == wxID_OK)
    {
        m_payee_id = dlg.getPayeeId();
        Model_Payee::Data* payee = Model_Payee::instance().get(m_payee_id);
        if (payee)
        {
            m_payee->SetLabelText(payee->PAYEENAME);

            // Only for new transactions: if user want to autofill last category used for payee and category has not been set.
            if ((Option::instance().getTransCategoryNone() == Option::LASTUSED) && (m_category_id < 0) && (m_subcategory_id < 0)
                        && (!Model_Category::is_hidden(payee->CATEGID)))
            {
                if (payee->CATEGID > 0)
                {
                    m_category_id = payee->CATEGID;
                    m_category->SetLabelText(Model_Category::full_name(m_category_id));
                }
            }
        }
    }
}

void UserTransactionPanel::OnTransCategoryCombobox(wxCommandEvent& WXUNUSED(event))
{
    if (!m_category->GetValue().empty())
    {
        m_category_id = m_category->mmGetCategoryId();
        return;
    }
    mmCategDialog dlg(this, true, m_category_id);
    if (dlg.ShowModal() == wxID_OK)
    {
        m_category_id = dlg.getCategId();
        m_category->SetLabelText(Model_Category::full_name(m_category_id));
    }
}

void UserTransactionPanel::OnFrequentNotes(wxCommandEvent& WXUNUSED(event))
{
    Model_Checking::getFrequentUsedNotes(m_frequent_notes);
    wxMenu menu;
    int id = wxID_HIGHEST;
    for (const auto& entry : m_frequent_notes)
    {
        const wxString& label = entry.Mid(0, 30) + (entry.size() > 30 ? "..." : "");
        menu.Append(++id, label);
    }

    if (!m_frequent_notes.empty())
        PopupMenu(&menu);
}

void UserTransactionPanel::onSelectedNote(wxCommandEvent& event)
{
    int i = event.GetId() - wxID_HIGHEST;
    if (i > 0 && i <= static_cast<int>(m_frequent_notes.size()))
        m_entered_notes->ChangeValue(m_frequent_notes[i - 1]);
}

void UserTransactionPanel::OnAttachments(wxCommandEvent& WXUNUSED(event))
{
    const wxString& RefType = Model_Attachment::REFTYPE_NAME_TRANSACTION;
    int64 RefId = m_transaction_id;

    if (RefId < 0)
        RefId = 0;

    mmAttachmentDialog dlg(this, RefType, RefId);
    dlg.ShowModal();
}

bool UserTransactionPanel::ValidCheckingAccountEntry()
{
    return (m_account_id != -1) && (m_payee_id != -1) && (m_category_id != -1) && (!m_entered_amount->GetValue().IsEmpty());
}

wxDateTime UserTransactionPanel::TransactionDate()
{
    return m_date_selector->GetValue();
}

void UserTransactionPanel::TransactionDate(const wxDateTime& trans_date)
{
    m_date_selector->SetValue(trans_date);
}

void UserTransactionPanel::SetTransactionValue(const double& trans_value, bool fixed_value)
{
    m_entered_amount->SetValue(trans_value);
    if (fixed_value) {
        m_entered_amount->Enable(false);
    }
}

void UserTransactionPanel::SetTransactionNumber(const wxString& trans_number)
{
    m_entered_number->SetValue(trans_number);
}

void UserTransactionPanel::SetTransactionStatus(const int trans_status_enum)
{
    m_status_selector->SetSelection(trans_status_enum);
}

void UserTransactionPanel::SetTransactionPayee(const int64 payeeid)
{
    m_payee_id = payeeid;
    Model_Payee::Data* payee = Model_Payee::instance().get(m_payee_id);
    if (payee)
        m_payee->SetLabelText(payee->PAYEENAME);
}

void UserTransactionPanel::SetTransactionCategory(const int64 categid)
{
    m_category_id = categid;
    m_category->SetLabelText(Model_Category::full_name(m_category_id));
}

void UserTransactionPanel::SetTransactionAccount(const wxString& trans_account)
{
    Model_Account::Data* account = Model_Account::instance().get(trans_account);
    if (account)
    {
        m_account->SetLabelText(account->ACCOUNTNAME);
        m_account_id = account->ACCOUNTID;
        SetLastPayeeAndCategory(m_account_id);
        Model_Currency::Data* currency = Model_Currency::instance().get(account->CURRENCYID);
        m_entered_amount->SetCurrency(currency);
        m_trans_currency->SetLabelText(currency->CURRENCY_SYMBOL);
    }
}

Model_Translink::CHECKING_TYPE UserTransactionPanel::CheckingType()
{
    if (m_transfer->IsChecked())
        return Model_Translink::AS_TRANSFER;
    else
        return Model_Translink::AS_INCOME_EXPENSE;
}

void UserTransactionPanel::CheckingType(Model_Translink::CHECKING_TYPE ct)
{
    m_transfer->SetValue(false);
    if (ct == Model_Translink::AS_TRANSFER) {
        m_transfer->SetValue(true);
    }
}

int64 UserTransactionPanel::SaveChecking()
{
    double initial_amount = 0;
    m_entered_amount->checkValue(initial_amount);
    
    const Model_Account::Data* account = Model_Account::instance().get(m_account_id);
    wxDateTime trxDate = m_date_selector->GetValue();
    if (trxDate.FormatISODate() < account->INITIALDATE)
    {
        mmErrorDialogs::ToolTip4Object(m_account, _t("The opening date for the account is later than the date of this transaction"), _t("Invalid Date"));
        return -1;
    }  

    if (!m_checking_entry) {
        m_checking_entry = Model_Checking::instance().create();
    }

    m_checking_entry->ACCOUNTID = m_account_id;
    m_checking_entry->TOACCOUNTID = TransactionType() == Model_Checking::TYPE_ID_TRANSFER ? m_account_id : -1; // Self Transfer as Revaluation

    m_checking_entry->PAYEEID = m_payee_id;
    m_checking_entry->TRANSCODE = Model_Checking::type_name(TransactionType());
    m_checking_entry->TRANSAMOUNT = initial_amount;
    m_checking_entry->STATUS = m_status_selector->GetStringSelection().Mid(0, 1);
    m_checking_entry->TRANSACTIONNUMBER = m_entered_number->GetValue();
    m_checking_entry->NOTES = m_entered_notes->GetValue();
    m_checking_entry->CATEGID = m_category_id;
    m_checking_entry->TRANSDATE = trxDate.FormatISOCombined();
    m_checking_entry->FOLLOWUPID = 0;
    m_checking_entry->TOTRANSAMOUNT = m_checking_entry->TRANSAMOUNT;
    m_checking_entry->COLOR = 0;

    return Model_Checking::instance().save(m_checking_entry);
}

int UserTransactionPanel::TransactionType()
{
    return m_type_selector->GetSelection();
}


void UserTransactionPanel::OnCategs(wxCommandEvent& WXUNUSED(event))
{
    if (m_local_splits.empty() && m_category->mmIsValid())
    {
        Split s;

        m_entered_amount->GetDouble(s.SPLITTRANSAMOUNT);

        s.CATEGID = m_category->mmGetCategoryId();
        s.NOTES = m_entered_notes->GetValue();
        m_local_splits.push_back(s);
    }

    mmSplitTransactionDialog dlg(this, m_local_splits, m_account_id);

    if (dlg.ShowModal() == wxID_OK)
    {
        m_local_splits = dlg.mmGetResult();

        if (m_local_splits.size() == 1)
        {
            m_category->SetLabelText(Model_Category::full_name(m_local_splits[0].CATEGID));
            m_entered_amount->SetValue(m_local_splits[0].SPLITTRANSAMOUNT);
            m_entered_notes->SetValue(m_local_splits[0].NOTES);

            m_local_splits.clear();
        }

        if (m_local_splits.empty())
        {
            m_category->Enable(true);
        }
        else
        {
            m_entered_amount->SetValue(Model_Splittransaction::get_total(m_local_splits));

            m_category->Enable(false);
            m_category->SetLabelText(_t("Split Transaction"));
        }
    }
}

