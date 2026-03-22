/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011-2022 Nikolay Akimov
 Copyright (C) 2011-2017 Stefano Giorgio [stef145g]
 Copyright (C) 2021-2026 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2025 Klaus Wich

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
#include <wx/numformatter.h>
#include <wx/timectrl.h>
#include <wx/collpane.h>
#include <wx/display.h>

#include "base/constants.h"
#include "base/paths.h"
#include "base/images_list.h"
#include "util/_util.h"
#include "util/_simple.h"
#include "util/mmTextCtrl.h"
#include "util/mmCalcValidator.h"

#include "model/AccountModel.h"
#include "model/AttachmentModel.h"
#include "model/CategoryModel.h"
#include "model/CurrencyHistoryModel.h"
#include "model/FieldValueModel.h"
#include "model/PrefModel.h"
#include "model/SettingModel.h"
#include "model/TagModel.h"

#include "manager/CategoryManager.h"
#include "manager/PayeeManager.h"

#include "AttachmentDialog.h"
#include "FieldValueDialog.h"
#include "SplitDialog.h"
#include "TrxDialog.h"

#include "pref/ViewPref.h"

#include "import_export/webapp.h"

wxIMPLEMENT_DYNAMIC_CLASS(TrxDialog, wxDialog);

wxBEGIN_EVENT_TABLE(TrxDialog, wxDialog)
    EVT_CHAR_HOOK(                                  TrxDialog::OnComboKey)
    EVT_CHILD_FOCUS(                                TrxDialog::OnFocusChange)
    EVT_COMBOBOX(mmID_PAYEE,                        TrxDialog::OnPayeeChanged)
    EVT_TEXT(mmID_PAYEE,                            TrxDialog::OnPayeeChanged)
    EVT_BUTTON(mmID_CATEGORY_SPLIT,                 TrxDialog::OnCategs)
    EVT_CHOICE(ID_DIALOG_TRANS_TYPE,                TrxDialog::OnTransTypeChanged)
    EVT_CHECKBOX(ID_DIALOG_TRANS_ADVANCED_CHECKBOX, TrxDialog::OnAdvanceChecked)
    EVT_BUTTON(wxID_FILE,                           TrxDialog::OnAttachments)
    EVT_BUTTON(ID_DIALOG_TRANS_CUSTOMFIELDS,        TrxDialog::OnMoreFields)
    EVT_BUTTON(wxID_OK,                             TrxDialog::OnOk)
    EVT_BUTTON(ID_BTN_OK_NEW,                       TrxDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL,                         TrxDialog::OnCancel)
    EVT_BUTTON(ID_DIALOG_TRANS_TODAY,               TrxDialog::OnToday)
    EVT_CLOSE(                                      TrxDialog::OnQuit)
    EVT_MENU(wxID_SAVE,                             TrxDialog::TrxDialog::OnOk)
    EVT_MENU(ID_BTN_OK_NEW,                         TrxDialog::TrxDialog::OnOk)
    EVT_MENU(ID_DIALOG_TRANS_TODAY,                 TrxDialog::OnToday)
wxEND_EVENT_TABLE()

TrxDialog::~TrxDialog()
{
    wxSize size = GetSize();
    if (m_custom_fields->IsCustomPanelShown())
        size = wxSize(GetSize().GetWidth() - m_custom_fields->GetMinWidth(), GetSize().GetHeight());
    InfoModel::instance().setSize("TRANSACTION_DIALOG_SIZE", size);
}

void TrxDialog::SetEventHandlers()
{
    m_textAmount->Connect(mmID_TEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(TrxDialog::OnTextEntered), nullptr, this);
    toTextAmount_->Connect(mmID_TOTEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(TrxDialog::OnTextEntered), nullptr, this);

#ifdef __WXGTK__ // Workaround for bug http://trac.wxwidgets.org/ticket/11630
    dpc_->Connect(ID_DIALOG_TRANS_BUTTONDATE, wxEVT_KILL_FOCUS
        , wxFocusEventHandler(TrxDialog::OnDpcKillFocus), nullptr, this);
#endif
}

// Used to determine if we need to refresh the tag text ctrl after
// accelerator hints are shown which only occurs once.
static bool altRefreshDone;
// store used date between two invocations for save & new
static wxDateTime previousDate;

TrxDialog::TrxDialog(
    wxWindow* parent,
    int64 account_id,
    JournalKey journal_key,
    bool duplicate,
    TrxType type
) :
    m_account_id(account_id)
{
    SetEvtHandlerEnabled(false);
    bool found = Journal::setJournalData(m_journal_data, journal_key);
    if (found) {
        // a bill can only be duplicated
        m_mode = (duplicate || journal_key.is_scheduled()) ? MODE_DUP : MODE_EDIT;
        RefTypeN split_ref_type = m_journal_data.key().ref_type();
        for (const auto& tp_d : Journal::split(m_journal_data)) {
            wxArrayInt64 tag_id_a;
            for (const auto& gl_d : TagLinkModel::instance().find(
                TagLinkCol::REFTYPE(split_ref_type.name_n()),
                TagLinkCol::REFID(tp_d.m_id))
            )
                tag_id_a.push_back(gl_d.m_tag_id);
            m_local_splits.push_back({
                tp_d.m_category_id, tp_d.m_amount, tp_d.m_notes, tag_id_a
            });
        }

        if (m_mode == MODE_DUP &&
            !SettingModel::instance().getBool(INIDB_USE_ORG_DATE_DUPLICATE, false)
        ) {
            // Use the empty transaction logic to generate the new date to be used
            TrxData emptyTrx;
            TrxModel::instance().setEmptyData(emptyTrx, account_id);
            m_journal_data.m_date_time = emptyTrx.m_date_time;
        }
    }
    else {
        m_mode = MODE_NEW;
        TrxModel::instance().setEmptyData(m_journal_data, account_id);
        m_journal_data.m_type = type;
    }

    m_transfer = m_journal_data.is_transfer();
    m_advanced = m_mode != MODE_NEW && m_transfer &&
        (m_journal_data.m_amount != m_journal_data.m_to_amount);

    m_custom_fields = new mmCustomDataTransaction(this,
        m_mode == MODE_NEW ? TrxModel::s_ref_type : m_journal_data.key().ref_type(),
        m_mode == MODE_NEW ? -1                   : m_journal_data.key().ref_id(),
        ID_CUSTOMFIELDS
    );

    // If duplicate then we may need to copy the attachments
// FIXME: commented out until fixed
/*
    if (m_mode == MODE_DUP && InfoModel::instance().getBool("ATTACHMENTSDUPLICATE", false)) {
        // FIXME: check if journal_key.is_reliazed()
        // FIXME: id -1 does not exist in database
        mmAttachmentManage::CloneAllAttachments(
            TrxModel::s_ref_type, journal_key.rid(), -1
        );
    }
*/

    this->SetFont(parent->GetFont());
    Create(parent);
    mmThemeAutoColour(this);
    dataToControls();

    mmSetSize(this);
    // set the initial dialog size to expand the payee and category comboboxes to fit their text
    int minWidth = std::max(0,
        cbPayee_->GetSizeFromText(cbPayee_->GetValue()).GetWidth() - 2 * cbPayee_->GetMinWidth());
    minWidth = std::max(minWidth,
        cbCategory_->GetSizeFromText(cbCategory_->GetValue()).GetWidth() - 2 * cbCategory_->GetMinWidth());

    int custom_fields_width = m_custom_fields->IsCustomPanelShown() ? m_custom_fields->GetMinWidth() : 0;
    wxSize size = wxSize(GetMinWidth() + minWidth + custom_fields_width, GetSize().GetHeight());
    if (size.GetWidth() > GetSize().GetWidth())
        SetSize(size);
    if (custom_fields_width)
        SetMinSize(wxSize(GetMinWidth() + m_custom_fields->GetMinWidth(), GetMinHeight()));
    Centre();
    SetEvtHandlerEnabled(true);
}

bool TrxDialog::Create(
    wxWindow* parent, wxWindowID id, const wxString& caption,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name
) {
    altRefreshDone = false; // reset the ALT refresh indicator on new dialog creation
    style |= wxRESIZE_BORDER;
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style, name);

    SetEvtHandlerEnabled(false);
    CreateControls();

    wxString header = (m_mode == MODE_NEW) ? _t("New Transaction") :
        (m_mode == MODE_DUP) ? _t("Duplicate Transaction") :
        !m_journal_data.is_deleted() ? _t("Edit Transaction") :
        "";  //_t("View Deleted Transaction");
    SetDialogTitle(header);

    SetIcon(mmex::getProgramIcon());

    SetEventHandlers();
    SetEvtHandlerEnabled(true);
    return true;
}

void TrxDialog::dataToControls()
{
    // short names for model instances
    CurrencyModel& U  = CurrencyModel::instance();
    AccountModel&  A  = AccountModel::instance();
    PayeeModel&    P  = PayeeModel::instance();
    TrxModel&      T  = TrxModel::instance();
    TagLinkModel&  GL = TagLinkModel::instance();

    TrxModel::instance().getFrequentUsedNotes(frequentNotes_, m_journal_data.m_account_id);
    wxButton* bFrequentUsedNotes = static_cast<wxButton*>(
        FindWindow(ID_DIALOG_TRANS_BUTTON_FREQENTNOTES)
    );
    bFrequentUsedNotes->Enable(!frequentNotes_.empty());

    // Date
    if (!skip_date_init_) {
        wxDateTime trx_date;
        if (previousDate.IsValid()) {
            trx_date = previousDate;
        }
        else {
            bool is_time_used = PrefModel::instance().UseTransDateTime();
            trx_date = is_time_used
                ? m_journal_data.m_date_time.getDateTime()
                : m_journal_data.m_date().getDateTime();
        }
        dpc_->SetValue(trx_date);
        dpc_->SetFocus();
        skip_date_init_ = true;
    }

    // Status
    if (!skip_status_init_) {
        bool useOriginalState = m_mode != MODE_DUP ||
            SettingModel::instance().getBool(INIDB_USE_ORG_STATE_DUPLICATE_PASTE, false);
        m_status = useOriginalState
            ? m_journal_data.m_status.key()
            : TrxStatus(PrefModel::instance().getTransStatusReconciled()).key();
        choiceStatus_->SetSelection(TrxStatus(m_status).id());
        skip_status_init_ = true;
    }

    // Type
    transaction_type_->SetSelection(m_journal_data.m_type.id());

    // Account
    if (!skip_account_init_) {
        const AccountData* acc_n = A.get_id_data_n(m_journal_data.m_account_id);
        if (acc_n) {
            cbAccount_->ChangeValue(acc_n->m_name);
            m_textAmount->SetCurrency(U.get_id_data_n(acc_n->m_currency_id));
        }
        const AccountData* to_acc = A.get_id_data_n(m_journal_data.m_to_account_id_n);
        if (to_acc) {
            cbToAccount_->ChangeValue(to_acc->m_name);
            toTextAmount_->SetCurrency(U.get_id_data_n(to_acc->m_currency_id));
        }

        skip_account_init_ = true;
        skip_amount_init_ = false; // Force amount format update in case account currencies change
    }

    if (m_transfer) {
        account_label_->SetLabelText(_t("From"));
        payee_label_->SetLabelText(_t("To"));
    }
    else if (!m_journal_data.is_deposit()) {
        account_label_->SetLabelText(_t("Account"));
        payee_label_->SetLabelText(_t("Payee"));
    }
    else {
        account_label_->SetLabelText(_t("Account"));
        payee_label_->SetLabelText(_t("From"));
    }

    //Advanced
    cAdvanced_->Show(m_transfer);
    cAdvanced_->SetValue(m_advanced && m_transfer);
    toTextAmount_->Show(m_advanced && m_transfer);

    bSwitch_->Show(m_transfer);

    //Amounts
    if (!skip_amount_init_) {
        if (m_transfer && m_advanced)
            toTextAmount_->SetValue(m_journal_data.m_to_amount);
        else
            toTextAmount_->ChangeValue("");

        if (m_mode != MODE_NEW)
            m_textAmount->SetValue(m_journal_data.m_amount);
        skip_amount_init_ = true;
    }

    //Payee
    if (!skip_payee_init_) {
        cbPayee_->SetEvtHandlerEnabled(false);

        cbAccount_->UnsetToolTip();
        cbPayee_->UnsetToolTip();
        if (!m_transfer) {
            if (!TrxModel::is_foreign(m_journal_data)) {
                m_journal_data.m_to_account_id_n = -1;
            }

            int64 accountID = cbAccount_->mmGetId();
            if (m_mode == MODE_NEW
                && PrefModel::instance().getTransPayeeNone() == PrefModel::LASTUSED
                && (accountID != -1)
            ) {
                TrxModel::DataA transactions = T.find(
                    TrxModel::TYPE(OP_NE, TrxType(TrxType::e_transfer)),
                    TrxCol::ACCOUNTID(OP_EQ, accountID));

                if (!transactions.empty()) {
                    const PayeeData* payee_n = P.get_id_data_n(transactions.back().m_payee_id_n);
                    cbPayee_->ChangeValue(payee_n->m_name);
                }
            }
            else if (m_mode == MODE_NEW
                && PrefModel::instance().getTransPayeeNone() == PrefModel::UNUSED
            ) {
                const PayeeData* payee_n = P.get_name_data_n(_t("Unknown"));
                if (!payee_n) {
                    PayeeData new_payee_d = PayeeData();
                    new_payee_d.m_name = _t("Unknown");
                    P.add_data_n(new_payee_d);
                    cbPayee_->mmDoReInitialize();
                }

                cbPayee_->ChangeValue(_t("Unknown"));
            }
            else {
                const PayeeData* payee_n = P.get_id_data_n(m_journal_data.m_payee_id_n);
                if (payee_n)
                    cbPayee_->ChangeValue(payee_n->m_name);
            }

            SetCategoryForPayee();
        }
        skip_payee_init_ = true;
        cbPayee_->SetEvtHandlerEnabled(true);
    }

    cbPayee_->Show(!m_transfer);
    payee_label_->Show(!m_transfer);
    to_acc_label_->Show(m_transfer);
    cbToAccount_->Show(m_transfer);
    Layout();

    bool has_split = !m_local_splits.empty();
    if (!skip_category_init_) {
        bSplit_->UnsetToolTip();
        if (has_split)
        {
            cbCategory_->ChangeValue(_t("Split Transaction"));
            cbCategory_->Disable();
            m_textAmount->SetValue(TrxSplitModel::instance().get_total(m_local_splits));
            m_journal_data.m_category_id_n = -1;
        }
        else if (m_mode == MODE_NEW && m_transfer &&
            PrefModel::instance().getTransCategoryTransferNone() == PrefModel::LASTUSED
        ) {
            TrxModel::DataA transactions = T.find(
                TrxModel::TYPE(OP_EQ, TrxType(TrxType::e_transfer))
            );

            if (!transactions.empty() &&
                CategoryModel::instance().get_id_active(transactions.back().m_category_id_n)
            ) {
                int64 cat_id = transactions.back().m_category_id_n;
                const wxString cat_fullname = CategoryModel::instance().get_id_fullname(cat_id);
                cbCategory_->ChangeValue(cat_fullname);
            }
        }
        else {
            int64 cat_id = m_journal_data.m_category_id_n;
            const wxString cat_fullname = CategoryModel::instance().get_id_fullname(cat_id);
            cbCategory_->ChangeValue(cat_fullname);
        }
        skip_category_init_ = true;
    }

    m_textAmount->Enable(!has_split);
    bCalc_->Enable(!has_split);
    cbCategory_->Enable(!has_split);
    bSplit_->Enable(!m_transfer);

    // Tags
    if (!skip_tag_init_) {
        wxArrayInt64 tag_id_a;
        for (const auto& gl_d : GL.find(
            TagLinkCol::REFTYPE(m_journal_data.key().ref_type().name_n()),
            TagLinkCol::REFID(m_journal_data.key().ref_id())
        ))
            tag_id_a.push_back(gl_d.m_tag_id);
        tagTextCtrl_->SetTags(tag_id_a);
        skip_tag_init_ = true;
    }

    //Notes & Transaction Number
    if (!skip_notes_init_) {
        textNumber_->SetValue(m_journal_data.m_number);
        textNotes_->SetValue(m_journal_data.m_notes);
        skip_notes_init_ = true;
    }

    if (!skip_tooltips_init_)
        SetTooltips();

    if (m_journal_data.is_deleted()) {
        dpc_->Enable(false);
        transaction_type_->Enable(false);
        cbAccount_->Enable(false);
        choiceStatus_->Enable(false);
        m_textAmount->Enable(false);
        bCalc_->Enable(false);
        cbToAccount_->Enable(false);
        toTextAmount_->Enable(false);
        cAdvanced_->Enable(false);
        cbPayee_->Enable(false);
        cbCategory_->Enable(false);
        tagTextCtrl_->Enable(false);
        bSplit_->Enable(false);
        bAuto->Enable(false);
        textNumber_->Enable(false);
        textNotes_->Enable(false);
        bColours_->Enable(false);
        bAttachments_->Enable(false);
        bFrequentUsedNotes->Enable(false);
    }
}

void TrxDialog::CreateControls()
{
    wxBoxSizer* box_sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* box_sizer1 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* box_sizer2 = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* box_sizer3 = new wxBoxSizer(wxHORIZONTAL);
    box_sizer->Add(box_sizer1, g_flagsExpand);
    box_sizer1->Add(box_sizer2, wxSizerFlags(g_flagsExpand).Border(0));
    box_sizer1->Add(box_sizer3, wxSizerFlags(g_flagsV).Expand().Border(0));

    wxStaticBox* static_box = new wxStaticBox(this, wxID_ANY, _t("Transaction Details"));
    wxStaticBoxSizer* box_sizer_left = new wxStaticBoxSizer(static_box, wxVERTICAL);
    wxFlexGridSizer* flex_sizer = new wxFlexGridSizer(0, 3, 0, 0);
    flex_sizer->AddGrowableCol(1, 0);
    box_sizer_left->Add(flex_sizer, wxSizerFlags(g_flagsV).Expand());
    box_sizer2->Add(box_sizer_left, g_flagsExpand);

    wxFont bold = this->GetFont().Bold();
    // Date -------------------------------------------
    wxStaticText* name_label = new wxStaticText(static_box, wxID_STATIC, _t("Date"));
    flex_sizer->Add(name_label, g_flagsH);
    name_label->SetFont(bold);

    dpc_ = new mmDatePickerCtrl(static_box, ID_DIALOG_TRANS_BUTTONDATE);
    flex_sizer->Add(dpc_->mmGetLayout());

    wxBitmapBundle bundle = mmBitmapBundle(png::ACC_CLOCK, mmBitmapButtonSize);
    wxBitmapButton* today = new wxBitmapButton(static_box, ID_DIALOG_TRANS_TODAY, bundle);
    today->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TrxDialog::OnToday), nullptr, this);

    const wxString tooltiptext = _t("Set date to today (Ctrl+;)");
    mmToolTip(today, tooltiptext);
    wxChar hotkeyToday = ExtractHotkeyChar(tooltiptext, '.');
    flex_sizer->Add(today, g_flagsH);

    // Type --------------------------------------------
    transaction_type_ = new wxChoice(static_box, ID_DIALOG_TRANS_TYPE);

    for (int i = 0; i < TrxType::size; ++i) {
        if (i != TrxType::e_transfer || AccountModel::instance().find_all().size() > 1) {
            wxString type = TrxType(i).name();
            transaction_type_->Append(wxGetTranslation(type), new wxStringClientData(type));
        }
    }

    cAdvanced_ = new wxCheckBox(static_box,
        ID_DIALOG_TRANS_ADVANCED_CHECKBOX,
        _t("&Advanced"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
    );

    wxBoxSizer* typeSizer = new wxBoxSizer(wxHORIZONTAL);

    flex_sizer->Add(new wxStaticText(static_box, wxID_STATIC, _t("Type")), g_flagsH);
    flex_sizer->Add(typeSizer, wxSizerFlags(g_flagsExpand).Border(0));
    typeSizer->Add(transaction_type_, g_flagsExpand);
    typeSizer->Add(cAdvanced_, g_flagsH);
    flex_sizer->AddSpacer(1);

    // Amount Fields --------------------------------------------
    m_textAmount = new mmTextCtrl(static_box, mmID_TEXTAMOUNT, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    m_textAmount->SetMinSize(m_textAmount->GetSize());
    toTextAmount_ = new mmTextCtrl( static_box, mmID_TOTEXTAMOUNT, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    toTextAmount_->SetMinSize(toTextAmount_->GetSize());
    wxBoxSizer* amountSizer = new wxBoxSizer(wxHORIZONTAL);
    amountSizer->Add(m_textAmount, g_flagsExpand);
    amountSizer->Add(toTextAmount_, g_flagsExpand);

    wxStaticText* amount_label = new wxStaticText(static_box, wxID_STATIC, _t("Amount"));
    amount_label->SetFont(bold);
    flex_sizer->Add(amount_label, g_flagsH);
    flex_sizer->Add(amountSizer, wxSizerFlags(g_flagsExpand).Border(0));

    bCalc_ = new wxBitmapButton(static_box, wxID_ANY, mmBitmapBundle(png::CALCULATOR, mmBitmapButtonSize));
    bCalc_->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TrxDialog::OnCalculator), nullptr, this);
    mmToolTip(bCalc_, _t("Open Calculator"));
    flex_sizer->Add(bCalc_, g_flagsH);
    calcTarget_ = m_textAmount;
    calcPopup_ = new mmCalculatorPopup(bCalc_, calcTarget_);

    // Account ---------------------------------------------
    account_label_ = new wxStaticText(static_box, wxID_STATIC, _t("Account"));
    account_label_->SetFont(bold);

    cbAccount_ = new mmComboBoxAccount(static_box, mmID_ACCOUNTNAME, wxDefaultSize, m_journal_data.m_account_id);
    cbAccount_->SetMinSize(cbAccount_->GetSize());
    flex_sizer->Add(account_label_, g_flagsH);
    flex_sizer->Add(cbAccount_, g_flagsExpand);

    bSwitch_ = new wxBitmapButton(static_box, wxID_ANY, mmBitmapBundle(png::UPDATE, mmBitmapButtonSize));
    bSwitch_->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TrxDialog::OnSwitch), nullptr, this);
    mmToolTip(bSwitch_, _t("Exchange to and from accounts"));
    flex_sizer->Add(bSwitch_, g_flagsH);

    // To Account ------------------------------------------------
    to_acc_label_ = new wxStaticText(static_box, mmID_TOACCOUNT_LABEL, _t("To"));
    to_acc_label_->SetFont(bold);
    flex_sizer->Add(to_acc_label_, g_flagsH);
    cbToAccount_ = new mmComboBoxAccount(static_box, mmID_TOACCOUNTNAME, wxDefaultSize, m_journal_data.m_to_account_id_n);
    cbToAccount_->SetMinSize(cbToAccount_->GetSize());
    flex_sizer->Add(cbToAccount_, g_flagsExpand);
    flex_sizer->AddSpacer(1);

    // Payee ---------------------------------------------
    payee_label_ = new wxStaticText(static_box, mmID_PAYEE_LABEL, _t("Payee"));
    payee_label_->SetFont(bold);

    cbPayee_ = new mmComboBoxPayee(static_box, mmID_PAYEE, wxDefaultSize, m_journal_data.m_payee_id_n, true);
    cbPayee_->SetMinSize(cbPayee_->GetSize());
    flex_sizer->Add(payee_label_, g_flagsH);
    flex_sizer->Add(cbPayee_, g_flagsExpand);
    flex_sizer->AddSpacer(1);

    // Category -------------------------------------------------
    categ_label_ = new wxStaticText(static_box, ID_DIALOG_TRANS_CATEGLABEL2, _t("Category"));
    categ_label_->SetFont(bold);
    cbCategory_ = new mmComboBoxCategory(static_box, mmID_CATEGORY, wxDefaultSize
        , m_journal_data.m_category_id_n, true);
    cbCategory_->SetMinSize(cbCategory_->GetSize());
    bSplit_ = new wxBitmapButton(static_box, mmID_CATEGORY_SPLIT, mmBitmapBundle(png::NEW_TRX, mmBitmapButtonSize));
    mmToolTip(bSplit_, _t("Use split Categories"));

    flex_sizer->Add(categ_label_, g_flagsH);
    flex_sizer->Add(cbCategory_, g_flagsExpand);
    flex_sizer->Add(bSplit_, g_flagsH);

    // Tags  ---------------------------------------------
    tagTextCtrl_ = new mmTagTextCtrl(static_box, ID_DIALOG_TRANS_TAGS);
    wxStaticText* tagLabel = new wxStaticText(static_box, wxID_STATIC, _t("Tags"));
    flex_sizer->Add(tagLabel, g_flagsH);
    flex_sizer->Add(tagTextCtrl_, g_flagsExpand);
    flex_sizer->AddSpacer(1);

    // Status --------------------------------------------
    choiceStatus_ = new wxChoice(static_box, ID_DIALOG_TRANS_STATUS);

    for (int i = 0; i < TrxStatus::size; ++i) {
        wxString status = TrxStatus(i).name();
        choiceStatus_->Append(wxGetTranslation(status), new wxStringClientData(status));
    }

    flex_sizer->Add(new wxStaticText(static_box, wxID_STATIC, _t("Status")), g_flagsH);
    flex_sizer->Add(choiceStatus_, g_flagsExpand);
    flex_sizer->AddSpacer(1);

    // Number  ---------------------------------------------
    textNumber_ = new wxTextCtrl(static_box, ID_DIALOG_TRANS_TEXTNUMBER, "", wxDefaultPosition, wxDefaultSize);

    bAuto = new wxBitmapButton(static_box, ID_DIALOG_TRANS_BUTTONTRANSNUM, mmBitmapBundle(png::TRXNUM, mmBitmapButtonSize));
    bAuto->Connect(ID_DIALOG_TRANS_BUTTONTRANSNUM, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TrxDialog::OnAutoTransNum), nullptr, this);
    mmToolTip(bAuto, _t("Populate Transaction #"));

    flex_sizer->Add(new wxStaticText(static_box, wxID_STATIC, _t("Number")), g_flagsH);
    flex_sizer->Add(textNumber_, g_flagsExpand);
    flex_sizer->Add(bAuto, g_flagsH);

    // Frequently Used Notes
    wxButton* bFrequentUsedNotes = new wxButton(static_box, ID_DIALOG_TRANS_BUTTON_FREQENTNOTES
        , "...", wxDefaultPosition, bAuto->GetSize(), 0);
    mmToolTip(bFrequentUsedNotes, _t("Select one of the frequently used notes"));
    bFrequentUsedNotes->Connect(ID_DIALOG_TRANS_BUTTON_FREQENTNOTES
        , wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(TrxDialog::OnFrequentUsedNotes), nullptr, this);

    // Colors
    bColours_ = new mmColorButton(static_box, wxID_LOWEST, bAuto->GetSize());
    mmToolTip(bColours_, _t("User Colors"));
    bColours_->SetColor(m_journal_data.m_color.GetValue());

    // Attachments
    bAttachments_ = new wxBitmapButton(static_box, wxID_FILE, mmBitmapBundle(png::CLIP, mmBitmapButtonSize));
    mmToolTip(bAttachments_, _t("Manage transaction attachments"));

    // Now display the Frequently Used Notes, Colour, Attachment buttons
    wxBoxSizer* notes_sizer = new wxBoxSizer(wxHORIZONTAL);
    flex_sizer->Add(notes_sizer);
    notes_sizer->Add(new wxStaticText(static_box, wxID_STATIC, _t("Notes")), g_flagsH);
    notes_sizer->Add(bFrequentUsedNotes, g_flagsH);

    wxBoxSizer* RightAlign_sizer = new wxBoxSizer(wxHORIZONTAL);
    flex_sizer->Add(RightAlign_sizer, wxSizerFlags(g_flagsH).Align(wxALIGN_RIGHT));
    RightAlign_sizer->Add(new wxStaticText(static_box, wxID_STATIC, _t("Color")), g_flagsH);
    RightAlign_sizer->Add(bColours_, wxSizerFlags());
    flex_sizer->Add(bAttachments_, g_flagsH);

    // Notes
    textNotes_ = new wxTextCtrl(static_box, ID_DIALOG_TRANS_TEXTNOTES, ""
        , wxDefaultPosition, wxSize(-1, dpc_->GetSize().GetHeight() * 5), wxTE_MULTILINE);
    mmToolTip(textNotes_, _t("Specify any text notes you want to add to this transaction."));
    box_sizer_left->Add(textNotes_, wxSizerFlags(g_flagsExpand).Border(wxLEFT | wxRIGHT | wxBOTTOM, 10));

    /**********************************************************************************************
     Button Panel with OK and Cancel Buttons
    ***********************************************************************************************/
    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    box_sizer2->Add(buttons_panel, wxSizerFlags(g_flagsV).Center().Border(wxALL, 0));

    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    wxBitmapButton* button_hide = new wxBitmapButton(buttons_panel, ID_DIALOG_TRANS_CUSTOMFIELDS, mmBitmapBundle(png::RIGHTARROW, mmBitmapButtonSize));
    mmToolTip(button_hide, _t("Show/Hide custom fields window"));
    if (m_custom_fields->GetCustomFieldsCount() == 0) {
        button_hide->Hide();
    }

    buttons_sizer->Add(new wxButton(buttons_panel, wxID_OK, _t("&Save")), wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));
    if (m_mode == MODE_NEW || m_mode == MODE_DUP) {
        buttons_sizer->Add(new wxButton(buttons_panel, ID_BTN_OK_NEW, _t("Save and &New")), wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));
    }
    m_button_cancel = new wxButton(buttons_panel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    buttons_sizer->Add(m_button_cancel, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));
    buttons_sizer->Add(button_hide, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));

    if (m_mode == MODE_EDIT) m_button_cancel->SetFocus();
    mmThemeAutoColour(buttons_panel, true);
    buttons_sizer->Realize();

    // Custom fields -----------------------------------

    m_custom_fields->FillCustomFields(box_sizer3);
    if (m_custom_fields->GetActiveCustomFieldsCount() > 0) {
        wxCommandEvent evt(wxEVT_BUTTON, ID_DIALOG_TRANS_CUSTOMFIELDS);
        OnMoreFields(evt);
    }

    this->SetSizerAndFit(box_sizer);
    wxSize panelSize = box_sizer2->GetMinSize();
    min_size_ = wxSize(panelSize.GetWidth() + 20, panelSize.GetHeight());
    SetMinSize(min_size_);
    box_sizer3->SetMinSize(panelSize);
    m_custom_fields->SetMinSize(panelSize);

    const wxAcceleratorEntry entries[] = {
        wxAcceleratorEntry(wxACCEL_CTRL, static_cast<int>('S'), wxID_SAVE),
        wxAcceleratorEntry(wxACCEL_CTRL, static_cast<int>('N'), ID_BTN_OK_NEW),
        wxAcceleratorEntry(wxACCEL_CTRL, static_cast<int>(hotkeyToday), ID_DIALOG_TRANS_TODAY),
    };
    wxAcceleratorTable tab(sizeof(entries) / sizeof(*entries), entries);
    SetAcceleratorTable(tab);
}

bool TrxDialog::ValidateData()
{
    if (!m_textAmount->checkValue(m_journal_data.m_amount))
        return false;
    if (!tagTextCtrl_->IsValid()) {
        mmErrorDialogs::ToolTip4Object(tagTextCtrl_, _t("Invalid value"), _t("Tags"), wxICON_ERROR);
        return false;
    }
    if (!cbAccount_->mmIsValid()) {
        mmErrorDialogs::ToolTip4Object(cbAccount_, _t("Invalid value"), _t("Account"), wxICON_ERROR);
        return false;
    }
    m_journal_data.m_account_id = cbAccount_->mmGetId();
    const AccountData* account_n = AccountModel::instance().get_id_data_n(m_journal_data.m_account_id);

    if (m_journal_data.m_date() < account_n->m_open_date) {
        mmErrorDialogs::ToolTip4Object(
            cbAccount_,
            _t("The opening date for the account is later than the date of this transaction"),
            _t("Invalid Date")
        );
        return false;
    }

    if (m_local_splits.empty()) {
        if (!cbCategory_->mmIsValid()) {
            mmErrorDialogs::ToolTip4Object(cbCategory_, _t("Invalid value"), _t("Category"), wxICON_ERROR);
            return false;
        }
        m_journal_data.m_category_id_n = cbCategory_->mmGetCategoryId();
    }

    if (!m_transfer) {
        wxString payee_name = cbPayee_->GetValue();
        if (payee_name.IsEmpty())
        {
            mmErrorDialogs::InvalidPayee(cbPayee_);
            return false;
        }

        // Get payee string from populated list to address issues with case compare differences between autocomplete and payee list
        int payee_loc = cbPayee_->FindString(payee_name);
        if (payee_loc != wxNOT_FOUND)
            payee_name = cbPayee_->GetString(payee_loc);

        const PayeeData* payee_n = PayeeModel::instance().get_name_data_n(payee_name);
        if (!payee_n) {
            wxMessageDialog msgDlg(this,
                wxString::Format(_t("Payee name has not been used before. Is the name correct?\n%s"), payee_name),
                _t("Confirm payee name"),
                wxYES_NO | wxYES_DEFAULT | wxICON_WARNING
            );
            if (msgDlg.ShowModal() == wxID_YES) {
                PayeeData new_payee_d = PayeeData();
                new_payee_d.m_name = payee_name;
                PayeeModel::instance().add_data_n(new_payee_d);
                payee_n = PayeeModel::instance().get_id_data_n(new_payee_d.m_id);
                mmWebApp::uploadPayee();
            }
            else
                return false;
        }
        m_journal_data.m_to_amount = m_journal_data.m_amount;
        m_journal_data.m_payee_id_n = payee_n->m_id;
        if (!TrxModel::is_foreign(m_journal_data)) {
            m_journal_data.m_to_account_id_n = -1;
        }

        if (PrefModel::instance().getTransCategoryNone() == PrefModel::LASTUSED
            && CategoryModel::instance().get_id_active(m_journal_data.m_category_id_n)
        ) {
            PayeeData payee_d = *payee_n;
            payee_d.m_category_id_n = m_journal_data.m_category_id_n;
            PayeeModel::instance().save_data_n(payee_d);
            mmWebApp::uploadPayee();
        }
    }
    else //transfer
    {
        const AccountData *to_account = AccountModel::instance().get_name_data_n(cbToAccount_->GetValue());

        if (!to_account || to_account->m_id == m_journal_data.m_account_id) {
            mmErrorDialogs::InvalidAccount(cbToAccount_, true);
            return false;
        }
        m_journal_data.m_to_account_id_n = to_account->m_id;

        if (m_journal_data.m_date() < to_account->m_open_date) {
            mmErrorDialogs::ToolTip4Object(cbToAccount_, _t("The opening date for the account is later than the date of this transaction"), _t("Invalid Date"));
            return false;
        }

        if (m_advanced) {
            if (!toTextAmount_->checkValue(m_journal_data.m_to_amount))
                return false;
        }
        m_journal_data.m_payee_id_n = -1;
    }

    /* Check if transaction is to proceed.*/
    if (account_n->is_locked_for(mmDate(dpc_->GetValue()))) {
        if (wxMessageBox(wxString::Format(
            _t("Lock transaction to date: %s") + "\n\n" + _t("Do you want to continue?"),
            mmGetDateTimeForDisplay(account_n->m_stmt_date_n.value().isoDate())),
            _t("MMEX Transaction Check"),
            wxYES_NO | wxICON_WARNING
        ) == wxNO) {
            return false;
        }
    }

    // Checking account does not exceed limits
    if (m_mode != MODE_EDIT) {
        if (!m_journal_data.is_void() &&
            (m_journal_data.is_withdrawal() || m_journal_data.is_transfer()) &&
            (account_n->m_min_balance != 0 || account_n->m_credit_limit != 0)
        ) {
            const double balance = AccountModel::instance().get_data_balance(*account_n);
            const double new_value = balance - m_journal_data.m_amount;

            bool abort_transaction =
                (account_n->m_min_balance != 0 && new_value < account_n->m_min_balance) ||
                (account_n->m_credit_limit != 0 && new_value < -(account_n->m_credit_limit));

            if (abort_transaction && wxMessageBox(
                _t("The transaction will exceed the account limit.") + "\n\n" +
                    _t("Do you want to continue?"),
                _t("MMEX Transaction Check"),
                wxYES_NO | wxICON_WARNING
            ) == wxNO) {
                return false;
            }
        }
    }

    int color_id = bColours_->GetColorId();
    if (color_id > 0 && color_id < 8)
        m_journal_data.m_color = color_id;
    else
        m_journal_data.m_color = -1;

    return true;
}

//----------------------------------------------------------------------------
// Workaround for bug http://trac.wxwidgets.org/ticket/11630
void TrxDialog::OnDpcKillFocus(wxFocusEvent& event)
{
    if (wxGetKeyState(WXK_TAB) && wxGetKeyState(WXK_SHIFT))
        m_button_cancel->SetFocus();
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

void TrxDialog::OnFocusChange(wxChildFocusEvent& event)
{
    wxWindow* w = event.GetWindow();
    if (!w || object_in_focus_ == w->GetId()) {
        return;
    }

    switch (object_in_focus_)
    {
    case mmID_ACCOUNTNAME:
        cbAccount_->ChangeValue(cbAccount_->GetValue());
        if (cbAccount_->mmIsValid())
        {
            m_journal_data.m_account_id = cbAccount_->mmGetId();
            skip_account_init_ = false;
        }
        break;
    case mmID_TOACCOUNTNAME:
        cbToAccount_->ChangeValue(cbToAccount_->GetValue());
        if (cbToAccount_->mmIsValid())
        {
            m_journal_data.m_to_account_id_n = cbToAccount_->mmGetId();
            skip_account_init_ = false;
        }
        break;
    case mmID_PAYEE:
        cbPayee_->ChangeValue(cbPayee_->GetValue());
        m_journal_data.m_payee_id_n = cbPayee_->mmGetId();
        SetCategoryForPayee();
        break;
    case mmID_CATEGORY:
        cbCategory_->ChangeValue(cbCategory_->GetValue());
        break;
    case mmID_TEXTAMOUNT:
        if (m_textAmount->Calculate()) {
            m_textAmount->GetDouble(m_journal_data.m_amount);
        }
        skip_amount_init_ = false;
        calcTarget_ = m_textAmount;
        break;
    case mmID_TOTEXTAMOUNT:
        if (toTextAmount_->Calculate()) {
            toTextAmount_->GetDouble(m_journal_data.m_to_amount);
        }
        skip_amount_init_ = false;
        calcTarget_ = toTextAmount_;
        break;
    }

    object_in_focus_ = w->GetId();

    if (!m_transfer)
    {
        toTextAmount_->ChangeValue("");
        m_journal_data.m_to_account_id_n = -1;
    }
    else
    {
        const AccountData* to_account_n = AccountModel::instance().get_id_data_n(cbToAccount_->mmGetId());
        if (to_account_n)
            m_journal_data.m_to_account_id_n = to_account_n->m_id;
    }

    dataToControls();
    event.Skip();
}

void TrxDialog::SetDialogTitle(const wxString& title)
{
    this->SetTitle(title);
}

void TrxDialog::OnPayeeChanged(wxCommandEvent& /*event*/)
{
    const PayeeData * payee = PayeeModel::instance().get_name_data_n(cbPayee_->GetValue());
    if (payee) {
        SetCategoryForPayee(payee);
    }
}

void TrxDialog::OnTransTypeChanged(wxCommandEvent& event)
{
    const TrxType old_type = m_journal_data.m_type;
    wxStringClientData *client_obj = static_cast<wxStringClientData*>(event.GetClientObject());
    if (client_obj)
        m_journal_data.m_type = TrxType(client_obj->GetData());
    if (old_type.id() != m_journal_data.m_type.id()) {
        m_transfer = m_journal_data.is_transfer();
        if (m_transfer || old_type.id() == TrxType::e_transfer)
            skip_payee_init_ = false;
        else
            skip_payee_init_ = true;
        skip_account_init_ = true;
        skip_tooltips_init_ = false;

        if (m_transfer) {
            m_journal_data.m_payee_id_n = -1;
            skip_category_init_ = false;
        } else {
            m_journal_data.m_to_amount = m_journal_data.m_amount;
            m_journal_data.m_to_account_id_n = -1;
        }
        dataToControls();
    }
}

void TrxDialog::OnComboKey(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_RETURN)
    {
        auto id = event.GetId();
        switch (id)
        {
        case mmID_PAYEE:
        {
            const auto payeeName = cbPayee_->GetValue();
            if (payeeName.empty()) {
                mmPayeeDialog dlg(this, true);
                dlg.ShowModal();
                if (dlg.getRefreshRequested())
                    cbPayee_->mmDoReInitialize();
                int64 payee_id = dlg.getPayeeId();
                const PayeeData* payee_n = PayeeModel::instance().get_id_data_n(payee_id);
                if (payee_n) {
                    cbPayee_->ChangeValue(payee_n->m_name);
                    cbPayee_->SetInsertionPointEnd();
                    SetCategoryForPayee(payee_n);
                }
                return;
            }
        }
        break;
        case mmID_CATEGORY:
        {
            auto category = cbCategory_->GetValue();
            if (category.empty())
            {
                CategoryManager dlg(this, true, -1);
                int rc = dlg.ShowModal();
                if (dlg.getRefreshRequested())
                    cbCategory_->mmDoReInitialize();
                if (rc != wxID_CANCEL)
                    cbCategory_->ChangeValue(CategoryModel::instance().get_id_fullname(dlg.getCategId()));
                return;
            }
        }
        break;
        default:
            break;
        }
    }

    // The first time the ALT key is pressed accelerator hints are drawn, but custom painting on the tags button
    // is not applied. We need to refresh the tag ctrl to redraw the drop button with the correct image.
    if (event.AltDown() && !altRefreshDone)
    {
        tagTextCtrl_->Refresh();
        altRefreshDone = true;
    }

    event.Skip();
}

void TrxDialog::SetCategoryForPayee(const PayeeData *payee_n)
{
    // Only for new transactions: if user does not want to use categories.
    // If this is a Split Transaction, ignore displaying last category for payee
    if (m_mode == MODE_NEW
        && PrefModel::instance().getTransCategoryNone() == PrefModel::UNUSED
        && m_local_splits.empty()
    ) {
        const CategoryData* category_n = CategoryModel::instance().get_key_data_n(
            _t("Unknown"), int64(-1)
        );
        if (!category_n) {
            CategoryData new_category_d = CategoryData();
            new_category_d.m_name = _t("Unknown");
            CategoryModel::instance().add_data_n(new_category_d);
            category_n = CategoryModel::instance().get_id_data_n(new_category_d.m_id);
            cbCategory_->mmDoReInitialize();
        }

        m_journal_data.m_category_id_n = category_n->m_id;
        cbCategory_->ChangeValue(_t("Unknown"));
        return;
    }

    if (!payee_n) {
        payee_n = PayeeModel::instance().get_name_data_n(cbPayee_->GetValue());
        if (!payee_n)
            return;
    }

    // Only for new transactions: if user want to autofill last category used for payee.
    // If this is a Split Transaction, ignore displaying last category for payee
    if ((PrefModel::instance().getTransCategoryNone() == PrefModel::LASTUSED ||
            PrefModel::instance().getTransCategoryNone() == PrefModel::DEFAULT
        ) &&
        m_mode == MODE_NEW && m_local_splits.empty() &&
        CategoryModel::instance().get_id_active(payee_n->m_category_id_n)
    ) {
        // if payee has memory of last category used then display last category for payee
        const CategoryData* category_n = CategoryModel::instance().get_id_data_n(payee_n->m_category_id_n);
        if (category_n) {
            m_journal_data.m_category_id_n = payee_n->m_category_id_n;
            cbCategory_->ChangeValue(CategoryModel::instance().get_id_fullname(payee_n->m_category_id_n));
            wxLogDebug("Category: %s = %.2f", cbCategory_->GetLabel(), m_journal_data.m_amount);
        }
        else {
            m_journal_data.m_category_id_n = -1;
            cbCategory_->ChangeValue("");
        }
    }
}

void TrxDialog::OnCalculator(wxCommandEvent& WXUNUSED(event))
{
    calcPopup_->SetTarget(calcTarget_);
    calcPopup_->Popup();
}

void TrxDialog::OnSwitch(wxCommandEvent& WXUNUSED(event))
{
    wxString temp = cbToAccount_->GetValue();
    cbToAccount_->ChangeValue(cbAccount_->GetValue());
    cbAccount_->ChangeValue(temp);
}

void TrxDialog::OnToday(wxCommandEvent& WXUNUSED(event))
{
    dpc_->SetValue(wxDateTime::Today());
}

void TrxDialog::OnAutoTransNum(wxCommandEvent& WXUNUSED(event))
{
    mmDate date = mmDate(m_journal_data.m_date().getDateTime().Subtract(wxDateSpan::Months(12)));
    double next_number = 0, temp_num;
    const auto numbers = TrxModel::instance().find(
        TrxModel::DATE(OP_GE, date),
        TrxCol::ACCOUNTID(OP_EQ, m_journal_data.m_account_id),
        TrxCol::TRANSACTIONNUMBER(OP_NE, "")
    );
    for (const auto &num : numbers) {
        if (!num.m_number.IsNumber()) continue;
        if (num.m_number.ToDouble(&temp_num) && temp_num > next_number)
            next_number = temp_num;
    }

    next_number++;
    textNumber_->SetValue(wxString::FromDouble(next_number, 0));
}

void TrxDialog::OnAdvanceChecked(wxCommandEvent& WXUNUSED(event))
{
    m_advanced = cAdvanced_->IsChecked();
    skip_amount_init_ = false;
    dataToControls();
}

void TrxDialog::OnCategs(wxCommandEvent& WXUNUSED(event))
{
    if (!m_textAmount->GetDouble(m_journal_data.m_amount)) {
        m_journal_data.m_amount = 0;
    }

    if (cbCategory_->IsEnabled() && !cbCategory_->GetValue().IsEmpty() && !cbCategory_->mmIsValid()) {
        mmErrorDialogs::ToolTip4Object(cbCategory_, _t("Invalid value"), _t("Category"), wxICON_ERROR);
        return;
    }
    wxLogDebug("Cat Valid %d, Cat Is Empty %d, Cat value [%s]", cbCategory_->mmIsValid(), cbCategory_->GetValue().IsEmpty(), cbCategory_->GetValue());
    if (m_local_splits.empty()) {
        Split split_d;
        split_d.m_amount = m_journal_data.m_amount;
        if (cbCategory_->mmIsValid())
            split_d.m_category_id = cbCategory_->mmGetCategoryId();
        m_local_splits.push_back(split_d);
    }

    SplitDialog dlg(this, m_local_splits, m_journal_data.m_account_id);

    if (dlg.ShowModal() == wxID_OK)
    {
        m_local_splits = dlg.mmGetResult();

        if (m_local_splits.size() == 1) {
            m_journal_data.m_category_id_n = m_local_splits[0].m_category_id;
            m_journal_data.m_amount = m_local_splits[0].m_amount;
            m_textAmount->SetValue(m_journal_data.m_amount);
            m_local_splits.clear();
        }

        if (!m_local_splits.empty()) {
            m_textAmount->SetValue(m_journal_data.m_amount);
        }

        skip_category_init_ = false;
        skip_amount_init_ = false;
        skip_tooltips_init_ = false;
        dataToControls();
    }
    tagTextCtrl_->Reinitialize();
}

void TrxDialog::OnAttachments(wxCommandEvent& WXUNUSED(event))
{
    RefTypeN ref_type = m_journal_data.key().ref_type();
    // FIXME: the following assumes that ref_type is Trx
    int64 transID = (m_mode == MODE_DUP) ? -1 : m_journal_data.m_id;
    AttachmentDialog dlg(this, ref_type, transID);
    dlg.ShowModal();
}

void TrxDialog::OnTextEntered(wxCommandEvent& WXUNUSED(event))
{
    if (object_in_focus_ == m_textAmount->GetId())
    {
        if (m_textAmount->Calculate())
        {
            m_textAmount->GetDouble(m_journal_data.m_amount);
        }
    }
    else if (object_in_focus_ == toTextAmount_->GetId())
    {
        if (toTextAmount_->Calculate())
        {
            toTextAmount_->GetDouble(m_journal_data.m_to_amount);
        }
    }
    skip_amount_init_ = false;
    dataToControls();
}

void TrxDialog::OnFrequentUsedNotes(wxCommandEvent& WXUNUSED(event))
{
    if (!frequentNotes_.empty())
    {
        wxMenu menu;
        int id = wxID_HIGHEST;

        for (const auto& entry : frequentNotes_) {
            wxString label = entry.Mid(0, 36) + (entry.size() > 36 ? "..." : "");
            label.Replace("\n", " ");
            menu.Append(++id, label);
        }
        menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &TrxDialog::OnNoteSelected, this);
        PopupMenu(&menu);
    }
}

void TrxDialog::OnNoteSelected(wxCommandEvent& event)
{
    int i = event.GetId() - wxID_HIGHEST;

    if (i > 0 && static_cast<size_t>(i) <= frequentNotes_.size()) {
        if (!textNotes_->GetValue().EndsWith("\n") && !textNotes_->GetValue().empty())
            textNotes_->AppendText("\n");
        textNotes_->AppendText(frequentNotes_[i - 1]);
    }
}

void TrxDialog::OnOk(wxCommandEvent& event)
{
    m_journal_data.m_notes = textNotes_->GetValue();
    m_journal_data.m_number = textNumber_->GetValue();
    m_journal_data.m_date_time = dpc_->GetValue().FormatISOCombined();
    wxStringClientData* status_obj = static_cast<wxStringClientData*>(
        choiceStatus_->GetClientObject(choiceStatus_->GetSelection())
    );
    if (status_obj) {
        m_status = TrxStatus(status_obj->GetData()).key();
        m_journal_data.m_status = TrxStatus(m_status);
    }

    if (!ValidateData()) return;
    if (!m_custom_fields->ValidateCustomValues())
        return;

    if (!m_advanced)
        m_journal_data.m_to_amount = m_journal_data.m_amount;

    if (m_transfer && !m_advanced && (
        AccountModel::instance().get_id_currency_p(m_journal_data.m_account_id) !=
        AccountModel::instance().get_id_currency_p(m_journal_data.m_to_account_id_n)
    )) {
        wxMessageDialog msgDlg(this,
            _t("The two accounts have different currencies, but no advanced transaction is defined. Is this correct?"),
            _t("Currencies are different"),
            wxYES_NO | wxNO_DEFAULT | wxICON_WARNING
        );
        if (msgDlg.ShowModal() == wxID_NO)
            return;
    }

    TrxData* trx_n;
    TrxData trx_d;
    if (m_mode == MODE_EDIT) {
        trx_n = TrxModel::instance().unsafe_get_id_data_n(m_journal_data.m_id);
    }
    else {
        trx_d = TrxData();
        trx_n = &trx_d;
    }

    TrxModel::copy_from_trx(trx_n, m_journal_data);
    TrxModel::instance().unsafe_save_trx_n(trx_n);
    m_journal_data.m_id        = trx_n->id();
    m_journal_data.m_sched_id  = -1;
    m_journal_data.m_repeat_id = -1;

    TrxSplitModel::DataA tp_a;
    for (const auto& split_d : m_local_splits) {
        TrxSplitData tp_d = TrxSplitData();
        tp_d.m_category_id = split_d.m_category_id;
        tp_d.m_amount      = split_d.m_amount;
        tp_d.m_notes       = split_d.m_notes;
        tp_a.push_back(tp_d);
    }
    TrxSplitModel::instance().update_trx(m_journal_data.m_id, tp_a);

    // Save split tags
    for (unsigned int i = 0; i < m_local_splits.size(); i++) {
        TagLinkModel::DataA new_tp_gl_a;
        for (const auto& tag_id : m_local_splits.at(i).m_tag_id_a) {
            TagLinkData new_gl_d = TagLinkData();
            new_gl_d.m_tag_id   = tag_id;
            new_gl_d.m_ref_type = TrxSplitModel::s_ref_type;
            new_gl_d.m_ref_id   = tp_a.at(i).m_id;
            new_tp_gl_a.push_back(new_gl_d);
        }
        TagLinkModel::instance().update(
            TrxSplitModel::s_ref_type, tp_a.at(i).m_id,
            new_tp_gl_a
        );
    }
    if (m_mode != MODE_EDIT) {
        // FIXME
        mmAttachmentManage::RelocateAllAttachments(
            TrxModel::s_ref_type, -1,
            TrxModel::s_ref_type, m_journal_data.m_id
        );
    }

    m_custom_fields->SaveCustomValues(TrxModel::s_ref_type, m_journal_data.m_id);

    // Save base transaction tags
    TagLinkModel::DataA new_gl_a;
    for (const auto& tag_id : tagTextCtrl_->GetTagIDs()) {
        TagLinkData new_gl_d = TagLinkData();
        new_gl_d.m_tag_id   = tag_id;
        new_gl_d.m_ref_type = TrxModel::s_ref_type;
        new_gl_d.m_ref_id   = m_journal_data.m_id;
        new_gl_a.push_back(new_gl_d);
    }
    TagLinkModel::instance().update(
        TrxModel::s_ref_type.name_n(), m_journal_data.m_id,
        new_gl_a
    );

    //TrxModel::DataExt trx(trx_d);
    //wxLogDebug("%s", trx.to_json());

    if (event.GetId() == ID_BTN_OK_NEW) {
        previousDate = dpc_->GetValue();  // store date for next invocation
    }
    else {
        previousDate = wxDateTime();
    }

    EndModal((m_mode == MODE_NEW || m_mode == MODE_DUP) && (event.GetId() == ID_BTN_OK_NEW || wxGetKeyState(WXK_SHIFT)) ? wxID_NEW : wxID_OK);
}

void TrxDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
#ifdef __WXMSW__
    if (object_in_focus_ != wxID_CANCEL && wxGetKeyState(WXK_ESCAPE))
        return m_button_cancel->SetFocus();

    if (object_in_focus_ != wxID_CANCEL) {
        return;
    }
#endif

    if (m_mode != MODE_EDIT) {
        // FIXME: temporary records (with id -1) are not stored in database
        mmAttachmentManage::DeleteAllAttachments(TrxModel::s_ref_type, -1);
        FieldValueModel::instance().purge_ref(TrxModel::s_ref_type, -1);
    }
    previousDate = wxDateTime(); // invalidate!
    EndModal(wxID_CANCEL);
}

void TrxDialog::SetTooltips()
{
    bSplit_->UnsetToolTip();
    skip_tooltips_init_ = true;
    if (this->m_local_splits.empty())
        mmToolTip(bSplit_, _t("Use split Categories"));
    else {
        const CurrencyData* currency = CurrencyModel::instance().get_base_data_n();
        const AccountData* account_n = AccountModel::instance().get_id_data_n(m_journal_data.m_account_id);
        if (account_n)
            currency = AccountModel::instance().get_data_currency_p(*account_n);

        bSplit_->SetToolTip(TrxSplitModel::instance().get_tooltip(m_local_splits, currency));
    }
    if (m_mode != MODE_NEW) return;

    m_textAmount->UnsetToolTip();
    toTextAmount_->UnsetToolTip();
    cbAccount_->UnsetToolTip();
    cbPayee_->UnsetToolTip();

    if (m_transfer) {
        mmToolTip(cbAccount_, _t("Specify account the money is taken from"));
        mmToolTip(cbPayee_, _t("Specify account the money is moved to"));
        mmToolTip(m_textAmount, _t("Specify the transfer amount in the From Account."));

        if (m_advanced)
            mmToolTip(toTextAmount_, _t("Specify the transfer amount in the To Account"));
    }
    else {
        mmToolTip(m_textAmount, _t("Specify the amount for this transaction"));
        mmToolTip(cbAccount_, _t("Specify account for the transaction"));
        if (!m_journal_data.is_deposit())
            mmToolTip(cbPayee_, _t("Specify to whom the transaction is going to"));
        else
            mmToolTip(cbPayee_, _t("Specify where the transaction is coming from"));
    }

    // Not dynamically changed tooltips
    mmToolTip(dpc_, _t("Specify the date of the transaction"));
    mmToolTip(choiceStatus_, _t("Specify the status for the transaction"));
    mmToolTip(transaction_type_, _t("Specify the type of transactions to be created."));
    mmToolTip(textNumber_, _t("Specify any associated check number or transaction number"));
    mmToolTip(textNotes_, _t("Specify any text notes you want to add to this transaction."));
    mmToolTip(cAdvanced_, _t("Allows the setting of different amounts in the FROM and TO accounts."));
}

void TrxDialog::OnQuit(wxCloseEvent& WXUNUSED(event))
{
    if (m_mode != MODE_EDIT) {
        // FIXME: temporary records (with id -1) are not stored in database
        mmAttachmentManage::DeleteAllAttachments(TrxModel::s_ref_type, -1);
        FieldValueModel::instance().purge_ref(TrxModel::s_ref_type, -1);
    }
    EndModal(wxID_CANCEL);
}

void TrxDialog::OnMoreFields(wxCommandEvent& WXUNUSED(event))
{
    wxBitmapButton* button = static_cast<wxBitmapButton*>(FindWindow(ID_DIALOG_TRANS_CUSTOMFIELDS));

    if (button)
        button->SetBitmap(mmBitmapBundle(m_custom_fields->IsCustomPanelShown() ? png::RIGHTARROW : png::LEFTARROW, mmBitmapButtonSize));

    m_custom_fields->ShowHideCustomPanel();
    if (m_custom_fields->IsCustomPanelShown()) {
        SetMinSize(wxSize(min_size_.GetWidth() + m_custom_fields->GetMinWidth(), min_size_.GetHeight()));
        SetSize(wxSize(GetSize().GetWidth() + m_custom_fields->GetMinWidth(), GetSize().GetHeight()));
    }
    else {
        SetMinSize(min_size_);
        SetSize(wxSize(GetSize().GetWidth() - m_custom_fields->GetMinWidth(), GetSize().GetHeight()));
    }
}
