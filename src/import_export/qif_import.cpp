/*******************************************************
Copyright (C) 2006-2012

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

#include "qif_import.h"
#include "constants.h"
#include "util.h"
#include "guiid.h"
#include "paths.h"
#include "model/Model_Payee.h"
#include "model/Model_Account.h"
#include "model/Model_Category.h"
#include "model/Model_Currency.h"

IMPLEMENT_DYNAMIC_CLASS( mmQIFImportDialog, wxDialog )

BEGIN_EVENT_TABLE( mmQIFImportDialog, wxDialog )
    EVT_CHECKBOX(wxID_ANY, mmQIFImportDialog::OnCheckboxClick )
    EVT_BUTTON(wxID_OK, mmQIFImportDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, mmQIFImportDialog::OnCancel)
    EVT_CLOSE(mmQIFImportDialog::OnQuit)
END_EVENT_TABLE()

mmQIFImportDialog::mmQIFImportDialog(
      wxWindow* parent, wxWindowID id
    , const wxString& caption, const wxPoint& pos
    , const wxSize& size, long style
) :
      parent_(parent)
    , last_imported_acc_id_(-1)
{
    Create(parent, id, caption, pos, size, style);
}

bool mmQIFImportDialog::Create( wxWindow* parent, wxWindowID id, const wxString& caption,
                           const wxPoint& pos, const wxSize& size, long style )
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    ColName_[COL_ACCOUNT]  = _("Account");
    ColName_[COL_DATE]     = _("Date");
    ColName_[COL_NUMBER]   = _("Number");
    ColName_[COL_PAYEE]    = _("Payee");
    ColName_[COL_STATUS]   = _("Status");
    ColName_[COL_CATEGORY] = _("Category");
    ColName_[COL_VALUE]    = _("Value");
    ColName_[COL_NOTES]    = _("Notes");

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    SetIcon(mmex::getProgramIcon());
    Centre();
    Fit();

    fillControls();

    return TRUE;
}

void mmQIFImportDialog::CreateControls()
{
    wxSizerFlags flags, flagsExpand;
    flags.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 5);
    flagsExpand.Align(wxALIGN_CENTER).Border(wxALL, 5).Expand().Proportion(1);

    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(main_sizer);
    wxBoxSizer* left_sizer = new wxBoxSizer(wxVERTICAL);

    wxFlexGridSizer* flex_sizer = new wxFlexGridSizer(0, 3, 0, 0);
    //flex_sizer->AddGrowableCol(1);

    // File Name --------------------------------------------
    wxStaticText* file_name_label = new wxStaticText(this, wxID_STATIC, _("File Name:"));
    button_search_ = new wxButton(this, wxID_OPEN, _("Choose &file"));
    button_search_->Connect(wxID_OPEN, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmQIFImportDialog::OnFileSearch), NULL, this);

    file_name_ctrl_ = new wxTextCtrl(this, wxID_FILE, wxEmptyString,
        wxDefaultPosition, wxDefaultSize, wxTE_READONLY);

    bbFile_ = new wxBitmapButton(this, wxID_ANY, wxBitmap(flag_xpm));
    flex_sizer->Add(file_name_label, flags);
    flex_sizer->Add(button_search_, flags);
    flex_sizer->Add(bbFile_, flags);
    main_sizer->Add(file_name_ctrl_, 0, wxALL|wxGROW, 5);
    left_sizer->Add(flex_sizer, flagsExpand);

    // Date Format Settings
    dateFormat_ = mmOptions::instance().dateFormat_;

    wxStaticText* dateFormat = new wxStaticText(this, wxID_STATIC, _("Date Format"));
    choiceDateFormat_ = new wxComboBox(this, wxID_ANY);
    for (const auto& i : date_formats_map())
    {
        choiceDateFormat_->Append(i.second, new wxStringClientData(i.first));
        if (dateFormat_ == i.first) choiceDateFormat_->SetStringSelection(i.second);
    }
    choiceDateFormat_->Connect(wxID_ANY, wxEVT_COMMAND_COMBOBOX_SELECTED,
        wxCommandEventHandler(mmQIFImportDialog::OnDateMaskChange), NULL, this);

    bbFormat_ = new wxBitmapButton(this, wxID_ANY, wxBitmap(flag_xpm));
    flex_sizer->Add(dateFormat, flags);
    flex_sizer->Add(choiceDateFormat_, flags);
    flex_sizer->Add(bbFormat_, flags);

    // Accounts
    wxStaticText* newAccountsText = new wxStaticText(this, wxID_STATIC, _("Missing Accounts"));
    newAccounts_ = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(150,-1));
    bbAccounts_ = new wxBitmapButton(this, wxID_ANY, wxBitmap(flag_xpm));
    flex_sizer->Add(newAccountsText, flags);
    flex_sizer->Add(newAccounts_, flags);
    flex_sizer->Add(bbAccounts_, flags);

    // From Date --------------------------------------------
    dateFromCheckBox_ = new wxCheckBox( this, wxID_ANY, _("From Date")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    fromDateCtrl_ = new wxDatePickerCtrl( this, wxID_STATIC, wxDefaultDateTime
        , wxDefaultPosition, wxSize(150, -1), wxDP_DROPDOWN);
    fromDateCtrl_->Enable(false);
    flex_sizer->Add(dateFromCheckBox_, flags);
    flex_sizer->Add(fromDateCtrl_, flags);
    flex_sizer->AddSpacer(1);

    // To Date --------------------------------------------
    dateToCheckBox_ = new wxCheckBox( this, wxID_ANY, _("To Date")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    toDateCtrl_ = new wxDatePickerCtrl( this, wxID_STATIC, wxDefaultDateTime
        , wxDefaultPosition, wxSize(150, -1), wxDP_DROPDOWN);
    toDateCtrl_->Enable(false);
    flex_sizer->Add(dateToCheckBox_, flags);
    flex_sizer->Add(toDateCtrl_, flags);
    flex_sizer->AddSpacer(1);

    //Log viewer
    wxBoxSizer* log_sizer = new wxBoxSizer(wxVERTICAL);

    log_field_ = new wxTextCtrl( this, wxID_STATIC, "", wxDefaultPosition, wxSize(300, -1), wxTE_MULTILINE|wxHSCROLL );
    log_sizer->Add(log_field_, 1, wxGROW|wxALL, 5);

    wxButton* itemClearButton = new wxButton(this, wxID_CLEAR, _("Clear"));
    log_sizer->Add(itemClearButton, 0, wxALIGN_CENTER|wxALL, 5);
    itemClearButton->Connect(wxID_CLEAR, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmQIFImportDialog::OnButtonClear), NULL, this);

    //Data viewer
    wxPanel* data_panel = new wxPanel(this, wxID_ANY);
    wxBoxSizer* data_sizer = new wxBoxSizer(wxHORIZONTAL);
    data_panel->SetSizer(data_sizer);

    dataListBox_ = new wxDataViewListCtrl(data_panel
        , wxID_ANY, wxDefaultPosition, wxSize(100, 200));
    dataListBox_->AppendTextColumn( ColName_[COL_ACCOUNT], wxDATAVIEW_CELL_INERT, 120);
    dataListBox_->AppendTextColumn( ColName_[COL_DATE], wxDATAVIEW_CELL_INERT, 100);
    dataListBox_->AppendTextColumn( ColName_[COL_NUMBER], wxDATAVIEW_CELL_INERT, 80);
    dataListBox_->AppendTextColumn( ColName_[COL_PAYEE], wxDATAVIEW_CELL_INERT, 120);
    dataListBox_->AppendTextColumn( ColName_[COL_STATUS], wxDATAVIEW_CELL_INERT, 60);
    dataListBox_->AppendTextColumn( ColName_[COL_CATEGORY], wxDATAVIEW_CELL_INERT, 120);
    dataListBox_->AppendTextColumn( ColName_[COL_VALUE], wxDATAVIEW_CELL_INERT, 100);
    dataListBox_->AppendTextColumn( ColName_[COL_NOTES], wxDATAVIEW_CELL_INERT, 300);
    data_sizer->Add(dataListBox_, flagsExpand);

    //Compose all sizers togethe
    wxBoxSizer* top_sizer = new wxBoxSizer(wxHORIZONTAL);
    main_sizer->Add(top_sizer, flagsExpand);
    top_sizer->Add(left_sizer, flags);
    top_sizer->Add(log_sizer, flagsExpand);
    main_sizer->Add(data_panel, flagsExpand);

    /**********************************************************************************************
     Button Panel with OK and Cancel Buttons
    ***********************************************************************************************/
    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    main_sizer->Add(buttons_panel, flags.Center());

    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    btnOK_ = new wxButton( buttons_panel, wxID_OK, _("&OK "));
    wxButton* itemButtonCancel_ = new wxButton( buttons_panel, wxID_CANCEL, _("&Cancel "));
    btnOK_->Connect(wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmQIFImportDialog::OnOk), NULL, this);

    buttons_sizer->Add(btnOK_, flags);
    buttons_sizer->Add(itemButtonCancel_, flags);

    buttons_sizer->Realize();
    Fit();
}

void mmQIFImportDialog::fillControls()
{
    bbFile_ ->SetBitmapLabel(wxBitmap(empty_xpm));
    bbFile_ ->Enable(false);
    bbFormat_->SetBitmapLabel(wxBitmap(empty_xpm));
    bbFormat_->Enable(false);
    bbAccounts_->SetBitmapLabel(wxBitmap(empty_xpm));
    bbAccounts_->Enable(false);
    newAccounts_->Enable(false);
    btnOK_->Enable(false);
}

bool mmQIFImportDialog::isLineOK(const wxString& line)
{
    return wxString("!DNPAT^MLSE$C/UI").Contains(line.Left(1));
}

wxString mmQIFImportDialog::getLineData(const wxString& line)
{
    wxString dataString = line.substr(1, line.Length()-1);
    return dataString;
}

wxString mmQIFImportDialog::getFileLine(wxTextInputStream& textFile, int& lineNumber)
{
    wxString textLine = textFile.ReadLine();
    lineNumber ++;
    textLine.Trim(); // remove any trailing spaces

    return textLine;
}

wxString mmQIFImportDialog::getFinancistoProject(wxString& sSubCateg)
{
    //Additional parsint for Financisto
    wxString sProject = "";
    wxStringTokenizer cattkz(sSubCateg, "/");

    sSubCateg = cattkz.GetNextToken();
    if (cattkz.HasMoreTokens())
        sProject = cattkz.GetNextToken();
    return sProject;
}

bool mmQIFImportDialog::warning_message()
{
    wxString msgStr;
    msgStr << _("To import QIF files correctly, the date format in the QIF file must match the date option set in MMEX.") << "\n\n"
           << _("Are you are sure you want to proceed with the import?");
    wxMessageDialog msgDlg(NULL, msgStr, _("QIF Import"), wxYES_NO|wxICON_QUESTION);
    if (msgDlg.ShowModal() != wxID_YES)
        return false;
    return true;
}

int mmQIFImportDialog::mmImportQIF(wxTextFile& tFile)
{
    Model_Account::Data* account = Model_Account::instance().get(last_imported_acc_id_);
    wxString acctName;
    if (account)
    {
        acctName = account->ACCOUNTNAME;
        fromAccountID_ = account->ACCOUNTID;
    }
    wxString sMsg;

    //Check date restrictions
    wxDateTime fromDate = wxDateTime::Now(), toDate = wxDateTime::Now();
    bool bFromDate = dateFromCheckBox_->IsChecked();
    bool bToDate = dateToCheckBox_->IsChecked();
    if (bFromDate)
        fromDate = fromDateCtrl_->GetValue().GetDateOnly();
    if (bToDate)
        toDate = toDateCtrl_->GetValue().GetDateOnly();

    wxSortedArrayString accountArray;
    for (const auto& account: Model_Account::instance().all()) accountArray.Add(account.ACCOUNTNAME);

    wxTextCtrl*& logWindow = log_field_;

    int numLines = 0;
    int trxNumLine = 1;

    wxString dt = wxDateTime::Now().FormatISODate();
    wxString sPayee, type, sAmount, transNum, notes, convDate, sToAccountName;
    wxString sFullCateg, sCateg, sSubCateg, sSplitCategs, sSplitAmount, sValid;

    wxDateTime dtdt = wxDateTime::Today();
    int payeeID = -1, categID = -1, subCategID = -1, to_account_id = -1, from_account_id = -1;
    double val = 0.0, dSplitAmount = 0.0;
    bool bTrxComplited = true;

    Model_Payee::Data* payee = 0;

    //mmSplitTransactionEntries* mmSplit(new mmSplitTransactionEntries());
    Model_Splittransaction::Cache mmSplit;

    wxFileInputStream input(sFileName_);
    wxTextInputStream text(input, "\x09", wxConvUTF8);

    while (input.IsOk() && !input.Eof())
    //for (readLine = tFile.GetFirstLine(); !tFile.Eof(); readLine = tFile.GetNextLine())
    {
        wxString readLine = text.ReadLine();
        //Init variables for each transaction
        if (bTrxComplited)
        {
            sSplitAmount.clear();
            sSplitCategs.clear();
            mmSplit.clear();

            sPayee.clear();
            type.clear();
            sFullCateg.clear();
            sCateg.clear();
            sSubCateg.clear();
            subCategID = -1;
            categID = -1;
            sAmount.clear();
            val = 0.0;
            dSplitAmount = 0.0;
            transNum = "";
            notes = "";
            convDate = wxDateTime::Now().FormatISODate();

            bTrxComplited = false;
            trxNumLine = numLines - 1;
            sMsg = "-------------------------------------------------------------------------------------------------------------------------\n";
            logWindow->AppendText(sMsg);
        }

        if (readLine.Length() == 0)
            continue;

        bool isOK = isLineOK(readLine);
        if (!isOK)
        {
            sMsg = wxString()<< _("Line: ") << numLines << "  " << _(" Unknown QIF line: ") << readLine;
            logWindow->AppendText(wxString() << sMsg << "\n");
            continue;
        }

        if (lineType(readLine) == AcctType)
        {
            wxString accountType = getLineData(readLine);
            if  ((!accountType.CmpNoCase("Type:Bank"))
                    || (!accountType.CmpNoCase("Type:Cash"))
                    || (!accountType.CmpNoCase("Type:CCard"))
                    || (!accountType.CmpNoCase("Type:Oth L"))
                )
            {
                sMsg = wxString::Format(_("Importing account type: %s"), accountType);
                logWindow->AppendText(sMsg << "\n");
                bTrxComplited = true;
                continue;
            }

            if ( accountType == "Type:Cat" )
            {
                bool reading = true;
                while (input.IsOk() && !input.Eof() && reading)
                {
                    readLine = text.ReadLine();
                    if (lineType(readLine) == AcctType  || tFile.Eof())
                    {
                        reading = false;
                        accountType = getLineData(readLine);
                    }
                }
            }

            if ( accountType == "Account")
            {
                wxString sDescription = "";
                wxString sBalance = "";
                // account information
                // Need to read till we get to end of account information
                while( (readLine = text.ReadLine() ) != "^")
                {
                    numLines++;

                    int i = accountInfoType(readLine);
                    if (i == Name)
                    {
                        acctName = getLineData(readLine);
                        continue;
                    }
                    else if (i == Description)
                    {
                        //TODO: Get currency symbol if provided (huck)
                        sDescription = getLineData(readLine);
                        continue;
                    }
                    else if (i == Balance)
                    {
                        sBalance = getLineData(readLine);
                        if (!sBalance.ToDouble(&val) && !Model_Currency::fromString(sBalance, val, Model_Account::currency(account)))
                            val = 0;
                        continue;
                    }
                    else if (i == AccountType || i == CreditLimit || i  == BalanceDate)
                    {
                        continue;
                    }
                }

                Model_Account::Data* account = Model_Account::instance().get(acctName);
                if (!account)
                {
                    //TODO: Repeated code
                    account = Model_Account::instance().create();

                    account->FAVORITEACCT = "TRUE";
                    account->STATUS = Model_Account::all_status()[Model_Account::OPEN];
                    account->ACCOUNTTYPE = Model_Account::all_type()[Model_Account::CHECKING];
                    account->ACCOUNTNAME = acctName;
                    account->INITIALBAL = val;
                    account->CURRENCYID = Model_Currency::GetBaseCurrency()->CURRENCYID;
                    for (const auto& curr : Model_Currency::instance().all())
                    {
                        if (wxString(curr.CURRENCY_SYMBOL).Prepend("[").Append("]") == sDescription)
                            account->CURRENCYID = curr.CURRENCYID;
                    }

                    Model_Account::instance().save(account);
                    from_account_id = account->ACCOUNTID;
                    accountArray.Add(account->ACCOUNTNAME);
                    sMsg = wxString::Format(_("Added account '%s'"), acctName)
                        << "\n" << wxString::Format(_("Initial Balance: %s"), (wxString()<<val));
                    logWindow->AppendText(wxString()<< sMsg << "\n");
                }

                fromAccountID_ = account->ACCOUNTID;

                sMsg = wxString::Format(_("Line: %ld"), numLines) << " : "
                    << wxString::Format(_("Account name: %s"), acctName);
                logWindow->AppendText(wxString()<< sMsg << "\n");

                continue;
            }

            // ignore these type of lines
            if ( accountType == "Option:AutoSwitch" )
            {
                readLine = tFile.GetNextLine();
                while(readLine != "^" || tFile.Eof())
                {
                    // ignore all lines
                }
                continue;
            }
            else if ( accountType == "Type:Security" || accountType == "Clear:AutoSwitch")
            {
                continue;
            }
            // we do not know how to process this type yet
            wxString errMsgStr = _("Cannot process these QIF Account Types yet.");
            wxString errLineMsgStr = wxString::Format(_("Line: %ld"), numLines)
                << "\n" << readLine;

            logWindow->AppendText(wxString()<< errLineMsgStr << "\n" << errMsgStr << "\n");
            wxMessageBox( errLineMsgStr + "\n\n" + errMsgStr, _("QIF Import"), wxICON_ERROR);

            // exit: while(!input.Eof()) loop and allow to exit routine and allow user to save or abort
            break;
        }

        to_account_id = -1;
        from_account_id = fromAccountID_;
        bool bValid = true;

        if (lineType(readLine) == Date) // 'D'
        {
            dt = getLineData(readLine);

            mmParseDisplayStringToDate(dtdt, dt, dateFormat_);
            dtdt = dtdt.GetDateOnly();
            convDate = dtdt.FormatISODate();
            continue;
        }
        else if (lineType(readLine) == Amount) // 'T'
        {
            sAmount = getLineData(readLine);

            if (!sAmount.ToDouble(&val) && !Model_Currency::fromString(sAmount, val, Model_Account::currency(account)))
            {
                sMsg = wxString::Format(_("Line: %ld invalid amount, skipping."), numLines);
                logWindow->AppendText(sMsg << "\n");
            }
            continue;
        }
        else if (lineType(readLine) == Payee) // 'P'
        {
            sPayee = getLineData(readLine);
            continue;
        }
        else if (lineType(readLine) == TransNumber) // 'N'
        {
            transNum = getLineData(readLine);
            continue;
        }
        else if (lineType(readLine) == Memo || lineType(readLine) == MemoSplit ) // 'M' // 'E'
        {
            notes << getLineData(readLine) << "\n";
            continue;
        }
        else if (lineType(readLine) == Category || lineType(readLine) == CategorySplit) // 'S' // 'L'
        {
            sFullCateg = getLineData(readLine);

            if (sFullCateg.Left(1).Contains("[") && sFullCateg.Right(1).Contains("]"))
            {
                sToAccountName = sFullCateg.substr(1, sFullCateg.Length()-2);
                sFullCateg = _("Transfer");
                type = TRANS_TYPE_TRANSFER_STR;
            }

            /* //Trick  for cut non standart qif category usage in Financisto application
            //Category field may contains additional information like Project
            //Format Category[:Subcategory][/Project] //*/
            if (sFullCateg.Contains("/"))
                transNum.Prepend(wxString::Format("[%s] ", getFinancistoProject(sFullCateg)));

            //core_->categoryList_.parseCategoryString(sFullCateg, sCateg, categID, sSubCateg, subCategID);
            wxStringTokenizer cattkz(sFullCateg, ":");
            sCateg = cattkz.GetNextToken();
            sSubCateg = "";
            if (cattkz.HasMoreTokens())
                sSubCateg = cattkz.GetNextToken();

            Model_Category::Data* category = Model_Category::instance().get(sCateg);
            if (!category && !sCateg.IsEmpty())
            {
                category = Model_Category::instance().create();
                category->CATEGNAME = sCateg;
                Model_Category::instance().save(category);
                sMsg = wxString::Format(_("Added category: %s"), sCateg);
                logWindow->AppendText(sMsg << "\n");
            }
            if (category) categID = category->CATEGID;

            Model_Subcategory::Data* sub_category = (Model_Subcategory::instance().get(sSubCateg, categID));
            if (!sub_category && !sSubCateg.IsEmpty())
            {
                sub_category = Model_Subcategory::instance().create();
                sub_category->SUBCATEGNAME = sSubCateg;
                Model_Subcategory::instance().save(sub_category);
                sMsg = wxString::Format(_("Added subcategory: %s"), sSubCateg);
                logWindow->AppendText(sMsg << "\n");
            }
            if (sub_category) subCategID = sub_category->SUBCATEGID;

            continue;
        }
        else if (lineType(readLine) == AmountSplit) // '$'
        {
            sSplitAmount = getLineData(readLine);

            //get amount
            if (!sSplitAmount.ToDouble(&dSplitAmount) && !Model_Currency::fromString(sSplitAmount, dSplitAmount, Model_Account::currency(account)))
                dSplitAmount = 0; //wrong amount
            //
            if (type == TRANS_TYPE_WITHDRAWAL_STR)
                dSplitAmount = -dSplitAmount;
            //Add split entry
            //mmSplitTransactionEntry* pSplitEntry(new mmSplitTransactionEntry);
            Model_Splittransaction::Data * pSplitEntry = Model_Splittransaction().instance().create();
            //pSplitEntry->splitAmount_  = dSplitAmount;
            pSplitEntry->SPLITTRANSAMOUNT = dSplitAmount;
            //pSplitEntry->categID_      = categID;
            pSplitEntry->CATEGID = categID;
            //pSplitEntry->subCategID_   = subCategID;
            pSplitEntry->SUBCATEGID = subCategID;
            //mmSplit->addSplit(pSplitEntry);
            mmSplit.push_back(pSplitEntry);
            continue;
        }
        //MemoSplit
        else if (lineType(readLine) == Address) // 'A'
        {
            notes << getLineData(readLine) << "\n";
            continue;
        }
        else if (lineType(readLine) == EOTLT) // ^
        {
            wxString status = "F";

            if (dt.Trim().IsEmpty())
            {
                sMsg = _("Date is missing");
                logWindow->AppendText(wxString()<< sMsg << "\n");
                bValid = false;
            }
            if (sAmount.Trim().IsEmpty())
            {
                sMsg = _("Amount is missing");
                logWindow->AppendText(sMsg << "\n");
                bValid = false;
            }

            if (sFullCateg.Trim().IsEmpty() && type != Model_Checking::all_type()[Model_Checking::TRANSFER])
            {
                sMsg = _("Category is missing");
                logWindow->AppendText(sMsg << "\n");
                sFullCateg = _("Unknown");

                Model_Category::Data* category = Model_Category::instance().get(sCateg);
                if (!category) 
                {
                    category = Model_Category::instance().create();
                    category->CATEGNAME = sCateg;
                    Model_Category::instance().save(category);
                    categID = category->CATEGID;

                    sMsg = wxString::Format(_("Added category: %s"), sCateg);
                    logWindow->AppendText(sMsg << "\n");
                }
            }

            to_account_id = -1;
            if (type == Model_Checking::all_type()[Model_Checking::TRANSFER])
            {
                Model_Account::Data* account = Model_Account::instance().get(sToAccountName);
                if (!account)
                {
                    account = Model_Account::instance().create();

                    account->FAVORITEACCT = "TRUE";
                    account->STATUS = Model_Account::all_status()[Model_Account::OPEN];
                    account->ACCOUNTTYPE = Model_Account::all_type()[Model_Account::CHECKING];
                    account->ACCOUNTNAME = sToAccountName;
                    account->INITIALBAL = 0;

                    Model_Account::instance().save(account);
                    from_account_id = account->ACCOUNTID;
                    accountArray.Add(account->ACCOUNTNAME);

                    to_account_id = account->ACCOUNTID;
                    accountArray.Add(sToAccountName);

                    sMsg = wxString::Format(_("Added account '%s'"), sToAccountName);
                    logWindow->AppendText(wxString()<< sMsg << "\n");
                }
                to_account_id = account->ACCOUNTID;
                if (val > 0.0)
                {
                    from_account_id = to_account_id;
                    to_account_id = fromAccountID_;
                }
                payeeID = -1;
                if (to_account_id == -1 || from_account_id == -1)
                {
                    sMsg = _("Account missing");
                    logWindow->AppendText(sMsg << "\n");
                    bValid = false;
                }
            }
            else //!=TRANSFER
            {
                if (val > 0.0)
                    type = Model_Checking::all_type()[Model_Checking::DEPOSIT];
                else if (val < 0.0)
                    type = Model_Checking::all_type()[Model_Checking::WITHDRAWAL];
                else
                    bValid = false;

                to_account_id = -1;
                if (sPayee.IsEmpty())
                {
                    sMsg = _("Payee missing");
                    logWindow->AppendText(sMsg << "\n");
                    sPayee = _("Unknown");
                }

                payee = Model_Payee::instance().get(sPayee);
                if (payee)
                {
                    payeeID = payee->PAYEEID;
                }
                else
                {
                    payee = Model_Payee::instance().create();
                    payee->PAYEENAME = sPayee;
                    Model_Payee::instance().save(payee);

                    payeeID = payee->PAYEEID;
                    logWindow->AppendText(wxString::Format(_("Payee Added: %s"), sPayee));
                }
            }

            if (!mmSplit.empty())
            {
                categID = -1;
                sFullCateg = _("Split Category");
            }
            else
            {
                Model_Category::Data* category = Model_Category::instance().get(categID);
                Model_Subcategory::Data* sub_category = (Model_Subcategory::instance().get(subCategID));
                sFullCateg = Model_Category::full_name(category, sub_category);
            }

            if (bValid)
                sValid = "OK";
            else
                sValid = "NO";

            if ((bValid) && (bFromDate) && ((dtdt < fromDate) || (bToDate && (dtdt > toDate))))
            {
                sValid = "SKIP";
                bValid = false;
            }
            const Model_Account::Data* from_account = Model_Account::instance().get(from_account_id);
            const Model_Account::Data* to_account = Model_Account::instance().get(to_account_id);

            sMsg = wxString::Format(
                "Line:%ld Trx:%ld %s D:%s Acc:'%s' %s P:'%s%s' Amt:%s C:'%s' \n"
                , trxNumLine
                , vQIF_trxs_.size() + 1
                , sValid
                , convDate
                , from_account ? from_account->ACCOUNTNAME : ""
                , wxString((type == Model_Checking::all_type()[Model_Checking::TRANSFER] ? "<->" : ""))
                , to_account ? to_account->ACCOUNTNAME : ""
                , payee ? payee->PAYEENAME : ""
                , (wxString()<<val)
                , sFullCateg
                );
            logWindow->AppendText(sMsg);

            for (const auto &split_entry : mmSplit)
            {
                int c = split_entry->CATEGID; //mmSplit->entries_[i]->categID_;
                int s = split_entry->SUBCATEGID; //mmSplit->entries_[i]->subCategID_;

                Model_Category::Data* category = Model_Category::instance().get(c);
                Model_Subcategory::Data* sub_category = (s != -1 ? Model_Subcategory::instance().get(s) : 0);

                wxString cn = category->CATEGNAME;
                wxString sn = (sub_category ? sub_category->SUBCATEGNAME : ""); 
                double v = split_entry->SPLITTRANSAMOUNT; // mmSplit->entries_[i]->splitAmount_;
                sMsg = (cn << ":" << sn << " " << v << "\n");
                logWindow->AppendText(sMsg);
            }
            bTrxComplited = true;
            if (!bValid) continue;

            Model_Checking::Data *transaction = Model_Checking::instance().create();
            transaction->TRANSDATE = dtdt.FormatISODate();
            transaction->ACCOUNTID = from_account_id;
            transaction->TOACCOUNTID = to_account_id;
            transaction->PAYEEID = payeeID;
            transaction->TRANSCODE = type;
            transaction->TRANSAMOUNT = val;
            transaction->STATUS = status;
            transaction->TRANSACTIONNUMBER = transNum;
            transaction->NOTES = notes;
            transaction->TOTRANSAMOUNT = val;
            if (!mmSplit.empty()) categID = -1;
            transaction->CATEGID = categID;
            transaction->SUBCATEGID = subCategID;

            //For any transfer transaction always mirrored transaction present
            //Just take alternate amount and skip it
            if (Model_Checking::type(transaction) == Model_Checking::TRANSFER)
            {
                for (auto& refTransaction : vQIF_trxs_)
                {
                    auto *refTrans(refTransaction.first);
                    if (Model_Checking::type(refTrans) != Model_Checking::TRANSFER) continue;
                    if (refTrans->STATUS == "D") continue; //TODO:
                    if (Model_Checking::TRANSDATE(refTrans) != dtdt) continue;
                    if (((refTrans->TRANSAMOUNT < 0) && (val < 0)) || ((refTrans->TRANSAMOUNT > 0) && (val >0))) continue;
                    if (refTrans->ACCOUNTID!= from_account_id) continue;
                    if (refTrans->TRANSACTIONNUMBER != transNum) continue;
                    if (refTrans->NOTES != notes) continue;

                    if (val > 0.0)
                        refTrans->TOTRANSAMOUNT = val;
                    else
                        refTrans->TRANSAMOUNT = val;
                    refTrans->STATUS = "D";

                    sMsg = wxString::Format("%f -> %f (%f)\n", refTrans->TRANSAMOUNT
                        , refTrans->TOTRANSAMOUNT
                        , (fabs(refTrans->TRANSAMOUNT) / fabs(refTrans->TOTRANSAMOUNT)<1)
                        ? fabs(refTrans->TOTRANSAMOUNT) / fabs(refTrans->TRANSAMOUNT)
                        : fabs(refTrans->TRANSAMOUNT) / fabs(refTrans->TOTRANSAMOUNT));
                    logWindow->AppendText(sMsg);

                    bValid = false;
                    break;
                }
            }

            if (bValid)
            {   
                vQIF_trxs_.push_back(std::make_pair(transaction, mmSplit));
            }
        }
    }

    sMsg = wxString::Format(_("Transactions imported from QIF: %ld"), vQIF_trxs_.size());
    logWindow->AppendText(sMsg << "\n");

    int num = 0;
    for (const auto& refTransaction : vQIF_trxs_)
    {
        Model_Checking::Data* transaction = refTransaction.first;
        const Model_Account::Data* account = Model_Account::instance().get(transaction->ACCOUNTID);
        wxVector<wxVariant> data;
        data.push_back(wxVariant(account->ACCOUNTNAME));
        data.push_back(wxVariant(transaction->TRANSDATE));
        data.push_back(wxVariant(transaction->TRANSACTIONNUMBER));
        wxString payee_name = "";
        if (Model_Checking::type(transaction) == Model_Checking::TRANSFER)
        {
            account = Model_Account::instance().get(transaction->TOACCOUNTID);
            if (account) payee_name = account->ACCOUNTNAME;
        }
        else
        {
            payee = Model_Payee::instance().get(transaction->PAYEEID);
            if (payee) payee_name = payee->PAYEENAME;
        }
        if (Model_Checking::type(transaction) != Model_Checking::DEPOSIT)
            transaction->TRANSAMOUNT = -transaction->TRANSAMOUNT;
        data.push_back(wxVariant(payee_name));
        data.push_back(wxVariant(transaction->STATUS));

        wxString categs = Model_Category::full_name(transaction->CATEGID, transaction->SUBCATEGID);
        for (const auto& split_item : refTransaction.second) 
            categs << Model_Category::full_name(split_item->CATEGID, split_item->SUBCATEGID) << "|";

        data.push_back(wxVariant(categs));
        data.push_back(wxVariant(wxString::Format("%.2f", Model_Checking::balance(transaction, transaction->ACCOUNTID))));
        data.push_back(wxVariant(transaction->NOTES));
        dataListBox_->AppendItem(data, (wxUIntPtr)num++);
    }

    return fromAccountID_;
}

void mmQIFImportDialog::OnFileSearch(wxCommandEvent& /*event*/)
{
    sFileName_ = file_name_ctrl_->GetValue();

    const wxString choose_ext = _("QIF Files");
    sFileName_ = wxFileSelector(_("Choose QIF data file to Import"),
        wxEmptyString, sFileName_, wxEmptyString,
        choose_ext + " (*.qif)|*.qif;*.QIF"
            , wxFD_OPEN|wxFD_CHANGE_DIR|wxFD_FILE_MUST_EXIST);

    if (!sFileName_.IsEmpty())
        correctEmptyFileExt( "qif", sFileName_);
	else
		return; // user pressed cancel
    file_name_ctrl_->SetValue(sFileName_);

    wxTextFile tFile;
    tFile.Open(sFileName_ /*, wxCSConv(wxFONTENCODING_CP1251)*/); //TODO: encoding convertion
    if (!tFile.Open())
    {
        wxMessageBox(_("Unable to open file."), _("QIF Import"), wxOK|wxICON_ERROR);
        return;
    }

    //Output file into log window
    wxFileInputStream input(sFileName_);
    wxTextInputStream text(input, "\x09", wxConvUTF8);
    int count = 0;
    while (input.IsOk() && !input.Eof())
    {
        *log_field_ << wxString::Format(_("Line %i "), ++count) << "\t"
            << text.ReadLine() << "\n";
    }

    if (checkQIFFile(tFile))
        mmImportQIF(tFile);
}

bool mmQIFImportDialog::checkQIFFile(wxTextFile& tFile)
{
    bbFile_->Enable(false);
    bbFile_->SetBitmapLabel(wxBitmap(empty_xpm));
    bbFormat_->Enable(false);
    bbFormat_->SetBitmapLabel(wxBitmap(empty_xpm));
    newAccounts_->Clear();
    bbAccounts_->SetBitmapLabel(wxBitmap(empty_xpm));
    btnOK_->Enable(false);

    bool dateFormatIsOK = false;
    wxString sAccountName; //Last parsed account name
    for (wxString str = tFile.GetFirstLine(); !tFile.Eof(); str = tFile.GetNextLine())
    {
        if (str.empty())
            continue;

        if (!isLineOK(str))
        {
            wxString sError = wxString()
                << wxString::Format(_("Line %i"), tFile.GetCurrentLine()+1)
                << "\n"
                << str;
            mmShowErrorMessageInvalid(this, sError);
            return false;
        }

        if ( lineType(str) == AcctType && getLineData(str) == "Account")
        {
            bool reading = true;
            while(!tFile.Eof() && reading )
            {
                str = tFile.GetNextLine();
                if (accountInfoType(str) == Name)
                {
                    sAccountName = getLineData(str);
                    if (Model_Account::instance().all_checking_account_names().Index(sAccountName) == wxNOT_FOUND)
                    {
                        newAccounts_->Append(sAccountName);
                    }
                }
                reading = (accountInfoType(str) != EOT);
            }
            continue;
        }

        if (lineType(str) == Date)
        {
            wxDateTime dtdt;
            wxString sDate = getLineData(str);

            dateFormatIsOK = dateFormatIsOK || mmParseDisplayStringToDate(dtdt, sDate, dateFormat_);
            continue;
        }
    }

    bbFile_->Enable(true);
    bbFile_->SetBitmapLabel(wxBitmap(flag_xpm));
    bbFormat_->Enable(dateFormatIsOK);
    if (dateFormatIsOK)
        bbFormat_->SetBitmapLabel(wxBitmap(flag_xpm));
    else
        return false;

    if (sAccountName.IsEmpty() && last_imported_acc_id_<0)
    {
        sAccountName = wxGetSingleChoice(_("Choose Account to Import to")
            , _("Account"), Model_Account::instance().all_checking_account_names());

        const Model_Account::Data* account = Model_Account::instance().get(sAccountName);
        last_imported_acc_id_ = account->ACCOUNTID;
        if (last_imported_acc_id_ < 0) return false;
    }

    if (newAccounts_->GetCount() > 0)
    {
        Model_Currency::Data* base_currency = Model_Currency::GetBaseCurrency();
        if (!base_currency)
        {
            mmShowErrorMessageInvalid(this, _("Base Currency Not Set"));
            return false;
        }
        newAccounts_->Enable(true);
        newAccounts_->SetSelection(0);
        bbAccounts_->SetBitmapLabel(wxBitmap(flag_xpm));
    }

    if (dateFormatIsOK)
    {
        bbFormat_->SetBitmapLabel(wxBitmap(flag_xpm));
        bbFormat_->Enable(dateFormatIsOK);
        btnOK_->Enable(true);
    }
    return true;
}

void mmQIFImportDialog::OnDateMaskChange(wxCommandEvent& /*event*/)
{
    wxStringClientData* data = (wxStringClientData*)(choiceDateFormat_->GetClientObject(choiceDateFormat_->GetSelection()));
    if (data) dateFormat_ = data->GetData();
    wxTextFile tFile(sFileName_);
    if (sFileName_.IsEmpty()) return;
    if (!tFile.IsOpened())
        if (tFile.Open())
            checkQIFFile(tFile);
}

void mmQIFImportDialog::OnCheckboxClick( wxCommandEvent& /*event*/ )
{
    fromDateCtrl_->Enable(dateFromCheckBox_->GetValue());
    toDateCtrl_->Enable(dateToCheckBox_->GetValue());
}

void mmQIFImportDialog::OnOk(wxCommandEvent& /*event*/)
{
    wxString sMsg;
    wxMessageDialog msgDlg(this, _("Do you want to import all transaction ?")
        , _("Confirm Import")
        , wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
    if (msgDlg.ShowModal() == wxID_YES)
    {

        for (auto& refTransaction : vQIF_trxs_)
        {
            auto *refTrans(refTransaction.first);
            refTrans->TRANSAMOUNT = fabs(refTrans->TRANSAMOUNT);
            refTrans->TOTRANSAMOUNT = fabs(refTrans->TOTRANSAMOUNT);

            int transID = Model_Checking::instance().save(refTrans); //TODO:fix speed
            for (auto& split : refTransaction.second)
            {
                split->TRANSID = transID;
                if (Model_Checking::type(refTrans) != Model_Checking::DEPOSIT)
                    split->SPLITTRANSAMOUNT = -split->SPLITTRANSAMOUNT;
                Model_Splittransaction::instance().save(split); //TODO:fix speed
            }

            last_imported_acc_id_ = refTrans->ACCOUNTID;
        }

        sMsg = _("Import finished successfully");
        btnOK_->Enable(false);
    }
    else
    {
        sMsg = _("Imported transactions discarded by user!");
    }
    wxMessageDialog(parent_, sMsg, _("QIF Import"), wxOK|wxICON_WARNING).ShowModal();
    //clear the vector to avoid memory leak - done at same level created.

}

void mmQIFImportDialog::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void mmQIFImportDialog::OnQuit(wxCloseEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void mmQIFImportDialog::OnButtonClear(wxCommandEvent& /*event*/)
{
    log_field_->Clear();
}
