/*******************************************************
Copyright (C) 2014 Nikolay Akimov
Copyright (C) 2015 Stefano Giorgio

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

#include "maincurrencydialog.h"
#include "mmSimpleDialogs.h"
#include "payeedialog.h"
#include "categdialog.h"
#include "attachmentdialog.h"
#include "mmTextCtrl.h"
#include "constants.h"
#include <wx/spinbutt.h>

#include "Model_Checking.h"
#include "Model_Payee.h"
#include "Model_Usage.h"

/*******************************************************/
wxBEGIN_EVENT_TABLE(UserTransactionPanel, wxPanel)
    EVT_SPIN_UP(ID_TRANS_DATE_CONTROLLER, UserTransactionPanel::OnDateSelectorForward)
    EVT_SPIN_DOWN(ID_TRANS_DATE_CONTROLLER, UserTransactionPanel::OnDateSelectorBackward)
    EVT_BUTTON(ID_TRANS_ACCOUNT_BUTTON, UserTransactionPanel::OnTransAccountButton)
    EVT_BUTTON(ID_TRANS_PAYEE_BUTTON, UserTransactionPanel::OnTransPayeeButton)
    EVT_BUTTON(ID_TRANS_CATEGORY_BUTTON, UserTransactionPanel::OnTransCategoryButton)
    EVT_CHOICE(wxID_VIEW_DETAILS, UserTransactionPanel::OnTypeChoice)
    EVT_MENU(wxID_ANY, UserTransactionPanel::onSelectedNote)
    EVT_BUTTON(wxID_FILE, UserTransactionPanel::OnAttachments)
wxEND_EVENT_TABLE()
/*******************************************************/

UserTransactionPanel::UserTransactionPanel()
{
}

UserTransactionPanel::UserTransactionPanel(wxWindow *parent
    , Model_Checking::Data* checking_entry
    , wxWindowID win_id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name)
    : m_checking_entry(checking_entry)
    , m_transaction_id(-1)
    , m_account_id(-1)
    , m_payee_id(-1)
    , m_category_id(-1)
    , m_subcategory_id(-1)
    , m_trans_value(0.0)
    , m_commission(0.0)
{
    wxPanel::Create(parent, win_id, pos, size, style, name);
    wxDateTime start = wxDateTime::UNow();
    Create();
    DataToControls();
    Model_Usage::instance().pageview(this, (wxDateTime::UNow() - start).GetMilliseconds().ToLong());
}

UserTransactionPanel::~UserTransactionPanel()
{
}

void UserTransactionPanel::Create()
{
    // Control properties according to system
    int spinCtrlDirection = wxSP_VERTICAL;
    int interval = 0;
#ifdef __WXMSW__
    wxSize spinCtrlSize = wxSize(18, 22);
    interval = 4;
#else
    wxSize spinCtrlSize = wxSize(16, -1);
#endif

    const wxSize std_size(230, -1);
    const wxSize std_half_size(110, -1);

    wxBoxSizer* main_panel_sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(main_panel_sizer);

    wxFlexGridSizer* transPanelSizer = new wxFlexGridSizer(0, 2, 0, 0);
    main_panel_sizer->Add(transPanelSizer);

    // Trans Date --------------------------------------------
    m_date_selector = new wxDatePickerCtrl(this, ID_TRANS_DATE_SELECTOR, wxDefaultDateTime
        , wxDefaultPosition, std_half_size, wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    m_date_selector->SetToolTip(_("Specify the date of the transaction"));

    m_date_controller = new wxSpinButton(this, ID_TRANS_DATE_CONTROLLER
        , wxDefaultPosition, spinCtrlSize, spinCtrlDirection | wxSP_ARROW_KEYS | wxSP_WRAP);
    m_date_controller->SetToolTip(_("Retard or advance the date of the transaction"));

    wxBoxSizer* date_sizer = new wxBoxSizer(wxHORIZONTAL);
    date_sizer->Add(m_date_selector, g_flagsH);
    date_sizer->Add(m_date_controller, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT, interval);

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Date")), g_flagsH);
    transPanelSizer->Add(date_sizer);

    // Account ------------------------------------------------
    transPanelSizer->Add(new wxStaticText(this, ID_TRANS_ACCOUNT_BUTTON_TEXT, _("Account")), g_flagsH);
    m_account = new wxButton(this, ID_TRANS_ACCOUNT_BUTTON, _("Select Account")
        , wxDefaultPosition, std_size);
    m_account->SetToolTip(_("Please select the Account that will be associated with this transaction"));
    transPanelSizer->Add(m_account, g_flagsH);

    // Type --------------------------------------------
    m_type_selector = new wxChoice(this, wxID_VIEW_DETAILS, wxDefaultPosition, std_half_size);
    for (const auto& i : Model_Checking::all_type())
    {
        if (i != Model_Checking::all_type()[Model_Checking::TRANSFER])
            m_type_selector->Append(wxGetTranslation(i), new wxStringClientData(i));
    }

    m_type_selector->SetSelection(Model_Checking::WITHDRAWAL);
    m_type_selector->SetToolTip(_("Withdraw funds from or deposit funds to this Account."));

    m_transfer = new wxCheckBox(this, ID_TRANS_TRANSFER, _("&Transfer")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    CheckingType(Model_Translink::AS_TRANSFER);
    m_transfer->SetToolTip(_("Funds transfer from/to this account. Uncheck to set as Expense/Income."));

    wxBoxSizer* type_sizer = new wxBoxSizer(wxHORIZONTAL);
    type_sizer->Add(m_type_selector, g_flagsH);
    type_sizer->Add(m_transfer, g_flagsH);

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Type")), g_flagsH);
    transPanelSizer->Add(type_sizer);

    // Amount ------------------------------------------------
    wxStaticText* entered_amount_text = new wxStaticText(this, wxID_STATIC, _("Amount"));
    m_entered_amount = new mmTextCtrl(this, ID_TRANS_ENTERED_AMOUNT, ""
        , wxDefaultPosition, std_half_size, wxALIGN_RIGHT | wxTE_PROCESS_ENTER
        , mmCalcValidator());
    m_entered_amount->SetToolTip(_("Specify the amount for this transaction"));
    m_entered_amount->Connect(ID_TRANS_ENTERED_AMOUNT
        , wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(UserTransactionPanel::OnEnteredText), nullptr, this);

    Model_Currency::Data* currency = Model_Currency::GetBaseCurrency();
    if (m_account_id > 0)
    {
        currency = Model_Account::currency(Model_Account::instance().get(m_account_id));
    }
    m_trans_currency = new wxStaticText(this, wxID_ANY, currency->CURRENCY_SYMBOL
        , wxDefaultPosition, std_half_size, wxBORDER_SUNKEN|wxALIGN_CENTRE);
    m_trans_currency->SetToolTip(_("Currency used for this transaction."));

    wxBoxSizer* entered_amount_sizer = new wxBoxSizer(wxHORIZONTAL);
    entered_amount_sizer->Add(m_entered_amount, g_flagsH);
    entered_amount_sizer->Add(m_trans_currency, g_flagsH);

    transPanelSizer->Add(entered_amount_text, g_flagsH);
    transPanelSizer->Add(entered_amount_sizer);

    // Status --------------------------------------------
    m_status_selector = new wxChoice(this, ID_TRANS_STATUS_SELECTOR
        , wxDefaultPosition, std_half_size);

    for (const auto& i : Model_Checking::all_status())
    {
        m_status_selector->Append(wxGetTranslation(i), new wxStringClientData(i));
    }

    m_status_selector->SetSelection(Option::instance().TransStatusReconciled());
    m_status_selector->SetToolTip(_("Specify the status for this transaction"));

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Status")), g_flagsH);
    transPanelSizer->Add(m_status_selector, g_flagsH);

    // Payee ------------------------------------------------
    wxStaticText* payee_button_text = new wxStaticText(this, ID_TRANS_PAYEE_BUTTON_TEXT, _("Payee"));
    m_payee = new wxButton(this, ID_TRANS_PAYEE_BUTTON, _("Select Payee"), wxDefaultPosition, std_size, 0);
    m_payee->SetToolTip(_("Specify a person, Company or Organisation for this transaction."));
    transPanelSizer->Add(payee_button_text, g_flagsH);
    transPanelSizer->Add(m_payee, g_flagsH);

    // Category ---------------------------------------------
    wxStaticText* category_button_text = new wxStaticText(this, wxID_STATIC, _("Category"));
    m_category = new wxButton(this, ID_TRANS_CATEGORY_BUTTON, _("Select Category")
        , wxDefaultPosition, std_size, 0);
    m_category->SetToolTip(_("Specify the category for this transaction"));

    transPanelSizer->Add(category_button_text, g_flagsH);
    transPanelSizer->Add(m_category, g_flagsH);

    // Number ---------------------------------------------
    m_entered_number = new wxTextCtrl(this, ID_TRANS_ENTERED_NUMBER, ""
        , wxDefaultPosition, std_size);
    m_entered_number->SetToolTip(_("Specify a transaction code or associated check details"));

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Number")), g_flagsH);
    transPanelSizer->Add(m_entered_number, g_flagsH);

    // Notes ---------------------------------------------
    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Notes")), g_flagsH);

    // Attachment ---------------------------------------------
    m_attachment = new wxBitmapButton(this, wxID_FILE, mmBitmap(png::CLIP));
    m_attachment->SetToolTip(_("Organize attachments of this transaction"));
    //TODO: m_attachment Enable/disable
    //m_attachment->Enable(false);

    // Frequent Notes ---------------------------------------------
    wxButton* frequent_notes = new wxButton(this, ID_TRANS_FREQUENT_NOTES, "..."
        , wxDefaultPosition, wxSize(m_attachment->GetSize().GetX(), -1));
    frequent_notes->SetToolTip(_("Select one of the frequently used notes"));
    frequent_notes->Connect(ID_TRANS_FREQUENT_NOTES
        , wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(UserTransactionPanel::OnFrequentNotes), nullptr, this);

    wxBoxSizer* right_align_sizer = new wxBoxSizer(wxHORIZONTAL);
    right_align_sizer->Add(m_attachment, g_flagsH);
    right_align_sizer->Add(frequent_notes, g_flagsH);

    m_entered_notes = new wxTextCtrl(this, ID_TRANS_ENTERED_NOTES, "", wxDefaultPosition, wxSize(220, 96), wxTE_MULTILINE);
    m_entered_notes->SetToolTip(_("Specify any text notes you want to add to this transaction."));

    transPanelSizer->Add(right_align_sizer, wxSizerFlags(g_flagsH).Align(wxALIGN_RIGHT).Border(wxALL, 0));
    main_panel_sizer->Add(m_entered_notes, wxSizerFlags(g_flagsExpand).Border(wxTOP, 5));
}

void UserTransactionPanel::DataToControls()
{
    if (!m_checking_entry) return;
        
    wxDateTime trans_date;
    trans_date.ParseDate(m_checking_entry->TRANSDATE);
    TransactionDate(trans_date);

    m_transaction_id = m_checking_entry->TRANSID;
    m_account_id = m_checking_entry->ACCOUNTID;
    m_account->SetLabelText(Model_Account::get_account_name(m_account_id));
    m_type_selector->SetSelection(Model_Checking::type(m_checking_entry->TRANSCODE));

    SetTransactionValue(m_checking_entry->TRANSAMOUNT, 0);
    m_status_selector->SetSelection(Model_Checking::status(m_checking_entry->STATUS));

    m_payee_id = m_checking_entry->PAYEEID;
    m_payee->SetLabelText(Model_Payee::get_payee_name(m_payee_id));
    
    m_category_id = m_checking_entry->CATEGID;
    m_subcategory_id = m_checking_entry->SUBCATEGID;
    m_category->SetLabelText(Model_Category::full_name(m_category_id, m_subcategory_id));

    m_entered_number->SetValue(m_checking_entry->TRANSACTIONNUMBER);
    m_entered_notes->SetValue(m_checking_entry->NOTES);

    if (m_account_id > 0)
    {
        Model_Currency::Data* currency = Model_Account::currency(Model_Account::instance().get(m_account_id));
        m_trans_currency->SetLabelText(currency->CURRENCY_SYMBOL);
    }
}

void UserTransactionPanel::SetLastPayeeAndCategory(const int account_id)
{
    if (Option::instance().TransPayeeSelection() == Option::LASTUSED)
    {
        Model_Checking::Data_Set trans_list = Model_Checking::instance().find(Model_Checking::ACCOUNTID(account_id));
        if (!trans_list.empty())
        {
            int last_trans_pos = trans_list.size() - 1;

            Model_Payee::Data* last_payee = Model_Payee::instance().get(trans_list.at(last_trans_pos).PAYEEID);
            m_payee->SetLabelText(last_payee->PAYEENAME);

            if (Option::instance().TransCategorySelection() == Option::LASTUSED)
            {
                m_payee_id = last_payee->PAYEEID;
                m_category_id = last_payee->CATEGID;
                m_category->SetLabelText(Model_Category::full_name(last_payee->CATEGID, last_payee->SUBCATEGID));
            }
        }
    }
}

void UserTransactionPanel::OnTransAccountButton(wxCommandEvent& WXUNUSED(event))
{
    const auto& accounts = Model_Account::instance().all_checking_account_names();
    mmSingleChoiceDialog scd(this
        , _("Select the required account")
        , _("Account Selection")
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
            if ((Option::instance().TransCategorySelection() == Option::LASTUSED) && (m_category_id < 0) && (m_subcategory_id < 0))
            {
                if (payee->CATEGID > 0)
                {
                    m_category_id = payee->CATEGID;
                    if (payee->SUBCATEGID > 0)
                    {
                        m_subcategory_id = payee->SUBCATEGID;
                    }
                    m_category->SetLabelText(Model_Category::full_name(m_category_id, m_subcategory_id));
                }
            }
        }
    }
}

void UserTransactionPanel::OnTransCategoryButton(wxCommandEvent& WXUNUSED(event))
{
    mmCategDialog dlg(this, m_category_id, m_subcategory_id);
    if (dlg.ShowModal() == wxID_OK)
    {
        m_category_id = dlg.getCategId();
        m_subcategory_id = dlg.getSubCategId();
        m_category->SetLabelText(Model_Category::full_name(m_category_id, m_subcategory_id));
    }
}

void UserTransactionPanel::OnDateSelectorForward(wxSpinEvent& WXUNUSED(event))
{
    SetNewDate(m_date_selector);
}

void UserTransactionPanel::OnDateSelectorBackward(wxSpinEvent& WXUNUSED(event))
{
    SetNewDate(m_date_selector, false);
}

void UserTransactionPanel::SetNewDate(wxDatePickerCtrl* dpc, bool forward)
{
    int day = -1;
    if (forward) day = 1;
    m_date_selector->SetValue(dpc->GetValue().Add(wxDateSpan::Days(day)));
}

void UserTransactionPanel::OnEnteredText(wxCommandEvent& event)
{
    Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();
    int currency_precision = Model_Currency::precision(currency);

    Model_Account::Data *account = Model_Account::instance().get(m_account_id);
    if (account)
    {
        currency = Model_Account::currency(account);
    }

    if (event.GetId() == m_entered_amount->GetId())
    {
        m_entered_amount->Calculate(currency_precision);
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
    if (i > 0 && i <= (int) m_frequent_notes.size())
        m_entered_notes->ChangeValue(m_frequent_notes[i - 1]);
}

void UserTransactionPanel::OnAttachments(wxCommandEvent& WXUNUSED(event))
{
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
    int RefId = m_transaction_id;

    if (RefId < 0)
        RefId = 0;

    mmAttachmentDialog dlg(this, RefType, RefId);
    dlg.ShowModal();
}

bool UserTransactionPanel::ValidCheckingAccountEntry(GUI_ERROR& g_err)
{
    if (m_account_id < 0)
    {
        g_err = GUI_ERROR::ACCOUNT;
        return false;
    }

    if (m_payee_id < 0)
    {
        g_err = GUI_ERROR::PAYEE;
        return false;
    }

    if (m_category_id < 0)
    {
        g_err = GUI_ERROR::CATEGORY;
        return false;
    }

    if (m_entered_amount->GetValue().IsEmpty())
    {
        g_err = GUI_ERROR::ENTRY;
        return false;
    }

    return true;
}

wxDateTime UserTransactionPanel::TransactionDate()
{
    return m_date_selector->GetValue();
}

void UserTransactionPanel::TransactionDate(const wxDateTime& trans_date)
{
    m_date_selector->SetValue(trans_date);
}

void UserTransactionPanel::SetTransactionValue(const double& trans_value, const double& commission, bool fixed_value)
{
    m_trans_value = trans_value;
    m_commission = commission;

    double value = m_trans_value + m_commission;
    if ((commission > 0) && (m_type_selector->GetSelection() == Model_Checking::DEPOSIT))
    {
        value = m_trans_value - m_commission;
    }

    m_entered_amount->SetValueNoEvent(value, 2);
    if (fixed_value)
    {
        m_entered_amount->Enable(false);
    }
}

void UserTransactionPanel::SetTransactionNumber(const wxString& trans_number)
{
    m_entered_number->SetValue(trans_number);
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
    m_transfer->SetValue(true);
    if (ct == Model_Translink::AS_INCOME_EXPENSE)
    {
        m_transfer->SetValue(false);
    }
}

int UserTransactionPanel::SaveChecking()
{
    double initial_amount = 0;
    m_entered_amount->checkValue(initial_amount);

    if (!m_checking_entry)
    {
        m_checking_entry = Model_Checking::instance().create();
    }

    m_checking_entry->ACCOUNTID = m_account_id;
    m_checking_entry->TOACCOUNTID = CheckingType();

    m_checking_entry->PAYEEID = m_payee_id;
    m_checking_entry->TRANSCODE = Model_Checking::instance().all_type()[TransactionType()];
    m_checking_entry->TRANSAMOUNT = initial_amount;

    TransactionStatus status(m_checking_entry);
    status.SetStatusA(Model_Checking::all_status()[m_status_selector->GetSelection()].Mid(0, 1));
    m_checking_entry->STATUS = status.Status(m_account_id);

    m_checking_entry->TRANSACTIONNUMBER = m_entered_number->GetValue();
    m_checking_entry->NOTES = m_entered_notes->GetValue();
    m_checking_entry->CATEGID = m_category_id;
    m_checking_entry->SUBCATEGID = m_subcategory_id;
    m_checking_entry->TRANSDATE = m_date_selector->GetValue().FormatISODate();
    m_checking_entry->FOLLOWUPID = 0;
    m_checking_entry->TOTRANSAMOUNT = m_checking_entry->TRANSAMOUNT;

    return Model_Checking::instance().save(m_checking_entry);
}

int UserTransactionPanel::TransactionType()
{
    return m_type_selector->GetSelection();
}

void UserTransactionPanel::OnTypeChoice(wxCommandEvent& WXUNUSED(event))
{
    SetTransactionValue(m_trans_value, m_commission);
}
