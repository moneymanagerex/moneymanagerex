/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011-2022 Nikolay Akimov
 Copyright (C) 2011-2017 Stefano Giorgio [stef145g]
 Copyright (C) 2021, 2022 Mark Whalley (mark@ipx.co.uk)

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

#include "transdialog.h"
#include "mmcustomdata.h"
#include "attachmentdialog.h"
#include "categdialog.h"
#include "constants.h"
#include "images_list.h"
#include "mmSimpleDialogs.h"
#include "mmTextCtrl.h"
#include "optionsettingsview.h"
#include "paths.h"
#include "payeedialog.h"
#include "splittransactionsdialog.h"
#include "util.h"
#include "validators.h"
#include "webapp.h"
#include "option.h"

#include "model/Model_Setting.h"
#include "model/Model_Account.h"
#include "model/Model_Category.h"
#include "model/Model_CurrencyHistory.h"
#include "model/Model_CustomFieldData.h"
#include "model/Model_Attachment.h"
#include "model/Model_Tag.h"

wxIMPLEMENT_DYNAMIC_CLASS(mmTransDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmTransDialog, wxDialog)
EVT_CHAR_HOOK(mmTransDialog::OnComboKey)
EVT_CHILD_FOCUS(mmTransDialog::OnFocusChange)
EVT_COMBOBOX(mmID_PAYEE, mmTransDialog::OnPayeeChanged)
EVT_TEXT(mmID_PAYEE, mmTransDialog::OnPayeeChanged)
EVT_BUTTON(mmID_CATEGORY_SPLIT, mmTransDialog::OnCategs)
EVT_CHOICE(ID_DIALOG_TRANS_TYPE, mmTransDialog::OnTransTypeChanged)
EVT_CHECKBOX(ID_DIALOG_TRANS_ADVANCED_CHECKBOX, mmTransDialog::OnAdvanceChecked)
EVT_BUTTON(wxID_FILE, mmTransDialog::OnAttachments)
EVT_BUTTON(ID_DIALOG_TRANS_CUSTOMFIELDS, mmTransDialog::OnMoreFields)
EVT_BUTTON(wxID_OK, mmTransDialog::OnOk)
EVT_BUTTON(wxID_CANCEL, mmTransDialog::OnCancel)
EVT_CLOSE(mmTransDialog::OnQuit)
wxEND_EVENT_TABLE()

mmTransDialog::~mmTransDialog()
{
    wxSize size = GetSize();
    if (m_custom_fields->IsCustomPanelShown())
        size = wxSize(GetSize().GetWidth() - m_custom_fields->GetMinWidth(), GetSize().GetHeight());
    Model_Infotable::instance().setSize("TRANSACTION_DIALOG_SIZE", size);
}

void mmTransDialog::SetEventHandlers()
{
    m_textAmount->Connect(mmID_TEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmTransDialog::OnTextEntered), nullptr, this);
    toTextAmount_->Connect(mmID_TOTEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmTransDialog::OnTextEntered), nullptr, this);

#ifdef __WXGTK__ // Workaround for bug http://trac.wxwidgets.org/ticket/11630
    dpc_->Connect(ID_DIALOG_TRANS_BUTTONDATE, wxEVT_KILL_FOCUS
        , wxFocusEventHandler(mmTransDialog::OnDpcKillFocus), nullptr, this);
#endif
}

// Used to determine if we need to refresh the tag text ctrl after
// accelerator hints are shown which only occurs once.
static bool altRefreshDone;

mmTransDialog::mmTransDialog(wxWindow* parent,
    int64 account_id,
    Fused_Transaction::IdB fused_id,
    bool duplicate,
    int type)
:
    m_account_id(account_id)
{
    SetEvtHandlerEnabled(false);
    bool found = Fused_Transaction::getFusedData(m_fused_data, fused_id);
    if (found) {
        // a bill can only be duplicated
        m_mode = (duplicate || fused_id.second) ? MODE_DUP : MODE_EDIT;
        const wxString& splitRefType = (m_fused_data.m_repeat_num == 0) ?
            Model_Attachment::REFTYPE_NAME_TRANSACTIONSPLIT :
            Model_Attachment::REFTYPE_NAME_BILLSDEPOSITSPLIT;
        for (const auto& split : Fused_Transaction::split(m_fused_data)) {
            wxArrayInt64 tags;
            for (const auto& tag : Model_Taglink::instance().find(
                Model_Taglink::REFTYPE(splitRefType),
                Model_Taglink::REFID(split.SPLITTRANSID))
            )
                tags.push_back(tag.TAGID);
            m_local_splits.push_back({split.CATEGID, split.SPLITTRANSAMOUNT, tags, split.NOTES});
        }

        if (m_mode == MODE_DUP && !Model_Setting::instance().getBool(INIDB_USE_ORG_DATE_DUPLICATE, false))
        {
            // Use the empty transaction logic to generate the new date to be used
            Model_Checking::Data emptyTrx;
            Model_Checking::getEmptyData(emptyTrx, account_id);
            m_fused_data.TRANSDATE = emptyTrx.TRANSDATE;
        }
    }
    else {
        m_mode = MODE_NEW;
        Model_Checking::getEmptyData(m_fused_data, account_id);
        m_fused_data.TRANSCODE = Model_Checking::type_name(type);
    }

    m_transfer = Model_Checking::type_id(m_fused_data.TRANSCODE) == Model_Checking::TYPE_ID_TRANSFER;
    m_advanced = m_mode != MODE_NEW && m_transfer && (m_fused_data.TRANSAMOUNT != m_fused_data.TOTRANSAMOUNT);

    int64 ref_id = (m_mode == MODE_NEW) ? 0 : (m_fused_data.m_repeat_num == 0) ?
        m_fused_data.TRANSID : -(m_fused_data.m_bdid);
    m_custom_fields = new mmCustomDataTransaction(this, ref_id, ID_CUSTOMFIELD);

    // If duplicate then we may need to copy the attachments
    if (m_mode == MODE_DUP && Model_Infotable::instance().getBool("ATTACHMENTSDUPLICATE", false))
    {
        const wxString& refType = Model_Attachment::REFTYPE_NAME_TRANSACTION;
        mmAttachmentManage::CloneAllAttachments(refType, fused_id.first, -1);
    }

    this->SetFont(parent->GetFont());
    Create(parent);
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

bool mmTransDialog::Create(wxWindow* parent, wxWindowID id, const wxString& caption
    , const wxPoint& pos, const wxSize& size, long style, const wxString& name)
{
    altRefreshDone = false; // reset the ALT refresh indicator on new dialog creation
    style |= wxRESIZE_BORDER;
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style, name);

    SetEvtHandlerEnabled(false);
    CreateControls();

    wxString header = (m_mode == MODE_NEW) ? _t("New Transaction") :
        (m_mode == MODE_DUP) ? _t("Duplicate Transaction") :
        m_fused_data.DELETEDTIME.IsEmpty() ? _t("Edit Transaction") :
        "";  //_t("View Deleted Transaction");
    SetDialogTitle(header);

    SetIcon(mmex::getProgramIcon());

    SetEventHandlers();
    SetEvtHandlerEnabled(true);
    return true;
}

void mmTransDialog::dataToControls()
{
    Model_Checking::getFrequentUsedNotes(frequentNotes_, m_fused_data.ACCOUNTID);
    wxButton* bFrequentUsedNotes = static_cast<wxButton*>(FindWindow(ID_DIALOG_TRANS_BUTTON_FREQENTNOTES));
    bFrequentUsedNotes->Enable(!frequentNotes_.empty());

    if (!skip_date_init_) //Date
    {
        bool is_time_used = Option::instance().UseTransDateTime();
        wxDateTime trx_date;
        const wxString mask = is_time_used ? "%Y-%m-%dT%H:%M:%S" : "%Y-%m-%d";
        if (!trx_date.ParseFormat(m_fused_data.TRANSDATE, mask))
            trx_date.ParseDate(m_fused_data.TRANSDATE);
        dpc_->SetValue(trx_date);
        dpc_->SetFocus();
        skip_date_init_ = true;
    }

    if (!skip_status_init_) //Status
    {
        m_status = m_fused_data.STATUS;
        choiceStatus_->SetSelection(Model_Checking::status_id(m_status));
        skip_status_init_ = true;
    }

    //Type
    transaction_type_->SetSelection(Model_Checking::type_id(m_fused_data.TRANSCODE));

    //Account
    if (!skip_account_init_)
    {
        Model_Account::Data* acc = Model_Account::instance().get(m_fused_data.ACCOUNTID);
        if (acc)
        {
            cbAccount_->ChangeValue(acc->ACCOUNTNAME);
            m_textAmount->SetCurrency(Model_Currency::instance().get(acc->CURRENCYID));
        }
        Model_Account::Data* to_acc = Model_Account::instance().get(m_fused_data.TOACCOUNTID);
        if (to_acc) {
            cbToAccount_->ChangeValue(to_acc->ACCOUNTNAME);
            toTextAmount_->SetCurrency(Model_Currency::instance().get(to_acc->CURRENCYID));
        }

        skip_account_init_ = true;
        skip_amount_init_ = false; // Force amount format update in case account currencies change
    }

    if (m_transfer) {
        account_label_->SetLabelText(_t("From"));
        payee_label_->SetLabelText(_t("To"));
    }
    else if (!Model_Checking::is_deposit(m_fused_data.TRANSCODE)) {
        account_label_->SetLabelText(_t("Account"));
        payee_label_->SetLabelText(_t("Payee"));
    }
    else {
        account_label_->SetLabelText(_t("Account"));
        payee_label_->SetLabelText(_t("From"));
    }

    //Advanced
    cAdvanced_->Enable(m_transfer);
    cAdvanced_->SetValue(m_advanced && m_transfer);
    toTextAmount_->Enable(m_advanced && m_transfer);

    //Amounts
    if (!skip_amount_init_)
    {
        if (m_transfer && m_advanced)
            toTextAmount_->SetValue(m_fused_data.TOTRANSAMOUNT);
        else
            toTextAmount_->ChangeValue("");

        if (m_mode != MODE_NEW)
            m_textAmount->SetValue(m_fused_data.TRANSAMOUNT);
        skip_amount_init_ = true;
    }

    if (!skip_payee_init_) //Payee
    {
        cbPayee_->SetEvtHandlerEnabled(false);

        cbAccount_->UnsetToolTip();
        cbPayee_->UnsetToolTip();
        if (!m_transfer)
        {
            if (!Model_Checking::foreignTransaction(m_fused_data)) {
                m_fused_data.TOACCOUNTID = -1;
            }

            int64 accountID = cbAccount_->mmGetId();
            if (m_mode == MODE_NEW && Option::instance().getTransPayeeNone() == Option::LASTUSED
                && (-1 != accountID))
            {
                Model_Checking::Data_Set transactions = Model_Checking::instance().find(
                    Model_Checking::TRANSCODE(Model_Checking::TYPE_ID_TRANSFER, NOT_EQUAL)
                    , Model_Checking::ACCOUNTID(accountID, EQUAL));

                if (!transactions.empty()) {
                    Model_Payee::Data* payee = Model_Payee::instance().get(transactions.back().PAYEEID);
                    cbPayee_->ChangeValue(payee->PAYEENAME);
                }
            }
            else if (m_mode == MODE_NEW && Option::instance().getTransPayeeNone() == Option::UNUSED)
            {
                Model_Payee::Data *payee = Model_Payee::instance().get(_t("Unknown"));
                if (!payee)
                {
                    payee = Model_Payee::instance().create();
                    payee->PAYEENAME = _t("Unknown");
                    payee->ACTIVE = 1;
                    Model_Payee::instance().save(payee);
                    cbPayee_->mmDoReInitialize();
                }

                cbPayee_->ChangeValue(_t("Unknown"));
            }
            else
            {
                Model_Payee::Data* payee = Model_Payee::instance().get(m_fused_data.PAYEEID);
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

    bool has_split = !(m_local_splits.size() <= 1);
    if (!skip_category_init_)
    {
        bSplit_->UnsetToolTip();
        if (has_split)
        {
            cbCategory_->ChangeValue(_t("Split Transaction"));
            cbCategory_->Disable();
            m_textAmount->SetValue(Model_Splittransaction::get_total(m_local_splits));
            m_fused_data.CATEGID = -1;
        }
        else if (m_mode == MODE_NEW && m_transfer
            && Option::instance().getTransCategoryTransferNone() == Option::LASTUSED)
        {
            Model_Checking::Data_Set transactions = Model_Checking::instance().find(
                Model_Checking::TRANSCODE(Model_Checking::TYPE_ID_TRANSFER, EQUAL));

            if (!transactions.empty()
                && (!Model_Category::is_hidden(transactions.back().CATEGID)))
            {
                const int64 cat = transactions.back().CATEGID;
                cbCategory_->ChangeValue(Model_Category::full_name(cat));
            }
        } else
        {
            auto fullCategoryName = Model_Category::full_name(m_fused_data.CATEGID);
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
        for (const auto& tag : Model_Taglink::instance().find(
            Model_Taglink::REFTYPE((m_fused_data.m_repeat_num == 0) ?
                Model_Attachment::REFTYPE_NAME_TRANSACTION :
                Model_Attachment::REFTYPE_NAME_BILLSDEPOSIT),
            Model_Taglink::REFID((m_fused_data.m_repeat_num == 0) ?
                m_fused_data.TRANSID :
                m_fused_data.m_bdid))
        )
            tagIds.push_back(tag.TAGID);
        tagTextCtrl_->SetTags(tagIds);
        skip_tag_init_ = true;
    }

    if (!skip_notes_init_) //Notes & Transaction Number
    {
        textNumber_->SetValue(m_fused_data.TRANSACTIONNUMBER);
        textNotes_->SetValue(m_fused_data.NOTES);
        skip_notes_init_ = true;
    }

    if (!skip_tooltips_init_)
        SetTooltips();

    if (!m_fused_data.DELETEDTIME.IsEmpty()) {
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

void mmTransDialog::CreateControls()
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

    // Date -------------------------------------------
    wxStaticText* name_label = new wxStaticText(this, wxID_STATIC, _t("Date"));
    flex_sizer->Add(name_label, g_flagsH);
    name_label->SetFont(this->GetFont().Bold());

    dpc_ = new mmDatePickerCtrl(this, ID_DIALOG_TRANS_BUTTONDATE);
    flex_sizer->Add(dpc_->mmGetLayout());

    flex_sizer->AddSpacer(1);

    // Status --------------------------------------------
    choiceStatus_ = new wxChoice(this, ID_DIALOG_TRANS_STATUS);

    for (int i = 0; i < Model_Checking::STATUS_ID_size; ++i) {
        wxString status = Model_Checking::status_name(i);
        choiceStatus_->Append(wxGetTranslation(status), new wxStringClientData(status));
    }

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _t("Status")), g_flagsH);
    flex_sizer->Add(choiceStatus_, g_flagsH);
    flex_sizer->AddSpacer(1);

    // Type --------------------------------------------
    transaction_type_ = new wxChoice(this, ID_DIALOG_TRANS_TYPE);

    for (int i = 0; i < Model_Checking::TYPE_ID_size; ++i) {
        if (i != Model_Checking::TYPE_ID_TRANSFER ||
            Model_Account::instance().all().size() > 1
        ) {
            wxString type = Model_Checking::type_name(i);
            transaction_type_->Append(wxGetTranslation(type), new wxStringClientData(type));
        }
    }

    cAdvanced_ = new wxCheckBox(this
        , ID_DIALOG_TRANS_ADVANCED_CHECKBOX, _t("&Advanced")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );

    wxBoxSizer* typeSizer = new wxBoxSizer(wxHORIZONTAL);

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _t("Type")), g_flagsH);
    flex_sizer->Add(typeSizer);
    typeSizer->Add(transaction_type_, g_flagsH);
    typeSizer->Add(cAdvanced_, g_flagsH);
    flex_sizer->AddSpacer(1);

    // Amount Fields --------------------------------------------
    m_textAmount = new mmTextCtrl(this, mmID_TEXTAMOUNT, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    m_textAmount->SetMinSize(m_textAmount->GetSize());
    toTextAmount_ = new mmTextCtrl( this, mmID_TOTEXTAMOUNT, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    toTextAmount_->SetMinSize(toTextAmount_->GetSize());
    wxBoxSizer* amountSizer = new wxBoxSizer(wxHORIZONTAL);
    amountSizer->Add(m_textAmount, g_flagsExpand);
    amountSizer->Add(toTextAmount_, g_flagsExpand);

    wxStaticText* amount_label = new wxStaticText(this, wxID_STATIC, _t("Amount"));
    amount_label->SetFont(this->GetFont().Bold());
    flex_sizer->Add(amount_label, g_flagsH);
    flex_sizer->Add(amountSizer, wxSizerFlags(g_flagsExpand).Border(0));

    bCalc_ = new wxBitmapButton(this, wxID_ANY, mmBitmapBundle(png::CALCULATOR, mmBitmapButtonSize));
    bCalc_->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(mmTransDialog::OnCalculator), nullptr, this);
    mmToolTip(bCalc_, _t("Open Calculator"));
    flex_sizer->Add(bCalc_, g_flagsH);
    calcTarget_ = m_textAmount;
    calcPopup_ = new mmCalculatorPopup(bCalc_, calcTarget_);

    // Account ---------------------------------------------
    account_label_ = new wxStaticText(this, wxID_STATIC, _t("Account"));
    account_label_->SetFont(this->GetFont().Bold());

    cbAccount_ = new mmComboBoxAccount(this, mmID_ACCOUNTNAME, wxDefaultSize, m_fused_data.ACCOUNTID);
    cbAccount_->SetMinSize(cbAccount_->GetSize());
    flex_sizer->Add(account_label_, g_flagsH);
    flex_sizer->Add(cbAccount_, g_flagsExpand);
    flex_sizer->AddSpacer(1);

    // To Account ------------------------------------------------
    to_acc_label_ = new wxStaticText(this, mmID_TOACCOUNT_LABEL, _t("To"));
    to_acc_label_->SetFont(this->GetFont().Bold());
    flex_sizer->Add(to_acc_label_, g_flagsH);
    cbToAccount_ = new mmComboBoxAccount(this, mmID_TOACCOUNTNAME, wxDefaultSize, m_fused_data.TOACCOUNTID);
    cbToAccount_->SetMinSize(cbToAccount_->GetSize());
    flex_sizer->Add(cbToAccount_, g_flagsExpand);
    flex_sizer->AddSpacer(1);

    // Payee ---------------------------------------------
    payee_label_ = new wxStaticText(this, mmID_PAYEE_LABEL, _t("Payee"));
    payee_label_->SetFont(this->GetFont().Bold());

    cbPayee_ = new mmComboBoxPayee(this, mmID_PAYEE, wxDefaultSize, m_fused_data.PAYEEID, true);
    cbPayee_->SetMinSize(cbPayee_->GetSize());
    flex_sizer->Add(payee_label_, g_flagsH);
    flex_sizer->Add(cbPayee_, g_flagsExpand);
    flex_sizer->AddSpacer(1);


    // Category -------------------------------------------------

    categ_label_ = new wxStaticText(this, ID_DIALOG_TRANS_CATEGLABEL2, _t("Category"));
    categ_label_->SetFont(this->GetFont().Bold());
    cbCategory_ = new mmComboBoxCategory(this, mmID_CATEGORY, wxDefaultSize
        , m_fused_data.CATEGID, true);
    cbCategory_->SetMinSize(cbCategory_->GetSize());
    bSplit_ = new wxBitmapButton(this, mmID_CATEGORY_SPLIT, mmBitmapBundle(png::NEW_TRX, mmBitmapButtonSize));
    mmToolTip(bSplit_, _t("Use split Categories"));

    flex_sizer->Add(categ_label_, g_flagsH);
    flex_sizer->Add(cbCategory_, g_flagsExpand);
    flex_sizer->Add(bSplit_, g_flagsH);

    // Tags  ---------------------------------------------
    tagTextCtrl_ = new mmTagTextCtrl(this, ID_DIALOG_TRANS_TAGS);
    wxStaticText* tagLabel = new wxStaticText(this, wxID_STATIC, _t("Tags"));
    flex_sizer->Add(tagLabel, g_flagsH);
    flex_sizer->Add(tagTextCtrl_, g_flagsExpand);
    flex_sizer->AddSpacer(1);

    // Number  ---------------------------------------------

    textNumber_ = new wxTextCtrl(this, ID_DIALOG_TRANS_TEXTNUMBER, "", wxDefaultPosition, wxDefaultSize);

    bAuto = new wxBitmapButton(this, ID_DIALOG_TRANS_BUTTONTRANSNUM, mmBitmapBundle(png::TRXNUM, mmBitmapButtonSize));
    bAuto->Connect(ID_DIALOG_TRANS_BUTTONTRANSNUM, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(mmTransDialog::OnAutoTransNum), nullptr, this);
    mmToolTip(bAuto, _t("Populate Transaction #"));

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _t("Number")), g_flagsH);
    flex_sizer->Add(textNumber_, g_flagsExpand);
    flex_sizer->Add(bAuto, g_flagsH);

    // Frequently Used Notes
    wxButton* bFrequentUsedNotes = new wxButton(this, ID_DIALOG_TRANS_BUTTON_FREQENTNOTES
        , "...", wxDefaultPosition, bAuto->GetSize(), 0);
    mmToolTip(bFrequentUsedNotes, _t("Select one of the frequently used notes"));
    bFrequentUsedNotes->Connect(ID_DIALOG_TRANS_BUTTON_FREQENTNOTES
        , wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmTransDialog::OnFrequentUsedNotes), nullptr, this);
    
    // Colours
    bColours_ = new mmColorButton(this, wxID_LOWEST, bAuto->GetSize());
    mmToolTip(bColours_, _t("User Colors"));
    bColours_->SetBackgroundColor(m_fused_data.COLOR.GetValue());

    // Attachments
    bAttachments_ = new wxBitmapButton(this, wxID_FILE, mmBitmapBundle(png::CLIP, mmBitmapButtonSize));
    mmToolTip(bAttachments_, _t("Manage transaction attachments"));

    // Now display the Frequently Used Notes, Colour, Attachment buttons
    wxBoxSizer* notes_sizer = new wxBoxSizer(wxHORIZONTAL);
    flex_sizer->Add(notes_sizer);
    notes_sizer->Add(new wxStaticText(this, wxID_STATIC, _t("Notes")), g_flagsH);
    notes_sizer->Add(bFrequentUsedNotes, g_flagsH);

    wxBoxSizer* RightAlign_sizer = new wxBoxSizer(wxHORIZONTAL);
    flex_sizer->Add(RightAlign_sizer, wxSizerFlags(g_flagsH).Align(wxALIGN_RIGHT));
    RightAlign_sizer->Add(new wxStaticText(this, wxID_STATIC, _t("Color")), g_flagsH);
    RightAlign_sizer->Add(bColours_, wxSizerFlags());
    flex_sizer->Add(bAttachments_, g_flagsH);

    // Notes
    textNotes_ = new wxTextCtrl(this, ID_DIALOG_TRANS_TEXTNOTES, ""
        , wxDefaultPosition, wxSize(-1, dpc_->GetSize().GetHeight() * 5), wxTE_MULTILINE);
    mmToolTip(textNotes_, _t("Specify any text notes you want to add to this transaction."));
    box_sizer_left->Add(textNotes_, wxSizerFlags(g_flagsExpand).Border(wxLEFT | wxRIGHT | wxBOTTOM, 10));

    /**********************************************************************************************
     Button Panel with OK and Cancel Buttons
    ***********************************************************************************************/
    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    box_sizer_left->Add(buttons_panel, wxSizerFlags(g_flagsV).Center().Border(wxALL, 0));

    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    wxButton* button_ok = new wxButton(buttons_panel, wxID_OK, _t("&OK "));
    m_button_cancel = new wxButton(buttons_panel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));

    wxBitmapButton* button_hide = new wxBitmapButton(buttons_panel, ID_DIALOG_TRANS_CUSTOMFIELDS, mmBitmapBundle(png::RIGHTARROW, mmBitmapButtonSize));
    mmToolTip(button_hide, _t("Show/Hide custom fields window"));
    if (m_custom_fields->GetCustomFieldsCount() == 0) {
        button_hide->Hide();
    }

    buttons_sizer->Add(button_ok, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));
    buttons_sizer->Add(m_button_cancel, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));
    buttons_sizer->Add(button_hide, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));

    if (m_mode == MODE_EDIT) m_button_cancel->SetFocus();

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
}

bool mmTransDialog::ValidateData()
{
    if (!m_textAmount->checkValue(m_fused_data.TRANSAMOUNT))
        return false;
    if (!tagTextCtrl_->IsValid()) {
        mmErrorDialogs::ToolTip4Object(tagTextCtrl_, _t("Invalid value"), _t("Tags"), wxICON_ERROR);
        return false;
    }
    if (!cbAccount_->mmIsValid()) {
        mmErrorDialogs::ToolTip4Object(cbAccount_, _t("Invalid value"), _t("Account"), wxICON_ERROR);
        return false;
    }
    m_fused_data.ACCOUNTID = cbAccount_->mmGetId();
    const Model_Account::Data* account = Model_Account::instance().get(m_fused_data.ACCOUNTID);

    if (m_fused_data.TRANSDATE < account->INITIALDATE)
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
        m_fused_data.CATEGID = cbCategory_->mmGetCategoryId();
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

        Model_Payee::Data* payee = Model_Payee::instance().get(payee_name);
        if (!payee)
        {
            wxMessageDialog msgDlg( this
                , wxString::Format(_t("Payee name has not been used before. Is the name correct?\n%s"), payee_name)
                , _t("Confirm payee name")
                , wxYES_NO | wxYES_DEFAULT | wxICON_WARNING);
            if (msgDlg.ShowModal() == wxID_YES)
            {
                payee = Model_Payee::instance().create();
                payee->PAYEENAME = payee_name;
                payee->ACTIVE = 1;
                Model_Payee::instance().save(payee);
                mmWebApp::MMEX_WebApp_UpdatePayee();
            }
            else
                return false;
        }
        m_fused_data.TOTRANSAMOUNT = m_fused_data.TRANSAMOUNT;
        m_fused_data.PAYEEID = payee->PAYEEID;
        if (!Model_Checking::foreignTransaction(m_fused_data))
        {
            m_fused_data.TOACCOUNTID = -1;
        }

        if ((Option::instance().getTransCategoryNone() == Option::LASTUSED)
            && (!Model_Category::is_hidden(m_fused_data.CATEGID)))
        {
            payee->CATEGID = m_fused_data.CATEGID;
            Model_Payee::instance().save(payee);
            mmWebApp::MMEX_WebApp_UpdatePayee();
        }
    }
    else //transfer
    {
        const Model_Account::Data *to_account = Model_Account::instance().get(cbToAccount_->GetValue());

        if (!to_account || to_account->ACCOUNTID == m_fused_data.ACCOUNTID)
        {
            mmErrorDialogs::InvalidAccount(cbToAccount_, true);
            return false;
        }
        m_fused_data.TOACCOUNTID = to_account->ACCOUNTID;

        if (m_fused_data.TRANSDATE < to_account->INITIALDATE)
        {
            mmErrorDialogs::ToolTip4Object(cbToAccount_, _t("The opening date for the account is later than the date of this transaction"), _t("Invalid Date"));
            return false;
        }

        if (m_advanced)
        {
            if (!toTextAmount_->checkValue(m_fused_data.TOTRANSAMOUNT))
                return false;
        }
        m_fused_data.PAYEEID = -1;
    }

    /* Check if transaction is to proceed.*/
    if (Model_Account::BoolOf(account->STATEMENTLOCKED))
    {
        if (dpc_->GetValue() <= Model_Account::DateOf(account->STATEMENTDATE))
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
        if (m_fused_data.STATUS != Model_Checking::STATUS_KEY_VOID &&
            (m_fused_data.TRANSCODE == Model_Checking::TYPE_NAME_WITHDRAWAL ||
             m_fused_data.TRANSCODE == Model_Checking::TYPE_NAME_TRANSFER) &&
            (account->MINIMUMBALANCE != 0 || account->CREDITLIMIT != 0))
        {
            const double fromAccountBalance = Model_Account::balance(account);
            const double new_value = fromAccountBalance - m_fused_data.TRANSAMOUNT;

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
        m_fused_data.COLOR = color_id;
    else
        m_fused_data.COLOR = -1;

    return true;
}

//----------------------------------------------------------------------------
// Workaround for bug http://trac.wxwidgets.org/ticket/11630
void mmTransDialog::OnDpcKillFocus(wxFocusEvent& event)
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

void mmTransDialog::OnFocusChange(wxChildFocusEvent& event)
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
            m_fused_data.ACCOUNTID = cbAccount_->mmGetId();
            skip_account_init_ = false;
        }
        break;
    case mmID_TOACCOUNTNAME:
        cbToAccount_->ChangeValue(cbToAccount_->GetValue());
        if (cbToAccount_->mmIsValid())
        {
            m_fused_data.TOACCOUNTID = cbToAccount_->mmGetId();
            skip_account_init_ = false;
        }
        break;
    case mmID_PAYEE:
        cbPayee_->ChangeValue(cbPayee_->GetValue());
        m_fused_data.PAYEEID = cbPayee_->mmGetId();
        SetCategoryForPayee();
        break;
    case mmID_CATEGORY:
        cbCategory_->ChangeValue(cbCategory_->GetValue());
        break;
    case mmID_TEXTAMOUNT:
        if (m_textAmount->Calculate()) {
            m_textAmount->GetDouble(m_fused_data.TRANSAMOUNT);
        }
        skip_amount_init_ = false;
        calcTarget_ = m_textAmount;
        break;
    case mmID_TOTEXTAMOUNT:
        if (toTextAmount_->Calculate()) {
            toTextAmount_->GetDouble(m_fused_data.TOTRANSAMOUNT);
        }
        skip_amount_init_ = false;
        calcTarget_ = toTextAmount_;
        break;
    }

    object_in_focus_ = w->GetId();

    if (!m_transfer)
    {
        toTextAmount_->ChangeValue("");
        m_fused_data.TOACCOUNTID = -1;
    }
    else
    {
        const Model_Account::Data* to_account = Model_Account::instance().get(cbToAccount_->mmGetId());
        if (to_account)
            m_fused_data.TOACCOUNTID = to_account->ACCOUNTID;
    }

    dataToControls();
    event.Skip();
}

void mmTransDialog::SetDialogTitle(const wxString& title)
{
    this->SetTitle(title);
}

void mmTransDialog::OnPayeeChanged(wxCommandEvent& /*event*/)
{
    Model_Payee::Data * payee = Model_Payee::instance().get(cbPayee_->GetValue());
    if (payee)
    {
        SetCategoryForPayee(payee);
    }
}

void mmTransDialog::OnTransTypeChanged(wxCommandEvent& event)
{
    const wxString old_type = m_fused_data.TRANSCODE;
    wxStringClientData *client_obj = static_cast<wxStringClientData*>(event.GetClientObject());
    if (client_obj) m_fused_data.TRANSCODE = client_obj->GetData();
    if (old_type != m_fused_data.TRANSCODE)
    {
        m_transfer = Model_Checking::is_transfer(m_fused_data.TRANSCODE);
        if (m_transfer || Model_Checking::is_transfer(old_type))
            skip_payee_init_ = false;
        else
            skip_payee_init_ = true;
        skip_account_init_ = true;
        skip_tooltips_init_ = false;

        if (m_transfer) {
            m_fused_data.PAYEEID = -1;
            skip_category_init_ = false;
        } else {
            m_fused_data.TOTRANSAMOUNT = m_fused_data.TRANSAMOUNT;
            m_fused_data.TOACCOUNTID = -1;
        }
        dataToControls();
    }
}

void mmTransDialog::OnComboKey(wxKeyEvent& event)
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
                Model_Payee::Data* payee = Model_Payee::instance().get(payee_id);
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
                mmCategDialog dlg(this, true, -1);
                int rc = dlg.ShowModal();
                if (dlg.getRefreshRequested())
                    cbCategory_->mmDoReInitialize();
                if (rc != wxID_CANCEL) cbCategory_->ChangeValue(Model_Category::full_name(dlg.getCategId()));
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

void mmTransDialog::SetCategoryForPayee(const Model_Payee::Data *payee)
{
    // Only for new transactions: if user does not want to use categories.
    // If this is a Split Transaction, ignore displaying last category for payee
    if (m_mode == MODE_NEW && Option::instance().getTransCategoryNone() == Option::UNUSED
        && m_local_splits.empty())
    {
        Model_Category::Data *category = Model_Category::instance().get(_t("Unknown"), int64(-1));
        if (!category)
        {
            category = Model_Category::instance().create();
            category->CATEGNAME = _t("Unknown");
            category->ACTIVE = 1;
            Model_Category::instance().save(category);
            cbCategory_->mmDoReInitialize();
        }

        m_fused_data.CATEGID = category->CATEGID;
        cbCategory_->ChangeValue(_t("Unknown"));
        return;
    }

    if (!payee)
    {
        payee = Model_Payee::instance().get(cbPayee_->GetValue());
        if (!payee)
            return;
    }

    // Only for new transactions: if user want to autofill last category used for payee.
    // If this is a Split Transaction, ignore displaying last category for payee
    if ((Option::instance().getTransCategoryNone() == Option::LASTUSED ||
        Option::instance().getTransCategoryNone() == Option::DEFAULT)
        && m_mode == MODE_NEW && m_local_splits.empty()
        && (!Model_Category::is_hidden(payee->CATEGID)))
    {
        // if payee has memory of last category used then display last category for payee
        Model_Category::Data *category = Model_Category::instance().get(payee->CATEGID);
        if (category)
        {
            m_fused_data.CATEGID = payee->CATEGID;
            cbCategory_->ChangeValue(Model_Category::full_name(payee->CATEGID));
            wxLogDebug("Category: %s = %.2f", cbCategory_->GetLabel(), m_fused_data.TRANSAMOUNT);
        }
        else
        {
            m_fused_data.CATEGID = -1;
            cbCategory_->ChangeValue("");
        }
    }
}

void mmTransDialog::OnCalculator(wxCommandEvent& WXUNUSED(event))
{
    calcPopup_->SetTarget(calcTarget_);
    calcPopup_->Popup();
}

void mmTransDialog::OnAutoTransNum(wxCommandEvent& WXUNUSED(event))
    {
    auto d = Model_Checking::TRANSDATE(m_fused_data).Subtract(wxDateSpan::Months(12));
    double next_number = 0, temp_num;
    const auto numbers = Model_Checking::instance().find(
        Model_Checking::ACCOUNTID(m_fused_data.ACCOUNTID, EQUAL)
        , Model_Checking::TRANSDATE(d, GREATER_OR_EQUAL)
        , Model_Checking::TRANSACTIONNUMBER("", NOT_EQUAL));
    for (const auto &num : numbers)
    {
        if (!num.TRANSACTIONNUMBER.IsNumber()) continue;
        if (num.TRANSACTIONNUMBER.ToDouble(&temp_num) && temp_num > next_number)
            next_number = temp_num;
    }

    next_number++;
    textNumber_->SetValue(wxString::FromDouble(next_number, 0));
}

void mmTransDialog::OnAdvanceChecked(wxCommandEvent& WXUNUSED(event))
{
    m_advanced = cAdvanced_->IsChecked();
    skip_amount_init_ = false;
    dataToControls();
}

void mmTransDialog::OnCategs(wxCommandEvent& WXUNUSED(event))
{
    if (!m_textAmount->GetDouble(m_fused_data.TRANSAMOUNT)) {
        m_fused_data.TRANSAMOUNT = 0;
    }

    if (m_local_splits.empty() && cbCategory_->mmIsValid())
    {
        Split s;
        s.SPLITTRANSAMOUNT = m_fused_data.TRANSAMOUNT;
        s.CATEGID = cbCategory_->mmGetCategoryId();
        tagTextCtrl_->ValidateTagText();
        s.TAGS = tagTextCtrl_->GetTagIDs();
        s.NOTES = textNotes_->GetValue();
        m_local_splits.push_back(s);
    }

    bool isDeposit = Model_Checking::is_deposit(m_fused_data.TRANSCODE);
    mmSplitTransactionDialog dlg(this, m_local_splits
        , m_fused_data.ACCOUNTID
        , isDeposit ? Model_Checking::TYPE_ID_DEPOSIT : Model_Checking::TYPE_ID_WITHDRAWAL
        , m_fused_data.TRANSAMOUNT);

    if (dlg.ShowModal() == wxID_OK)
    {
        m_local_splits = dlg.mmGetResult();

        if (m_local_splits.size() == 1) {
            m_fused_data.CATEGID = m_local_splits[0].CATEGID;
            m_fused_data.TRANSAMOUNT = m_local_splits[0].SPLITTRANSAMOUNT;
            textNotes_->SetValue(m_local_splits[0].NOTES);
            m_textAmount->SetValue(m_fused_data.TRANSAMOUNT);
            tagTextCtrl_->Clear();
            wxString tagnames;
            for (const auto& tag : m_local_splits[0].TAGS)
                tagnames.Append(Model_Tag::instance().get(tag)->TAGNAME + " ");
            tagTextCtrl_->SetText(tagnames);
            m_local_splits.clear();
        }

        if (!m_local_splits.empty()) {
            m_textAmount->SetValue(m_fused_data.TRANSAMOUNT);
        }

        skip_category_init_ = false;
        skip_amount_init_ = false;
        skip_tooltips_init_ = false;
        dataToControls();
    }
    tagTextCtrl_->Reinitialize();
}

void mmTransDialog::OnAttachments(wxCommandEvent& WXUNUSED(event))
{
    const wxString& refType = (m_fused_data.m_repeat_num == 0) ?
        Model_Attachment::REFTYPE_NAME_TRANSACTION :
        Model_Attachment::REFTYPE_NAME_BILLSDEPOSIT;
    int64 transID = (m_mode == MODE_DUP) ? -1 : m_fused_data.TRANSID;
    mmAttachmentDialog dlg(this, refType, transID);
    dlg.ShowModal();
}

void mmTransDialog::OnTextEntered(wxCommandEvent& WXUNUSED(event))
{
    if (object_in_focus_ == m_textAmount->GetId())
    {
        if (m_textAmount->Calculate())
        {
            m_textAmount->GetDouble(m_fused_data.TRANSAMOUNT);
        }
    }
    else if (object_in_focus_ == toTextAmount_->GetId())
    {
        if (toTextAmount_->Calculate())
        {
            toTextAmount_->GetDouble(m_fused_data.TOTRANSAMOUNT);
        }
    }
    skip_amount_init_ = false;
    dataToControls();
}

void mmTransDialog::OnFrequentUsedNotes(wxCommandEvent& WXUNUSED(event))
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
        menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &mmTransDialog::OnNoteSelected, this);
        PopupMenu(&menu);
    }
}

void mmTransDialog::OnNoteSelected(wxCommandEvent& event)
{
    int i = event.GetId() - wxID_HIGHEST;

    if (i > 0 && static_cast<size_t>(i) <= frequentNotes_.size()) {
        if (!textNotes_->GetValue().EndsWith("\n") && !textNotes_->GetValue().empty())
            textNotes_->AppendText("\n");
        textNotes_->AppendText(frequentNotes_[i - 1]);
    }
}

void mmTransDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    m_fused_data.NOTES = textNotes_->GetValue();
    m_fused_data.TRANSACTIONNUMBER = textNumber_->GetValue();
    m_fused_data.TRANSDATE = dpc_->GetValue().FormatISOCombined();
    wxStringClientData* status_obj = static_cast<wxStringClientData*>(choiceStatus_->GetClientObject(choiceStatus_->GetSelection()));
    if (status_obj)
    {
        m_status = Model_Checking::status_key(status_obj->GetData());
        m_fused_data.STATUS = m_status;
    }

    if (!ValidateData()) return;
    if (!m_custom_fields->ValidateCustomValues((m_fused_data.m_repeat_num == 0) ?
        m_fused_data.TRANSID : -(m_fused_data.m_bdid))
    )
        return;

    if (!m_advanced)
        m_fused_data.TOTRANSAMOUNT = m_fused_data.TRANSAMOUNT;

    if (m_transfer && !m_advanced && (Model_Account::currency(Model_Account::instance().get(m_fused_data.ACCOUNTID))
        != Model_Account::currency(Model_Account::instance().get(m_fused_data.TOACCOUNTID))))
    {
        wxMessageDialog msgDlg( this
            , _t("The two accounts have different currencies, but no advanced transaction is defined. Is this correct?")
            , _t("Currencies are different")
            , wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
        if (msgDlg.ShowModal() == wxID_NO)
            return;
    }

    Model_Checking::Data *r = (m_mode == MODE_EDIT) ?
        Model_Checking::instance().get(m_fused_data.TRANSID) :
        Model_Checking::instance().create();

    Model_Checking::putDataToTransaction(r, m_fused_data);
    m_fused_data.TRANSID = Model_Checking::instance().save(r);
    m_fused_data.m_bdid = 0;
    m_fused_data.m_repeat_num = 0;

    Model_Splittransaction::Data_Set splt;
    for (const auto& entry : m_local_splits)
    {
        Model_Splittransaction::Data *s = Model_Splittransaction::instance().create();
        s->CATEGID = entry.CATEGID;
        s->SPLITTRANSAMOUNT = entry.SPLITTRANSAMOUNT;
        s->NOTES = entry.NOTES;
        splt.push_back(*s);
    }
    Model_Splittransaction::instance().update(splt, m_fused_data.TRANSID);

    // Save split tags
    const wxString& splitRefType = Model_Attachment::REFTYPE_NAME_TRANSACTIONSPLIT;

    for (unsigned int i = 0; i < m_local_splits.size(); i++)
    {
        Model_Taglink::Data_Set splitTaglinks;
        for (const auto& tagId : m_local_splits.at(i).TAGS)
        {
            Model_Taglink::Data* t = Model_Taglink::instance().create();
            t->REFTYPE = splitRefType;
            t->REFID = splt.at(i).SPLITTRANSID;
            t->TAGID = tagId;
            splitTaglinks.push_back(*t);
        }
        Model_Taglink::instance().update(splitTaglinks, splitRefType, splt.at(i).SPLITTRANSID);
    }
    const wxString& RefType = Model_Attachment::REFTYPE_NAME_TRANSACTION;
    if (m_mode != MODE_EDIT) {
        mmAttachmentManage::RelocateAllAttachments(RefType, -1, RefType, m_fused_data.TRANSID);
    }

    m_custom_fields->SaveCustomValues(m_fused_data.TRANSID);

    // Save base transaction tags
    Model_Taglink::Data_Set taglinks;
    for (const auto& tagId : tagTextCtrl_->GetTagIDs())
    {
        Model_Taglink::Data* t = Model_Taglink::instance().create();
        t->REFTYPE = RefType;
        t->REFID = m_fused_data.TRANSID;
        t->TAGID = tagId;
        taglinks.push_back(*t);
    }
    Model_Taglink::instance().update(taglinks, RefType, m_fused_data.TRANSID);

    const Model_Checking::Data& tran(*r);
    Model_Checking::Full_Data trx(tran);
    wxLogDebug("%s", trx.to_json());

    bool loop = Option::instance().getBulkTransactions();
    bool s = (wxGetKeyState(WXK_SHIFT) && !loop) || (!wxGetKeyState(WXK_SHIFT) && loop);
    if (m_mode == MODE_NEW && s)
        return EndModal(wxID_NEW);

    EndModal(wxID_OK);
}

void mmTransDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
#ifdef __WXMSW__
    if (object_in_focus_ != wxID_CANCEL && wxGetKeyState(WXK_ESCAPE))
        return m_button_cancel->SetFocus();

    if (object_in_focus_ != wxID_CANCEL) {
        return;
    }
#endif

    if (m_mode != MODE_EDIT) {
        const wxString& RefType = Model_Attachment::REFTYPE_NAME_TRANSACTION;
        mmAttachmentManage::DeleteAllAttachments(RefType, -1);
        Model_CustomFieldData::instance().DeleteAllData(RefType, -1);
    }
    EndModal(wxID_CANCEL);
}

void mmTransDialog::SetTooltips()
{
    bSplit_->UnsetToolTip();
    skip_tooltips_init_ = true;
    if (this->m_local_splits.empty())
        mmToolTip(bSplit_, _t("Use split Categories"));
    else {
        const Model_Currency::Data* currency = Model_Currency::GetBaseCurrency();
        const Model_Account::Data* account = Model_Account::instance().get(m_fused_data.ACCOUNTID);
        if (account)
            currency = Model_Account::currency(account);

        bSplit_->SetToolTip(Model_Splittransaction::get_tooltip(m_local_splits, currency));
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
        if (!Model_Checking::is_deposit(m_fused_data.TRANSCODE))
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

void mmTransDialog::OnQuit(wxCloseEvent& WXUNUSED(event))
{
    const wxString& RefType = Model_Attachment::REFTYPE_NAME_TRANSACTION;
    if (m_mode != MODE_EDIT) {
        mmAttachmentManage::DeleteAllAttachments(RefType, -1);
        Model_CustomFieldData::instance().DeleteAllData(RefType, -1);
    }
    EndModal(wxID_CANCEL);
}

void mmTransDialog::OnMoreFields(wxCommandEvent& WXUNUSED(event))
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
