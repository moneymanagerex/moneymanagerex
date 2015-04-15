/*******************************************************
Copyright (C) 2014 Nikolay
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

#include "mmUserPanelTrans.h"
#include "mmOption.h"
#include "validators.h"

#include "maincurrencydialog.h"
#include "mmSimpleDialogs.h"
#include "payeedialog.h"
#include "categdialog.h"
#include "attachmentdialog.h"

#include "model/Model_Currency.h"
#include "model/Model_Payee.h"
#include "model/Model_Category.h"
#include "model/Model_Checking.h"
#include "model/Model_Attachment.h"

#include "../resources/attachment.xpm"

/*******************************************************/
wxBEGIN_EVENT_TABLE(mmUserPanelTrans, wxPanel)
    EVT_SPIN_UP(ID_TRANS_DATE_CONTROLLER, mmUserPanelTrans::OnDateSelectorForward)
    EVT_SPIN_DOWN(ID_TRANS_DATE_CONTROLLER, mmUserPanelTrans::OnDateSelectorBackward)
    EVT_BUTTON(ID_TRANS_ACCOUNT_BUTTON, mmUserPanelTrans::OnTransAccountButton)
    EVT_BUTTON(ID_TRANS_CURRENCY_BUTTON, mmUserPanelTrans::OnTransCurrencyButton)
    EVT_BUTTON(ID_TRANS_PAYEE_BUTTON, mmUserPanelTrans::OnTransPayeeButton)
    EVT_BUTTON(ID_TRANS_CATEGORY_BUTTON, mmUserPanelTrans::OnTransCategoryButton)
    EVT_MENU(wxID_ANY, mmUserPanelTrans::onSelectedNote)
    EVT_BUTTON(wxID_FILE, mmUserPanelTrans::OnAttachments)
wxEND_EVENT_TABLE()
/*******************************************************/

mmUserPanelTrans::mmUserPanelTrans()
{
}

mmUserPanelTrans::mmUserPanelTrans(wxWindow *parent
    , Model_Checking::Data* checking_entry
    , wxWindowID win_id
    , const wxPoint &pos
    , const wxSize &size
    , long style, const wxString &name)
    : m_checking_entry(checking_entry)
    , m_checking_trans_id(-1)
    , m_account_id(-1)
    , m_payee_id(-1)
    , m_category_id(-1)
    , m_subcategory_id(-1)
{
    wxPanel::Create(parent, win_id, pos, size, style, name);
    Create();
    DataToControls();
}

mmUserPanelTrans::~mmUserPanelTrans()
{
}

void mmUserPanelTrans::Create()
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
    date_sizer->Add(m_date_selector, g_flags);
    date_sizer->Add(m_date_controller, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT, interval);

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Date")), g_flags);
    transPanelSizer->Add(date_sizer);

    // Account ------------------------------------------------
    transPanelSizer->Add(new wxStaticText(this, ID_TRANS_ACCOUNT_BUTTON_TEXT, _("Account")), g_flags);
    m_account = new wxButton(this, ID_TRANS_ACCOUNT_BUTTON, _("Select Account")
        , wxDefaultPosition, std_size);
    m_account->SetToolTip(_("Specify the associated Account that will contain this transaction"));
    transPanelSizer->Add(m_account, g_flags);

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
    SetCheckingType(Model_TransferTrans::AS_TRANSFER);
    m_transfer->SetToolTip(_("Funds transfer from/to this account. Uncheck to set as Expense/Income."));

    wxBoxSizer* type_sizer = new wxBoxSizer(wxHORIZONTAL);
    type_sizer->Add(m_type_selector, g_flags);
    type_sizer->Add(m_transfer, g_flags);

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Type")), g_flags);
    transPanelSizer->Add(type_sizer);

    // Amount ------------------------------------------------
    wxStaticText* entered_amount_text = new wxStaticText(this, wxID_STATIC, _("Amount"));
    m_entered_amount = new mmTextCtrl(this, ID_TRANS_ENTERED_AMOUNT, ""
        , wxDefaultPosition, std_half_size, wxALIGN_RIGHT | wxTE_PROCESS_ENTER
        , mmCalcValidator());
    m_entered_amount->SetToolTip(_("Specify the amount for this transaction"));
    m_entered_amount->Connect(ID_TRANS_ENTERED_AMOUNT
        , wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(mmUserPanelTrans::OnEnteredText), nullptr, this);
    //TODO: m_entered_amount Enable/disable
    //m_entered_amount->Enable(false);

    Model_Currency::Data* currency = Model_Currency::GetBaseCurrency();
    if (m_account_id > 0)
    {
        currency = Model_Account::currency(Model_Account::instance().get(m_account_id));
    }
    m_trans_currency = new wxButton(this, ID_TRANS_CURRENCY_BUTTON, currency->CURRENCY_SYMBOL
        , wxDefaultPosition, std_half_size);
    m_trans_currency->SetToolTip(_("Set the currency to be used for this transaction."));

    wxBoxSizer* entered_amount_sizer = new wxBoxSizer(wxHORIZONTAL);
    entered_amount_sizer->Add(m_entered_amount, g_flags);
    entered_amount_sizer->Add(m_trans_currency, g_flags);
    //TODO m_trans_currency Show/ Hide
    //m_trans_currency->Hide();

    transPanelSizer->Add(entered_amount_text, g_flags);
    transPanelSizer->Add(entered_amount_sizer);

    // Status --------------------------------------------
    m_status_selector = new wxChoice(this, ID_TRANS_STATUS_SELECTOR
        , wxDefaultPosition, std_half_size);

    for (const auto& i : Model_Checking::all_status())
    {
        m_status_selector->Append(wxGetTranslation(i), new wxStringClientData(i));
    }

    m_status_selector->SetSelection(mmIniOptions::instance().transStatusReconciled_);
    m_status_selector->SetToolTip(_("Specify the status for this transaction"));

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Status")), g_flags);
    transPanelSizer->Add(m_status_selector, g_flags);

    // Payee ------------------------------------------------
    wxStaticText* payee_button_text = new wxStaticText(this, ID_TRANS_PAYEE_BUTTON_TEXT, _("Payee"));
    m_payee = new wxButton(this, ID_TRANS_PAYEE_BUTTON, _("Select Payee"), wxDefaultPosition, std_size, 0);
    m_payee->SetToolTip(_("Specify a person, Company or Organisation for this transaction."));
    transPanelSizer->Add(payee_button_text, g_flags);
    transPanelSizer->Add(m_payee, g_flags);

    // Category ---------------------------------------------
    wxStaticText* category_button_text = new wxStaticText(this, wxID_STATIC, _("Category"));
    m_category = new wxButton(this, ID_TRANS_CATEGORY_BUTTON, _("Select Category")
        , wxDefaultPosition, std_size, 0);
    m_category->SetToolTip(_("Specify the category for this transaction"));

    transPanelSizer->Add(category_button_text, g_flags);
    transPanelSizer->Add(m_category, g_flags);

    // Number ---------------------------------------------
    m_entered_number = new wxTextCtrl(this, ID_TRANS_ENTERED_NUMBER, ""
        , wxDefaultPosition, std_size);
    m_entered_number->SetToolTip(_("Specify a transaction code or associated check details"));

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Number")), g_flags);
    transPanelSizer->Add(m_entered_number, g_flags);

    // Notes ---------------------------------------------
    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Notes")), g_flags);

    // Attachment ---------------------------------------------
    m_attachment = new wxBitmapButton(this, wxID_FILE, wxBitmap(attachment_xpm));
    m_attachment->SetToolTip(_("Organize attachments of this transaction"));
    //TODO: m_attachment Enable/disable
    //m_attachment->Enable(false);

    // Frequent Notes ---------------------------------------------
    wxButton* frequent_notes = new wxButton(this, ID_TRANS_FREQUENT_NOTES, "..."
        , wxDefaultPosition, wxSize(m_attachment->GetSize().GetX(), -1));
    frequent_notes->SetToolTip(_("Select one of the frequently used notes"));
    frequent_notes->Connect(ID_TRANS_FREQUENT_NOTES
        , wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(mmUserPanelTrans::OnFrequentNotes), nullptr, this);

    wxBoxSizer* right_align_sizer = new wxBoxSizer(wxHORIZONTAL);
    right_align_sizer->Add(m_attachment, g_flags);
    right_align_sizer->Add(frequent_notes, g_flags);

    m_entered_notes = new wxTextCtrl(this, ID_TRANS_ENTERED_NOTES, "", wxDefaultPosition, wxSize(220, 96), wxTE_MULTILINE);
    m_entered_notes->SetToolTip(_("Specify any text notes you want to add to this transaction."));

    transPanelSizer->Add(right_align_sizer, wxSizerFlags(g_flags).Align(wxALIGN_RIGHT).Border(wxALL, 0));
    main_panel_sizer->Add(m_entered_notes, wxSizerFlags(g_flagsExpand).Border(wxTOP, 5));
}

void mmUserPanelTrans::DataToControls()
{
    if (!m_checking_entry) return;

    wxDateTime trans_date;
    trans_date.ParseDate(m_checking_entry->TRANSDATE);
    SetTransactionDate(trans_date);

    m_checking_trans_id = m_checking_entry->TRANSID;
    m_account_id = m_checking_entry->ACCOUNTID;
    m_account->SetLabelText(Model_Account::get_account_name(m_account_id));
    m_type_selector->SetSelection(Model_Checking::type(m_checking_entry->TRANSCODE));

    SetTransactionValue(m_checking_entry->TRANSAMOUNT);
    m_status_selector->SetSelection(Model_Checking::status(m_checking_entry->STATUS));

    m_payee_id = m_checking_entry->PAYEEID;
    m_payee->SetLabelText(Model_Payee::get_payee_name(m_payee_id));
    
    m_category_id = m_checking_entry->CATEGID;
    m_subcategory_id = m_checking_entry->SUBCATEGID;
    m_category->SetLabelText(Model_Category::full_name(m_category_id, m_subcategory_id));

    m_entered_number->SetValue(m_checking_entry->TRANSACTIONNUMBER);
    m_entered_notes->SetValue(m_checking_entry->NOTES);
}

void mmUserPanelTrans::OnTransAccountButton(wxCommandEvent& WXUNUSED(event))
{
    const auto& accounts = Model_Account::instance().all_checking_account_names();
    mmSingleChoiceDialog scd(this
        , _("Select the required account")
        , _("Account Selection")
        , accounts);

    if (scd.ShowModal() == wxID_OK)
    {
        Model_Account::Data* account = Model_Account::instance().get(scd.GetStringSelection());
        m_account->SetLabelText(account->ACCOUNTNAME);
        m_account_id = account->ACCOUNTID;
        Model_Currency::Data* currency = Model_Currency::instance().get(account->CURRENCYID);
        m_trans_currency->SetLabelText(currency->CURRENCY_SYMBOL);
    }
}

void mmUserPanelTrans::OnTransCurrencyButton(wxCommandEvent& WXUNUSED(event))
{
    int currency_id = Model_Infotable::instance().GetBaseCurrencyId();

    if (mmMainCurrencyDialog::Execute(this, currency_id) && currency_id != -1)
    {
        Model_Currency::Data* currency = Model_Currency::instance().get(currency_id);
        m_trans_currency->SetLabelText(currency->CURRENCY_SYMBOL);
    }
}

void mmUserPanelTrans::OnTransPayeeButton(wxCommandEvent& WXUNUSED(event))
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
            if ((mmIniOptions::instance().transCategorySelectionNone_ == 1) && (m_category_id < 0) && (m_subcategory_id < 0))
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

void mmUserPanelTrans::OnTransCategoryButton(wxCommandEvent& WXUNUSED(event))
{
    mmCategDialog dlg(this);
    dlg.setTreeSelection(m_category_id, m_subcategory_id);
    if (dlg.ShowModal() == wxID_OK)
    {
        m_category_id = dlg.getCategId();
        m_subcategory_id = dlg.getSubCategId();
        m_category->SetLabelText(Model_Category::full_name(m_category_id, m_subcategory_id));
    }
}

void mmUserPanelTrans::OnDateSelectorForward(wxSpinEvent& WXUNUSED(event))
{
    SetNewDate(m_date_selector);
}

void mmUserPanelTrans::OnDateSelectorBackward(wxSpinEvent& WXUNUSED(event))
{
    SetNewDate(m_date_selector, false);
}

void mmUserPanelTrans::SetNewDate(wxDatePickerCtrl* dpc, bool forward)
{
    int day = -1;
    if (forward) day = 1;
    m_date_selector->SetValue(dpc->GetValue().Add(wxDateSpan::Days(day)));
}

void mmUserPanelTrans::OnEnteredText(wxCommandEvent& event)
{
    Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();
    Model_Account::Data *account = Model_Account::instance().get(m_account_id);
    if (account)
    {
        currency = Model_Account::currency(account);
    }

    if (event.GetId() == m_entered_amount->GetId())
    {
        m_entered_amount->Calculate(currency);
    }
}

void mmUserPanelTrans::OnFrequentNotes(wxCommandEvent& WXUNUSED(event))
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

void mmUserPanelTrans::onSelectedNote(wxCommandEvent& event)
{
    int i = event.GetId() - wxID_HIGHEST;
    if (i > 0 && i <= (int) m_frequent_notes.size())
        m_entered_notes->ChangeValue(m_frequent_notes[i - 1]);
}

void mmUserPanelTrans::OnAttachments(wxCommandEvent& WXUNUSED(event))
{
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
    int RefId = m_checking_trans_id;

    if (RefId < 0)
        RefId = 0;

    mmAttachmentDialog dlg(this, RefType, RefId);
    dlg.ShowModal();
}

bool mmUserPanelTrans::ValidCheckingAccountEntry()
{
    if ((m_account_id != -1) && (m_payee_id != -1) && (m_category_id != -1))
        return true;
    else
        return false;
}

void mmUserPanelTrans::SetTransactionDate(const wxDateTime& trans_date)
{
    m_date_selector->SetValue(trans_date);
}

void mmUserPanelTrans::SetTransactionValue(const wxString& trans_value)
{
    m_entered_amount->SetValue(trans_value);
}

void mmUserPanelTrans::SetTransactionValue(const double& trans_value)
{
    SetTransactionValue(wxString::FromDouble(trans_value, 2));
}

void mmUserPanelTrans::SetTransactionNumber(const wxString& trans_number)
{
    m_entered_number->SetValue(trans_number);
}

const wxString mmUserPanelTrans::CurrencySymbol()
{
    return m_trans_currency->GetLabelText();
}

Model_Currency::Data* mmUserPanelTrans::GetCurrencyData()
{
    return Model_Currency::instance().GetCurrencyRecord(CurrencySymbol());
}

Model_TransferTrans::CHECKING_TYPE mmUserPanelTrans::CheckingType()
{
    if (m_transfer->IsChecked())
        return Model_TransferTrans::AS_TRANSFER;
    else
        return Model_TransferTrans::AS_INCOME_EXPENSE;
}

void mmUserPanelTrans::SetCheckingType(Model_TransferTrans::CHECKING_TYPE ct)
{
    m_transfer->SetValue(true);
    if (ct == Model_TransferTrans::AS_INCOME_EXPENSE)
    {
        m_transfer->SetValue(false);
    }
}

int mmUserPanelTrans::SaveChecking()
{
    Model_Currency::Data* currency = Model_Currency::instance().GetCurrencyRecord(CurrencySymbol());

    double value = 0;
    m_entered_amount->checkValue(value);

    if (!m_checking_entry)
    {
        m_checking_entry = Model_Checking::instance().create();
    }

    m_checking_entry->ACCOUNTID = m_account_id;
    m_checking_entry->TOACCOUNTID = CheckingType();

    m_checking_entry->PAYEEID = m_payee_id;
    m_checking_entry->TRANSCODE = Model_Checking::instance().all_type()[m_type_selector->GetSelection()];
    m_checking_entry->TRANSAMOUNT = value * currency->BASECONVRATE;
    m_checking_entry->STATUS = Model_Checking::all_status()[m_status_selector->GetSelection()].Mid(0, 1);
    m_checking_entry->TRANSACTIONNUMBER = m_entered_number->GetValue();
    m_checking_entry->NOTES = m_entered_notes->GetValue();
    m_checking_entry->CATEGID = m_category_id;
    m_checking_entry->SUBCATEGID = m_subcategory_id;
    m_checking_entry->TRANSDATE = m_date_selector->GetValue().FormatISODate();
    m_checking_entry->FOLLOWUPID = 0;
    m_checking_entry->TOTRANSAMOUNT = m_checking_entry->TRANSAMOUNT;

    return Model_Checking::instance().save(m_checking_entry);
}

int mmUserPanelTrans::TransactionType()
{
    return m_type_selector->GetSelection();
}
