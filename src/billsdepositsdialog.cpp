/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2016 Stefano Giorgio
 Copyright (C) 2016 - 2022 Nikolay Akimov
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

#include <wx/calctrl.h>
#include "billsdepositsdialog.h"
#include "attachmentdialog.h"
#include "categdialog.h"
#include "constants.h"
#include "images_list.h"
#include "option.h"
#include "mmSimpleDialogs.h"
#include "paths.h"
#include "payeedialog.h"
#include "util.h"
#include "validators.h"
#include "webapp.h"
#include "mmTextCtrl.h"
#include "splittransactionsdialog.h"
#include "model/Model_Payee.h"
#include "model/Model_Attachment.h"
#include "model/Model_CurrencyHistory.h"
#include <wx/valnum.h>

// the order in gui may be different than the database encoding order.
const std::vector<std::pair<int, wxString> > mmBDDialog::BILLSDEPOSITS_REPEATS =
{
    { Model_Billsdeposits::REPEAT_ONCE, wxTRANSLATE("Once")},
    { Model_Billsdeposits::REPEAT_WEEKLY, wxTRANSLATE("Weekly")},
    { Model_Billsdeposits::REPEAT_BI_WEEKLY, wxTRANSLATE("Fortnightly")},
    { Model_Billsdeposits::REPEAT_MONTHLY, wxTRANSLATE("Monthly")},
    { Model_Billsdeposits::REPEAT_BI_MONTHLY, wxTRANSLATE("Every 2 Months")},
    { Model_Billsdeposits::REPEAT_QUARTERLY, wxTRANSLATE("Quarterly")},
    { Model_Billsdeposits::REPEAT_HALF_YEARLY, wxTRANSLATE("Half-Yearly")},
    { Model_Billsdeposits::REPEAT_YEARLY, wxTRANSLATE("Yearly")},
    { Model_Billsdeposits::REPEAT_FOUR_MONTHLY, wxTRANSLATE("Four Months")},
    { Model_Billsdeposits::REPEAT_FOUR_WEEKLY, wxTRANSLATE("Four Weeks")},
    { Model_Billsdeposits::REPEAT_DAILY, wxTRANSLATE("Daily")},
    { Model_Billsdeposits::REPEAT_IN_X_DAYS, wxTRANSLATE("In (n) Days")},
    { Model_Billsdeposits::REPEAT_IN_X_MONTHS, wxTRANSLATE("In (n) Months")},
    { Model_Billsdeposits::REPEAT_EVERY_X_DAYS, wxTRANSLATE("Every (n) Days")},
    { Model_Billsdeposits::REPEAT_EVERY_X_MONTHS, wxTRANSLATE("Every (n) Months")},
    { Model_Billsdeposits::REPEAT_MONTHLY_LAST_DAY, wxTRANSLATE("Monthly (last day)")},
    { Model_Billsdeposits::REPEAT_MONTHLY_LAST_BUSINESS_DAY, wxTRANSLATE("Monthly (last business day)")}
};

// Used to determine if we need to refresh the tag text ctrl after
// accelerator hints are shown which only occurs once.
static bool altRefreshDone;

wxIMPLEMENT_DYNAMIC_CLASS(mmBDDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmBDDialog, wxDialog)
EVT_CHAR_HOOK(mmBDDialog::OnComboKey)
EVT_CHILD_FOCUS(mmBDDialog::OnFocusChange)
EVT_BUTTON(wxID_OK, mmBDDialog::OnOk)
EVT_BUTTON(wxID_CANCEL, mmBDDialog::OnCancel)
EVT_BUTTON(mmID_CATEGORY, mmBDDialog::OnCategs)
EVT_BUTTON(ID_DIALOG_TRANS_BUTTONSPLIT, mmBDDialog::OnCategs)
EVT_TEXT(mmID_PAYEE, mmBDDialog::OnPayee)
EVT_BUTTON(wxID_FILE, mmBDDialog::OnAttachments)
EVT_BUTTON(ID_BTN_CUSTOMFIELDS, mmBDDialog::OnMoreFields)
EVT_CHOICE(wxID_VIEW_DETAILS, mmBDDialog::OnTypeChanged)
EVT_CHECKBOX(ID_DIALOG_TRANS_ADVANCED_CHECKBOX, mmBDDialog::OnAdvanceChecked)
EVT_CHECKBOX(ID_DIALOG_BD_CHECKBOX_AUTO_EXECUTE_USERACK, mmBDDialog::OnAutoExecutionUserAckChecked)
EVT_CHECKBOX(ID_DIALOG_BD_CHECKBOX_AUTO_EXECUTE_SILENT, mmBDDialog::OnAutoExecutionSilentChecked)
EVT_CHOICE(ID_DIALOG_BD_COMBOBOX_REPEATS, mmBDDialog::OnRepeatTypeChanged)
EVT_BUTTON(ID_DIALOG_TRANS_BUTTONTRANSNUMPREV, mmBDDialog::OnsetPrevOrNextRepeatDate)
EVT_BUTTON(ID_DIALOG_TRANS_BUTTONTRANSNUM, mmBDDialog::OnsetPrevOrNextRepeatDate)
EVT_TEXT(mmID_ACCOUNTNAME, mmBDDialog::OnAccountUpdated)
EVT_CLOSE(mmBDDialog::OnQuit)
wxEND_EVENT_TABLE()


mmBDDialog::mmBDDialog()
{
}

mmBDDialog::~mmBDDialog()
{
    wxSize size = GetSize();
    if (m_custom_fields->IsCustomPanelShown())
        size = wxSize(GetSize().GetWidth() - m_custom_fields->GetMinWidth(), GetSize().GetHeight());
    Model_Infotable::instance().Set("RECURRINGTRANS_DIALOG_SIZE", size);
}

mmBDDialog::mmBDDialog(wxWindow* parent, int64 bdID, bool duplicate, bool enterOccur)
    : m_dup_bill(duplicate)
    , m_enter_occur(enterOccur)
{
    const Model_Billsdeposits::Data* bill = Model_Billsdeposits::instance().get(bdID);
    m_new_bill = bill ? false : true;

    if (!m_new_bill)
    {
        // If duplicate then we will be creating a new identity
        if (!m_dup_bill)
            m_bill_data.BDID = bdID;
        m_bill_data.TRANSDATE = bill->TRANSDATE;
        m_bill_data.ACCOUNTID = bill->ACCOUNTID;
        m_bill_data.TOACCOUNTID = bill->TOACCOUNTID;
        m_bill_data.PAYEEID = bill->PAYEEID;
        m_bill_data.CATEGID = bill->CATEGID;
        m_bill_data.TOTRANSAMOUNT = bill->TOTRANSAMOUNT;
        m_bill_data.TRANSAMOUNT = bill->TRANSAMOUNT;
        m_bill_data.NEXTOCCURRENCEDATE = bill->NEXTOCCURRENCEDATE;
        m_bill_data.REPEATS = bill->REPEATS;
        m_bill_data.NUMOCCURRENCES = bill->NUMOCCURRENCES;
        m_bill_data.NOTES = bill->NOTES;
        m_bill_data.STATUS = bill->STATUS;
        m_bill_data.TRANSACTIONNUMBER = bill->TRANSACTIONNUMBER;
        m_bill_data.TRANSCODE = bill->TRANSCODE;
        m_bill_data.FOLLOWUPID = bill->FOLLOWUPID;
        m_bill_data.COLOR = bill->COLOR;
        wxArrayInt64 billtags;
        for (const auto& tag : Model_Taglink::instance().find(
            Model_Taglink::REFTYPE(Model_Attachment::REFTYPE_STR_BILLSDEPOSIT),
            Model_Taglink::REFID(bill->BDID)
        ))
            billtags.push_back(tag.TAGID);
        m_bill_data.TAGS = billtags;
        //
        const wxString& splitRefType = Model_Attachment::REFTYPE_STR_BILLSDEPOSITSPLIT;
        for (const auto& item : Model_Billsdeposits::split(bill)) {
            wxArrayInt64 splittags;
            for (const auto& tag : Model_Taglink::instance().find(Model_Taglink::REFTYPE(splitRefType), Model_Taglink::REFID(item.SPLITTRANSID)))
                splittags.push_back(tag.TAGID);
            m_bill_data.local_splits.push_back({ item.CATEGID, item.SPLITTRANSAMOUNT, splittags, item.NOTES });
        }

        // If duplicate then we may need to copy the attachments
        if (m_dup_bill && Model_Infotable::instance().GetBoolInfo("ATTACHMENTSDUPLICATE", false))
        {
            const wxString& RefType = Model_Attachment::REFTYPE_STR_BILLSDEPOSIT;
            mmAttachmentManage::CloneAllAttachments(RefType, bdID, 0);
        }
    }

    m_transfer = (m_bill_data.TRANSCODE == Model_Checking::TYPE_STR_TRANSFER);

    int64 ref_id = m_dup_bill ?  -bdID : (m_new_bill ? 0 : -m_bill_data.BDID);
    m_custom_fields = new mmCustomDataTransaction(this, ref_id, ID_CUSTOMFIELDS);

    this->SetFont(parent->GetFont());
    Create(parent);
}

bool mmBDDialog::Create(wxWindow* parent, wxWindowID id, const wxString& caption
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

void mmBDDialog::dataToControls()
{
    Model_Checking::getFrequentUsedNotes(frequentNotes_);
    wxButton* bFrequentUsedNotes = static_cast<wxButton*>(FindWindow(ID_DIALOG_TRANS_BUTTON_FREQENTNOTES));
    bFrequentUsedNotes->Enable(!frequentNotes_.empty());

    bColours_->SetBackgroundColor(m_bill_data.COLOR.GetValue());

    for (const auto& entry : BILLSDEPOSITS_REPEATS)
    {
        m_choice_repeat->Append(wxGetTranslation(entry.second));
    }
    setRepeatType(Model_Billsdeposits::REPEAT_MONTHLY);

    for (const auto& i : Model_Checking::TYPE_STR)
    {
        if (i == Model_Checking::TYPE_STR_TRANSFER && Model_Account::instance().all().size() < 2)
            break;
        m_choice_transaction_type->Append(wxGetTranslation(i), new wxStringClientData(i));
    }
    m_choice_transaction_type->SetSelection(Model_Checking::TYPE_ID_WITHDRAWAL);

    SetTransferControls();  // hide appropriate fields
    setCategoryLabel();

    if (!(!m_new_bill || m_enter_occur)) {
        return;
    }

    m_choice_status->SetSelection(Model_Checking::status_id(m_bill_data.STATUS));

    // Set the date paid
    wxDateTime field_date;
    field_date.ParseDateTime(m_bill_data.TRANSDATE) || field_date.ParseDate(m_bill_data.TRANSDATE);
    m_date_paid->SetValue(field_date);

    // Set the due Date
    field_date.ParseDate(m_bill_data.NEXTOCCURRENCEDATE);
    m_date_due->SetValue(field_date);

    // demultiplex m_bill_data.REPEATS
    int autoExecute = m_bill_data.REPEATS.GetValue() / BD_REPEATS_MULTIPLEX_BASE;
    int repeats = m_bill_data.REPEATS.GetValue() % BD_REPEATS_MULTIPLEX_BASE;

    // fix repeats
    if (repeats < Model_Billsdeposits::REPEAT_ONCE || repeats > Model_Billsdeposits::REPEAT_MONTHLY_LAST_BUSINESS_DAY)
    {
        wxFAIL;
        repeats = Model_Billsdeposits::REPEAT_MONTHLY;
    }
    if (repeats >= Model_Billsdeposits::REPEAT_IN_X_DAYS && repeats <= Model_Billsdeposits::REPEAT_EVERY_X_MONTHS && m_bill_data.NUMOCCURRENCES < 1)
    {
        // old inactive entry. transform to REPEAT_ONCE and turn off automatic execution.
        repeats = Model_Billsdeposits::REPEAT_ONCE;
        autoExecute = Model_Billsdeposits::REPEAT_AUTO_NONE;
    }
    setRepeatType(repeats);

    if (repeats != Model_Billsdeposits::REPEAT_ONCE && m_bill_data.NUMOCCURRENCES > 0) {
        textNumRepeats_->SetValue(wxString::Format("%lld", m_bill_data.NUMOCCURRENCES));
    }

    if (autoExecute == Model_Billsdeposits::REPEAT_AUTO_SILENT)
    {
        autoExecuteSilent_ = true;
        itemCheckBoxAutoExeSilent_->SetValue(true);
        itemCheckBoxAutoExeUserAck_->Enable(false);
    }
    else if (autoExecute == Model_Billsdeposits::REPEAT_AUTO_MANUAL)
    {
        autoExecuteUserAck_ = true;
        itemCheckBoxAutoExeUserAck_->SetValue(true);
        itemCheckBoxAutoExeSilent_->Enable(false);
    }
    setRepeatDetails();

    m_choice_transaction_type->SetSelection(Model_Checking::type_id(m_bill_data.TRANSCODE));
    updateControlsForTransType();

    Model_Account::Data* account = Model_Account::instance().get(m_bill_data.ACCOUNTID);
    cbAccount_->ChangeValue(account ? account->ACCOUNTNAME : "");

    tagTextCtrl_->SetTags(m_bill_data.TAGS);

    textNotes_->SetValue(m_bill_data.NOTES);
    textNumber_->SetValue(m_bill_data.TRANSACTIONNUMBER);

    if (!m_bill_data.local_splits.empty())
        m_bill_data.TRANSAMOUNT = Model_Splittransaction::get_total(m_bill_data.local_splits);

    SetAmountCurrencies(m_bill_data.ACCOUNTID, m_bill_data.TOACCOUNTID);
    textAmount_->SetValue(m_bill_data.TRANSAMOUNT);

    if (m_transfer)
    {
        m_bill_data.PAYEEID = -1;

        // When editing an advanced transaction record, we do not reset the m_bill_data.TOTRANSAMOUNT
        if ((!m_new_bill || m_enter_occur) && (m_bill_data.TOTRANSAMOUNT != m_bill_data.TRANSAMOUNT))
        {
            cAdvanced_->SetValue(true);
            SetAdvancedTransferControls(true);
        }
    }

    if (!m_enter_occur)
    {
        if (m_dup_bill)
            SetDialogHeader(_("Duplicate Scheduled Transaction"));
        else
            SetDialogHeader(_("Edit Scheduled Transaction"));
        textAmount_->SetFocus();
    }
    else
    {
        SetDialogHeader(_("Enter Scheduled Transaction"));
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

void mmBDDialog::SetDialogHeader(const wxString& header)
{
    this->SetTitle(header);
}

void mmBDDialog::SetDialogParameters(int64 trx_id)
{
    const auto split = Model_Splittransaction::instance().get_all();
    const auto tags = Model_Taglink::instance().get_all(Model_Attachment::REFTYPE_STR_BILLSDEPOSIT);
    //const auto trx = Model_Checking::instance().find(Model_Checking::TRANSID(trx_id)).at(0);
    const auto trx = Model_Checking::instance().get(trx_id);
    Model_Checking::Full_Data t(*trx, split, tags);
    m_bill_data.ACCOUNTID = t.ACCOUNTID;
    cbAccount_->SetValue(t.ACCOUNTNAME);

    m_bill_data.TRANSCODE = t.TRANSCODE;
    m_choice_transaction_type->SetSelection(Model_Checking::type_id(t.TRANSCODE));
    m_transfer = (m_bill_data.TRANSCODE == Model_Checking::TYPE_STR_TRANSFER);
    updateControlsForTransType();

    m_bill_data.TRANSAMOUNT = t.TRANSAMOUNT;
    SetAmountCurrencies(t.ACCOUNTID, t.TOACCOUNTID);
    textAmount_->SetValue(m_bill_data.TRANSAMOUNT);

    if (m_transfer)
    {
        m_bill_data.TOACCOUNTID = t.TOACCOUNTID;
        cbToAccount_->ChangeValue(t.TOACCOUNTNAME);

        m_bill_data.TOTRANSAMOUNT = t.TOTRANSAMOUNT;
        toTextAmount_->SetValue(m_bill_data.TOTRANSAMOUNT);
        if (m_bill_data.TOTRANSAMOUNT != m_bill_data.TRANSAMOUNT)
        {
            cAdvanced_->SetValue(true);
            SetAdvancedTransferControls(true);
        }
    }
    else
    {
        m_bill_data.PAYEEID = t.PAYEEID;
        cbPayee_->ChangeValue(t.PAYEENAME);
    }

    if (t.has_split())
    {
        for (auto &split_trans : t.m_splits)
        {
            Split s;
            s.CATEGID = split_trans.CATEGID;
            s.SPLITTRANSAMOUNT = split_trans.SPLITTRANSAMOUNT;
            s.NOTES = split_trans.NOTES;
            m_bill_data.local_splits.push_back(s);
        }
    }
    else
    {
        m_bill_data.CATEGID = t.CATEGID;
    }

    m_bill_data.TRANSACTIONNUMBER = t.TRANSACTIONNUMBER;
    textNumber_->SetValue(m_bill_data.TRANSACTIONNUMBER);
    m_bill_data.NOTES = t.NOTES;
    textNotes_->SetValue(m_bill_data.NOTES);
    setCategoryLabel();
}

void mmBDDialog::CreateControls()
{
    wxBoxSizer* mainBoxSizerOuter = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* mainBoxSizerInner = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* custom_fields_box_sizer = new wxBoxSizer(wxHORIZONTAL);

    /**********************************************************************************************
     Determining where the controls go
    ***********************************************************************************************/

    /* Bills & Deposits Details */

    wxStaticBox* repeatDetailsStaticBox = new wxStaticBox(this, wxID_ANY, _("Scheduled Transaction Details"));
    wxStaticBoxSizer* repeatTransBoxSizer = new wxStaticBoxSizer(repeatDetailsStaticBox, wxVERTICAL);

    //mainBoxSizerInner will align contents horizontally
    mainBoxSizerInner->Add(repeatTransBoxSizer, g_flagsV);
    //mainBoxSizerOuter will align contents vertically
    mainBoxSizerOuter->Add(mainBoxSizerInner, g_flagsExpand);

    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    repeatTransBoxSizer->Add(itemFlexGridSizer5);

    // Date Due --------------------------------------------

    m_date_due = new mmDatePickerCtrl(this, ID_DIALOG_BD_DUE_DATE);
    mmToolTip(m_date_due, _("Specify the date when this bill or deposit is due"));
    itemFlexGridSizer5->Add(new wxStaticText(this, wxID_STATIC, _("Date Due")), g_flagsH);
    itemFlexGridSizer5->Add(m_date_due->mmGetLayout(false));

    // Repeats --------------------------------------------

    staticTextRepeats_ = new wxStaticText(this, wxID_STATIC, _("Repeats"));
    itemFlexGridSizer5->Add(staticTextRepeats_, g_flagsH);
    itemFlexGridSizer5->AddSpacer(1);

    m_btn_due_prev_date = new wxBitmapButton(this, ID_DIALOG_TRANS_BUTTONTRANSNUMPREV
        , mmBitmapBundle(png::LEFTARROW, mmBitmapButtonSize));
    mmToolTip(m_btn_due_prev_date, _("Back to the last occurring date with the specified values"));

    m_choice_repeat = new wxChoice(this, ID_DIALOG_BD_COMBOBOX_REPEATS);

    wxBoxSizer* repeatBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    m_btn_due_date = new wxBitmapButton(this, ID_DIALOG_TRANS_BUTTONTRANSNUM
        , mmBitmapBundle(png::RIGHTARROW, mmBitmapButtonSize));
    mmToolTip(m_btn_due_date, _("Advance the next occurring date with the specified values"));
    repeatBoxSizer->Add(m_btn_due_prev_date, g_flagsH);
    repeatBoxSizer->Add(m_choice_repeat, wxSizerFlags(g_flagsExpand));
    repeatBoxSizer->Add(m_btn_due_date, g_flagsH);

    repeatTransBoxSizer->Add(repeatBoxSizer);

    wxFlexGridSizer* itemFlexGridSizer52 = new wxFlexGridSizer(0, 2, 0, 0);
    repeatTransBoxSizer->Add(itemFlexGridSizer52);

    // Repeat Times --------------------------------------------
    staticTimesRepeat_ = new wxStaticText(this, wxID_STATIC, _("Payments Left"));
    itemFlexGridSizer52->Add(staticTimesRepeat_, g_flagsH);

    wxBoxSizer* repeatTimesBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer52->Add(repeatTimesBoxSizer);

    wxIntegerValidator<int> intValidator(nullptr, wxNUM_VAL_ZERO_AS_BLANK);
    intValidator.SetMin(0);
    textNumRepeats_ = new wxTextCtrl(this, ID_DIALOG_BD_TEXTCTRL_NUM_TIMES, "", wxDefaultPosition, m_date_due->GetSize(), 0, intValidator);
    repeatTimesBoxSizer->Add(textNumRepeats_, g_flagsH);
    textNumRepeats_->SetMaxLength(12);
    setRepeatDetails();

    /* Auto Execution Status */
    itemCheckBoxAutoExeUserAck_ = new wxCheckBox(this, ID_DIALOG_BD_CHECKBOX_AUTO_EXECUTE_USERACK
        ,  _("Request user to enter payment"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    mmToolTip(itemCheckBoxAutoExeUserAck_, _("User requested to enter this transaction on the 'Date Paid'"));

    itemCheckBoxAutoExeSilent_ = new wxCheckBox(this, ID_DIALOG_BD_CHECKBOX_AUTO_EXECUTE_SILENT
        , _("Grant automatic execute"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    mmToolTip(itemCheckBoxAutoExeSilent_, _("The requested payment will occur without user interaction"));

    repeatTransBoxSizer->Add(itemCheckBoxAutoExeUserAck_, g_flagsExpand);
    repeatTransBoxSizer->Add(itemCheckBoxAutoExeSilent_, g_flagsExpand);

    /*************************************************************************************************************/

    wxStaticBox* transDetailsStaticBox = new wxStaticBox(this, wxID_REMOVE, _("Transaction Details"));
    wxStaticBoxSizer* transDetailsStaticBoxSizer = new wxStaticBoxSizer(transDetailsStaticBox, wxVERTICAL);
    wxFlexGridSizer* transPanelSizer = new wxFlexGridSizer(0, 3, 0, 0);
    transPanelSizer->AddGrowableCol(1, 0);
    transDetailsStaticBoxSizer->Add(transPanelSizer, wxSizerFlags(g_flagsV).Expand());
    mainBoxSizerInner->Add(transDetailsStaticBoxSizer, g_flagsExpand);

    // Trans Date --------------------------------------------
    m_date_paid = new mmDatePickerCtrl(this, ID_DIALOG_TRANS_BUTTON_PAYDATE);
    mmToolTip(m_date_paid, _("Specify the date the user is requested to enter this transaction"));
    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Date Paid")), g_flagsH);
    transPanelSizer->Add(m_date_paid->mmGetLayout());
    transPanelSizer->AddSpacer(1);

    // Status --------------------------------------------
    m_choice_status = new wxChoice(this, ID_DIALOG_TRANS_STATUS);

    for (const auto& i : Model_Checking::STATUS_STR)
    {
        m_choice_status->Append(wxGetTranslation(i), new wxStringClientData(i));
    }
    m_choice_status->SetSelection(Option::instance().getTransStatusReconciled());
    mmToolTip(m_choice_status, _("Specify the status for the transaction"));

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Status")), g_flagsH);
    transPanelSizer->Add(m_choice_status, g_flagsH);
    transPanelSizer->AddSpacer(1);

    // Type --------------------------------------------
    m_choice_transaction_type = new wxChoice(this, wxID_VIEW_DETAILS);
    mmToolTip(m_choice_transaction_type, _("Specify the type of transactions to be created."));
    cAdvanced_ = new wxCheckBox(this, ID_DIALOG_TRANS_ADVANCED_CHECKBOX, _("&Advanced")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cAdvanced_->SetValue(false);
    mmToolTip(cAdvanced_, _("Allows the setting of different amounts in the FROM and TO accounts."));

    wxBoxSizer* typeSizer = new wxBoxSizer(wxHORIZONTAL);
    typeSizer->Add(m_choice_transaction_type, g_flagsH);
    typeSizer->Add(cAdvanced_, g_flagsH);

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Type")), g_flagsH);
    transPanelSizer->Add(typeSizer);
    transPanelSizer->AddSpacer(1);

    // Amount Fields --------------------------------------------
    wxStaticText* amount_label = new wxStaticText(this, wxID_STATIC, _("Amount"));
    amount_label->SetFont(this->GetFont().Bold());

    textAmount_ = new mmTextCtrl(this, ID_DIALOG_TRANS_TEXTAMOUNT, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER
        , mmCalcValidator());
    mmToolTip(textAmount_, amountNormalTip_);

    toTextAmount_ = new mmTextCtrl(this, ID_DIALOG_TRANS_TOTEXTAMOUNT, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER
        , mmCalcValidator());
    mmToolTip(toTextAmount_, _("Specify the transfer amount in the To Account"));

    wxBoxSizer* amountSizer = new wxBoxSizer(wxHORIZONTAL);
    amountSizer->Add(textAmount_, g_flagsExpand);
    amountSizer->Add(toTextAmount_, g_flagsExpand);

    transPanelSizer->Add(amount_label, g_flagsH);
    transPanelSizer->Add(amountSizer, wxSizerFlags(g_flagsExpand).Border(0));

    bCalc_ = new wxBitmapButton(this, wxID_ANY, mmBitmapBundle(png::CALCULATOR, mmBitmapButtonSize));
    bCalc_->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(mmBDDialog::OnCalculator), nullptr, this);
    mmToolTip(bCalc_, _("Open Calculator"));
    transPanelSizer->Add(bCalc_, g_flagsH);
    calcTarget_ = textAmount_;
    calcPopup_ = new mmCalculatorPopup(bCalc_, calcTarget_);

    // Account ------------------------------------------------
    wxStaticText* acc_label = new wxStaticText(this, ID_DIALOG_TRANS_STATIC_ACCOUNT, _("Account"));
    acc_label->SetFont(this->GetFont().Bold());
    transPanelSizer->Add(acc_label, g_flagsH);
    cbAccount_ = new mmComboBoxAccount(this, mmID_ACCOUNTNAME, wxDefaultSize, m_bill_data.ACCOUNTID);
    cbAccount_->SetMinSize(cbAccount_->GetSize());
    mmToolTip(cbAccount_, _("Specify the Account that will own the scheduled transaction"));
    transPanelSizer->Add(cbAccount_, g_flagsExpand);
    transPanelSizer->AddSpacer(1);

    // To Account ------------------------------------------------
    wxStaticText* to_acc_label = new wxStaticText(this, ID_DIALOG_TRANS_STATIC_TOACCOUNT, _("To"));
    to_acc_label->SetFont(this->GetFont().Bold());
    transPanelSizer->Add(to_acc_label, g_flagsH);
    cbToAccount_ = new mmComboBoxAccount(this, mmID_TOACCOUNTNAME, wxDefaultSize, m_bill_data.TOACCOUNTID);
    cbToAccount_->SetMinSize(cbToAccount_->GetSize());
    mmToolTip(cbToAccount_, payeeTransferTip_);
    transPanelSizer->Add(cbToAccount_, g_flagsExpand);
    transPanelSizer->AddSpacer(1);

    // Payee ------------------------------------------------
    wxStaticText* payee_label = new wxStaticText(this, ID_DIALOG_TRANS_STATIC_PAYEE, _("Payee"));
    payee_label->SetFont(this->GetFont().Bold());

    cbPayee_ = new mmComboBoxPayee(this, mmID_PAYEE, wxDefaultSize, m_bill_data.PAYEEID, true);
    mmToolTip(cbPayee_, payeeWithdrawalTip_);
    cbPayee_->SetMinSize(cbPayee_->GetSize());
    transPanelSizer->Add(payee_label, g_flagsH);
    transPanelSizer->Add(cbPayee_, g_flagsExpand);
    transPanelSizer->AddSpacer(1);

    // Category ---------------------------------------------

    wxStaticText* categ_label2 = new wxStaticText(this, ID_DIALOG_TRANS_CATEGLABEL, _("Category"));
    categ_label2->SetFont(this->GetFont().Bold());
    cbCategory_ = new mmComboBoxCategory(this, mmID_CATEGORY, wxDefaultSize
                                            , m_bill_data.CATEGID, true);
    cbCategory_->SetMinSize(cbCategory_->GetSize());
    bSplit_ = new wxBitmapButton(this, ID_DIALOG_TRANS_BUTTONSPLIT, mmBitmapBundle(png::NEW_TRX, mmBitmapButtonSize));
    mmToolTip(bSplit_, _("Use split Categories"));

    transPanelSizer->Add(categ_label2, g_flagsH);
    transPanelSizer->Add(cbCategory_, g_flagsExpand);
    transPanelSizer->Add(bSplit_, g_flagsH);

    // Tags ---------------------------------------------

    wxStaticText* tag_label = new wxStaticText(this, wxID_ANY, _("Tags"));
    tagTextCtrl_ = new mmTagTextCtrl(this);
    
    transPanelSizer->Add(tag_label, g_flagsH);
    transPanelSizer->Add(tagTextCtrl_, g_flagsExpand);
    transPanelSizer->AddSpacer(1);

    // Number ---------------------------------------------
    textNumber_ = new wxTextCtrl(this, ID_DIALOG_TRANS_TEXTNUMBER, "", wxDefaultPosition, wxDefaultSize);
    mmToolTip(textNumber_, _("Specify any associated check number or transaction number"));

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Number")), g_flagsH);
    transPanelSizer->Add(textNumber_, g_flagsExpand);
    transPanelSizer->AddSpacer(1);

    // Frequently Used Notes
    wxButton* bFrequentUsedNotes = new wxButton(this, ID_DIALOG_TRANS_BUTTON_FREQENTNOTES, "..."
        , wxDefaultPosition, bSplit_->GetSize());
    mmToolTip(bFrequentUsedNotes, _("Select one of the frequently used notes"));
    bFrequentUsedNotes->Connect(ID_DIALOG_TRANS_BUTTON_FREQENTNOTES
        , wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(mmBDDialog::OnFrequentUsedNotes), nullptr, this);

    // Colours
    bColours_ = new mmColorButton(this, wxID_LOWEST, bSplit_->GetSize());
    mmToolTip(bColours_, _("User Colors"));

    // Attachments
    bAttachments_ = new wxBitmapButton(this, wxID_FILE, mmBitmapBundle(png::CLIP, mmBitmapButtonSize));
    mmToolTip(bAttachments_, _("Organize attachments of this scheduled transaction"));

    // Now display the Frequntly Used Notes, Colour, Attachment buttons
    wxBoxSizer* notes_sizer = new wxBoxSizer(wxHORIZONTAL);
    transPanelSizer->Add(notes_sizer);
    notes_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Notes")), g_flagsH);
    notes_sizer->Add(bFrequentUsedNotes, g_flagsH);

    wxBoxSizer* RightAlign_sizer = new wxBoxSizer(wxHORIZONTAL);
    transPanelSizer->Add(RightAlign_sizer, wxSizerFlags(g_flagsH).Align(wxALIGN_RIGHT));
    RightAlign_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Color")), g_flagsH);
    RightAlign_sizer->Add(bColours_, wxSizerFlags());
    transPanelSizer->Add(bAttachments_, g_flagsH);

    // Notes
    textNotes_ = new wxTextCtrl(this, ID_DIALOG_TRANS_TEXTNOTES, ""
        , wxDefaultPosition, wxSize(-1, m_date_due->GetSize().GetHeight() * 5), wxTE_MULTILINE);
    mmToolTip(textNotes_, _("Specify any text notes you want to add to this transaction."));
    transDetailsStaticBoxSizer->Add(textNotes_, wxSizerFlags(g_flagsExpand).Border(wxLEFT | wxRIGHT | wxBOTTOM, 10));

    /**********************************************************************************************
        Button Panel with OK and Cancel Buttons
    ***********************************************************************************************/
    wxPanel* buttonsPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    wxBoxSizer* button_sizer = new wxBoxSizer(wxHORIZONTAL);
    buttonsPanel->SetSizer(button_sizer);

    wxButton* button_ok = new wxButton(buttonsPanel, wxID_OK, _("&OK "));

    m_button_cancel = new wxButton(buttonsPanel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));

    mainBoxSizerOuter->Add(buttonsPanel, wxSizerFlags(g_flagsV).Center().Border(wxALL, 0));
    wxBitmapButton* button_hide = new wxBitmapButton(buttonsPanel
        , ID_BTN_CUSTOMFIELDS, mmBitmapBundle(png::RIGHTARROW, mmBitmapButtonSize));
    mmToolTip(button_hide, _("Show/Hide custom fields window"));
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

void mmBDDialog::OnQuit(wxCloseEvent& WXUNUSED(event))
{
    const wxString& RefType = Model_Attachment::REFTYPE_STR_BILLSDEPOSIT;
    if (m_bill_data.BDID != 0)
        mmAttachmentManage::DeleteAllAttachments(RefType, m_bill_data.BDID);
    EndModal(wxID_CANCEL);
}

void mmBDDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
#ifdef __WXMSW__
    wxWindow* w = FindFocus();
    if (w && w->GetId() != wxID_CANCEL && wxGetKeyState(WXK_ESCAPE))
        return m_button_cancel->SetFocus();

    if (w && w->GetId() != wxID_CANCEL) {
        return;
    }
#endif

    const wxString& RefType = Model_Attachment::REFTYPE_STR_BILLSDEPOSIT;
    if (m_bill_data.BDID != 0)
        mmAttachmentManage::DeleteAllAttachments(RefType, m_bill_data.BDID);
    EndModal(wxID_CANCEL);
}

void mmBDDialog::OnPayee(wxCommandEvent& WXUNUSED(event))
{
    Model_Payee::Data* payee = Model_Payee::instance().get(cbPayee_->mmGetId());
    if (payee && m_new_bill)
    {
        // Only for new/duplicate transactions: if user want to autofill last category used for payee.
        // If this is a Split Transaction, ignore displaying last category for payee
        if (payee->CATEGID != -1 && m_bill_data.local_splits.empty()
            && (Option::instance().getTransCategoryNone() == Option::LASTUSED ||
                Option::instance().getTransCategoryNone() == Option::DEFAULT)
            && (!Model_Category::is_hidden(payee->CATEGID) && !Model_Category::is_hidden(payee->CATEGID)))
        {
            m_bill_data.CATEGID = payee->CATEGID;

            cbCategory_->ChangeValue(Model_Category::full_name(m_bill_data.CATEGID));
        }
    }
}

void mmBDDialog::SetAmountCurrencies(int64 accountID, int64 toAccountID)
{
    Model_Account::Data* account = Model_Account::instance().get(accountID);
    if (account)
        textAmount_->SetCurrency(Model_Currency::instance().get(account->CURRENCYID));

    account = Model_Account::instance().get(toAccountID);
    if (account)
        toTextAmount_->SetCurrency(Model_Currency::instance().get(account->CURRENCYID));

}

void mmBDDialog::OnCategs(wxCommandEvent& WXUNUSED(event))
{
    activateSplitTransactionsDlg();
}

void mmBDDialog::OnTypeChanged(wxCommandEvent& WXUNUSED(event))
{
    updateControlsForTransType();
}

void mmBDDialog::OnComboKey(wxKeyEvent& event)
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
                    cbPayee_->SelectAll();
                    wxCommandEvent evt;
                    OnPayee(evt);
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
                dlg.ShowModal();
                if (dlg.getRefreshRequested())
                    cbCategory_->mmDoReInitialize();
                category = Model_Category::full_name(dlg.getCategId());
                cbCategory_->ChangeValue(category);
                cbCategory_->SelectAll();
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

void mmBDDialog::OnAttachments(wxCommandEvent& WXUNUSED(event))
{
    const wxString& RefType = Model_Attachment::REFTYPE_STR_BILLSDEPOSIT;
    mmAttachmentDialog dlg(this, RefType, m_bill_data.BDID);
    dlg.ShowModal();
}

void mmBDDialog::updateControlsForTransType()
{
    wxStaticText* accountLabel = static_cast<wxStaticText*>(FindWindow(ID_DIALOG_TRANS_STATIC_ACCOUNT));
    wxStaticText* stp = static_cast<wxStaticText*>(FindWindow(ID_DIALOG_TRANS_STATIC_PAYEE));

    m_transfer = false;
    switch (m_choice_transaction_type->GetSelection())
    {
    case Model_Checking::TYPE_ID_TRANSFER:
    {
        m_transfer = true;
        mmToolTip(textAmount_, amountTransferTip_);
        accountLabel->SetLabelText(_("From"));

        cbToAccount_->mmSetId(m_bill_data.TOACCOUNTID);
        m_bill_data.PAYEEID = -1;
        break;
    }
    case Model_Checking::TYPE_ID_WITHDRAWAL:
    {
        mmToolTip(textAmount_, amountNormalTip_);
        accountLabel->SetLabelText(_("Account"));
        stp->SetLabelText(_("Payee"));
        mmToolTip(cbPayee_, payeeWithdrawalTip_);

        cbPayee_->mmSetId(m_bill_data.PAYEEID);
        m_bill_data.TOACCOUNTID = -1;
        wxCommandEvent evt;
        OnPayee(evt);
        break;
    }
    case Model_Checking::TYPE_ID_DEPOSIT:
    {
        mmToolTip(textAmount_, amountNormalTip_);
        accountLabel->SetLabelText(_("Account"));
        stp->SetLabelText(_("From"));
        mmToolTip(cbPayee_, payeeDepositTip_);

        cbPayee_->mmSetId(m_bill_data.PAYEEID);
        m_bill_data.TOACCOUNTID = -1;
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

void mmBDDialog::OnFrequentUsedNotes(wxCommandEvent& WXUNUSED(event))
{
    wxMenu menu;
    int id = wxID_HIGHEST;
    for (const auto& entry : frequentNotes_) {
        const wxString& label = entry.Mid(0, 30) + (entry.size() > 30 ? "..." : "");
        menu.Append(++id, label);

    }
    menu.Bind(wxEVT_COMMAND_MENU_SELECTED, &mmBDDialog::OnNoteSelected, this);
    if (!frequentNotes_.empty())
        PopupMenu(&menu);
}

void mmBDDialog::OnNoteSelected(wxCommandEvent& event)
{
    int i = event.GetId() - wxID_HIGHEST;
    if (i > 0 && static_cast<size_t>(i) <= frequentNotes_.size()) {
        if (!textNotes_->GetValue().EndsWith("\n") && !textNotes_->GetValue().empty())
            textNotes_->AppendText("\n");
        textNotes_->AppendText(frequentNotes_[i - 1]);
    }
}

void mmBDDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    // Ideally 'paid date' should be on or before the 'due date'
    if (m_date_paid->GetValue().GetDateOnly() > m_date_due->GetValue())
        if (wxMessageBox(_("The payment date is after the due date, is this what you intended?"),
            _("Looks like a late payment"),
            wxYES_NO | wxNO_DEFAULT | wxICON_WARNING) != wxYES)
            return;

    if (!cbAccount_->mmIsValid()) {
        return mmErrorDialogs::InvalidAccount(cbAccount_, m_transfer, mmErrorDialogs::MESSAGE_DROPDOWN_BOX);
    }
    m_bill_data.ACCOUNTID = cbAccount_->mmGetId();
    Model_Account::Data* acc = Model_Account::instance().get(m_bill_data.ACCOUNTID);

    if (!textAmount_->checkValue(m_bill_data.TRANSAMOUNT)) return;

    m_bill_data.TOTRANSAMOUNT = m_bill_data.TRANSAMOUNT;
    if (m_transfer)
    {
        if (!cbToAccount_->mmIsValid()) {
            return mmErrorDialogs::InvalidAccount(cbToAccount_, m_transfer, mmErrorDialogs::MESSAGE_DROPDOWN_BOX);
        }
        m_bill_data.TOACCOUNTID = cbToAccount_->mmGetId();

        if (m_bill_data.TOACCOUNTID == m_bill_data.ACCOUNTID) {
            return mmErrorDialogs::InvalidAccount(cbPayee_, true);
        }

        if (m_advanced && !toTextAmount_->checkValue(m_bill_data.TOTRANSAMOUNT)) return;
    }
    else
    {
        wxString payee_name = cbPayee_->GetValue();
        if (payee_name.IsEmpty())
        {
            mmErrorDialogs::InvalidPayee(cbPayee_);
            return;
        }

        // Get payee string from populated list to address issues with case compare differences between autocomplete and payee list
        int payee_loc = cbPayee_->FindString(payee_name);
        if (payee_loc != wxNOT_FOUND)
            payee_name = cbPayee_->GetString(payee_loc);

        Model_Payee::Data* payee = Model_Payee::instance().get(payee_name);
        if (!payee)
        {
            wxMessageDialog msgDlg( this
                , wxString::Format(_("You have not used this payee name before. Is the name correct?\n%s"), payee_name)
                , _("Confirm payee name")
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
                return;
        }
        m_bill_data.PAYEEID = payee->PAYEEID;
    }

    if (m_bill_data.local_splits.empty())
    {
        if (!cbCategory_->mmIsValid()) {
            return mmErrorDialogs::ToolTip4Object(cbCategory_, _("Invalid value"), _("Category"), wxICON_ERROR);
        }
        m_bill_data.CATEGID = cbCategory_->mmGetCategoryId();
    }

    if (!tagTextCtrl_->IsValid()) {
        return mmErrorDialogs::ToolTip4Object(tagTextCtrl_, _("Invalid value"), _("Tags"), wxICON_ERROR);
    }

    if (!m_custom_fields->ValidateCustomValues(-m_bill_data.BDID))
        return;

    if (!m_advanced || m_bill_data.TOTRANSAMOUNT < 0)
    {
        // if we are adding a new record and the user did not touch advanced dialog
        // we are going to use the transfer amount by calculating conversion rate.
        // subsequent edits will not allow automatic update of the amount
        if (m_new_bill)
        {
            if (m_bill_data.TOACCOUNTID != -1)
            {
                Model_Account::Data* to_account = Model_Account::instance().get(m_bill_data.TOACCOUNTID);

                Model_Currency::Data* from_currency = Model_Account::currency(acc);
                Model_Currency::Data* to_currency = Model_Account::currency(to_account);

                double rateFrom = Model_CurrencyHistory::getDayRate(from_currency->CURRENCYID, m_bill_data.TRANSDATE);
                double rateTo = Model_CurrencyHistory::getDayRate(to_currency->CURRENCYID, m_bill_data.TRANSDATE);

                double convToBaseFrom = rateFrom * m_bill_data.TRANSAMOUNT;
                m_bill_data.TOTRANSAMOUNT = convToBaseFrom / rateTo;
            }
            else
            {
                m_bill_data.TOTRANSAMOUNT = m_bill_data.TRANSAMOUNT;
            }
        }
    }

    int autoExecute =
        autoExecuteSilent_ ? Model_Billsdeposits::REPEAT_AUTO_SILENT :
        autoExecuteUserAck_ ? Model_Billsdeposits::REPEAT_AUTO_MANUAL :
        Model_Billsdeposits::REPEAT_AUTO_NONE;
    int repeats = getRepeatType();
    // multiplex autoExecute and repeats
    m_bill_data.REPEATS = autoExecute * BD_REPEATS_MULTIPLEX_BASE + repeats;

    const wxString& numRepeatStr = textNumRepeats_->GetValue();
    m_bill_data.NUMOCCURRENCES = -1;
    if (repeats >= Model_Billsdeposits::REPEAT_IN_X_DAYS && repeats <= Model_Billsdeposits::REPEAT_EVERY_X_MONTHS)
        m_bill_data.NUMOCCURRENCES = 1;
    if (!numRepeatStr.empty())
    {
        long cnt = 0;
        if (numRepeatStr.ToLong(&cnt) && cnt > 0) {
            wxASSERT(cnt <= std::numeric_limits<int>::max());
            m_bill_data.NUMOCCURRENCES = cnt;
        }
    }

    m_bill_data.NEXTOCCURRENCEDATE = m_date_due->GetValue().FormatISODate();
    m_bill_data.TRANSDATE = m_date_paid->GetValue().FormatISOCombined();

    wxStringClientData* status_obj = static_cast<wxStringClientData *>(m_choice_status->GetClientObject(m_choice_status->GetSelection()));
    if (status_obj) {
        m_bill_data.STATUS = Model_Checking::status_key(status_obj->GetData());
    }

    m_bill_data.TRANSACTIONNUMBER = textNumber_->GetValue();
    m_bill_data.NOTES = textNotes_->GetValue();

    int color_id = bColours_->GetColorId();
    if (color_id > 0 && color_id < 8)
        m_bill_data.COLOR = color_id;
    else
        m_bill_data.COLOR = -1;

    const Model_Account::Data* account = Model_Account::instance().get(m_bill_data.ACCOUNTID);
    const Model_Account::Data* toAccount = Model_Account::instance().get(m_bill_data.TOACCOUNTID);
    if (m_bill_data.TRANSDATE < account->INITIALDATE)
        return mmErrorDialogs::ToolTip4Object(cbAccount_, _("The opening date for the account is later than the date of this transaction"), _("Invalid Date"));
  
    if (toAccount && (m_bill_data.TRANSDATE < toAccount->INITIALDATE))
        return mmErrorDialogs::ToolTip4Object(cbToAccount_, _("The opening date for the account is later than the date of this transaction"), _("Invalid Date"));

    if (!m_enter_occur)
    {
        Model_Billsdeposits::Data* bill = Model_Billsdeposits::instance().get(m_bill_data.BDID);
        if (m_new_bill || m_dup_bill) {
            bill = Model_Billsdeposits::instance().create();
        }

        bill->ACCOUNTID = m_bill_data.ACCOUNTID;
        bill->TOACCOUNTID = m_bill_data.TOACCOUNTID;
        bill->PAYEEID = m_bill_data.PAYEEID;
        bill->TRANSCODE = Model_Checking::TYPE_STR[m_choice_transaction_type->GetSelection()];
        bill->TRANSAMOUNT = m_bill_data.TRANSAMOUNT;
        bill->STATUS = m_bill_data.STATUS;
        bill->TRANSACTIONNUMBER = m_bill_data.TRANSACTIONNUMBER;
        bill->NOTES = m_bill_data.NOTES;
        bill->CATEGID = m_bill_data.CATEGID;
        bill->TRANSDATE = m_bill_data.TRANSDATE;
        bill->TOTRANSAMOUNT = m_bill_data.TOTRANSAMOUNT;
        bill->REPEATS = m_bill_data.REPEATS;
        bill->NEXTOCCURRENCEDATE = m_bill_data.NEXTOCCURRENCEDATE;
        bill->NUMOCCURRENCES = m_bill_data.NUMOCCURRENCES;
        bill->FOLLOWUPID = m_bill_data.FOLLOWUPID;
        bill->COLOR = m_bill_data.COLOR;

        m_trans_id = Model_Billsdeposits::instance().save(bill);

        Model_Budgetsplittransaction::Data_Set splt;
        for (const auto& entry : m_bill_data.local_splits)
        {
            Model_Budgetsplittransaction::Data *s = Model_Budgetsplittransaction::instance().create();
            s->CATEGID = entry.CATEGID;
            s->SPLITTRANSAMOUNT = entry.SPLITTRANSAMOUNT;
            s->NOTES = entry.NOTES;
            splt.push_back(*s);
        }
        Model_Budgetsplittransaction::instance().update(splt, m_trans_id);

        // Save split tags
        const wxString& splitRefType = Model_Attachment::REFTYPE_STR_BILLSDEPOSITSPLIT;

        for (size_t i = 0; i < m_bill_data.local_splits.size(); i++)
        {
            Model_Taglink::Data_Set splitTaglinks;
            for (const auto& tagId : m_bill_data.local_splits.at(i).TAGS)
            {
                Model_Taglink::Data* t = Model_Taglink::instance().create();
                t->REFTYPE = splitRefType;
                t->REFID = splt.at(i).SPLITTRANSID;
                t->TAGID = tagId;
                splitTaglinks.push_back(*t);
            }
            Model_Taglink::instance().update(splitTaglinks, splitRefType, splt.at(i).SPLITTRANSID);
        }

        const wxString& RefType = Model_Attachment::REFTYPE_STR_BILLSDEPOSIT;
        mmAttachmentManage::RelocateAllAttachments(RefType, 0, RefType, m_trans_id);

        // Save base transaction tags
        Model_Taglink::Data_Set taglinks;
        for (const auto& tagId : tagTextCtrl_->GetTagIDs())
        {
            Model_Taglink::Data* t = Model_Taglink::instance().create();
            t->REFTYPE = RefType;
            t->REFID = m_trans_id;
            t->TAGID = tagId;
            taglinks.push_back(*t);
        }
        Model_Taglink::instance().update(taglinks, RefType, m_trans_id);

        //Custom Data
        m_custom_fields->SaveCustomValues(-m_trans_id);

    }
    else
    {
        // m_bill_data.REPEATS holds extra info; use repeats instead
        // the following condition is always true, since old inactive entries of type
        // REPEAT_IN_X_*, REPEAT_EVERY_X_* have been converted to entries of type REPEAT_ONCE
        if ((repeats < Model_Billsdeposits::REPEAT_IN_X_DAYS)
            || (repeats > Model_Billsdeposits::REPEAT_EVERY_X_MONTHS)
            || (m_bill_data.NUMOCCURRENCES > 0))
        {
            Model_Billsdeposits::Data bill_data;
            bill_data.ACCOUNTID = m_bill_data.ACCOUNTID;
            bill_data.TRANSCODE = m_bill_data.TRANSCODE;
            bill_data.TRANSAMOUNT = m_bill_data.TRANSAMOUNT;
            if (!Model_Billsdeposits::instance().AllowTransaction(bill_data)) return;

            Model_Checking::Data* tran = Model_Checking::instance().create();
            tran->ACCOUNTID = m_bill_data.ACCOUNTID;
            tran->TOACCOUNTID = m_bill_data.TOACCOUNTID;
            tran->PAYEEID = m_bill_data.PAYEEID;
            tran->TRANSCODE = Model_Checking::TYPE_STR[m_choice_transaction_type->GetSelection()];
            tran->TRANSAMOUNT = m_bill_data.TRANSAMOUNT;
            tran->STATUS = m_bill_data.STATUS;
            tran->TRANSACTIONNUMBER = m_bill_data.TRANSACTIONNUMBER;
            tran->NOTES = m_bill_data.NOTES;
            tran->CATEGID = m_bill_data.CATEGID;
            tran->TRANSDATE = m_bill_data.TRANSDATE;
            tran->TOTRANSAMOUNT = m_bill_data.TOTRANSAMOUNT;
            tran->FOLLOWUPID = m_bill_data.FOLLOWUPID;
            tran->COLOR = m_bill_data.COLOR;
            int64 trans_id = Model_Checking::instance().save(tran);

            Model_Splittransaction::Data_Set checking_splits;
            for (auto &item : m_bill_data.local_splits)
            {
                Model_Splittransaction::Data *split = Model_Splittransaction::instance().create();
                split->TRANSID = trans_id;
                split->CATEGID = item.CATEGID;
                split->SPLITTRANSAMOUNT = item.SPLITTRANSAMOUNT;
                split->NOTES = item.NOTES;
                checking_splits.push_back(*split);
            }
            Model_Splittransaction::instance().update(checking_splits, trans_id);

            // Save split tags
            const wxString& splitRefType = Model_Attachment::REFTYPE_STR_TRANSACTIONSPLIT;

            for (size_t i = 0; i < m_bill_data.local_splits.size(); i++)
            {
                Model_Taglink::Data_Set splitTaglinks;
                for (const auto& tagId : m_bill_data.local_splits.at(i).TAGS)
                {
                    Model_Taglink::Data* t = Model_Taglink::instance().create();
                    t->REFTYPE = splitRefType;
                    t->REFID = checking_splits.at(i).SPLITTRANSID;
                    t->TAGID = tagId;
                    splitTaglinks.push_back(*t);
                }
                Model_Taglink::instance().update(splitTaglinks, splitRefType, checking_splits.at(i).SPLITTRANSID);
            }

            //Custom Data
            m_custom_fields->SaveCustomValues(trans_id);

            const wxString& oldRefType = Model_Attachment::REFTYPE_STR_BILLSDEPOSIT;
            const wxString& newRefType = Model_Attachment::REFTYPE_STR_TRANSACTION;
            mmAttachmentManage::RelocateAllAttachments(oldRefType, m_bill_data.BDID, newRefType, trans_id);

            // Save base transaction tags
            Model_Taglink::Data_Set taglinks;
            for (const auto& tagId : tagTextCtrl_->GetTagIDs())
            {
                Model_Taglink::Data* t = Model_Taglink::instance().create();
                t->REFTYPE = newRefType;
                t->REFID = trans_id;
                t->TAGID = tagId;
                taglinks.push_back(*t);
            }
            Model_Taglink::instance().update(taglinks, newRefType, trans_id);
        }
        Model_Billsdeposits::instance().completeBDInSeries(m_bill_data.BDID);
    }

    EndModal(wxID_OK);
}

void mmBDDialog::SetSplitControls(bool split)
{
    textAmount_->Enable(!split);
    bCalc_->Enable(!split);
    if (split)
    {
        m_bill_data.TRANSAMOUNT = Model_Splittransaction::get_total(m_bill_data.local_splits);
        m_bill_data.CATEGID = -1;
    }
    else
    {
        m_bill_data.local_splits.clear();
    }
    setCategoryLabel();
}

void mmBDDialog::OnAutoExecutionUserAckChecked(wxCommandEvent& WXUNUSED(event))
{
    autoExecuteUserAck_ = !autoExecuteUserAck_;
    if (autoExecuteUserAck_)
    {
        itemCheckBoxAutoExeSilent_->SetValue(false);
        itemCheckBoxAutoExeSilent_->Enable(false);
        autoExecuteSilent_ = false;
    }
    else
    {
        itemCheckBoxAutoExeSilent_->Enable(true);
    }
}

void mmBDDialog::OnAutoExecutionSilentChecked(wxCommandEvent& WXUNUSED(event))
{
    autoExecuteSilent_ = !autoExecuteSilent_;
    if (autoExecuteSilent_)
    {
        itemCheckBoxAutoExeUserAck_->SetValue(false);
        itemCheckBoxAutoExeUserAck_->Enable(false);
        autoExecuteUserAck_ = false;
    }
    else
    {
        itemCheckBoxAutoExeUserAck_->Enable(true);
    }
}

void mmBDDialog::OnAdvanceChecked(wxCommandEvent& WXUNUSED(event))
{
    SetAdvancedTransferControls(cAdvanced_->IsChecked());
}

void mmBDDialog::SetTransferControls(bool transfers)
{
    wxStaticText* stp = static_cast<wxStaticText*>(FindWindow(ID_DIALOG_TRANS_STATIC_PAYEE));
    wxStaticText* stta = static_cast<wxStaticText*>(FindWindow(ID_DIALOG_TRANS_STATIC_TOACCOUNT));

    cAdvanced_->Enable(transfers);
    if (transfers)
    {
        SetSplitControls();
    }
    else
    {
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

void mmBDDialog::SetAdvancedTransferControls(bool advanced)
{
    m_advanced = advanced;
    toTextAmount_->Enable(m_advanced);
    mmToolTip(textAmount_, m_advanced ? amountTransferTip_ : _("Specify the transfer amount in the From Account"));
    if (m_advanced)
        toTextAmount_->SetValue(m_bill_data.TOTRANSAMOUNT);
    else
        toTextAmount_->ChangeValue("");
}

void mmBDDialog::setRepeatDetails()
{
    staticTextRepeats_->SetLabelText(_("Repeats"));

    int repeats = getRepeatType();
    if (repeats == Model_Billsdeposits::REPEAT_IN_X_DAYS || repeats == Model_Billsdeposits::REPEAT_EVERY_X_DAYS)
    {
        staticTimesRepeat_->SetLabelText(_("Period: Days"));
        const auto toolTipsStr = _("Specify period in Days.");
        mmToolTip(textNumRepeats_, toolTipsStr);
    }
    else if (repeats == Model_Billsdeposits::REPEAT_IN_X_MONTHS || repeats == Model_Billsdeposits::REPEAT_EVERY_X_MONTHS)
    {
        staticTimesRepeat_->SetLabelText(_("Period: Months"));
        const auto toolTipsStr = _("Specify period in Months.");
        mmToolTip(textNumRepeats_, toolTipsStr);
    }
    else if (repeats == Model_Billsdeposits::REPEAT_ONCE)
    {
        staticTimesRepeat_->SetLabelText(_("Payments Left"));
        const auto toolTipsStr = _("Ignored (leave blank).");
        mmToolTip(textNumRepeats_, toolTipsStr);
    }
    else
    {
        staticTimesRepeat_->SetLabelText(_("Payments Left"));
        const auto toolTipsStr = _("Specify the number of payments to be made.\n"
            "Leave blank if the payments continue forever.");
        mmToolTip(textNumRepeats_, toolTipsStr);
    }
}

void mmBDDialog::OnRepeatTypeChanged(wxCommandEvent& WXUNUSED(event))
{
    setRepeatDetails();
}

int mmBDDialog::getRepeatType()
{
    int repeatIndex = m_choice_repeat->GetSelection();
    return repeatIndex >= 0 ? BILLSDEPOSITS_REPEATS.at(repeatIndex).first : -1;
}

void mmBDDialog::setRepeatType(int repeatType)
{
    if (repeatType < 0)
    {
        wxFAIL;
        return;
    }

    // fast path
    int repeatIndex = repeatType;
    if (BILLSDEPOSITS_REPEATS.at(repeatIndex).first != repeatType)
    {
        // slow path: BILLSDEPOSITS_REPEATS is not sorted by REPEAT_TYPE
        // cache the mapping from type to index
        static std::vector<int> index;
        if (index.size() == 0) {
            wxLogDebug("mmBDDialog::setRepeatType : cache index");
            index.resize(BILLSDEPOSITS_REPEATS.size(), -1);
            for (size_t i = 0; i < BILLSDEPOSITS_REPEATS.size(); i++)
            {
                unsigned int j = BILLSDEPOSITS_REPEATS.at(i).first;
                if (j < BILLSDEPOSITS_REPEATS.size() && index.at(j) == -1)
                    index.at(j) = i;
                else
                    wxFAIL;
            }
        }

        repeatIndex = index.at(repeatType);
        if (repeatIndex == -1)
        {
            wxFAIL;
            repeatIndex = 0;
        }
    }

    m_choice_repeat->SetSelection(repeatIndex);
}

void mmBDDialog::OnsetPrevOrNextRepeatDate(wxCommandEvent& event)
{
    int repeatType = getRepeatType();
    wxString valueStr = textNumRepeats_->GetValue();
    int span = 1;
    bool goPrev = (event.GetId() == ID_DIALOG_TRANS_BUTTONTRANSNUMPREV);

    switch (repeatType)
    {
    case Model_Billsdeposits::REPEAT_IN_X_DAYS:
        wxFALLTHROUGH;
    case Model_Billsdeposits::REPEAT_IN_X_MONTHS:
        wxFALLTHROUGH;
    case Model_Billsdeposits::REPEAT_EVERY_X_DAYS:
        wxFALLTHROUGH;
    case Model_Billsdeposits::REPEAT_EVERY_X_MONTHS:
        span = wxAtoi(valueStr);
        if (!valueStr.IsNumber() || !span) {
            mmErrorDialogs::ToolTip4Object(textNumRepeats_, _("Invalid value"), _("Error"));
            break;
        }
        wxFALLTHROUGH;
    default:
        m_date_paid->SetValue(Model_Billsdeposits::nextOccurDate(repeatType, span, m_date_paid->GetValue(), goPrev));
        m_date_due->SetValue(Model_Billsdeposits::nextOccurDate(repeatType, span, m_date_due->GetValue(), goPrev));
    }
}

void mmBDDialog::activateSplitTransactionsDlg()
{
    if (m_bill_data.CATEGID > -1 && m_bill_data.local_splits.empty())
    {
        if (!textAmount_->GetDouble(m_bill_data.TRANSAMOUNT))
        {
            m_bill_data.TRANSAMOUNT = 0;
        }
        Split s;
        s.SPLITTRANSAMOUNT = m_bill_data.TRANSAMOUNT;
        s.CATEGID = m_bill_data.CATEGID;
        s.NOTES = m_bill_data.NOTES;
        m_bill_data.local_splits.push_back(s);
    }

    mmSplitTransactionDialog dlg(this, m_bill_data.local_splits, m_bill_data.ACCOUNTID, m_choice_transaction_type->GetSelection());
    if (dlg.ShowModal() == wxID_OK)
    {
        m_bill_data.local_splits = dlg.mmGetResult();
        m_bill_data.TRANSAMOUNT = Model_Splittransaction::get_total(m_bill_data.local_splits);
        m_bill_data.CATEGID = -1;
        if (m_choice_transaction_type->GetSelection() == Model_Checking::TYPE_ID_TRANSFER && m_bill_data.TRANSAMOUNT < 0)
        {
            m_bill_data.TRANSAMOUNT = -m_bill_data.TRANSAMOUNT;
        }
        textAmount_->SetValue(m_bill_data.TRANSAMOUNT);
    }

    if (m_bill_data.local_splits.size() == 1)
    {
        m_bill_data.CATEGID = m_bill_data.local_splits[0].CATEGID;
        textNotes_->SetValue(m_bill_data.local_splits[0].NOTES);
        m_bill_data.local_splits.clear();
    }

    setCategoryLabel();
}

void mmBDDialog::setTooltips()
{
    if (!this->m_bill_data.local_splits.empty())
    {
        const Model_Currency::Data* currency = Model_Currency::GetBaseCurrency();
        const Model_Account::Data* account = Model_Account::instance().get(m_bill_data.ACCOUNTID);
        if (account) {
            currency = Model_Account::currency(account);
        }

        bSplit_->SetToolTip(Model_Splittransaction::get_tooltip(m_bill_data.local_splits, currency));
    }
    else
        mmToolTip(bSplit_, _("Use split Categories"));
}

void mmBDDialog::setCategoryLabel()
{
    bool has_split = !m_bill_data.local_splits.empty();

    bSplit_->UnsetToolTip();
    if (has_split)
    {
        cbCategory_->SetLabelText(_("Split Transaction"));
        textAmount_->SetValue(Model_Splittransaction::get_total(m_bill_data.local_splits));
        m_bill_data.CATEGID = -1;
    }
    else if (m_transfer && m_new_bill
        && Option::instance().getTransCategoryTransferNone() == Option::LASTUSED)
    {
        Model_Checking::Data_Set transactions = Model_Checking::instance().find(
            Model_Checking::TRANSCODE(Model_Checking::TYPE_ID_TRANSFER, EQUAL)
            , Model_Checking::TRANSDATE(wxDateTime(23,59,59,999), LESS_OR_EQUAL));

        if (!transactions.empty())
        {
            const int64 cat = transactions.back().CATEGID;
            cbCategory_->ChangeValue(Model_Category::full_name(cat));
        }
    } else
    {
        const auto fullCategoryName = Model_Category::full_name(m_bill_data.CATEGID);
        cbCategory_->ChangeValue(fullCategoryName);
    }

    setTooltips();

    bool is_split = !m_bill_data.local_splits.empty();
    textAmount_->Enable(!is_split);
    bCalc_->Enable(!is_split);
    wxButton* bSplit = static_cast<wxBitmapButton*>(FindWindow(ID_DIALOG_TRANS_BUTTONSPLIT));
    bSplit->Enable(!m_transfer);
    cbCategory_->Enable(!is_split);
    Layout();
}

void mmBDDialog::OnMoreFields(wxCommandEvent& WXUNUSED(event))
{
    wxBitmapButton* button = static_cast<wxBitmapButton*>(FindWindow(ID_BTN_CUSTOMFIELDS));

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

void mmBDDialog::OnAccountUpdated(wxCommandEvent& WXUNUSED(event))
{
    int64 acc_id = cbAccount_->mmGetId();
    Model_Account::Data* account = Model_Account::instance().get(acc_id);
    if (account)
    {
        SetAmountCurrencies(acc_id, -1);
        if (textAmount_->Calculate())
        {
            textAmount_->GetDouble(m_bill_data.TRANSAMOUNT);
        }

        m_bill_data.ACCOUNTID = account->ACCOUNTID;
    }
}

void mmBDDialog::OnFocusChange(wxChildFocusEvent& event)
{
    switch (object_in_focus_)
    {
    case mmID_ACCOUNTNAME:
        cbAccount_->ChangeValue(cbAccount_->GetValue());
        if (cbAccount_->mmIsValid())
        {
            m_bill_data.ACCOUNTID = cbAccount_->mmGetId();
            SetAmountCurrencies(m_bill_data.ACCOUNTID, -1);
        }
        break;
    case mmID_TOACCOUNTNAME:
        cbToAccount_->ChangeValue(cbToAccount_->GetValue());
        if (cbToAccount_->mmIsValid())
        {
            m_bill_data.TOACCOUNTID = cbToAccount_->mmGetId();
            SetAmountCurrencies(-1, m_bill_data.TOACCOUNTID);
        }
        break;
    case mmID_PAYEE:
        cbPayee_->ChangeValue(cbPayee_->GetValue());
        break;
    case mmID_CATEGORY:
        cbCategory_->ChangeValue(cbCategory_->GetValue());
        if (cbCategory_->mmIsValid()) {
            m_bill_data.CATEGID = cbCategory_->mmGetCategoryId();
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
        textAmount_->GetDouble(m_bill_data.TRANSAMOUNT);
        textAmount_->SelectAll();
    }
    if (m_advanced && toTextAmount_->Calculate()) {
        toTextAmount_->GetDouble(m_bill_data.TOTRANSAMOUNT);
        toTextAmount_->SelectAll();
    }
}

void mmBDDialog::OnCalculator(wxCommandEvent& WXUNUSED(event))
{
    calcPopup_->SetTarget(calcTarget_);
    calcPopup_->Popup();
}
