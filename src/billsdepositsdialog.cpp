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
#include "mmTextCtrl.h"
#include "splittransactionsdialog.h"
#include "model/Model_Payee.h"
#include "model/Model_Attachment.h"
#include "model/Model_CurrencyHistory.h"
#include <wx/spinbutt.h>
#include <wx/valnum.h>

enum { NONE, WEEKLY, FORTNIGHTLY, MONTHLY
    , BIMONTHLY, QUARTERLY, HALFYEARLY
    , YEARLY, FOURMONTHS, FOURWEEKS
    , DAILY, INXDAYS, INXMONTHS
    , EVERYXDAYS, EVERYXMONTHS, MONTHLYLASTDAY, MONTHLYLASTBUSINESSDAY
};

const std::vector<std::pair<int, wxString> > mmBDDialog::BILLSDEPOSITS_REPEATS =
{
    { NONE, wxTRANSLATE("None")},
    { WEEKLY, wxTRANSLATE("Weekly")},
    { FORTNIGHTLY, wxTRANSLATE("Fortnightly")},
    { MONTHLY, wxTRANSLATE("Monthly")},
    { BIMONTHLY, wxTRANSLATE("Every 2 Months")},
    { QUARTERLY, wxTRANSLATE("Quarterly")},
    { HALFYEARLY, wxTRANSLATE("Half-Yearly")},
    { YEARLY, wxTRANSLATE("Yearly")},
    { FOURMONTHS, wxTRANSLATE("Four Months")},
    { FOURWEEKS, wxTRANSLATE("Four Weeks")},
    { DAILY, wxTRANSLATE("Daily")},
    { INXDAYS, wxTRANSLATE("In (x) Days")},
    { INXMONTHS, wxTRANSLATE("In (x) Months")},
    { EVERYXDAYS, wxTRANSLATE("Every (x) Days")},
    { EVERYXMONTHS, wxTRANSLATE("Every (x) Months")},
    { MONTHLYLASTDAY, wxTRANSLATE("Monthly (last day)")},
    { MONTHLYLASTBUSINESSDAY, wxTRANSLATE("Monthly (last business day)")}

};

wxIMPLEMENT_DYNAMIC_CLASS(mmBDDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmBDDialog, wxDialog)
    EVT_BUTTON(wxID_OK, mmBDDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, mmBDDialog::OnCancel)
    EVT_BUTTON(ID_DIALOG_TRANS_BUTTONCATEGS, mmBDDialog::OnCategs)
    EVT_BUTTON(ID_DIALOG_BD_COMBOBOX_ACCOUNTNAME, mmBDDialog::OnAccountName)
    EVT_BUTTON(ID_DIALOG_TRANS_BUTTONPAYEE, mmBDDialog::OnPayee)
    EVT_BUTTON(ID_DIALOG_TRANS_BUTTONTO, mmBDDialog::OnTo)
    EVT_BUTTON(wxID_FILE, mmBDDialog::OnAttachments)
    EVT_BUTTON(wxID_INFO, mmBDDialog::OnColourButton)
    EVT_BUTTON(ID_BTN_CUSTOMFIELDS, mmBDDialog::OnMoreFields)
    EVT_CHOICE(wxID_VIEW_DETAILS, mmBDDialog::OnTypeChanged)
    EVT_DATE_CHANGED(ID_DIALOG_TRANS_BUTTON_PAYDATE, mmBDDialog::OnPaidDateChanged)
    EVT_DATE_CHANGED(ID_DIALOG_BD_DUE_DATE, mmBDDialog::OnDueDateChanged)
    EVT_SPIN(ID_DIALOG_BD_REPEAT_DATE_SPINNER, mmBDDialog::OnSpinEventDue)
    EVT_SPIN(ID_DIALOG_TRANS_DATE_SPINNER, mmBDDialog::OnSpinEventPaid)
    EVT_CHECKBOX(ID_DIALOG_TRANS_ADVANCED_CHECKBOX, mmBDDialog::OnAdvanceChecked)
    EVT_CHECKBOX(ID_DIALOG_TRANS_SPLITCHECKBOX, mmBDDialog::OnSplitChecked)
    EVT_CHECKBOX(ID_DIALOG_BD_CHECKBOX_AUTO_EXECUTE_USERACK, mmBDDialog::OnAutoExecutionUserAckChecked)
    EVT_CHECKBOX(ID_DIALOG_BD_CHECKBOX_AUTO_EXECUTE_SILENT, mmBDDialog::OnAutoExecutionSilentChecked)
    EVT_CHOICE(ID_DIALOG_BD_COMBOBOX_REPEATS, mmBDDialog::OnRepeatTypeChanged)
    EVT_BUTTON(ID_DIALOG_TRANS_BUTTONTRANSNUMPREV, mmBDDialog::OnsetPrevOrNextRepeatDate)
    EVT_BUTTON(ID_DIALOG_TRANS_BUTTONTRANSNUM, mmBDDialog::OnsetPrevOrNextRepeatDate)
    EVT_MENU_RANGE(wxID_LOWEST, wxID_LOWEST + 20, mmBDDialog::OnNoteSelected)
    EVT_MENU_RANGE(wxID_HIGHEST, wxID_HIGHEST + 8, mmBDDialog::OnColourSelected)
    EVT_CLOSE(mmBDDialog::OnQuit)
wxEND_EVENT_TABLE()


mmBDDialog::mmBDDialog()
{
}

mmBDDialog::mmBDDialog(wxWindow* parent, int bdID, bool duplicate, bool enterOccur)
    : payeeUnknown_(true)
    , m_dup_bill(duplicate)
    , m_enter_occur(enterOccur)
    , autoExecuteUserAck_(false)
    , autoExecuteSilent_(false)
    , m_advanced(false)
    , categUpdated_(false)
    , prevType_(-1)
    , textNumber_(nullptr)
    , textAmount_(nullptr)
    , toTextAmount_(nullptr)
    , textNotes_(nullptr)
    , textCategory_(nullptr)
    , textNumRepeats_(nullptr)
    , bCategory_(nullptr)
    , bPayee_(nullptr)
    , bAccount_(nullptr)
    , bAttachments_(nullptr)
    , bColours_(nullptr)
    , cSplit_(nullptr)
    , cAdvanced_(nullptr)
    , m_choice_status(nullptr)
    , m_choice_transaction_type(nullptr)
    , m_date_paid(nullptr)
    , m_date_due(nullptr)
    , m_choice_repeat(nullptr)
    , itemCheckBoxAutoExeUserAck_(nullptr)
    , itemCheckBoxAutoExeSilent_(nullptr)
    , staticTimesRepeat_(nullptr)
    , staticTextRepeats_(nullptr)
    , m_btn_due_prev_date(nullptr)
    , m_btn_due_date(nullptr)
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
        m_bill_data.SUBCATEGID = bill->SUBCATEGID;
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
        //
        for (const auto& item : Model_Billsdeposits::splittransaction(bill)) {
            m_bill_data.local_splits.push_back({ item.CATEGID, item.SUBCATEGID, item.SPLITTRANSAMOUNT });
        }

        // If duplicate then we may need to copy the attachments
        if (m_dup_bill && Model_Infotable::instance().GetBoolInfo("ATTACHMENTSDUPLICATE", false))
        {
            const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::BILLSDEPOSIT);
            mmAttachmentManage::CloneAllAttachments(RefType, bdID, 0);
        }
    }

    m_transfer = (m_bill_data.TRANSCODE == Model_Billsdeposits::all_type()[Model_Billsdeposits::TRANSFER]);

    int ref_id = m_dup_bill ?  -bdID : (m_new_bill ? NULL : -m_bill_data.BDID);
    m_custom_fields = new mmCustomDataTransaction(this, ref_id, ID_CUSTOMFIELDS);

    Create(parent);
}

bool mmBDDialog::Create(wxWindow* parent, wxWindowID id, const wxString& caption
    , const wxPoint& pos, const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    CreateControls();
    dataToControls();

    //generate date change events for set weekday name
    wxDateEvent dateEventPaid(m_date_paid, m_date_paid->GetValue(), wxEVT_DATE_CHANGED);
    GetEventHandler()->ProcessEvent(dateEventPaid);
    wxDateEvent dateEventDue(m_date_due, m_date_due->GetValue(), wxEVT_DATE_CHANGED);
    GetEventHandler()->ProcessEvent(dateEventDue);

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    SetIcon(mmex::getProgramIcon());
    this->SetInitialSize();
    Centre();

    return TRUE;
}

void mmBDDialog::dataToControls()
{
    Model_Checking::getFrequentUsedNotes(frequentNotes_);
    wxButton* bFrequentUsedNotes = static_cast<wxButton*>(FindWindow(ID_DIALOG_TRANS_BUTTON_FREQENTNOTES));
    bFrequentUsedNotes->Enable(!frequentNotes_.empty());

    bColours_->SetBackgroundColour(getUDColour(m_bill_data.FOLLOWUPID));

    for (const auto& entry : BILLSDEPOSITS_REPEATS)
    {
        m_choice_repeat->Append(wxGetTranslation(entry.second));
    }
    m_choice_repeat->SetSelection(MONTHLY);

    for (const auto& i : Model_Billsdeposits::all_type())
    {
        if (i != Model_Billsdeposits::all_type()[Model_Billsdeposits::TRANSFER] || Model_Account::instance().all().size() > 1)
        {
            m_choice_transaction_type->Append(wxGetTranslation(i), new wxStringClientData(i));
        }
    }
    m_choice_transaction_type->SetSelection(0);

    SetTransferControls();  // hide appropriate fields

    resetPayeeString();

    if (!(!m_new_bill || m_enter_occur)) {
        return;
    }

    if (m_bill_data.PAYEEID > 0) {
        payeeUnknown_ = false;
    }

    m_choice_status->SetSelection(Model_Checking::status(m_bill_data.STATUS));
    if (m_bill_data.NUMOCCURRENCES > 0) {
        textNumRepeats_->SetValue(wxString::Format("%d", m_bill_data.NUMOCCURRENCES));
    }

    // Set the date paid
    wxDateTime field_date;
    field_date.ParseDate(m_bill_data.TRANSDATE);
    m_date_paid->SetValue(field_date);

    // Set the due Date
    field_date.ParseDate(m_bill_data.NEXTOCCURRENCEDATE);
    m_date_due->SetValue(field_date);

    // Have used repeatSel to multiplex auto repeat fields.
    switch (m_bill_data.REPEATS/BD_REPEATS_MULTIPLEX_BASE)
    {
        case 2:
            autoExecuteSilent_ = true;
            itemCheckBoxAutoExeSilent_->SetValue(true);
            /* intentionally fall through */
        case 1:
            autoExecuteUserAck_ = true;
            itemCheckBoxAutoExeUserAck_->SetValue(true);
            itemCheckBoxAutoExeSilent_->Enable(true);
    }
    m_bill_data.REPEATS %= BD_REPEATS_MULTIPLEX_BASE;

    if (m_bill_data.REPEATS == 0) {// if none
        textNumRepeats_->SetValue("");
    }

    if (m_bill_data.REPEATS < NONE || m_bill_data.REPEATS > MONTHLYLASTBUSINESSDAY) {
        wxFAIL;
        m_choice_repeat->SetSelection(MONTHLY);
    }
    else {
        m_choice_repeat->SetSelection(m_bill_data.REPEATS);
    }
    setRepeatDetails();

    m_choice_transaction_type->SetSelection(Model_Billsdeposits::type(m_bill_data.TRANSCODE));
    updateControlsForTransType();

    Model_Account::Data* account = Model_Account::instance().get(m_bill_data.ACCOUNTID);
    bAccount_->SetLabelText(account ? account->ACCOUNTNAME : "");

    setCategoryLabel();
    cSplit_->SetValue(!m_bill_data.local_splits.empty());

    textNotes_->SetValue(m_bill_data.NOTES);
    textNumber_->SetValue(m_bill_data.TRANSACTIONNUMBER);

    if (!m_bill_data.local_splits.empty())
        m_bill_data.TRANSAMOUNT = Model_Splittransaction::get_total(m_bill_data.local_splits);

    textAmount_->Enable(m_bill_data.local_splits.empty());
    textAmount_->SetValue(m_bill_data.TRANSAMOUNT, Model_Currency::precision(m_bill_data.ACCOUNTID));

    if (m_transfer)
    {
        m_bill_data.PAYEEID = -1;
        Model_Account::Data* to_account = Model_Account::instance().get(m_bill_data.TOACCOUNTID);
        if (to_account)
        {
            bPayee_->SetLabelText(to_account->ACCOUNTNAME);
            toTextAmount_->SetValue(m_bill_data.TOTRANSAMOUNT, to_account);
        }

        // When editing an advanced transaction record, we do not reset the m_bill_data.TOTRANSAMOUNT
        if ((!m_new_bill || m_enter_occur) && (m_bill_data.TOTRANSAMOUNT != m_bill_data.TRANSAMOUNT))
        {
            cAdvanced_->SetValue(true);
            SetAdvancedTransferControls(true);
        }
    }
    else
    {
        Model_Payee::Data* payee = Model_Payee::instance().get(m_bill_data.PAYEEID);
        if (payee)
            bPayee_->SetLabelText(payee->PAYEENAME);
    }

    if (!m_enter_occur)
    {
        if (m_dup_bill)
            SetDialogHeader(_("Duplicate Recurring Transaction"));
        else
            SetDialogHeader(_("Edit Recurring Transaction"));
    }
    else
    {
        SetDialogHeader(_("Enter Recurring Transaction"));
        m_date_due->Disable();
        itemStaticTextWeekDue_->Disable();
        wxSpinButton* spinTransDate = static_cast<wxSpinButton*>(FindWindow(ID_DIALOG_BD_REPEAT_DATE_SPINNER));
        if (spinTransDate) spinTransDate->Disable();
        m_choice_transaction_type->Disable();
        m_choice_repeat->Disable();
        textAmount_->SetFocus();
        itemCheckBoxAutoExeSilent_->Disable();
        itemCheckBoxAutoExeUserAck_->Disable();
        textNumRepeats_->Disable();
        m_btn_due_prev_date->Disable();
        m_btn_due_date->Disable();
    }

    setTooltips();
}

void mmBDDialog::SetDialogHeader(const wxString& header)
{
    this->SetTitle(header);
}

void mmBDDialog::SetDialogParameters(int trx_id)
{
    const auto split = Model_Splittransaction::instance().get_all();

    //const auto trx = Model_Checking::instance().find(Model_Checking::TRANSID(trx_id)).at(0);
    const auto trx = Model_Checking::instance().get(trx_id);
    Model_Checking::Full_Data t(*trx, split);
    m_bill_data.ACCOUNTID = t.ACCOUNTID;
    bAccount_->SetLabelText(t.ACCOUNTNAME);

    m_bill_data.TRANSCODE = t.TRANSCODE;
    m_choice_transaction_type->SetSelection(Model_Billsdeposits::type(t.TRANSCODE));
    m_transfer = (m_bill_data.TRANSCODE == Model_Billsdeposits::all_type()[Model_Billsdeposits::TRANSFER]);
    updateControlsForTransType();

    m_bill_data.TRANSAMOUNT = t.TRANSAMOUNT;
    textAmount_->SetValue(m_bill_data.TRANSAMOUNT);

    if (m_transfer)
    {
        m_bill_data.TOACCOUNTID = t.TOACCOUNTID;
        bPayee_->SetLabelText(t.TOACCOUNTNAME);

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
        bPayee_->SetLabelText(t.PAYEENAME);
    }

    if (t.has_split())
    {
        for (auto &split_trans : t.m_splits)
        {
            Split s;
            s.CATEGID = split_trans.CATEGID;
            s.SPLITTRANSAMOUNT = split_trans.SPLITTRANSAMOUNT;
            s.SUBCATEGID = split_trans.SUBCATEGID;
            m_bill_data.local_splits.push_back(s);
        }
        cSplit_->SetValue(true);
    }
    else
    {
        m_bill_data.CATEGID = t.CATEGID;
        m_bill_data.SUBCATEGID = t.SUBCATEGID;
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
    this->SetSizer(mainBoxSizerOuter);
    wxBoxSizer* mainBoxSizerInner = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* repeatTransBoxSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* custom_fields_box_sizer = new wxBoxSizer(wxVERTICAL);

    /**********************************************************************************************
     Determining where the controls go
    ***********************************************************************************************/
    //mainBoxSizerInner will align contents horizontally
    mainBoxSizerInner->Add(repeatTransBoxSizer, g_flagsV);
    //mainBoxSizerOuter will align contents vertically
    mainBoxSizerOuter->Add(mainBoxSizerInner, g_flagsV);

    /* Bills & Deposits Details */
    wxStaticBox* repeatDetailsStaticBox = new wxStaticBox(this, wxID_ANY, _("Recurring Transaction Details"));
    wxStaticBoxSizer* repeatDetailsStaticBoxSizer = new wxStaticBoxSizer(repeatDetailsStaticBox, wxHORIZONTAL);
    repeatTransBoxSizer->Add(repeatDetailsStaticBoxSizer, 0, wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    repeatDetailsStaticBoxSizer->Add(itemFlexGridSizer5);

    // Date Due --------------------------------------------

    m_date_due = new wxDatePickerCtrl(this, ID_DIALOG_BD_DUE_DATE, wxDefaultDateTime
        , wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    m_date_due->SetValue(wxDateTime::Now()); // Required for Mac: Does not default to today
    mmToolTip(m_date_due, _("Specify the date when this bill or deposit is due"));

    wxBoxSizer* dueDateDateBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    dueDateDateBoxSizer->Add(m_date_due, g_flagsH);

#ifdef __WXMSW__
    int interval = 0;
    int spinCtrlDirection = wxSP_VERTICAL;
    wxSpinButton* spinNextOccDate = new wxSpinButton(this, ID_DIALOG_BD_REPEAT_DATE_SPINNER
        , wxDefaultPosition, wxSize(-1, m_date_due->GetSize().GetHeight())
        , spinCtrlDirection | wxSP_ARROW_KEYS | wxSP_WRAP);
    mmToolTip(spinNextOccDate, _("Retard or advance the date of the 'next occurrence'"));
    spinNextOccDate->SetRange(-32768, 32768);
    dueDateDateBoxSizer->Add(spinNextOccDate, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT, interval);
#endif

    //Text field for name of day of the week
    wxSize WeekDayNameMaxSize(wxDefaultSize);
    for (wxDateTime::WeekDay d = wxDateTime::Sun;
            d != wxDateTime::Inv_WeekDay;
            d = wxDateTime::WeekDay(d+1))
        WeekDayNameMaxSize.IncTo(GetTextExtent(
            wxGetTranslation(wxDateTime::GetEnglishWeekDayName(d))+ " "));

    itemStaticTextWeekDue_ = new wxStaticText(this, wxID_STATIC, "", wxDefaultPosition, WeekDayNameMaxSize, wxST_NO_AUTORESIZE);
    dueDateDateBoxSizer->Add(itemStaticTextWeekDue_, g_flagsH);

    itemFlexGridSizer5->Add(new wxStaticText(this, wxID_STATIC, _("Date Due")), g_flagsH);
    itemFlexGridSizer5->Add(dueDateDateBoxSizer);

    // Repeats --------------------------------------------
    m_btn_due_prev_date = new wxBitmapButton(this, ID_DIALOG_TRANS_BUTTONTRANSNUMPREV
        , mmBitmap(png::LEFTARROW, mmBitmapButtonSize));
    mmToolTip(m_btn_due_prev_date, _("Back to the last occurring date with the specified values"));

    staticTextRepeats_ = new wxStaticText(this, wxID_STATIC, _("Repeats"));
    itemFlexGridSizer5->Add(staticTextRepeats_, g_flagsH);

    m_choice_repeat = new wxChoice(this, ID_DIALOG_BD_COMBOBOX_REPEATS);

    wxBoxSizer* repeatBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    m_btn_due_date = new wxBitmapButton(this, ID_DIALOG_TRANS_BUTTONTRANSNUM
        , mmBitmap(png::RIGHTARROW, mmBitmapButtonSize));
    mmToolTip(m_btn_due_date, _("Advance the next occurring date with the specified values"));
    repeatBoxSizer->Add(m_btn_due_prev_date, g_flagsH);
    repeatBoxSizer->Add(m_choice_repeat, wxSizerFlags(g_flagsExpand));
    repeatBoxSizer->Add(m_btn_due_date, g_flagsH);

    itemFlexGridSizer5->Add(repeatBoxSizer);

    // Repeat Times --------------------------------------------
    staticTimesRepeat_ = new wxStaticText(this, wxID_STATIC, _("Payments Left"));
    itemFlexGridSizer5->Add(staticTimesRepeat_, g_flagsH);

    wxBoxSizer* repeatTimesBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer5->Add(repeatTimesBoxSizer);

    textNumRepeats_ = new wxTextCtrl(this, ID_DIALOG_BD_TEXTCTRL_NUM_TIMES, ""
        , wxDefaultPosition, m_date_due->GetSize(), 0, wxIntegerValidator<int>());
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
    itemCheckBoxAutoExeSilent_->Disable();

    repeatTransBoxSizer->Add(itemCheckBoxAutoExeUserAck_, g_flagsExpand);
    repeatTransBoxSizer->Add(itemCheckBoxAutoExeSilent_, g_flagsExpand);

    /*************************************************************************************************************/

    wxStaticBox* transDetailsStaticBox = new wxStaticBox(this, wxID_REMOVE, _("Transaction Details"));
    wxStaticBoxSizer* transDetailsStaticBoxSizer = new wxStaticBoxSizer(transDetailsStaticBox, wxVERTICAL);
    wxFlexGridSizer* transPanelSizer = new wxFlexGridSizer(0, 2, 0, 0);
    transDetailsStaticBoxSizer->Add(transPanelSizer, g_flagsV);
    mainBoxSizerInner->Add(transDetailsStaticBoxSizer, g_flagsExpand);

    // Trans Date --------------------------------------------
    m_date_paid = new wxDatePickerCtrl(this, ID_DIALOG_TRANS_BUTTON_PAYDATE
        , wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    m_date_paid->SetValue(wxDateTime::Now()); // Required for Mac: Does not default to today
    mmToolTip(m_date_paid, _("Specify the date the user is requested to enter this transaction"));

    wxBoxSizer* transDateBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    transDateBoxSizer->Add(m_date_paid, g_flagsH);

#ifdef __WXMSW__
    wxSpinButton* spinTransDate = new wxSpinButton(this, ID_DIALOG_TRANS_DATE_SPINNER
        , wxDefaultPosition, wxSize(-1, m_date_paid->GetSize().GetHeight())
        , spinCtrlDirection | wxSP_ARROW_KEYS | wxSP_WRAP);
    mmToolTip(spinTransDate, _("Advance or retard the user request date of this transaction"));
    spinTransDate->SetRange(-32768, 32768);
    transDateBoxSizer->Add(spinTransDate, g_flagsH);
#endif

    itemStaticTextWeekPaid_ = new wxStaticText(this, wxID_STATIC, "", wxDefaultPosition, WeekDayNameMaxSize, wxST_NO_AUTORESIZE);
    transDateBoxSizer->Add(itemStaticTextWeekPaid_, g_flagsH);

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Date Paid")), g_flagsH);
    transPanelSizer->Add(transDateBoxSizer);

    // Status --------------------------------------------
    m_choice_status = new wxChoice(this, ID_DIALOG_TRANS_STATUS);

    for (const auto& i : Model_Billsdeposits::all_status())
    {
        m_choice_status->Append(wxGetTranslation(i), new wxStringClientData(i));
    }
    m_choice_status->SetSelection(Option::instance().TransStatusReconciled());
    mmToolTip(m_choice_status, _("Specify the status for the transaction"));

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Status")), g_flagsH);
    transPanelSizer->Add(m_choice_status, g_flagsH);

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

    // Amount Fields --------------------------------------------
    wxStaticText* amount_label = new wxStaticText(this, wxID_STATIC, _("Amount"));
    amount_label->SetFont(this->GetFont().Bold());

    textAmount_ = new mmTextCtrl(this, ID_DIALOG_TRANS_TEXTAMOUNT, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER
        , mmCalcValidator());
    mmToolTip(textAmount_, amountNormalTip_);
    textAmount_->Connect(ID_DIALOG_TRANS_TEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmBDDialog::OnTextEntered), nullptr, this);

    toTextAmount_ = new mmTextCtrl(this, ID_DIALOG_TRANS_TOTEXTAMOUNT, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER
        , mmCalcValidator());
    mmToolTip(toTextAmount_, _("Specify the transfer amount in the To Account"));
    toTextAmount_->Connect(ID_DIALOG_TRANS_TOTEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmBDDialog::OnTextEntered), nullptr, this);

    wxBoxSizer* amountSizer = new wxBoxSizer(wxHORIZONTAL);
    amountSizer->Add(textAmount_, g_flagsExpand);
    amountSizer->Add(toTextAmount_, g_flagsExpand);

    transPanelSizer->Add(amount_label, g_flagsH);
    transPanelSizer->Add(amountSizer);

    // Account ------------------------------------------------
    wxStaticText* acc_label = new wxStaticText(this, ID_DIALOG_TRANS_STATIC_ACCOUNT, _("Account"));
    acc_label->SetFont(this->GetFont().Bold());
    transPanelSizer->Add(acc_label, g_flagsH);
    bAccount_ = new wxButton(this, ID_DIALOG_BD_COMBOBOX_ACCOUNTNAME, _("Select Account"));
    mmToolTip(bAccount_, _("Specify the Account that will own the recurring transaction"));
    transPanelSizer->Add(bAccount_, g_flagsExpand);
    // Payee ------------------------------------------------
    wxStaticText* payee_label = new wxStaticText(this, ID_DIALOG_TRANS_STATIC_PAYEE, _("Payee"));
    payee_label->SetFont(this->GetFont().Bold());

    bPayee_ = new wxButton(this, ID_DIALOG_TRANS_BUTTONPAYEE, _("Select Payee"));
    mmToolTip(bPayee_, payeeWithdrawalTip_);

    transPanelSizer->Add(payee_label, g_flagsH);
    transPanelSizer->Add(bPayee_, g_flagsExpand);

    // Split Category -------------------------------------------
    cSplit_ = new wxCheckBox(this, ID_DIALOG_TRANS_SPLITCHECKBOX, _("Split")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cSplit_->SetValue(FALSE);
    mmToolTip(cSplit_, _("Use split Categories"));

    transPanelSizer->AddSpacer(20); // Fill empty space.
    transPanelSizer->Add(cSplit_, g_flagsH);

    // Category ---------------------------------------------
    wxStaticText* categ_label = new wxStaticText(this, wxID_STATIC, _("Category"));
    categ_label->SetFont(this->GetFont().Bold());
    bCategory_ = new wxButton(this, ID_DIALOG_TRANS_BUTTONCATEGS, _("Select Category")
        , wxDefaultPosition, wxDefaultSize, 0);
    //mmToolTip(bCategory_, _("Specify the category for this transaction"));

    transPanelSizer->Add(categ_label, g_flagsExpand);
    transPanelSizer->Add(bCategory_, g_flagsExpand);

    // Number ---------------------------------------------
    textNumber_ = new wxTextCtrl(this, ID_DIALOG_TRANS_TEXTNUMBER);
    mmToolTip(textNumber_, _("Specify any associated check number or transaction number"));

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Number")), g_flagsH);
    transPanelSizer->Add(textNumber_, g_flagsExpand);

    // Frequently Used Notes
    wxButton* bFrequentUsedNotes = new wxButton(this, ID_DIALOG_TRANS_BUTTON_FREQENTNOTES, "..."
        , wxDefaultPosition, wxSize(m_btn_due_prev_date->GetSize().GetX(), -1));
    mmToolTip(bFrequentUsedNotes, _("Select one of the frequently used notes"));
    bFrequentUsedNotes->Connect(ID_DIALOG_TRANS_BUTTON_FREQENTNOTES
        , wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(mmBDDialog::OnFrequentUsedNotes), nullptr, this);

    // Colours
    bColours_ = new wxButton(this, wxID_INFO, " ", wxDefaultPosition, bFrequentUsedNotes->GetSize());
    mmToolTip(bColours_, _("User Colors"));

    // Attachments
    bAttachments_ = new wxBitmapButton(this, wxID_FILE, mmBitmap(png::CLIP, mmBitmapButtonSize));
    mmToolTip(bAttachments_, _("Organize attachments of this recurring transaction"));

    // Now display the Frequntly Used Notes, Colour, Attachment buttons
    wxBoxSizer* notes_sizer = new wxBoxSizer(wxHORIZONTAL); 
    transPanelSizer->Add(notes_sizer);
    notes_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Notes")), g_flagsH);
    notes_sizer->Add(bFrequentUsedNotes, wxSizerFlags(g_flagsH));

    wxBoxSizer* RightAlign_sizer = new wxBoxSizer(wxHORIZONTAL);
    transPanelSizer->Add(RightAlign_sizer, wxSizerFlags(g_flagsH).Align(wxALIGN_RIGHT).Border(wxALL, 0));
    RightAlign_sizer->Add(bColours_, g_flagsH);
    RightAlign_sizer->Add(bAttachments_, g_flagsH);

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
    m_button_cancel->SetFocus();

    mainBoxSizerOuter->Add(buttonsPanel, wxSizerFlags(g_flagsV).Center().Border(wxALL, 0));
    wxBitmapButton* button_hide = new wxBitmapButton(buttonsPanel
        , ID_BTN_CUSTOMFIELDS, mmBitmap(png::RIGHTARROW, mmBitmapButtonSize));
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

    mainBoxSizerInner->Add(custom_fields_box_sizer, g_flagsExpand);

    Center();
}

void mmBDDialog::OnQuit(wxCloseEvent& WXUNUSED(event))
{
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::BILLSDEPOSIT);
    if (!m_bill_data.BDID)
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

    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::BILLSDEPOSIT);
    if (!m_bill_data.BDID)
        mmAttachmentManage::DeleteAllAttachments(RefType, m_bill_data.BDID);
    EndModal(wxID_CANCEL);
}

void mmBDDialog::OnAccountName(wxCommandEvent& WXUNUSED(event))
{
    const auto& accounts = Model_Account::instance().all_checking_account_names(true);
    mmSingleChoiceDialog scd(this
        , _("Choose Bank Account or Term Account")
        , _("Select Account")
        , accounts);

    if (!accounts.empty())
    {
        Model_Account::Data *acc = Model_Account::instance().get(m_bill_data.ACCOUNTID);
        const wxString acc_name = acc ? acc->ACCOUNTNAME : "";
        scd.SetSelection(accounts.Index(acc_name) != wxNOT_FOUND ? accounts.Index(acc_name) : 0);
    }

    if (scd.ShowModal() == wxID_OK)
    {
        wxString acctName = scd.GetStringSelection();
        Model_Account::Data* account = Model_Account::instance().get(acctName);
        if (account)
        {
            if (textAmount_->Calculate(Model_Currency::precision(account->ACCOUNTID)))
            {
                textAmount_->GetDouble(m_bill_data.TRANSAMOUNT);
            }

            m_bill_data.ACCOUNTID = account->ACCOUNTID;
            bAccount_->SetLabelText(acctName);
        }
    }
}

void mmBDDialog::OnPayee(wxCommandEvent& WXUNUSED(event))
{
    if (m_transfer)
    {
        m_bill_data.PAYEEID = -1;
        mmSingleChoiceDialog scd(this, _("Account name"), _("Select Account")
            , Model_Account::instance().all_checking_account_names(true));
        if (scd.ShowModal() == wxID_OK)
        {
            const wxString& acctName = scd.GetStringSelection();
            Model_Account::Data *to_acc = Model_Account::instance().get(acctName);
            if (to_acc) {
                m_bill_data.TOACCOUNTID = to_acc->id();
                bPayee_->SetLabelText(to_acc->ACCOUNTNAME);
            }
        }
    }
    else
    {
        m_bill_data.TOACCOUNTID = -1;
        mmPayeeDialog dlg(this, true);
        dlg.DisableTools();
        dlg.ShowModal();

        m_bill_data.PAYEEID = dlg.getPayeeId();
        Model_Payee::Data* payee = Model_Payee::instance().get(m_bill_data.PAYEEID);
        if (payee)
        {
            bPayee_->SetLabelText(payee->PAYEENAME);
            payeeUnknown_ = false;
            // Only for new/duplicate transactions: if user want to autofill last category used for payee.
            // If this is a Split Transaction, ignore displaying last category for payee
            if (payee->CATEGID != -1 && m_bill_data.local_splits.empty()
                && (Option::instance().TransCategorySelection() == Option::LASTUSED || 
                    Option::instance().TransCategorySelection() == Option::DEFAULT)
                && !categUpdated_ && m_bill_data.BDID == 0)
            {
                m_bill_data.CATEGID = payee->CATEGID;
                m_bill_data.SUBCATEGID = payee->SUBCATEGID;

                bCategory_->SetLabelText(Model_Category::full_name(m_bill_data.CATEGID, m_bill_data.SUBCATEGID));
            }
        }
        else
        {
            payeeUnknown_ = true;
            resetPayeeString();
        }
    }
}

void mmBDDialog::OnTo(wxCommandEvent& WXUNUSED(event))
{
    // This should only get called if we are in a transfer

    mmSingleChoiceDialog scd(this, _("Account name"), _("Select Account")
        , Model_Account::instance().all_checking_account_names());
    if (scd.ShowModal() == wxID_OK)
    {
        const wxString& acctName = scd.GetStringSelection();
        Model_Account::Data* account = Model_Account::instance().get(acctName);
        if (account) {
            m_bill_data.TOACCOUNTID = account->ACCOUNTID;
            bPayee_->SetLabelText(account->ACCOUNTNAME);
        }
    }
}

void mmBDDialog::OnCategs(wxCommandEvent& WXUNUSED(event))
{
    if (cSplit_->GetValue())
    {
        activateSplitTransactionsDlg();
    }
    else
    {
        mmCategDialog dlg(this, true, m_bill_data.CATEGID, m_bill_data.SUBCATEGID);
        if (dlg.ShowModal() == wxID_OK)
        {
            m_bill_data.CATEGID = dlg.getCategId();
            m_bill_data.SUBCATEGID = dlg.getSubCategId();

            categUpdated_ = true;
        }
    }
    setCategoryLabel();
}

void mmBDDialog::OnTypeChanged(wxCommandEvent& WXUNUSED(event))
{
    updateControlsForTransType();
}

void mmBDDialog::OnAttachments(wxCommandEvent& WXUNUSED(event))
{
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::BILLSDEPOSIT);
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
    case Model_Billsdeposits::TRANSFER:
    {
        m_transfer = true;
        mmToolTip(textAmount_, amountTransferTip_);
        accountLabel->SetLabelText(_("From"));
        if (m_bill_data.ACCOUNTID < 0)
        {
            bAccount_->SetLabelText(_("Select Account"));
            m_bill_data.PAYEEID = -1;
            payeeUnknown_ = true;
        }
        else
        {
            m_bill_data.PAYEEID = m_bill_data.ACCOUNTID;
        }

        stp->SetLabelText(_("To"));
        bPayee_->SetLabelText(_("Select To Account"));
        mmToolTip(bPayee_, payeeTransferTip_);
        if (prevType_ != -1)
        {
            m_bill_data.TOACCOUNTID = -1;
            payeeUnknown_ = true;
        }
        break;
    }
    case Model_Billsdeposits::WITHDRAWAL:
    {
        mmToolTip(textAmount_, amountNormalTip_);
        accountLabel->SetLabelText(_("Account"));
        stp->SetLabelText(_("Payee"));
        mmToolTip(bPayee_, payeeWithdrawalTip_);
        if (payeeUnknown_)
        {
            m_bill_data.PAYEEID = -1;
            m_bill_data.TOACCOUNTID = -1;
            resetPayeeString();
        }
        break;
    }
    case Model_Billsdeposits::DEPOSIT:
    {
        mmToolTip(textAmount_, amountNormalTip_);
        accountLabel->SetLabelText(_("Account"));
        stp->SetLabelText(_("From"));
        mmToolTip(bPayee_, payeeDepositTip_);
        if (payeeUnknown_)
        {
            m_bill_data.PAYEEID = -1;
            m_bill_data.TOACCOUNTID = -1;
            resetPayeeString();
        }
        break;
    }
    }

    SetTransferControls(m_transfer);

    if (cAdvanced_->IsChecked()) {
        SetAdvancedTransferControls(true);
    }

    prevType_ = m_choice_transaction_type->GetSelection();
}

void mmBDDialog::resetPayeeString()
{
    wxString payeeStr = _("Select Payee");
    if (m_bill_data.PAYEEID == -1)
    {
        const auto &filtd = Model_Payee::instance().FilterPayees("");
        if (filtd.size() == 1)
        {
            //only one payee present. Choose it
            payeeStr = filtd[0].PAYEENAME;
            m_bill_data.PAYEEID = filtd[0].PAYEEID;
            payeeUnknown_ = false;

            // Only for new/duplicate transactions: if user want to autofill last category used for payee.
            // If this is a Split Transaction, ignore displaying last category for payee
            if (filtd[0].CATEGID != -1
                && m_bill_data.local_splits.empty()
                && Option::instance().TransCategorySelection() == Option::LASTUSED
                && !categUpdated_
                && m_bill_data.BDID == 0)
            {
                m_bill_data.CATEGID = filtd[0].CATEGID;
                m_bill_data.SUBCATEGID = filtd[0].SUBCATEGID;

                const Model_Category::Data* category = Model_Category::instance().get(m_bill_data.CATEGID);
                const Model_Subcategory::Data* sub_category = (m_bill_data.SUBCATEGID != -1 ? Model_Subcategory::instance().get(m_bill_data.SUBCATEGID) : 0);
                bCategory_->SetLabelText(Model_Category::full_name(category, sub_category));
            }
        }
    }
    bPayee_->SetLabelText(payeeStr);
}

void mmBDDialog::OnFrequentUsedNotes(wxCommandEvent& WXUNUSED(event))
{
    wxMenu menu;
    int id = wxID_LOWEST;
    for (const auto& entry : frequentNotes_) {
        const wxString& label = entry.Mid(0, 30) + (entry.size() > 30 ? "..." : "");
        menu.Append(++id, label);

    }
    if (!frequentNotes_.empty())
        PopupMenu(&menu);
}

void mmBDDialog::OnNoteSelected(wxCommandEvent& event)
{
    int i = event.GetId() - wxID_LOWEST;
    if (i > 0 && static_cast<size_t>(i) <= frequentNotes_.size()) {
        if (!textNotes_->GetValue().EndsWith("\n"))
            textNotes_->AppendText("\n");
        textNotes_->AppendText(frequentNotes_[i - 1]);
    }
}

void mmBDDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    // Ideally 'paid date' should be on or before the 'due date'
    if (m_date_paid->GetValue() > m_date_due->GetValue())
        if (wxMessageBox(_("The payment date is after the due date, is this what you intended?"),
                _("Looks like a late payment"),
                wxYES_NO | wxNO_DEFAULT | wxICON_WARNING) != wxYES)
            return;

    Model_Account::Data* acc = Model_Account::instance().get(m_bill_data.ACCOUNTID);
    if (!acc)
    {
        return mmErrorDialogs::InvalidAccount(bAccount_, m_transfer, mmErrorDialogs::MESSAGE_POPUP_BOX);
    }

    Model_Billsdeposits::Data bill_data;
    bill_data.ACCOUNTID = m_bill_data.ACCOUNTID;
    bill_data.TRANSAMOUNT = m_bill_data.TRANSAMOUNT;
    bill_data.TRANSCODE = m_bill_data.TRANSCODE;

    Model_Billsdeposits::AccountBalance bal;

    if (!Model_Billsdeposits::instance().AllowTransaction(bill_data, bal)) return;
    if (!textAmount_->checkValue(m_bill_data.TRANSAMOUNT)) return;

    m_bill_data.TOTRANSAMOUNT = m_bill_data.TRANSAMOUNT;
    if (m_transfer)
    {
        Model_Account::Data *to_acc = Model_Account::instance().get(m_bill_data.TOACCOUNTID);
        if (!to_acc || to_acc->ACCOUNTID == acc->ACCOUNTID)
        {
            return mmErrorDialogs::InvalidAccount(bPayee_, true);
        }

        if (m_advanced && !toTextAmount_->checkValue(m_bill_data.TOTRANSAMOUNT)) return;
    }
    else
    {
        Model_Payee::Data *payee = Model_Payee::instance().get(m_bill_data.PAYEEID);
        if (!payee)
        {
            return mmErrorDialogs::InvalidPayee(bPayee_);
        }
    }

    if ((cSplit_->IsChecked() && m_bill_data.local_splits.empty())
        || (!cSplit_->IsChecked() && Model_Category::full_name(m_bill_data.CATEGID, m_bill_data.SUBCATEGID).empty()))
    {
        return mmErrorDialogs::InvalidCategory(bCategory_, false);
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

    // Multiplex Auto executable onto the repeat field of the database.
    m_bill_data.REPEATS = m_choice_repeat->GetSelection();
    if (autoExecuteUserAck_)
    {
        m_bill_data.REPEATS += BD_REPEATS_MULTIPLEX_BASE;
    }
    if (autoExecuteSilent_)
    {
        m_bill_data.REPEATS += BD_REPEATS_MULTIPLEX_BASE;
    }

    const wxString& numRepeatStr = textNumRepeats_->GetValue();
    m_bill_data.NUMOCCURRENCES = -1;

    if (!numRepeatStr.empty())
    {
        long cnt = 0;
        if (numRepeatStr.ToLong(&cnt))
        {
            wxASSERT(std::numeric_limits<int>::min() <= cnt);
            wxASSERT(cnt <= std::numeric_limits<int>::max());
            m_bill_data.NUMOCCURRENCES = static_cast<int>(cnt);
        }
    }

    m_bill_data.NEXTOCCURRENCEDATE = m_date_due->GetValue().FormatISODate();
    m_bill_data.TRANSDATE = m_date_paid->GetValue().FormatISODate();

    wxStringClientData* status_obj = static_cast<wxStringClientData *>(m_choice_status->GetClientObject(m_choice_status->GetSelection()));
    if (status_obj) {
        m_bill_data.STATUS = Model_Billsdeposits::toShortStatus(status_obj->GetData());
    }

    m_bill_data.TRANSACTIONNUMBER = textNumber_->GetValue();
    m_bill_data.NOTES = textNotes_->GetValue();

    if (!m_enter_occur)
    {
        Model_Billsdeposits::Data* bill = Model_Billsdeposits::instance().get(m_bill_data.BDID);
        if (m_new_bill || m_dup_bill)
        {
            bill = Model_Billsdeposits::instance().create();
        }

        bill->ACCOUNTID = m_bill_data.ACCOUNTID;
        bill->TOACCOUNTID = m_bill_data.TOACCOUNTID;
        bill->PAYEEID = m_bill_data.PAYEEID;
        bill->TRANSCODE = Model_Billsdeposits::all_type()[m_choice_transaction_type->GetSelection()];
        bill->TRANSAMOUNT = m_bill_data.TRANSAMOUNT;
        bill->STATUS = m_bill_data.STATUS;
        bill->TRANSACTIONNUMBER = m_bill_data.TRANSACTIONNUMBER;
        bill->NOTES = m_bill_data.NOTES;
        bill->CATEGID = m_bill_data.CATEGID;
        bill->SUBCATEGID = m_bill_data.SUBCATEGID;
        bill->TRANSDATE = m_bill_data.TRANSDATE;
        bill->TOTRANSAMOUNT = m_bill_data.TOTRANSAMOUNT;
        bill->REPEATS = m_bill_data.REPEATS;
        bill->NEXTOCCURRENCEDATE = m_bill_data.NEXTOCCURRENCEDATE;
        bill->NUMOCCURRENCES = m_bill_data.NUMOCCURRENCES;
        bill->FOLLOWUPID = m_bill_data.FOLLOWUPID;

        m_trans_id = Model_Billsdeposits::instance().save(bill);

        Model_Budgetsplittransaction::Data_Set splt;
        for (const auto& entry : m_bill_data.local_splits)
        {
            Model_Budgetsplittransaction::Data *s = Model_Budgetsplittransaction::instance().create();
            s->CATEGID = entry.CATEGID;
            s->SUBCATEGID = entry.SUBCATEGID;
            s->SPLITTRANSAMOUNT = entry.SPLITTRANSAMOUNT;
            splt.push_back(*s);
        }
        Model_Budgetsplittransaction::instance().update(splt, m_trans_id);

        const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::BILLSDEPOSIT);
        mmAttachmentManage::RelocateAllAttachments(RefType, 0, m_trans_id);

        //Custom Data
        m_custom_fields->SaveCustomValues(-m_trans_id);

    }
    else
    {
        // repeats now hold extra info. Need to get repeats from dialog selection
        if ((m_choice_repeat->GetSelection() < INXDAYS)
            || (m_choice_repeat->GetSelection() > EVERYXMONTHS)
            || (m_bill_data.REPEATS > NONE))
        {
            Model_Checking::Data* tran = Model_Checking::instance().create();
            tran->ACCOUNTID = m_bill_data.ACCOUNTID;
            tran->TOACCOUNTID = m_bill_data.TOACCOUNTID;
            tran->PAYEEID = m_bill_data.PAYEEID;
            tran->TRANSCODE = Model_Billsdeposits::all_type()[m_choice_transaction_type->GetSelection()];
            tran->TRANSAMOUNT = m_bill_data.TRANSAMOUNT;
            tran->STATUS = m_bill_data.STATUS;
            tran->TRANSACTIONNUMBER = m_bill_data.TRANSACTIONNUMBER;
            tran->NOTES = m_bill_data.NOTES;
            tran->CATEGID = m_bill_data.CATEGID;
            tran->SUBCATEGID = m_bill_data.SUBCATEGID;
            tran->TRANSDATE = m_bill_data.TRANSDATE;
            tran->TOTRANSAMOUNT = m_bill_data.TOTRANSAMOUNT;
            tran->FOLLOWUPID = m_bill_data.FOLLOWUPID;

            int trans_id = Model_Checking::instance().save(tran);

            Model_Splittransaction::Data_Set checking_splits;
            for (auto &item : m_bill_data.local_splits)
            {
                Model_Splittransaction::Data *split = Model_Splittransaction::instance().create();
                split->TRANSID = trans_id;
                split->CATEGID = item.CATEGID;
                split->SUBCATEGID = item.SUBCATEGID;
                split->SPLITTRANSAMOUNT = item.SPLITTRANSAMOUNT;
                checking_splits.push_back(*split);
            }
            Model_Splittransaction::instance().update(checking_splits, trans_id);

            //Custom Data
            m_custom_fields->SaveCustomValues(trans_id);

        }
        Model_Billsdeposits::instance().completeBDInSeries(m_bill_data.BDID);
    }

    EndModal(wxID_OK);
}

void mmBDDialog::OnSplitChecked(wxCommandEvent& WXUNUSED(event))
{
    if (cSplit_->IsChecked())
    {
        activateSplitTransactionsDlg();
    }
    else
    {
        wxASSERT(m_bill_data.local_splits.size() > 0);
        if (m_bill_data.local_splits.size() > 1)
        {
            //Delete split items first (data protection)
            cSplit_->SetValue(true);
        }
        else if (m_bill_data.local_splits.size() == 1)
        {
            m_bill_data.CATEGID = m_bill_data.local_splits.begin()->CATEGID;
            m_bill_data.SUBCATEGID = m_bill_data.local_splits.begin()->SUBCATEGID;
            m_bill_data.TRANSAMOUNT = m_bill_data.local_splits.begin()->SPLITTRANSAMOUNT;

            if (m_bill_data.TRANSAMOUNT < 0)
            {
                m_bill_data.TRANSAMOUNT = -m_bill_data.TRANSAMOUNT;
                m_choice_transaction_type->SetSelection(Model_Checking::WITHDRAWAL);
            }

            m_bill_data.local_splits.clear();
        }
    }
    textAmount_->Enable(m_bill_data.local_splits.empty());
    setCategoryLabel();
}

void mmBDDialog::SetSplitControls(bool split)
{
    textAmount_->Enable(!split);
    if (split)
    {
        m_bill_data.TRANSAMOUNT = Model_Splittransaction::get_total(m_bill_data.local_splits);
        m_bill_data.CATEGID = -1;
        m_bill_data.SUBCATEGID = -1;
    }
    else
    {
        textAmount_->Clear();
        m_bill_data.local_splits.clear();
    }
    setCategoryLabel();
}

void mmBDDialog::OnAutoExecutionUserAckChecked(wxCommandEvent& WXUNUSED(event))
{
    autoExecuteUserAck_ = !autoExecuteUserAck_;
    if (autoExecuteUserAck_)
    {
        itemCheckBoxAutoExeSilent_->Enable(true);
    }
    else
    {
        itemCheckBoxAutoExeSilent_->SetValue(false);
        itemCheckBoxAutoExeSilent_->Enable(false);
        autoExecuteSilent_ = false;
    }
}

void mmBDDialog::OnAutoExecutionSilentChecked(wxCommandEvent& WXUNUSED(event))
{
    autoExecuteSilent_ = !autoExecuteSilent_;
}

void mmBDDialog::OnAdvanceChecked(wxCommandEvent& WXUNUSED(event))
{
    SetAdvancedTransferControls(cAdvanced_->IsChecked());
}

void mmBDDialog::SetTransferControls(bool transfers)
{
    cAdvanced_->Enable(transfers);
    if (transfers)
    {
        if (prevType_ != Model_Billsdeposits::TRANSFER)
        {
            if (cSplit_->GetValue())
            {
                cSplit_->SetValue(FALSE);
                SetSplitControls();
            }
            cSplit_->Disable();
        }
    }
    else
    {
        if (!(prevType_ == Model_Billsdeposits::WITHDRAWAL || prevType_ == Model_Billsdeposits::DEPOSIT))
        {
            SetAdvancedTransferControls();
            cSplit_->Enable();
            toTextAmount_->ChangeValue("");
            cAdvanced_->SetValue(false);
        }
    }
}

void mmBDDialog::SetAdvancedTransferControls(bool advanced)
{
    m_advanced = advanced;
    toTextAmount_->Enable(m_advanced);
    mmToolTip(textAmount_, m_advanced ? amountTransferTip_ : _("Specify the transfer amount in the From Account"));
    if (m_advanced)
    {
        // Display the transfer amount in the toTextAmount control.
        if (m_bill_data.TOTRANSAMOUNT > 0)
        {
            toTextAmount_->SetValue(m_bill_data.TOTRANSAMOUNT);
        }
        else
        {
            toTextAmount_->SetValue(textAmount_->GetValue());
        }
    }
}

void mmBDDialog::OnSpinEventPaid(wxSpinEvent& event)
{
    wxSpinButton* spinCtrl = static_cast<wxSpinButton*>(event.GetEventObject());
    spinCtrl->SetValue(0);
    int step = event.GetValue();
    wxDateTime date_paid = m_date_paid->GetValue().Add(wxDateSpan::Days(step));
    m_date_paid->SetValue(date_paid);
    wxDateEvent dateEvent(m_date_paid, date_paid, wxEVT_DATE_CHANGED);
    GetEventHandler()->ProcessEvent(dateEvent);
    event.Skip();
}

void mmBDDialog::OnSpinEventDue(wxSpinEvent& event)
{
    wxSpinButton* spinCtrl = static_cast<wxSpinButton*>(event.GetEventObject());
    spinCtrl->SetValue(0);
    int step = event.GetValue();
    wxDateTime date_due = m_date_due->GetValue().Add(wxDateSpan::Days(step));
    m_date_due->SetValue(date_due);
    wxDateEvent dateEvent(m_date_due, date_due, wxEVT_DATE_CHANGED);
    GetEventHandler()->ProcessEvent(dateEvent);
    event.Skip();
}

void mmBDDialog::setRepeatDetails()
{
    const wxString& repeatLabelRepeats = _("Repeats");
    const wxString& repeatLabelActivate = _("Activates");

    const wxString& timeLabelDays = _("Period: Days");
    const wxString& timeLabelMonths = _("Period: Months");

    int repeats = m_choice_repeat->GetSelection();
    if (repeats == INXDAYS)
    {
        staticTextRepeats_->SetLabelText(repeatLabelActivate);
        staticTimesRepeat_->SetLabelText(timeLabelDays);
        const auto& toolTipsStr = _("Specify period in Days to activate.\n"
            "Becomes blank when not active.");
        mmToolTip(textNumRepeats_, toolTipsStr);
    }
    else if (repeats == INXMONTHS)
    {
        staticTextRepeats_->SetLabelText(repeatLabelActivate);
        staticTimesRepeat_->SetLabelText(timeLabelMonths);
        const auto& toolTipsStr = _("Specify period in Months to activate.\n"
            "Becomes blank when not active.");
        mmToolTip(textNumRepeats_, toolTipsStr);
    }
    else if (repeats == EVERYXDAYS)
    {
        staticTextRepeats_->SetLabelText(repeatLabelRepeats);
        staticTimesRepeat_->SetLabelText(timeLabelDays);
        const auto& toolTipsStr = _("Specify period in Days to activate.\n"
            "Leave blank when not active.");
        mmToolTip(textNumRepeats_, toolTipsStr);
    }
    else if (repeats == EVERYXMONTHS)
    {
        staticTextRepeats_->SetLabelText(repeatLabelRepeats);
        staticTimesRepeat_->SetLabelText(timeLabelMonths);
        const auto& toolTipsStr = _("Specify period in Months to activate.\n"
            "Leave blank when not active.");
        mmToolTip(textNumRepeats_, toolTipsStr);
    }
    else
    {
        staticTextRepeats_->SetLabelText(repeatLabelRepeats);
        staticTimesRepeat_->SetLabelText(_("Payments Left"));
        const auto& toolTipsStr = _("Specify the number of payments to be made.\n"
            "Leave blank if the payments continue forever.");
        mmToolTip(textNumRepeats_, toolTipsStr);
    }
}

void mmBDDialog::OnRepeatTypeChanged(wxCommandEvent& WXUNUSED(event))
{
    setRepeatDetails();
}

void mmBDDialog::OnsetPrevOrNextRepeatDate(wxCommandEvent& event)
{
    int repeatType = m_choice_repeat->GetSelection();
    wxString valueStr = textNumRepeats_->GetValue();
    int span = 1;
    bool goPrev = (event.GetId() == ID_DIALOG_TRANS_BUTTONTRANSNUMPREV);
    switch (repeatType)
    {
        case INXDAYS:
        case INXMONTHS:
        case EVERYXDAYS:
        case EVERYXMONTHS:
            span = wxAtoi(valueStr);
            if (!valueStr.IsNumber() || !span) {
                mmErrorDialogs::ToolTip4Object(textNumRepeats_, _("Invalid value"), _("Error"));
                break;
            }
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
        s.SUBCATEGID = m_bill_data.SUBCATEGID;
        m_bill_data.local_splits.push_back(s);
    }

    SplitTransactionDialog dlg(this, m_bill_data.local_splits
        , m_choice_transaction_type->GetSelection(), m_bill_data.ACCOUNTID);
    if (dlg.ShowModal() == wxID_OK)
    {
        m_bill_data.local_splits = dlg.getResult();
        m_bill_data.TRANSAMOUNT = Model_Splittransaction::get_total(m_bill_data.local_splits);
        m_bill_data.CATEGID = -1;
        m_bill_data.SUBCATEGID = -1;
        if (m_choice_transaction_type->GetSelection() == Model_Billsdeposits::TRANSFER && m_bill_data.TRANSAMOUNT < 0)
        {
            m_bill_data.TRANSAMOUNT = -m_bill_data.TRANSAMOUNT;
        }
        textAmount_->SetValue(m_bill_data.TRANSAMOUNT);
    }
    textAmount_->Enable(m_bill_data.local_splits.empty());
    setCategoryLabel();
}

void mmBDDialog::OnTextEntered(wxCommandEvent& event)
{
    if (event.GetId() == textAmount_->GetId())
    {
        textAmount_->Calculate(Model_Currency::precision(m_bill_data.ACCOUNTID));
    }
    else if (event.GetId() == toTextAmount_->GetId())
    {
        toTextAmount_->Calculate(Model_Currency::precision(m_bill_data.TOACCOUNTID));
    }
}

void mmBDDialog::setTooltips()
{
    if (this->m_bill_data.local_splits.empty())
    {
        mmToolTip(bCategory_, _("Specify the category for this transaction"));
    }
    else
    {
        const Model_Currency::Data* currency = Model_Currency::GetBaseCurrency();
        const Model_Account::Data* account = Model_Account::instance().get(m_bill_data.ACCOUNTID);
        if (account)
        {
            currency = Model_Account::currency(account);
        }

        mmToolTip(bCategory_, Model_Splittransaction::get_tooltip(m_bill_data.local_splits, currency));
    }
}

void mmBDDialog::setCategoryLabel()
{
    bool has_split = !m_bill_data.local_splits.empty();
    wxString fullCategoryName;
    bCategory_->UnsetToolTip();
    if (has_split)
    {
        fullCategoryName = _("Categories");
        textAmount_->SetValue(Model_Splittransaction::get_total(m_bill_data.local_splits));
        m_bill_data.CATEGID = -1;
        m_bill_data.SUBCATEGID = -1;
    }
    else
    {
        fullCategoryName = Model_Category::full_name(m_bill_data.CATEGID, m_bill_data.SUBCATEGID);
        if (fullCategoryName.IsEmpty())
        {
            fullCategoryName = _("Select Category");
        }
    }

    bCategory_->SetLabelText(fullCategoryName);
    setTooltips();
}

void mmBDDialog::OnPaidDateChanged(wxDateEvent& event)
{
    //get weekday name
    wxDateTime date = event.GetDate();
    if (date.IsValid())
        itemStaticTextWeekPaid_->SetLabelText(wxGetTranslation(date.GetEnglishWeekDayName(date.GetWeekDay())));
}

void mmBDDialog::OnDueDateChanged(wxDateEvent& event)
{
    //get weekday name
    wxDateTime date = event.GetDate();
    if (date.IsValid())
        itemStaticTextWeekDue_->SetLabelText(wxGetTranslation(date.GetEnglishWeekDayName(date.GetWeekDay())));
}

void mmBDDialog::OnColourButton(wxCommandEvent& /*event*/)
{
    wxCommandEvent ev(wxEVT_COMMAND_MENU_SELECTED, wxID_INFO);
    ev.SetEventObject(this);

    wxSharedPtr<wxMenu> mainMenu(new wxMenu);

    wxMenuItem* menuItem = new wxMenuItem(mainMenu.get(), wxID_HIGHEST, wxString::Format(_("Clear color"), 0));
    mainMenu->Append(menuItem);

    for (int i = 1; i <= 7; ++i)
    {
        menuItem = new wxMenuItem(mainMenu.get(), wxID_HIGHEST + i, wxString::Format(_("Color #%i"), i));
#ifdef __WXMSW__
        menuItem->SetBackgroundColour(getUDColour(i)); //only available for the wxMSW port.
#endif
        wxBitmap bitmap(mmBitmap(png::EMPTY, mmBitmapButtonSize).GetSize());
        wxMemoryDC memoryDC(bitmap);
        wxRect rect(memoryDC.GetSize());

        memoryDC.SetBackground(wxBrush(getUDColour(i)));
        memoryDC.Clear();
        memoryDC.DrawBitmap(mmBitmap(png::EMPTY, mmBitmapButtonSize), 0, 0, true);
        memoryDC.SelectObject(wxNullBitmap);
        menuItem->SetBitmap(bitmap);

        mainMenu->Append(menuItem);
    }

    PopupMenu(mainMenu.get());
}

void mmBDDialog::OnColourSelected(wxCommandEvent& event)
{
    int selected_nemu_item = event.GetId() - wxID_HIGHEST;
    bColours_->SetBackgroundColour(getUDColour(selected_nemu_item));
    m_bill_data.FOLLOWUPID = selected_nemu_item;
}

void mmBDDialog::OnMoreFields(wxCommandEvent& WXUNUSED(event))
{
    wxBitmapButton* button = static_cast<wxBitmapButton*>(FindWindow(ID_BTN_CUSTOMFIELDS));

    if (button)
        button->SetBitmap(mmBitmap(m_custom_fields->IsCustomPanelShown() ? png::RIGHTARROW : png::LEFTARROW, mmBitmapButtonSize));

    m_custom_fields->ShowHideCustomPanel();

    this->SetMinSize(wxSize(0, 0));
    this->Fit();
}
