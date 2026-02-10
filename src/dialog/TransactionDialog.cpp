/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011-2022 Nikolay Akimov
 Copyright (C) 2011-2017 Stefano Giorgio [stef145g]
 Copyright (C) 2021, 2022 Mark Whalley (mark@ipx.co.uk)
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

#include <wx/numformatter.h>
#include <wx/timectrl.h>
#include <wx/collpane.h>
#include <wx/display.h>

#include "constants.h"
#include "util/util.h"
#include "paths.h"

#include "model/AccountModel.h"
#include "model/AttachmentModel.h"
#include "model/CategoryModel.h"
#include "model/CurrencyHistoryModel.h"
#include "model/FieldValueModel.h"
#include "model/PreferencesModel.h"
#include "model/SettingModel.h"
#include "model/TagModel.h"

#include "AttachmentDialog.h"
#include "CategoryDialog.h"
#include "PayeeDialog.h"
#include "SplitDialog.h"
#include "TransactionDialog.h"

#include "preferences/ViewPreferences.h"

#include "mmSimpleDialogs.h"
#include "mmTextCtrl.h"
#include "images_list.h"
#include "mmcustomdata.h"
#include "validators.h"
#include "webapp.h"

wxIMPLEMENT_DYNAMIC_CLASS(TransactionDialog, wxDialog);

wxBEGIN_EVENT_TABLE(TransactionDialog, wxDialog)
EVT_CHAR_HOOK(TransactionDialog::OnComboKey)
EVT_CHILD_FOCUS(TransactionDialog::OnFocusChange)
EVT_COMBOBOX(mmID_PAYEE, TransactionDialog::OnPayeeChanged)
EVT_TEXT(mmID_PAYEE, TransactionDialog::OnPayeeChanged)
EVT_BUTTON(mmID_CATEGORY_SPLIT, TransactionDialog::OnCategs)
EVT_CHOICE(ID_DIALOG_TRANS_TYPE, TransactionDialog::OnTransTypeChanged)
EVT_CHECKBOX(ID_DIALOG_TRANS_ADVANCED_CHECKBOX, TransactionDialog::OnAdvanceChecked)
EVT_BUTTON(wxID_FILE, TransactionDialog::OnAttachments)
EVT_BUTTON(ID_DIALOG_TRANS_CUSTOMFIELDS, TransactionDialog::OnMoreFields)
EVT_BUTTON(wxID_OK, TransactionDialog::OnOk)
EVT_BUTTON(ID_BTN_OK_NEW, TransactionDialog::OnOk)
EVT_BUTTON(wxID_CANCEL, TransactionDialog::OnCancel)
EVT_BUTTON(ID_DIALOG_TRANS_TODAY, TransactionDialog::OnToday)
EVT_CLOSE(TransactionDialog::OnQuit)

EVT_MENU(wxID_SAVE, TransactionDialog::TransactionDialog::OnOk)
EVT_MENU(ID_BTN_OK_NEW, TransactionDialog::TransactionDialog::OnOk)
EVT_MENU(ID_DIALOG_TRANS_TODAY, TransactionDialog::OnToday)
wxEND_EVENT_TABLE()

TransactionDialog::~TransactionDialog()
{
    wxSize size = GetSize();
    if (m_custom_fields->IsCustomPanelShown())
        size = wxSize(GetSize().GetWidth() - m_custom_fields->GetMinWidth(), GetSize().GetHeight());
    InfotableModel::instance().setSize("TRANSACTION_DIALOG_SIZE", size);
}

void TransactionDialog::SetEventHandlers()
{
    m_textAmount->Connect(mmID_TEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(TransactionDialog::OnTextEntered), nullptr, this);
    toTextAmount_->Connect(mmID_TOTEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(TransactionDialog::OnTextEntered), nullptr, this);

#ifdef __WXGTK__ // Workaround for bug http://trac.wxwidgets.org/ticket/11630
    dpc_->Connect(ID_DIALOG_TRANS_BUTTONDATE, wxEVT_KILL_FOCUS
        , wxFocusEventHandler(TransactionDialog::OnDpcKillFocus), nullptr, this);
#endif
}

// Used to determine if we need to refresh the tag text ctrl after
// accelerator hints are shown which only occurs once.
static bool altRefreshDone;
// store used date between two invocations for save & new
static wxDateTime previousDate;

TransactionDialog::TransactionDialog(
    wxWindow* parent,
    int64 account_id,
    Journal::IdB journal_id,
    bool duplicate,
    int type
) :
    m_account_id(account_id)
{
    SetEvtHandlerEnabled(false);
    bool found = Journal::getJournalData(m_journal_data, journal_id);
    if (found) {
        // a bill can only be duplicated
        m_mode = (duplicate || journal_id.second) ? MODE_DUP : MODE_EDIT;
        const wxString& splitRefType = (m_journal_data.m_repeat_num == 0) ?
            TransactionSplitModel::refTypeName :
            ScheduledSplitModel::refTypeName;
        for (const auto& split : Journal::split(m_journal_data)) {
            wxArrayInt64 tags;
            for (const auto& tag : TagLinkModel::instance().find(
                TagLinkModel::REFTYPE(splitRefType),
                TagLinkModel::REFID(split.SPLITTRANSID))
            )
                tags.push_back(tag.TAGID);
            m_local_splits.push_back({split.CATEGID, split.SPLITTRANSAMOUNT, tags, split.NOTES});
        }

        if (m_mode == MODE_DUP && !SettingModel::instance().getBool(INIDB_USE_ORG_DATE_DUPLICATE, false))
        {
            // Use the empty transaction logic to generate the new date to be used
            TransactionModel::Data emptyTrx;
            TransactionModel::getEmptyData(emptyTrx, account_id);
            m_journal_data.TRANSDATE = emptyTrx.TRANSDATE;
        }
    }
    else {
        m_mode = MODE_NEW;
        TransactionModel::getEmptyData(m_journal_data, account_id);
        m_journal_data.TRANSCODE = TransactionModel::type_name(type);
    }

    m_transfer = TransactionModel::type_id(m_journal_data.TRANSCODE) == TransactionModel::TYPE_ID_TRANSFER;
    m_advanced = m_mode != MODE_NEW && m_transfer && (m_journal_data.TRANSAMOUNT != m_journal_data.TOTRANSAMOUNT);

    int64 ref_id = (m_mode == MODE_NEW) ? 0 : (m_journal_data.m_repeat_num == 0) ?
        m_journal_data.TRANSID : -(m_journal_data.m_bdid);
    m_custom_fields = new mmCustomDataTransaction(this, ref_id, ID_CUSTOMFIELDS);

    // If duplicate then we may need to copy the attachments
    if (m_mode == MODE_DUP && InfotableModel::instance().getBool("ATTACHMENTSDUPLICATE", false))
    {
        const wxString& refType = TransactionModel::refTypeName;
        mmAttachmentManage::CloneAllAttachments(refType, journal_id.first, -1);
    }

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

bool TransactionDialog::Create(
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
        m_journal_data.DELETEDTIME.IsEmpty() ? _t("Edit Transaction") :
        "";  //_t("View Deleted Transaction");
    SetDialogTitle(header);

    SetIcon(mmex::getProgramIcon());

    SetEventHandlers();
    SetEvtHandlerEnabled(true);
    return true;
}

void TransactionDialog::dataToControls()
{
    TransactionModel::getFrequentUsedNotes(frequentNotes_, m_journal_data.ACCOUNTID);
    wxButton* bFrequentUsedNotes = static_cast<wxButton*>(FindWindow(ID_DIALOG_TRANS_BUTTON_FREQENTNOTES));
    bFrequentUsedNotes->Enable(!frequentNotes_.empty());

    if (!skip_date_init_) //Date
    {
        wxDateTime trx_date;
        if (previousDate.IsValid()) {
            trx_date = previousDate;
        }
        else {
            bool is_time_used = PreferencesModel::instance().UseTransDateTime();
            const wxString mask = is_time_used ? "%Y-%m-%dT%H:%M:%S" : "%Y-%m-%d";
            if (!trx_date.ParseFormat(m_journal_data.TRANSDATE, mask)) {
                trx_date.ParseDate(m_journal_data.TRANSDATE);
            }
        }
        dpc_->SetValue(trx_date);
        dpc_->SetFocus();
        skip_date_init_ = true;
    }

    if (!skip_status_init_) //Status
    {
        bool useOriginalState = m_mode != MODE_DUP || SettingModel::instance().getBool(INIDB_USE_ORG_STATE_DUPLICATE_PASTE, false);
        m_status = useOriginalState? m_journal_data.STATUS : TransactionModel::status_key(PreferencesModel::instance().getTransStatusReconciled());
        choiceStatus_->SetSelection(TransactionModel::status_id(m_status));
        skip_status_init_ = true;
    }

    //Type
    transaction_type_->SetSelection(TransactionModel::type_id(m_journal_data.TRANSCODE));

    //Account
    if (!skip_account_init_)
    {
        AccountModel::Data* acc = AccountModel::instance().get(m_journal_data.ACCOUNTID);
        if (acc)
        {
            cbAccount_->ChangeValue(acc->ACCOUNTNAME);
            m_textAmount->SetCurrency(CurrencyModel::instance().get(acc->CURRENCYID));
        }
        AccountModel::Data* to_acc = AccountModel::instance().get(m_journal_data.TOACCOUNTID);
        if (to_acc) {
            cbToAccount_->ChangeValue(to_acc->ACCOUNTNAME);
            toTextAmount_->SetCurrency(CurrencyModel::instance().get(to_acc->CURRENCYID));
        }

        skip_account_init_ = true;
        skip_amount_init_ = false; // Force amount format update in case account currencies change
    }

    if (m_transfer) {
        account_label_->SetLabelText(_t("From"));
        payee_label_->SetLabelText(_t("To"));
    }
    else if (!TransactionModel::is_deposit(m_journal_data.TRANSCODE)) {
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
    if (!skip_amount_init_)
    {
        if (m_transfer && m_advanced)
            toTextAmount_->SetValue(m_journal_data.TOTRANSAMOUNT);
        else
            toTextAmount_->ChangeValue("");

        if (m_mode != MODE_NEW)
            m_textAmount->SetValue(m_journal_data.TRANSAMOUNT);
        skip_amount_init_ = true;
    }

    if (!skip_payee_init_) //Payee
    {
        cbPayee_->SetEvtHandlerEnabled(false);

        cbAccount_->UnsetToolTip();
        cbPayee_->UnsetToolTip();
        if (!m_transfer)
        {
            if (!TransactionModel::foreignTransaction(m_journal_data)) {
                m_journal_data.TOACCOUNTID = -1;
            }

            int64 accountID = cbAccount_->mmGetId();
            if (m_mode == MODE_NEW && PreferencesModel::instance().getTransPayeeNone() == PreferencesModel::LASTUSED
                && (-1 != accountID))
            {
                TransactionModel::Data_Set transactions = TransactionModel::instance().find(
                    TransactionModel::TRANSCODE(TransactionModel::TYPE_ID_TRANSFER, NOT_EQUAL)
                    , TransactionModel::ACCOUNTID(accountID, EQUAL));

                if (!transactions.empty()) {
                    PayeeModel::Data* payee = PayeeModel::instance().get(transactions.back().PAYEEID);
                    cbPayee_->ChangeValue(payee->PAYEENAME);
                }
            }
            else if (m_mode == MODE_NEW && PreferencesModel::instance().getTransPayeeNone() == PreferencesModel::UNUSED)
            {
                PayeeModel::Data *payee = PayeeModel::instance().get(_t("Unknown"));
                if (!payee)
                {
                    payee = PayeeModel::instance().create();
                    payee->PAYEENAME = _t("Unknown");
                    payee->ACTIVE = 1;
                    PayeeModel::instance().save(payee);
                    cbPayee_->mmDoReInitialize();
                }

                cbPayee_->ChangeValue(_t("Unknown"));
            }
            else
            {
                PayeeModel::Data* payee = PayeeModel::instance().get(m_journal_data.PAYEEID);
                if (payee)
                    cbPayee_->ChangeValue(payee->PAYEENAME);
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
    if (!skip_category_init_)
    {
        bSplit_->UnsetToolTip();
        if (has_split)
        {
            cbCategory_->ChangeValue(_t("Split Transaction"));
            cbCategory_->Disable();
            m_textAmount->SetValue(TransactionSplitModel::get_total(m_local_splits));
            m_journal_data.CATEGID = -1;
        }
        else if (m_mode == MODE_NEW && m_transfer
            && PreferencesModel::instance().getTransCategoryTransferNone() == PreferencesModel::LASTUSED)
        {
            TransactionModel::Data_Set transactions = TransactionModel::instance().find(
                TransactionModel::TRANSCODE(TransactionModel::TYPE_ID_TRANSFER, EQUAL));

            if (!transactions.empty()
                && (!CategoryModel::is_hidden(transactions.back().CATEGID)))
            {
                const int64 cat = transactions.back().CATEGID;
                cbCategory_->ChangeValue(CategoryModel::full_name(cat));
            }
        } else
        {
            auto fullCategoryName = CategoryModel::full_name(m_journal_data.CATEGID);
            cbCategory_->ChangeValue(fullCategoryName);
        }
        skip_category_init_ = true;
    }

    m_textAmount->Enable(!has_split);
    bCalc_->Enable(!has_split);
    cbCategory_->Enable(!has_split);
    bSplit_->Enable(!m_transfer);

    // Tags
    if (!skip_tag_init_)
    {
        wxArrayInt64 tagIds;
        for (const auto& tag : TagLinkModel::instance().find(
            TagLinkModel::REFTYPE((m_journal_data.m_repeat_num == 0) ?
                TransactionModel::refTypeName :
                ScheduledModel::refTypeName),
            TagLinkModel::REFID((m_journal_data.m_repeat_num == 0) ?
                m_journal_data.TRANSID :
                m_journal_data.m_bdid))
        )
            tagIds.push_back(tag.TAGID);
        tagTextCtrl_->SetTags(tagIds);
        skip_tag_init_ = true;
    }

    if (!skip_notes_init_) //Notes & Transaction Number
    {
        textNumber_->SetValue(m_journal_data.TRANSACTIONNUMBER);
        textNotes_->SetValue(m_journal_data.NOTES);
        skip_notes_init_ = true;
    }

    if (!skip_tooltips_init_)
        SetTooltips();

    if (!m_journal_data.DELETEDTIME.IsEmpty()) {
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

void TransactionDialog::CreateControls()
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
    today->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TransactionDialog::OnToday), nullptr, this);

    const wxString tooltiptext = _t("Set date to today (Ctrl+;)");
    mmToolTip(today, tooltiptext);
    wxChar hotkeyToday = ExtractHotkeyChar(tooltiptext, '.');
    flex_sizer->Add(today, g_flagsH);

    // Type --------------------------------------------
    transaction_type_ = new wxChoice(static_box, ID_DIALOG_TRANS_TYPE);

    for (int i = 0; i < TransactionModel::TYPE_ID_size; ++i) {
        if (i != TransactionModel::TYPE_ID_TRANSFER ||
            AccountModel::instance().all().size() > 1
        ) {
            wxString type = TransactionModel::type_name(i);
            transaction_type_->Append(wxGetTranslation(type), new wxStringClientData(type));
        }
    }

    cAdvanced_ = new wxCheckBox(static_box
        , ID_DIALOG_TRANS_ADVANCED_CHECKBOX, _t("&Advanced")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );

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
    bCalc_->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TransactionDialog::OnCalculator), nullptr, this);
    mmToolTip(bCalc_, _t("Open Calculator"));
    flex_sizer->Add(bCalc_, g_flagsH);
    calcTarget_ = m_textAmount;
    calcPopup_ = new mmCalculatorPopup(bCalc_, calcTarget_);

    // Account ---------------------------------------------
    account_label_ = new wxStaticText(static_box, wxID_STATIC, _t("Account"));
    account_label_->SetFont(bold);

    cbAccount_ = new mmComboBoxAccount(static_box, mmID_ACCOUNTNAME, wxDefaultSize, m_journal_data.ACCOUNTID);
    cbAccount_->SetMinSize(cbAccount_->GetSize());
    flex_sizer->Add(account_label_, g_flagsH);
    flex_sizer->Add(cbAccount_, g_flagsExpand);

    bSwitch_ = new wxBitmapButton(static_box, wxID_ANY, mmBitmapBundle(png::UPDATE, mmBitmapButtonSize));
    bSwitch_->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TransactionDialog::OnSwitch), nullptr, this);
    mmToolTip(bSwitch_, _t("Exchange to and from accounts"));
    flex_sizer->Add(bSwitch_, g_flagsH);

    // To Account ------------------------------------------------
    to_acc_label_ = new wxStaticText(static_box, mmID_TOACCOUNT_LABEL, _t("To"));
    to_acc_label_->SetFont(bold);
    flex_sizer->Add(to_acc_label_, g_flagsH);
    cbToAccount_ = new mmComboBoxAccount(static_box, mmID_TOACCOUNTNAME, wxDefaultSize, m_journal_data.TOACCOUNTID);
    cbToAccount_->SetMinSize(cbToAccount_->GetSize());
    flex_sizer->Add(cbToAccount_, g_flagsExpand);
    flex_sizer->AddSpacer(1);

    // Payee ---------------------------------------------
    payee_label_ = new wxStaticText(static_box, mmID_PAYEE_LABEL, _t("Payee"));
    payee_label_->SetFont(bold);

    cbPayee_ = new mmComboBoxPayee(static_box, mmID_PAYEE, wxDefaultSize, m_journal_data.PAYEEID, true);
    cbPayee_->SetMinSize(cbPayee_->GetSize());
    flex_sizer->Add(payee_label_, g_flagsH);
    flex_sizer->Add(cbPayee_, g_flagsExpand);
    flex_sizer->AddSpacer(1);

    // Category -------------------------------------------------
    categ_label_ = new wxStaticText(static_box, ID_DIALOG_TRANS_CATEGLABEL2, _t("Category"));
    categ_label_->SetFont(bold);
    cbCategory_ = new mmComboBoxCategory(static_box, mmID_CATEGORY, wxDefaultSize
        , m_journal_data.CATEGID, true);
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

    for (int i = 0; i < TransactionModel::STATUS_ID_size; ++i) {
        wxString status = TransactionModel::status_name(i);
        choiceStatus_->Append(wxGetTranslation(status), new wxStringClientData(status));
    }

    flex_sizer->Add(new wxStaticText(static_box, wxID_STATIC, _t("Status")), g_flagsH);
    flex_sizer->Add(choiceStatus_, g_flagsExpand);
    flex_sizer->AddSpacer(1);

    // Number  ---------------------------------------------
    textNumber_ = new wxTextCtrl(static_box, ID_DIALOG_TRANS_TEXTNUMBER, "", wxDefaultPosition, wxDefaultSize);

    bAuto = new wxBitmapButton(static_box, ID_DIALOG_TRANS_BUTTONTRANSNUM, mmBitmapBundle(png::TRXNUM, mmBitmapButtonSize));
    bAuto->Connect(ID_DIALOG_TRANS_BUTTONTRANSNUM, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TransactionDialog::OnAutoTransNum), nullptr, this);
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
        , wxCommandEventHandler(TransactionDialog::OnFrequentUsedNotes), nullptr, this);

    // Colours
    bColours_ = new mmColorButton(static_box, wxID_LOWEST, bAuto->GetSize());
    mmToolTip(bColours_, _t("User Colors"));
    bColours_->SetBackgroundColor(m_journal_data.COLOR.GetValue());

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
    wxPanel* buttons_panel = new wxPanel(static_box, wxID_ANY);
    box_sizer_left->Add(buttons_panel, wxSizerFlags(g_flagsV).Center().Border(wxALL, 0));

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

bool TransactionDialog::ValidateData()
{
    if (!m_textAmount->checkValue(m_journal_data.TRANSAMOUNT))
        return false;
    if (!tagTextCtrl_->IsValid()) {
        mmErrorDialogs::ToolTip4Object(tagTextCtrl_, _t("Invalid value"), _t("Tags"), wxICON_ERROR);
        return false;
    }
    if (!cbAccount_->mmIsValid()) {
        mmErrorDialogs::ToolTip4Object(cbAccount_, _t("Invalid value"), _t("Account"), wxICON_ERROR);
        return false;
    }
    m_journal_data.ACCOUNTID = cbAccount_->mmGetId();
    const AccountModel::Data* account = AccountModel::instance().get(m_journal_data.ACCOUNTID);

    if (m_journal_data.TRANSDATE < account->INITIALDATE)
    {
        mmErrorDialogs::ToolTip4Object(cbAccount_, _t("The opening date for the account is later than the date of this transaction"), _t("Invalid Date"));
        return false;
    }

    if (m_local_splits.empty())
    {
        if (!cbCategory_->mmIsValid()) {
            mmErrorDialogs::ToolTip4Object(cbCategory_, _t("Invalid value"), _t("Category"), wxICON_ERROR);
            return false;
        }
        m_journal_data.CATEGID = cbCategory_->mmGetCategoryId();
    }

    if (!m_transfer)
    {
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

        PayeeModel::Data* payee = PayeeModel::instance().get(payee_name);
        if (!payee)
        {
            wxMessageDialog msgDlg( this
                , wxString::Format(_t("Payee name has not been used before. Is the name correct?\n%s"), payee_name)
                , _t("Confirm payee name")
                , wxYES_NO | wxYES_DEFAULT | wxICON_WARNING);
            if (msgDlg.ShowModal() == wxID_YES)
            {
                payee = PayeeModel::instance().create();
                payee->PAYEENAME = payee_name;
                payee->ACTIVE = 1;
                PayeeModel::instance().save(payee);
                mmWebApp::MMEX_WebApp_UpdatePayee();
            }
            else
                return false;
        }
        m_journal_data.TOTRANSAMOUNT = m_journal_data.TRANSAMOUNT;
        m_journal_data.PAYEEID = payee->PAYEEID;
        if (!TransactionModel::foreignTransaction(m_journal_data))
        {
            m_journal_data.TOACCOUNTID = -1;
        }

        if ((PreferencesModel::instance().getTransCategoryNone() == PreferencesModel::LASTUSED)
            && (!CategoryModel::is_hidden(m_journal_data.CATEGID)))
        {
            payee->CATEGID = m_journal_data.CATEGID;
            PayeeModel::instance().save(payee);
            mmWebApp::MMEX_WebApp_UpdatePayee();
        }
    }
    else //transfer
    {
        const AccountModel::Data *to_account = AccountModel::instance().get(cbToAccount_->GetValue());

        if (!to_account || to_account->ACCOUNTID == m_journal_data.ACCOUNTID)
        {
            mmErrorDialogs::InvalidAccount(cbToAccount_, true);
            return false;
        }
        m_journal_data.TOACCOUNTID = to_account->ACCOUNTID;

        if (m_journal_data.TRANSDATE < to_account->INITIALDATE)
        {
            mmErrorDialogs::ToolTip4Object(cbToAccount_, _t("The opening date for the account is later than the date of this transaction"), _t("Invalid Date"));
            return false;
        }

        if (m_advanced)
        {
            if (!toTextAmount_->checkValue(m_journal_data.TOTRANSAMOUNT))
                return false;
        }
        m_journal_data.PAYEEID = -1;
    }

    /* Check if transaction is to proceed.*/
    if (AccountModel::BoolOf(account->STATEMENTLOCKED))
    {
        if (dpc_->GetValue() <= parseDateTime(account->STATEMENTDATE))
        {
            if (wxMessageBox(wxString::Format(
                _t("Lock transaction to date: %s") + "\n\n" + _t("Do you want to continue?")
                , mmGetDateTimeForDisplay(account->STATEMENTDATE))
                , _t("MMEX Transaction Check"), wxYES_NO | wxICON_WARNING) == wxNO)
            {
                return false;
            }
        }
    }

    //Checking account does not exceed limits
    if (m_mode != MODE_EDIT)
    {
        if (m_journal_data.STATUS != TransactionModel::STATUS_KEY_VOID &&
            (m_journal_data.TRANSCODE == TransactionModel::TYPE_NAME_WITHDRAWAL ||
             m_journal_data.TRANSCODE == TransactionModel::TYPE_NAME_TRANSFER) &&
            (account->MINIMUMBALANCE != 0 || account->CREDITLIMIT != 0))
        {
            const double fromAccountBalance = AccountModel::balance(account);
            const double new_value = fromAccountBalance - m_journal_data.TRANSAMOUNT;

            bool abort_transaction =
                (account->MINIMUMBALANCE != 0 && new_value < account->MINIMUMBALANCE) ||
                (account->CREDITLIMIT != 0 && new_value < -(account->CREDITLIMIT));

            if (abort_transaction && wxMessageBox(
                _t("The transaction will exceed the account limit.") + "\n\n" + _t("Do you want to continue?")
                , _t("MMEX Transaction Check"), wxYES_NO | wxICON_WARNING) == wxNO)
            {
                return false;
            }
        }
    }

    int color_id = bColours_->GetColorId();
    if (color_id > 0 && color_id < 8)
        m_journal_data.COLOR = color_id;
    else
        m_journal_data.COLOR = -1;

    return true;
}

//----------------------------------------------------------------------------
// Workaround for bug http://trac.wxwidgets.org/ticket/11630
void TransactionDialog::OnDpcKillFocus(wxFocusEvent& event)
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

void TransactionDialog::OnFocusChange(wxChildFocusEvent& event)
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
            m_journal_data.ACCOUNTID = cbAccount_->mmGetId();
            skip_account_init_ = false;
        }
        break;
    case mmID_TOACCOUNTNAME:
        cbToAccount_->ChangeValue(cbToAccount_->GetValue());
        if (cbToAccount_->mmIsValid())
        {
            m_journal_data.TOACCOUNTID = cbToAccount_->mmGetId();
            skip_account_init_ = false;
        }
        break;
    case mmID_PAYEE:
        cbPayee_->ChangeValue(cbPayee_->GetValue());
        m_journal_data.PAYEEID = cbPayee_->mmGetId();
        SetCategoryForPayee();
        break;
    case mmID_CATEGORY:
        cbCategory_->ChangeValue(cbCategory_->GetValue());
        break;
    case mmID_TEXTAMOUNT:
        if (m_textAmount->Calculate()) {
            m_textAmount->GetDouble(m_journal_data.TRANSAMOUNT);
        }
        skip_amount_init_ = false;
        calcTarget_ = m_textAmount;
        break;
    case mmID_TOTEXTAMOUNT:
        if (toTextAmount_->Calculate()) {
            toTextAmount_->GetDouble(m_journal_data.TOTRANSAMOUNT);
        }
        skip_amount_init_ = false;
        calcTarget_ = toTextAmount_;
        break;
    }

    object_in_focus_ = w->GetId();

    if (!m_transfer)
    {
        toTextAmount_->ChangeValue("");
        m_journal_data.TOACCOUNTID = -1;
    }
    else
    {
        const AccountModel::Data* to_account = AccountModel::instance().get(cbToAccount_->mmGetId());
        if (to_account)
            m_journal_data.TOACCOUNTID = to_account->ACCOUNTID;
    }

    dataToControls();
    event.Skip();
}

void TransactionDialog::SetDialogTitle(const wxString& title)
{
    this->SetTitle(title);
}

void TransactionDialog::OnPayeeChanged(wxCommandEvent& /*event*/)
{
    PayeeModel::Data * payee = PayeeModel::instance().get(cbPayee_->GetValue());
    if (payee)
    {
        SetCategoryForPayee(payee);
    }
}

void TransactionDialog::OnTransTypeChanged(wxCommandEvent& event)
{
    const wxString old_type = m_journal_data.TRANSCODE;
    wxStringClientData *client_obj = static_cast<wxStringClientData*>(event.GetClientObject());
    if (client_obj) m_journal_data.TRANSCODE = client_obj->GetData();
    if (old_type != m_journal_data.TRANSCODE)
    {
        m_transfer = TransactionModel::is_transfer(m_journal_data.TRANSCODE);
        if (m_transfer || TransactionModel::is_transfer(old_type))
            skip_payee_init_ = false;
        else
            skip_payee_init_ = true;
        skip_account_init_ = true;
        skip_tooltips_init_ = false;

        if (m_transfer) {
            m_journal_data.PAYEEID = -1;
            skip_category_init_ = false;
        } else {
            m_journal_data.TOTRANSAMOUNT = m_journal_data.TRANSAMOUNT;
            m_journal_data.TOACCOUNTID = -1;
        }
        dataToControls();
    }
}

void TransactionDialog::OnComboKey(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_RETURN)
    {
        auto id = event.GetId();
        switch (id)
        {
        case mmID_PAYEE:
        {
            const auto payeeName = cbPayee_->GetValue();
            if (payeeName.empty())
            {
                mmPayeeDialog dlg(this, true);
                dlg.ShowModal();
                if (dlg.getRefreshRequested())
                    cbPayee_->mmDoReInitialize();
                int64 payee_id = dlg.getPayeeId();
                PayeeModel::Data* payee = PayeeModel::instance().get(payee_id);
                if (payee) {
                    cbPayee_->ChangeValue(payee->PAYEENAME);
                    cbPayee_->SetInsertionPointEnd();
                    SetCategoryForPayee(payee);
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
                CategoryDialog dlg(this, true, -1);
                int rc = dlg.ShowModal();
                if (dlg.getRefreshRequested())
                    cbCategory_->mmDoReInitialize();
                if (rc != wxID_CANCEL) cbCategory_->ChangeValue(CategoryModel::full_name(dlg.getCategId()));
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

void TransactionDialog::SetCategoryForPayee(const PayeeModel::Data *payee)
{
    // Only for new transactions: if user does not want to use categories.
    // If this is a Split Transaction, ignore displaying last category for payee
    if (m_mode == MODE_NEW && PreferencesModel::instance().getTransCategoryNone() == PreferencesModel::UNUSED
        && m_local_splits.empty())
    {
        CategoryModel::Data *category = CategoryModel::instance().get(_t("Unknown"), int64(-1));
        if (!category)
        {
            category = CategoryModel::instance().create();
            category->CATEGNAME = _t("Unknown");
            category->ACTIVE = 1;
            CategoryModel::instance().save(category);
            cbCategory_->mmDoReInitialize();
        }

        m_journal_data.CATEGID = category->CATEGID;
        cbCategory_->ChangeValue(_t("Unknown"));
        return;
    }

    if (!payee)
    {
        payee = PayeeModel::instance().get(cbPayee_->GetValue());
        if (!payee)
            return;
    }

    // Only for new transactions: if user want to autofill last category used for payee.
    // If this is a Split Transaction, ignore displaying last category for payee
    if ((PreferencesModel::instance().getTransCategoryNone() == PreferencesModel::LASTUSED ||
        PreferencesModel::instance().getTransCategoryNone() == PreferencesModel::DEFAULT)
        && m_mode == MODE_NEW && m_local_splits.empty()
        && (!CategoryModel::is_hidden(payee->CATEGID)))
    {
        // if payee has memory of last category used then display last category for payee
        CategoryModel::Data *category = CategoryModel::instance().get(payee->CATEGID);
        if (category)
        {
            m_journal_data.CATEGID = payee->CATEGID;
            cbCategory_->ChangeValue(CategoryModel::full_name(payee->CATEGID));
            wxLogDebug("Category: %s = %.2f", cbCategory_->GetLabel(), m_journal_data.TRANSAMOUNT);
        }
        else
        {
            m_journal_data.CATEGID = -1;
            cbCategory_->ChangeValue("");
        }
    }
}

void TransactionDialog::OnCalculator(wxCommandEvent& WXUNUSED(event))
{
    calcPopup_->SetTarget(calcTarget_);
    calcPopup_->Popup();
}

void TransactionDialog::OnSwitch(wxCommandEvent& WXUNUSED(event))
{
    wxString temp = cbToAccount_->GetValue();
    cbToAccount_->ChangeValue(cbAccount_->GetValue());
    cbAccount_->ChangeValue(temp);
}

void TransactionDialog::OnToday(wxCommandEvent& WXUNUSED(event))
{
    dpc_->SetValue(wxDateTime::Today());
}

void TransactionDialog::OnAutoTransNum(wxCommandEvent& WXUNUSED(event))
    {
    auto d = TransactionModel::getTransDateTime(m_journal_data).Subtract(wxDateSpan::Months(12));
    double next_number = 0, temp_num;
    const auto numbers = TransactionModel::instance().find(
        TransactionModel::ACCOUNTID(m_journal_data.ACCOUNTID, EQUAL)
        , TransactionModel::TRANSDATE(d, GREATER_OR_EQUAL)
        , TransactionModel::TRANSACTIONNUMBER("", NOT_EQUAL));
    for (const auto &num : numbers)
    {
        if (!num.TRANSACTIONNUMBER.IsNumber()) continue;
        if (num.TRANSACTIONNUMBER.ToDouble(&temp_num) && temp_num > next_number)
            next_number = temp_num;
    }

    next_number++;
    textNumber_->SetValue(wxString::FromDouble(next_number, 0));
}

void TransactionDialog::OnAdvanceChecked(wxCommandEvent& WXUNUSED(event))
{
    m_advanced = cAdvanced_->IsChecked();
    skip_amount_init_ = false;
    dataToControls();
}

void TransactionDialog::OnCategs(wxCommandEvent& WXUNUSED(event))
{
    if (!m_textAmount->GetDouble(m_journal_data.TRANSAMOUNT)) {
        m_journal_data.TRANSAMOUNT = 0;
    }

    if (cbCategory_->IsEnabled() && !cbCategory_->GetValue().IsEmpty() && !cbCategory_->mmIsValid()) {
        mmErrorDialogs::ToolTip4Object(cbCategory_, _t("Invalid value"), _t("Category"), wxICON_ERROR);
        return;
    }
    wxLogDebug("Cat Valid %d, Cat Is Empty %d, Cat value [%s]", cbCategory_->mmIsValid(), cbCategory_->GetValue().IsEmpty(), cbCategory_->GetValue());
    if (m_local_splits.empty())
    {
        Split s;
        s.SPLITTRANSAMOUNT = m_journal_data.TRANSAMOUNT;
        if (cbCategory_->mmIsValid())
            s.CATEGID = cbCategory_->mmGetCategoryId();
        m_local_splits.push_back(s);
    }

    SplitDialog dlg(this, m_local_splits, m_journal_data.ACCOUNTID);

    if (dlg.ShowModal() == wxID_OK)
    {
        m_local_splits = dlg.mmGetResult();

        if (m_local_splits.size() == 1) {
            m_journal_data.CATEGID = m_local_splits[0].CATEGID;
            m_journal_data.TRANSAMOUNT = m_local_splits[0].SPLITTRANSAMOUNT;
            m_textAmount->SetValue(m_journal_data.TRANSAMOUNT);
            m_local_splits.clear();
        }

        if (!m_local_splits.empty()) {
            m_textAmount->SetValue(m_journal_data.TRANSAMOUNT);
        }

        skip_category_init_ = false;
        skip_amount_init_ = false;
        skip_tooltips_init_ = false;
        dataToControls();
    }
    tagTextCtrl_->Reinitialize();
}

void TransactionDialog::OnAttachments(wxCommandEvent& WXUNUSED(event))
{
    const wxString& refType = (m_journal_data.m_repeat_num == 0) ?
        TransactionModel::refTypeName :
        ScheduledModel::refTypeName;
    int64 transID = (m_mode == MODE_DUP) ? -1 : m_journal_data.TRANSID;
    AttachmentDialog dlg(this, refType, transID);
    dlg.ShowModal();
}

void TransactionDialog::OnTextEntered(wxCommandEvent& WXUNUSED(event))
{
    if (object_in_focus_ == m_textAmount->GetId())
    {
        if (m_textAmount->Calculate())
        {
            m_textAmount->GetDouble(m_journal_data.TRANSAMOUNT);
        }
    }
    else if (object_in_focus_ == toTextAmount_->GetId())
    {
        if (toTextAmount_->Calculate())
        {
            toTextAmount_->GetDouble(m_journal_data.TOTRANSAMOUNT);
        }
    }
    skip_amount_init_ = false;
    dataToControls();
}

void TransactionDialog::OnFrequentUsedNotes(wxCommandEvent& WXUNUSED(event))
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
        menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &TransactionDialog::OnNoteSelected, this);
        PopupMenu(&menu);
    }
}

void TransactionDialog::OnNoteSelected(wxCommandEvent& event)
{
    int i = event.GetId() - wxID_HIGHEST;

    if (i > 0 && static_cast<size_t>(i) <= frequentNotes_.size()) {
        if (!textNotes_->GetValue().EndsWith("\n") && !textNotes_->GetValue().empty())
            textNotes_->AppendText("\n");
        textNotes_->AppendText(frequentNotes_[i - 1]);
    }
}

void TransactionDialog::OnOk(wxCommandEvent& event)
{
    m_journal_data.NOTES = textNotes_->GetValue();
    m_journal_data.TRANSACTIONNUMBER = textNumber_->GetValue();
    m_journal_data.TRANSDATE = dpc_->GetValue().FormatISOCombined();
    wxStringClientData* status_obj = static_cast<wxStringClientData*>(choiceStatus_->GetClientObject(choiceStatus_->GetSelection()));
    if (status_obj)
    {
        m_status = TransactionModel::status_key(status_obj->GetData());
        m_journal_data.STATUS = m_status;
    }

    if (!ValidateData()) return;
    if (!m_custom_fields->ValidateCustomValues((m_journal_data.m_repeat_num == 0) ?
        m_journal_data.TRANSID : -(m_journal_data.m_bdid))
    )
        return;

    if (!m_advanced)
        m_journal_data.TOTRANSAMOUNT = m_journal_data.TRANSAMOUNT;

    if (m_transfer && !m_advanced && (AccountModel::currency(AccountModel::instance().get(m_journal_data.ACCOUNTID))
        != AccountModel::currency(AccountModel::instance().get(m_journal_data.TOACCOUNTID))))
    {
        wxMessageDialog msgDlg( this
            , _t("The two accounts have different currencies, but no advanced transaction is defined. Is this correct?")
            , _t("Currencies are different")
            , wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
        if (msgDlg.ShowModal() == wxID_NO)
            return;
    }

    TransactionModel::Data *r = (m_mode == MODE_EDIT) ?
        TransactionModel::instance().get(m_journal_data.TRANSID) :
        TransactionModel::instance().create();

    TransactionModel::putDataToTransaction(r, m_journal_data);
    m_journal_data.TRANSID = TransactionModel::instance().save(r);
    m_journal_data.m_bdid = 0;
    m_journal_data.m_repeat_num = 0;

    TransactionSplitModel::Data_Set splt;
    for (const auto& entry : m_local_splits)
    {
        TransactionSplitModel::Data *s = TransactionSplitModel::instance().create();
        s->CATEGID = entry.CATEGID;
        s->SPLITTRANSAMOUNT = entry.SPLITTRANSAMOUNT;
        s->NOTES = entry.NOTES;
        splt.push_back(*s);
    }
    TransactionSplitModel::instance().update(splt, m_journal_data.TRANSID);

    // Save split tags
    const wxString& splitRefType = TransactionSplitModel::refTypeName;

    for (unsigned int i = 0; i < m_local_splits.size(); i++)
    {
        TagLinkModel::Data_Set splitTaglinks;
        for (const auto& tagId : m_local_splits.at(i).TAGS)
        {
            TagLinkModel::Data* t = TagLinkModel::instance().create();
            t->REFTYPE = splitRefType;
            t->REFID = splt.at(i).SPLITTRANSID;
            t->TAGID = tagId;
            splitTaglinks.push_back(*t);
        }
        TagLinkModel::instance().update(splitTaglinks, splitRefType, splt.at(i).SPLITTRANSID);
    }
    const wxString& RefType = TransactionModel::refTypeName;
    if (m_mode != MODE_EDIT) {
        mmAttachmentManage::RelocateAllAttachments(RefType, -1, RefType, m_journal_data.TRANSID);
    }

    m_custom_fields->SaveCustomValues(m_journal_data.TRANSID);

    // Save base transaction tags
    TagLinkModel::Data_Set taglinks;
    for (const auto& tagId : tagTextCtrl_->GetTagIDs())
    {
        TagLinkModel::Data* t = TagLinkModel::instance().create();
        t->REFTYPE = RefType;
        t->REFID = m_journal_data.TRANSID;
        t->TAGID = tagId;
        taglinks.push_back(*t);
    }
    TagLinkModel::instance().update(taglinks, RefType, m_journal_data.TRANSID);

    const TransactionModel::Data& tran(*r);
    TransactionModel::Full_Data trx(tran);
    //wxLogDebug("%s", trx.to_json());

    if (event.GetId() == ID_BTN_OK_NEW) {
        previousDate = dpc_->GetValue();  // store date for next invocation
    }
    else {
        previousDate = wxDateTime();
    }

    EndModal((m_mode == MODE_NEW || m_mode == MODE_DUP) && (event.GetId() == ID_BTN_OK_NEW || wxGetKeyState(WXK_SHIFT)) ? wxID_NEW : wxID_OK);
}

void TransactionDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
#ifdef __WXMSW__
    if (object_in_focus_ != wxID_CANCEL && wxGetKeyState(WXK_ESCAPE))
        return m_button_cancel->SetFocus();

    if (object_in_focus_ != wxID_CANCEL) {
        return;
    }
#endif

    if (m_mode != MODE_EDIT) {
        const wxString& RefType = TransactionModel::refTypeName;
        mmAttachmentManage::DeleteAllAttachments(RefType, -1);
        FieldValueModel::instance().DeleteAllData(RefType, -1);
    }
    previousDate = wxDateTime(); // invalidate!
    EndModal(wxID_CANCEL);
}

void TransactionDialog::SetTooltips()
{
    bSplit_->UnsetToolTip();
    skip_tooltips_init_ = true;
    if (this->m_local_splits.empty())
        mmToolTip(bSplit_, _t("Use split Categories"));
    else {
        const CurrencyModel::Data* currency = CurrencyModel::GetBaseCurrency();
        const AccountModel::Data* account = AccountModel::instance().get(m_journal_data.ACCOUNTID);
        if (account)
            currency = AccountModel::currency(account);

        bSplit_->SetToolTip(TransactionSplitModel::get_tooltip(m_local_splits, currency));
    }
    if (m_mode != MODE_NEW) return;

    m_textAmount->UnsetToolTip();
    toTextAmount_->UnsetToolTip();
    cbAccount_->UnsetToolTip();
    cbPayee_->UnsetToolTip();

    if (m_transfer)
    {
        mmToolTip(cbAccount_, _t("Specify account the money is taken from"));
        mmToolTip(cbPayee_, _t("Specify account the money is moved to"));
        mmToolTip(m_textAmount, _t("Specify the transfer amount in the From Account."));

        if (m_advanced)
            mmToolTip(toTextAmount_, _t("Specify the transfer amount in the To Account"));
    }
    else
    {
        mmToolTip(m_textAmount, _t("Specify the amount for this transaction"));
        mmToolTip(cbAccount_, _t("Specify account for the transaction"));
        if (!TransactionModel::is_deposit(m_journal_data.TRANSCODE))
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

void TransactionDialog::OnQuit(wxCloseEvent& WXUNUSED(event))
{
    const wxString& RefType = TransactionModel::refTypeName;
    if (m_mode != MODE_EDIT) {
        mmAttachmentManage::DeleteAllAttachments(RefType, -1);
        FieldValueModel::instance().DeleteAllData(RefType, -1);
    }
    EndModal(wxID_CANCEL);
}

void TransactionDialog::OnMoreFields(wxCommandEvent& WXUNUSED(event))
{
    wxBitmapButton* button = static_cast<wxBitmapButton*>(FindWindow(ID_DIALOG_TRANS_CUSTOMFIELDS));

    if (button)
        button->SetBitmap(mmBitmapBundle(m_custom_fields->IsCustomPanelShown() ? png::RIGHTARROW : png::LEFTARROW, mmBitmapButtonSize));

    m_custom_fields->ShowHideCustomPanel();
    if (m_custom_fields->IsCustomPanelShown())
    {
        SetMinSize(wxSize(min_size_.GetWidth() + m_custom_fields->GetMinWidth(), min_size_.GetHeight()));
        SetSize(wxSize(GetSize().GetWidth() + m_custom_fields->GetMinWidth(), GetSize().GetHeight()));
    }
    else
    {
        SetMinSize(min_size_);
        SetSize(wxSize(GetSize().GetWidth() - m_custom_fields->GetMinWidth(), GetSize().GetHeight()));
    }
}
