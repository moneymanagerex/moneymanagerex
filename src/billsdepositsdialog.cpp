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
#include "util.h"
#include "mmOption.h"
#include "mmsinglechoicedialog.h"
#include "paths.h"
#include "constants.h"
#include "categdialog.h"
#include "payeedialog.h"
#include "splittransactionsdialog.h"
#include "validators.h"
#include "model/Model_Payee.h"
#include "model/Model_Account.h"
#include "model/Model_Category.h"
#include "constants.h"
#include <wx/valnum.h>

IMPLEMENT_DYNAMIC_CLASS( mmBDDialog, wxDialog )

BEGIN_EVENT_TABLE( mmBDDialog, wxDialog )
    EVT_BUTTON(wxID_OK, mmBDDialog::OnOk)
    EVT_BUTTON(ID_DIALOG_TRANS_BUTTONCATEGS, mmBDDialog::OnCategs)
    EVT_BUTTON(ID_DIALOG_BD_COMBOBOX_ACCOUNTNAME, mmBDDialog::OnAccountName)
    EVT_BUTTON(ID_DIALOG_TRANS_BUTTONPAYEE, mmBDDialog::OnPayee)
    EVT_BUTTON(ID_DIALOG_TRANS_BUTTONTO, mmBDDialog::OnTo)
    EVT_CHOICE(wxID_VIEW_DETAILS, mmBDDialog::OnTransTypeChanged)
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
    END_EVENT_TABLE()

//const wxString REPEAT_TRANSACTIONS_MSGBOX_HEADING = _("Repeat Transaction - Auto Execution Checking");

mmBDDialog::mmBDDialog( )
{
}

mmBDDialog::mmBDDialog(wxWindow* parent, int bdID, bool edit, bool enterOccur)
    : bdID_(bdID)
    , edit_(edit)
    , enterOccur_(enterOccur)
    , categID_(-1)
    , subcategID_(-1)
    , payeeID_(-1)
    , accountID_(-1)
    , toID_(-1)
    , toTransAmount_(0)
    , advancedToTransAmountSet_(false)
    , payeeUnknown_(false)
    , autoExecuteUserAck_(false)
    , autoExecuteSilent_(false)
    , categUpdated_(false)
    , prevType_(-1)
{
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
    if (edit_ || enterOccur_)
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
            dpcbd_->Disable();
            itemRepeats_->Disable();
            itemAccountName_->Disable();
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
    const Model_Billsdeposits::Data * bill = Model_Billsdeposits::instance().get(bdID_);
    if (bill)
    {
        categID_ = bill->CATEGID;
        subcategID_ = bill->SUBCATEGID;

        toTransAmount_ = bill->TOTRANSAMOUNT;

        choiceStatus_->SetSelection(Model_Billsdeposits::status(bill));

        if (bill->NUMOCCURRENCES > 0)
            textNumRepeats_->SetValue(wxString::Format("%d", bill->NUMOCCURRENCES));

        wxDateTime dtno = Model_Billsdeposits::NEXTOCCURRENCEDATE(bill);
        dpcbd_->SetValue(dtno);
        dpc_->SetValue(dtno);
        calendarCtrl_->SetDate (dtno);

        int repeatSel = bill->REPEATS;
        // Have used repeatSel to multiplex auto repeat fields.
        if (repeatSel >= BD_REPEATS_MULTIPLEX_BASE)
        {
            repeatSel -= BD_REPEATS_MULTIPLEX_BASE;
            autoExecuteUserAck_ = true;
            itemCheckBoxAutoExeUserAck_->SetValue(true);
            itemCheckBoxAutoExeSilent_->Enable(true);

            if (repeatSel >= BD_REPEATS_MULTIPLEX_BASE)
            {
                repeatSel -= BD_REPEATS_MULTIPLEX_BASE;
                autoExecuteSilent_ = true;
                itemCheckBoxAutoExeSilent_->SetValue(true);
            }
        }

        itemRepeats_->SetSelection(repeatSel);
        setRepeatDetails();
        if (repeatSel == 0) // if none
            textNumRepeats_->SetValue("");

        transaction_type_->SetSelection(Model_Billsdeposits::type(bill));
        updateControlsForTransType();

        payeeID_ = bill->PAYEEID;
        toID_ = bill->TOACCOUNTID;
        accountID_ = bill->ACCOUNTID;
        Model_Account::Data* account = Model_Account::instance().get(accountID_);
        itemAccountName_->SetLabel(account->ACCOUNTNAME);

        for (const auto& item : Model_Billsdeposits::splittransaction(bill)) local_splits_.push_back(item);

        if (!local_splits_.empty())
        {
            bCategory_->SetLabel(_("Split Category"));
            cSplit_->SetValue(true);
        }
        else
        {
            const Model_Category::Data* category = Model_Category::instance().get(categID_);
            const Model_Subcategory::Data* sub_category = (subcategID_ != -1 ? Model_Subcategory::instance().get(subcategID_) : 0);
            bCategory_->SetLabel(Model_Category::full_name(category, sub_category));
        }

        textNotes_->SetValue(bill->NOTES);
        textNumber_->SetValue(bill->TRANSACTIONNUMBER);

        double transAmount = bill->TRANSAMOUNT;

        if (!local_splits_.empty())
        {
            transAmount = Model_Budgetsplittransaction::instance().get_total(local_splits_);
            textAmount_->Enable(false);
        }

        textAmount_->SetValue(transAmount, account);

        if (Model_Billsdeposits::type(bill) == Model_Billsdeposits::TRANSFER)
        {
            Model_Account::Data* to_account = Model_Account::instance().get(toID_);

            payeeID_ = account->ACCOUNTID;
            bPayee_->SetLabel(account->ACCOUNTNAME);
            bTo_->SetLabel(to_account->ACCOUNTNAME);

            // When editing an advanced transaction record, we do not reset the toTransAmount_
            if ((edit_ || enterOccur_) && (toTransAmount_ != transAmount))
            {
                cAdvanced_->SetValue(true);
                SetAdvancedTransferControls(true);
            }
        }
        else
        {
            Model_Payee::Data* payee = Model_Payee::instance().get(payeeID_);
            if (payee)
                bPayee_->SetLabel(payee->PAYEENAME);
        }
    }
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
    repeatTransBoxSizer->Add(repeatDetailsStaticBoxSizer, 0, wxALIGN_CENTER|wxALL, 5);

    wxFlexGridSizer* itemFlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    repeatDetailsStaticBoxSizer->Add(itemFlexGridSizer5, g_flags);

    itemFlexGridSizer5->Add(new wxStaticText( this, wxID_STATIC, _("Account Name")), g_flags);
    itemAccountName_ = new wxButton( this, ID_DIALOG_BD_COMBOBOX_ACCOUNTNAME, _("Select Account"),
                                     wxDefaultPosition, wxSize(180, -1));
    itemFlexGridSizer5->Add(itemAccountName_, g_flags);
    itemAccountName_->SetToolTip(_("Specify the Account that will own the repeating transaction"));

    const auto &accounts = Model_Account::instance().all();
    if (accounts.size() == 1)
    {
        accountID_ = accounts.begin()->ACCOUNTID;
        itemAccountName_->SetLabel(accounts.begin()->ACCOUNTNAME);
        itemAccountName_->Enable(false);
    }

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
    dpcbd_ = new wxDatePickerCtrl( this, ID_DIALOG_BD_BUTTON_NEXTOCCUR, wxDefaultDateTime,
                                   wxDefaultPosition, wxSize(110,-1), wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    dpcbd_->SetToolTip(_("Specify the date of the next bill or deposit"));

    spinNextOccDate_ = new wxSpinButton( this, ID_DIALOG_BD_REPEAT_DATE_SPINNER,
                                         wxDefaultPosition, spinCtrlSize,spinCtrlDirection|wxSP_ARROW_KEYS|wxSP_WRAP);
    spinNextOccDate_->SetToolTip(_("Retard or advance the date of the 'next occurrence"));

    wxBoxSizer* nextOccurDateBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    nextOccurDateBoxSizer->Add(dpcbd_, g_flags);
    nextOccurDateBoxSizer->Add(spinNextOccDate_, g_flags);

    itemFlexGridSizer5->Add(new wxStaticText( this, wxID_STATIC, _("Next Occurrence")), g_flags);
    itemFlexGridSizer5->Add(nextOccurDateBoxSizer);

    // Repeats --------------------------------------------
    staticTextRepeats_ = new wxStaticText( this, wxID_STATIC, _("Repeats") );
    itemFlexGridSizer5->Add(staticTextRepeats_, g_flags);

    itemRepeats_ = new wxChoice( this, ID_DIALOG_BD_COMBOBOX_REPEATS,
        wxDefaultPosition, wxSize(110, -1));
    size_t size = sizeof(BILLSDEPOSITS_REPEATS)/sizeof(wxString);
    for(size_t i = 0; i < size; ++i)
    if ( i <=10 || i>14)
        itemRepeats_->Append(wxGetTranslation(BILLSDEPOSITS_REPEATS[i]));
    else
        itemRepeats_->Append(wxString::Format( wxGetTranslation(BILLSDEPOSITS_REPEATS[i]), "(x)"));


    wxBoxSizer* repeatBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    bSetNextOccurDate_ = new wxButton( this, ID_DIALOG_TRANS_BUTTONTRANSNUM, _("Next"),
                                       wxDefaultPosition, wxSize(60, -1));
    bSetNextOccurDate_->SetToolTip(_("Advance the Next Occurance Date with the specified values"));
    repeatBoxSizer->Add(itemRepeats_, g_flags);
    repeatBoxSizer->Add(bSetNextOccurDate_, g_flags);

    itemFlexGridSizer5->Add(repeatBoxSizer);
    itemRepeats_->SetSelection(0);

    // Repeat Times --------------------------------------------
    staticTimesRepeat_ = new wxStaticText( this, wxID_STATIC, _("Times Repeated") );
    itemFlexGridSizer5->Add(staticTimesRepeat_, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 0);

    textNumRepeats_ = new wxTextCtrl( this, ID_DIALOG_BD_TEXTCTRL_NUM_TIMES, "",
        wxDefaultPosition, wxSize(110, -1), 0, wxIntegerValidator<int>() );
    itemFlexGridSizer5->Add(textNumRepeats_, g_flags);
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
    wxPanel* transactionPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    transDetailsStaticBoxSizer->Add(transactionPanel, 0, wxGROW|wxALL, 10);

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
    choiceStatus_ = new wxChoice( transactionPanel, ID_DIALOG_TRANS_STATUS,
                                  wxDefaultPosition, wxSize(110, -1));

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

    wxStaticText* staticTextAmount = new wxStaticText( transactionPanel, wxID_STATIC, _("Amount"));

    textAmount_ = new mmTextCtrl( transactionPanel, ID_DIALOG_TRANS_TEXTAMOUNT, ""
        , wxDefaultPosition, wxSize(110, -1), wxALIGN_RIGHT|wxTE_PROCESS_ENTER
        , mmCalcValidator());
    textAmount_->SetToolTip(amountNormalTip_);
    textAmount_->Connect(ID_DIALOG_TRANS_TEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER,
        wxCommandEventHandler(mmBDDialog::OnTextEntered), NULL, this);

    toTextAmount_ = new mmTextCtrl(transactionPanel, ID_DIALOG_TRANS_TOTEXTAMOUNT, ""
        , wxDefaultPosition, wxSize(110, -1), wxALIGN_RIGHT|wxTE_PROCESS_ENTER
        , mmCalcValidator());
    toTextAmount_->SetToolTip(_("Specify the transfer amount in the To Account"));
    toTextAmount_->Connect(ID_DIALOG_TRANS_TOTEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER,
        wxCommandEventHandler(mmBDDialog::OnTextEntered), NULL, this);

    wxBoxSizer* amountSizer = new wxBoxSizer(wxHORIZONTAL);
    amountSizer->Add(textAmount_, g_flags);
    amountSizer->Add(toTextAmount_, g_flags);

    transPanelSizer->Add(staticTextAmount, g_flags);
    transPanelSizer->Add(amountSizer);

    // Payee ------------------------------------------------
    wxStaticText* staticTextPayee = new wxStaticText( transactionPanel, ID_DIALOG_TRANS_STATIC_PAYEE,_("Payee") );

    bPayee_ = new wxButton( transactionPanel, ID_DIALOG_TRANS_BUTTONPAYEE, _("Select Payee"),
                            wxDefaultPosition, wxSize(225, -1), 0 );
    payeeWithdrawalTip_ = _("Specify where the transaction is going to");
    payeeDepositTip_    = _("Specify where the transaction is coming from");
    bPayee_->SetToolTip(payeeWithdrawalTip_);

    transPanelSizer->Add(staticTextPayee, g_flags);
    transPanelSizer->Add(bPayee_, g_flags);

    // Payee Alternate ------------------------------------------------
    wxStaticText* staticTextTo = new wxStaticText( transactionPanel, ID_DIALOG_TRANS_STATIC_FROM, " " );
    bTo_ = new wxButton( transactionPanel, ID_DIALOG_TRANS_BUTTONTO, _("Select To Acct"),
                         wxDefaultPosition, wxSize(225, -1), 0 );
    bTo_->SetToolTip(_("Specify which account the transfer is going to"));

    transPanelSizer->Add(staticTextTo, g_flags);
    transPanelSizer->Add(bTo_, g_flags);

    // Split Category -------------------------------------------
    cSplit_ = new wxCheckBox( transactionPanel, ID_DIALOG_TRANS_SPLITCHECKBOX, _("Split"),
                              wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    cSplit_->SetValue(FALSE);
    cSplit_->SetToolTip(_("Use split Categories"));

    transPanelSizer->AddSpacer(20); // Fill empty space.
    transPanelSizer->Add(cSplit_, g_flags);

    // Category ---------------------------------------------
    wxStaticText* staticTextCategory = new wxStaticText( transactionPanel, wxID_STATIC, _("Category"));
    bCategory_ = new wxButton( transactionPanel, ID_DIALOG_TRANS_BUTTONCATEGS, _("Select Category"),
                               wxDefaultPosition, wxSize(225, -1), 0 );
    //bCategory_->SetToolTip(_("Specify the category for this transaction"));

    transPanelSizer->Add(staticTextCategory, g_flags);
    transPanelSizer->Add(bCategory_, g_flags);

    // Number ---------------------------------------------
    textNumber_ = new wxTextCtrl( transactionPanel, ID_DIALOG_TRANS_TEXTNUMBER, "",
                                  wxDefaultPosition, wxSize(225, -1));
    textNumber_->SetToolTip(_("Specify any associated check number or transaction number"));

    transPanelSizer->Add(new wxStaticText( transactionPanel, wxID_STATIC, _("Number")), g_flags);
    transPanelSizer->Add(textNumber_, g_flags);

    // Notes ---------------------------------------------
    textNotes_ = new wxTextCtrl(transactionPanel, ID_DIALOG_TRANS_TEXTNOTES, "",
                                 wxDefaultPosition, wxSize(225, 80), wxTE_MULTILINE );
    textNotes_->SetToolTip(_("Specify any text notes you want to add to this transaction."));

    transPanelSizer->Add(new wxStaticText( transactionPanel, wxID_STATIC, _("Notes")), g_flags);
    wxButton* bFrequentUsedNotes = new wxButton(transactionPanel, ID_DIALOG_TRANS_BUTTON_FREQENTNOTES, "...",
        wxDefaultPosition, wxSize(40, -1));
    bFrequentUsedNotes->SetToolTip(_("Select one of the frequently used notes"));
    bFrequentUsedNotes->Connect(ID_DIALOG_TRANS_BUTTON_FREQENTNOTES, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(mmBDDialog::OnFrequentUsedNotes), NULL, this);
    transPanelSizer->Add(bFrequentUsedNotes, wxSizerFlags(g_flags).Align(wxALIGN_RIGHT));
    box_sizer1->Add(textNotes_, g_flagsExpand);

    SetTransferControls();  // hide appropriate fields
    prevType_ = Model_Billsdeposits::WITHDRAWAL;
    /**********************************************************************************************
     Button Panel with OK and Cancel Buttons
    ***********************************************************************************************/
    wxPanel* buttonsPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
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

void mmBDDialog::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void mmBDDialog::OnAccountName(wxCommandEvent& /*event*/)
{
    mmSingleChoiceDialog scd(this
        , _("Choose Bank Account or Term Account")
        , _("Select Account")
        , Model_Account::instance().all_checking_account_names());

    if (scd.ShowModal() == wxID_OK)
    {
        wxString acctName = scd.GetStringSelection();
        Model_Account::Data* account = Model_Account::instance().get(acctName);
        if (account)
        {
            double amount = 0.0;
            textAmount_->GetDouble(amount);
            textAmount_->SetValue(amount, account);
            if (advancedToTransAmountSet_)
            {
                double toAmount = 0.0;
                toTextAmount_->GetDouble(toAmount);
                toTextAmount_->SetValue(toAmount, account);
            }
            accountID_ = account->ACCOUNTID;
            itemAccountName_->SetLabel(acctName);
            if (transaction_type_->GetSelection() == Model_Billsdeposits::TRANSFER)
            {
                payeeID_ = account->ACCOUNTID;
                bPayee_->SetLabel(acctName);
            }
        }
    }
}

void mmBDDialog::OnPayee(wxCommandEvent& /*event*/)
{
    if (transaction_type_->GetSelection() == Model_Billsdeposits::TRANSFER)
    {
        mmSingleChoiceDialog scd(this, _("Account name"), _("Select Account")
            , Model_Account::instance().all_checking_account_names());
        if (scd.ShowModal() == wxID_OK)
        {
            wxString acctName = scd.GetStringSelection();
            Model_Account::Data* account = Model_Account::instance().get(acctName);
            payeeID_ = account->ACCOUNTID;
            bPayee_->SetLabel(acctName);
            itemAccountName_->SetLabel(acctName);
        }
    }
    else
    {
            mmPayeeDialog dlg(this);

            if ( dlg.ShowModal() == wxID_OK )
            {
                payeeID_ = dlg.getPayeeId();
                Model_Payee::Data* payee = Model_Payee::instance().get(payeeID_);
                if (payee)
                {
                    bPayee_->SetLabel(payee->PAYEENAME);
                    payeeUnknown_ = false;
                    // Only for new transactions: if user want to autofill last category used for payee.
                    // If this is a Split Transaction, ignore displaying last category for payee
                    if (payee->CATEGID != -1 && local_splits_.empty() && mmIniOptions::instance().transCategorySelectionNone_ == 1 && !categUpdated_ && bdID_ == 0)
                    {
                        categID_ = payee->CATEGID;
                        subcategID_ = payee->SUBCATEGID;

                        const Model_Category::Data* category = Model_Category::instance().get(categID_);
                        const Model_Subcategory::Data* sub_category = (subcategID_ != -1 ? Model_Subcategory::instance().get(subcategID_) : 0);
                        bCategory_->SetLabel(Model_Category::full_name(category, sub_category));
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
        wxString acctName = scd.GetStringSelection();
        Model_Account::Data* account = Model_Account::instance().get(acctName);
        toID_ = account->ACCOUNTID;
        bTo_->SetLabel(acctName);
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
        dlg.setTreeSelection(categID_, subcategID_);
        if ( dlg.ShowModal() == wxID_OK )
        {
            categID_ = dlg.getCategId();
            subcategID_ = dlg.getSubCategId();

            const Model_Category::Data* category = Model_Category::instance().get(categID_);
            const Model_Subcategory::Data* sub_category = (subcategID_ != -1 ? Model_Subcategory::instance().get(subcategID_) : 0);
            bCategory_->SetLabel(Model_Category::full_name(category, sub_category));
            categUpdated_ = true;
        }
    }
}

void mmBDDialog::displayControlsForType(Model_Billsdeposits::TYPE transType, bool enableAdvanced)
{
    wxStaticText* stFrom = (wxStaticText*)FindWindow(ID_DIALOG_TRANS_STATIC_FROM);
    stFrom->Enable(enableAdvanced);
    bTo_->Enable(enableAdvanced);

    bPayee_->SetToolTip(_("Specify where the transaction is going to or coming from "));
    textAmount_->SetToolTip(_("Specify the amount for this transaction"));

    if (transType == Model_Billsdeposits::TRANSFER) {
        bPayee_->SetToolTip(_("Specify which account the transfer is comming from"));
        textAmount_->SetToolTip(amountTransferTip_);
    } else {
        bPayee_->SetToolTip(_("Specify to whom the transaction is going to or coming from "));
        textAmount_->SetToolTip(amountNormalTip_);
    }
}

void mmBDDialog::OnTransTypeChanged(wxCommandEvent& /*event*/)
{
    updateControlsForTransType();
}

void mmBDDialog::updateControlsForTransType()
{
    wxStaticText* st = (wxStaticText*)FindWindow(ID_DIALOG_TRANS_STATIC_FROM);
    wxStaticText* stp = (wxStaticText*)FindWindow(ID_DIALOG_TRANS_STATIC_PAYEE);

    if (transaction_type_->GetSelection() == Model_Billsdeposits::WITHDRAWAL)
    {
        displayControlsForType(Model_Billsdeposits::WITHDRAWAL);
        SetTransferControls();
        stp->SetLabel(_("Payee"));
        st->SetLabel("");
        bPayee_->SetToolTip(payeeWithdrawalTip_);
        if (payeeUnknown_)
        {
            payeeID_ = -1;
            toID_ = -1;
            resetPayeeString();
        }
        prevType_ = Model_Billsdeposits::WITHDRAWAL;
    }
    else if (transaction_type_->GetSelection() == Model_Billsdeposits::DEPOSIT)
    {
        displayControlsForType(Model_Billsdeposits::DEPOSIT);
        SetTransferControls();
        stp->SetLabel(_("From"));
        st->SetLabel("");
        bPayee_->SetToolTip(payeeDepositTip_);
        if (payeeUnknown_)
        {
            payeeID_ = -1;
            toID_ = -1;
            resetPayeeString();
        }
        prevType_ = Model_Billsdeposits::DEPOSIT;
    }
    else if (transaction_type_->GetSelection() == Model_Billsdeposits::TRANSFER)
    {
        displayControlsForType(Model_Billsdeposits::TRANSFER, true);
        if (accountID_ < 0)
        {
            bPayee_->SetLabel(_("Select From Account"));
            payeeID_ = -1;
        }
        else
        {
            bPayee_->SetLabel(itemAccountName_->GetLabel());
            payeeID_ = accountID_;
        }

        SetTransferControls(true);
        if (cAdvanced_->IsChecked()) SetAdvancedTransferControls(true);

        stp->SetLabel(_("From"));
        st->SetLabel(_("To"));
        bTo_->SetLabel(_("Select To Account"));
        toID_ = -1;
        payeeUnknown_ = true;
        prevType_ = Model_Billsdeposits::TRANSFER;
    }
}

void mmBDDialog::resetPayeeString()
{
    wxString payeeStr = _("Select Payee");
    if (payeeID_ == -1)
    {
        const auto &filtd = Model_Payee::instance().FilterPayees("");
        if (filtd.size() == 1)
        {
            //only one payee present. Choose it
            payeeStr = filtd[0].PAYEENAME;
            payeeID_ = filtd[0].PAYEEID;
            payeeUnknown_ = false;

            // Only for new transactions: if user want to autofill last category used for payee.
            // If this is a Split Transaction, ignore displaying last category for payee
            if (filtd[0].CATEGID != -1 && local_splits_.empty() && mmIniOptions::instance().transCategorySelectionNone_ == 1 && !categUpdated_ && bdID_ == 0)
            {
                categID_ = filtd[0].CATEGID;
                subcategID_ = filtd[0].SUBCATEGID;

                const Model_Category::Data* category = Model_Category::instance().get(categID_);
                const Model_Subcategory::Data* sub_category = (subcategID_ != -1 ? Model_Subcategory::instance().get(subcategID_) : 0);
                bCategory_->SetLabel(Model_Category::full_name(category, sub_category));
            }
        }
    }
    bPayee_->SetLabel(payeeStr);
}

void mmBDDialog::OnFrequentUsedNotes(wxCommandEvent& WXUNUSED(event))
{
    Model_Checking::getFrequentUsedNotes(accountID_, frequentNotes_);
    wxMenu menu;
    for (int id = 0; id < (int)frequentNotes_.size(); id++)
        menu.Append(id + 1, frequentNotes_[id].first);
    if (frequentNotes_.size() > 0)
        PopupMenu(&menu);
}

void mmBDDialog::onNoteSelected(wxCommandEvent& event)
{
    int i = event.GetId();
    if (i > 0)
        *textNotes_ << frequentNotes_[i - 1].second;
}

void mmBDDialog::OnOk(wxCommandEvent& /*event*/)
{
    if (payeeID_ == -1)
    {
        if (transaction_type_->GetSelection() != Model_Billsdeposits::TRANSFER)
            mmShowErrorMessageInvalid(this, _("Payee"));
        else
            mmShowErrorMessageInvalid(this, _("From Account"));
        return;
    }

    if (cSplit_->GetValue())
    {
        if (local_splits_.empty())
        {
            mmShowErrorMessageInvalid(this, _("Category"));
            return;
        }
    }
    else
    {
        if (categID_ == -1)
        {
            mmShowErrorMessageInvalid(this, _("Category"));
            return;
        }
    }

    double amount = 0;
    if (cSplit_->GetValue())
    {
        amount = Model_Budgetsplittransaction::instance().get_total(local_splits_);
        if (amount < 0.0)
        {
            if (transaction_type_->GetSelection() == Model_Billsdeposits::TRANSFER) {
                if (amount < 0)
                    amount = - amount;
            } else {
                mmShowErrorMessageInvalid(this, _("Amount"));
                return;
            }
        }
    }
    else
    {
        if (!textAmount_->checkValue(amount))
        {
            return;
        }
    }

    if (advancedToTransAmountSet_)
    {
        if (!toTextAmount_->checkValue(toTransAmount_))
        {
            return;
        }
    }
    else
        toTransAmount_ = amount;

    if ((transaction_type_->GetSelection() != Model_Billsdeposits::TRANSFER) && (accountID_ == -1))
    {
        mmShowErrorMessageInvalid(this, _("Account"));
        return;
    }

    int toAccountID = -1;
    int fromAccountID = accountID_;
    if (transaction_type_->GetSelection() == Model_Billsdeposits::TRANSFER)
    {
        if (toID_ == -1)
        {
            mmShowErrorMessageInvalid(this, _("To Account"));
            return;
        }

        if (payeeID_ == toID_)
        {
            mmShowErrorMessage(this, _("From and To Account cannot be the same."), _("Error"));
            return;
        }

        fromAccountID = payeeID_;
        toAccountID = toID_;
        payeeID_ = -1;
    }

    if (!advancedToTransAmountSet_ || toTransAmount_ < 0)
    {
        // if we are adding a new record and the user did not touch advanced dialog
        // we are going to use the transfer amount by calculating conversion rate.
        // subsequent edits will not allow automatic update of the amount
        if (!edit_)
        {
            if(toAccountID != -1) 
            {
                Model_Account::Data* from_account = Model_Account::instance().get(fromAccountID);
                Model_Account::Data* to_account = Model_Account::instance().get(toAccountID);

                Model_Currency::Data* from_currency = Model_Account::currency(from_account);
                Model_Currency::Data* to_currency = Model_Account::currency(to_account);

                double rateFrom = from_currency->BASECONVRATE; 
                double rateTo = to_currency->BASECONVRATE;

                double convToBaseFrom = rateFrom * amount;
                toTransAmount_ = convToBaseFrom / rateTo;
            } else {
                toTransAmount_ = amount;
            }
        }
    }

    wxString transNum = textNumber_->GetValue();
    wxString notes = textNotes_->GetValue();

    // Multiplex Auto executable onto the repeat field of the database.
    int repeats = itemRepeats_->GetSelection();
    if (autoExecuteUserAck_)
        repeats += BD_REPEATS_MULTIPLEX_BASE;
    if (autoExecuteSilent_)
        repeats += BD_REPEATS_MULTIPLEX_BASE;

    wxString numRepeatStr = textNumRepeats_->GetValue();
    int numRepeats = -1;

    if (!numRepeatStr.empty()) {
        long cnt = 0;
        if (numRepeatStr.ToLong(&cnt))
        {
            wxASSERT(std::numeric_limits<int>::min() <= cnt);
            wxASSERT(cnt <= std::numeric_limits<int>::max());
            numRepeats = static_cast<int>(cnt);
        }
    }

    wxString nextOccurDate = dpcbd_->GetValue().FormatISODate();

    wxString status = "";
    wxStringClientData* status_obj = (wxStringClientData *)choiceStatus_->GetClientObject(choiceStatus_->GetSelection());
    if (status_obj) status = Model_Billsdeposits::toShortStatus(status_obj->GetData());

    wxString date1 = dpc_->GetValue().FormatISODate();

    if (!edit_ && !enterOccur_)
    {
        Model_Billsdeposits::Data* bill = Model_Billsdeposits::instance().create();
        bill->ACCOUNTID = fromAccountID;
        bill->TOACCOUNTID = toAccountID;
        bill->PAYEEID = payeeID_;
        bill->TRANSCODE = Model_Billsdeposits::all_type()[transaction_type_->GetSelection()];
        bill->TRANSAMOUNT = amount;
        bill->STATUS = status;
        bill->TRANSACTIONNUMBER = transNum;
        bill->NOTES = notes;
        bill->CATEGID = categID_;
        bill->SUBCATEGID = subcategID_;
        bill->TRANSDATE = date1;
        bill->TOTRANSAMOUNT = toTransAmount_;
        bill->REPEATS = repeats;
        bill->NEXTOCCURRENCEDATE = nextOccurDate;
        bill->NUMOCCURRENCES = numRepeats;

        transID_ = Model_Billsdeposits::instance().save(bill);

        for (auto &item : local_splits_) item.TRANSID = transID_;
        Model_Budgetsplittransaction::instance().save(local_splits_);
    }
    else if (edit_)
    {
        Model_Billsdeposits::Data * bill = Model_Billsdeposits::instance().get(bdID_);
        bill->ACCOUNTID = accountID_;
        bill->TOACCOUNTID = toAccountID;
        bill->PAYEEID = payeeID_;
        bill->TRANSCODE = Model_Billsdeposits::all_type()[transaction_type_->GetSelection()];
        bill->TRANSAMOUNT = amount;
        bill->STATUS = status;
        bill->TRANSACTIONNUMBER = transNum;
        bill->NOTES = notes;
        bill->CATEGID = categID_;
        bill->SUBCATEGID = subcategID_;
        bill->TRANSDATE = date1;
        bill->TOTRANSAMOUNT = toTransAmount_;
        bill->REPEATS = repeats;
        bill->NEXTOCCURRENCEDATE = nextOccurDate;
        bill->NUMOCCURRENCES = numRepeats;

        transID_ = Model_Billsdeposits::instance().save(bill);

        for (auto& item : Model_Billsdeposits::splittransaction(bill))
            Model_Budgetsplittransaction::instance().remove(item.SPLITTRANSID);

        for (auto &item : local_splits_)
        {
            item.SPLITTRANSID = -1;
            item.TRANSID = transID_;
        }
        Model_Budgetsplittransaction::instance().save(local_splits_);
    }
    else if (enterOccur_)
    {
        // repeats now hold extra info. Need to get repeats from dialog selection
        if ( (itemRepeats_->GetSelection() < 11) || (itemRepeats_->GetSelection() > 14) || (numRepeats > 0) )
        {
            Model_Checking::Data* tran = Model_Checking::instance().create();
            tran->ACCOUNTID = fromAccountID;
            tran->TOACCOUNTID = toAccountID;
            tran->PAYEEID = payeeID_;
            tran->TRANSCODE = Model_Billsdeposits::all_type()[transaction_type_->GetSelection()];
            tran->TRANSAMOUNT = amount;
            tran->STATUS = status;
            tran->TRANSACTIONNUMBER = transNum;
            tran->NOTES = notes;
            tran->CATEGID = categID_;
            tran->SUBCATEGID = subcategID_;
            tran->TRANSDATE = date1;
            tran->TOTRANSAMOUNT = toTransAmount_;

            int transID = Model_Checking::instance().save(tran);

            Model_Splittransaction::Data_Set checking_splits;
            for (auto &item : local_splits_)
            {
                Model_Splittransaction::Data *split = Model_Splittransaction::instance().create();
                split->TRANSID = transID;
                split->CATEGID = item.CATEGID;
                split->SUBCATEGID = item.SUBCATEGID;
                split->SPLITTRANSAMOUNT = item.SPLITTRANSAMOUNT;
                checking_splits.push_back(*split);
            }
            Model_Splittransaction::instance().save(checking_splits);
        }
        Model_Billsdeposits::instance().completeBDInSeries(bdID_);
    }
    EndModal(wxID_OK);
}

void mmBDDialog::OnSplitChecked(wxCommandEvent& /*event*/)
{
    if (local_splits_.empty() && categID_ > -1)
    {
        bool bDeposit = transaction_type_->GetSelection() == Model_Billsdeposits::DEPOSIT;
        double amount;
        if (!textAmount_->GetDouble(amount))
            amount = 0;

        Model_Budgetsplittransaction::Data *split = Model_Budgetsplittransaction::instance().create();
        split->CATEGID = categID_;
        split->SUBCATEGID = subcategID_;
        split->SPLITTRANSAMOUNT = bDeposit ? amount : amount;
        local_splits_.push_back(*split);
    }

    categID_ = -1;
    subcategID_ = -1;

    bool state = cSplit_->GetValue();
    SetSplitControls(state);
}

void mmBDDialog::SetSplitControls(bool split)
{
    if (split)
    {
        bCategory_->SetLabel(_("Split Category"));
        textAmount_->Enable(false);
        textAmount_->SetValue(Model_Budgetsplittransaction::instance().get_total(local_splits_));
        activateSplitTransactionsDlg();
    }
    else
    {
        bCategory_->SetLabel(_("Select Category"));
        textAmount_->Enable(true);
        textAmount_->SetValue(0.0);
        local_splits_.clear();
    }
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
        dpcbd_->SetValue(date) ;
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
            bTo_->Show();
        }
    }
    else
    {
        if (!(prevType_ == Model_Billsdeposits::WITHDRAWAL || prevType_ == Model_Billsdeposits::DEPOSIT))
        {
            bTo_->Hide();
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
        advancedToTransAmountSet_ = true;
        // Display the transfer amount in the toTextAmount control.
        if (toTransAmount_ >= 0)
        {
            toTextAmount_->SetValue(toTransAmount_);
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
        advancedToTransAmountSet_ = false;
    }
}

void mmBDDialog::SetNewDate(wxDatePickerCtrl* dpc, bool forward)
{
    int day = -1;
    if (forward) day = 1;

    wxDateTime date = dpc->GetValue().Add(wxDateSpan::Days(day));

    dpc_->SetValue( date );
    dpcbd_->SetValue( date );
    calendarCtrl_->SetDate(date);
}

void mmBDDialog::OnNextOccurDateForward(wxSpinEvent& /*event*/)
{
    SetNewDate(dpcbd_);
}

void mmBDDialog::OnNextOccurDateBack(wxSpinEvent& /*event*/)
{
    SetNewDate(dpcbd_, false);
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
    wxString repeatLabelRepeats  = _("Repeats");
    wxString repeatLabelActivate = _("Activates");

    wxString timeLabelDays   = _("Period: Days");
    wxString timeLabelMonths = _("Period: Months");
    wxString toolTipsStr = wxEmptyString;

    bSetNextOccurDate_->Disable();
    int repeats = itemRepeats_->GetSelection();
    if (repeats == 11)
    {
        staticTextRepeats_->SetLabel( repeatLabelActivate );
        staticTimesRepeat_->SetLabel( timeLabelDays);
        toolTipsStr << _("Specify period in Days to activate.") << "\n" << _("Becomes blank when not active.");
        textNumRepeats_->SetToolTip(toolTipsStr);
    }
    else if (repeats == 12)
    {
        staticTextRepeats_->SetLabel(repeatLabelActivate );
        staticTimesRepeat_->SetLabel(timeLabelMonths);
        toolTipsStr << _("Specify period in Months to activate.") << "\n" << _("Becomes blank when not active.");
        textNumRepeats_->SetToolTip(toolTipsStr);
    }
    else if (repeats == 13)
    {
        staticTextRepeats_->SetLabel(repeatLabelRepeats);
        staticTimesRepeat_->SetLabel(timeLabelDays);
        toolTipsStr << _("Specify period in Days to activate.") << "\n" << _("Leave blank when not active.");
        textNumRepeats_->SetToolTip(toolTipsStr);
    }
    else if (repeats == 14)
    {
        staticTextRepeats_->SetLabel(repeatLabelRepeats);
        staticTimesRepeat_->SetLabel(timeLabelMonths);
        toolTipsStr << _("Specify period in Months to activate.") << "\n" << _("Leave blank when not active.");
        textNumRepeats_->SetToolTip(toolTipsStr);
    }
    else
    {
        staticTextRepeats_->SetLabel(repeatLabelRepeats);
        staticTimesRepeat_->SetLabel( _("Times Repeated") );
        toolTipsStr << _("Specify the number of times this series repeats.") << "\n" << _("Leave blank if this series continues forever.");
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
        wxDateTime  date = dpcbd_->GetValue();

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
        dpcbd_->SetValue( date );
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
    Model_Splittransaction::Data_Set checking_splits;
    for (auto &item : local_splits_)
    {
        Model_Splittransaction::Data *split = Model_Splittransaction::instance().create();
        split->CATEGID = item.CATEGID;
        split->SUBCATEGID = item.SUBCATEGID;
        split->SPLITTRANSAMOUNT = item.SPLITTRANSAMOUNT;
        checking_splits.push_back(*split);
    }

    SplitTransactionDialog dlg(this, &checking_splits, transaction_type_->GetSelection(), accountID_);
    if (dlg.ShowModal() == wxID_OK)
    {
        double amount = Model_Splittransaction::instance().get_total(checking_splits);
        if (transaction_type_->GetSelection() == Model_Billsdeposits::TRANSFER && amount < 0) amount = -amount;
        textAmount_->SetValue(amount);

        local_splits_.clear();
        for (auto &item : checking_splits)
        {
            Model_Budgetsplittransaction::Data *split = Model_Budgetsplittransaction::instance().create();
            split->CATEGID = item.CATEGID;
            split->SUBCATEGID = item.SUBCATEGID;
            split->SPLITTRANSAMOUNT = item.SPLITTRANSAMOUNT;
            local_splits_.push_back(*split);
        }
    }
}

void mmBDDialog::OnTextEntered(wxCommandEvent& event)
{
    Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();
    Model_Account::Data *account = Model_Account::instance().get(accountID_);
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

