/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011 Nikolay & Stefano Giorgio

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

#include "billsdepositsdialog.h"
#include "attachmentdialog.h"
#include "categdialog.h"
#include "constants.h"
#include "mmOption.h"
#include "mmsinglechoicedialog.h"
#include "paths.h"
#include "payeedialog.h"
#include "util.h"
#include "validators.h"

#include "model/Model_Account.h"
#include "model/Model_Attachment.h"
#include "model/Model_Category.h"
#include "model/Model_Payee.h"

#include "../resources/attachment.xpm"

#include <wx/valnum.h>


wxIMPLEMENT_DYNAMIC_CLASS(mmBDDialog, wxDialog);

wxBEGIN_EVENT_TABLE( mmBDDialog, wxDialog )
    EVT_BUTTON(wxID_OK, mmBDDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, mmBDDialog::OnCancel)
    EVT_BUTTON(ID_DIALOG_TRANS_BUTTONCATEGS, mmBDDialog::OnCategs)
    EVT_BUTTON(ID_DIALOG_BD_COMBOBOX_ACCOUNTNAME, mmBDDialog::OnAccountName)
    EVT_BUTTON(ID_DIALOG_TRANS_BUTTONPAYEE, mmBDDialog::OnPayee)
    EVT_BUTTON(ID_DIALOG_TRANS_BUTTONTO, mmBDDialog::OnTo)
	EVT_BUTTON(wxID_FILE, mmBDDialog::OnAttachments)
    EVT_CHOICE(wxID_VIEW_DETAILS, mmBDDialog::OnTypeChanged)
    EVT_SPIN_UP(ID_DIALOG_TRANS_DATE_SPINNER,mmBDDialog::OnTransDateForward)
    EVT_SPIN_DOWN(ID_DIALOG_TRANS_DATE_SPINNER,mmBDDialog::OnTransDateBack)
    EVT_SPIN_UP(ID_DIALOG_BD_REPEAT_DATE_SPINNER,mmBDDialog::OnNextOccurDateForward)
    EVT_SPIN_DOWN(ID_DIALOG_BD_REPEAT_DATE_SPINNER,mmBDDialog::OnNextOccurDateBack)
    EVT_CHECKBOX(ID_DIALOG_TRANS_ADVANCED_CHECKBOX, mmBDDialog::OnAdvanceChecked)
    EVT_CHECKBOX(ID_DIALOG_TRANS_SPLITCHECKBOX, mmBDDialog::OnSplitChecked)
    EVT_CHECKBOX(ID_DIALOG_BD_CHECKBOX_AUTO_EXECUTE_USERACK, mmBDDialog::OnAutoExecutionUserAckChecked)
    EVT_CHECKBOX(ID_DIALOG_BD_CHECKBOX_AUTO_EXECUTE_SILENT, mmBDDialog::OnAutoExecutionSilentChecked)
    EVT_CALENDAR_SEL_CHANGED(ID_DIALOG_BD_CALENDAR, mmBDDialog::OnCalendarSelChanged)
    EVT_DATE_CHANGED(ID_DIALOG_TRANS_BUTTONDATE, mmBDDialog::OnDateChanged)
    EVT_DATE_CHANGED(ID_DIALOG_BD_BUTTON_NEXTOCCUR, mmBDDialog::OnDateChanged)
    EVT_CHOICE(ID_DIALOG_BD_COMBOBOX_REPEATS, mmBDDialog::OnRepeatTypeChanged)
    EVT_BUTTON(ID_DIALOG_TRANS_BUTTONTRANSNUM, mmBDDialog::OnsetNextRepeatDate)
    EVT_TEXT(ID_DIALOG_BD_TEXTCTRL_NUM_TIMES,mmBDDialog::OnPeriodChange)
    EVT_MENU(wxID_ANY, mmBDDialog::onNoteSelected)
    EVT_CLOSE(mmBDDialog::OnQuit)
wxEND_EVENT_TABLE()

//const wxString REPEAT_TRANSACTIONS_MSGBOX_HEADING = _("Repeat Transaction - Auto Execution Checking");

mmBDDialog::mmBDDialog( )
{
}

mmBDDialog::mmBDDialog(wxWindow* parent, int bdID, bool edit, bool enterOccur)
    : m_new_bill(!edit)
    , enterOccur_(enterOccur)
    , m_advanced(false)
    , payeeUnknown_(true)
    , autoExecuteUserAck_(false)
    , autoExecuteSilent_(false)
    , categUpdated_(false)
    , prevType_(-1)
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
        m_bill_data.NEXTOCCURRENCEDATE = Model_Billsdeposits::NEXTOCCURRENCEDATE(bill).FormatISODate();
        m_bill_data.REPEATS = bill->REPEATS;
        m_bill_data.NUMOCCURRENCES = bill->NUMOCCURRENCES;
        m_bill_data.NOTES = bill->NOTES;
        m_bill_data.STATUS = bill->STATUS;
        m_bill_data.TRANSACTIONNUMBER = bill->TRANSACTIONNUMBER;
        m_bill_data.TRANSCODE = bill->TRANSCODE;
        //
        for (const auto& item : Model_Billsdeposits::splittransaction(bill))
            m_bill_data.local_splits.push_back({ item.CATEGID, item.SUBCATEGID, item.SPLITTRANSAMOUNT });
    }

    m_transfer = (m_bill_data.TRANSCODE == Model_Billsdeposits::all_type()[Model_Billsdeposits::TRANSFER]);
    //---------------------------------------------------
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY
        , _("New Repeating Transaction")
        , wxDefaultPosition, wxSize(500, 300), style);

}

bool mmBDDialog::Create(wxWindow* parent, wxWindowID id, const wxString& caption
    , const wxPoint& pos, const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());

    /**********************************************************************************************
     Ament controls according to function settings
    ***********************************************************************************************/
    if (!m_new_bill || enterOccur_)
    {
        dataToControls();
        if (! enterOccur_)
        {
            SetDialogHeader(_(" Edit Repeating Transaction"));
            dpc_->Disable();
        }
        else
        {
            SetDialogHeader(_(" Enter Repeating Transaction"));
            transaction_type_->Disable();
            dpcNextOccDate_->Disable();
            itemRepeats_->Disable();
            textAmount_->SetFocus();
            itemCheckBoxAutoExeSilent_->Disable();
            itemCheckBoxAutoExeUserAck_->Disable();
            textNumRepeats_->Disable();
            bSetNextOccurDate_->Disable();
        }
    }

    Centre();
    Fit();

    return TRUE;
}

void mmBDDialog::dataToControls()
{
    if (m_bill_data.PAYEEID > 0)
        payeeUnknown_ = false;
    choiceStatus_->SetSelection(Model_Checking::status(m_bill_data.STATUS));
    if (m_bill_data.NUMOCCURRENCES > 0)
        textNumRepeats_->SetValue(wxString::Format("%d", m_bill_data.NUMOCCURRENCES));

    wxDateTime dtno;
    dtno.ParseDate(m_bill_data.NEXTOCCURRENCEDATE);
    dpcNextOccDate_->SetValue(dtno);
    dpc_->SetValue(dtno);
    calendarCtrl_->SetDate(dtno);

    // Have used repeatSel to multiplex auto repeat fields.
    if (m_bill_data.REPEATS >= BD_REPEATS_MULTIPLEX_BASE)
    {
        m_bill_data.REPEATS -= BD_REPEATS_MULTIPLEX_BASE;
        autoExecuteUserAck_ = true;
        itemCheckBoxAutoExeUserAck_->SetValue(true);
        itemCheckBoxAutoExeSilent_->Enable(true);

        if (m_bill_data.REPEATS >= BD_REPEATS_MULTIPLEX_BASE)
        {
            m_bill_data.REPEATS -= BD_REPEATS_MULTIPLEX_BASE;
            autoExecuteSilent_ = true;
            itemCheckBoxAutoExeSilent_->SetValue(true);
        }
    }

    itemRepeats_->SetSelection(m_bill_data.REPEATS);
    setRepeatDetails();
    if (m_bill_data.REPEATS == 0) // if none
        textNumRepeats_->SetValue("");

    transaction_type_->SetSelection(Model_Billsdeposits::type(m_bill_data.TRANSCODE));
    updateControlsForTransType();

    Model_Account::Data* account = Model_Account::instance().get(m_bill_data.ACCOUNTID);
    bAccount_->SetLabelText(account->ACCOUNTNAME);

    setCategoryLabel();
    cSplit_->SetValue(!m_bill_data.local_splits.empty());

    textNotes_->SetValue(m_bill_data.NOTES);
    textNumber_->SetValue(m_bill_data.TRANSACTIONNUMBER);

    if (!m_bill_data.local_splits.empty())
        m_bill_data.TRANSAMOUNT = Model_Splittransaction::get_total(m_bill_data.local_splits);

    textAmount_->Enable(m_bill_data.local_splits.empty());

    textAmount_->SetValue(m_bill_data.TRANSAMOUNT, account);

    if (m_transfer)
    {
        m_bill_data.PAYEEID = -1;
        Model_Account::Data* to_account = Model_Account::instance().get(m_bill_data.TOACCOUNTID);
        if (to_account)
            bPayee_->SetLabelText(to_account->ACCOUNTNAME);

        // When editing an advanced transaction record, we do not reset the m_bill_data.TOTRANSAMOUNT
        if ((!m_new_bill || enterOccur_) && (m_bill_data.TOTRANSAMOUNT != m_bill_data.TRANSAMOUNT))
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
    setTooltips();
}

void mmBDDialog::SetDialogHeader(const wxString& header)
{
    this->SetTitle(header);
}

void mmBDDialog::CreateControls()
{
    const wxString BILLSDEPOSITS_REPEATS[] =
    {
        wxTRANSLATE("None"),
        wxTRANSLATE("Weekly"),
        wxTRANSLATE("Bi-Weekly"),
        wxTRANSLATE("Monthly"),
        wxTRANSLATE("Bi-Monthly"),
        wxTRANSLATE("Quarterly"),
        wxTRANSLATE("Half-Yearly"),
        wxTRANSLATE("Yearly"),
        wxTRANSLATE("Four Months"),
        wxTRANSLATE("Four Weeks"),
        wxTRANSLATE("Daily"),
        wxTRANSLATE("In %s Days"),
        wxTRANSLATE("In %s Months"),
        wxTRANSLATE("Every %s Days"),
        wxTRANSLATE("Every %s Months"),
        wxTRANSLATE("Monthly (last day)"),
        wxTRANSLATE("Monthly (last business day)")

    };

    wxBoxSizer* mainBoxSizerOuter = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* mainBoxSizerInner = new wxBoxSizer(wxHORIZONTAL);

    /* Repeat Transaction Details */
    wxBoxSizer* repeatTransBoxSizer = new wxBoxSizer(wxVERTICAL);

    /* Transaction Details */
    wxStaticBox* transDetailsStaticBox = new wxStaticBox(this, wxID_ANY, _("Transaction Details") );
    wxStaticBoxSizer* transDetailsStaticBoxSizer = new wxStaticBoxSizer(transDetailsStaticBox, wxVERTICAL);

    this->SetSizer(mainBoxSizerOuter);

    /* Calendar */
    wxStaticBox* calendarStaticBox = new wxStaticBox(this, wxID_ANY, _("Calendar") );
    wxStaticBoxSizer* calendarStaticBoxSizer = new wxStaticBoxSizer(calendarStaticBox, wxHORIZONTAL);
    repeatTransBoxSizer->Add(calendarStaticBoxSizer, 10, wxALIGN_CENTER|wxLEFT|wxBOTTOM|wxRIGHT, 15);

    //TODO: Set these up as user selectable. Some users wish to have monday first in calendar!
    bool startSunday = true;
    bool showSuroundingWeeks = true;

    int style = wxSUNKEN_BORDER| wxCAL_SHOW_HOLIDAYS| wxCAL_SEQUENTIAL_MONTH_SELECTION;
    if (startSunday)
        style = wxCAL_SUNDAY_FIRST| style;
    else
        style = wxCAL_MONDAY_FIRST| style;

    if (showSuroundingWeeks)
        style = wxCAL_SHOW_SURROUNDING_WEEKS| style;

    calendarCtrl_ = new wxCalendarCtrl( this, ID_DIALOG_BD_CALENDAR, wxDateTime(),
                                        wxDefaultPosition, wxDefaultSize, style);
    calendarStaticBoxSizer->Add(calendarCtrl_, 10, wxALIGN_CENTER_HORIZONTAL|wxALL, 15);

    /* Bills & Deposits Details */
    wxStaticBox* repeatDetailsStaticBox = new wxStaticBox(this, wxID_ANY, _("Repeating Transaction Details") );
    wxStaticBoxSizer* repeatDetailsStaticBoxSizer = new wxStaticBoxSizer(repeatDetailsStaticBox, wxHORIZONTAL);
    repeatTransBoxSizer->Add(repeatDetailsStaticBoxSizer, 0, wxALIGN_CENTER | wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    repeatDetailsStaticBoxSizer->Add(itemFlexGridSizer5, g_flags);

// change properties depending on system parameters
    int spinCtrlDirection = wxSP_VERTICAL;
    int interval = 0;
#ifdef __WXMSW__
    wxSize spinCtrlSize = wxSize(18, 22);
//    spinCtrlDirection = wxSP_HORIZONTAL;
    interval = 4;
#else
    wxSize spinCtrlSize = wxSize(16,-1);
#endif

    // Next Occur Date --------------------------------------------
    dpcNextOccDate_ = new wxDatePickerCtrl( this, ID_DIALOG_BD_BUTTON_NEXTOCCUR, wxDefaultDateTime
        , wxDefaultPosition, wxSize(110, -1), wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    dpcNextOccDate_->SetToolTip(_("Specify the date of the next bill or deposit"));

    spinNextOccDate_ = new wxSpinButton( this, ID_DIALOG_BD_REPEAT_DATE_SPINNER
        , wxDefaultPosition, spinCtrlSize, spinCtrlDirection | wxSP_ARROW_KEYS | wxSP_WRAP);
    spinNextOccDate_->SetToolTip(_("Retard or advance the date of the 'next occurrence"));

    wxBoxSizer* nextOccurDateBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    nextOccurDateBoxSizer->Add(dpcNextOccDate_, g_flags);
    nextOccurDateBoxSizer->Add(spinNextOccDate_, g_flags);

    itemFlexGridSizer5->Add(new wxStaticText( this, wxID_STATIC, _("Next Occurrence")), g_flags);
    itemFlexGridSizer5->Add(nextOccurDateBoxSizer);

    // Repeats --------------------------------------------
    staticTextRepeats_ = new wxStaticText( this, wxID_STATIC, _("Repeats") );
    itemFlexGridSizer5->Add(staticTextRepeats_, g_flags);

    itemRepeats_ = new wxChoice(this, ID_DIALOG_BD_COMBOBOX_REPEATS
        , wxDefaultPosition, wxSize(110, -1));
    size_t size = sizeof(BILLSDEPOSITS_REPEATS)/sizeof(wxString);
    for(size_t i = 0; i < size; ++i)
    if ( i <=10 || i>14)
        itemRepeats_->Append(wxGetTranslation(BILLSDEPOSITS_REPEATS[i]));
    else
        itemRepeats_->Append(wxString::Format( wxGetTranslation(BILLSDEPOSITS_REPEATS[i]), "(x)"));

    wxBoxSizer* repeatBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    bSetNextOccurDate_ = new wxButton(this, ID_DIALOG_TRANS_BUTTONTRANSNUM, _("Next")
        , wxDefaultPosition, wxSize(60, -1));
    bSetNextOccurDate_->SetToolTip(_("Advance the Next Occurance Date with the specified values"));
    repeatBoxSizer->Add(itemRepeats_, g_flags);
    repeatBoxSizer->Add(bSetNextOccurDate_, g_flags);

    itemFlexGridSizer5->Add(repeatBoxSizer);
    itemRepeats_->SetSelection(0);

    // Repeat Times --------------------------------------------
    staticTimesRepeat_ = new wxStaticText(this, wxID_STATIC, _("Times Repeated"));
    itemFlexGridSizer5->Add(staticTimesRepeat_, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

	wxBoxSizer* repeatTimesBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	itemFlexGridSizer5->Add(repeatTimesBoxSizer);

    textNumRepeats_ = new wxTextCtrl(this, ID_DIALOG_BD_TEXTCTRL_NUM_TIMES, ""
        , wxDefaultPosition, wxSize(110, -1), 0, wxIntegerValidator<int>() );
	repeatTimesBoxSizer->Add(textNumRepeats_, g_flags);
    textNumRepeats_->SetMaxLength(12);
    setRepeatDetails();

    /* Auto Execution Status */
    itemCheckBoxAutoExeUserAck_ = new wxCheckBox( this, ID_DIALOG_BD_CHECKBOX_AUTO_EXECUTE_USERACK,
        _("Set to 'Auto Execute' on the 'Next Occurrence' date."), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    itemCheckBoxAutoExeUserAck_->SetToolTip(_("Automatic Execution will require user acknowledgement."));

    itemCheckBoxAutoExeSilent_ = new wxCheckBox( this, ID_DIALOG_BD_CHECKBOX_AUTO_EXECUTE_SILENT,
        _("Set 'Auto Execute' without user acknowlegement."), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    itemCheckBoxAutoExeSilent_->Disable();
    itemCheckBoxAutoExeSilent_->SetToolTip(_("Automatic Execution will occur without user interaction"));

    repeatTransBoxSizer->Add(itemCheckBoxAutoExeUserAck_, g_flags);
    repeatTransBoxSizer->Add(itemCheckBoxAutoExeSilent_, g_flags);

    /************************************************************************************************************
    transactionPanel controlled by transPanelSizer - is contained in the transDetailsStaticBoxSizer.
    *************************************************************************************************************/
    wxPanel* transactionPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    transDetailsStaticBoxSizer->Add(transactionPanel, 0, wxGROW | wxALL, 10);

    wxBoxSizer* box_sizer1 = new wxBoxSizer(wxVERTICAL);
    transactionPanel->SetSizer(box_sizer1);
    wxFlexGridSizer* transPanelSizer = new wxFlexGridSizer(0, 2, 0, 0);
    box_sizer1->Add(transPanelSizer);

    // Trans Date --------------------------------------------
    dpc_ = new wxDatePickerCtrl( transactionPanel, ID_DIALOG_TRANS_BUTTONDATE, wxDefaultDateTime,
                                 wxDefaultPosition, wxSize(110, -1), wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    dpc_->SetToolTip(_("Specify the date of the transaction"));
    spinTransDate_ = new wxSpinButton( transactionPanel,ID_DIALOG_TRANS_DATE_SPINNER,
                                       wxDefaultPosition, spinCtrlSize,spinCtrlDirection|wxSP_ARROW_KEYS|wxSP_WRAP);
    spinTransDate_->SetToolTip(_("Retard or advance the date of the transaction"));

    wxBoxSizer* transDateBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    transDateBoxSizer->Add(dpc_, g_flags);
    transDateBoxSizer->Add(spinTransDate_, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT, interval);

    transPanelSizer->Add(new wxStaticText( transactionPanel, wxID_STATIC, _("Date")), g_flags);
    transPanelSizer->Add(transDateBoxSizer);

    // Status --------------------------------------------
    choiceStatus_ = new wxChoice(transactionPanel, ID_DIALOG_TRANS_STATUS
        , wxDefaultPosition, wxSize(110, -1));

    for(const auto& i: Model_Billsdeposits::all_status())
        choiceStatus_->Append(wxGetTranslation(i), new wxStringClientData(i));
    choiceStatus_->SetSelection(mmIniOptions::instance().transStatusReconciled_);
    choiceStatus_->SetToolTip(_("Specify the status for the transaction"));

    transPanelSizer->Add(new wxStaticText( transactionPanel, wxID_STATIC, _("Status")), g_flags);
    transPanelSizer->Add(choiceStatus_, g_flags);

    // Type --------------------------------------------
    transaction_type_ = new wxChoice(transactionPanel, wxID_VIEW_DETAILS
        , wxDefaultPosition, wxSize(110, -1));

    for (const auto& i : Model_Billsdeposits::all_type())
    {
        if (i != Model_Billsdeposits::all_type()[Model_Billsdeposits::TRANSFER] || Model_Account::instance().all().size() > 1)
            transaction_type_->Append(wxGetTranslation(i), new wxStringClientData(i));
    }

    transaction_type_->SetSelection(0);
    transaction_type_->SetToolTip(_("Specify the type of transactions to be created."));
    cAdvanced_ = new wxCheckBox(transactionPanel, ID_DIALOG_TRANS_ADVANCED_CHECKBOX, _("&Advanced")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cAdvanced_->SetValue(false);
    cAdvanced_->SetToolTip(_("Allows the setting of different amounts in the FROM and TO accounts."));

    wxBoxSizer* typeSizer = new wxBoxSizer(wxHORIZONTAL);
    typeSizer->Add(transaction_type_, g_flags);
    typeSizer->Add(cAdvanced_, g_flags);

    transPanelSizer->Add(new wxStaticText( transactionPanel, wxID_STATIC, _("Type")), g_flags);
    transPanelSizer->Add(typeSizer);

    // Amount Fields --------------------------------------------
    amountNormalTip_   = _("Specify the amount for this transaction");
    amountTransferTip_ = _("Specify the amount to be transfered");

    wxStaticText* staticTextAmount = new wxStaticText(transactionPanel, wxID_STATIC, _("Amount"));

    textAmount_ = new mmTextCtrl(transactionPanel, ID_DIALOG_TRANS_TEXTAMOUNT, ""
        , wxDefaultPosition, wxSize(110, -1), wxALIGN_RIGHT|wxTE_PROCESS_ENTER
        , mmCalcValidator());
    textAmount_->SetToolTip(amountNormalTip_);
    textAmount_->Connect(ID_DIALOG_TRANS_TEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER,
        wxCommandEventHandler(mmBDDialog::OnTextEntered), nullptr, this);

    toTextAmount_ = new mmTextCtrl(transactionPanel, ID_DIALOG_TRANS_TOTEXTAMOUNT, ""
        , wxDefaultPosition, wxSize(110, -1), wxALIGN_RIGHT|wxTE_PROCESS_ENTER
        , mmCalcValidator());
    toTextAmount_->SetToolTip(_("Specify the transfer amount in the To Account"));
    toTextAmount_->Connect(ID_DIALOG_TRANS_TOTEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmBDDialog::OnTextEntered), nullptr, this);

    wxBoxSizer* amountSizer = new wxBoxSizer(wxHORIZONTAL);
    amountSizer->Add(textAmount_, g_flags);
    amountSizer->Add(toTextAmount_, g_flags);

    transPanelSizer->Add(staticTextAmount, g_flags);
    transPanelSizer->Add(amountSizer);

    // Account ------------------------------------------------
    transPanelSizer->Add(new wxStaticText(transactionPanel, ID_DIALOG_TRANS_STATIC_ACCOUNT, _("Account")), g_flags);
    bAccount_ = new wxButton(transactionPanel, ID_DIALOG_BD_COMBOBOX_ACCOUNTNAME, _("Select Account")
        , wxDefaultPosition, wxSize(230, -1));
    bAccount_->SetToolTip(_("Specify the Account that will own the repeating transaction"));
    transPanelSizer->Add(bAccount_, g_flags);
    // Payee ------------------------------------------------
    wxStaticText* staticTextPayee = new wxStaticText(transactionPanel, ID_DIALOG_TRANS_STATIC_PAYEE, _("Payee"));

    bPayee_ = new wxButton(transactionPanel, ID_DIALOG_TRANS_BUTTONPAYEE, _("Select Payee")
        , wxDefaultPosition, wxSize(230, -1), 0);
    payeeWithdrawalTip_ = _("Specify where the transaction is going to");

    bPayee_->SetToolTip(payeeWithdrawalTip_);

    transPanelSizer->Add(staticTextPayee, g_flags);
    transPanelSizer->Add(bPayee_, g_flags);

    // Split Category -------------------------------------------
    cSplit_ = new wxCheckBox( transactionPanel, ID_DIALOG_TRANS_SPLITCHECKBOX, _("Split"),
                              wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    cSplit_->SetValue(FALSE);
    cSplit_->SetToolTip(_("Use split Categories"));

    transPanelSizer->AddSpacer(20); // Fill empty space.
    transPanelSizer->Add(cSplit_, g_flags);

    // Category ---------------------------------------------
    wxStaticText* staticTextCategory = new wxStaticText( transactionPanel, wxID_STATIC, _("Category"));
    bCategory_ = new wxButton(transactionPanel, ID_DIALOG_TRANS_BUTTONCATEGS, _("Select Category")
        , wxDefaultPosition, wxSize(230, -1), 0);
    //bCategory_->SetToolTip(_("Specify the category for this transaction"));

    transPanelSizer->Add(staticTextCategory, g_flags);
    transPanelSizer->Add(bCategory_, g_flags);

    // Number ---------------------------------------------
    textNumber_ = new wxTextCtrl(transactionPanel, ID_DIALOG_TRANS_TEXTNUMBER, ""
        , wxDefaultPosition, wxSize(230, -1));
    textNumber_->SetToolTip(_("Specify any associated check number or transaction number"));

    transPanelSizer->Add(new wxStaticText(transactionPanel, wxID_STATIC, _("Number")), g_flags);
    transPanelSizer->Add(textNumber_, g_flags);

    // Notes ---------------------------------------------
    transPanelSizer->Add(new wxStaticText(transactionPanel, wxID_STATIC, _("Notes")), g_flags);

    bAttachments_ = new wxBitmapButton(transactionPanel, wxID_FILE
        , wxBitmap(attachment_xpm), wxDefaultPosition
        , wxSize(bSetNextOccurDate_->GetSize().GetY(), bSetNextOccurDate_->GetSize().GetY()));
    bAttachments_->SetToolTip(_("Organize attachments of this repeating transaction"));

    wxButton* bFrequentUsedNotes = new wxButton(transactionPanel, ID_DIALOG_TRANS_BUTTON_FREQENTNOTES, "..."
        , wxDefaultPosition, wxSize(bAttachments_->GetSize().GetX(), -1));
    bFrequentUsedNotes->SetToolTip(_("Select one of the frequently used notes"));
    bFrequentUsedNotes->Connect(ID_DIALOG_TRANS_BUTTON_FREQENTNOTES
        , wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(mmBDDialog::OnFrequentUsedNotes), nullptr, this);

    wxBoxSizer* RightAlign_sizer = new wxBoxSizer(wxHORIZONTAL);
    RightAlign_sizer->Add(bAttachments_, g_flags);
    RightAlign_sizer->Add(bFrequentUsedNotes, g_flags);

    textNotes_ = new wxTextCtrl(transactionPanel, ID_DIALOG_TRANS_TEXTNOTES, ""
        , wxDefaultPosition, wxSize(225, 80), wxTE_MULTILINE);
    textNotes_->SetToolTip(_("Specify any text notes you want to add to this transaction."));

    transPanelSizer->Add(RightAlign_sizer, wxSizerFlags(g_flags).Align(wxALIGN_RIGHT).Border(wxALL, 0));
    box_sizer1->Add(textNotes_, wxSizerFlags(g_flagsExpand).Border(wxTOP, 5));

    SetTransferControls();  // hide appropriate fields
    //prevType_ = Model_Billsdeposits::WITHDRAWAL;
    /**********************************************************************************************
     Button Panel with OK and Cancel Buttons
    ***********************************************************************************************/
    wxPanel* buttonsPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    wxBoxSizer* buttonsPanelSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonsPanel->SetSizer(buttonsPanelSizer);

    wxButton* okButton = new wxButton(buttonsPanel, wxID_OK, _("&OK "));
    buttonsPanelSizer->Add(okButton, g_flags);

    wxButton* cancelButton = new wxButton(buttonsPanel, wxID_CANCEL, _("&Cancel "));
    buttonsPanelSizer->Add(cancelButton, g_flags);
    cancelButton->SetFocus();

    /**********************************************************************************************
     Determining where the controls go
    ***********************************************************************************************/
    //mainBoxSizerInner will allign contents horizontally
    mainBoxSizerInner->Add(repeatTransBoxSizer, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);
    mainBoxSizerInner->Add(transDetailsStaticBoxSizer, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    //mainBoxSizerOuter will allign contents vertically
    mainBoxSizerOuter->Add(mainBoxSizerInner, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxTOP|wxRIGHT, 5);
    mainBoxSizerOuter->Add(buttonsPanel, 0, wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL|wxLEFT|wxBOTTOM|wxRIGHT, 5);

    /**********************************************************************************************
     Adjust controls according to function settings
    ***********************************************************************************************/
    resetPayeeString();
    if (enterOccur_)
    {
        spinNextOccDate_->Disable();
    }
    else
    {
        dpc_->Disable();
        spinTransDate_->Disable();
    }
}

void mmBDDialog::OnQuit(wxCloseEvent& /*event*/)
{
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::BILLSDEPOSIT);
    if (!m_bill_data.BDID)
        mmAttachmentManage::DeleteAllAttachments(RefType, m_bill_data.BDID);
    EndModal(wxID_CANCEL);
}

void mmBDDialog::OnCancel(wxCommandEvent& /*event*/)
{
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::BILLSDEPOSIT);
    if (!m_bill_data.BDID)
        mmAttachmentManage::DeleteAllAttachments(RefType, m_bill_data.BDID);
    EndModal(wxID_CANCEL);
}

void mmBDDialog::OnAccountName(wxCommandEvent& /*event*/)
{
    const auto& accounts = Model_Account::instance().all_checking_account_names();
    mmSingleChoiceDialog scd(this
        , _("Choose Bank Account or Term Account")
        , _("Select Account")
        , accounts);

    if (!accounts.empty()) {
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
            double amount = 0.0;
            textAmount_->GetDouble(amount);
            textAmount_->SetValue(amount, account);
            if (m_advanced)
            {
                double toAmount = 0.0;
                toTextAmount_->GetDouble(toAmount);
                toTextAmount_->SetValue(toAmount, account);
            }
            m_bill_data.ACCOUNTID = account->ACCOUNTID;
            bAccount_->SetLabelText(acctName);
            if (m_transfer)
            {
                //m_bill_data.PAYEEID = account->ACCOUNTID;//TODO:??
                bPayee_->SetLabelText(acctName);
            }
        }
    }
}

void mmBDDialog::OnPayee(wxCommandEvent& /*event*/)
{
    if (m_transfer)
    {
        m_bill_data.PAYEEID = -1;
        mmSingleChoiceDialog scd(this, _("Account name"), _("Select Account")
            , Model_Account::instance().all_checking_account_names());
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
        mmPayeeDialog dlg(this,true);

        if (dlg.ShowModal() == wxID_OK)
        {
            m_bill_data.PAYEEID = dlg.getPayeeId();
            Model_Payee::Data* payee = Model_Payee::instance().get(m_bill_data.PAYEEID);
            if (payee)
            {
                bPayee_->SetLabelText(payee->PAYEENAME);
                payeeUnknown_ = false;
                // Only for new transactions: if user want to autofill last category used for payee.
                // If this is a Split Transaction, ignore displaying last category for payee
                if (payee->CATEGID != -1 && m_bill_data.local_splits.empty() 
                    && mmIniOptions::instance().transCategorySelectionNone_ == 1 
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
}

void mmBDDialog::OnTo(wxCommandEvent& /*event*/)
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

void mmBDDialog::OnCategs(wxCommandEvent& /*event*/)
{
    if (cSplit_->GetValue())
    {
        activateSplitTransactionsDlg();
    }
    else
    {
        mmCategDialog dlg(this);
        dlg.setTreeSelection(m_bill_data.CATEGID, m_bill_data.SUBCATEGID);
        if (dlg.ShowModal() == wxID_OK)
        {
            m_bill_data.CATEGID = dlg.getCategId();
            m_bill_data.SUBCATEGID = dlg.getSubCategId();

            categUpdated_ = true;
        }
    }
    setCategoryLabel();
}

void mmBDDialog::displayControlsForType(Model_Billsdeposits::TYPE transType, bool enableAdvanced)
{
    bPayee_->UnsetToolTip();
    textAmount_->SetToolTip(_("Specify the amount for this transaction"));

    if (transType == Model_Billsdeposits::TRANSFER) {
        bPayee_->SetToolTip(_("Specify which account the transfer is going to"));
        textAmount_->SetToolTip(amountTransferTip_);
    } else {
        bPayee_->SetToolTip(_("Specify to whom the transaction is going to or coming from "));
        textAmount_->SetToolTip(amountNormalTip_);
    }
}

void mmBDDialog::OnTypeChanged(wxCommandEvent& /*event*/)
{
    updateControlsForTransType();
}

void mmBDDialog::OnAttachments(wxCommandEvent& /*event*/)
{
	const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::BILLSDEPOSIT);
	mmAttachmentDialog dlg(this, RefType, m_bill_data.BDID);
	dlg.ShowModal();
}

void mmBDDialog::updateControlsForTransType()
{
    wxStaticText* accountLabel = (wxStaticText*) FindWindow(ID_DIALOG_TRANS_STATIC_ACCOUNT);
    wxStaticText* stp = (wxStaticText*) FindWindow(ID_DIALOG_TRANS_STATIC_PAYEE);

    if (transaction_type_->GetSelection() == Model_Billsdeposits::WITHDRAWAL)
    {
        displayControlsForType(Model_Billsdeposits::WITHDRAWAL);
        SetTransferControls();
        stp->SetLabelText(_("Payee"));
        bPayee_->SetToolTip(payeeWithdrawalTip_);
        if (payeeUnknown_)
        {
            m_bill_data.PAYEEID = -1;
            m_bill_data.TOACCOUNTID = -1;
            resetPayeeString();
        }
        prevType_ = Model_Billsdeposits::WITHDRAWAL;
        m_transfer = false;
    }
    else if (transaction_type_->GetSelection() == Model_Billsdeposits::DEPOSIT)
    {
        displayControlsForType(Model_Billsdeposits::DEPOSIT);
        SetTransferControls();
        stp->SetLabelText(_("From"));
        bPayee_->SetToolTip(_("Specify where the transaction is coming from"));
        if (payeeUnknown_)
        {
            m_bill_data.PAYEEID = -1;
            m_bill_data.TOACCOUNTID = -1;
            resetPayeeString();
        }
        prevType_ = Model_Billsdeposits::DEPOSIT;
        m_transfer = false;
    }
    else //transfer
    {
        displayControlsForType(Model_Billsdeposits::TRANSFER, true);
        if (m_bill_data.ACCOUNTID < 0)
        {
            bAccount_->SetLabelText(_("From"));
            m_bill_data.PAYEEID = -1;
            payeeUnknown_ = true;
        }
        else
        {
            bPayee_->SetLabelText(bAccount_->GetLabel());
            m_bill_data.PAYEEID = m_bill_data.ACCOUNTID;
        }

        SetTransferControls(true);
        if (cAdvanced_->IsChecked()) SetAdvancedTransferControls(true);

        stp->SetLabelText(_("To"));
        accountLabel->SetLabelText(_("From"));
        bPayee_->SetLabelText(_("Select To Account"));
        if (prevType_ != -1)
        {
            m_bill_data.TOACCOUNTID = -1;
            payeeUnknown_ = true;
        }
        prevType_ = Model_Billsdeposits::TRANSFER;
        m_transfer = true;
    }
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
            if (filtd[0].CATEGID != -1 && m_bill_data.local_splits.empty() && mmIniOptions::instance().transCategorySelectionNone_ == 1 && !categUpdated_ && m_bill_data.BDID == 0)
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
    Model_Checking::getFrequentUsedNotes(frequentNotes_);
    wxMenu menu;
    int id = wxID_HIGHEST;
    for (const auto& entry : frequentNotes_) {
        const wxString label = entry.Mid(0, 30) + (entry.size() > 30 ? "..." : "");
        menu.Append(++id, label);
        
    }
        if (!frequentNotes_.empty())
        PopupMenu(&menu);
}

void mmBDDialog::onNoteSelected(wxCommandEvent& event)
{
    int i = event.GetId() - wxID_HIGHEST;
    if (i > 0 && i <= (int)frequentNotes_.size())
        textNotes_->ChangeValue(frequentNotes_[i - 1]);
}

void mmBDDialog::OnOk(wxCommandEvent& /*event*/)
{
    Model_Account::Data *acc = Model_Account::instance().get(m_bill_data.ACCOUNTID);
    if (!acc)
    {
        mmMessageAccountInvalid((wxWindow*)bAccount_);
        return;
    }

    if (m_transfer)
    {
        Model_Account::Data *to_acc = Model_Account::instance().get(m_bill_data.TOACCOUNTID);
        if (!to_acc || to_acc->ACCOUNTID == acc->ACCOUNTID) {
            mmMessageAccountInvalid((wxWindow*)bPayee_, true);
            return;
        }

        if (m_advanced)
        {
            if (!toTextAmount_->checkValue(m_bill_data.TOTRANSAMOUNT))
                return;
        }
        else
            m_bill_data.TOTRANSAMOUNT = m_bill_data.TRANSAMOUNT;

    }
    else
    {
        Model_Payee::Data *payee = Model_Payee::instance().get(m_bill_data.PAYEEID);
        if (!payee) {
            mmMessagePayeeInvalid((wxWindow*) bPayee_);
            return;
        }
        m_bill_data.TOTRANSAMOUNT = m_bill_data.TRANSAMOUNT;
    }

    if (cSplit_->IsChecked())
    {
        if (m_bill_data.local_splits.empty())
        {
            mmMessageCategoryInvalid((wxWindow*)bCategory_);
            return;
        }
    }
    else
    {
        if (Model_Category::full_name(m_bill_data.CATEGID, m_bill_data.SUBCATEGID).empty())
        {
            mmMessageCategoryInvalid((wxWindow*)bCategory_);
            return;
        }
    }

    if (cSplit_->IsChecked())
    {
        m_bill_data.TRANSAMOUNT = Model_Splittransaction::get_total(m_bill_data.local_splits);

        if (m_bill_data.TRANSAMOUNT < 0.0)
        {
            if (transaction_type_->GetSelection() == Model_Billsdeposits::TRANSFER) {
                if (m_bill_data.TRANSAMOUNT < 0)
                    m_bill_data.TRANSAMOUNT = -m_bill_data.TRANSAMOUNT;
            }
            else {
                mmShowErrorMessageInvalid(this, _("Amount"));
                return;
            }
        }
    }
    else
    {
        if (!textAmount_->checkValue(m_bill_data.TRANSAMOUNT))
            return;
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

                double rateFrom = from_currency->BASECONVRATE;
                double rateTo = to_currency->BASECONVRATE;

                double convToBaseFrom = rateFrom * m_bill_data.TRANSAMOUNT;
                m_bill_data.TOTRANSAMOUNT = convToBaseFrom / rateTo;
            }
            else {
                m_bill_data.TOTRANSAMOUNT = m_bill_data.TRANSAMOUNT;
            }
        }
    }

    // Multiplex Auto executable onto the repeat field of the database.
    m_bill_data.REPEATS = itemRepeats_->GetSelection();
    if (autoExecuteUserAck_)
        m_bill_data.REPEATS += BD_REPEATS_MULTIPLEX_BASE;
    if (autoExecuteSilent_)
        m_bill_data.REPEATS += BD_REPEATS_MULTIPLEX_BASE;

    const wxString& numRepeatStr = textNumRepeats_->GetValue();
    m_bill_data.NUMOCCURRENCES = -1;

    if (!numRepeatStr.empty()) {
        long cnt = 0;
        if (numRepeatStr.ToLong(&cnt))
        {
            wxASSERT(std::numeric_limits<int>::min() <= cnt);
            wxASSERT(cnt <= std::numeric_limits<int>::max());
            m_bill_data.NUMOCCURRENCES = static_cast<int>(cnt);
        }
    }

    m_bill_data.NEXTOCCURRENCEDATE = dpcNextOccDate_->GetValue().FormatISODate();

    wxStringClientData* status_obj = (wxStringClientData *) choiceStatus_->GetClientObject(choiceStatus_->GetSelection());
    if (status_obj) m_bill_data.STATUS = Model_Billsdeposits::toShortStatus(status_obj->GetData());

    m_bill_data.TRANSDATE = dpc_->GetValue().FormatISODate();
    m_bill_data.TRANSACTIONNUMBER = textNumber_->GetValue();
    m_bill_data.NOTES = textNotes_->GetValue();

    if (!enterOccur_)
    {
        Model_Billsdeposits::Data* bill = Model_Billsdeposits::instance().get(m_bill_data.BDID);
        if (m_new_bill)
            bill = Model_Billsdeposits::instance().create();

        bill->ACCOUNTID = m_bill_data.ACCOUNTID;
        bill->TOACCOUNTID = m_bill_data.TOACCOUNTID;
        bill->PAYEEID = m_bill_data.PAYEEID;
        bill->TRANSCODE = Model_Billsdeposits::all_type()[transaction_type_->GetSelection()];
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
        for (const auto& entry : m_bill_data.local_splits) {
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
        if ((itemRepeats_->GetSelection() < 11) || (itemRepeats_->GetSelection() > 14) || (m_bill_data.REPEATS > 0))
        {
            Model_Checking::Data* tran = Model_Checking::instance().create();
            tran->ACCOUNTID = m_bill_data.ACCOUNTID;
            tran->TOACCOUNTID = m_bill_data.TOACCOUNTID;
            tran->PAYEEID = m_bill_data.PAYEEID;
            tran->TRANSCODE = Model_Billsdeposits::all_type()[transaction_type_->GetSelection()];
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

void mmBDDialog::OnSplitChecked(wxCommandEvent& /*event*/)
{
    if (cSplit_->IsChecked())
    {
        activateSplitTransactionsDlg();
    }
    else
    {
        if (m_bill_data.local_splits.size() > 1)
        {
            //Delete split items first (data protection)
            cSplit_->SetValue(true);
        }
        else
        {
            if (m_bill_data.local_splits.size() == 1)
            {
                m_bill_data.CATEGID = m_bill_data.local_splits.begin()->CATEGID;
                m_bill_data.SUBCATEGID = m_bill_data.local_splits.begin()->SUBCATEGID;
                m_bill_data.TRANSAMOUNT = m_bill_data.local_splits.begin()->SPLITTRANSAMOUNT;

                if (m_bill_data.TRANSAMOUNT < 0)
                {
                    m_bill_data.TRANSAMOUNT = -m_bill_data.TRANSAMOUNT;
                    transaction_type_->SetSelection(Model_Checking::WITHDRAWAL);
                }
            }
            else
                m_bill_data.TRANSAMOUNT = 0;

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

void mmBDDialog::OnAutoExecutionUserAckChecked(wxCommandEvent& /*event*/)
{
    autoExecuteUserAck_ = ! autoExecuteUserAck_;
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

void mmBDDialog::OnAutoExecutionSilentChecked(wxCommandEvent& /*event*/)
{
    autoExecuteSilent_ = ! autoExecuteSilent_;
}

void mmBDDialog::OnCalendarSelChanged(wxCalendarEvent& event)
{
    wxDateTime date = event.GetDate();
    if (!enterOccur_)
    {
        dpcNextOccDate_->SetValue(date) ;
        dpc_->SetValue(date) ;
    }
}

void mmBDDialog::OnDateChanged(wxDateEvent& event)
{
    wxDateTime date = event.GetDate();
    calendarCtrl_->SetDate(date) ;
}

void mmBDDialog::OnAdvanceChecked(wxCommandEvent& /*event*/)
{
    if (cAdvanced_->IsChecked())
    {
        SetAdvancedTransferControls(true);
    }
    else
    {
        SetAdvancedTransferControls();
        textAmount_->SetToolTip(amountTransferTip_);
    }
}

void mmBDDialog::SetTransferControls(bool transfers)
{
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
            cAdvanced_->Enable();
        }
    }
    else
    {
        if (!(prevType_ == Model_Billsdeposits::WITHDRAWAL || prevType_ == Model_Billsdeposits::DEPOSIT))
        {
            cAdvanced_->Disable();
            SetAdvancedTransferControls();
            cSplit_->Enable();
            textAmount_->Enable();
        }
    }
}

void mmBDDialog::SetAdvancedTransferControls(bool advanced)
{
    if (advanced)
    {
        toTextAmount_->Enable();
        m_advanced = true;
        // Display the transfer amount in the toTextAmount control.
        if (m_bill_data.TOTRANSAMOUNT >= 0)
        {
            toTextAmount_->SetValue(m_bill_data.TOTRANSAMOUNT);
        }
        else
        {
            toTextAmount_->SetValue(textAmount_->GetValue());
        }

        textAmount_->SetToolTip(_("Specify the transfer amount in the From Account"));

    }
    else
    {
        toTextAmount_->Disable();
        m_advanced = false;
    }
}

void mmBDDialog::SetNewDate(wxDatePickerCtrl* dpc, bool forward)
{
    int day = -1;
    if (forward) day = 1;

    wxDateTime date = dpc->GetValue().Add(wxDateSpan::Days(day));

    dpc_->SetValue( date );
    dpcNextOccDate_->SetValue( date );
    calendarCtrl_->SetDate(date);
}

void mmBDDialog::OnNextOccurDateForward(wxSpinEvent& /*event*/)
{
    SetNewDate(dpcNextOccDate_);
}

void mmBDDialog::OnNextOccurDateBack(wxSpinEvent& /*event*/)
{
    SetNewDate(dpcNextOccDate_, false);
}

void mmBDDialog::OnTransDateForward(wxSpinEvent& /*event*/)
{
    SetNewDate(dpc_);
}

void mmBDDialog::OnTransDateBack(wxSpinEvent& /*event*/)
{
    SetNewDate(dpc_, false);
}

void mmBDDialog::setRepeatDetails()
{
    const wxString& repeatLabelRepeats  = _("Repeats");
    const wxString& repeatLabelActivate = _("Activates");

    const wxString& timeLabelDays   = _("Period: Days");
    const wxString& timeLabelMonths = _("Period: Months");

    bSetNextOccurDate_->Disable();
    int repeats = itemRepeats_->GetSelection();
    if (repeats == 11)
    {
        staticTextRepeats_->SetLabelText(repeatLabelActivate);
        staticTimesRepeat_->SetLabelText(timeLabelDays);
        const auto& toolTipsStr = _("Specify period in Days to activate.") 
            + "\n" + _("Becomes blank when not active.");
        textNumRepeats_->SetToolTip(toolTipsStr);
    }
    else if (repeats == 12)
    {
        staticTextRepeats_->SetLabelText(repeatLabelActivate);
        staticTimesRepeat_->SetLabelText(timeLabelMonths);
        const auto& toolTipsStr = _("Specify period in Months to activate.") 
            + "\n" + _("Becomes blank when not active.");
        textNumRepeats_->SetToolTip(toolTipsStr);
    }
    else if (repeats == 13)
    {
        staticTextRepeats_->SetLabelText(repeatLabelRepeats);
        staticTimesRepeat_->SetLabelText(timeLabelDays);
        const auto& toolTipsStr = _("Specify period in Days to activate.") 
            + "\n" + _("Leave blank when not active.");
        textNumRepeats_->SetToolTip(toolTipsStr);
    }
    else if (repeats == 14)
    {
        staticTextRepeats_->SetLabelText(repeatLabelRepeats);
        staticTimesRepeat_->SetLabelText(timeLabelMonths);
        const auto& toolTipsStr = _("Specify period in Months to activate.") + "\n" + _("Leave blank when not active.");
        textNumRepeats_->SetToolTip(toolTipsStr);
    }
    else
    {
        staticTextRepeats_->SetLabelText(repeatLabelRepeats);
        staticTimesRepeat_->SetLabelText(_("Times Repeated"));
        const auto& toolTipsStr = _("Specify the number of times this series repeats.") 
            + "\n" + _("Leave blank if this series continues forever.");
        textNumRepeats_->SetToolTip(toolTipsStr);
    }
}

void mmBDDialog::OnRepeatTypeChanged(wxCommandEvent& /*event*/)
{
    setRepeatDetails();
}

void mmBDDialog::OnsetNextRepeatDate(wxCommandEvent& /*event*/)
{
    wxString valueStr = textNumRepeats_->GetValue();
    if (valueStr.IsNumber())
    {
        int value = wxAtoi(valueStr);
        wxDateTime  date = dpcNextOccDate_->GetValue();

        int repeats = itemRepeats_->GetSelection();
        if (repeats == 11)
        {
            date = date.Add(wxDateSpan::Days(value));
        }
        else if (repeats == 12)
        {
            date = date.Add(wxDateSpan::Months(value));
        }

        dpc_->SetValue( date );
        dpcNextOccDate_->SetValue( date );
        calendarCtrl_->SetDate( date );

        bSetNextOccurDate_->Disable();
    }
}

void mmBDDialog::OnPeriodChange(wxCommandEvent& /*event*/)
{
    // event is ignored when showing: Times Repeated
    int repeats = itemRepeats_->GetSelection();
    if ((repeats == 11) || (repeats == 12)) bSetNextOccurDate_->Enable();
}

void mmBDDialog::activateSplitTransactionsDlg()
{
    bool bDeposit = (m_bill_data.TRANSCODE == Model_Billsdeposits::all_type()[Model_Checking::DEPOSIT]);

    if (m_bill_data.CATEGID > -1 && m_bill_data.local_splits.empty())
    {
        if (!textAmount_->GetDouble(m_bill_data.TRANSAMOUNT))
            m_bill_data.TRANSAMOUNT = 0;
        Split s;
        s.SPLITTRANSAMOUNT = bDeposit ? m_bill_data.TRANSAMOUNT : m_bill_data.TRANSAMOUNT;
        s.CATEGID = m_bill_data.CATEGID;
        s.SUBCATEGID = m_bill_data.SUBCATEGID;
        m_bill_data.local_splits.push_back(s);
    }

    SplitTransactionDialog dlg(this, m_bill_data.local_splits
        , transaction_type_->GetSelection(), m_bill_data.ACCOUNTID);
    if (dlg.ShowModal() == wxID_OK)
    {
        m_bill_data.local_splits = dlg.getResult();
        m_bill_data.TRANSAMOUNT = Model_Splittransaction::get_total(m_bill_data.local_splits);
        m_bill_data.CATEGID = -1;
        m_bill_data.SUBCATEGID = -1;
        if (transaction_type_->GetSelection() == Model_Billsdeposits::TRANSFER && m_bill_data.TRANSAMOUNT < 0)
            m_bill_data.TRANSAMOUNT = -m_bill_data.TRANSAMOUNT;
        textAmount_->SetValue(m_bill_data.TRANSAMOUNT);
    }
    textAmount_->Enable(m_bill_data.local_splits.empty());
    setCategoryLabel();
}

void mmBDDialog::OnTextEntered(wxCommandEvent& event)
{
    Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();
    Model_Account::Data *account = Model_Account::instance().get(m_bill_data.ACCOUNTID);
    if (account) currency = Model_Account::currency(account);

    if (event.GetId() == textAmount_->GetId())
    {
        textAmount_->Calculate(currency);
    }
    else if (event.GetId() == toTextAmount_->GetId())
    {
        toTextAmount_->Calculate(currency);
    }
}

void mmBDDialog::setTooltips()
{
    bCategory_->UnsetToolTip();
    if (this->m_bill_data.local_splits.empty())
        bCategory_->SetToolTip(_("Specify the category for this transaction"));
    else {
        const Model_Currency::Data* currency = Model_Currency::GetBaseCurrency();
        const Model_Account::Data* account = Model_Account::instance().get(m_bill_data.ACCOUNTID);
        if (account)
            currency = Model_Account::currency(account);

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
        if (fullCategoryName.IsEmpty()) fullCategoryName = _("Select Category");
    }

    bCategory_->SetLabelText(fullCategoryName);
    setTooltips();
}