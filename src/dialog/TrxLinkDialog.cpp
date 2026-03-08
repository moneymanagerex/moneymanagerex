/*******************************************************
Copyright (C) 2014 Nikolay Akimov
Copyright (C) 2015 Stefano Giorgio
Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)
Copyright (C) 2026 Klaus Wich

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

#include "base/defs.h"
#include "base/images_list.h"
#include "util/_util.h"
#include "util/_simple.h"
#include "util/mmCalcValidator.h"

#include "model/_all.h"
#include "model/PrefModel.h"

#include "manager/CategoryManager.h"
#include "manager/PayeeManager.h"
#include "AttachmentDialog.h"
#include "SplitDialog.h"
#include "TrxLinkDialog.h"

/*******************************************************/
wxBEGIN_EVENT_TABLE(TrxLinkDialog, wxPanel)
EVT_BUTTON(ID_TRANS_ACCOUNT_BUTTON, TrxLinkDialog::OnTransAccountButton)
EVT_BUTTON(ID_TRANS_PAYEE_BUTTON, TrxLinkDialog::OnTransPayeeButton)
EVT_BUTTON(mmID_CATEGORY_SPLIT, TrxLinkDialog::OnCategs)
EVT_COMBOBOX(ID_TRANS_CATEGORY_COMBOBOX, TrxLinkDialog::OnTransCategoryCombobox)
EVT_BUTTON(wxID_FILE, TrxLinkDialog::OnAttachments)
wxEND_EVENT_TABLE()
/*******************************************************/

TrxLinkDialog::TrxLinkDialog()
{
}

TrxLinkDialog::TrxLinkDialog(
    wxWindow *parent,
    TrxData* transaction_n,
    bool enable_revalue,
    wxWindowID win_id,
    const wxPoint &pos,
    const wxSize &size,
    long style, const wxString &name
) :
    m_transaction_n(transaction_n),
    m_enable_revalue(enable_revalue)
{
    if (m_transaction_n) {
        for (const auto& tp_d: TrxSplitModel::instance().find(
            TrxSplitCol::TRANSID(m_transaction_n->m_id)
        )) {
            wxArrayInt64 tag_id_a;
            for (const auto& gl_d : TagLinkModel::instance().find(
                TagLinkCol::REFTYPE(TrxSplitModel::s_ref_type.name_n()),
                TagLinkCol::REFID(tp_d.m_id)
            )) {
                tag_id_a.push_back(gl_d.m_tag_id);
            }
            m_local_splits.push_back(
                {tp_d.m_category_id, tp_d.m_amount, tag_id_a, tp_d.m_notes}
            );
        }
    }

    Create(parent, win_id, pos, size, style, name);
    DataToControls();
    BindEventsAndTrigger();
    UsageModel::instance().pageview(this);
}

TrxLinkDialog::~TrxLinkDialog()
{
}

bool TrxLinkDialog::Create(
    wxWindow* parent,
    wxWindowID win_id,
    const wxPoint &pos,
    const wxSize &size,
    long style,
    const wxString &name
) {
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
    for (int i = 0; i < TrxModel::TYPE_ID_size; ++i)
    {
        if (i != TrxModel::TYPE_ID_TRANSFER || this->m_enable_revalue)
        {
            wxString type = TrxModel::trade_type_name(i);
            m_type_selector->Append(wxGetTranslation(type), new wxStringClientData(type));
        }
    }

    m_type_selector->SetSelection(TrxModel::TYPE_ID_WITHDRAWAL);
    mmToolTip(m_type_selector, _t("Withdraw funds from or deposit funds to this Account."));

    // transfer indicator (refined)
    m_transfer = new wxCheckBox(this, ID_TRANS_TRANSFER, _t("&Transfer")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    CheckingType(TrxLinkModel::AS_INCOME_EXPENSE);
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

    const CurrencyData* currency = CurrencyModel::GetBaseCurrency();
    if (m_account_id > 0) {
        currency = AccountModel::instance().get_id_currency_p(m_account_id);
    }
    m_trans_currency = new wxButton(this, ID_TRANS_CURRENCY_BUTTON, currency->m_symbol
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

    for (int i = 0; i < TrxModel::STATUS_ID_size; ++i)
    {
        wxString status = TrxModel::status_name(i);
        m_status_selector->Append(wxGetTranslation(status), new wxStringClientData(status));
    }

    m_status_selector->SetSelection(TrxModel::STATUS_ID_RECONCILED);
    mmToolTip(m_status_selector, _t("Specify the status for this transaction"));

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _t("Status")), g_flagsH);
    transPanelSizer->Add(m_status_selector, g_flagsH);

    // Payee ------------------------------------------------
    m_payee_text = new wxStaticText(this, ID_TRANS_PAYEE_BUTTON_TEXT, _t("Payee"));
    m_payee = new wxButton(this, ID_TRANS_PAYEE_BUTTON, _t("Select Payee"), wxDefaultPosition, std_size, 0);
    mmToolTip(m_payee, _t("Specify a person, Company or Organisation for this transaction."));
    transPanelSizer->Add(m_payee_text, g_flagsH);
    transPanelSizer->Add(m_payee, g_flagsH);

    // Category ---------------------------------------------
    wxStaticText* category_button_text = new wxStaticText(this, wxID_STATIC, _t("Category"));
    transPanelSizer->Add(category_button_text, g_flagsH);

    m_category = new mmComboBoxCategory(this, ID_TRANS_CATEGORY_COMBOBOX, std_size, m_category_id, true);
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
    m_frequent_notes = new wxButton(this, ID_TRANS_FREQUENT_NOTES, "..."
        , wxDefaultPosition, wxSize(m_attachment->GetSize().GetX(), -1));
    mmToolTip(m_frequent_notes, _t("Select one of the frequently used notes"));

    wxBoxSizer* right_align_sizer = new wxBoxSizer(wxHORIZONTAL);
    right_align_sizer->Add(m_attachment, g_flagsH);
    right_align_sizer->Add(m_frequent_notes, g_flagsH);

    m_entered_notes = new wxTextCtrl(this, ID_TRANS_ENTERED_NOTES, "", wxDefaultPosition, wxSize(220, 96), wxTE_MULTILINE);
    mmToolTip(m_entered_notes, _t("Specify any text notes you want to add to this transaction."));

    transPanelSizer->Add(right_align_sizer, wxSizerFlags(g_flagsH).Align(wxALIGN_RIGHT).Border(wxALL, 0));
    main_panel_sizer->Add(m_entered_notes, wxSizerFlags(g_flagsExpand).Border(wxTOP, 5));

    return true;
}

void TrxLinkDialog::DataToControls()
{
    if (!m_transaction_n)
        return;

    wxDateTime trans_date;
    trans_date.ParseDateTime(m_transaction_n->TRANSDATE)
        || trans_date.ParseDate(m_transaction_n->TRANSDATE);
    TransactionDate(trans_date);

    m_transaction_id = m_transaction_n->m_id;
    m_account_id = m_transaction_n->m_account_id;
    m_account->SetLabelText(AccountModel::instance().get_id_name(m_account_id));
    m_type_selector->SetSelection(TrxModel::type_id(m_transaction_n->TRANSCODE));

    if (m_account_id > 0) {
        const CurrencyData* currency = AccountModel::instance().get_id_currency_p(m_account_id);
        m_trans_currency->SetLabelText(currency->m_symbol);
        m_entered_amount->SetCurrency(currency);
    }

    SetTransactionValue(m_transaction_n->m_amount);
    m_status_selector->SetSelection(TrxModel::status_id(m_transaction_n->STATUS));

    m_payee_id = m_transaction_n->m_payee_id_n;
    m_payee->SetLabelText(PayeeModel::instance().get_id_name(m_payee_id));

    m_category_id = m_transaction_n->m_category_id_n;
    m_category->SetValue(CategoryModel::instance().full_name(m_category_id));

    m_entered_number->SetValue(m_transaction_n->m_number);
    m_entered_notes->SetValue(m_transaction_n->m_notes);

    bool has_split = !(m_local_splits.size() <= 1);
    if (has_split) {
        m_category->Enable(!has_split);
        m_category->SetLabelText(_t("Split Transaction"));

        SetTransactionValue(TrxSplitModel::get_total(m_local_splits));
    }

    if (!m_transaction_n->DELETEDTIME.IsEmpty()) {
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
        m_frequent_notes->Enable(false);
    }
}

void TrxLinkDialog::BindEventsAndTrigger()
{
    m_type_selector->Bind(wxEVT_CHOICE, [this](wxCommandEvent&)
    {
        int selection = m_type_selector->GetSelection();
        m_payee_text->Show(selection != TrxModel::TYPE_ID_TRANSFER);
        m_payee->Show(selection != TrxModel::TYPE_ID_TRANSFER);

        this->Layout();
    });
    wxCommandEvent evt(wxEVT_CHOICE, ID_TRANS_TYPE);
    wxPostEvent(m_type_selector, evt);

    m_frequent_notes->Bind(wxEVT_BUTTON, &TrxLinkDialog::OnFrequentNotes, this);
}

void TrxLinkDialog::SetLastPayeeAndCategory(const int64 account_id)
{
    if (PrefModel::instance().getTransPayeeNone() == PrefModel::LASTUSED) {
        TrxModel::DataA trans_list = TrxModel::instance().find(
            TrxCol::ACCOUNTID(account_id),
            TrxModel::TRANSCODE(OP_NE, TrxModel::TYPE_ID_TRANSFER)
        );
        if (!trans_list.empty()) {
            int last_trans_pos = trans_list.size() - 1;

            const PayeeData* last_payee_n = PayeeModel::instance().get_id_data_n(
                trans_list.at(last_trans_pos).m_payee_id_n
            );
            if (last_payee_n) {
                m_payee->SetLabelText(last_payee_n->m_name);
                m_payee_id = last_payee_n->m_id;
                if ((PrefModel::instance().getTransCategoryNone() == PrefModel::LASTUSED)
                    && !CategoryModel::instance().is_hidden(last_payee_n->m_category_id_n)
                ) {
                    m_category_id = last_payee_n->m_category_id_n;
                    m_category->SetLabelText(CategoryModel::instance().full_name(last_payee_n->m_category_id_n));
                }
            }
        }
    }
}

void TrxLinkDialog::OnTransAccountButton(wxCommandEvent& WXUNUSED(event))
{
    const auto& accounts = AccountModel::instance().find_all_name_a();
    mmSingleChoiceDialog scd(this
        , _t("Select the required account")
        , _t("Account Selection")
        , accounts);

    if (scd.ShowModal() == wxID_OK)
    {
        SetTransactionAccount(scd.GetStringSelection());
    }
}

void TrxLinkDialog::OnTransPayeeButton(wxCommandEvent& WXUNUSED(event))
{
    mmPayeeDialog dlg(this, true);
    if (dlg.ShowModal() == wxID_OK)
    {
        m_payee_id = dlg.getPayeeId();
        const PayeeData* payee_n = PayeeModel::instance().get_id_data_n(m_payee_id);
        if (payee_n) {
            m_payee->SetLabelText(payee_n->m_name);

            // Only for new transactions: if user want to autofill last category used for payee and category has not been set.
            if (PrefModel::instance().getTransCategoryNone() == PrefModel::LASTUSED
                && m_category_id < 0
                && m_subcategory_id < 0
                && !CategoryModel::instance().is_hidden(payee_n->m_category_id_n)
            ) {
                if (payee_n->m_category_id_n > 0) {
                    m_category_id = payee_n->m_category_id_n;
                    m_category->SetLabelText(CategoryModel::instance().full_name(m_category_id));
                }
            }
        }
    }
}

void TrxLinkDialog::OnTransCategoryCombobox(wxCommandEvent& WXUNUSED(event))
{
    if (!m_category->GetValue().empty())
    {
        m_category_id = m_category->mmGetCategoryId();
        return;
    }
    CategoryManager dlg(this, true, m_category_id);
    if (dlg.ShowModal() == wxID_OK)
    {
        m_category_id = dlg.getCategId();
        m_category->SetLabelText(CategoryModel::instance().full_name(m_category_id));
    }
}

void TrxLinkDialog::OnFrequentNotes(wxCommandEvent& WXUNUSED(event))
{
    std::vector<wxString> frequent_notes;
    TrxModel::getFrequentUsedNotes(frequent_notes);
    wxMenu menu;
    int id = wxID_HIGHEST;
    for (const auto& entry : frequent_notes)
    {
        const wxString& label = entry.Mid(0, 30) + (entry.size() > 30 ? "..." : "");
        int menu_id = ++id;
        menu.Append(menu_id, label);
        Bind(wxEVT_MENU, [this, notes = frequent_notes, i = menu_id - wxID_HIGHEST](wxCommandEvent&)
        {
            if (i > 0 && i <= static_cast<int>(notes.size()))
            m_entered_notes->ChangeValue(notes[i - 1]);
        }, menu_id);
    }

    if (!frequent_notes.empty())
        PopupMenu(&menu);
}

void TrxLinkDialog::OnAttachments(wxCommandEvent& WXUNUSED(event))
{
    int64 RefId = m_transaction_id;

    if (RefId < 0)
        RefId = 0;

    AttachmentDialog dlg(this, TrxModel::s_ref_type, RefId);
    dlg.ShowModal();
}

bool TrxLinkDialog::ValidCheckingAccountEntry()
{
    m_category_id = m_category->mmGetCategoryId();  // update from selection
    return (m_account_id != -1) && (m_payee_id != -1 || TransactionType() == TrxModel::TYPE_ID_TRANSFER) && (m_category_id != -1) && (!m_entered_amount->GetValue().IsEmpty());
}

wxDateTime TrxLinkDialog::TransactionDate()
{
    return m_date_selector->GetValue();
}

void TrxLinkDialog::TransactionDate(const wxDateTime& trans_date)
{
    m_date_selector->SetValue(trans_date);
}

void TrxLinkDialog::SetTransactionValue(const double& trans_value, bool fixed_value)
{
    m_entered_amount->SetValue(trans_value);
    if (fixed_value) {
        m_entered_amount->Enable(false);
    }
}

void TrxLinkDialog::SetTransactionNumber(const wxString& trans_number)
{
    m_entered_number->SetValue(trans_number);
}

void TrxLinkDialog::SetTransactionStatus(const int trans_status_enum)
{
    m_status_selector->SetSelection(trans_status_enum);
}

void TrxLinkDialog::SetTransactionPayee(const int64 payeeid)
{
    m_payee_id = payeeid;
    const PayeeData* payee_n = PayeeModel::instance().get_id_data_n(m_payee_id);
    if (payee_n)
        m_payee->SetLabelText(payee_n->m_name);
}

void TrxLinkDialog::SetTransactionCategory(const int64 categid)
{
    m_category_id = categid;
    m_category->SetLabelText(CategoryModel::instance().full_name(m_category_id));
}

void TrxLinkDialog::SetTransactionAccount(const wxString& trans_account)
{
    const AccountData* account = AccountModel::instance().get_name_data_n(trans_account);
    if (account) {
        m_account->SetLabelText(account->m_name);
        m_account_id = account->m_id;
        SetLastPayeeAndCategory(m_account_id);
        const CurrencyData* currency = CurrencyModel::instance().get_id_data_n(account->m_currency_id);
        m_entered_amount->SetCurrency(currency);
        m_trans_currency->SetLabelText(currency->m_symbol);
    }
}

TrxLinkModel::CHECKING_TYPE TrxLinkDialog::CheckingType()
{
    if (m_transfer->IsChecked())
        return TrxLinkModel::AS_TRANSFER;
    else
        return TrxLinkModel::AS_INCOME_EXPENSE;
}

void TrxLinkDialog::CheckingType(TrxLinkModel::CHECKING_TYPE ct)
{
    m_transfer->SetValue(false);
    if (ct == TrxLinkModel::AS_TRANSFER) {
        m_transfer->SetValue(true);
    }
}

int64 TrxLinkDialog::SaveChecking()
{
    double initial_amount = 0;
    m_entered_amount->checkValue(initial_amount);

    const AccountData* account = AccountModel::instance().get_id_data_n(m_account_id);
    wxDateTime trx_datetime = m_date_selector->GetValue();
    if (mmDate(trx_datetime) < account->m_open_date) {
        mmErrorDialogs::ToolTip4Object(m_account,
            _t("The opening date for the account is later than the date of this transaction"),
            _t("Invalid Date")
        );
        return -1;
    }

    if (!m_transaction_n) {
        m_transaction_d = TrxData();
        // only set if new transaction, otherwise retain old value!
        m_transaction_d.m_color = 0;
        m_transaction_d.m_followup_id = 0;
        m_transaction_n = &m_transaction_d;
    }

    m_transaction_n->m_account_id = m_account_id;
    m_transaction_n->m_to_account_id_n = (
        TransactionType() == TrxModel::TYPE_ID_TRANSFER ||
        CheckingType() == TrxLinkModel::AS_TRANSFER
    ) ? m_account_id : -1; // Self Transfer as Revaluation

    m_transaction_n->m_payee_id_n    = m_payee_id;
    m_transaction_n->TRANSCODE       = TrxModel::type_name(TransactionType());
    m_transaction_n->m_amount        = initial_amount;
    m_transaction_n->STATUS          = m_status_selector->GetStringSelection().Mid(0, 1);
    m_transaction_n->m_number        = m_entered_number->GetValue();
    m_transaction_n->m_notes         = m_entered_notes->GetValue();
    m_transaction_n->m_category_id_n = m_category_id;
    m_transaction_n->TRANSDATE       = trx_datetime.FormatISOCombined();
    m_transaction_n->m_to_amount     = m_transaction_n->m_amount;

    TrxModel::instance().unsafe_save_trx_n(m_transaction_n);
    return m_transaction_n->id();
}

int TrxLinkDialog::TransactionType()
{
    return m_type_selector->GetSelection();
}


void TrxLinkDialog::OnCategs(wxCommandEvent& WXUNUSED(event))
{
    if (m_local_splits.empty() && m_category->mmIsValid()) {
        Split split_d;

        m_entered_amount->GetDouble(split_d.SPLITTRANSAMOUNT);

        split_d.CATEGID = m_category->mmGetCategoryId();
        split_d.NOTES   = m_entered_notes->GetValue();
        m_local_splits.push_back(split_d);
    }

    SplitDialog dlg(this, m_local_splits, m_account_id);

    if (dlg.ShowModal() == wxID_OK) {
        m_local_splits = dlg.mmGetResult();

        if (m_local_splits.size() == 1) {
            m_category->SetLabelText(CategoryModel::instance().full_name(m_local_splits[0].CATEGID));
            m_entered_amount->SetValue(m_local_splits[0].SPLITTRANSAMOUNT);
            m_entered_notes->SetValue(m_local_splits[0].NOTES);

            m_local_splits.clear();
        }

        if (m_local_splits.empty()) {
            m_category->Enable(true);
        }
        else {
            m_entered_amount->SetValue(TrxSplitModel::get_total(m_local_splits));

            m_category->Enable(false);
            m_category->SetLabelText(_t("Split Transaction"));
        }
    }
}
