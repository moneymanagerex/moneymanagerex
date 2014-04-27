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
    , m_userDefinedDateMask(false)
    , m_QIFimport(new mmQIFImport())
{
    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, _("QIF Import"), wxDefaultPosition, wxSize(500, 300), style);
}

wxString mmQIFImportDialog::OnGetItemText(long item, long column) const
{
    return vQIF_trxs_.at(item).at(column);
}

bool mmQIFImportDialog::Create(wxWindow* parent, wxWindowID id, const wxString& caption
    , const wxPoint& pos, const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    ColName_[COL_ID] = _("#");
    ColName_[COL_ACCOUNT] = _("Account");
    ColName_[COL_DATE]     = _("Date");
    ColName_[COL_NUMBER]   = _("Number");
    ColName_[COL_PAYEE]    = _("Payee");
    ColName_[COL_TYPE]     = _("Type");
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

    wxFlexGridSizer* flex_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    //flex_sizer->AddGrowableCol(1);

    // File Name --------------------------------------------
    wxStaticText* file_name_label = new wxStaticText(this, wxID_STATIC, _("File Name:"));
    button_search_ = new wxButton(this, wxID_OPEN, _("Choose &file"));
    button_search_->Connect(wxID_OPEN, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmQIFImportDialog::OnFileSearch), NULL, this);

    file_name_ctrl_ = new wxTextCtrl(this, wxID_FILE, wxEmptyString
        , wxDefaultPosition, wxDefaultSize, wxTE_READONLY);

    flex_sizer->Add(file_name_label, g_flags);
    flex_sizer->Add(button_search_, g_flags);
    main_sizer->Add(file_name_ctrl_, 0, wxALL|wxGROW, 5);
    left_sizer->Add(flex_sizer, g_flagsExpand);

    // Date Format Settings
    m_dateFormatStr = mmOptions::instance().dateFormat_;

    wxStaticText* dateFormat = new wxStaticText(this, wxID_STATIC, _("Date Format"));
    choiceDateFormat_ = new wxComboBox(this, wxID_ANY);
    for (const auto& i : date_formats_map())
    {
        choiceDateFormat_->Append(i.second, new wxStringClientData(i.first));
        if (m_dateFormatStr == i.first) choiceDateFormat_->SetStringSelection(i.second);
    }
    choiceDateFormat_->Connect(wxID_ANY, wxEVT_COMMAND_COMBOBOX_SELECTED
        , wxCommandEventHandler(mmQIFImportDialog::OnDateMaskChange), NULL, this);

    flex_sizer->Add(dateFormat, g_flags);
    flex_sizer->Add(choiceDateFormat_, g_flags);

    //Account
    accountCheckBox_ = new wxCheckBox(this, wxID_FILE5, _("Account")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    accountDropDown_ = new wxChoice(this
        , wxID_ANY, wxDefaultPosition, wxSize(180, -1));
    for (const auto& a : Model_Account::instance().all_checking_account_names())
        accountDropDown_->Append(a, new wxStringClientData(a));

    accountDropDown_->Enable(false);
    accountDropDown_->SetSelection(0);

    flex_sizer->Add(accountCheckBox_, g_flags);
    flex_sizer->Add(accountDropDown_, g_flags);

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
    wxBoxSizer* data_sizer = new wxBoxSizer(wxVERTICAL);
    data_tab->SetSizer(data_sizer);

    dataListBox_ = new wxDataViewListCtrl(data_tab
        , wxID_ANY, wxDefaultPosition, wxSize(100, 200));
    dataListBox_->AppendTextColumn(ColName_[COL_ID], wxDATAVIEW_CELL_INERT, 40, wxALIGN_RIGHT);
    dataListBox_->AppendTextColumn(ColName_[COL_ACCOUNT], wxDATAVIEW_CELL_INERT, 120, wxALIGN_LEFT);
    dataListBox_->AppendTextColumn(ColName_[COL_DATE], wxDATAVIEW_CELL_INERT, 90, wxALIGN_RIGHT);
    dataListBox_->AppendTextColumn(ColName_[COL_NUMBER], wxDATAVIEW_CELL_INERT, 80, wxALIGN_RIGHT);
    dataListBox_->AppendTextColumn(ColName_[COL_PAYEE], wxDATAVIEW_CELL_INERT, 120, wxALIGN_LEFT);
    dataListBox_->AppendTextColumn(ColName_[COL_TYPE], wxDATAVIEW_CELL_INERT, 60, wxALIGN_LEFT);
    dataListBox_->AppendTextColumn(ColName_[COL_CATEGORY], wxDATAVIEW_CELL_INERT, 140, wxALIGN_LEFT);
    dataListBox_->AppendTextColumn(ColName_[COL_VALUE], wxDATAVIEW_CELL_INERT, 100, wxALIGN_RIGHT);
    dataListBox_->AppendTextColumn(ColName_[COL_NOTES], wxDATAVIEW_CELL_INERT, 300, wxALIGN_LEFT);
    data_sizer->Add(dataListBox_, g_flagsExpand);

    //Accounts
    wxPanel* acc_tab = new wxPanel(qif_notebook, wxID_ANY);
    qif_notebook->AddPage(acc_tab, _("Account"));
    wxBoxSizer* acc_sizer = new wxBoxSizer(wxHORIZONTAL);
    acc_tab->SetSizer(acc_sizer);

    accListBox_ = new wxDataViewListCtrl(acc_tab
        , wxID_ANY, wxDefaultPosition, wxSize(100, 200));
    accListBox_->AppendTextColumn(_("Name"), wxDATAVIEW_CELL_INERT, 250, wxALIGN_LEFT);
    accListBox_->AppendTextColumn(_("Currency"), wxDATAVIEW_CELL_INERT, 50, wxALIGN_LEFT);
    accListBox_->AppendTextColumn(_("Status"), wxDATAVIEW_CELL_INERT, 150, wxALIGN_LEFT);
    acc_sizer->Add(accListBox_, g_flagsExpand);

    //Payees
    wxPanel* payee_tab = new wxPanel(qif_notebook, wxID_ANY);
    qif_notebook->AddPage(payee_tab, _("Payee"));
    wxBoxSizer* payee_sizer = new wxBoxSizer(wxHORIZONTAL);
    payee_tab->SetSizer(payee_sizer);

    payeeListBox_ = new wxDataViewListCtrl(payee_tab
        , wxID_ANY, wxDefaultPosition, wxSize(100, 200));
    payeeListBox_->AppendTextColumn(_("Name"), wxDATAVIEW_CELL_INERT, 250, wxALIGN_LEFT);
    payeeListBox_->AppendTextColumn(_("Status"), wxDATAVIEW_CELL_INERT, 150, wxALIGN_LEFT);
    payee_sizer->Add(payeeListBox_, g_flagsExpand);

    //Category
    wxPanel* categ_tab = new wxPanel(qif_notebook, wxID_ANY);
    qif_notebook->AddPage(categ_tab, _("Category"));
    wxBoxSizer* category_sizer = new wxBoxSizer(wxHORIZONTAL);
    categ_tab->SetSizer(category_sizer);
    categoryListBox_ = new wxDataViewListCtrl(categ_tab
        , wxID_ANY, wxDefaultPosition, wxSize(100, 200));
    categoryListBox_->AppendTextColumn(_("Name"), wxDATAVIEW_CELL_INERT, 250, wxALIGN_LEFT);
    categoryListBox_->AppendTextColumn(_("Status"), wxDATAVIEW_CELL_INERT, 150, wxALIGN_LEFT);
    category_sizer->Add(categoryListBox_, g_flagsExpand);

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
    refreshTabs();
    btnOK_->Enable(!file_name_ctrl_->GetValue().empty());
}

bool mmQIFImportDialog::mmReadQIFFile()
{
    int numLines = 0;
    std::map<wxString, wxString> date_formats_temp = date_formats_map();
    vQIF_trxs_.clear();
    m_QIFaccounts.clear();
    m_QIFpayeeNames.clear();
    m_QIFcategoryNames.clear();
    m_date_parsing_stat.clear();

    wxFileInputStream input(m_FileNameStr);
    wxTextInputStream text(input, "\x09", wxConvUTF8);

    wxProgressDialog progressDlg(_("Please wait"), _("Scanning")
        , 0, this, wxPD_APP_MODAL | wxPD_CAN_ABORT);

    wxLongLong start = wxGetUTCTimeMillis();
    wxLongLong interval = wxGetUTCTimeMillis() - start;

    wxString accName = ""; //TODO: check Account check box
    std::map <int, wxString> trx;
    while (input.IsOk() && !input.Eof())
    {
        ++numLines;

        const wxString lineStr = text.ReadLine();
        if (lineStr.Length() == 0)
            continue;
        const qifLineType lineType = m_QIFimport->lineType(lineStr);

        if (numLines % 100 == 0)
        {
            interval = wxGetUTCTimeMillis() - start;
            if (!progressDlg.Pulse(wxString::Format(_("Reading line %i, %ld ms")
                , numLines, interval.ToLong())))
                break;
        }

        const auto data = m_QIFimport->getLineData(lineStr);
        if (lineType == EOTLT)
        {
            if (trx.find(AcctType) != trx.end() && trx[AcctType] == "Account")
            {
                accName = (trx.find(TransNumber) == trx.end() ? "" : trx[TransNumber]);
                if (m_QIFaccounts.find(accName) == m_QIFaccounts.end())
                    m_QIFaccounts[accName] = trx;
            }
            else
            {
                compliteTransaction(trx, accName);
                vQIF_trxs_.push_back(trx);
            }
            trx.clear();
            continue;
        }

        if (trx[lineType].empty())
            trx[lineType] = data;
        else
            trx[lineType] += "\n" + data;
        
        if (numLines <= 50)
        {
            *log_field_ << wxString::Format(_("Line %i \t %s\n"), numLines, lineStr);
            if (numLines == 50)
                *log_field_ << "-------------------------------------- 8< --------------------------------------\n";
        }

        //Parse date format
        if (date_formats_temp.size() > 1 && lineType == Date
            && (data.Mid(0, 1) != "["))
        {
            parseDate(data, date_formats_temp);
        }
    }
    log_field_->ScrollLines(log_field_->GetNumberOfLines());
    fillControls();

    progressDlg.Destroy();
    interval = wxGetUTCTimeMillis() - start;
    wxString sMsg = wxString::Format(_("Number of lines read from QIF file: %i in %ld ms")
        , int(numLines), interval.ToLong());
    *log_field_ << sMsg << "\n";
    sMsg = wxString::Format(_("Press OK Button to continue"));
    *log_field_ << sMsg << "\n";

    return true;
}

void mmQIFImportDialog::compliteTransaction(std::map <int, wxString> &trx, const wxString &accName)
{
    bool isTransfer = false;
    auto accountName = accName;
    if (accName.empty()){
        accountName = m_accountNameStr;
    }
    trx[AccountName] = accountName;
    if (trx.find(Category) != trx.end())
    {
        if (trx[Category].Mid(0,1) == "[" && trx[Category].Last() == ']')
        {
            isTransfer = true;
            wxString toAccName = trx[Category].SubString(1, trx[Category].length() - 2);
            trx[Category] = "Transfer";
            trx[TrxType] = Model_Checking::all_type()[Model_Checking::TRANSFER];
            trx[ToAccountName] = toAccName;
        }
        if (m_QIFcategoryNames.find(trx[Category]) == m_QIFcategoryNames.end())
            m_QIFcategoryNames[trx[Category]] = std::make_pair(-1, -1);
    }
    else if (trx.find(CategorySplit) != trx.end())
    {
        wxStringTokenizer token(trx[CategorySplit], "\n");
        while (token.HasMoreTokens())
        {
            const wxString c = token.GetNextToken();
            if (m_QIFcategoryNames.find(c) == m_QIFcategoryNames.end())
                m_QIFcategoryNames[c] = std::make_pair(-1, -1);
        }
    }

    if (trx.find(Payee) != trx.end() && !isTransfer) {
        if (m_QIFpayeeNames.find(trx[Payee]) == m_QIFpayeeNames.end()) {
            m_QIFpayeeNames[trx[Payee]] = -1;
        }
    }

    if (!isTransfer) {
        const wxString amtStr = (trx.find(Amount) == trx.end() ? "" : trx[Amount]);
        if (amtStr.Mid(0,1) == "-")
            trx[TrxType] = Model_Checking::all_type()[Model_Checking::WITHDRAWAL];
        else if (!amtStr.empty())
            trx[TrxType] = Model_Checking::all_type()[Model_Checking::DEPOSIT];
    }
}

void mmQIFImportDialog::refreshTabs(int tabs)
{
    int num = 0;
    if (tabs%2)
    {
        num = 0;
        wxString acc;
        dataListBox_->DeleteAllItems();
        for (const auto& trx : vQIF_trxs_)
        {
            const std::map <int, wxString> &map = trx;
            wxVector<wxVariant> data;
            data.push_back(wxVariant(wxString::Format("%i", num + 1)));
            data.push_back(wxVariant(map.find(AccountName) != map.end()
                && !map.at(AccountName).empty() ? map.at(AccountName) : m_accountNameStr));

            data.push_back(wxVariant(map.find(Date) == map.end() ? "" : map.at(Date)));
            data.push_back(wxVariant(map.find(TransNumber) == map.end() ? "" : map.at(TransNumber)));
            const wxString type = (map.find(TrxType) == map.end() ? "" : map.at(TrxType));
            if (type == Model_Checking::all_type()[Model_Checking::TRANSFER])
                data.push_back(wxVariant(map.find(ToAccountName) == map.end() ? "" : map.at(ToAccountName)));
            else
                data.push_back(wxVariant(map.find(Payee) == map.end() ? "" : map.at(Payee)));
            data.push_back(wxVariant(map.find(TrxType) == map.end() ? "" : map.at(TrxType)));
            if (map.find(CategorySplit) == map.end())
                data.push_back(wxVariant(map.find(Category) == map.end() ? "" : map.at(Category)));
            else
                data.push_back(wxVariant(map.at(CategorySplit)));

            data.push_back(wxVariant(map.find(Amount) == map.end() ? "" : map.at(Amount)));
            data.push_back(wxVariant(map.find(Memo) == map.end() ? "" : map.at(Memo)));

            dataListBox_->AppendItem(data, (wxUIntPtr) num++);
        }
    }

    if (int(tabs / 2) % 2)
    {
        num = 0;
        accListBox_->DeleteAllItems();
        for (const auto& acc : m_QIFaccounts)
        {
            wxVector<wxVariant> data;
            const std::map <int, wxString> &a = acc.second;
            data.push_back(wxVariant(acc.first));
            wxString currencySymbol = a.find(Date) == a.end() ? "" : a.at(Date);
            currencySymbol = currencySymbol.SubString(1, currencySymbol.length() - 2);
            data.push_back(wxVariant(currencySymbol));
            Model_Account::Data* account = Model_Account::instance().get(acc.first);
            wxString status;
            if (account) {
                Model_Currency::Data *curr = Model_Currency::instance().get(account->CURRENCYID);
                if (curr && curr->CURRENCY_SYMBOL == currencySymbol)
                    status = _("OK");
                else
                    status = _("Warning");
            }
            else
                status = _("Missing");
            data.push_back(wxVariant(status));
            accListBox_->AppendItem(data, (wxUIntPtr) num++);
        }
    }

    if (int(tabs / 4) % 2)
    {
        payeeListBox_->DeleteAllItems();
        for (const auto& payee : m_QIFpayeeNames)
        {
            wxVector<wxVariant> data;
            data.push_back(wxVariant(payee.first));
            Model_Payee::Data_Set p = Model_Payee::instance().find(Model_Payee::PAYEENAME(payee.first));
            data.push_back(wxVariant(p.empty() ? _("Missing") : _("OK")));
            payeeListBox_->AppendItem(data, (wxUIntPtr) num++);
        }
    }

    if (int(tabs / 8) % 2)
    {
        num = 0;
        const auto &c(Model_Category::all_categories());
        categoryListBox_->DeleteAllItems();
        for (const auto& categ : m_QIFcategoryNames)
        {
            wxVector<wxVariant> data;
            data.push_back(wxVariant(categ.first));
            if (c.find(categ.first) == c.end())
                data.push_back(wxVariant("Missing"));
            else
                data.push_back(wxVariant(_("OK")));
            categoryListBox_->AppendItem(data, (wxUIntPtr) num++);
        }
    }
}

void mmQIFImportDialog::parseDate(const wxString &dateStr, std::map<wxString, wxString> &date_formats_temp)
{
    if (date_formats_temp.size() == 1) return;
    const std::map<wxString, wxString> date_formats = date_formats_temp;
    for (const auto& date_mask : date_formats)
    {
        const wxString mask = m_userDefinedDateMask ? m_dateFormatStr : date_mask.first;
        wxDateTime dtdt;
        if (mmParseDisplayStringToDate(dtdt, dateStr, mask) && !m_userDefinedDateMask)
            m_date_parsing_stat[mask] ++;
        else {
            date_formats_temp.erase(mask);
            if (date_formats_temp.size() == 0) {
                date_formats_temp["%Y-%m-%d"] = "YYYY-MM-DD";
                m_date_parsing_stat["%Y-%m-%d"] = 999;
            }
            //wxLogDebug("Deleted mask %s for date %s -- %i -----------------", mask, dateStr, int(date_formats_temp.size()));
        }

        if (m_userDefinedDateMask) break;
    }
}

void mmQIFImportDialog::getDateMask()
{
    //Check parsing results
    int i = 0;
    for (const auto& d : m_date_parsing_stat)
    {
        if (d.second > i)
        {
            i = d.second;
            if (!m_userDefinedDateMask)
            {
                m_dateFormatStr = d.first;
                choiceDateFormat_->SetStringSelection(m_dateFormatStr);
            }
        }
        wxLogDebug("%i \t%s \t%i", i++, date_formats_map()[d.first], d.second);
    }
}

void mmQIFImportDialog::OnFileSearch(wxCommandEvent& /*event*/)
{
    this->Freeze();
    m_FileNameStr = file_name_ctrl_->GetValue();

    const wxString choose_ext = _("QIF Files");
    m_FileNameStr = wxFileSelector(_("Choose QIF data file to Import")
        , wxEmptyString, m_FileNameStr, wxEmptyString
        , choose_ext + " (*.qif)|*.qif;*.QIF"
        , wxFD_OPEN | wxFD_CHANGE_DIR | wxFD_FILE_MUST_EXIST);

    if (!m_FileNameStr.IsEmpty()) {
        correctEmptyFileExt("qif", m_FileNameStr);

        log_field_->ChangeValue("");
        file_name_ctrl_->SetValue(m_FileNameStr);
        mmReadQIFFile();
        m_userDefinedDateMask = false;
        getDateMask();
    }
    this->Thaw();
}

void mmQIFImportDialog::OnDateMaskChange(wxCommandEvent& /*event*/)
{
    wxStringClientData* data = (wxStringClientData*)(choiceDateFormat_->GetClientObject(choiceDateFormat_->GetSelection()));
    if (data) m_dateFormatStr = data->GetData();
    if (m_FileNameStr.IsEmpty())
        return;
    m_userDefinedDateMask = true;
}

void mmQIFImportDialog::OnCheckboxClick( wxCommandEvent& /*event*/ )
{
    fromDateCtrl_->Enable(dateFromCheckBox_->IsChecked());
    toDateCtrl_->Enable(dateToCheckBox_->IsChecked());
    if (accountCheckBox_->IsChecked()
        && !Model_Account::instance().all_checking_account_names().empty())
    {
        accountDropDown_->Enable(true);
        m_accountNameStr = "";
        wxStringClientData* data_obj = (wxStringClientData*) accountDropDown_->GetClientObject(accountDropDown_->GetSelection());
        if (data_obj)
            m_accountNameStr = data_obj->GetData();
    }
    else {
        accountDropDown_->Enable(false);
        accountCheckBox_->SetValue(false);
    }
}

void mmQIFImportDialog::OnOk(wxCommandEvent& /*event*/)
{
    wxString sMsg;
    wxMessageDialog msgDlg(this, _("Do you want to import all transaction ?")
        + "\n" + _("All missing account, payees and categories will be created.")
        , _("Confirm Import")
        , wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
    if (msgDlg.ShowModal() == wxID_YES)
    {
        int numTransactions = vQIF_trxs_.size();
        wxProgressDialog progressDlg(_("Please wait"), _("Importing")
            , numTransactions + 1, this, wxPD_APP_MODAL | wxPD_CAN_ABORT | wxPD_AUTO_HIDE);
        progressDlg.Update(1, _("Importing Accounts"));
        if (getOrCreateAccounts() == 0 && !accountCheckBox_->GetValue()) {
            progressDlg.Destroy();
            mmShowErrorMessageInvalid(this, _("Account"));
            return;
        }
        mmWebApp::MMEX_WebApp_UpdateAccount();
        progressDlg.Update(1, _("Importing Payees"));
        getOrCreatePayees();
        mmWebApp::MMEX_WebApp_UpdatePayee();
        progressDlg.Update(1, _("Importing Categories"));
        getOrCreateCategories();
        mmWebApp::MMEX_WebApp_UpdateCategory();

        Model_Checking::Data_Set trx_data_set;
        Model_Checking::Data_Set transfer_to_data_set;
        Model_Checking::Data_Set transfer_from_data_set;
        int count = 0;
        m_today = wxDateTime::Today();
        const wxString transferStr = Model_Checking::all_type()[Model_Checking::TRANSFER];
        for (const auto& entry : vQIF_trxs_)
        {
            if (count % 100 == 0 || count == numTransactions)
            {
                if (!progressDlg.Update(count
                    , wxString::Format(_("Importing transaction %i of %i"), count, numTransactions))) // if cancel clicked
                        break; // abort processing
            }
            //
            Model_Checking::Data *trx = Model_Checking::instance().create();
            if (createTransaction(entry, trx)) {
                if (trx->TRANSCODE == transferStr && trx->TOTRANSAMOUNT > 0.0)
                    transfer_from_data_set.push_back(*trx);
                else if (trx->TRANSCODE == transferStr && trx->TOTRANSAMOUNT <= 0.0)
                    transfer_to_data_set.push_back(*trx);
                else
                    trx_data_set.push_back(*trx);
            }
            ++count;
        }

        progressDlg.Update(count, _("Importing Transfers"));
        mergeTransferPair(transfer_to_data_set, transfer_from_data_set);
        appendTransfers(trx_data_set, transfer_to_data_set);
        Model_Checking::instance().save(trx_data_set);
        progressDlg.Update(count, _("Importing Split transactions"));
        joinSplit(trx_data_set, m_splitDataSets);
        saveSplit();

        sMsg = _("Import finished successfully");
        btnOK_->Enable(false);
        progressDlg.Destroy();
        trx_data_set.clear();
    }
    else
    {
        sMsg = _("Imported transactions discarded by user!"); //TODO: strange message may be _("Import has discarded by user!")
    }
    wxMessageDialog(this, sMsg, _("QIF Import"), wxOK | wxICON_WARNING).ShowModal();
    *log_field_ << sMsg << "\n";

    vQIF_trxs_.clear();
    refreshTabs(2 + 4 + 8);
}

void mmQIFImportDialog::saveSplit()
{
    while (!m_splitDataSets.empty()) {
        Model_Splittransaction::instance().save(m_splitDataSets.back());
        m_splitDataSets.pop_back();
    }
}
void mmQIFImportDialog::joinSplit(Model_Checking::Data_Set &destination, std::vector <Model_Splittransaction::Data_Set> &target)
{
    for (auto &item : destination)
    {
        if (item.CATEGID != -1) continue;
        if (item.SUBCATEGID == -1) continue; //TODO:: may be std::find(...)
        for (auto &split_item : target.at(item.SUBCATEGID))
            split_item.TRANSID = item.TRANSID;
        item.SUBCATEGID = -1;
    }
}

void mmQIFImportDialog::appendTransfers(Model_Checking::Data_Set &destination, Model_Checking::Data_Set &target)
{
    while (!target.empty()) {
        destination.push_back(target.back());
        target.pop_back();
    }
    //for (const auto& t : target)
    //    destination.push_back(t);
}

bool mmQIFImportDialog::mergeTransferPair(Model_Checking::Data_Set &to, Model_Checking::Data_Set &from)
{
    if (to.empty() || from.empty()) return false; //Nothing to merge

    for (auto& refTrxTo : to)
    {
        int i = -1;
        for (auto& refTrxFrom : from)
        {
            ++i;
            if (refTrxTo.ACCOUNTID != refTrxFrom.TOACCOUNTID) continue;
            if (refTrxTo.TRANSACTIONNUMBER != refTrxFrom.TRANSACTIONNUMBER) continue;
            if (refTrxTo.NOTES != refTrxFrom.NOTES) continue;
            if (Model_Checking::TRANSDATE(refTrxFrom) != Model_Checking::TRANSDATE(refTrxFrom)) continue;
            refTrxTo.TOTRANSAMOUNT = refTrxFrom.TRANSAMOUNT;
            from.erase(from.begin() + i);
            break;
        }
    }

    return true;
}

bool mmQIFImportDialog::createTransaction(/*in*/ const std::map <int, wxString> &i
    , /*out*/ Model_Checking::Data* &trx)
{
    bool validTransaction = true;
    auto t = i;
    const wxString date = (t.find(Date) == t.end() ? "" : t[Date]);
    wxDateTime dtdt;
    dtdt.ParseFormat(date, m_dateFormatStr, m_today);
    trx->TRANSDATE = dtdt.FormatISODate();
    wxString accountName = (t.find(AccountName) == t.end() ? m_accountNameStr : t[AccountName]);
    if (accountName.empty()) accountName = m_accountNameStr;
    int accountID = m_QIFaccountsID.at(t[AccountName]);
    if (accountID == -1) validTransaction = false;
    trx->ACCOUNTID = (accountID);
    trx->TOACCOUNTID = (t.find(ToAccountName) == t.end() ? -1 : m_QIFaccountsID[t[ToAccountName]]);
    trx->PAYEEID = (t.find(Payee) == t.end() ? -1 : m_QIFpayeeNames[t.at(Payee)]); //TODO: transfer?
    trx->TRANSACTIONNUMBER = (t.find(TransNumber) == t.end() ? "" : t[TransNumber]);
    trx->NOTES = (t.find(Memo) == t.end() ? "" : t[Memo]);
    trx->STATUS = "";
    trx->FOLLOWUPID = -1;
    double amt;
    wxString(t.find(Amount) == t.end() ? "" : t[Amount]).ToDouble(&amt);
    trx->TRANSAMOUNT = fabs(amt);
    trx->TOTRANSAMOUNT = amt;
    trx->TRANSCODE = (t.find(TrxType) == t.end() ? "" : t[TrxType]);

    if (t.find(CategorySplit) == t.end()) {
        trx->CATEGID = (t.find(Category) == t.end() ? -1 : m_QIFcategoryNames[t[Category]].first);
        trx->SUBCATEGID = (t.find(Category) == t.end() ? -1 : m_QIFcategoryNames[t[Category]].second);
    }
    else
    {
        Model_Splittransaction::Data_Set split;
        wxStringTokenizer token(t[CategorySplit], "\n");
        wxStringTokenizer amtToken(t.find(AmountSplit) == t.end() ? "" : t[AmountSplit], "\n");
        while (token.HasMoreTokens()) {
            const wxString c = token.GetNextToken();
            Model_Splittransaction::Data* s = Model_Splittransaction::instance().create();
            s->CATEGID = m_QIFcategoryNames[c].first;
            s->SUBCATEGID = m_QIFcategoryNames[c].second;
            double amount;
            amtToken.GetNextToken().ToDouble(&amount);
            s->SPLITTRANSAMOUNT = (trx->TRANSCODE == Model_Checking::all_type()[Model_Checking::DEPOSIT] ? amount : -amount);
            s->TRANSID = trx->TRANSID;
            split.push_back(*s);
        }
        trx->SUBCATEGID = m_splitDataSets.size();
        m_splitDataSets.push_back(split);
    }
    return validTransaction;
}

void mmQIFImportDialog::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void mmQIFImportDialog::OnQuit(wxCloseEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

int mmQIFImportDialog::getOrCreateAccounts()
{
    m_QIFaccountsID.clear();

    for (const auto &item : m_QIFaccounts)
    {
        int accountID = -1;
        Model_Account::Data* acc = Model_Account::instance().get(item.first);
        if (!acc)
        {
            Model_Account::Data *account = Model_Account::instance().create();

            account->FAVORITEACCT = "TRUE";
            account->STATUS = Model_Account::all_status()[Model_Account::OPEN];
            account->ACCOUNTTYPE = Model_Account::all_type()[Model_Account::CHECKING];
            account->ACCOUNTNAME = item.first;
            account->INITIALBAL = 0;
            account->CURRENCYID = Model_Currency::GetBaseCurrency()->CURRENCYID;
            for (const auto& curr : Model_Currency::instance().all())
            {
                const wxString c = (item.second.find(Date) == item.second.end() ? "" : item.second.at(Date));
                if (wxString::Format("[%s]", curr.CURRENCY_SYMBOL) == c) {
                    account->CURRENCYID = curr.CURRENCYID;
                    break;
                }
            }
            accountID = Model_Account::instance().save(account);
            wxString sMsg = wxString::Format(_("Added account: %s"), item.first);
            *log_field_ << sMsg << "\n";
        }
        else
            accountID = acc->ACCOUNTID;

        m_QIFaccountsID[item.first] = accountID;
    }

    if (m_QIFaccountsID.size() == 0)
    {
        Model_Account::Data* acc = Model_Account::instance().get(m_accountNameStr);
        if (acc) {
            m_QIFaccountsID[m_accountNameStr] = acc->ACCOUNTID;
        }
    }
    return m_QIFaccountsID.size();
}

void mmQIFImportDialog::getOrCreatePayees()
{
    Model_Payee::Data_Set data_set;
    for (const auto &item : m_QIFpayeeNames)
    {
        int payeeID = -1;
        Model_Payee::Data_Set payees = Model_Payee::instance().find(Model_Payee::PAYEENAME(item.first));
        if (payees.empty())
        {
            Model_Payee::Data *p = Model_Payee::instance().create();
            p->PAYEENAME = item.first;
            p->CATEGID = -1;
            p->SUBCATEGID = -1;
            data_set.push_back(*p);
            //payeeID = Model_Payee::instance().save(p);
            wxString sMsg = wxString::Format(_("Added payee: %s"), item.first);
            log_field_->AppendText(wxString() << sMsg << "\n");
        }
        else
            data_set.push_back(*payees.begin());
    }

    Model_Payee::instance().save(data_set);
    for (const auto& entry : data_set) {
        m_QIFpayeeNames[entry.PAYEENAME] = entry.PAYEEID;
    }
}

void mmQIFImportDialog::getOrCreateCategories()
{
    const auto all_categs = Model_Category::all_categories();
    for (const auto &item : m_QIFcategoryNames)
    {
        wxStringTokenizer token(item.first, ":");
        const wxString categStr = token.GetNextToken();
        const wxString subcategStr = token.GetNextToken();
        int categID = -1;
        int subcategID = -1;
        if (all_categs.find(item.first) == all_categs.end())
        {
            if (Model_Category::instance().find(Model_Category::CATEGNAME(categStr)).empty()) {
                Model_Category::Data *c = Model_Category::instance().create();
                c->CATEGNAME = categStr;
                categID = Model_Category::instance().save(c);
            }
            else
                categID = Model_Category::instance().find(Model_Category::CATEGNAME(categStr)).begin()->CATEGID;

            if (!subcategStr.empty() && categID != -1) {
                Model_Subcategory::Data *subcategory = Model_Subcategory::instance().create();
                subcategory->SUBCATEGNAME = subcategStr;
                subcategory->CATEGID = categID;
                subcategID = Model_Subcategory::instance().save(subcategory);
            }

            const wxString sMsg = wxString::Format(_("Added category: %s = %s")
                , item.first
                , Model_Category::instance().full_name(categID, subcategID));
            wxLogDebug("%s", sMsg);
            log_field_->AppendText(wxString() << sMsg << "\n");
        }
        else {
            categID = all_categs.at(item.first).first ;
            subcategID = all_categs.at(item.first).second;
        }

        m_QIFcategoryNames[item.first] = std::make_pair(categID, subcategID);
    }
}
