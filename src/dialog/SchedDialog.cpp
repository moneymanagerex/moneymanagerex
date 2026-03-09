/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2016 Stefano Giorgio
 Copyright (C) 2016 - 2022 Nikolay Akimov
 Copyright (C) 2021, 2022 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2025 KLaus Wich

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
#include <wx/calctrl.h>
#include <wx/valnum.h>

#include "base/constants.h"
#include "base/paths.h"
#include "base/images_list.h"
#include "util/_util.h"
#include "util/_simple.h"
#include "util/mmDate.h"
#include "util/mmTextCtrl.h"
#include "util/mmCalcValidator.h"

#include "model/PayeeModel.h"
#include "model/CurrencyHistoryModel.h"
#include "model/PrefModel.h"

#include "manager/CategoryManager.h"
#include "manager/PayeeManager.h"
#include "AttachmentDialog.h"
#include "SchedDialog.h"
#include "SplitDialog.h"
#include "import_export/webapp.h"

// the order in gui may be different than the database encoding order.
const std::vector<std::pair<int, wxString> > SchedDialog::BILLSDEPOSITS_REPEATS =
{
    { SchedModel::REPEAT_FREQ_ONCE,                      _n("Once") },
    { SchedModel::REPEAT_FREQ_WEEKLY,                    _n("Weekly") },
    { SchedModel::REPEAT_FREQ_BI_WEEKLY,                 _n("Fortnightly") },
    { SchedModel::REPEAT_FREQ_MONTHLY,                   _n("Monthly") },
    { SchedModel::REPEAT_FREQ_BI_MONTHLY,                _n("Every 2 Months") },
    { SchedModel::REPEAT_FREQ_QUARTERLY,                 _n("Quarterly") },
    { SchedModel::REPEAT_FREQ_HALF_YEARLY,               _n("Half-Yearly") },
    { SchedModel::REPEAT_FREQ_YEARLY,                    _n("Yearly") },
    { SchedModel::REPEAT_FREQ_FOUR_MONTHLY,              _n("Four Months") },
    { SchedModel::REPEAT_FREQ_FOUR_WEEKLY,               _n("Four Weeks") },
    { SchedModel::REPEAT_FREQ_DAILY,                     _n("Daily") },
    { SchedModel::REPEAT_FREQ_IN_X_DAYS,                 _n("In (n) Days") },
    { SchedModel::REPEAT_FREQ_IN_X_MONTHS,               _n("In (n) Months") },
    { SchedModel::REPEAT_FREQ_EVERY_X_DAYS,              _n("Every (n) Days") },
    { SchedModel::REPEAT_FREQ_EVERY_X_MONTHS,            _n("Every (n) Months") },
    { SchedModel::REPEAT_FREQ_MONTHLY_LAST_DAY,          _n("Monthly (last day)") },
    { SchedModel::REPEAT_FREQ_MONTHLY_LAST_BUSINESS_DAY, _n("Monthly (last business day)") }
};

// Used to determine if we need to refresh the tag text ctrl after
// accelerator hints are shown which only occurs once.
static bool altRefreshDone;

wxIMPLEMENT_DYNAMIC_CLASS(SchedDialog, wxDialog);

wxBEGIN_EVENT_TABLE(SchedDialog, wxDialog)
EVT_CHAR_HOOK(SchedDialog::OnComboKey)
EVT_CHILD_FOCUS(SchedDialog::OnFocusChange)
EVT_BUTTON(wxID_OK, SchedDialog::OnOk)
EVT_BUTTON(wxID_CANCEL, SchedDialog::OnCancel)
EVT_BUTTON(mmID_CATEGORY, SchedDialog::OnCategs)
EVT_BUTTON(ID_DIALOG_TRANS_BUTTONSPLIT, SchedDialog::OnCategs)
EVT_TEXT(mmID_PAYEE, SchedDialog::OnPayee)
EVT_BUTTON(wxID_FILE, SchedDialog::OnAttachments)
EVT_BUTTON(ID_BTN_CUSTOMFIELDS, SchedDialog::OnMoreFields)
EVT_CHOICE(wxID_VIEW_DETAILS, SchedDialog::OnTypeChanged)
EVT_CHECKBOX(ID_DIALOG_TRANS_ADVANCED_CHECKBOX, SchedDialog::OnAdvanceChecked)
EVT_CHECKBOX(ID_DIALOG_BD_CHECKBOX_AUTO_EXECUTE_USERACK, SchedDialog::OnAutoExecutionUserAckChecked)
EVT_CHECKBOX(ID_DIALOG_BD_CHECKBOX_AUTO_EXECUTE_SILENT, SchedDialog::OnAutoExecutionSilentChecked)
EVT_CHOICE(ID_DIALOG_BD_COMBOBOX_REPEATS, SchedDialog::OnRepeatTypeChanged)
EVT_BUTTON(ID_DIALOG_TRANS_BUTTONTRANSNUMPREV, SchedDialog::OnsetPrevOrNextRepeatDate)
EVT_BUTTON(ID_DIALOG_TRANS_BUTTONTRANSNUM, SchedDialog::OnsetPrevOrNextRepeatDate)
EVT_TEXT(mmID_ACCOUNTNAME, SchedDialog::OnAccountUpdated)
EVT_CLOSE(SchedDialog::OnQuit)
wxEND_EVENT_TABLE()


SchedDialog::SchedDialog()
{
}

SchedDialog::~SchedDialog()
{
    wxSize size = GetSize();
    if (m_custom_fields->IsCustomPanelShown())
        size = wxSize(GetSize().GetWidth() - m_custom_fields->GetMinWidth(), GetSize().GetHeight());
    InfoModel::instance().setSize("RECURRINGTRANS_DIALOG_SIZE", size);
}

SchedDialog::SchedDialog(
    wxWindow* parent,
    int64 sched_id,
    bool duplicate,
    bool enterOccur
) :
    m_dup_bill(duplicate),
    m_enter_occur(enterOccur)
{
    const SchedData* sched_n = SchedModel::instance().get_id_data_n(sched_id);
    m_new_bill = sched_n ? false : true;

    if (!m_new_bill) {
        // If duplicate then we will be creating a new identity
        if (!m_dup_bill)
            m_sched_xd.m_id = sched_id;
        m_sched_xd.TRANSDATE          = sched_n->TRANSDATE;
        m_sched_xd.m_account_id       = sched_n->m_account_id;
        m_sched_xd.m_to_account_id_n  = sched_n->m_to_account_id_n;
        m_sched_xd.m_payee_id_n       = sched_n->m_payee_id_n;
        m_sched_xd.m_category_id_n    = sched_n->m_category_id_n;
        m_sched_xd.m_to_amount        = sched_n->m_to_amount;
        m_sched_xd.m_amount           = sched_n->m_amount;
        m_sched_xd.NEXTOCCURRENCEDATE = sched_n->NEXTOCCURRENCEDATE;
        m_sched_xd.REPEATS            = sched_n->REPEATS;
        m_sched_xd.NUMOCCURRENCES     = sched_n->NUMOCCURRENCES;
        m_sched_xd.m_notes            = sched_n->m_notes;
        m_sched_xd.STATUS             = sched_n->STATUS;
        m_sched_xd.m_number           = sched_n->m_number;
        m_sched_xd.TRANSCODE          = sched_n->TRANSCODE;
        m_sched_xd.m_followup_id      = sched_n->m_followup_id;
        m_sched_xd.m_color            = sched_n->m_color;

        wxArrayInt64 tag_id_a;
        for (const auto& gl_d : TagLinkModel::instance().find(
            TagLinkCol::REFTYPE(SchedModel::s_ref_type.name_n()),
            TagLinkCol::REFID(sched_n->m_id)
        ))
            tag_id_a.push_back(gl_d.m_tag_id);
        m_sched_xd.TAGS = tag_id_a;

        for (const auto& qp_d : SchedModel::split(*sched_n)) {
            wxArrayInt64 split_tag_id_a;
            for (const auto& gl_d : TagLinkModel::instance().find(
                TagLinkCol::REFTYPE(SchedSplitModel::s_ref_type.name_n()),
                TagLinkCol::REFID(qp_d.m_id)
            ))
                split_tag_id_a.push_back(gl_d.m_tag_id);
            m_sched_xd.local_splits.push_back(
                { qp_d.m_category_id, qp_d.m_amount, split_tag_id_a, qp_d.m_notes }
            );
        }

        // If duplicate then we may need to copy the attachments
        if (m_dup_bill && InfoModel::instance().getBool("ATTACHMENTSDUPLICATE", false)) {
            // FIXME: id 0 does not exist in database
            mmAttachmentManage::CloneAllAttachments(
                SchedModel::s_ref_type, sched_id, 0
            );
        }
    }

    m_transfer = (m_sched_xd.TRANSCODE == TrxModel::TYPE_NAME_TRANSFER);

    m_custom_fields = new mmCustomDataTransaction(this,
        SchedModel::s_ref_type,
        (m_dup_bill ? sched_id : !m_new_bill ? m_sched_xd.m_id : 0),
        ID_CUSTOMFIELDS
    );

    this->SetFont(parent->GetFont());
    Create(parent);
    mmThemeAutoColour(this);
}

bool SchedDialog::Create(wxWindow* parent, wxWindowID id, const wxString& caption
    , const wxPoint& pos, const wxSize& size, long style, const wxString& name)
{
    altRefreshDone = false; // reset the ALT refresh indicator on new dialog creation
    style |= wxRESIZE_BORDER;
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style, name);

    CreateControls();
    dataToControls();

    //generate date change events for set weekday name
    wxDateEvent dateEventPaid(m_date_paid, m_date_paid->GetValue(), wxEVT_DATE_CHANGED);
    GetEventHandler()->ProcessEvent(dateEventPaid);
    wxDateEvent dateEventDue(m_date_due, m_date_due->GetValue(), wxEVT_DATE_CHANGED);
    GetEventHandler()->ProcessEvent(dateEventDue);

    mmSetSize(this);
    // set the initial dialog size to expand the payee and category comboboxes to fit their text
    int minWidth = std::max(cbPayee_->GetSize().GetX(),
        cbPayee_->GetSizeFromTextSize(cbPayee_->GetTextExtent(cbPayee_->GetValue()).GetX()).GetX()) - cbPayee_->GetSize().GetWidth();
    minWidth = std::max(minWidth,
        cbCategory_->GetSizeFromTextSize(cbCategory_->GetTextExtent(cbCategory_->GetValue()).GetX()).GetX() - cbCategory_->GetSize().GetWidth());
    wxSize sz = wxSize(GetMinWidth() + minWidth + (m_custom_fields->IsCustomPanelShown() ? m_custom_fields->GetMinWidth() : 0), GetMinHeight());
    if (sz.GetWidth() > GetSize().GetWidth()) SetSize(sz);
    SetIcon(mmex::getProgramIcon());
    Centre(wxCENTER_ON_SCREEN);

    return true;
}

void SchedDialog::dataToControls()
{
    TrxModel::getFrequentUsedNotes(frequentNotes_);
    wxButton* bFrequentUsedNotes = static_cast<wxButton*>(FindWindow(ID_DIALOG_TRANS_BUTTON_FREQENTNOTES));
    bFrequentUsedNotes->Enable(!frequentNotes_.empty());

    bColours_->SetBackgroundColor(m_sched_xd.m_color.GetValue());

    for (const auto& entry : BILLSDEPOSITS_REPEATS)
    {
        m_choice_repeat->Append(wxGetTranslation(entry.second));
    }
    setRepeatType(SchedModel::REPEAT_FREQ_MONTHLY);

    for (int i = 0; i < TrxModel::TYPE_ID_size; ++i) {
        if (i == TrxModel::TYPE_ID_TRANSFER && AccountModel::instance().find_all().size() < 2)
            break;
        wxString type = TrxModel::type_name(i);
        m_choice_transaction_type->Append(wxGetTranslation(type), new wxStringClientData(type));
    }
    m_choice_transaction_type->SetSelection(TrxModel::TYPE_ID_WITHDRAWAL);

    SetTransferControls();  // hide appropriate fields
    setCategoryLabel();

    if (!(!m_new_bill || m_enter_occur)) {
        return;
    }

    m_choice_status->SetSelection(TrxModel::status_id(m_sched_xd.STATUS));

    // Set the date paid
    wxDateTime field_date;
    field_date.ParseDateTime(m_sched_xd.TRANSDATE) || field_date.ParseDate(m_sched_xd.TRANSDATE);
    m_date_paid->SetValue(field_date);

    // Set the due Date
    field_date.ParseDate(m_sched_xd.NEXTOCCURRENCEDATE);
    m_date_due->SetValue(field_date);

    // demultiplex REPEATS and NUMOCCURRENCES
    SchedModel::RepeatNum rn;
    SchedModel::decode_repeat_num(m_sched_xd, rn);
    // fix repeats
    if (rn.freq < 0 || rn.freq > SchedModel::REPEAT_FREQ_size) {
        wxFAIL;
        rn.freq = SchedModel::REPEAT_FREQ_MONTHLY;
    }
    // old invalid entry. change to REPEAT_FREQ_ONCE and turn off automatic execution.
    if (rn.freq >= SchedModel::REPEAT_FREQ_IN_X_DAYS &&
        rn.freq <= SchedModel::REPEAT_FREQ_EVERY_X_MONTHS && rn.x < 1
    ) {
        rn.exec = SchedModel::REPEAT_EXEC_NONE;
        rn.freq = SchedModel::REPEAT_FREQ_ONCE;
    }
    setRepeatType(rn.freq);

    if (rn.freq != SchedModel::REPEAT_FREQ_ONCE && m_sched_xd.NUMOCCURRENCES > 0) {
        textNumRepeats_->SetValue(wxString::Format("%lld", m_sched_xd.NUMOCCURRENCES));
    }

    if (rn.exec == SchedModel::REPEAT_EXEC_SILENT) {
        autoExecuteSilent_ = true;
        itemCheckBoxAutoExeSilent_->SetValue(true);
        itemCheckBoxAutoExeUserAck_->Enable(false);
    }
    else if (rn.exec == SchedModel::REPEAT_EXEC_MANUAL) {
        autoExecuteUserAck_ = true;
        itemCheckBoxAutoExeUserAck_->SetValue(true);
        itemCheckBoxAutoExeSilent_->Enable(false);
    }
    setRepeatDetails();

    m_choice_transaction_type->SetSelection(TrxModel::type_id(m_sched_xd.TRANSCODE));
    updateControlsForTransType();

    const AccountData* account_n = AccountModel::instance().get_id_data_n(m_sched_xd.m_account_id);
    cbAccount_->ChangeValue(account_n ? account_n->m_name : "");

    tagTextCtrl_->SetTags(m_sched_xd.TAGS);

    textNotes_->SetValue(m_sched_xd.m_notes);
    textNumber_->SetValue(m_sched_xd.m_number);

    if (!m_sched_xd.local_splits.empty())
        m_sched_xd.m_amount = TrxSplitModel::get_total(m_sched_xd.local_splits);

    SetAmountCurrencies(m_sched_xd.m_account_id, m_sched_xd.m_to_account_id_n);
    textAmount_->SetValue(m_sched_xd.m_amount);

    if (m_transfer) {
        m_sched_xd.m_payee_id_n = -1;

        // When editing an advanced transaction record, we do not reset the m_sched_xd.m_to_amount
        if ((!m_new_bill || m_enter_occur)
            && (m_sched_xd.m_to_amount != m_sched_xd.m_amount)
        ) {
            cAdvanced_->SetValue(true);
            SetAdvancedTransferControls(true);
        }
    }

    if (!m_enter_occur) {
        if (m_dup_bill)
            SetDialogHeader(_t("Duplicate Scheduled Transaction"));
        else
            SetDialogHeader(_t("Edit Scheduled Transaction"));
        textAmount_->SetFocus();
    }
    else {
        SetDialogHeader(_t("Enter Scheduled Transaction"));
        m_date_due->Enable(false);
        m_choice_transaction_type->Disable();
        m_choice_repeat->Disable();
        itemCheckBoxAutoExeSilent_->Disable();
        itemCheckBoxAutoExeUserAck_->Disable();
        textNumRepeats_->Disable();
        m_btn_due_prev_date->Disable();
        m_btn_due_date->Disable();
        auto bok = static_cast<wxButton*>(FindWindowById(wxID_OK, this));
        if (bok) bok->SetFocus();
    }

    setTooltips();
}

void SchedDialog::SetDialogHeader(const wxString& header)
{
    this->SetTitle(header);
}

void SchedDialog::SetDialogParameters(int64 trx_id)
{
    const auto split = TrxSplitModel::instance().get_all_id();
    const auto schedId_glA_m = TagLinkModel::instance().find_refType_mRefId(
        SchedModel::s_ref_type
    );
    //const auto trx = TrxModel::instance().find(TrxCol::TRANSID(trx_id)).at(0);
    const TrxData* trx_n = TrxModel::instance().get_id_data_n(trx_id);
    TrxModel::Full_Data trx_xd(*trx_n, split, schedId_glA_m);
    m_sched_xd.m_account_id = trx_xd.m_account_id;
    cbAccount_->SetValue(trx_xd.ACCOUNTNAME);

    m_sched_xd.TRANSCODE = trx_xd.TRANSCODE;
    m_choice_transaction_type->SetSelection(TrxModel::type_id(trx_xd.TRANSCODE));
    m_transfer = (m_sched_xd.TRANSCODE == TrxModel::TYPE_NAME_TRANSFER);
    updateControlsForTransType();

    m_sched_xd.m_amount = trx_xd.m_amount;
    SetAmountCurrencies(trx_xd.m_account_id, trx_xd.m_to_account_id_n);
    textAmount_->SetValue(m_sched_xd.m_amount);

    if (m_transfer) {
        m_sched_xd.m_to_account_id_n = trx_xd.m_to_account_id_n;
        cbToAccount_->ChangeValue(trx_xd.TOACCOUNTNAME);

        m_sched_xd.m_to_amount = trx_xd.m_to_amount;
        toTextAmount_->SetValue(m_sched_xd.m_to_amount);
        if (m_sched_xd.m_to_amount != m_sched_xd.m_amount) {
            cAdvanced_->SetValue(true);
            SetAdvancedTransferControls(true);
        }
    }
    else {
        m_sched_xd.m_payee_id_n = trx_xd.m_payee_id_n;
        cbPayee_->ChangeValue(trx_xd.PAYEENAME);
    }

    if (trx_xd.has_split()) {
        for (auto& tp_d : trx_xd.m_splits) {
            Split split_d;
            split_d.CATEGID          = tp_d.m_category_id;
            split_d.SPLITTRANSAMOUNT = tp_d.m_amount;
            split_d.NOTES            = tp_d.m_notes;
            m_sched_xd.local_splits.push_back(split_d);
        }
    }
    else {
        m_sched_xd.m_category_id_n = trx_xd.m_category_id_n;
    }

    m_sched_xd.m_number = trx_xd.m_number;
    textNumber_->SetValue(m_sched_xd.m_number);
    m_sched_xd.m_notes = trx_xd.m_notes;
    textNotes_->SetValue(m_sched_xd.m_notes);
    setCategoryLabel();
}

void SchedDialog::CreateControls()
{
    wxBoxSizer* mainBoxSizerOuter = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* mainBoxSizerInner = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* custom_fields_box_sizer = new wxBoxSizer(wxHORIZONTAL);

    /**********************************************************************************************
     Determining where the controls go
    ***********************************************************************************************/

    /* Bills & Deposits Details */

    wxStaticBox* repeatDetailsStaticBox = new wxStaticBox(this, wxID_ANY, _t("Scheduled Transaction Details"));
    wxStaticBoxSizer* repeatTransBoxSizer = new wxStaticBoxSizer(repeatDetailsStaticBox, wxVERTICAL);

    //mainBoxSizerInner will align contents horizontally
    mainBoxSizerInner->Add(repeatTransBoxSizer, g_flagsV);
    //mainBoxSizerOuter will align contents vertically
    mainBoxSizerOuter->Add(mainBoxSizerInner, g_flagsExpand);

    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    repeatTransBoxSizer->Add(itemFlexGridSizer5);

    // Date Due --------------------------------------------

    m_date_due = new mmDatePickerCtrl(this, ID_DIALOG_BD_DUE_DATE);
    mmToolTip(m_date_due, _t("Specify the date when this bill or deposit is due"));
    itemFlexGridSizer5->Add(new wxStaticText(this, wxID_STATIC, _t("Date Due")), g_flagsH);
    itemFlexGridSizer5->Add(m_date_due->mmGetLayout(false));

    // Repeats --------------------------------------------

    staticTextRepeats_ = new wxStaticText(this, wxID_STATIC, _t("Repeats"));
    itemFlexGridSizer5->Add(staticTextRepeats_, g_flagsH);
    itemFlexGridSizer5->AddSpacer(1);

    m_btn_due_prev_date = new wxBitmapButton(this, ID_DIALOG_TRANS_BUTTONTRANSNUMPREV
        , mmBitmapBundle(png::LEFTARROW, mmBitmapButtonSize));
    mmToolTip(m_btn_due_prev_date, _t("Back to the last occurring date with the specified values"));

    m_choice_repeat = new wxChoice(this, ID_DIALOG_BD_COMBOBOX_REPEATS);

    wxBoxSizer* repeatBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    m_btn_due_date = new wxBitmapButton(this, ID_DIALOG_TRANS_BUTTONTRANSNUM
        , mmBitmapBundle(png::RIGHTARROW, mmBitmapButtonSize));
    mmToolTip(m_btn_due_date, _t("Advance the next occurring date with the specified values"));
    repeatBoxSizer->Add(m_btn_due_prev_date, g_flagsH);
    repeatBoxSizer->Add(m_choice_repeat, wxSizerFlags(g_flagsExpand));
    repeatBoxSizer->Add(m_btn_due_date, g_flagsH);

    repeatTransBoxSizer->Add(repeatBoxSizer);

    wxFlexGridSizer* itemFlexGridSizer52 = new wxFlexGridSizer(0, 2, 0, 0);
    repeatTransBoxSizer->Add(itemFlexGridSizer52);

    // Repeat Times --------------------------------------------
    staticTimesRepeat_ = new wxStaticText(this, wxID_STATIC, _t("Payments Left"));
    itemFlexGridSizer52->Add(staticTimesRepeat_, g_flagsH);

    wxBoxSizer* repeatTimesBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer52->Add(repeatTimesBoxSizer);

    wxIntegerValidator<int> intValidator(nullptr, wxNUM_VAL_ZERO_AS_BLANK);
    intValidator.SetMin(0);
    textNumRepeats_ = new wxTextCtrl(this, ID_DIALOG_BD_TEXTCTRL_NUM_TIMES, "", wxDefaultPosition, wxDefaultSize, 0, intValidator);
    repeatTimesBoxSizer->Add(textNumRepeats_, g_flagsH);
    textNumRepeats_->SetMaxLength(12);
    setRepeatDetails();

    /* Auto Execution Status */
    itemCheckBoxAutoExeUserAck_ = new wxCheckBox(this, ID_DIALOG_BD_CHECKBOX_AUTO_EXECUTE_USERACK
        ,  _t("Request user to enter payment"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    mmToolTip(itemCheckBoxAutoExeUserAck_, _t("User requested to enter this transaction on the 'Date Paid'"));

    itemCheckBoxAutoExeSilent_ = new wxCheckBox(this, ID_DIALOG_BD_CHECKBOX_AUTO_EXECUTE_SILENT
        , _t("Grant automatic execute"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    mmToolTip(itemCheckBoxAutoExeSilent_, _t("The requested payment will occur without user interaction"));

    repeatTransBoxSizer->Add(itemCheckBoxAutoExeUserAck_, g_flagsExpand);
    repeatTransBoxSizer->Add(itemCheckBoxAutoExeSilent_, g_flagsExpand);

    /*************************************************************************************************************/

    wxStaticBox* transDetailsStaticBox = new wxStaticBox(this, wxID_REMOVE, _t("Transaction Details"));
    wxStaticBoxSizer* transDetailsStaticBoxSizer = new wxStaticBoxSizer(transDetailsStaticBox, wxVERTICAL);
    wxFlexGridSizer* transPanelSizer = new wxFlexGridSizer(0, 3, 0, 0);
    transPanelSizer->AddGrowableCol(1, 0);
    transDetailsStaticBoxSizer->Add(transPanelSizer, wxSizerFlags(g_flagsV).Expand());
    mainBoxSizerInner->Add(transDetailsStaticBoxSizer, g_flagsExpand);

    // Trans Date --------------------------------------------
    m_date_paid = new mmDatePickerCtrl(this, ID_DIALOG_TRANS_BUTTON_PAYDATE);
    mmToolTip(m_date_paid, _t("Specify the date the user is requested to enter this transaction"));
    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _t("Date Paid")), g_flagsH);
    transPanelSizer->Add(m_date_paid->mmGetLayout());
    transPanelSizer->AddSpacer(1);

    // Status --------------------------------------------
    m_choice_status = new wxChoice(this, ID_DIALOG_TRANS_STATUS);

    for (int i = 0; i < TrxModel::STATUS_ID_size; ++i) {
        wxString status = TrxModel::status_name(i);
        m_choice_status->Append(wxGetTranslation(status), new wxStringClientData(status));
    }
    m_choice_status->SetSelection(PrefModel::instance().getTransStatusReconciled());
    mmToolTip(m_choice_status, _t("Specify the status for the transaction"));

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _t("Status")), g_flagsH);
    transPanelSizer->Add(m_choice_status, g_flagsExpand);
    transPanelSizer->AddSpacer(1);

    // Type --------------------------------------------
    m_choice_transaction_type = new wxChoice(this, wxID_VIEW_DETAILS);
    mmToolTip(m_choice_transaction_type, _t("Specify the type of transactions to be created."));
    cAdvanced_ = new wxCheckBox(this, ID_DIALOG_TRANS_ADVANCED_CHECKBOX, _t("&Advanced")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cAdvanced_->SetValue(false);
    mmToolTip(cAdvanced_, _t("Allows the setting of different amounts in the FROM and TO accounts."));

    wxBoxSizer* typeSizer = new wxBoxSizer(wxHORIZONTAL);
    typeSizer->Add(m_choice_transaction_type, g_flagsExpand);
    typeSizer->Add(cAdvanced_, g_flagsH);

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _t("Type")), g_flagsH);
    transPanelSizer->Add(typeSizer, g_flagsExpand);
    transPanelSizer->AddSpacer(1);

    // Amount Fields --------------------------------------------
    wxStaticText* amount_label = new wxStaticText(this, wxID_STATIC, _t("Amount"));
    amount_label->SetFont(this->GetFont().Bold());

    textAmount_ = new mmTextCtrl(this, ID_DIALOG_TRANS_TEXTAMOUNT, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER
        , mmCalcValidator());
    mmToolTip(textAmount_, amountNormalTip_);

    toTextAmount_ = new mmTextCtrl(this, ID_DIALOG_TRANS_TOTEXTAMOUNT, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER
        , mmCalcValidator());
    mmToolTip(toTextAmount_, _t("Specify the transfer amount in the To Account"));

    wxBoxSizer* amountSizer = new wxBoxSizer(wxHORIZONTAL);
    amountSizer->Add(textAmount_, g_flagsExpand);
    amountSizer->Add(toTextAmount_, g_flagsExpand);

    transPanelSizer->Add(amount_label, g_flagsH);
    transPanelSizer->Add(amountSizer, wxSizerFlags(g_flagsExpand).Border(0));

    bCalc_ = new wxBitmapButton(this, wxID_ANY, mmBitmapBundle(png::CALCULATOR, mmBitmapButtonSize));
    bCalc_->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(SchedDialog::OnCalculator), nullptr, this);
    mmToolTip(bCalc_, _t("Open Calculator"));
    transPanelSizer->Add(bCalc_, g_flagsH);
    calcTarget_ = textAmount_;
    calcPopup_ = new mmCalculatorPopup(bCalc_, calcTarget_);

    // Account ------------------------------------------------
    wxStaticText* acc_label = new wxStaticText(this, ID_DIALOG_TRANS_STATIC_ACCOUNT, _t("Account"));
    acc_label->SetFont(this->GetFont().Bold());
    transPanelSizer->Add(acc_label, g_flagsH);
    cbAccount_ = new mmComboBoxAccount(this, mmID_ACCOUNTNAME, wxDefaultSize, m_sched_xd.m_account_id);
    cbAccount_->SetMinSize(cbAccount_->GetSize());
    mmToolTip(cbAccount_, _t("Specify the Account that will own the scheduled transaction"));
    transPanelSizer->Add(cbAccount_, g_flagsExpand);
    transPanelSizer->AddSpacer(1);

    // To Account ------------------------------------------------
    wxStaticText* to_acc_label = new wxStaticText(this, ID_DIALOG_TRANS_STATIC_TOACCOUNT, _t("To"));
    to_acc_label->SetFont(this->GetFont().Bold());
    transPanelSizer->Add(to_acc_label, g_flagsH);
    cbToAccount_ = new mmComboBoxAccount(this, mmID_TOACCOUNTNAME, wxDefaultSize, m_sched_xd.m_to_account_id_n);
    cbToAccount_->SetMinSize(cbToAccount_->GetSize());
    mmToolTip(cbToAccount_, payeeTransferTip_);
    transPanelSizer->Add(cbToAccount_, g_flagsExpand);
    transPanelSizer->AddSpacer(1);

    // Payee ------------------------------------------------
    wxStaticText* payee_label = new wxStaticText(this, ID_DIALOG_TRANS_STATIC_PAYEE, _t("Payee"));
    payee_label->SetFont(this->GetFont().Bold());

    cbPayee_ = new mmComboBoxPayee(this, mmID_PAYEE, wxDefaultSize, m_sched_xd.m_payee_id_n, true);
    mmToolTip(cbPayee_, payeeWithdrawalTip_);
    cbPayee_->SetMinSize(cbPayee_->GetSize());
    transPanelSizer->Add(payee_label, g_flagsH);
    transPanelSizer->Add(cbPayee_, g_flagsExpand);
    transPanelSizer->AddSpacer(1);

    // Category ---------------------------------------------

    wxStaticText* categ_label2 = new wxStaticText(this, ID_DIALOG_TRANS_CATEGLABEL, _t("Category"));
    categ_label2->SetFont(this->GetFont().Bold());
    cbCategory_ = new mmComboBoxCategory(this, mmID_CATEGORY, wxDefaultSize
                                            , m_sched_xd.m_category_id_n, true);
    cbCategory_->SetMinSize(cbCategory_->GetSize());
    bSplit_ = new wxBitmapButton(this, ID_DIALOG_TRANS_BUTTONSPLIT, mmBitmapBundle(png::NEW_TRX, mmBitmapButtonSize));
    mmToolTip(bSplit_, _t("Use split Categories"));

    transPanelSizer->Add(categ_label2, g_flagsH);
    transPanelSizer->Add(cbCategory_, g_flagsExpand);
    transPanelSizer->Add(bSplit_, g_flagsH);

    // Tags ---------------------------------------------

    wxStaticText* tag_label = new wxStaticText(this, wxID_ANY, _t("Tags"));
    tagTextCtrl_ = new mmTagTextCtrl(this);

    transPanelSizer->Add(tag_label, g_flagsH);
    transPanelSizer->Add(tagTextCtrl_, g_flagsExpand);
    transPanelSizer->AddSpacer(1);

    // Number ---------------------------------------------
    textNumber_ = new wxTextCtrl(this, ID_DIALOG_TRANS_TEXTNUMBER, "", wxDefaultPosition, wxDefaultSize);
    mmToolTip(textNumber_, _t("Specify any associated check number or transaction number"));

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _t("Number")), g_flagsH);
    transPanelSizer->Add(textNumber_, g_flagsExpand);
    transPanelSizer->AddSpacer(1);

    // Frequently Used Notes
    wxButton* bFrequentUsedNotes = new wxButton(this, ID_DIALOG_TRANS_BUTTON_FREQENTNOTES, "..."
        , wxDefaultPosition, bSplit_->GetSize());
    mmToolTip(bFrequentUsedNotes, _t("Select one of the frequently used notes"));
    bFrequentUsedNotes->Connect(ID_DIALOG_TRANS_BUTTON_FREQENTNOTES
        , wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(SchedDialog::OnFrequentUsedNotes), nullptr, this);

    // Colours
    bColours_ = new mmColorButton(this, wxID_LOWEST, bSplit_->GetSize());
    mmToolTip(bColours_, _t("User Colors"));

    // Attachments
    bAttachments_ = new wxBitmapButton(this, wxID_FILE, mmBitmapBundle(png::CLIP, mmBitmapButtonSize));
    mmToolTip(bAttachments_, _t("Organize attachments of this scheduled transaction"));

    // Now display the Frequntly Used Notes, Colour, Attachment buttons
    wxBoxSizer* notes_sizer = new wxBoxSizer(wxHORIZONTAL);
    transPanelSizer->Add(notes_sizer);
    notes_sizer->Add(new wxStaticText(this, wxID_STATIC, _t("Notes")), g_flagsH);
    notes_sizer->Add(bFrequentUsedNotes, g_flagsH);

    wxBoxSizer* RightAlign_sizer = new wxBoxSizer(wxHORIZONTAL);
    transPanelSizer->Add(RightAlign_sizer, wxSizerFlags(g_flagsH).Align(wxALIGN_RIGHT));
    RightAlign_sizer->Add(new wxStaticText(this, wxID_STATIC, _t("Color")), g_flagsH);
    RightAlign_sizer->Add(bColours_, wxSizerFlags());
    transPanelSizer->Add(bAttachments_, g_flagsH);

    // Notes
    textNotes_ = new wxTextCtrl(this, ID_DIALOG_TRANS_TEXTNOTES, ""
        , wxDefaultPosition, wxSize(-1, m_date_due->GetSize().GetHeight() * 5), wxTE_MULTILINE);
    mmToolTip(textNotes_, _t("Specify any text notes you want to add to this transaction."));
    transDetailsStaticBoxSizer->Add(textNotes_, wxSizerFlags(g_flagsExpand).Border(wxLEFT | wxRIGHT | wxBOTTOM, 10));

    /**********************************************************************************************
        Button Panel with OK and Cancel Buttons
    ***********************************************************************************************/
    wxPanel* buttonsPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    wxBoxSizer* button_sizer = new wxBoxSizer(wxHORIZONTAL);
    buttonsPanel->SetSizer(button_sizer);

    wxButton* button_ok = new wxButton(buttonsPanel, wxID_OK, _t("&OK "));

    m_button_cancel = new wxButton(buttonsPanel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));

    mainBoxSizerOuter->Add(buttonsPanel, wxSizerFlags(g_flagsV).Center().Border(wxALL, 0));
    wxBitmapButton* button_hide = new wxBitmapButton(buttonsPanel
        , ID_BTN_CUSTOMFIELDS, mmBitmapBundle(png::RIGHTARROW, mmBitmapButtonSize));
    mmToolTip(button_hide, _t("Show/Hide custom fields window"));
    if (m_custom_fields->GetCustomFieldsCount() == 0) {
        button_hide->Hide();
    }

    button_sizer->Add(button_ok, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));
    button_sizer->Add(m_button_cancel, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));
    button_sizer->Add(button_hide, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));

    // Custom fields -----------------------------------

    m_custom_fields->FillCustomFields(custom_fields_box_sizer);
    if (m_custom_fields->GetActiveCustomFieldsCount() > 0) {
        wxCommandEvent evt(wxEVT_BUTTON, ID_BTN_CUSTOMFIELDS);
        this->GetEventHandler()->AddPendingEvent(evt);
    }

    mainBoxSizerInner->Add(custom_fields_box_sizer, 0, wxEXPAND);
    this->SetSizerAndFit(mainBoxSizerOuter);
    min_size_ = GetMinSize();
    custom_fields_box_sizer->SetMinSize(transDetailsStaticBoxSizer->GetSize());
    m_custom_fields->SetMinSize(custom_fields_box_sizer->GetMinSize());
}

void SchedDialog::OnQuit(wxCloseEvent& WXUNUSED(event))
{
    if (m_enter_occur && m_sched_xd.m_id != 0) {
        mmAttachmentManage::DeleteAllAttachments(SchedModel::s_ref_type, m_sched_xd.m_id);
    }
    EndModal(wxID_CANCEL);
}

void SchedDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
#ifdef __WXMSW__
    wxWindow* w = FindFocus();
    if (w && w->GetId() != wxID_CANCEL && wxGetKeyState(WXK_ESCAPE))
        return m_button_cancel->SetFocus();

    if (w && w->GetId() != wxID_CANCEL) {
        return;
    }
#endif

    if (m_enter_occur && m_sched_xd.m_id != 0) {
        mmAttachmentManage::DeleteAllAttachments(SchedModel::s_ref_type, m_sched_xd.m_id);
    }
    EndModal(wxID_CANCEL);
}

void SchedDialog::OnPayee(wxCommandEvent& WXUNUSED(event))
{
    const PayeeData* payee_n = PayeeModel::instance().get_id_data_n(cbPayee_->mmGetId());
    if (!payee_n || !m_new_bill)
        return;

    // Only for new/duplicate transactions: if user want to autofill last category used for payee.
    // If this is a Split Transaction, ignore displaying last category for payee
    if (m_sched_xd.local_splits.empty()
        && (PrefModel::instance().getTransCategoryNone() == PrefModel::LASTUSED ||
            PrefModel::instance().getTransCategoryNone() == PrefModel::DEFAULT)
        && (!CategoryModel::instance().is_hidden(payee_n->m_category_id_n) && !CategoryModel::instance().is_hidden(payee_n->m_category_id_n)))
    {
        m_sched_xd.m_category_id_n = payee_n->m_category_id_n;

        cbCategory_->ChangeValue(CategoryModel::instance().full_name(m_sched_xd.m_category_id_n));
    }
}

void SchedDialog::SetAmountCurrencies(int64 accountID, int64 toAccountID)
{
    const AccountData* account_n = AccountModel::instance().get_id_data_n(accountID);
    if (account_n)
        textAmount_->SetCurrency(CurrencyModel::instance().get_id_data_n(account_n->m_currency_id));

    account_n = AccountModel::instance().get_id_data_n(toAccountID);
    if (account_n)
        toTextAmount_->SetCurrency(CurrencyModel::instance().get_id_data_n(account_n->m_currency_id));
}

void SchedDialog::OnCategs(wxCommandEvent& WXUNUSED(event))
{
    activateSplitTransactionsDlg();
}

void SchedDialog::OnTypeChanged(wxCommandEvent& WXUNUSED(event))
{
    updateControlsForTransType();
}

void SchedDialog::OnComboKey(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_RETURN) {
        auto id = event.GetId();
        switch (id) {
        case mmID_PAYEE: {
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
                    cbPayee_->SelectAll();
                    wxCommandEvent evt;
                    OnPayee(evt);
                }
                return;
            }
            break;
        }
        case mmID_CATEGORY: {
            auto category = cbCategory_->GetValue();
            if (category.empty()) {
                CategoryManager dlg(this, true, -1);
                dlg.ShowModal();
                if (dlg.getRefreshRequested())
                    cbCategory_->mmDoReInitialize();
                category = CategoryModel::instance().full_name(dlg.getCategId());
                cbCategory_->ChangeValue(category);
                cbCategory_->SelectAll();
                return;
            }
            break;
        }
        default:
            break;
        }
    }

    // The first time the ALT key is pressed accelerator hints are drawn, but custom painting on the tags button
    // is not applied. We need to refresh the tag ctrl to redraw the drop button with the correct image.
    if (event.AltDown() && !altRefreshDone) {
        tagTextCtrl_->Refresh();
        altRefreshDone = true;
    }

    event.Skip();
}

void SchedDialog::OnAttachments(wxCommandEvent& WXUNUSED(event))
{
    AttachmentDialog dlg(this, SchedModel::s_ref_type, m_sched_xd.m_id);
    dlg.ShowModal();
}

void SchedDialog::updateControlsForTransType()
{
    wxStaticText* accountLabel = static_cast<wxStaticText*>(FindWindow(ID_DIALOG_TRANS_STATIC_ACCOUNT));
    wxStaticText* stp = static_cast<wxStaticText*>(FindWindow(ID_DIALOG_TRANS_STATIC_PAYEE));

    m_transfer = false;
    switch (m_choice_transaction_type->GetSelection())
    {
    case TrxModel::TYPE_ID_TRANSFER: {
        m_transfer = true;
        mmToolTip(textAmount_, amountTransferTip_);
        accountLabel->SetLabelText(_t("From"));

        cbToAccount_->mmSetId(m_sched_xd.m_to_account_id_n);
        m_sched_xd.m_payee_id_n = -1;
        break;
    }
    case TrxModel::TYPE_ID_WITHDRAWAL: {
        mmToolTip(textAmount_, amountNormalTip_);
        accountLabel->SetLabelText(_t("Account"));
        stp->SetLabelText(_t("Payee"));
        mmToolTip(cbPayee_, payeeWithdrawalTip_);

        cbPayee_->mmSetId(m_sched_xd.m_payee_id_n);
        m_sched_xd.m_to_account_id_n = -1;
        wxCommandEvent evt;
        OnPayee(evt);
        break;
    }
    case TrxModel::TYPE_ID_DEPOSIT: {
        mmToolTip(textAmount_, amountNormalTip_);
        accountLabel->SetLabelText(_t("Account"));
        stp->SetLabelText(_t("From"));
        mmToolTip(cbPayee_, payeeDepositTip_);

        cbPayee_->mmSetId(m_sched_xd.m_payee_id_n);
        m_sched_xd.m_to_account_id_n = -1;
        wxCommandEvent evt;
        OnPayee(evt);
        break;
    }
    }

    SetTransferControls(m_transfer);

    if (cAdvanced_->IsChecked()) {
        SetAdvancedTransferControls(true);
    }
}

void SchedDialog::OnFrequentUsedNotes(wxCommandEvent& WXUNUSED(event))
{
    wxMenu menu;
    int id = wxID_HIGHEST;
    for (const auto& entry : frequentNotes_) {
        const wxString& label = entry.Mid(0, 30) + (entry.size() > 30 ? "..." : "");
        menu.Append(++id, label);

    }
    menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &SchedDialog::OnNoteSelected, this);
    if (!frequentNotes_.empty())
        PopupMenu(&menu);
}

void SchedDialog::OnNoteSelected(wxCommandEvent& event)
{
    int i = event.GetId() - wxID_HIGHEST;
    if (i > 0 && static_cast<size_t>(i) <= frequentNotes_.size()) {
        if (!textNotes_->GetValue().EndsWith("\n") && !textNotes_->GetValue().empty())
            textNotes_->AppendText("\n");
        textNotes_->AppendText(frequentNotes_[i - 1]);
    }
}

void SchedDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    // Ideally 'paid date' should be on or before the 'due date'
    if (m_date_paid->GetValue().GetDateOnly() > m_date_due->GetValue())
        if (wxMessageBox(_t("The payment date is after the due date. Is this intended?"),
            _t("Looks like a late payment"),
            wxYES_NO | wxNO_DEFAULT | wxICON_WARNING) != wxYES)
            return;

    if (!cbAccount_->mmIsValid()) {
        return mmErrorDialogs::InvalidAccount(cbAccount_, m_transfer, mmErrorDialogs::MESSAGE_DROPDOWN_BOX);
    }
    m_sched_xd.m_account_id = cbAccount_->mmGetId();
    const AccountData* acc = AccountModel::instance().get_id_data_n(m_sched_xd.m_account_id);

    if (!textAmount_->checkValue(m_sched_xd.m_amount)) return;

    m_sched_xd.m_to_amount = m_sched_xd.m_amount;
    if (m_transfer) {
        if (!cbToAccount_->mmIsValid()) {
            return mmErrorDialogs::InvalidAccount(cbToAccount_, m_transfer, mmErrorDialogs::MESSAGE_DROPDOWN_BOX);
        }
        m_sched_xd.m_to_account_id_n = cbToAccount_->mmGetId();

        if (m_sched_xd.m_to_account_id_n == m_sched_xd.m_account_id) {
            return mmErrorDialogs::InvalidAccount(cbPayee_, true);
        }

        if (m_advanced && !toTextAmount_->checkValue(m_sched_xd.m_to_amount)) return;
    }
    else {
        wxString payee_name = cbPayee_->GetValue();
        if (payee_name.IsEmpty()) {
            mmErrorDialogs::InvalidPayee(cbPayee_);
            return;
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
                payee_n = PayeeModel::instance().get_id_data_n(new_payee_d.id());
                mmWebApp::MMEX_WebApp_UpdatePayee();
            }
            else
                return;
        }
        m_sched_xd.m_payee_id_n = payee_n->id();
    }

    if (m_sched_xd.local_splits.empty()) {
        if (!cbCategory_->mmIsValid()) {
            return mmErrorDialogs::ToolTip4Object(cbCategory_, _t("Invalid value"), _t("Category"), wxICON_ERROR);
        }
        m_sched_xd.m_category_id_n = cbCategory_->mmGetCategoryId();
    }

    if (!tagTextCtrl_->IsValid()) {
        return mmErrorDialogs::ToolTip4Object(tagTextCtrl_, _t("Invalid value"), _t("Tags"), wxICON_ERROR);
    }

    if (!m_custom_fields->ValidateCustomValues(-m_sched_xd.m_id))
        return;

    if (!m_advanced || m_sched_xd.m_to_amount < 0) {
        // if we are adding a new record and the user did not touch advanced dialog
        // we are going to use the transfer amount by calculating conversion rate.
        // subsequent edits will not allow automatic update of the amount
        if (m_new_bill) {
            if (m_sched_xd.m_to_account_id_n != -1) {
                const AccountData* to_account = AccountModel::instance().get_id_data_n(m_sched_xd.m_to_account_id_n);

                const CurrencyData* from_currency = AccountModel::instance().get_data_currency_p(*acc);
                const CurrencyData* to_currency = AccountModel::instance().get_data_currency_p(*to_account);

                double rateFrom = CurrencyHistoryModel::getDayRate(from_currency->m_id, m_sched_xd.TRANSDATE);
                double rateTo = CurrencyHistoryModel::getDayRate(to_currency->m_id, m_sched_xd.TRANSDATE);

                double convToBaseFrom = rateFrom * m_sched_xd.m_amount;
                m_sched_xd.m_to_amount = convToBaseFrom / rateTo;
            }
            else {
                m_sched_xd.m_to_amount = m_sched_xd.m_amount;
            }
        }
    }

    SchedModel::RepeatNum rn;
    rn.exec = 
        autoExecuteSilent_  ? SchedModel::REPEAT_EXEC_SILENT :
        autoExecuteUserAck_ ? SchedModel::REPEAT_EXEC_MANUAL :
                              SchedModel::REPEAT_EXEC_NONE;
    rn.freq = static_cast<SchedModel::REPEAT_FREQ>(getRepeatType());
    rn.num = SchedModel::REPEAT_NUM_INFINITY;
    rn.x = 1;
    const wxString& numRepeatStr = textNumRepeats_->GetValue();
    long cnt = 0;
    if (!numRepeatStr.empty() && numRepeatStr.ToLong(&cnt) && cnt > 0) {
        wxASSERT(cnt <= std::numeric_limits<int>::max());
        if (rn.freq >= SchedModel::REPEAT_FREQ_IN_X_DAYS &&
            rn.freq <= SchedModel::REPEAT_FREQ_EVERY_X_MONTHS
        )
            rn.x = cnt;
        else
            rn.num = cnt;
    }
    SchedModel::encode_repeat_num(m_sched_xd, rn);

    m_sched_xd.NEXTOCCURRENCEDATE = m_date_due->GetValue().FormatISODate();
    m_sched_xd.TRANSDATE = m_date_paid->GetValue().FormatISOCombined();

    wxStringClientData* status_obj = static_cast<wxStringClientData *>(
        m_choice_status->GetClientObject(m_choice_status->GetSelection())
    );
    if (status_obj) {
        m_sched_xd.STATUS = TrxModel::status_key(status_obj->GetData());
    }

    m_sched_xd.m_number = textNumber_->GetValue();
    m_sched_xd.m_notes = textNotes_->GetValue();

    int color_id = bColours_->GetColorId();
    if (color_id > 0 && color_id < 8)
        m_sched_xd.m_color = color_id;
    else
        m_sched_xd.m_color = -1;

    const AccountData* account = AccountModel::instance().get_id_data_n(m_sched_xd.m_account_id);
    const AccountData* toAccount = AccountModel::instance().get_id_data_n(m_sched_xd.m_to_account_id_n);
    if (mmDate(m_sched_xd.TRANSDATE) < account->m_open_date)
        return mmErrorDialogs::ToolTip4Object(
            cbAccount_,
            _t("The opening date for the account is later than the date of this transaction"),
            _t("Invalid Date")
        );

    if (toAccount && (mmDate(m_sched_xd.TRANSDATE) < toAccount->m_open_date))
        return mmErrorDialogs::ToolTip4Object(
            cbToAccount_,
            _t("The opening date for the account is later than the date of this transaction"),
            _t("Invalid Date")
        );

    if (!m_enter_occur) {
        SchedData sched_d = (!m_new_bill && !m_dup_bill)
            ? *(SchedModel::instance().get_id_data_n(m_sched_xd.m_id))
            : SchedData();
        sched_d.m_account_id       = m_sched_xd.m_account_id;
        sched_d.m_to_account_id_n  = m_sched_xd.m_to_account_id_n;
        sched_d.m_payee_id_n       = m_sched_xd.m_payee_id_n;
        sched_d.TRANSCODE          = TrxModel::type_name(m_choice_transaction_type->GetSelection());
        sched_d.m_amount           = m_sched_xd.m_amount;
        sched_d.STATUS             = m_sched_xd.STATUS;
        sched_d.m_number           = m_sched_xd.m_number;
        sched_d.m_notes            = m_sched_xd.m_notes;
        sched_d.m_category_id_n    = m_sched_xd.m_category_id_n;
        sched_d.TRANSDATE          = m_sched_xd.TRANSDATE;
        sched_d.m_to_amount        = m_sched_xd.m_to_amount;
        sched_d.REPEATS            = m_sched_xd.REPEATS;
        sched_d.NEXTOCCURRENCEDATE = m_sched_xd.NEXTOCCURRENCEDATE;
        sched_d.NUMOCCURRENCES     = m_sched_xd.NUMOCCURRENCES;
        sched_d.m_followup_id      = m_sched_xd.m_followup_id;
        sched_d.m_color            = m_sched_xd.m_color;
        SchedModel::instance().save_data_n(sched_d);
        m_trans_id = sched_d.id();

        SchedSplitModel::DataA new_qp_a;
        for (const auto& split_d : m_sched_xd.local_splits) {
            SchedSplitData new_qp_d = SchedSplitData();
            new_qp_d.m_category_id = split_d.CATEGID;
            new_qp_d.m_amount      = split_d.SPLITTRANSAMOUNT;
            new_qp_d.m_notes       = split_d.NOTES;
            new_qp_a.push_back(new_qp_d);
        }
        SchedSplitModel::instance().update(m_trans_id, new_qp_a);

        // Save split tags
        for (size_t i = 0; i < m_sched_xd.local_splits.size(); i++) {
            TagLinkModel::DataA new_qp_gl_a;
            for (const auto& tag_id : m_sched_xd.local_splits.at(i).TAGS) {
                TagLinkData new_gl_d = TagLinkData();
                new_gl_d.m_tag_id   = tag_id;
                new_gl_d.m_ref_type = SchedSplitModel::s_ref_type;
                new_gl_d.m_ref_id   = new_qp_a.at(i).m_id;
                new_qp_gl_a.push_back(new_gl_d);
            }
            TagLinkModel::instance().update(
                SchedSplitModel::s_ref_type, new_qp_a.at(i).m_id,
                new_qp_gl_a
            );
        }

        // FIXME: ref_id 0 does not exists in database
        mmAttachmentManage::RelocateAllAttachments(
            SchedModel::s_ref_type, 0,
            SchedModel::s_ref_type, m_trans_id
        );

        // Save base transaction tags
        TagLinkModel::DataA new_gl_a;
        for (const auto& tag_id : tagTextCtrl_->GetTagIDs()) {
            TagLinkData new_gl_d = TagLinkData();
            new_gl_d.m_tag_id   = tag_id;
            new_gl_d.m_ref_type = SchedModel::s_ref_type;
            new_gl_d.m_ref_id   = m_trans_id;
            new_gl_a.push_back(new_gl_d);
        }
        TagLinkModel::instance().update(
            SchedModel::s_ref_type, m_trans_id,
            new_gl_a
        );

        //Custom Data
        m_custom_fields->SaveCustomValues(SchedModel::s_ref_type, m_trans_id);
    }
    else {
        // the following condition is always true, since old inactive entries of type
        // REPEAT_FREQ_IN_X_*, REPEAT_EVERY_X_* have been converted to entries of type REPEAT_FREQ_ONCE
        if (rn.freq < SchedModel::REPEAT_FREQ_IN_X_DAYS ||
            rn.freq > SchedModel::REPEAT_FREQ_EVERY_X_MONTHS ||
            rn.x > 0
        ) {
            // FIXME: use m_sched_xd directly
            SchedData sched_d;
            sched_d.m_account_id = m_sched_xd.m_account_id;
            sched_d.TRANSCODE    = m_sched_xd.TRANSCODE;
            sched_d.m_amount     = m_sched_xd.m_amount;
            if (!SchedModel::instance().AllowTransaction(sched_d))
                return;

            TrxData new_trx_d = TrxData();
            new_trx_d.m_account_id      = m_sched_xd.m_account_id;
            new_trx_d.m_to_account_id_n = m_sched_xd.m_to_account_id_n;
            new_trx_d.m_payee_id_n      = m_sched_xd.m_payee_id_n;
            new_trx_d.TRANSCODE         = TrxModel::type_name(m_choice_transaction_type->GetSelection());
            new_trx_d.m_amount          = m_sched_xd.m_amount;
            new_trx_d.STATUS            = m_sched_xd.STATUS;
            new_trx_d.m_number          = m_sched_xd.m_number;
            new_trx_d.m_notes           = m_sched_xd.m_notes;
            new_trx_d.m_category_id_n   = m_sched_xd.m_category_id_n;
            new_trx_d.TRANSDATE         = m_sched_xd.TRANSDATE;
            new_trx_d.m_to_amount       = m_sched_xd.m_to_amount;
            new_trx_d.m_followup_id     = m_sched_xd.m_followup_id;
            new_trx_d.m_color           = m_sched_xd.m_color;
            TrxModel::instance().save_trx_n(new_trx_d);
            int64 new_trx_id = new_trx_d.id();

            TrxSplitModel::DataA new_tp_a;
            for (auto& split_d : m_sched_xd.local_splits) {
                TrxSplitData new_tp_d = TrxSplitData();
                new_tp_d.m_trx_id      = new_trx_id;
                new_tp_d.m_category_id = split_d.CATEGID;
                new_tp_d.m_amount      = split_d.SPLITTRANSAMOUNT;
                new_tp_d.m_notes       = split_d.NOTES;
                new_tp_a.push_back(new_tp_d);
            }
            TrxSplitModel::instance().update(new_tp_a, new_trx_id);

            // Save split tags
            for (size_t i = 0; i < m_sched_xd.local_splits.size(); i++) {
                TagLinkModel::DataA new_tp_gl_a;
                for (const auto& tag_id : m_sched_xd.local_splits.at(i).TAGS) {
                    TagLinkData new_gl_d = TagLinkData();
                    new_gl_d.m_tag_id   = tag_id;
                    new_gl_d.m_ref_type = TrxSplitModel::s_ref_type;
                    new_gl_d.m_ref_id   = new_tp_a.at(i).m_id;
                    new_tp_gl_a.push_back(new_gl_d);
                }
                TagLinkModel::instance().update(
                    TrxSplitModel::s_ref_type, new_tp_a.at(i).m_id,
                    new_tp_gl_a
                );
            }

            // Custom Data
            m_custom_fields->SaveCustomValues(TrxModel::s_ref_type, new_trx_id);

            mmAttachmentManage::RelocateAllAttachments(
                SchedModel::s_ref_type, m_sched_xd.m_id,
                TrxModel::s_ref_type, new_trx_id
            );

            // Save base transaction tags
            TagLinkModel::DataA new_gl_a;
            for (const auto& tag_id : tagTextCtrl_->GetTagIDs()) {
                TagLinkData new_gl_d = TagLinkData();
                new_gl_d.m_tag_id   = tag_id;
                new_gl_d.m_ref_type = TrxModel::s_ref_type;
                new_gl_d.m_ref_id   = new_trx_id;
                new_gl_a.push_back(new_gl_d);
            }
            TagLinkModel::instance().update(
                TrxModel::s_ref_type, new_trx_id,
                new_gl_a
            );
        }
        SchedModel::instance().completeBDInSeries(m_sched_xd.m_id);
    }

    EndModal(wxID_OK);
}

void SchedDialog::SetSplitControls(bool split)
{
    textAmount_->Enable(!split);
    bCalc_->Enable(!split);
    if (split) {
        m_sched_xd.m_amount = TrxSplitModel::get_total(m_sched_xd.local_splits);
        m_sched_xd.m_category_id_n = -1;
    }
    else {
        m_sched_xd.local_splits.clear();
    }
    setCategoryLabel();
}

void SchedDialog::OnAutoExecutionUserAckChecked(wxCommandEvent& WXUNUSED(event))
{
    autoExecuteUserAck_ = !autoExecuteUserAck_;
    if (autoExecuteUserAck_) {
        itemCheckBoxAutoExeSilent_->SetValue(false);
        itemCheckBoxAutoExeSilent_->Enable(false);
        autoExecuteSilent_ = false;
    }
    else {
        itemCheckBoxAutoExeSilent_->Enable(true);
    }
}

void SchedDialog::OnAutoExecutionSilentChecked(wxCommandEvent& WXUNUSED(event))
{
    autoExecuteSilent_ = !autoExecuteSilent_;
    if (autoExecuteSilent_) {
        itemCheckBoxAutoExeUserAck_->SetValue(false);
        itemCheckBoxAutoExeUserAck_->Enable(false);
        autoExecuteUserAck_ = false;
    }
    else {
        itemCheckBoxAutoExeUserAck_->Enable(true);
    }
}

void SchedDialog::OnAdvanceChecked(wxCommandEvent& WXUNUSED(event))
{
    SetAdvancedTransferControls(cAdvanced_->IsChecked());
}

void SchedDialog::SetTransferControls(bool transfers)
{
    wxStaticText* stp = static_cast<wxStaticText*>(FindWindow(ID_DIALOG_TRANS_STATIC_PAYEE));
    wxStaticText* stta = static_cast<wxStaticText*>(FindWindow(ID_DIALOG_TRANS_STATIC_TOACCOUNT));

    cAdvanced_->Enable(transfers);
    if (transfers) {
        SetSplitControls();
    }
    else {
        SetAdvancedTransferControls();
        toTextAmount_->ChangeValue("");
        cAdvanced_->SetValue(false);
    }

    bSplit_->Enable(!transfers);
    cbToAccount_->Show(transfers);
    cbPayee_->Show(!transfers);
    stta->Show(m_transfer);
    stp->Show(!m_transfer);
    Layout();
}

void SchedDialog::SetAdvancedTransferControls(bool advanced)
{
    m_advanced = advanced;
    toTextAmount_->Enable(m_advanced);
    mmToolTip(textAmount_, m_advanced
        ? amountTransferTip_
        : _t("Specify the transfer amount in the From Account")
    );
    if (m_advanced)
        toTextAmount_->SetValue(m_sched_xd.m_to_amount);
    else
        toTextAmount_->ChangeValue("");
}

void SchedDialog::setRepeatDetails()
{
    staticTextRepeats_->SetLabelText(_t("Repeats"));

    int repeats = getRepeatType();
    if (repeats == SchedModel::REPEAT_FREQ_IN_X_DAYS ||
        repeats == SchedModel::REPEAT_FREQ_EVERY_X_DAYS
    ) {
        staticTimesRepeat_->SetLabelText(_t("Period: Days"));
        const auto toolTipsStr = _t("Specify period in Days.");
        mmToolTip(textNumRepeats_, toolTipsStr);
    }
    else if (repeats == SchedModel::REPEAT_FREQ_IN_X_MONTHS ||
        repeats == SchedModel::REPEAT_FREQ_EVERY_X_MONTHS
    ) {
        staticTimesRepeat_->SetLabelText(_t("Period: Months"));
        const auto toolTipsStr = _t("Specify period in Months.");
        mmToolTip(textNumRepeats_, toolTipsStr);
    }
    else if (repeats == SchedModel::REPEAT_FREQ_ONCE) {
        staticTimesRepeat_->SetLabelText(_t("Payments Left"));
        const auto toolTipsStr = _t("Ignored (leave blank).");
        mmToolTip(textNumRepeats_, toolTipsStr);
    }
    else {
        staticTimesRepeat_->SetLabelText(_t("Payments Left"));
        const auto toolTipsStr = _t("Specify the number of payments to be made.\n"
            "Leave blank if the payments continue forever.");
        mmToolTip(textNumRepeats_, toolTipsStr);
    }
}

void SchedDialog::OnRepeatTypeChanged(wxCommandEvent& WXUNUSED(event))
{
    setRepeatDetails();
}

int SchedDialog::getRepeatType()
{
    int repeatIndex = m_choice_repeat->GetSelection();
    return repeatIndex >= 0 ? BILLSDEPOSITS_REPEATS.at(repeatIndex).first : -1;
}

void SchedDialog::setRepeatType(int repeatType)
{
    if (repeatType < 0) {
        wxFAIL;
        return;
    }

    // fast path
    int repeatIndex = repeatType;
    if (BILLSDEPOSITS_REPEATS.at(repeatIndex).first != repeatType) {
        // slow path: BILLSDEPOSITS_REPEATS is not sorted by REPEAT_FREQ
        // cache the mapping from type to index
        static std::vector<int> index;
        if (index.size() == 0) {
            wxLogDebug("SchedDialog::setRepeatType : cache index");
            index.resize(BILLSDEPOSITS_REPEATS.size(), -1);
            for (size_t i = 0; i < BILLSDEPOSITS_REPEATS.size(); i++) {
                unsigned int j = BILLSDEPOSITS_REPEATS.at(i).first;
                if (j < BILLSDEPOSITS_REPEATS.size() && index.at(j) == -1)
                    index.at(j) = i;
                else {
                    wxFAIL;
                }
            }
        }

        repeatIndex = index.at(repeatType);
        if (repeatIndex == -1) {
            wxFAIL;
            repeatIndex = 0;
        }
    }

    m_choice_repeat->SetSelection(repeatIndex);
}

void SchedDialog::OnsetPrevOrNextRepeatDate(wxCommandEvent& event)
{
    SchedModel::RepeatNum rn;
    rn.freq = static_cast<SchedModel::REPEAT_FREQ>(getRepeatType());
    rn.x = 1;
    wxString valueStr = textNumRepeats_->GetValue();
    bool goPrev = (event.GetId() == ID_DIALOG_TRANS_BUTTONTRANSNUMPREV);

    switch (rn.freq)
    {
    case SchedModel::REPEAT_FREQ_IN_X_DAYS:
        wxFALLTHROUGH;
    case SchedModel::REPEAT_FREQ_IN_X_MONTHS:
        wxFALLTHROUGH;
    case SchedModel::REPEAT_FREQ_EVERY_X_DAYS:
        wxFALLTHROUGH;
    case SchedModel::REPEAT_FREQ_EVERY_X_MONTHS:
        rn.x = wxAtoi(valueStr);
        if (!valueStr.IsNumber() || !rn.x) {
            mmErrorDialogs::ToolTip4Object(textNumRepeats_, _t("Invalid value"), _t("Error"));
            break;
        }
        wxFALLTHROUGH;
    default:
        m_date_paid->SetValue(SchedModel::nextOccurDate(
            m_date_paid->GetValue(), rn, goPrev)
        );
        m_date_due->SetValue(SchedModel::nextOccurDate(
            m_date_due->GetValue(), rn, goPrev)
        );
    }
}

void SchedDialog::activateSplitTransactionsDlg()
{
    if (m_sched_xd.local_splits.empty()) {
        if (!textAmount_->GetDouble(m_sched_xd.m_amount)) {
            m_sched_xd.m_amount = 0;
        }
        Split split_d;
        split_d.SPLITTRANSAMOUNT = m_sched_xd.m_amount;
        split_d.CATEGID          = m_sched_xd.m_category_id_n;
        split_d.NOTES            = m_sched_xd.m_notes;
        m_sched_xd.local_splits.push_back(split_d);
    }

    SplitDialog dlg(this, m_sched_xd.local_splits, m_sched_xd.m_account_id);
    if (dlg.ShowModal() == wxID_OK) {
        m_sched_xd.local_splits    = dlg.mmGetResult();
        m_sched_xd.m_amount        = TrxSplitModel::get_total(m_sched_xd.local_splits);
        m_sched_xd.m_category_id_n = -1;
        if (m_choice_transaction_type->GetSelection() == TrxModel::TYPE_ID_TRANSFER && m_sched_xd.m_amount < 0) {
            m_sched_xd.m_amount = -m_sched_xd.m_amount;
        }
        textAmount_->SetValue(m_sched_xd.m_amount);
    }

    if (m_sched_xd.local_splits.size() == 1) {
        m_sched_xd.m_category_id_n = m_sched_xd.local_splits[0].CATEGID;
        textNotes_->SetValue(m_sched_xd.local_splits[0].NOTES);
        m_sched_xd.local_splits.clear();
    }

    setCategoryLabel();
}

void SchedDialog::setTooltips()
{
    if (!this->m_sched_xd.local_splits.empty()) {
        const CurrencyData* currency = CurrencyModel::GetBaseCurrency();
        const AccountData* account = AccountModel::instance().get_id_data_n(m_sched_xd.m_account_id);
        if (account) {
            currency = AccountModel::instance().get_data_currency_p(*account);
        }

        bSplit_->SetToolTip(TrxSplitModel::get_tooltip(m_sched_xd.local_splits, currency));
    }
    else
        mmToolTip(bSplit_, _t("Use split Categories"));
}

void SchedDialog::setCategoryLabel()
{
    bool has_split = !m_sched_xd.local_splits.empty();

    bSplit_->UnsetToolTip();
    if (has_split) {
        cbCategory_->SetLabelText(_t("Split Transaction"));
        textAmount_->SetValue(TrxSplitModel::get_total(m_sched_xd.local_splits));
        m_sched_xd.m_category_id_n = -1;
    }
    else if (m_transfer && m_new_bill
        && PrefModel::instance().getTransCategoryTransferNone() == PrefModel::LASTUSED
    ) {
        TrxModel::DataA transactions = TrxModel::instance().find(
            TrxModel::TRANSCODE(OP_EQ, TrxModel::TYPE_ID_TRANSFER),
            TrxModel::TRANSDATE(OP_LE, mmDate::today())
        );

        if (!transactions.empty()) {
            const int64 cat = transactions.back().m_category_id_n;
            cbCategory_->ChangeValue(CategoryModel::instance().full_name(cat));
        }
    }
    else {
        const auto fullCategoryName = CategoryModel::instance().full_name(m_sched_xd.m_category_id_n);
        cbCategory_->ChangeValue(fullCategoryName);
    }

    setTooltips();

    bool is_split = !m_sched_xd.local_splits.empty();
    textAmount_->Enable(!is_split);
    bCalc_->Enable(!is_split);
    wxBitmapButton* bSplit = static_cast<wxBitmapButton*>(FindWindow(ID_DIALOG_TRANS_BUTTONSPLIT));
    bSplit->Enable(!m_transfer);
    cbCategory_->Enable(!is_split);
    Layout();
}

void SchedDialog::OnMoreFields(wxCommandEvent& WXUNUSED(event))
{
    wxBitmapButton* button = static_cast<wxBitmapButton*>(FindWindow(ID_BTN_CUSTOMFIELDS));

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

void SchedDialog::OnAccountUpdated(wxCommandEvent& WXUNUSED(event))
{
    int64 acc_id = cbAccount_->mmGetId();
    const AccountData* account_n = AccountModel::instance().get_id_data_n(acc_id);
    if (account_n) {
        SetAmountCurrencies(acc_id, -1);
        if (textAmount_->Calculate()) {
            textAmount_->GetDouble(m_sched_xd.m_amount);
        }

        m_sched_xd.m_account_id = account_n->m_id;
    }
}

void SchedDialog::OnFocusChange(wxChildFocusEvent& event)
{
    switch (object_in_focus_)
    {
    case mmID_ACCOUNTNAME:
        cbAccount_->ChangeValue(cbAccount_->GetValue());
        if (cbAccount_->mmIsValid()) {
            m_sched_xd.m_account_id = cbAccount_->mmGetId();
            SetAmountCurrencies(m_sched_xd.m_account_id, -1);
        }
        break;
    case mmID_TOACCOUNTNAME:
        cbToAccount_->ChangeValue(cbToAccount_->GetValue());
        if (cbToAccount_->mmIsValid()) {
            m_sched_xd.m_to_account_id_n = cbToAccount_->mmGetId();
            SetAmountCurrencies(-1, m_sched_xd.m_to_account_id_n);
        }
        break;
    case mmID_PAYEE:
        cbPayee_->ChangeValue(cbPayee_->GetValue());
        break;
    case mmID_CATEGORY:
        cbCategory_->ChangeValue(cbCategory_->GetValue());
        if (cbCategory_->mmIsValid()) {
            m_sched_xd.m_category_id_n = cbCategory_->mmGetCategoryId();
        }
        break;
    case ID_DIALOG_TRANS_TEXTAMOUNT:
        calcTarget_ = textAmount_;
        break;
    case ID_DIALOG_TRANS_TOTEXTAMOUNT:
        calcTarget_ = toTextAmount_;
        break;

    }

    wxWindow* w = event.GetWindow();
    if (w) {
        object_in_focus_ = w->GetId();
    }

    if (textAmount_->Calculate()) {
        textAmount_->GetDouble(m_sched_xd.m_amount);
        textAmount_->SelectAll();
    }
    if (m_advanced && toTextAmount_->Calculate()) {
        toTextAmount_->GetDouble(m_sched_xd.m_to_amount);
        toTextAmount_->SelectAll();
    }
}

void SchedDialog::OnCalculator(wxCommandEvent& WXUNUSED(event))
{
    calcPopup_->SetTarget(calcTarget_);
    calcPopup_->Popup();
}
