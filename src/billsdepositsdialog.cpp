/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2016 - 2020 Nikolay Akimov
 Copyright (C) 2016 Stefano Giorgio

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
    { BIMONTHLY, wxTRANSLATE("Bi-Monthly")},
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
    EVT_BUTTON(ID_DIALOG_TRANS_BUTTONTRANSNUM, mmBDDialog::OnsetNextRepeatDate)
    EVT_MENU(wxID_ANY, mmBDDialog::onNoteSelected)
    EVT_CLOSE(mmBDDialog::OnQuit)
wxEND_EVENT_TABLE()


mmBDDialog::mmBDDialog()
{
}

mmBDDialog::mmBDDialog(wxWindow* parent, int bdID, bool edit, bool enterOccur)
    : payeeUnknown_(true)
    , m_new_bill(!edit)
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
    , m_btn_due_date(nullptr)
{
    const Model_Billsdeposits::Data* bill = Model_Billsdeposits::instance().get(bdID);
    m_new_bill = bill ? false : true;

    if (!m_new_bill)
    {
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
        //
        for (const auto& item : Model_Billsdeposits::splittransaction(bill)) {
            m_bill_data.local_splits.push_back({ item.CATEGID, item.SUBCATEGID, item.SPLITTRANSAMOUNT });
        }
    }

    m_transfer = (m_bill_data.TRANSCODE == Model_Billsdeposits::all_type()[Model_Billsdeposits::TRANSFER]);

    //---------------------------------------------------
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY
        , _("New Recurring Transaction")
        , wxDefaultPosition, wxDefaultSize, style);

}

bool mmBDDialog::Create(wxWindow* parent, wxWindowID id, const wxString& caption
    , const wxPoint& pos, const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    CreateControls();
    dataToControls();
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
    field_date.ParseDate(m_bill_data.NEXTOCCURRENCEDATE);
    m_date_paid->SetValue(field_date);

    // Set the due Date
    field_date.ParseDate(m_bill_data.TRANSDATE);
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
        SetDialogHeader(_("Edit Recurring Transaction"));
    }
    else
    {
        SetDialogHeader(_("Enter Recurring Transaction"));
        m_date_paid->Disable();
        wxSpinButton* spinTransDate = static_cast<wxSpinButton*>(FindWindow(ID_DIALOG_TRANS_DATE_SPINNER));
        if (spinTransDate) spinTransDate->Disable();
        m_choice_transaction_type->Disable();
        m_choice_repeat->Disable();
        textAmount_->SetFocus();
        itemCheckBoxAutoExeSilent_->Disable();
        itemCheckBoxAutoExeUserAck_->Disable();
        textNumRepeats_->Disable();
        m_btn_due_date->Disable();
    }

    setTooltips();
}

void mmBDDialog::SetDialogHeader(const wxString& header)
{
    this->SetTitle(header);
}

void mmBDDialog::SetDialogParameters(const Model_Checking::Full_Data& transaction)
{
    m_bill_data.ACCOUNTID = transaction.ACCOUNTID;
    bAccount_->SetLabelText(transaction.ACCOUNTNAME);

    m_bill_data.TRANSCODE = transaction.TRANSCODE;
    m_choice_transaction_type->SetSelection(Model_Billsdeposits::type(transaction.TRANSCODE));
    m_transfer = (m_bill_data.TRANSCODE == Model_Billsdeposits::all_type()[Model_Billsdeposits::TRANSFER]);
    updateControlsForTransType();

    m_bill_data.TRANSAMOUNT = transaction.TRANSAMOUNT;
    textAmount_->SetValue(m_bill_data.TRANSAMOUNT);

    if (m_transfer)
    {
        m_bill_data.TOACCOUNTID = transaction.TOACCOUNTID;
        bPayee_->SetLabelText(transaction.TOACCOUNTNAME);

        m_bill_data.TOTRANSAMOUNT = transaction.TOTRANSAMOUNT;
        toTextAmount_->SetValue(m_bill_data.TOTRANSAMOUNT);
        if (m_bill_data.TOTRANSAMOUNT != m_bill_data.TRANSAMOUNT)
        {
            cAdvanced_->SetValue(true);
            SetAdvancedTransferControls(true);
        }
    }
    else
    {
        m_bill_data.PAYEEID = transaction.PAYEEID;
        bPayee_->SetLabelText(transaction.PAYEENAME);
    }

    if (transaction.has_split())
    {
        Model_Splittransaction::Data_Set bill_splits;
        for (auto &split_trans : transaction.m_splits)
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
        m_bill_data.CATEGID = transaction.CATEGID;
        m_bill_data.SUBCATEGID = transaction.SUBCATEGID;
    }

    m_bill_data.TRANSACTIONNUMBER = transaction.TRANSACTIONNUMBER;
    m_bill_data.NOTES = transaction.NOTES;
    setCategoryLabel();
}

void mmBDDialog::CreateControls()
{
    wxBoxSizer* mainBoxSizerOuter = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(mainBoxSizerOuter);
    wxBoxSizer* repeatTransBoxSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* mainBoxSizerInner = new wxBoxSizer(wxHORIZONTAL);

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

    m_date_paid = new wxDatePickerCtrl(this, ID_DIALOG_TRANS_BUTTON_PAYDATE
        , wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    m_date_paid->SetValue(wxDateTime::Now()); // Required for Mac: Does not default to today
    m_date_paid->SetToolTip(_("Specify the date the user is requested to enter this transaction"));

    wxBoxSizer* dueDateDateBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    dueDateDateBoxSizer->Add(m_date_paid, g_flagsH);

#ifdef __WXMSW__
    int spinCtrlDirection = wxSP_VERTICAL;
    wxSpinButton* spinTransDate = new wxSpinButton(this, ID_DIALOG_TRANS_DATE_SPINNER
        , wxDefaultPosition, wxSize(-1, m_date_paid->GetSize().GetHeight())
        , spinCtrlDirection | wxSP_ARROW_KEYS | wxSP_WRAP);
    spinTransDate->SetToolTip(_("Advance or retard the user request date of this transaction"));
    spinTransDate->SetRange(-32768, 32768);
    dueDateDateBoxSizer->Add(spinTransDate, g_flagsH);
#endif

    itemFlexGridSizer5->Add(new wxStaticText(this, wxID_STATIC, _("Date Paid")), g_flagsH);
    itemFlexGridSizer5->Add(dueDateDateBoxSizer);

    // Repeats --------------------------------------------
    staticTextRepeats_ = new wxStaticText(this, wxID_STATIC, _("Repeats"));
    itemFlexGridSizer5->Add(staticTextRepeats_, g_flagsH);

    m_choice_repeat = new wxChoice(this, ID_DIALOG_BD_COMBOBOX_REPEATS);

    wxBoxSizer* repeatBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    m_btn_due_date = new wxBitmapButton(this, ID_DIALOG_TRANS_BUTTONTRANSNUM, mmBitmap(png::RIGHTARROW));
    m_btn_due_date->SetToolTip(_("Advance the next occurring date with the specified values"));
    repeatBoxSizer->Add(m_choice_repeat, wxSizerFlags(g_flagsExpand).Proportion(6));
    repeatBoxSizer->Add(m_btn_due_date, g_flagsExpand);

    itemFlexGridSizer5->Add(repeatBoxSizer);

    // Repeat Times --------------------------------------------
    staticTimesRepeat_ = new wxStaticText(this, wxID_STATIC, _("Payments Left"));
    itemFlexGridSizer5->Add(staticTimesRepeat_, g_flagsH);

    wxBoxSizer* repeatTimesBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer5->Add(repeatTimesBoxSizer);

    textNumRepeats_ = new wxTextCtrl(this, ID_DIALOG_BD_TEXTCTRL_NUM_TIMES, ""
        , wxDefaultPosition, m_date_paid->GetSize(), 0, wxIntegerValidator<int>());
    repeatTimesBoxSizer->Add(textNumRepeats_, g_flagsH);
    textNumRepeats_->SetMaxLength(12);
    setRepeatDetails();

    /* Auto Execution Status */
    itemCheckBoxAutoExeUserAck_ = new wxCheckBox(this, ID_DIALOG_BD_CHECKBOX_AUTO_EXECUTE_USERACK
       ,  _("Request user to enter payment"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemCheckBoxAutoExeUserAck_->SetToolTip(_("User requested to enter this transaction on the 'Date Paid'"));

    itemCheckBoxAutoExeSilent_ = new wxCheckBox(this, ID_DIALOG_BD_CHECKBOX_AUTO_EXECUTE_SILENT
        , _("Grant automatic execute"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemCheckBoxAutoExeSilent_->SetToolTip(_("The requested payment will occur without user interaction"));
    itemCheckBoxAutoExeSilent_->Disable();

    repeatTransBoxSizer->Add(itemCheckBoxAutoExeUserAck_, g_flagsExpand);
    repeatTransBoxSizer->Add(itemCheckBoxAutoExeSilent_, g_flagsExpand);

    /**********************************************************************************************
    Button Panel with OK and Cancel Buttons
    ***********************************************************************************************/
    wxPanel* buttonsPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    wxBoxSizer* buttonsPanelSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonsPanel->SetSizer(buttonsPanelSizer);

    wxButton* okButton = new wxButton(buttonsPanel, wxID_OK, _("&OK "));
    buttonsPanelSizer->Add(okButton, g_flagsH);

    wxButton* cancelButton = new wxButton(buttonsPanel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    buttonsPanelSizer->Add(cancelButton, g_flagsH);
    cancelButton->SetFocus();

    mainBoxSizerOuter->Add(buttonsPanel, wxSizerFlags(g_flagsV).Center().Border(wxALL, 0));

    /*************************************************************************************************************/

    wxStaticBox* transDetailsStaticBox = new wxStaticBox(this, wxID_REMOVE, _("Transaction Details"));
    wxStaticBoxSizer* transDetailsStaticBoxSizer = new wxStaticBoxSizer(transDetailsStaticBox, wxVERTICAL);
    wxFlexGridSizer* transPanelSizer = new wxFlexGridSizer(0, 2, 0, 0);
    transDetailsStaticBoxSizer->Add(transPanelSizer, g_flagsV);
    mainBoxSizerInner->Add(transDetailsStaticBoxSizer, g_flagsExpand);

    // Trans Date --------------------------------------------
    m_date_due = new wxDatePickerCtrl(this, ID_DIALOG_BD_DUE_DATE, wxDefaultDateTime
        , wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    m_date_due->SetValue(wxDateTime::Now()); // Required for Mac: Does not default to today
    m_date_due->SetToolTip(_("Specify the date when this bill or deposit is due"));

    wxBoxSizer* transDateBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    transDateBoxSizer->Add(m_date_due, g_flagsH);

#ifdef __WXMSW__
    int interval = 0;
    wxSpinButton* spinNextOccDate = new wxSpinButton(this, ID_DIALOG_BD_REPEAT_DATE_SPINNER
        , wxDefaultPosition, wxSize(-1, m_date_due->GetSize().GetHeight())
        , spinCtrlDirection | wxSP_ARROW_KEYS | wxSP_WRAP);
    spinNextOccDate->SetToolTip(_("Retard or advance the date of the 'next occurrence'"));
    spinNextOccDate->SetRange(-32768, 32768);
    transDateBoxSizer->Add(spinNextOccDate, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT, interval);
#endif

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Date Due")), g_flagsH);
    transPanelSizer->Add(transDateBoxSizer);

    // Status --------------------------------------------
    m_choice_status = new wxChoice(this, ID_DIALOG_TRANS_STATUS);

    for (const auto& i : Model_Billsdeposits::all_status())
    {
        m_choice_status->Append(wxGetTranslation(i), new wxStringClientData(i));
    }
    m_choice_status->SetSelection(Option::instance().TransStatusReconciled());
    m_choice_status->SetToolTip(_("Specify the status for the transaction"));

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Status")), g_flagsH);
    transPanelSizer->Add(m_choice_status, g_flagsH);

    // Type --------------------------------------------
    m_choice_transaction_type = new wxChoice(this, wxID_VIEW_DETAILS);
    m_choice_transaction_type->SetToolTip(_("Specify the type of transactions to be created."));
    cAdvanced_ = new wxCheckBox(this, ID_DIALOG_TRANS_ADVANCED_CHECKBOX, _("&Advanced")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cAdvanced_->SetValue(false);
    cAdvanced_->SetToolTip(_("Allows the setting of different amounts in the FROM and TO accounts."));

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
    textAmount_->SetToolTip(amountNormalTip_);
    textAmount_->Connect(ID_DIALOG_TRANS_TEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmBDDialog::OnTextEntered), nullptr, this);

    toTextAmount_ = new mmTextCtrl(this, ID_DIALOG_TRANS_TOTEXTAMOUNT, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER
        , mmCalcValidator());
    toTextAmount_->SetToolTip(_("Specify the transfer amount in the To Account"));
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
    bAccount_->SetToolTip(_("Specify the Account that will own the recurring transaction"));
    transPanelSizer->Add(bAccount_, g_flagsExpand);
    // Payee ------------------------------------------------
    wxStaticText* payee_label = new wxStaticText(this, ID_DIALOG_TRANS_STATIC_PAYEE, _("Payee"));
    payee_label->SetFont(this->GetFont().Bold());

    bPayee_ = new wxButton(this, ID_DIALOG_TRANS_BUTTONPAYEE, _("Select Payee"));
    bPayee_->SetToolTip(payeeWithdrawalTip_);

    transPanelSizer->Add(payee_label, g_flagsH);
    transPanelSizer->Add(bPayee_, g_flagsExpand);

    // Split Category -------------------------------------------
    cSplit_ = new wxCheckBox(this, ID_DIALOG_TRANS_SPLITCHECKBOX, _("Split")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cSplit_->SetValue(FALSE);
    cSplit_->SetToolTip(_("Use split Categories"));

    transPanelSizer->AddSpacer(20); // Fill empty space.
    transPanelSizer->Add(cSplit_, g_flagsH);

    // Category ---------------------------------------------
    wxStaticText* categ_label = new wxStaticText(this, wxID_STATIC, _("Category"));
    categ_label->SetFont(this->GetFont().Bold());
    bCategory_ = new wxButton(this, ID_DIALOG_TRANS_BUTTONCATEGS, _("Select Category")
        , wxDefaultPosition, wxDefaultSize, 0);
    //bCategory_->SetToolTip(_("Specify the category for this transaction"));

    transPanelSizer->Add(categ_label, g_flagsExpand);
    transPanelSizer->Add(bCategory_, g_flagsExpand);

    // Number ---------------------------------------------
    textNumber_ = new wxTextCtrl(this, ID_DIALOG_TRANS_TEXTNUMBER);
    textNumber_->SetToolTip(_("Specify any associated check number or transaction number"));

    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Number")), g_flagsH);
    transPanelSizer->Add(textNumber_, g_flagsExpand);

    // Notes ---------------------------------------------
    transPanelSizer->Add(new wxStaticText(this, wxID_STATIC, _("Notes")), g_flagsH);

    bAttachments_ = new wxBitmapButton(this, wxID_FILE
        , mmBitmap(png::CLIP), wxDefaultPosition
        , wxSize(m_btn_due_date->GetSize().GetY(), m_btn_due_date->GetSize().GetY()));
    bAttachments_->SetToolTip(_("Organize attachments of this recurring transaction"));

    wxButton* bFrequentUsedNotes = new wxButton(this, ID_DIALOG_TRANS_BUTTON_FREQENTNOTES, "..."
        , wxDefaultPosition, wxSize(bAttachments_->GetSize().GetX(), -1));
    bFrequentUsedNotes->SetToolTip(_("Select one of the frequently used notes"));
    bFrequentUsedNotes->Connect(ID_DIALOG_TRANS_BUTTON_FREQENTNOTES
        , wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(mmBDDialog::OnFrequentUsedNotes), nullptr, this);

    wxBoxSizer* RightAlign_sizer = new wxBoxSizer(wxHORIZONTAL);
    transPanelSizer->Add(RightAlign_sizer, wxSizerFlags(g_flagsH).Align(wxALIGN_RIGHT).Border(wxALL, 0));
    RightAlign_sizer->Add(bAttachments_, g_flagsH);
    RightAlign_sizer->Add(bFrequentUsedNotes, g_flagsH);

    textNotes_ = new wxTextCtrl(this, ID_DIALOG_TRANS_TEXTNOTES, ""
        , wxDefaultPosition, wxSize(-1, m_date_due->GetSize().GetHeight() * 5), wxTE_MULTILINE);
    textNotes_->SetToolTip(_("Specify any text notes you want to add to this transaction."));

    transDetailsStaticBoxSizer->Add(textNotes_, g_flagsExpand);

    Fit();
    Layout();
    Centre();
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
            // Only for new transactions: if user want to autofill last category used for payee.
            // If this is a Split Transaction, ignore displaying last category for payee
            if (payee->CATEGID != -1 && m_bill_data.local_splits.empty()
                && Option::instance().TransCategorySelection() == Option::LASTUSED
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

    switch (m_choice_transaction_type->GetSelection())
    {
    case Model_Billsdeposits::TRANSFER:
    {
        m_transfer = true;
        textAmount_->SetToolTip(amountTransferTip_);
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
        bPayee_->SetToolTip(payeeTransferTip_);
        if (prevType_ != -1)
        {
            m_bill_data.TOACCOUNTID = -1;
            payeeUnknown_ = true;
        }
        break;
    }
    case Model_Billsdeposits::WITHDRAWAL:
    {
        textAmount_->SetToolTip(amountNormalTip_);
        accountLabel->SetLabelText(_("Account"));
        stp->SetLabelText(_("Payee"));
        bPayee_->SetToolTip(payeeWithdrawalTip_);
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
        textAmount_->SetToolTip(amountNormalTip_);
        accountLabel->SetLabelText(_("Account"));
        stp->SetLabelText(_("From"));
        bPayee_->SetToolTip(payeeDepositTip_);
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

            // Only for new transactions: if user want to autofill last category used for payee.
            // If this is a Split Transaction, ignore displaying last category for payee
            if (filtd[0].CATEGID != -1 && m_bill_data.local_splits.empty() && Option::instance().TransCategorySelection() == Option::LASTUSED && !categUpdated_ && m_bill_data.BDID == 0)
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
    int id = wxID_HIGHEST;
    for (const auto& entry : frequentNotes_) {
        const wxString& label = entry.Mid(0, 30) + (entry.size() > 30 ? "..." : "");
        menu.Append(++id, label);

    }
    if (!frequentNotes_.empty())
        PopupMenu(&menu);
}

void mmBDDialog::onNoteSelected(wxCommandEvent& event)
{
    size_t i = event.GetId() - wxID_HIGHEST;
    if (i > 0 && i <= frequentNotes_.size()) {
        textNotes_->ChangeValue(frequentNotes_[i - 1]);
    }
}

void mmBDDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
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
            mmErrorDialogs::InvalidPayee(bPayee_);
            return;
        }
    }

    if ((cSplit_->IsChecked() && m_bill_data.local_splits.empty())
        || (!cSplit_->IsChecked() && Model_Category::full_name(m_bill_data.CATEGID, m_bill_data.SUBCATEGID).empty()))
    {
        return mmErrorDialogs::InvalidCategory(bCategory_, false);
    }


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

    m_bill_data.NEXTOCCURRENCEDATE = m_date_paid->GetValue().FormatISODate();
    m_bill_data.TRANSDATE = m_date_due->GetValue().FormatISODate();
    // Ensure that TRANDSATE is set correctly
    if (m_date_paid->GetValue() > m_date_due->GetValue())
    {
        m_bill_data.TRANSDATE = m_date_paid->GetValue().FormatISODate();
    }

    wxStringClientData* status_obj = static_cast<wxStringClientData *>(m_choice_status->GetClientObject(m_choice_status->GetSelection()));
    if (status_obj)
    {
        m_bill_data.STATUS = Model_Billsdeposits::toShortStatus(status_obj->GetData());
    }

    m_bill_data.TRANSACTIONNUMBER = textNumber_->GetValue();
    m_bill_data.NOTES = textNotes_->GetValue();

    if (!m_enter_occur)
    {
        Model_Billsdeposits::Data* bill = Model_Billsdeposits::instance().get(m_bill_data.BDID);
        if (m_new_bill)
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

        transID_ = Model_Billsdeposits::instance().save(bill);

        Model_Budgetsplittransaction::Data_Set splt;
        for (const auto& entry : m_bill_data.local_splits)
        {
            Model_Budgetsplittransaction::Data *s = Model_Budgetsplittransaction::instance().create();
            s->CATEGID = entry.CATEGID;
            s->SUBCATEGID = entry.SUBCATEGID;
            s->SPLITTRANSAMOUNT = entry.SPLITTRANSAMOUNT;
            splt.push_back(*s);
        }
        Model_Budgetsplittransaction::instance().update(splt, transID_);

        const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::BILLSDEPOSIT);
        mmAttachmentManage::RelocateAllAttachments(RefType, 0, transID_);
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

            int transID = Model_Checking::instance().save(tran);

            Model_Splittransaction::Data_Set checking_splits;
            for (auto &item : m_bill_data.local_splits)
            {
                Model_Splittransaction::Data *split = Model_Splittransaction::instance().create();
                split->TRANSID = transID;
                split->CATEGID = item.CATEGID;
                split->SUBCATEGID = item.SUBCATEGID;
                split->SPLITTRANSAMOUNT = item.SPLITTRANSAMOUNT;
                checking_splits.push_back(*split);
            }
            Model_Splittransaction::instance().update(checking_splits, transID);
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
    textAmount_->SetToolTip(m_advanced ? amountTransferTip_ : _("Specify the transfer amount in the From Account"));
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
    m_btn_due_date->Enable(false);
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
        textNumRepeats_->SetToolTip(toolTipsStr);
        m_btn_due_date->Enable();
    }
    else if (repeats == INXMONTHS)
    {
        staticTextRepeats_->SetLabelText(repeatLabelActivate);
        staticTimesRepeat_->SetLabelText(timeLabelMonths);
        const auto& toolTipsStr = _("Specify period in Months to activate.\n"
            "Becomes blank when not active.");
        textNumRepeats_->SetToolTip(toolTipsStr);
        m_btn_due_date->Enable();
    }
    else if (repeats == EVERYXDAYS)
    {
        staticTextRepeats_->SetLabelText(repeatLabelRepeats);
        staticTimesRepeat_->SetLabelText(timeLabelDays);
        const auto& toolTipsStr = _("Specify period in Days to activate.\n"
            "Leave blank when not active.");
        textNumRepeats_->SetToolTip(toolTipsStr);
    }
    else if (repeats == EVERYXMONTHS)
    {
        staticTextRepeats_->SetLabelText(repeatLabelRepeats);
        staticTimesRepeat_->SetLabelText(timeLabelMonths);
        const auto& toolTipsStr = _("Specify period in Months to activate.\n"
            "Leave blank when not active.");
        textNumRepeats_->SetToolTip(toolTipsStr);
    }
    else
    {
        staticTextRepeats_->SetLabelText(repeatLabelRepeats);
        staticTimesRepeat_->SetLabelText(_("Payments Left"));
        const auto& toolTipsStr = _("Specify the number of payments to be made.\n"
            "Leave blank if the payments continue forever.");
        textNumRepeats_->SetToolTip(toolTipsStr);
    }
}

void mmBDDialog::OnRepeatTypeChanged(wxCommandEvent& WXUNUSED(event))
{
    setRepeatDetails();
}

void mmBDDialog::OnsetNextRepeatDate(wxCommandEvent& WXUNUSED(event))
{
    wxString valueStr = textNumRepeats_->GetValue();
    if (valueStr.IsNumber())
    {
        int value = wxAtoi(valueStr);
        wxDateTime date = m_date_due->GetValue();

        int repeats = m_choice_repeat->GetSelection();
        if (repeats == INXDAYS)
        {
            if (value) {
                date = date.Add(wxDateSpan::Days(value));
            }
            else {
                mmErrorDialogs::ToolTip4Object(textNumRepeats_, _("Invalid value"), _("Error"));
            }
        }
        else if (repeats == INXMONTHS)
        {
            if (value) {
                date = date.Add(wxDateSpan::Months(value));
            }
            else {
                mmErrorDialogs::ToolTip4Object(textNumRepeats_, _("Invalid value"), _("Error"));
            }
        }
        else
        {
            mmErrorDialogs::ToolTip4Object(m_choice_repeat
                , _("Invalid Choice") + "\n\n" + _("Choose one of the following:") + "\n"
                    + _("In (x) Days") + "\n" + _("In (x) Months")
                , _("Error"));
        }

        m_date_paid->SetValue(date);
        m_date_due->SetValue(date);
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
        bCategory_->SetToolTip(_("Specify the category for this transaction"));
    }
    else
    {
        const Model_Currency::Data* currency = Model_Currency::GetBaseCurrency();
        const Model_Account::Data* account = Model_Account::instance().get(m_bill_data.ACCOUNTID);
        if (account)
        {
            currency = Model_Account::currency(account);
        }

        bCategory_->SetToolTip(Model_Splittransaction::get_tooltip(m_bill_data.local_splits, currency));
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

void mmBDDialog::OnPaidDateChanged(wxDateEvent& WXUNUSED(event))
{
    wxDateTime paid_date = m_date_paid->GetValue();
    wxDateTime due_date = m_date_due->GetValue();
    if (paid_date > due_date)
    {
        m_date_due->SetValue(paid_date);
    }
}

void mmBDDialog::OnDueDateChanged(wxDateEvent& WXUNUSED(event))
{
    wxDateTime due_date = m_date_due->GetValue();
    wxDateTime paid_date = m_date_paid->GetValue();

    if (!m_enter_occur && paid_date > due_date)
    {
        m_date_paid->SetValue(due_date);
    }
}
