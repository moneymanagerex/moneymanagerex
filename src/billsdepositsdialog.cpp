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
#include "constants.h"
#include "util.h"
#include "mmCurrencyFormatter.h"
#include "mmOption.h"
#include "paths.h"
#include "categdialog.h"
#include "payeedialog.h"
#include "splittransactionsdialog.h"
#include "validators.h"
#include <wx/valnum.h>

IMPLEMENT_DYNAMIC_CLASS( mmBDDialog, wxDialog )

BEGIN_EVENT_TABLE( mmBDDialog, wxDialog )
    EVT_BUTTON(wxID_OK, mmBDDialog::OnOk)
    EVT_BUTTON(ID_DIALOG_TRANS_BUTTONCATEGS, mmBDDialog::OnCategs)
    EVT_BUTTON(ID_DIALOG_TRANS_BUTTONPAYEE, mmBDDialog::OnPayee)
    EVT_BUTTON(ID_DIALOG_TRANS_BUTTONTO, mmBDDialog::OnTo)
    EVT_CHOICE(ID_DIALOG_TRANS_TYPE, mmBDDialog::OnTransTypeChanged)
    EVT_SPIN_UP(ID_DIALOG_TRANS_DATE_SPINNER,mmBDDialog::OnTransDateForward)
    EVT_SPIN_DOWN(ID_DIALOG_TRANS_DATE_SPINNER,mmBDDialog::OnTransDateBack)
    EVT_SPIN_UP(ID_DIALOG_BD_REPEAT_DATE_SPINNER,mmBDDialog::OnNextOccurDateForward)
    EVT_SPIN_DOWN(ID_DIALOG_BD_REPEAT_DATE_SPINNER,mmBDDialog::OnNextOccurDateBack)
    EVT_CHECKBOX(ID_DIALOG_TRANS_ADVANCED_CHECKBOX, mmBDDialog::OnAdvanceChecked)
    EVT_BUTTON(ID_DIALOG_BD_COMBOBOX_ACCOUNTNAME, mmBDDialog::OnAccountName)
    EVT_CHECKBOX(ID_DIALOG_TRANS_SPLITCHECKBOX, mmBDDialog::OnSplitChecked)
    EVT_CHECKBOX(ID_DIALOG_BD_CHECKBOX_AUTO_EXECUTE_USERACK, mmBDDialog::OnAutoExecutionUserAckChecked)
    EVT_CHECKBOX(ID_DIALOG_BD_CHECKBOX_AUTO_EXECUTE_SILENT, mmBDDialog::OnAutoExecutionSilentChecked)
    EVT_CALENDAR_SEL_CHANGED(ID_DIALOG_BD_CALENDAR, mmBDDialog::OnCalendarSelChanged)
    EVT_DATE_CHANGED(ID_DIALOG_TRANS_BUTTONDATE, mmBDDialog::OnDateChanged)
    EVT_DATE_CHANGED(ID_DIALOG_BD_BUTTON_NEXTOCCUR, mmBDDialog::OnDateChanged)
    EVT_CHOICE(ID_DIALOG_BD_COMBOBOX_REPEATS, mmBDDialog::OnRepeatTypeChanged)
    EVT_BUTTON(ID_DIALOG_TRANS_BUTTONTRANSNUM, mmBDDialog::OnsetNextRepeatDate)
    EVT_TEXT(ID_DIALOG_BD_TEXTCTRL_NUM_TIMES,mmBDDialog::OnPeriodChange)
END_EVENT_TABLE()

// Defines for Transaction Status and Type now located in dbWrapper.h

const wxString REPEAT_TRANSACTIONS_MSGBOX_HEADING = _("Repeat Transaction - Auto Execution Checking");

mmBDDialog::mmBDDialog( )
{
}

mmBDDialog::mmBDDialog(mmCoreDB* core, int bdID, bool edit, bool enterOccur,
                       wxWindow* parent, wxWindowID id, const wxString& caption,
                       const wxPoint& pos, const wxSize& size, long style )
:core_(core), bdID_(bdID), edit_(edit)
    , categID_(-1), subcategID_(-1), payeeID_(-1), accountID_(-1), toID_(-1)
    , toTransAmount_(-1), enterOccur_(enterOccur)
    , advancedToTransAmountSet_(false), payeeUnknown_(true)
    , autoExecuteUserAck_(false), autoExecuteSilent_(false)
{
    Create(parent, id, caption, pos, size, style);
}

bool mmBDDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption,
                           const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());

    std::shared_ptr<mmSplitTransactionEntries> split(new mmSplitTransactionEntries());
    split_ = split;

    /**********************************************************************************************
     Ament controls according to function settings
    ***********************************************************************************************/
    if (edit_ || enterOccur_)
    {
        dataToControls();
        if (! enterOccur_)
        {
            dpc_->Disable();
        }
        else
        {
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
    static const char billsdepositsSql[] =

        "select BDID, ACCOUNTID, TOACCOUNTID, PAYEEID, TRANSCODE, TRANSAMOUNT, STATUS, TRANSACTIONNUMBER, NOTES, CATEGID, "
        "SUBCATEGID, FOLLOWUPID, TOTRANSAMOUNT, REPEATS, NUMOCCURRENCES, "
        "NEXTOCCURRENCEDATE, date(TRANSDATE, 'localtime') as TRANSDATE "
        "from BILLSDEPOSITS_V1 where BDID = ? ";
        // Removed "date(NEXTOCCURRENCEDATE, 'localtime') as NEXTOCCURRENCEDATE, "
        // because causing problems with some systems and in different time zones

    wxSQLite3Statement st = core_->db_.get()->PrepareStatement(billsdepositsSql);
    st.Bind(1, bdID_);

    wxSQLite3ResultSet q1 = st.ExecuteQuery();

    if (q1.NextRow())
    {
        categID_ = q1.GetInt("CATEGID");
        subcategID_ = q1.GetInt("SUBCATEGID");

        wxString transNumString = q1.GetString("TRANSACTIONNUMBER");
        wxString notesString  = q1.GetString("NOTES");
        wxString transTypeString = q1.GetString("TRANSCODE");
        double transAmount = q1.GetDouble("TRANSAMOUNT");
        toTransAmount_ = q1.GetDouble("TOTRANSAMOUNT");

        wxString statusString  = q1.GetString("STATUS");
        //wxString statusString  = pBankTransaction_->status_;
        if (statusString == "") statusString = "N";
        choiceStatus_->SetSelection(wxString("NRVFD").Find(statusString));

        wxString numRepeatStr  = q1.GetString("NUMOCCURRENCES");
        if (numRepeatStr != "-1")
            textNumRepeats_->SetValue(numRepeatStr);

        wxDateTime dtno =  q1.GetDate("NEXTOCCURRENCEDATE");
        wxString dtnostr = mmGetDateForDisplay(dtno);
        dpcbd_->SetValue(dtno);
        dpc_->SetValue(dtno);
        calendarCtrl_->SetDate (dtno);

        int repeatSel = q1.GetInt("REPEATS");
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

        if (transTypeString == TRANS_TYPE_WITHDRAWAL_STR)
            transaction_type_->SetSelection(DEF_WITHDRAWAL);
        else if (transTypeString == TRANS_TYPE_DEPOSIT_STR)
            transaction_type_->SetSelection(DEF_DEPOSIT);
        else if (transTypeString == TRANS_TYPE_TRANSFER_STR)
            transaction_type_->SetSelection(DEF_TRANSFER);
        updateControlsForTransType();

        payeeID_ = q1.GetInt("PAYEEID");
        toID_ = q1.GetInt("TOACCOUNTID");
        accountID_ = q1.GetInt("ACCOUNTID");
        wxString accountName = core_->accountList_.GetAccountName(accountID_);
        itemAccountName_->SetLabel(accountName);

        split_->loadFromBDDB(core_, bdID_);

        if (split_->numEntries() > 0)
        {
            bCategory_->SetLabel(_("Split Category"));
            cSplit_->SetValue(true);
        }
        else
        {
            wxString categString = core_->categoryList_.GetFullCategoryString(categID_, subcategID_);
            categoryName_ = core_->categoryList_.GetCategoryName(categID_);
            subCategoryName_ = core_->categoryList_.GetSubCategoryName(categID_, subcategID_);
            bCategory_->SetLabel(categString);
        }

        textNotes_->SetValue(notesString);
        textNumber_->SetValue(transNumString);

        if (split_->numEntries() > 0)
        {
            transAmount = split_->getTotalSplits();
            textAmount_->Enable(false);
        }
        wxString dispAmount = CurrencyFormatter::float2String(transAmount);
        textAmount_->SetValue(dispAmount);

        wxString payeeString = core_->payeeList_.GetPayeeName(payeeID_);
        bPayee_->SetLabel(payeeString);

        if (transTypeString == TRANS_TYPE_TRANSFER_STR)
        {
            wxString fromAccount = core_->accountList_.GetAccountName(accountID_);
            wxString toAccount = core_->accountList_.GetAccountName(toID_);

            bPayee_->SetLabel(fromAccount);
            bTo_->SetLabel(toAccount);
            payeeID_ = accountID_;

            // When editing an advanced transaction record, we do not reset the toTransAmount_
            if ((edit_ || enterOccur_) && (toTransAmount_ != transAmount))
            {
                cAdvanced_->SetValue(true);
                SetAdvancedTransferControls(true);
            }
        }
    }

    st.Finalize();
    st.Reset();
}

void mmBDDialog::CreateControls()
{
    int border = 5;
    wxSizerFlags flags, flagsExpand;
    flags.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, border);
    flagsExpand.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, border).Expand();

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
    repeatDetailsStaticBoxSizer->Add(itemFlexGridSizer5, flags);

    itemFlexGridSizer5->Add(new wxStaticText( this, wxID_STATIC, _("Account Name")), flags);
    itemAccountName_ = new wxButton( this, ID_DIALOG_BD_COMBOBOX_ACCOUNTNAME, _("Select Account"),
                                     wxDefaultPosition, wxSize(180, -1));
    if (core_->accountList_.getNumBankAccounts() == 1)
    {
        wxArrayString accName = core_->accountList_.getAccountsName();
        wxString accNameStr = accName[0];
        itemAccountName_->SetLabel(accNameStr);
        accountID_= core_->accountList_.GetAccountId(accNameStr);
    };
    itemFlexGridSizer5->Add(itemAccountName_, flags);
    itemAccountName_->SetToolTip(_("Specify the Account that will own the repeating transaction"));

// change properties depending on system parameters
    wxSize spinCtrlSize = wxSize(16,-1);
    int spinCtrlDirection = wxSP_VERTICAL;
    int interval = 0;
#ifdef __WXMSW__
    spinCtrlSize = wxSize(18,22);
//    spinCtrlDirection = wxSP_HORIZONTAL;
    interval = 4;
#endif

    // Next Occur Date --------------------------------------------
    dpcbd_ = new wxDatePickerCtrl( this, ID_DIALOG_BD_BUTTON_NEXTOCCUR, wxDefaultDateTime,
                                   wxDefaultPosition, wxSize(110,-1), wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    dpcbd_->SetToolTip(_("Specify the date of the next bill or deposit"));

    spinNextOccDate_ = new wxSpinButton( this, ID_DIALOG_BD_REPEAT_DATE_SPINNER,
                                         wxDefaultPosition, spinCtrlSize,spinCtrlDirection|wxSP_ARROW_KEYS|wxSP_WRAP);
    spinNextOccDate_->SetToolTip(_("Retard or advance the date of the 'next occurrence"));

    wxBoxSizer* nextOccurDateBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    nextOccurDateBoxSizer->Add(dpcbd_, flags);
    nextOccurDateBoxSizer->Add(spinNextOccDate_, flags);

    itemFlexGridSizer5->Add(new wxStaticText( this, wxID_STATIC, _("Next Occurrence")), flags);
    itemFlexGridSizer5->Add(nextOccurDateBoxSizer);

    // Repeats --------------------------------------------
    staticTextRepeats_ = new wxStaticText( this, wxID_STATIC, _("Repeats") );
    itemFlexGridSizer5->Add(staticTextRepeats_, flags);

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
    repeatBoxSizer->Add(itemRepeats_, flags);
    repeatBoxSizer->Add(bSetNextOccurDate_, flags);

    itemFlexGridSizer5->Add(repeatBoxSizer);
    itemRepeats_->SetSelection(0);

    // Repeat Times --------------------------------------------
    staticTimesRepeat_ = new wxStaticText( this, wxID_STATIC, _("Times Repeated") );
    itemFlexGridSizer5->Add(staticTimesRepeat_, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL|wxADJUST_MINSIZE, 0);

    textNumRepeats_ = new wxTextCtrl( this, ID_DIALOG_BD_TEXTCTRL_NUM_TIMES, "",
        wxDefaultPosition, wxSize(110, -1), 0, wxIntegerValidator<int>() );
    itemFlexGridSizer5->Add(textNumRepeats_, flags);
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

    repeatTransBoxSizer->Add(itemCheckBoxAutoExeUserAck_, flags);
    repeatTransBoxSizer->Add(itemCheckBoxAutoExeSilent_, flags);

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
    transDateBoxSizer->Add(dpc_, flags);
    transDateBoxSizer->Add(spinTransDate_, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxLEFT, interval);

    transPanelSizer->Add(new wxStaticText( transactionPanel, wxID_STATIC, _("Date")), flags);
    transPanelSizer->Add(transDateBoxSizer);

    // Status --------------------------------------------
    choiceStatus_ = new wxChoice( transactionPanel, ID_DIALOG_TRANS_STATUS,
                                  wxDefaultPosition, wxSize(110, -1));

    for(size_t i = 0; i < sizeof(TRANSACTION_STATUS)/sizeof(wxString); ++i)
        choiceStatus_->Append(wxGetTranslation(TRANSACTION_STATUS[i]),
        new wxStringClientData(TRANSACTION_STATUS[i]));
    choiceStatus_->SetSelection(mmIniOptions::instance().transStatusReconciled_);
    choiceStatus_->SetToolTip(_("Specify the status for the transaction"));

    transPanelSizer->Add(new wxStaticText( transactionPanel, wxID_STATIC, _("Status")), flags);
    transPanelSizer->Add(choiceStatus_, flags);

    // Type --------------------------------------------
    transaction_type_ = new wxChoice( transactionPanel, ID_DIALOG_TRANS_TYPE,
                                 wxDefaultPosition, wxSize(110, -1));

    // Restrict choise if accounts number less than 2
    size = sizeof(TRANSACTION_TYPE)/sizeof(wxString);
    if (core_->accountList_.getNumBankAccounts() < 2) size--;
    for(size_t i = 0; i < size; ++i)
    transaction_type_->Append(wxGetTranslation(TRANSACTION_TYPE[i]),
        new wxStringClientData(TRANSACTION_TYPE[i]));

    transaction_type_->SetSelection(0);
    transaction_type_->SetToolTip(_("Specify the type of transactions to be created."));
    cAdvanced_ = new wxCheckBox( transactionPanel, ID_DIALOG_TRANS_ADVANCED_CHECKBOX, _("Advanced"),
                                 wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    cAdvanced_->SetValue(false);
    cAdvanced_->SetToolTip(_("Allows the setting of different amounts in the FROM and TO accounts."));

    wxBoxSizer* typeSizer = new wxBoxSizer(wxHORIZONTAL);
    typeSizer->Add(transaction_type_, flags);
    typeSizer->Add(cAdvanced_, flags);

    transPanelSizer->Add(new wxStaticText( transactionPanel, wxID_STATIC, _("Type")), flags);
    transPanelSizer->Add(typeSizer);

    // Amount Fields --------------------------------------------
    amountNormalTip_   = _("Specify the amount for this transaction");
    amountTransferTip_ = _("Specify the amount to be transfered");

    wxStaticText* staticTextAmount = new wxStaticText( transactionPanel, wxID_STATIC, _("Amount"));

    textAmount_ = new wxTextCtrl( transactionPanel, ID_DIALOG_TRANS_TEXTAMOUNT, ""
        , wxDefaultPosition, wxSize(110, -1), wxALIGN_RIGHT|wxTE_PROCESS_ENTER
        , mmDoubleValidator2() );
    textAmount_->SetToolTip(amountNormalTip_);

    toTextAmount_ = new wxTextCtrl( transactionPanel, ID_DIALOG_TRANS_TEXTAMOUNT, ""
        , wxDefaultPosition, wxSize(110, -1), wxALIGN_RIGHT|wxTE_PROCESS_ENTER
        , mmDoubleValidator2() );
    toTextAmount_->SetToolTip(_("Specify the transfer amount in the To Account"));

    wxBoxSizer* amountSizer = new wxBoxSizer(wxHORIZONTAL);
    amountSizer->Add(textAmount_,   flags);
    amountSizer->Add(toTextAmount_, flags);

    transPanelSizer->Add(staticTextAmount, flags);
    transPanelSizer->Add(amountSizer);

    // Payee ------------------------------------------------
    wxStaticText* staticTextPayee = new wxStaticText( transactionPanel, ID_DIALOG_TRANS_STATIC_PAYEE,_("Payee") );

    bPayee_ = new wxButton( transactionPanel, ID_DIALOG_TRANS_BUTTONPAYEE, _("Select Payee"),
                            wxDefaultPosition, wxSize(225, -1), 0 );
    payeeWithdrawalTip_ = _("Specify where the transaction is going to");
    payeeDepositTip_    = _("Specify where the transaction is coming from");
    bPayee_->SetToolTip(payeeWithdrawalTip_);
    resetPayeeString();

    transPanelSizer->Add(staticTextPayee, flags);
    transPanelSizer->Add(bPayee_, flags);

    // Payee Alternate ------------------------------------------------
    wxStaticText* staticTextTo = new wxStaticText( transactionPanel, ID_DIALOG_TRANS_STATIC_FROM, " " );
    bTo_ = new wxButton( transactionPanel, ID_DIALOG_TRANS_BUTTONTO, _("Select To Acct"),
                         wxDefaultPosition, wxSize(225, -1), 0 );
    bTo_->SetToolTip(_("Specify which account the transfer is going to"));

    transPanelSizer->Add(staticTextTo, flags);
    transPanelSizer->Add(bTo_, flags);

    // Split Category -------------------------------------------
    cSplit_ = new wxCheckBox( transactionPanel, ID_DIALOG_TRANS_SPLITCHECKBOX, _("Split"),
                              wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    cSplit_->SetValue(FALSE);
    cSplit_->SetToolTip(_("Use split Categories"));

    transPanelSizer->AddSpacer(20); // Fill empty space.
    transPanelSizer->Add(cSplit_, flags);

    // Category ---------------------------------------------
    wxStaticText* staticTextCategory = new wxStaticText( transactionPanel, wxID_STATIC, _("Category"));
    bCategory_ = new wxButton( transactionPanel, ID_DIALOG_TRANS_BUTTONCATEGS, _("Select Category"),
                               wxDefaultPosition, wxSize(225, -1), 0 );
    //bCategory_->SetToolTip(_("Specify the category for this transaction"));

    transPanelSizer->Add(staticTextCategory, flags);
    transPanelSizer->Add(bCategory_, flags);

    // Number ---------------------------------------------
    textNumber_ = new wxTextCtrl( transactionPanel, ID_DIALOG_TRANS_TEXTNUMBER, "",
                                  wxDefaultPosition, wxSize(225, -1));
    textNumber_->SetToolTip(_("Specify any associated check number or transaction number"));

    transPanelSizer->Add(new wxStaticText( transactionPanel, wxID_STATIC, _("Number")), flags);
    transPanelSizer->Add(textNumber_, flags);

    // Notes ---------------------------------------------
    textNotes_ = new wxTextCtrl( transactionPanel, ID_DIALOG_TRANS_TEXTNOTES, "",
                                 wxDefaultPosition, wxSize(225, 80), wxTE_MULTILINE );
    textNotes_->SetToolTip(_("Specify any text notes you want to add to this transaction."));

    transPanelSizer->Add(new wxStaticText( transactionPanel, wxID_STATIC, _("Notes")), flags);
    transPanelSizer->AddSpacer(1);
    box_sizer1->Add(textNotes_, flagsExpand);

    SetTransferControls();  // hide appropriate fields
    /**********************************************************************************************
     Button Panel with OK and Cancel Buttons
    ***********************************************************************************************/
    wxPanel* buttonsPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    wxBoxSizer* buttonsPanelSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonsPanel->SetSizer(buttonsPanelSizer);

    wxButton* okButton = new wxButton( buttonsPanel, wxID_OK);
    buttonsPanelSizer->Add(okButton, flags);

    wxButton* cancelButton = new wxButton( buttonsPanel, wxID_CANCEL);
    buttonsPanelSizer->Add(cancelButton, flags);
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
    wxArrayString as = core_->accountList_.getAccountsName();

    wxSingleChoiceDialog scd(this, _("Choose Bank Account or Term Account"), _("Select Account"), as);
    if (scd.ShowModal() == wxID_OK)
    {
        wxString acctName = scd.GetStringSelection();
        accountID_ = core_->accountList_.GetAccountId(acctName);
        itemAccountName_->SetLabel(acctName);
    }
}

void mmBDDialog::OnPayee(wxCommandEvent& /*event*/)
{
    if (transaction_type_->GetSelection() == DEF_TRANSFER)
    {
        wxArrayString as = core_->accountList_.getAccountsName();

        wxString acctName = itemAccountName_->GetLabel();
        bPayee_->SetLabel(acctName);

        wxSingleChoiceDialog scd(this, _("Account name"), _("Select Account"), as);
        if (scd.ShowModal() == wxID_OK)
        {
            acctName = scd.GetStringSelection();
            payeeID_ = core_->accountList_.GetAccountId(acctName);
            bPayee_->SetLabel(acctName);
            itemAccountName_->SetLabel(acctName);
        }
    }
    else
    {
        mmPayeeDialog dlg(this, core_);

        if ( dlg.ShowModal() == wxID_OK )
        {
            payeeID_ = dlg.getPayeeId();
            if (payeeID_ == -1)
            {
                resetPayeeString();
                payeeUnknown_ = true;
                return;
            }

            // ... If this is a Split Transaction, ignore the Payee change
            if (split_->numEntries())
                return;

            std::shared_ptr<mmPayee> pPayee = core_->payeeList_.GetPayeeSharedPtr(payeeID_);
            bPayee_->SetLabel(pPayee->name_);
            payeeUnknown_ = false;

            if (pPayee->categoryId_ == -1)
            {
                return;
            }

            categID_ = pPayee->categoryId_;
            subcategID_ = pPayee->subcategoryId_;
            wxString categString = core_->categoryList_.GetFullCategoryString(categID_, subcategID_);
            bCategory_->SetLabel(categString);
        }
        else
        {
            wxString payeeName = core_->payeeList_.GetPayeeName(payeeID_);
            if (payeeName.IsEmpty())
            {
                //payeeID_ = -1;
                categID_ = -1;
                subcategID_ = -1;
                categoryName_ = wxEmptyString;
                subCategoryName_ = wxEmptyString;
                bCategory_->SetLabel(_("Select Category"));
                //bPayee_->SetLabel(_("Select Payee"));
                resetPayeeString();
                payeeUnknown_ = true;
            }
            else
            {
                bPayee_->SetLabel(payeeName);
                payeeUnknown_ = false;
            }
        }
    }
}

void mmBDDialog::OnTo(wxCommandEvent& /*event*/)
{
    // This should only get called if we are in a transfer
    wxArrayString as = core_->accountList_.getAccountsName();

    wxSingleChoiceDialog scd(this, _("Account name"), _("Select Account"), as);
    if (scd.ShowModal() == wxID_OK)
    {
        wxString acctName = scd.GetStringSelection();
        toID_ = core_->accountList_.GetAccountId(acctName);
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
      mmCategDialog dlg(core_, this);
      dlg.setTreeSelection(categoryName_, subCategoryName_);
      if ( dlg.ShowModal() == wxID_OK )
      {
         categID_ = dlg.getCategId();
         subcategID_ = dlg.getSubCategId();

         wxString categString = core_->categoryList_.GetFullCategoryString(categID_, subcategID_);
         bCategory_->SetLabel(categString);
      }
   }
}

void mmBDDialog::displayControlsForType( int transType, bool enableAdvanced )
{
    wxStaticText* stFrom = (wxStaticText*)FindWindow(ID_DIALOG_TRANS_STATIC_FROM);
    stFrom->Enable(enableAdvanced);
    bTo_->Enable(enableAdvanced);

    bPayee_->SetToolTip(_("Specify where the transaction is going to or coming from "));
    textAmount_->SetToolTip(_("Specify the amount for this transaction"));

    if (transType == DEF_TRANSFER) {
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

    if (transaction_type_->GetSelection() == DEF_WITHDRAWAL)
    {
        displayControlsForType(DEF_WITHDRAWAL);
        SetTransferControls();
        stp->SetLabel(_("Payee"));
        st->SetLabel("");
        bPayee_->SetToolTip(payeeWithdrawalTip_);
        if (payeeUnknown_)
            resetPayeeString();
    }
    else if (transaction_type_->GetSelection() == DEF_DEPOSIT)
    {
        displayControlsForType(DEF_DEPOSIT);
        SetTransferControls();
        stp->SetLabel(_("From"));
        st->SetLabel("");
        bPayee_->SetToolTip(payeeDepositTip_);
        if (payeeUnknown_)
            resetPayeeString();
    }
    else if (transaction_type_->GetSelection() == DEF_TRANSFER)
    {
        displayControlsForType(DEF_TRANSFER, true);
        if (accountID_ < 0 )
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
        toID_    = -1;
        payeeUnknown_ = true;
    }
}

void mmBDDialog::resetPayeeString(bool normal)
{
    wxString payeeStr = _("Select Payee");
    payeeID_ = -1;
    wxArrayString filtd = core_->payeeList_.FilterPayees("");

    if (filtd.Count() == 1)
    {
        //only one payee present. Choose it
        payeeStr = filtd[0];
        payeeID_ = core_->payeeList_.GetPayeeId(payeeStr);
    }
    bPayee_->SetLabel(payeeStr);
    if (normal)
    {
        toID_ = -1;
    }
}

void mmBDDialog::OnOk(wxCommandEvent& /*event*/)
{
    wxString transaction_type = "";
    wxStringClientData* type_obj = (wxStringClientData *)transaction_type_->GetClientObject(transaction_type_->GetSelection());
    if (type_obj) transaction_type = type_obj->GetData();

    if (payeeID_ == -1)
    {
        if (transaction_type != TRANS_TYPE_TRANSFER_STR)
            mmShowErrorMessageInvalid(this, _("Payee"));
        else
            mmShowErrorMessageInvalid(this, _("From Account"));
        return;
    }

    if (cSplit_->GetValue())
    {
        if (split_->numEntries() == 0)
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

    double amount;
    if (cSplit_->GetValue())
    {
        amount = split_->getTotalSplits();
        if (amount < 0.0)
        {
            if (transaction_type_->GetSelection() == DEF_TRANSFER) {
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
        wxString amountStr = textAmount_->GetValue().Trim();
        if (! CurrencyFormatter::formatCurrencyToDouble(amountStr, amount) || (amount < 0.0))
        {
            mmShowErrorMessageInvalid(this, _("Amount"));
            return;
        }
    }

    if (advancedToTransAmountSet_)
    {
        wxString amountStr = toTextAmount_->GetValue().Trim();
        if (! CurrencyFormatter::formatCurrencyToDouble(amountStr, toTransAmount_) || (toTransAmount_ < 0.0))
        {
            mmShowErrorMessageInvalid(this, _("Advanced Amount"));
            return;
        }
    } else
        toTransAmount_ = amount;

    if ((transaction_type != TRANS_TYPE_TRANSFER_STR) && (accountID_ == -1))
    {
        mmShowErrorMessageInvalid(this, _("Account"));
        return;
    }

    int toAccountID = -1;
    int fromAccountID = accountID_;
    if (transaction_type == TRANS_TYPE_TRANSFER_STR)
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
            if(toAccountID != -1) {
                double rateFrom = core_->accountList_.getAccountBaseCurrencyConvRate(fromAccountID);
                double rateTo = core_->accountList_.getAccountBaseCurrencyConvRate(toAccountID);

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
    if (status_obj) status = status_obj->GetData().Left(1);
    status.Replace("N", "");

    wxString date1 = dpc_->GetValue().FormatISODate();

    if (!edit_ && !enterOccur_)
    {
        static const char sql[] =
        "insert into BILLSDEPOSITS_V1 (ACCOUNTID, TOACCOUNTID, PAYEEID, TRANSCODE, "
          "TRANSAMOUNT, STATUS, TRANSACTIONNUMBER, NOTES,"
          "CATEGID, SUBCATEGID, TRANSDATE, FOLLOWUPID, TOTRANSAMOUNT, REPEATS, "
          "NEXTOCCURRENCEDATE, NUMOCCURRENCES) "
        "values ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, -1, ?, ?, ?, ? )";

        wxSQLite3Statement st = core_->db_.get()->PrepareStatement(sql);

        int i = 0;
        st.Bind(++i, fromAccountID);
        st.Bind(++i, toAccountID);
        st.Bind(++i, payeeID_);
        st.Bind(++i, transaction_type);
        st.Bind(++i, amount);
        st.Bind(++i, status);
        st.Bind(++i, transNum);
        st.Bind(++i, notes);
        st.Bind(++i, categID_);
        st.Bind(++i, subcategID_);
        st.Bind(++i, date1);
        st.Bind(++i, toTransAmount_);
        st.Bind(++i, repeats);
        st.Bind(++i, nextOccurDate);
        st.Bind(++i, numRepeats);

        wxASSERT(st.GetParamCount() == i);

        st.ExecuteUpdate();
        int transID = core_->db_.get()->GetLastRowId().ToLong();
        st.Finalize();

        // --

        static const char sql_ins[] =
        "insert into BUDGETSPLITTRANSACTIONS_V1 (TRANSID, CATEGID, SUBCATEGID, SPLITTRANSAMOUNT) "
        "values (?, ?, ?, ?)";

        st = core_->db_.get()->PrepareStatement(sql_ins);

        for (size_t i = 0; i < split_->numEntries(); ++i)
        {
            mmSplitTransactionEntry &r = *split_->entries_[i];

            st.Bind(1, transID);
            st.Bind(2, r.categID_);
            st.Bind(3, r.subCategID_);
            st.Bind(4, r.splitAmount_);

            st.ExecuteUpdate();
            r.splitEntryID_ = core_->db_.get()->GetLastRowId().ToLong();

            st.Reset();
        }

        st.Finalize();
        transID_ = core_->db_.get()->GetLastRowId().ToLong();
    }
    else if (edit_)
    {
        static const char sql[] =
        "update BILLSDEPOSITS_V1 "
        "SET ACCOUNTID=?, TOACCOUNTID=?, PAYEEID=?, TRANSCODE=?,"
            "TRANSAMOUNT=?, STATUS=?, TRANSACTIONNUMBER=?, NOTES=?,"
            "CATEGID=?, SUBCATEGID=?, TRANSDATE=?, TOTRANSAMOUNT=?, REPEATS=?, "
            "NEXTOCCURRENCEDATE=?, NUMOCCURRENCES=? "
        "WHERE BDID=?";

        wxSQLite3Statement st = core_->db_.get()->PrepareStatement(sql);

        int i = 0;
        st.Bind(++i, accountID_);
        st.Bind(++i, toAccountID);
        st.Bind(++i, payeeID_);
        st.Bind(++i, transaction_type);
        st.Bind(++i, amount);
        st.Bind(++i, status);
        st.Bind(++i, transNum);
        st.Bind(++i, notes);
        st.Bind(++i, categID_);
        st.Bind(++i, subcategID_);
        st.Bind(++i, date1);
        st.Bind(++i, toTransAmount_);
        st.Bind(++i, repeats);
        st.Bind(++i, nextOccurDate);
        st.Bind(++i, numRepeats);
        st.Bind(++i, bdID_);

        wxASSERT(st.GetParamCount() == i);

        st.ExecuteUpdate();
        st.Finalize();
        transID_ = bdID_;
        // --

        st = core_->db_.get()->PrepareStatement("delete from BUDGETSPLITTRANSACTIONS_V1 where TRANSID = ?");
        st.Bind(1, bdID_);
        st.ExecuteUpdate();
        st.Finalize();

        // --

        static const char sql_ins[] =
        "insert into BUDGETSPLITTRANSACTIONS_V1 (TRANSID, CATEGID, SUBCATEGID, SPLITTRANSAMOUNT) "
        "values (?, ?, ?, ?)";

        st = core_->db_.get()->PrepareStatement(sql_ins);

        for (size_t i = 0; i < split_->numEntries(); ++i)
        {
            mmSplitTransactionEntry &r = *split_->entries_[i];

            st.Bind(1, bdID_);
            st.Bind(2, r.categID_);
            st.Bind(3, r.subCategID_);
            st.Bind(4, r.splitAmount_);

            st.ExecuteUpdate();
            r.splitEntryID_ = core_->db_.get()->GetLastRowId().ToLong();

            st.Reset();
        }

        st.Finalize();
    }
    else if (enterOccur_)
    {
        // repeats now hold extra info. Need to get repeats from dialog selection
        if ( (itemRepeats_->GetSelection() < 11) || (itemRepeats_->GetSelection() > 14) || (numRepeats > 0) )
        {
            mmBankTransaction* pTransaction;
            mmBankTransaction* pTemp(new mmBankTransaction(core_));
            pTransaction = pTemp;

            std::shared_ptr<mmCurrency> pCurrencyPtr = core_->accountList_.getCurrencySharedPtr(fromAccountID);
            wxASSERT(pCurrencyPtr);

            pTransaction->accountID_ = fromAccountID;
            pTransaction->toAccountID_ = toAccountID;
            pTransaction->payee_ = core_->payeeList_.GetPayeeSharedPtr(payeeID_);
            pTransaction->transType_ = transaction_type;
            pTransaction->amt_ = amount;
            pTransaction->status_ = status;
            pTransaction->transNum_ = transNum;
            pTransaction->notes_ = notes;
            pTransaction->category_ = core_->categoryList_.GetCategorySharedPtr(categID_, subcategID_);
            pTransaction->date_ = dpc_->GetValue();
            pTransaction->toAmt_ = toTransAmount_;

            *pTransaction->splitEntries_.get() = *split_.get();
            //pTransaction->updateAllData(core_, fromAccountID, pCurrencyPtr);
            core_->bTransactionList_.addTransaction(pTransaction);
        }
        mmDBWrapper::completeBDInSeries(core_->db_.get(), bdID_);
    }
    mmOptions::instance().databaseUpdated_ = true;
    EndModal(wxID_OK);
}

void mmBDDialog::OnSplitChecked(wxCommandEvent& /*event*/)
{
    categID_ = -1;
    subcategID_ = -1;
    split_ = std::shared_ptr<mmSplitTransactionEntries>(new mmSplitTransactionEntries());

    bool state = cSplit_->GetValue();
    if (state)
    {
        bCategory_->SetLabel(_("Split Category"));
        textAmount_->Enable(false);
        wxString dispAmount = CurrencyFormatter::float2String(split_->getTotalSplits());
        textAmount_->SetValue(dispAmount);
        activateSplitTransactionsDlg();
    }
    else
    {
        bCategory_->SetLabel(_("Select Category"));
        textAmount_->Enable(true);
        wxString dispAmount = CurrencyFormatter::float2String(0.0);
        textAmount_->SetValue(dispAmount);
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
        cAdvanced_->Enable();
        bTo_->Show();
    }
    else
    {
        bTo_->Hide();
        cAdvanced_->Disable();
        SetAdvancedTransferControls();
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
            wxString dispAmount = CurrencyFormatter::float2String(toTransAmount_);
            toTextAmount_->SetValue(dispAmount);
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
    SplitTransactionDialog dlg(core_, split_.get(),transaction_type_->GetSelection(), this);
    if (dlg.ShowModal() == wxID_OK)
    {
        double amount = split_->getTotalSplits();
        if (transaction_type_->GetSelection() == DEF_TRANSFER && amount < 0) amount = - amount;
        wxString dispAmount = CurrencyFormatter::float2String(amount);
        textAmount_->SetValue(dispAmount);
     }
}
