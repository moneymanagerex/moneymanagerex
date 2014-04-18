/*******************************************************
Copyright (C) 2013-2014 Nikolay

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

#include "qif_import_gui.h"
#include "qif_import.h"
#include "constants.h"
#include "util.h"
#include "paths.h"
#include "webapp.h"
#include "model/Model_Payee.h"
#include "model/Model_Account.h"
#include "model/Model_Category.h"
#include "model/Model_Currency.h"
#include <wx/progdlg.h>

#include "../resources/reconciled.xpm"
#include "../resources/void.xpm"

IMPLEMENT_DYNAMIC_CLASS( mmQIFImportDialog, wxDialog )

BEGIN_EVENT_TABLE(mmQIFImportDialog, wxDialog)
    EVT_CHECKBOX(wxID_ANY, mmQIFImportDialog::OnCheckboxClick )
    EVT_BUTTON(wxID_OK, mmQIFImportDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, mmQIFImportDialog::OnCancel)
    EVT_CLOSE(mmQIFImportDialog::OnQuit)
END_EVENT_TABLE()

mmQIFImportDialog::mmQIFImportDialog(wxWindow* parent)
      : m_firstReferencedAccountID(-1)
      , m_userDefinedFormat(false)
      , m_parsedOK(false)
      , m_IsFileValid(false)
      , m_IsDatesValid(false)
      , m_IsAccountsOK(false)
      , m_QIFimport(new mmQIFImport())
{
    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, _("QIF Import"), wxDefaultPosition, wxSize(500, 300), style);
}

bool mmQIFImportDialog::Create(wxWindow* parent, wxWindowID id, const wxString& caption
    , const wxPoint& pos, const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

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

    bbFile_ = new wxBitmapButton(this, wxID_ANY, wxNullBitmap, wxDefaultPosition
        , wxSize(file_name_ctrl_->GetSize().GetHeight(), file_name_ctrl_->GetSize().GetHeight()));
    flex_sizer->Add(file_name_label, g_flags);
    flex_sizer->Add(button_search_, g_flags);
    flex_sizer->Add(bbFile_, g_flags);
    main_sizer->Add(file_name_ctrl_, 0, wxALL|wxGROW, 5);
    left_sizer->Add(flex_sizer, g_flagsExpand);

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

    bbFormat_ = new wxBitmapButton(this, wxID_ANY, wxNullBitmap, wxDefaultPosition
        , wxSize(file_name_ctrl_->GetSize().GetHeight(), file_name_ctrl_->GetSize().GetHeight()));
    flex_sizer->Add(dateFormat, g_flags);
    flex_sizer->Add(choiceDateFormat_, g_flags);
    flex_sizer->Add(bbFormat_, g_flags);

    // Accounts
    wxStaticText* newAccountsText = new wxStaticText(this, wxID_STATIC, _("Missing Accounts"));
    newAccounts_ = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(150,-1));
    bbAccounts_ = new wxBitmapButton(this, wxID_ANY, wxNullBitmap, wxDefaultPosition
        , wxSize(file_name_ctrl_->GetSize().GetHeight(), file_name_ctrl_->GetSize().GetHeight()));
    flex_sizer->Add(newAccountsText, g_flags);
    flex_sizer->Add(newAccounts_, g_flags);
    flex_sizer->Add(bbAccounts_, g_flags);

    //Filtering Details --------------------------------------------
    wxStaticBox* static_box = new wxStaticBox(this, wxID_ANY, _("Filtering Details:"));
    wxStaticBoxSizer* filter_sizer = new wxStaticBoxSizer(static_box, wxVERTICAL);
    wxFlexGridSizer* flex_sizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    filter_sizer->Add(flex_sizer2, g_flagsExpand);

    // From Date 
    dateFromCheckBox_ = new wxCheckBox(static_box, wxID_ANY, _("From Date")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    fromDateCtrl_ = new wxDatePickerCtrl(static_box, wxID_STATIC, wxDefaultDateTime
        , wxDefaultPosition, wxSize(150, -1), wxDP_DROPDOWN);
    fromDateCtrl_->Enable(false);
    flex_sizer2->Add(dateFromCheckBox_, g_flags);
    flex_sizer2->Add(fromDateCtrl_, g_flags);

    // To Date
    dateToCheckBox_ = new wxCheckBox(static_box, wxID_ANY, _("To Date")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    toDateCtrl_ = new wxDatePickerCtrl(static_box, wxID_STATIC, wxDefaultDateTime
        , wxDefaultPosition, wxSize(150, -1), wxDP_DROPDOWN);
    toDateCtrl_->Enable(false);
    flex_sizer2->Add(dateToCheckBox_, g_flags);
    flex_sizer2->Add(toDateCtrl_, g_flags);

    //Data viewer ----------------------------------------------
    wxNotebook* qif_notebook = new wxNotebook(this
        , wxID_FILE9, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE);

    wxPanel* log_tab = new wxPanel(qif_notebook, wxID_ANY);
    qif_notebook->AddPage(log_tab, _("Log"));
    wxBoxSizer* log_sizer = new wxBoxSizer(wxVERTICAL);
    log_tab->SetSizer(log_sizer);

    log_field_ = new wxTextCtrl(log_tab, wxID_ANY, ""
        , wxDefaultPosition, wxSize(500, -1), wxTE_MULTILINE|wxHSCROLL );
    log_sizer->Add(log_field_, g_flagsExpand);

    wxPanel* data_tab = new wxPanel(qif_notebook, wxID_ANY);
    qif_notebook->AddPage(data_tab, _("Data"));
    wxBoxSizer* data_sizer = new wxBoxSizer(wxHORIZONTAL);
    data_tab->SetSizer(data_sizer);

    dataListBox_ = new wxDataViewListCtrl(data_tab
        , wxID_ANY, wxDefaultPosition, wxSize(100, 200));
    dataListBox_->AppendTextColumn(ColName_[COL_ACCOUNT], wxDATAVIEW_CELL_INERT, 120, wxALIGN_RIGHT);
    dataListBox_->AppendTextColumn(ColName_[COL_DATE], wxDATAVIEW_CELL_INERT, 90, wxALIGN_RIGHT);
    dataListBox_->AppendTextColumn(ColName_[COL_NUMBER], wxDATAVIEW_CELL_INERT, 80, wxALIGN_RIGHT);
    dataListBox_->AppendTextColumn(ColName_[COL_PAYEE], wxDATAVIEW_CELL_INERT, 120, wxALIGN_RIGHT);
    dataListBox_->AppendTextColumn(ColName_[COL_STATUS], wxDATAVIEW_CELL_INERT, 60, wxALIGN_RIGHT);
    dataListBox_->AppendTextColumn(ColName_[COL_CATEGORY], wxDATAVIEW_CELL_INERT, 140, wxALIGN_RIGHT);
    dataListBox_->AppendTextColumn(ColName_[COL_VALUE], wxDATAVIEW_CELL_INERT, 100, wxALIGN_RIGHT);
    dataListBox_->AppendTextColumn(ColName_[COL_NOTES], wxDATAVIEW_CELL_INERT, 300);
    data_sizer->Add(dataListBox_, g_flagsExpand);

    //Compose all sizers togethe
    wxBoxSizer* top_sizer = new wxBoxSizer(wxHORIZONTAL);
    main_sizer->Add(top_sizer, g_flags);
    top_sizer->Add(left_sizer, g_flags);
    top_sizer->Add(filter_sizer, g_flags);
    main_sizer->Add(qif_notebook, g_flagsExpand);

    /**********************************************************************************************
     Button Panel with OK and Cancel Buttons
    ***********************************************************************************************/
    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    main_sizer->Add(buttons_panel, wxSizerFlags(g_flags).Center());

    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    btnOK_ = new wxButton( buttons_panel, wxID_OK, _("&OK "));
    wxButton* itemButtonCancel_ = new wxButton( buttons_panel, wxID_CANCEL, _("&Cancel "));
    btnOK_->Connect(wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmQIFImportDialog::OnOk), NULL, this);

    buttons_sizer->Add(btnOK_, g_flags);
    buttons_sizer->Add(itemButtonCancel_, g_flags);

    buttons_sizer->Realize();
    Fit();
}

void mmQIFImportDialog::fillControls()
{
    dataListBox_->DeleteAllItems();

    bbFile_->SetBitmap(m_IsFileValid ? wxBitmap(reconciled_xpm) : wxBitmap(void_xpm));
    bbFile_->Enable(m_IsFileValid);
    bbFormat_->SetBitmap(m_IsDatesValid ? wxBitmap(reconciled_xpm) : wxBitmap(void_xpm));
    bbFormat_->Enable(m_IsDatesValid);
    bbAccounts_->SetBitmapLabel(m_IsAccountsOK ? wxBitmap(reconciled_xpm) : wxBitmap(void_xpm));
    bbAccounts_->Enable(m_IsAccountsOK);
    newAccounts_->Enable(newAccounts_->GetCount() > 0);
    btnOK_->Enable(m_IsFileValid && m_IsDatesValid);
}

wxString mmQIFImportDialog::getLineData(const wxString& line) const
{
    return line.substr(1, line.Length()-1);
}

wxString mmQIFImportDialog::getFileLine(wxTextInputStream& textFile, int& lineNumber) const
{
    wxString textLine = textFile.ReadLine();
    lineNumber ++;
    textLine.Trim(); // remove any trailing spaces

    return textLine;
}

wxString mmQIFImportDialog::getFinancistoProject(wxString& sSubCateg) const
{
    //Additional parsint for Financisto
    wxString sProject = "";
    wxStringTokenizer cattkz(sSubCateg, "/");

    sSubCateg = cattkz.GetNextToken();
    if (cattkz.HasMoreTokens())
        sProject = cattkz.GetNextToken().Trim();
    return sProject;
}

bool mmQIFImportDialog::mmParseQIF()
{
    Model_Account::Data* account = Model_Account::instance().get(m_firstReferencedAccountID);
    wxString acctName;
    if (account)
    {
        acctName = account->ACCOUNTNAME;
        fromAccountID_ = account->ACCOUNTID;
    }
    wxString sMsg;

    //Check date restrictions
    wxDateTime fromDate = wxDateTime::Today(), toDate = wxDateTime::Today();
    bool bFromDate = dateFromCheckBox_->IsChecked();
    bool bToDate = dateToCheckBox_->IsChecked();
    if (bFromDate)
        fromDate = fromDateCtrl_->GetValue().GetDateOnly();
    if (bToDate)
        toDate = toDateCtrl_->GetValue().GetDateOnly();

    wxTextCtrl* &logWindow = log_field_;

    int numLines = 0;
    vQIF_trxs_.clear();
    Model_Payee::Data* payee = 0;
    Model_Splittransaction::Cache mmSplit;

    wxFileInputStream input(sFileName_);
    wxTextInputStream text(input, "\x09", wxConvUTF8);

    wxProgressDialog progressDlg(_("Please wait"), _("Scanning")
        , m_numLines, this, wxPD_APP_MODAL | wxPD_CAN_ABORT | wxPD_AUTO_HIDE);

    m_data.trxComplited = true;
    wxLongLong start = wxGetUTCTimeMillis();
    while (input.IsOk() && !input.Eof())
    {
        ++ numLines;
        if (numLines % 100 == 0 || numLines == m_numLines)
        {
            wxLongLong interval = wxGetUTCTimeMillis() - start;
            if (!progressDlg.Update(numLines, wxString::Format(_("Reading line %i of %i, %ld ms"), numLines, m_numLines, interval.ToLong()))) 
                break;
        }

        wxString readLine = text.ReadLine();

        //Init variables for each transaction
        if (m_data.trxComplited)
        {
            m_data.trxComplited = false;
            m_data.valid = true;
            m_data.sDescription.clear();
            m_data.sSplitAmount.clear();
            m_data.sSplitCategs.clear();
            mmSplit.clear();
            payee = 0;
            m_data.payeeString.clear();

            m_data.type.clear();
            m_data.sFullCateg.clear();
            m_data.sCateg.clear();
            m_data.sSubCateg.clear();
            m_data.subCategID = -1;
            m_data.categID = -1;
            m_data.amountString.clear();
            m_data.val = 0.0;
            m_data.dSplitAmount = 0.0;
            m_data.transNum = "";
            m_data.notes = "";
            m_data.convDate = wxDateTime::Today().FormatISODate();
            m_data.dt = m_data.convDate;
        }

        if (readLine.Length() == 0)
            continue;

        bool isOK = m_QIFimport->isLineOK(readLine);
        if (!isOK)
        {
            sMsg = wxString::Format(_("Line: %i"), numLines) << "  " << _(" Unknown QIF line: ") << readLine;
            logWindow->AppendText(wxString() << sMsg << "\n");
            continue;
        }

        qifLineType lineType = m_QIFimport->lineType(readLine);
        if (lineType == AcctType)
        {
            wxString accountType = getLineData(readLine);
            if  ((!accountType.CmpNoCase("Type:Bank"))
                    || (!accountType.CmpNoCase("Type:Cash"))
                    || (!accountType.CmpNoCase("Type:CCard"))
                    || (!accountType.CmpNoCase("Type:Oth L"))
                )
            {
                /*sMsg = wxString::Format(_("Importing account type: %s"), accountType);
                logWindow->AppendText(sMsg << "\n");*/
                m_data.trxComplited = true;
                continue;
            }

            if ( accountType == "Type:Cat" )
            {
                bool reading = true;
                while (input.IsOk() && !input.Eof() && reading)
                {
                    readLine = text.ReadLine();
                    lineType = m_QIFimport->lineType(readLine);
                    numLines++;
                    if (lineType == AcctType)
                    {
                        reading = false;
                        accountType = getLineData(readLine);
                    }
                }
            }

            if (accountType == "Account")
            {
                wxString sBalance = "";
                // account information
                // Need to read till we get to end of account information
                while ((readLine = text.ReadLine()) != "^")
                {
                    numLines++;

                    int i = m_QIFimport->accountInfoType(readLine);
                    if (i == Name)
                    {
                        acctName = getLineData(readLine);
                        continue;
                    }
                    else if (i == Description)
                    {
                        //TODO: Get currency symbol if provided (huck)
                        m_data.sDescription = getLineData(readLine);
                        continue;
                    }
                    else if (i == Balance)
                    {
                        sBalance = getLineData(readLine);
                        if (!sBalance.ToDouble(&m_data.val) && !Model_Currency::fromString(sBalance, m_data.val, Model_Account::currency(account)))
                            m_data.val = 0;
                        continue;
                    }
                    else if (i == AccountType || i == CreditLimit || i  == BalanceDate)
                    {
                        continue;
                    }
                }

                wxString currency_name = m_data.sDescription;
                fromAccountID_ = getOrCreateAccount(acctName, m_data.val, currency_name);
                continue;
            }

            // ignore these type of lines
            if ( accountType == "Option:AutoSwitch" )
            {
                readLine = text.ReadLine();
                lineType = m_QIFimport->lineType(readLine);
                numLines++;
                while(readLine != "^" || input.Eof())
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
            wxString errLineMsgStr = wxString::Format(_("Line: %i"), numLines)
                << "\n" << readLine;

            logWindow->AppendText(wxString()<< errLineMsgStr << "\n" << errMsgStr << "\n");
            wxMessageBox( errLineMsgStr + "\n\n" + errMsgStr, _("QIF Import"), wxICON_ERROR);

            break;
        }

        m_data.to_accountID = -1;
        m_data.from_accountID = fromAccountID_;

        if (lineType == Date) // 'D'
        {
            m_data.dt = getLineData(readLine);

            m_data.valid = mmParseDisplayStringToDate(m_data.dtdt, m_data.dt, dateFormat_);
            m_data.dtdt = m_data.dtdt.GetDateOnly();
            m_data.convDate = m_data.dtdt.FormatISODate();
            continue;
        }
        else if (lineType == Amount) // 'T'
        {
            m_data.amountString = getLineData(readLine);

            if (!m_data.amountString.ToDouble(&m_data.val) 
                && !Model_Currency::fromString(m_data.amountString, m_data.val, Model_Account::currency(account)))
            {
                sMsg = wxString::Format(_("Line: %i invalid amount, skipping."), numLines);
                logWindow->AppendText(sMsg << "\n");
            }
            continue;
        }
        else if (lineType == Payee) // 'P'
        {
            m_data.payeeString = getLineData(readLine);
            continue;
        }
        else if (lineType == TransNumber) // 'N'
        {
            m_data.transNum = getLineData(readLine);
            continue;
        }
        else if (lineType == Memo || lineType == MemoSplit) // 'M' // 'E'
        {
            m_data.notes << getLineData(readLine) << "\n";
            continue;
        }
        else if (lineType == Category || lineType == CategorySplit) // 'S' // 'L'
        {
            m_data.sFullCateg = getLineData(readLine);

            if (m_data.sFullCateg.Left(1).Contains("[") && m_data.sFullCateg.Right(1).Contains("]"))
            {
                m_data.sToAccountName = m_data.sFullCateg.substr(1, m_data.sFullCateg.Length()-2);
                m_data.sFullCateg = _("Transfer");
                m_data.type = Model_Checking::all_type()[Model_Checking::TRANSFER];
            }

            /* //Trick  for cut non standart qif category usage in Financisto application
            //Category field may contains additional information like Project
            //Format Category[:Subcategory][/Project] //*/
            if (m_data.sFullCateg.Contains("/"))
                m_data.transNum.Prepend(wxString::Format("[%s]", getFinancistoProject(m_data.sFullCateg)));

            wxStringTokenizer cattkz(m_data.sFullCateg, ":");
            m_data.sCateg = cattkz.GetNextToken();
            m_data.sSubCateg = "";
            if (cattkz.HasMoreTokens())
                m_data.sSubCateg = cattkz.GetNextToken();

            if (m_data.sCateg.IsEmpty()) m_data.sCateg = _("Unknown");
            Model_Category::Data* category = Model_Category::instance().get(m_data.sCateg);
            if (!category)
            {
                category = Model_Category::instance().create();
                category->CATEGNAME = m_data.sCateg;
                Model_Category::instance().save(category);
                sMsg = wxString::Format(_("Added category: %s"), m_data.sCateg);
                logWindow->AppendText(sMsg << "\n");
            }
            if (category) m_data.categID = category->CATEGID;

            Model_Subcategory::Data* sub_category = (Model_Subcategory::instance().get(m_data.sSubCateg, m_data.categID));
            if (!sub_category && !m_data.sSubCateg.IsEmpty())
            {
                sub_category = Model_Subcategory::instance().create();
                sub_category->CATEGID = m_data.categID;
                sub_category->SUBCATEGNAME = m_data.sSubCateg;
                Model_Subcategory::instance().save(sub_category);
                sMsg = wxString::Format(_("Added subcategory: %s"), m_data.sSubCateg);
                logWindow->AppendText(sMsg << "\n");
            }
            if (sub_category) m_data.subCategID = sub_category->SUBCATEGID;

            continue;
        }
        else if (lineType == AmountSplit) // '$'
        {
            m_data.sSplitAmount = getLineData(readLine);

            //get amount
            if (!m_data.sSplitAmount.ToDouble(&m_data.dSplitAmount)
                && !Model_Currency::fromString(m_data.sSplitAmount, m_data.dSplitAmount, Model_Account::currency(account)))
                m_data.dSplitAmount = 0; //wrong amount
            //
            if (m_data.type == Model_Checking::all_type()[Model_Checking::TRANSFER])
                m_data.dSplitAmount = -m_data.dSplitAmount;
            //Add split entry
            Model_Splittransaction::Data * pSplitEntry = Model_Splittransaction().instance().create();
            pSplitEntry->SPLITTRANSAMOUNT = m_data.dSplitAmount;
            pSplitEntry->CATEGID = m_data.categID;
            pSplitEntry->SUBCATEGID = m_data.subCategID;
            mmSplit.push_back(pSplitEntry);
            continue;
        }
        //MemoSplit
        else if (lineType == Address) // 'A'
        {
            m_data.notes << getLineData(readLine) << "\n";
            continue;
        }
        else if (lineType == EOTLT) // ^
        {
            wxString status = "F";

            if (m_data.dt.Trim().IsEmpty())
            {
                sMsg = _("Date is missing");
                logWindow->AppendText(wxString()<< sMsg << "\n");
                m_data.valid = false;
            }
            if (m_data.amountString.Trim().IsEmpty())
            {
                sMsg = _("Amount is missing");
                logWindow->AppendText(sMsg << "\n");
                m_data.valid = false;
            }

            if (m_data.sFullCateg.Trim().IsEmpty() && m_data.type != Model_Checking::all_type()[Model_Checking::TRANSFER])
            {
                m_data.sCateg = _("Unknown");

                Model_Category::Data* category = Model_Category::instance().get(m_data.sCateg);
                if (!category)
                {
                    category = Model_Category::instance().create();
                    category->CATEGNAME = m_data.sCateg;
                    m_data.categID = Model_Category::instance().save(category);

                    sMsg = wxString::Format(_("Added category: %s"), m_data.sCateg);
                    logWindow->AppendText(sMsg << "\n");
                }
                if (category) m_data.categID = category->CATEGID;
            }

            m_data.to_accountID = -1;
            if (m_data.type == Model_Checking::all_type()[Model_Checking::TRANSFER])
            {
                m_data.to_accountID = getOrCreateAccount(m_data.sToAccountName, 0, m_data.sDescription);

                if (m_data.val > 0.0)
                {
                    m_data.from_accountID = m_data.to_accountID;
                    m_data.to_accountID = fromAccountID_;
                }
                m_data.payeeID = -1;
            }
            else //!=TRANSFER
            {
                if (m_data.val > 0.0)
                    m_data.type = Model_Checking::all_type()[Model_Checking::DEPOSIT];
                else if (m_data.val <= 0.0)
                    m_data.type = Model_Checking::all_type()[Model_Checking::WITHDRAWAL];
                else
                    m_data.valid = false;

                m_data.to_accountID = -1;
                if (m_data.payeeString.IsEmpty())
                {
                    m_data.payeeString = _("Unknown");
                }

                payee = Model_Payee::instance().get(m_data.payeeString);
                if (payee)
                {
                    m_data.payeeID = payee->PAYEEID;
                }
                else
                {
                    payee = Model_Payee::instance().create();
                    payee->PAYEENAME = m_data.payeeString;
                    Model_Payee::instance().save(payee);

                    m_data.payeeID = payee->PAYEEID;
                    logWindow->AppendText(wxString::Format(_("Added payee: %s"), m_data.payeeString) + "\n");
                }
            }

            if (!mmSplit.empty())
            {
                m_data.categID = m_data.subCategID = -1;
                m_data.sFullCateg = _("Split Category");
            }
            else
            {
                m_data.sFullCateg = Model_Category::full_name(m_data.categID, m_data.subCategID);
            }

            m_data.sValid = m_data.valid ? "OK" : "NO";

            if ((m_data.valid) && (bFromDate) && ((m_data.dtdt < fromDate) || (bToDate && (m_data.dtdt > toDate))))
            {
                m_data.sValid = "SKIP";
                m_data.valid = false;
            }

            const Model_Account::Data* from_account = Model_Account::instance().get(m_data.from_accountID);
            const Model_Account::Data* to_account = Model_Account::instance().get(m_data.to_accountID);

            /*for (const auto &split_entry : mmSplit)
            {
                int c = split_entry->CATEGID;
                int s = split_entry->SUBCATEGID;

                Model_Category::Data* category = Model_Category::instance().get(c);
                Model_Subcategory::Data* sub_category = (s != -1 ? Model_Subcategory::instance().get(s) : 0);
                double v = split_entry->SPLITTRANSAMOUNT;
                sMsg = (Model_Category::full_name(category, sub_category) << " " << v << "\n");
                logWindow->AppendText(sMsg);
            }*/
            m_data.trxComplited = true;
            if (!m_data.valid)
            {
                sMsg = wxString::Format(
                    _("Line: %i") + " Valid:%s D:%s Acc:'%s' %s P:'%s%s' Amt:%s C:'%s' \n"
                    , numLines
                    , m_data.sValid
                    , m_data.convDate
                    , wxString(from_account ? from_account->ACCOUNTNAME : "")
                    , wxString(m_data.type == Model_Checking::all_type()[Model_Checking::TRANSFER] ? "<->" : "")
                    , wxString(to_account ? to_account->ACCOUNTNAME : "")
                    , wxString(payee ? payee->PAYEENAME : "")
                    , (wxString() << m_data.val)
                    , m_data.sFullCateg
                    );
                logWindow->AppendText(sMsg);
                continue;
            }

            Model_Checking::Data *transaction = Model_Checking::instance().create();
            transaction->TRANSDATE = m_data.dtdt.FormatISODate();
            transaction->ACCOUNTID = m_data.from_accountID;
            transaction->TOACCOUNTID = m_data.to_accountID;
            transaction->PAYEEID = m_data.payeeID;
            transaction->TRANSCODE = m_data.type;
            transaction->TRANSAMOUNT = m_data.val;
            transaction->STATUS = status;
            transaction->TRANSACTIONNUMBER = m_data.transNum;
            transaction->NOTES = m_data.notes;
            transaction->TOTRANSAMOUNT = m_data.val;
            transaction->CATEGID = m_data.categID;
            transaction->SUBCATEGID = m_data.subCategID;

            //For any transfer transaction always mirrored transaction present
            //Just take alternate amount and skip it
            if (Model_Checking::type(transaction) == Model_Checking::TRANSFER)
            {
                for (auto& refTransaction : vQIF_trxs_)
                {
                    auto &refTrans = refTransaction.first;
                    if (Model_Checking::type(refTrans) != Model_Checking::TRANSFER) continue;
                    if (refTrans->STATUS == "D") continue;
                    if (Model_Checking::TRANSDATE(refTrans) != m_data.dtdt) continue;
                    if (((refTrans->TRANSAMOUNT < 0) && (m_data.val < 0)) || ((refTrans->TRANSAMOUNT > 0) && (m_data.val >0))) continue;
                    if (refTrans->ACCOUNTID != m_data.from_accountID) continue;
                    if (refTrans->TRANSACTIONNUMBER != m_data.transNum) continue;
                    if (refTrans->NOTES != m_data.notes) continue;

                    if (m_data.val > 0.0)
                        refTrans->TOTRANSAMOUNT = m_data.val;
                    else
                        refTrans->TRANSAMOUNT = m_data.val;
                    refTrans->STATUS = "D";

                    m_data.valid = false;
                    break;
                }
            }

            if (m_data.valid)
            {
                vQIF_trxs_.push_back(std::make_pair(transaction, mmSplit));
            }
        }
    }

    progressDlg.Destroy();
    sMsg = wxString::Format(_("Number of transactions read from QIF file: %i"), int(vQIF_trxs_.size()));
    logWindow->AppendText(sMsg << "\n");

    int num = 0;
    dataListBox_->DeleteAllItems();
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
        size_t i = 0;
        for (const auto& split_item : refTransaction.second)
            categs << Model_Category::full_name(split_item->CATEGID, split_item->SUBCATEGID)
            << (refTransaction.second.size() > ++i ? "|" : "");

        data.push_back(wxVariant(categs));
        data.push_back(wxVariant(wxString::Format("%.2f", Model_Checking::balance(transaction, transaction->ACCOUNTID))));
        wxString notes = transaction->NOTES;
        notes.Replace("\n", " ");
        data.push_back(wxVariant(notes));
        dataListBox_->AppendItem(data, (wxUIntPtr)num++);
    }

    sMsg = wxString::Format(_("Press OK Button to continue"));
    logWindow->AppendText(sMsg << "\n");

    return true;
}

void mmQIFImportDialog::OnFileSearch(wxCommandEvent& /*event*/)
{
    m_IsFileValid = false;

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

    wxNotebook* n = (wxNotebook*) FindWindow(wxID_FILE9);
    if (n) n->SetSelection(0);
    wxTextFile tFile;
    tFile.Open(sFileName_ /*, wxCSConv(wxFONTENCODING_CP1251)*/); //TODO: encoding convertion
    if (!tFile.Open())
    {
        wxMessageBox(_("Unable to open file."), _("QIF Import"), wxOK|wxICON_ERROR);
        return;
    }
    *log_field_ << _("Reading data from file") << "\n";
    tFile.Close();

    m_userDefinedFormat = false;
    m_firstReferencedAccountID = -1;

    //Output file into log window
    wxFileInputStream input(sFileName_);
    wxTextInputStream text(input, "\x09", wxConvUTF8);
    int count = 0;
    while (input.IsOk() && !input.Eof() && count <= 100)
    {
        *log_field_ << wxString::Format(_("Line %i "), ++count) << "\t"
            << text.ReadLine() << "\n";
    }
    if (count > 100) *log_field_ << "-------------------------------------- 8< --------------------------------------" << "\n";

    *log_field_ << _("Checking of QIF file started. Please wait...") << "\n";

    m_IsFileValid = checkQIFFile();
    btnOK_->Enable(m_IsFileValid);
    if (m_IsFileValid)
        *log_field_ << _("Checking of QIF file finished successfully") << "\n"
        << _("Press OK Button to continue") << "\n";
    else
        * log_field_ << _("Checking of QIF file failed") << "\n";

    fillControls();
}

bool mmQIFImportDialog::checkQIFFile()
{
    std::map<wxString, int> date_parsing_stat;
    wxArrayString dates;
    wxString sAccountName;
    wxString lastDate = "";

    wxFileInputStream input(sFileName_);
    wxTextInputStream text(input, "\x09", wxConvUTF8);

    size_t numLines = 0;
    while (input.IsOk() && !input.Eof())
    {
        numLines++;
        wxString str = text.ReadLine();
        if (str.empty()) continue;

        if (!m_QIFimport->isLineOK(str))
        {
            wxString sError = wxString()
                << wxString::Format(_("Line %i"), numLines)
                << "\n"
                << str;
            mmShowErrorMessageInvalid(this, sError);
            return false;
        }

        if (numLines < 1000)
        {
            if (m_QIFimport->lineType(str) == Date)
            {
                const wxString sDate = getLineData(str);
                if (sDate == lastDate) continue; //Speedup
                lastDate = sDate;
                if (dates.Index(sDate) == wxNOT_FOUND) dates.Add(sDate);
                continue;
            }
        }

        if (m_QIFimport->lineType(str) == AcctType && getLineData(str) == "Account")
        {
            bool reading = true;
            while (input.IsOk() && !input.Eof() && reading)
            {
                numLines++;
                str = text.ReadLine();
                if (m_QIFimport->accountInfoType(str) == Name)
                {
                    sAccountName = getLineData(str);
                    wxLogDebug("%s", sAccountName);
                    if (Model_Account::instance().all_checking_account_names().Index(sAccountName) == wxNOT_FOUND)
                    {
                        newAccounts_->Append(sAccountName);
                    }
                }
                reading = (m_QIFimport->accountInfoType(str) != EOT);
            }
            continue;
        }
    }

    std::map<wxString, wxString> date_formats_temp = date_formats_map();
    for (const auto& sDate : dates)
    {
        if (date_formats_temp.size() == 1)
            break;
        std::map<wxString, wxString> date_formats = date_formats_temp;
        for (const auto& date_mask : date_formats)
        {
            wxString mask = m_userDefinedFormat ? dateFormat_ : date_mask.first;
            wxDateTime dtdt;
            if (mmParseDisplayStringToDate(dtdt, sDate, mask))
                date_parsing_stat[mask] ++;
            else
                date_formats_temp.erase(mask);

            if (m_userDefinedFormat) break;
        }
    }

    //Check parsing results
    int i = 0;
    for (const auto& d : date_parsing_stat)
    {
        if (d.second > i)
        {
            i = d.second;
            if (!m_userDefinedFormat)
            {
                choiceDateFormat_->SetStringSelection(date_formats_map()[d.first]);
                dateFormat_ = d.first;
            }
        }
        wxLogDebug("%i \t%s \t%i", i++, date_formats_map()[d.first], d.second);
    }

    if (!date_parsing_stat.empty())
    {
        m_IsDatesValid = true;
        btnOK_->Enable(true);
    }
    else
    {
        m_IsDatesValid = false;
        return false;
    }

    if (sAccountName.IsEmpty() && m_firstReferencedAccountID < 0)
    {
        wxArrayString accounts = Model_Account::instance().all_checking_account_names();
        if (accounts.empty())
        {
            mmShowErrorMessage(this, _("There are no accounts available for import to"), _("QIF Import"));
            return false;
        }
        else
        {
            sAccountName = wxGetSingleChoice(_("Choose Account to Import to")
                , _("Account")
                , Model_Account::instance().all_checking_account_names()
                , this);
        }

        const Model_Account::Data* account = Model_Account::instance().get(sAccountName);
        if (account)
            m_firstReferencedAccountID = account->ACCOUNTID;
        else
            return false;
    }

    if (newAccounts_->GetCount() > 0)
    {
        Model_Currency::Data* base_currency = Model_Currency::GetBaseCurrency();
        if (!base_currency)
        {
            mmShowErrorMessageInvalid(this, _("Base Currency Not Set"));
            return false;
        }
        newAccounts_->SetSelection(0);
        m_IsAccountsOK = false;
    }
    else
    {
        m_IsAccountsOK = true;
    }

    m_numLines = numLines;
    *log_field_ << wxString::Format(_("Total number of lines: %i"), int(numLines)) << "\n";
    return true;
}

void mmQIFImportDialog::OnDateMaskChange(wxCommandEvent& /*event*/)
{
    wxStringClientData* data = (wxStringClientData*)(choiceDateFormat_->GetClientObject(choiceDateFormat_->GetSelection()));
    if (data) dateFormat_ = data->GetData();
    if (sFileName_.IsEmpty())
        return;
    m_userDefinedFormat = true;

    m_parsedOK = checkQIFFile();
}

void mmQIFImportDialog::OnCheckboxClick( wxCommandEvent& /*event*/ )
{
    fromDateCtrl_->Enable(dateFromCheckBox_->GetValue());
    toDateCtrl_->Enable(dateToCheckBox_->GetValue());
}

void mmQIFImportDialog::OnOk(wxCommandEvent& /*event*/)
{
    wxString sMsg;

    if (!m_parsedOK)
    {
        wxMessageDialog msgDlg(this, _("Do you want to scan all transaction ?")
            + "\n" + _("All missing account, payees and categories will be created.")
            , _("Please Confirm:")
            , wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
        if (msgDlg.ShowModal() == wxID_YES)
        {
            wxNotebook* n = (wxNotebook*) FindWindow(wxID_FILE9);
            if (n) n->SetSelection(1);
            m_parsedOK = mmParseQIF();
            btnOK_->Enable(m_parsedOK);
        }
        return;
    }

    wxMessageDialog msgDlg(this, _("Do you want to import all transaction ?")
        , _("Confirm Import")
        , wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
    if (msgDlg.ShowModal() == wxID_YES)
    {
        int numTransactions = vQIF_trxs_.size();
        int count = 0;
        wxProgressDialog progressDlg(_("Please wait"), _("Importing")
            , numTransactions, this, wxPD_APP_MODAL | wxPD_CAN_ABORT | wxPD_AUTO_HIDE);

        for (auto& refTransaction : vQIF_trxs_)
        {
            count ++; 
            if (count % 100 == 0 || count == numTransactions)
            {
                if (!progressDlg.Update(count, wxString::Format(_("Importing transaction %i of %i"), count, numTransactions))) // if cancel clicked
                    break; // abort processing
            }

            auto *refTrans(refTransaction.first);
            refTrans->TRANSAMOUNT = fabs(refTrans->TRANSAMOUNT);
            refTrans->TOTRANSAMOUNT = fabs(refTrans->TOTRANSAMOUNT);
            refTrans->STATUS = "F";

            int transID = Model_Checking::instance().save(refTrans); //TODO: check for duplicate
            for (auto& split : refTransaction.second)
            {
                split->TRANSID = transID;
                if (Model_Checking::type(refTrans) != Model_Checking::DEPOSIT)
                    split->SPLITTRANSAMOUNT = -split->SPLITTRANSAMOUNT;
                Model_Splittransaction::instance().save(split);
            }

            if (m_firstReferencedAccountID < 0) m_firstReferencedAccountID = refTrans->ACCOUNTID;
        }

		mmWebApp::MMEX_WebApp_UpdateAccount();
		mmWebApp::MMEX_WebApp_UpdatePayee();
		mmWebApp::MMEX_WebApp_UpdateCategory();
        sMsg = _("Import finished successfully");
        btnOK_->Enable(false);
        progressDlg.Destroy();
    }
    else
    {
        sMsg = _("Imported transactions discarded by user!"); //TODO: strange message may be _("Import has discarded by user!")
    }
    wxMessageDialog(this, sMsg, _("QIF Import"), wxOK | wxICON_WARNING).ShowModal();
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

int mmQIFImportDialog::getOrCreateAccount(const wxString& name, double init_balance, const wxString& currency_name)
{
    int accountID = -1;
    Model_Account::Data* account = Model_Account::instance().get(name);
    if (!account)
    {

        Model_Account::Data *account = Model_Account::instance().create();

        account->FAVORITEACCT = "TRUE";
        account->STATUS = Model_Account::all_status()[Model_Account::OPEN];
        account->ACCOUNTTYPE = Model_Account::all_type()[Model_Account::CHECKING];
        account->ACCOUNTNAME = name;
        account->INITIALBAL = init_balance;
        account->CURRENCYID = Model_Currency::GetBaseCurrency()->CURRENCYID;
        for (const auto& curr : Model_Currency::instance().all())
        {
            if (wxString(curr.CURRENCY_SYMBOL).Prepend("[").Append("]") == currency_name)
                account->CURRENCYID = curr.CURRENCYID;
        }
        accountID = Model_Account::instance().save(account);
        wxString sMsg = wxString::Format(_("Added account: %s"), name);
        log_field_->AppendText(wxString() << sMsg << "\n");
    }
    else
        accountID = account->ACCOUNTID;

    return accountID;
}
