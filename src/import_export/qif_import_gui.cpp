/*******************************************************
Copyright (C) 2013-2020 Nikolay Akimov
Copyright (C) 2022  Mark Whalley (mark@ipx.co.uk)

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

#include <wx/progdlg.h>
#include <wx/dataview.h>

#include "qif_import_gui.h"
#include "qif_import.h"
#include "export.h"
#include "constants.h"
#include "mmSimpleDialogs.h"
#include "paths.h"
#include "util.h"
#include "webapp.h"
#include "option.h"
#include "payeedialog.h"
#include "categdialog.h"

#include "model/Model_Setting.h"
#include "model/Model_Category.h"
#include "model/Model_Payee.h"
#include "model/Model_Tag.h"

#include "reports/mmDateRange.h"

enum tab_id {
    LOG_TAB = 1,
    TRX_TAB = 2,
    ACC_TAB = 4,
    PAYEE_TAB = 8,
    CAT_TAB = 16,
};

wxIMPLEMENT_DYNAMIC_CLASS(mmQIFImportDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmQIFImportDialog, wxDialog)
EVT_CHECKBOX(wxID_ANY, mmQIFImportDialog::OnCheckboxClick)
EVT_BUTTON(wxID_OK, mmQIFImportDialog::OnOk)
EVT_MENU(wxID_HIGHEST, mmQIFImportDialog::OnMenuSelected)
EVT_BUTTON(wxID_CANCEL, mmQIFImportDialog::OnCancel)
EVT_CHOICE(ID_ACCOUNT, mmQIFImportDialog::OnAccountChanged)
EVT_CLOSE(mmQIFImportDialog::OnQuit)
wxEND_EVENT_TABLE()

mmQIFImportDialog::mmQIFImportDialog(wxWindow* parent, int64 account_id, const wxString& file_path)
    : m_FileNameStr(file_path)
    , m_today(wxDate::Today())
    , m_fresh(wxDate::Today().Subtract(wxDateSpan::Months(1)))     
{
    decimal_ = Model_Currency::GetBaseCurrency()->DECIMAL_POINT;
    payeeIsNotes_ = false;
    const auto& acc = Model_Account::instance().get(account_id);
    if (acc)
        m_accountNameStr = acc->ACCOUNTNAME;

    this->SetFont(parent->GetFont());
    Create(parent);
    SetMinSize(wxSize(500, 300));
}

wxString mmQIFImportDialog::OnGetItemText(long item, long column) const
{
    return vQIF_trxs_.at(item).at(column);
}

bool mmQIFImportDialog::Create(wxWindow* parent, wxWindowID id, const wxString& caption
    , const wxPoint& pos, const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    ColName_[LIST_ID_ID] = "#";
    ColName_[LIST_ID_ACCOUNT] = _t("Account");
    ColName_[LIST_ID_DATE] = _t("Date");
    ColName_[LIST_ID_NUMBER] = _t("Number");
    ColName_[LIST_ID_PAYEE] = _t("Payee");
    ColName_[LIST_ID_TYPE] = _t("Type");
    ColName_[LIST_ID_CATEGORY] = _t("Category");
    ColName_[LIST_ID_TAGS] = _t("Tags");
    ColName_[LIST_ID_VALUE] = _t("Value");
    ColName_[LIST_ID_NOTES] = _t("Notes");

    CreateControls();
    if (m_FileNameStr != wxEmptyString)
        mmReadQIFFile();
    else
        fillControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    this->SetInitialSize();
    SetIcon(mmex::getProgramIcon());
    Centre();

    return true;
}

void mmQIFImportDialog::CreateControls()
{
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(main_sizer);
    wxBoxSizer* left_sizer = new wxBoxSizer(wxVERTICAL);

    //File to import, file path and browse button
    wxPanel* file_panel = new wxPanel(this
        , wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    main_sizer->Add(file_panel, 0, wxEXPAND | wxALL, 1);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    file_panel->SetSizer(itemBoxSizer7);

    wxStaticText* file_name_label = new wxStaticText(file_panel, wxID_ANY, _t("File Name:"));
    itemBoxSizer7->Add(file_name_label, g_flagsH);

    wxArrayString files = Model_Setting::instance().getArrayString("RECENT_QIF_FILES");
    file_name_ctrl_ = new  wxComboBox(file_panel, wxID_FILE, m_FileNameStr, wxDefaultPosition, wxDefaultSize, files, wxTE_PROCESS_ENTER);
    file_name_ctrl_->SetMinSize(wxSize(300, -1));
    itemBoxSizer7->Add(file_name_ctrl_, 1, wxALL | wxGROW, 5);
    file_name_ctrl_->Connect(wxID_FILE
        , wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(mmQIFImportDialog::OnFileNameChanged), nullptr, this);
    file_name_ctrl_->Connect(wxID_FILE
        , wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(mmQIFImportDialog::OnFileNameChanged), nullptr, this);

    button_search_ = new wxButton(file_panel, wxID_OPEN, _tu("&Browse…"));
    itemBoxSizer7->Add(button_search_, g_flagsH);
    button_search_->Connect(wxID_OPEN, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmQIFImportDialog::OnFileSearch), nullptr, this);

    wxFlexGridSizer* flex_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    //flex_sizer->AddGrowableCol(1);
    left_sizer->Add(flex_sizer, g_flagsExpand);

    //Encoding
    wxStaticText* itemStaticText88 = new wxStaticText(this, wxID_STATIC, wxString(_t("Encoding:")));
    //itemStaticText88->SetFont(staticBoxFontSetting);
    flex_sizer->Add(itemStaticText88, g_flagsH);

    m_choiceEncoding = new wxChoice(this, wxID_ANY);
    for (const auto &i : g_encoding)
        m_choiceEncoding->Append(wxGetTranslation(i.second.second), new wxStringClientData(i.second.second));
    m_choiceEncoding->SetSelection(0);

    flex_sizer->Add(m_choiceEncoding, g_flagsH);

    //Account
    accountCheckBox_ = new wxCheckBox(this, wxID_FILE5, _t("Account")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    accountDropDown_ = new wxChoice(this, ID_ACCOUNT);
    accountDropDown_->SetMinSize(wxSize(180, -1));
    accountDropDown_->Enable(false);

    for (const auto& a : Model_Account::instance().all_checking_account_names()) {
        accountDropDown_->Append(a, new wxStringClientData(a));
        if (a == m_accountNameStr) {
            accountDropDown_->SetStringSelection(a);
            accountDropDown_->Enable();
            accountCheckBox_->SetValue(true);
        }
    }

    if (accountDropDown_->GetSelection() == wxNOT_FOUND) {
        accountDropDown_->Enable(false);
    }

    flex_sizer->Add(accountCheckBox_, g_flagsH);
    flex_sizer->Add(accountDropDown_, g_flagsH);

    //Filtering Details --------------------------------------------
    wxStaticBox* static_box = new wxStaticBox(this, wxID_ANY, _t("Filtering Details:"));
    wxStaticBoxSizer* filter_sizer = new wxStaticBoxSizer(static_box, wxVERTICAL);

    // Create a horizontal sizer to hold the date and duplicate columns
    wxBoxSizer* filter_grid_sizer = new wxBoxSizer(wxHORIZONTAL);
    filter_sizer->Add(filter_grid_sizer, g_flagsExpand);

    // Left column for dates
    wxFlexGridSizer* dates_sizer = new wxFlexGridSizer(0, 2, 0, 0);

    // From Date
    dateFromCheckBox_ = new wxCheckBox(static_box, wxID_FILE8, _t("From Date")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    fromDateCtrl_ = new mmDatePickerCtrl(static_box, wxID_STATIC, wxDefaultDateTime
        , wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
    fromDateCtrl_->Enable(false);
    dates_sizer->Add(dateFromCheckBox_, g_flagsH);
    dates_sizer->Add(fromDateCtrl_, g_flagsH);

    // To Date
    dateToCheckBox_ = new wxCheckBox(static_box, wxID_FILE9, _t("To Date")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    toDateCtrl_ = new mmDatePickerCtrl(static_box, wxID_STATIC, wxDefaultDateTime
        , wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
    toDateCtrl_->Enable(false);
    dates_sizer->Add(dateToCheckBox_, g_flagsH);
    dates_sizer->Add(toDateCtrl_, g_flagsH);

    // Right column for duplicates
    wxFlexGridSizer* dup_sizer = new wxFlexGridSizer(0, 2, 0, 0);

    // Duplicate Transactions Method
    dupTransCheckBox_ = new wxCheckBox(static_box, wxID_FILE3, _t("Duplicates")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    dupTransMethod_ = new wxChoice(static_box, wxID_ANY);
    dupTransMethod_->Append(_t("By transaction number"));
    dupTransMethod_->Append(_t("By amount and exact date"));
    dupTransMethod_->Append(_t("By amount and nearby date"));
    dupTransMethod_->SetSelection(0);
    dupTransMethod_->Enable(false);
    dup_sizer->Add(dupTransCheckBox_, g_flagsH);
    dup_sizer->Add(dupTransMethod_, g_flagsH);

    // Duplicate Transactions Action
    wxStaticText* dupTransActionLabel = new wxStaticText(static_box, wxID_STATIC, _t("Action"));
    dupTransAction_ = new wxChoice(static_box, wxID_ANY);
    dupTransAction_->Append(_t("Skip"));
    dupTransAction_->Append(_t("Flag as duplicate"));
    dupTransAction_->SetSelection(0);
    dupTransAction_->Enable(false);
    dup_sizer->Add(dupTransActionLabel, g_flagsH);
    dup_sizer->Add(dupTransAction_, g_flagsH);

    filter_grid_sizer->Add(dates_sizer, g_flagsH);

    // Add vertical line separator
    wxStaticLine* vline = new wxStaticLine(static_box, wxID_ANY, wxDefaultPosition,
        wxDefaultSize, wxLI_VERTICAL);
    filter_grid_sizer->Add(vline, wxSizerFlags().Left().Border(wxLEFT | wxRIGHT, 5).Expand());

    filter_grid_sizer->Add(dup_sizer, g_flagsH);

    //Data viewer ----------------------------------------------
    wxNotebook* qif_notebook = new wxNotebook(this
        , wxID_FILE9, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE);

    wxPanel* log_tab = new wxPanel(qif_notebook, wxID_ANY);
    qif_notebook->AddPage(log_tab, _t("Log"));
    wxBoxSizer* log_sizer = new wxBoxSizer(wxVERTICAL);
    log_tab->SetSizer(log_sizer);

    log_field_ = new wxTextCtrl(log_tab, wxID_ANY, ""
        , wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxHSCROLL);
    log_field_->SetMinSize(wxSize(500, -1));
    log_sizer->Add(log_field_, g_flagsExpand);

    wxPanel* data_tab = new wxPanel(qif_notebook, wxID_ANY);
    qif_notebook->AddPage(data_tab, _t("Data"));
    wxBoxSizer* data_sizer = new wxBoxSizer(wxVERTICAL);
    data_tab->SetSizer(data_sizer);

    dataListBox_ = new wxDataViewListCtrl(data_tab, wxID_ANY);
    dataListBox_->SetMinSize(wxSize(100, 200));
    dataListBox_->AppendTextColumn(ColName_[LIST_ID_ID], wxDATAVIEW_CELL_INERT, 40, wxALIGN_RIGHT);
    dataListBox_->AppendTextColumn(ColName_[LIST_ID_ACCOUNT], wxDATAVIEW_CELL_INERT, 120, wxALIGN_LEFT);
    dataListBox_->AppendTextColumn(ColName_[LIST_ID_DATE], wxDATAVIEW_CELL_INERT, 90, wxALIGN_RIGHT);
    dataListBox_->AppendTextColumn(ColName_[LIST_ID_NUMBER], wxDATAVIEW_CELL_INERT, 80, wxALIGN_RIGHT);
    dataListBox_->AppendTextColumn(ColName_[LIST_ID_PAYEE], wxDATAVIEW_CELL_INERT, 120, wxALIGN_LEFT);
    dataListBox_->AppendTextColumn(ColName_[LIST_ID_TYPE], wxDATAVIEW_CELL_INERT, 60, wxALIGN_LEFT);
    dataListBox_->AppendTextColumn(ColName_[LIST_ID_CATEGORY], wxDATAVIEW_CELL_INERT, 140, wxALIGN_LEFT);
    dataListBox_->AppendTextColumn(ColName_[LIST_ID_TAGS], wxDATAVIEW_CELL_INERT, 140, wxALIGN_LEFT);
    dataListBox_->AppendTextColumn(ColName_[LIST_ID_VALUE], wxDATAVIEW_CELL_INERT, 100, wxALIGN_RIGHT);
    dataListBox_->AppendTextColumn(ColName_[LIST_ID_NOTES], wxDATAVIEW_CELL_INERT, 300, wxALIGN_LEFT);
    data_sizer->Add(dataListBox_, g_flagsExpand);

    //Accounts
    wxPanel* acc_tab = new wxPanel(qif_notebook, wxID_ANY);
    qif_notebook->AddPage(acc_tab, _t("Account"));
    wxBoxSizer* acc_sizer = new wxBoxSizer(wxHORIZONTAL);
    acc_tab->SetSizer(acc_sizer);

    accListBox_ = new wxDataViewListCtrl(acc_tab, wxID_ANY);
    accListBox_->AppendTextColumn(_t("Name"), wxDATAVIEW_CELL_INERT, 250, wxALIGN_LEFT);
    accListBox_->AppendTextColumn(_t("Type"), wxDATAVIEW_CELL_INERT, 50, wxALIGN_LEFT);
    accListBox_->AppendTextColumn(_t("Currency"), wxDATAVIEW_CELL_INERT, 50, wxALIGN_LEFT);
    accListBox_->AppendTextColumn(_t("Status"), wxDATAVIEW_CELL_INERT, 150, wxALIGN_LEFT);
    acc_sizer->Add(accListBox_, g_flagsExpand);

    //Payees
    wxPanel* payee_tab = new wxPanel(qif_notebook, wxID_ANY);
    qif_notebook->AddPage(payee_tab, _t("Payee"));
    wxBoxSizer* payee_sizer = new wxBoxSizer(wxHORIZONTAL);
    payee_tab->SetSizer(payee_sizer);

    payeeListBox_ = new wxDataViewListCtrl(payee_tab, wxID_ANY);
    payeeListBox_->AppendTextColumn(_t("Name"), wxDATAVIEW_CELL_INERT, 250, wxALIGN_LEFT);
    payeeListBox_->AppendTextColumn(_t("Status"), wxDATAVIEW_CELL_INERT, 150, wxALIGN_LEFT);
    payee_sizer->Add(payeeListBox_, g_flagsExpand);

    //Category
    wxPanel* categ_tab = new wxPanel(qif_notebook, wxID_ANY);
    qif_notebook->AddPage(categ_tab, _t("Category"));
    wxBoxSizer* category_sizer = new wxBoxSizer(wxHORIZONTAL);
    categ_tab->SetSizer(category_sizer);
    categoryListBox_ = new wxDataViewListCtrl(categ_tab, wxID_ANY);
    categoryListBox_->AppendTextColumn(_t("Name"), wxDATAVIEW_CELL_INERT, 250, wxALIGN_LEFT);
    categoryListBox_->AppendTextColumn(_t("Status"), wxDATAVIEW_CELL_INERT, 150, wxALIGN_LEFT);
    category_sizer->Add(categoryListBox_, g_flagsExpand);

    payeeListBox_->GetMainWindow()->Bind(wxEVT_LEFT_DCLICK, &mmQIFImportDialog::OnShowPayeeDialog, this);
    categoryListBox_->GetMainWindow()->Bind(wxEVT_LEFT_DCLICK, &mmQIFImportDialog::OnShowCategDialog, this);

    //Compose all sizers togethe
    wxBoxSizer* top_sizer = new wxBoxSizer(wxHORIZONTAL);
    top_sizer->Add(left_sizer, g_flagsH);
    top_sizer->Add(filter_sizer, g_flagsH);

    //Use account number instead of account name :
    accountNumberCheckBox_ = new wxCheckBox(this, wxID_FILE6, _t("Use account number instead of account name")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);

    //Use payee as desc :
    payeeIsNotesCheckBox_ = new wxCheckBox(this, wxID_FILE7, _t("Include payee field in notes")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);

    //Pattern match Payees :
    payeeMatchCheckBox_ = new wxCheckBox(this, mmID_PAYEE, _t("Pattern match Payees")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    payeeRegExInitialized_ = false;

    //Append payee match info to notes :
    payeeMatchAddNotes_ = new wxCheckBox(this, wxID_ANY, _t("Add match details to Notes")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    payeeMatchAddNotes_->Disable();

    // Date Format Settings
    m_dateFormatStr = Option::instance().getDateFormat();

    wxStaticText* dateFormat = new wxStaticText(this, wxID_STATIC, _t("Date Format"));
    choiceDateFormat_ = new wxComboBox(this, wxID_ANY);
    wxArrayString auto_fills;
    for (const auto& i : g_date_formats_map())
    {
        choiceDateFormat_->Append(i.second, new wxStringClientData(i.first));
        auto_fills.Add(i.second);
        if (m_dateFormatStr == i.first) choiceDateFormat_->SetStringSelection(i.second);
    }
    choiceDateFormat_->AutoComplete(auto_fills);
    choiceDateFormat_->Connect(wxID_ANY, wxEVT_COMMAND_COMBOBOX_SELECTED
        , wxCommandEventHandler(mmQIFImportDialog::OnDateMaskChange), nullptr, this);

    wxFlexGridSizer* flex_sizer_b = new wxFlexGridSizer(0, 3, 0, 0);

    flex_sizer_b->Add(accountNumberCheckBox_, g_flagsBorder1H);
    flex_sizer_b->Add(payeeMatchCheckBox_, g_flagsBorder1H);
    flex_sizer_b->AddSpacer(1);
    flex_sizer_b->Add(payeeIsNotesCheckBox_, g_flagsBorder1H);
    flex_sizer_b->Add(payeeMatchAddNotes_, g_flagsBorder1H);
    flex_sizer_b->AddSpacer(1);

    wxBoxSizer* date_sizer = new wxBoxSizer(wxHORIZONTAL);
    date_sizer->Add(dateFormat, g_flagsBorder1H);
    date_sizer->Add(choiceDateFormat_, g_flagsBorder1H);
    flex_sizer_b->Add(date_sizer, g_flagsH);

    wxStaticText* decamalCharText = new wxStaticText(this, wxID_STATIC, _t("Decimal Char"));
    m_choiceDecimalSeparator = new mmChoiceAmountMask(this, wxID_ANY);
    wxBoxSizer* decamalCharSizer = new wxBoxSizer(wxHORIZONTAL);
    decamalCharSizer->Add(decamalCharText, g_flagsBorder1H);
    decamalCharSizer->Add(m_choiceDecimalSeparator, g_flagsBorder1H);
    m_choiceDecimalSeparator->SetDecimalChar(decimal_);
    m_choiceDecimalSeparator->Connect(wxID_ANY, wxEVT_COMMAND_CHOICE_SELECTED
        , wxCommandEventHandler(mmQIFImportDialog::OnDecimalChange), nullptr, this);

    flex_sizer_b->Add(decamalCharSizer, g_flagsH);
    //

    colorCheckBox_ = new wxCheckBox(this, wxID_PASTE, _t("Color")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    mmColorBtn_ = new mmColorButton(this, wxID_HIGHEST
        , wxSize(m_choiceDecimalSeparator->GetSize().GetY(), m_choiceDecimalSeparator->GetSize().GetY()));
    mmColorBtn_->Enable(false);
    wxBoxSizer* colorSizer = new wxBoxSizer(wxHORIZONTAL);
    colorSizer->Add(colorCheckBox_, g_flagsBorder1H);
    colorSizer->Add(mmColorBtn_, g_flagsBorder1H);
    flex_sizer_b->Add(colorSizer, g_flagsBorder1H);

    wxBoxSizer* inTop_sizer = new wxBoxSizer(wxVERTICAL);
    inTop_sizer->Add(top_sizer, g_flagsV);
    inTop_sizer->Add(flex_sizer_b, g_flagsExpand);
    main_sizer->Add(inTop_sizer, g_flagsV);
    main_sizer->Add(qif_notebook, g_flagsExpand);


    /**********************************************************************************************
     Button Panel with OK and Cancel Buttons
    ***********************************************************************************************/
    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    main_sizer->Add(buttons_panel, wxSizerFlags(g_flagsV).Center());

    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    btnOK_ = new wxButton(buttons_panel, wxID_OK, _t("&OK "));
    wxButton* itemButtonCancel_ = new wxButton(buttons_panel, wxID_CANCEL, wxGetTranslation(g_CloseLabel));
    btnOK_->Connect(wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmQIFImportDialog::OnOk), nullptr, this);

    buttons_sizer->Add(btnOK_, g_flagsH);
    buttons_sizer->Add(itemButtonCancel_, g_flagsH);

    buttons_sizer->Realize();
    Fit();
}

void mmQIFImportDialog::fillControls()
{
    refreshTabs(LOG_TAB | TRX_TAB | ACC_TAB | PAYEE_TAB | CAT_TAB);
    btnOK_->Enable(!file_name_ctrl_->GetValue().IsEmpty());
}

bool mmQIFImportDialog::mmReadQIFFile()
{
    size_t numLines = 0;
    vQIF_trxs_.clear();
    m_QIFaccounts.clear();
    m_accountNameStr.clear();
    m_QIFcategoryNames.clear();
    m_QIFcategoryNames[_t("Unknown")] = -1;
    m_QIFpayeeNames.clear();
    m_payee_names.clear();
    m_payee_names.Add(_t("Unknown"));
    m_duplicateTransactions.clear();  // Clear the list of matched transactions
    wxString catDelimiter = Model_Infotable::instance().getString("CATEG_DELIMITER", ":");

    wxFileInputStream input(m_FileNameStr);
    wxConvAuto conv = g_encoding.at(m_choiceEncoding->GetSelection()).first;
    wxTextInputStream text(input, "\x09", conv);

    wxProgressDialog progressDlg(_t("Please wait"), _t("Scanning")
        , 0, this, wxPD_APP_MODAL | wxPD_CAN_ABORT);

    wxLongLong start = wxGetUTCTimeMillis();
    wxLongLong interval = wxGetUTCTimeMillis() - start;

    wxString accName = "";
    if (accountCheckBox_->IsChecked()) {
        accName = accountDropDown_->GetStringSelection();
        Model_Account::Data* acc = Model_Account::instance().get(accName);
        if (acc) {
            m_accountNameStr = acc->ACCOUNTNAME;
        }
    }

    std::unordered_map <int, wxString> trx;
    int64 split_id = 0;
    wxSharedPtr<mmDates> dParser(new mmDates);
    std::map<wxString, int> comma({ {".", 0}, {",", 0} });
    while (input.IsOk() && !input.Eof())
    {
        ++numLines;
        const wxString lineStr = text.ReadLine();
        if (lineStr.IsEmpty())
            continue;

        if (numLines % 100 == 0)
        {
            interval = wxGetUTCTimeMillis() - start;
            if (!progressDlg.Pulse(wxString::Format(_t("Reading line %zu, %lld ms")
                , numLines, interval)))
                break;
        }
        if (numLines <= 50)
        {
            *log_field_ << wxString::Format(_t("Line %zu \t %s\n"), numLines, lineStr);
            if (numLines == 50)
                *log_field_ << "-------------------------------------- 8< --------------------------------------\n";
        }

        const qifLineType lineType = mmQIFImport::lineType(lineStr);
        auto data = mmQIFImport::getLineData(lineStr);
        if (lineType == EOTLT || input.Eof())
        {
            if (trx.find(AcctType) != trx.end())
            {
                if (trx[AcctType] == "Account") {
                    accName = (trx.find(TransNumber) == trx.end() ? "" : trx[TransNumber]);
                    std::unordered_map <int, wxString> a;
                    a[AccountType] = (trx.find(Description) != trx.end() ? trx.at(Description) : "");
                    a[Description] = (trx.find(AccountType) != trx.end() ? trx.at(AccountType) : "");
                    m_QIFaccounts[accName] = a;
                    m_accountNameStr = accName;
                }
            }

            if (trx[AcctType] != "Account" && completeTransaction(trx, m_accountNameStr)) {
                vQIF_trxs_.push_back(trx);
            }
            trx.clear();
            split_id = 0;
            continue;
        }

        //Parse Categories
        if (lineType == CategorySplit || lineType == Category)
        {
            if (data.empty())
                data = _t("Unknown");
            wxRegEx regex(" ?: ?");
            regex.Replace(&data, catDelimiter);
            wxString catStr = data.BeforeFirst('/');
            if (!catStr.IsEmpty())
            {
                if (catStr.Left(1) == "[" && catStr.Last() == ']')
                    catStr = _t("Transfer");
                m_QIFcategoryNames[catStr] = -1;
            }
        }

        //Parse date format
        if (!m_userDefinedDateMask && lineType == Date && (data.Mid(0, 1) != "["))
        {
            dParser->doHandleStatistics(data);
        }

        //Parse numbers
        if (lineType == Amount || lineType == AmountSplit)
        {
            comma["."] += data.Contains(".") ? data.find(".") + 1 : 0;
            comma[","] += data.Contains(",") ? data.find(",") + 1 : 0;
        }

        if (lineType == CategorySplit)
            split_id++;

        if (lineType == AcctType)
            trx[lineType] = data;
        else
        {
            wxString prefix;
            if (!trx[lineType].empty())
                prefix = "\n";

            if (lineType == MemoSplit)
                data.Prepend(wxString::Format("%lld:", split_id));
            
            trx[lineType] += prefix + data;
        }

    }
    log_field_->ScrollLines(log_field_->GetNumberOfLines());

    if (comma[","] > comma["."]) {
        m_choiceDecimalSeparator->SetDecimalChar(",");
    }
    if (comma["."] > comma[","]) {
        m_choiceDecimalSeparator->SetDecimalChar(".");
    }

    if (!m_userDefinedDateMask)
    {
        dParser->doFinalizeStatistics();
        if (dParser->isDateFormatFound()) {
            m_dateFormatStr = dParser->getDateFormat();
            const wxString date_mask = dParser->getDateMask();
            choiceDateFormat_->SetStringSelection(date_mask);
        }
    }

    fillControls();

    progressDlg.Destroy();

    interval = wxGetUTCTimeMillis() - start;
    wxString sMsg = wxString::Format(_t("Number of lines read from QIF file: %zu in %lld ms")
        , numLines, interval);
    *log_field_ << sMsg << "\n";

    if (!m_QIFaccounts.empty()) {
        sMsg = _t("Accounts:");
        for (const auto& i : m_QIFaccounts) {
            sMsg += ("\n" + i.first);
        }
        *log_field_ << sMsg << "\n";
    }

    sMsg = _t("Date Format: ") + "\n" + choiceDateFormat_->GetValue();
    *log_field_ << sMsg << "\n";

    sMsg = wxString::Format(_t("Press OK Button to continue"));
    *log_field_ << sMsg << "\n";

    return true;
}

bool mmQIFImportDialog::completeTransaction(std::unordered_map<int, wxString> &trx, const wxString &accName)
{
    if (trx.find(Date) == trx.end())
        return false;

    bool isTransfer = false;

    if (accName.empty()) {
        trx[AccountName] = m_accountNameStr;
    }
    else {
        trx[AccountName] = accName;
    }


    if (trx.find(CategorySplit) != trx.end())
    {
        //TODO:Dublicate code
        wxStringTokenizer token(trx[CategorySplit], "\n");
        while (token.HasMoreTokens())
        {
            wxString c = token.GetNextToken();
            const wxString project = qif_api->getFinancistoProject(c);
            if (!project.empty())
                trx[TransNumber] += project + "\n"; //TODO: trx number or notes
        }
    }

    if (trx[Payee] == "Opening Balance") {
        trx[Memo] += (trx[Memo].empty() ? "" : "\n") + trx[Payee];
        trx[Category] = trx[Payee];
    }

    if (trx.find(Category) != trx.end())
    {
        wxString tags;
        wxString categname = trx[Category].BeforeFirst('/', &tags);
        if (categname.Left(1) == "[" && categname.Last() == ']')
        {
            wxString toAccName = categname.SubString(1, categname.length() - 2);

            if (toAccName == m_accountNameStr)
            {
                trx[Category] = trx[Payee];
                trx[Payee] = toAccName;
            }
            else
            {
                isTransfer = true;
                trx[Category] = _t("Transfer") + (!tags.IsEmpty() ? "/" + tags : "");
                trx[TrxType] = Model_Checking::TYPE_NAME_TRANSFER;
                trx[ToAccountName] = toAccName;
                trx[Memo] += (trx[Memo].empty() ? "" : "\n") + trx[Payee];
                if (m_QIFaccounts.find(toAccName) == m_QIFaccounts.end())
                {
                    std::unordered_map<int, wxString> a;
                    a[Description] = "[" + Model_Currency::GetBaseCurrency()->CURRENCY_SYMBOL + "]";
                    a[AccountType] = (trx.find(Description) != trx.end() ? trx.at(Description) : "");
                    m_QIFaccounts[toAccName] = a;
                }
            }
        }
    }

    if (!isTransfer)
    {
        wxString payee_name = trx.find(Payee) != trx.end() ? trx[Payee] : "";
        if (payee_name.empty() && trx[AcctType] != "Account" )
        {
            payee_name = trx.find(AccountName) != trx.end() ? trx[AccountName] : _t("Unknown");
            trx[Payee] = payee_name;
        }

        if (!payee_name.empty())
        {
            int i = m_payee_names.Index(payee_name, false);
            if (i == wxNOT_FOUND)
            {
                m_payee_names.Add(payee_name);
            }
            else
                trx[Payee] = m_payee_names.Item(i);

            if (payee_name == "Opening Balance")
                m_QIFcategoryNames["Opening Balance"] = -1;

        }
    }

    if (payeeIsNotes_) {
        trx[Memo] += (trx[Memo].empty() ? "" : "\n") + trx[Payee];
    }

    wxString amtStr = (trx.find(Amount) == trx.end() ? "" : trx[Amount]);
    if (!isTransfer) {
        if (amtStr.Mid(0, 1) == "-")
            trx[TrxType] = Model_Checking::TYPE_NAME_WITHDRAWAL;
        else if (!amtStr.empty())
            trx[TrxType] = Model_Checking::TYPE_NAME_DEPOSIT;
    }

    return !amtStr.empty();
}

void mmQIFImportDialog::refreshTabs(int tabs)
{
    int num = 0;
    if (tabs & TRX_TAB)
    {
        wxString acc;
        dataListBox_->DeleteAllItems();
        for (const auto& trx : vQIF_trxs_)
        {
            wxVector<wxVariant> data;
            Model_Account::Data* account = Model_Account::instance().getByAccNum(trx.at(AccountName));
            data.push_back(wxVariant(wxString::Format("%i", num + 1)));
            data.push_back(
                wxVariant(
                    trx.find(AccountName) != trx.end()
                    && (trx.at(AccountName).empty() || accountCheckBox_->IsChecked())
                    ? m_accountNameStr
                    : ((accountNumberCheckBox_->IsChecked() && account)
                        ? account->ACCOUNTNAME : trx.at(AccountName)
                        )
                )
            );

            wxString dateStr = "";
            if (trx.find(Date) != trx.end())
            {
                dateStr = trx.at(Date);
                dateStr.Replace(" ", "");
                wxDateTime dtdt;
                wxString::const_iterator end;
                if (dtdt.ParseFormat(dateStr, m_dateFormatStr, &end))
                    dateStr = mmGetDateTimeForDisplay(dtdt.FormatISODate());
                else
                    dateStr.Prepend("!");
            }

            data.push_back(wxVariant(dateStr));
            data.push_back(wxVariant(trx.find(TransNumber) != trx.end() ? trx.at(TransNumber) : ""));
            const wxString type = (trx.find(TrxType) != trx.end() ? trx.at(TrxType) : "");
            if (type == Model_Checking::TYPE_NAME_TRANSFER)
                data.push_back(wxVariant(trx.find(ToAccountName) != trx.end() ? trx.at(ToAccountName) : ""));
            else
                data.push_back(wxVariant(trx.find(Payee) != trx.end() ? trx.at(Payee) : ""));
            data.push_back(wxVariant(trx.find(TrxType) != trx.end() ? trx.at(TrxType) : ""));

            wxString category;
            wxString tags;
            if (trx.find(CategorySplit) != trx.end()) {
                wxStringTokenizer tokenizer = wxStringTokenizer(trx.at(CategorySplit), "\n");
                while (tokenizer.HasMoreTokens())
                {
                    wxString token = tokenizer.GetNextToken();
                    wxString splitTags;
                    category += (category.IsEmpty() ? "" : "|") + token.BeforeFirst('/', &splitTags);
                    if (!splitTags.IsEmpty())
                        tags += (tags.IsEmpty() ? "" : "|") + splitTags;
                }
                category.Prepend("*");
            }
            else
            {
                category = (trx.find(Category) != trx.end() ? trx.at(Category).BeforeFirst('/') : "");
            }
            wxString txnTags = trx.find(Category) != trx.end() ? trx.at(Category).AfterFirst('/') : "";
            if (!txnTags.IsEmpty())
                tags.Prepend(tags.IsEmpty() ? "" : "|").Prepend(txnTags);
            data.push_back(wxVariant(category));
            data.push_back(wxVariant(tags));
            data.push_back(wxVariant(trx.find(Amount) != trx.end() ? trx.at(Amount) : ""));
            data.push_back(wxVariant(trx.find(Memo) != trx.end() ? trx.at(Memo) : ""));

            dataListBox_->AppendItem(data, static_cast<wxUIntPtr>(num++));
        }
    }

    if (tabs & ACC_TAB)
    {
        num = 0;
        accListBox_->DeleteAllItems();
        for (const auto& acc : m_QIFaccounts)
        {
            wxVector<wxVariant> data;
            const auto &a = acc.second;

            wxString currencySymbol = a.find(Description) == a.end() ? "" : a.at(Description);
            currencySymbol = currencySymbol.SubString(1, currencySymbol.length() - 2);

            Model_Account::Data* account = (accountNumberCheckBox_->IsChecked())
                ? Model_Account::instance().getByAccNum(acc.first)
                : Model_Account::instance().get(acc.first);

            wxString status;
            const wxString type = acc.second.find(AccountType) != acc.second.end()
                ? acc.second.at(AccountType) : "";

            if (account)
            {
                Model_Currency::Data *curr = Model_Currency::instance().get(account->CURRENCYID);
                if (curr && curr->CURRENCY_SYMBOL == currencySymbol)
                    status = _t("OK");
                else
                    status = _t("Warning");
                if (account->ACCOUNTTYPE != mmExportTransaction::mm_acc_type(type))
                {
                    status = _t("Warning");
                }
                data.push_back(wxVariant(account->ACCOUNTNAME));
            }
            else {
                status = _t("Missing");
                data.push_back(wxVariant(acc.first));
            }

            data.push_back(wxVariant(type));
            data.push_back(wxVariant(currencySymbol));
            data.push_back(wxVariant(status));
            accListBox_->AppendItem(data, static_cast<wxUIntPtr>(num++));
        }
    }

    if (tabs & PAYEE_TAB)
    {
        validatePayees();

        payeeListBox_->DeleteAllItems();
        for (const auto& payee : m_payee_names)
        {
            wxVector<wxVariant> data;
            data.push_back(wxVariant(payee));
            if (payee == _t("Unknown") || (m_QIFpayeeNames.find(payee) != m_QIFpayeeNames.end() && std::get<0>(m_QIFpayeeNames[payee]) != -1))
            {
                if (std::get<2>(m_QIFpayeeNames[payee]) == wxEmptyString)
                    data.push_back(wxVariant(_t("OK")));
                else
                    data.push_back(wxVariant(wxString::Format(_t("Matched to %1$s by pattern %2$s"),
                        std::get<1>(m_QIFpayeeNames[payee]),
                        std::get<2>(m_QIFpayeeNames[payee])
                    )));
            }
            else
                data.push_back(wxVariant(_t("Missing")));
            payeeListBox_->AppendItem(data, static_cast<wxUIntPtr>(num++));
        }
    }

    if (tabs & CAT_TAB)
    {
        num = 0;
        const auto& c(Model_Category::all_categories());
        categoryListBox_->DeleteAllItems();
        for (const auto& categ : m_QIFcategoryNames)
        {
            wxVector<wxVariant> data;
            data.push_back(wxVariant(categ.first));
            if (c.find(categ.first) == c.end() &&
                !(categ.first.Left(1) == '[' && categ.first.Last() == ']'))
                data.push_back(wxVariant("Missing"));
            else
                data.push_back(wxVariant(_t("OK")));
            categoryListBox_->AppendItem(data, static_cast<wxUIntPtr>(num++));
        }
    }
}

void mmQIFImportDialog::OnShowPayeeDialog(wxMouseEvent&)
{
    wxString payeeName;
    if (payeeListBox_->GetSelectedRow() >= 0) {
        wxVariant value;
        payeeListBox_->GetValue(value, payeeListBox_->GetSelectedRow(), 0);
        payeeName = (std::get<0>(m_QIFpayeeNames[value.GetString()]) != -1) ?
            std::get<1>(m_QIFpayeeNames[value.GetString()]) :
            value.GetString();
    }
    mmPayeeDialog dlg(this, false, "mmPayeeDialog", payeeName);
    dlg.ShowModal();
    if (dlg.getRefreshRequested())
    {
        payeeRegExInitialized_ = false;
        refreshTabs(PAYEE_TAB);
    }
}

void mmQIFImportDialog::OnShowCategDialog(wxMouseEvent&)
{
    int64 id = -1;
    if (categoryListBox_->GetSelectedRow() >= 0)
    {
        wxVariant value;
        categoryListBox_->GetValue(value, categoryListBox_->GetSelectedRow(), 0);
        wxString selectedCategname = value.GetString();
        id = m_QIFcategoryNames[selectedCategname];
        if (id == -1) {
            std::map<wxString, int64 > categories = Model_Category::all_categories();
            for (const auto& category : categories)
            {
                if (category.first.CmpNoCase(selectedCategname) <= 0) id = category.second;
                else break;
            }
        }
    }
    mmCategDialog dlg(this, false, id);
    dlg.ShowModal();
    if (dlg.getRefreshRequested())
    {
        refreshTabs(CAT_TAB);
    }
}

void mmQIFImportDialog::OnFileSearch(wxCommandEvent& WXUNUSED(event))
{
    m_FileNameStr = file_name_ctrl_->GetValue();

    m_FileNameStr = wxFileSelector(_t("Choose QIF data file to Import")
        , wxEmptyString, m_FileNameStr, wxEmptyString
        , _t("QIF Files (*.qif)") + "|*.qif;*.QIF"
        , wxFD_OPEN | wxFD_CHANGE_DIR | wxFD_FILE_MUST_EXIST, this); //TODO: Remove UI Blinking

    if (m_FileNameStr.IsEmpty()) {
        m_FileNameStr = file_name_ctrl_->GetValue();
    }
    else
    {
        correctEmptyFileExt("qif", m_FileNameStr);

        log_field_->ChangeValue("");
        file_name_ctrl_->ChangeValue(m_FileNameStr);
        mmReadQIFFile();
    }
}

void mmQIFImportDialog::OnDateMaskChange(wxCommandEvent& WXUNUSED(event))
{
    wxStringClientData* data = static_cast<wxStringClientData*>(choiceDateFormat_->GetClientObject(choiceDateFormat_->GetSelection()));
    if (data && m_dateFormatStr != data->GetData()) {
        m_dateFormatStr = data->GetData();
        m_userDefinedDateMask = true;
        wxString sMsg = _t("Date Format: ") + "\n" + choiceDateFormat_->GetValue();
        *log_field_ << sMsg << "\n";
    }
    refreshTabs(TRX_TAB);
}

void mmQIFImportDialog::OnCheckboxClick(wxCommandEvent& event)
{
    int t = TRX_TAB;

    switch (event.GetId())
    {
    case wxID_PASTE:
        mmColorBtn_->Enable(colorCheckBox_->IsChecked());
        return;
    case wxID_FILE8:
    case wxID_FILE9:
        fromDateCtrl_->Enable(dateFromCheckBox_->IsChecked());
        toDateCtrl_->Enable(dateToCheckBox_->IsChecked());
        return;
    case wxID_FILE3:
        dupTransMethod_->Enable(dupTransCheckBox_->IsChecked());
        dupTransAction_->Enable(dupTransCheckBox_->IsChecked());
        return;
    case wxID_FILE6:
        t = t | PAYEE_TAB;
        break;
    case wxID_FILE7:
        t = t | PAYEE_TAB;
        payeeIsNotes_ = payeeIsNotesCheckBox_->IsChecked();
        if (!m_FileNameStr.IsEmpty())
            mmReadQIFFile(); //TODO: 1:Why read file again? 2:In future may be def payee in settings
    case mmID_PAYEE:
        payeeMatchAddNotes_->Enable(payeeMatchCheckBox_->IsChecked());
        payeeMatchAddNotes_->SetValue(false);
        t = t | PAYEE_TAB;
    case wxID_FILE5:
    {
        t = t | ACC_TAB;
        if (accountCheckBox_->IsChecked()
            && !Model_Account::instance().all_checking_account_names().empty())
        {
            accountDropDown_->Enable(true);
            m_accountNameStr = "";
            auto sel = accountDropDown_->GetSelection();
            if (sel == wxNOT_FOUND) {
                accountDropDown_->SetSelection(0);
                sel = 0;
            }
            wxStringClientData* data_obj = static_cast<wxStringClientData*>(accountDropDown_->GetClientObject(sel));
            if (data_obj)
                m_accountNameStr = data_obj->GetData();

        }
        else {
            accountDropDown_->Enable(false);
            accountCheckBox_->SetValue(false);
            m_accountNameStr = "";
        }
    }
    }

    refreshTabs(t);
}

void mmQIFImportDialog::compilePayeeRegEx() {

    // pre-compile all payee match strings if not already done
    if (payeeMatchCheckBox_->IsChecked() && !payeeRegExInitialized_)
    {
        // only look at payees that have a match pattern set
        Model_Payee::Data_Set payees = Model_Payee::instance().find(Model_Payee::PATTERN(wxEmptyString, NOT_EQUAL));
        for (const auto& payee : payees)
        {
            Document json_doc;
            if (json_doc.Parse(payee.PATTERN.utf8_str()).HasParseError()) {
                continue;
            }
            int key = -1;
            // loop over all keys in the pattern json data
            for (const auto& member : json_doc.GetObject())
            {
                key++;
                const auto pattern = wxString::FromUTF8(member.value.GetString());
                // add the pattern string (for non-regex match, match notes, and the payee tab preview)
                payeeMatchPatterns_[std::make_pair(payee.PAYEEID, payee.PAYEENAME)][key].first = pattern;
                // complie the regex if necessary
                if (pattern.StartsWith("regex:")) {
                    payeeMatchPatterns_[std::make_pair(payee.PAYEEID, payee.PAYEENAME)][key].second
                        .Compile(pattern.Right(pattern.length() - 6), wxRE_ICASE | wxRE_EXTENDED);
                }
            }
        }
        payeeRegExInitialized_ = true;
    }

}
void mmQIFImportDialog::validatePayees() {
    if (!payeeRegExInitialized_) compilePayeeRegEx();

    for (const auto& payee_name : m_payee_names) {
        bool payee_found = false;
        // initialize
        m_QIFpayeeNames[payee_name] = std::make_tuple(-1, "", "");
        // perform pattern match
        if (payeeMatchCheckBox_->IsChecked()) {
            // loop over all the precompiled patterns
            for (auto& payeeId : payeeMatchPatterns_) {
                for (auto& pattern : payeeId.second) {
                    bool match = false;
                    // match against regex if the pattern begins with "regex:"
                    if (pattern.second.first.StartsWith("regex:"))
                        match = pattern.second.second.Matches(payee_name);
                    else // use the normal wxString match for non-regex patterns
                        match = payee_name.Lower().Matches(pattern.second.first.Lower());
                    if (match)
                    {
                        payee_found = true;
                        // save the target payee ID, name, and match details
                        m_QIFpayeeNames[payee_name] = std::make_tuple(payeeId.first.first, payeeId.first.second, pattern.second.first);
                        break;
                    }
                }
                if (payee_found) break;
            }
        }
        if (!payee_found) {
            Model_Payee::Data* payee = Model_Payee::instance().get(payee_name);
            if (payee) {
                m_QIFpayeeNames[payee_name] = std::make_tuple(payee->PAYEEID, payee->PAYEENAME, "");
            }
        }
    }
}

void mmQIFImportDialog::OnAccountChanged(wxCommandEvent& event)
{
    wxStringClientData* client_obj = static_cast<wxStringClientData*>(event.GetClientObject());
    if (client_obj) {
        m_accountNameStr = client_obj->GetData();
        refreshTabs(TRX_TAB);
    }
}

void mmQIFImportDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    if (m_QIFaccounts.empty() && m_accountNameStr.empty() && !accountCheckBox_->IsChecked()) {
        return mmErrorDialogs::InvalidAccount(accountDropDown_);
    }

    wxString sMsg;
    wxMessageDialog msgDlg(this, _t("Do you want to import all transaction?")
        + "\n" + _t("All missing account, payees and categories will be created.")
        , _t("Confirm Import")
        , wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
    if (msgDlg.ShowModal() == wxID_YES)
    {
        getOrCreateAccounts();
        int nTransactions = vQIF_trxs_.size();
        wxProgressDialog progressDlg(_t("Please wait"), _t("Importing")
            , nTransactions + 1, this, wxPD_APP_MODAL | wxPD_CAN_ABORT | wxPD_AUTO_HIDE);
        progressDlg.Update(1, _t("Importing Accounts"));
        bool is_webbapp_enabled = mmWebApp::MMEX_WebApp_UpdateAccount();

        progressDlg.Update(1, _t("Importing Payees"));
        getOrCreatePayees();
        if (is_webbapp_enabled)
            is_webbapp_enabled = mmWebApp::MMEX_WebApp_UpdatePayee();
        progressDlg.Update(1, _t("Importing Categories"));
        getOrCreateCategories();
        if (is_webbapp_enabled)
            mmWebApp::MMEX_WebApp_UpdateCategory();

        Model_Checking::Cache trx_data_set;
        Model_Checking::Cache transfer_to_data_set;
        Model_Checking::Cache transfer_from_data_set;
        int count = 0;
        const wxString& transferStr = Model_Checking::TYPE_NAME_TRANSFER;

        const auto begin_date = toDateCtrl_->GetValue().FormatISODate();
        const auto end_date = fromDateCtrl_->GetValue().FormatISODate();

        wxCommandEvent evt;
        OnDecimalChange(evt);

        for (const auto& entry : vQIF_trxs_)
        {
            if (count % 100 == 0 || count == nTransactions)
            {
                if (!progressDlg.Update(count
                    , wxString::Format(_t("Importing transaction %1$i of %2$i"), count, nTransactions))) // if cancel clicked
                    break; // abort processing
            }
            //
            Model_Checking::Data *trx = Model_Checking::instance().create();
            wxString msg;
            if (completeTransaction(entry, trx, msg))
            {
                wxString strDate = Model_Checking::TRANSDATE(trx).FormatISODate();
                if (dateFromCheckBox_->IsChecked() && strDate < begin_date)
                    continue;
                if (dateToCheckBox_->IsChecked() && strDate > end_date)
                    continue;
                
                Model_Account::Data* account = Model_Account::instance().get(trx->ACCOUNTID);
                Model_Account::Data* toAccount = Model_Account::instance().get(trx->TOACCOUNTID);

                if ((trx->TRANSDATE < account->STATEMENTDATE && account->STATEMENTLOCKED.GetValue()) ||
                    (toAccount && (trx->TRANSDATE < toAccount->STATEMENTDATE && toAccount->STATEMENTLOCKED.GetValue())))
                    continue;

                if (trx->TRANSDATE < account->INITIALDATE) {
                    account->INITIALDATE = trx->TRANSDATE;
                }
                if (toAccount && (trx->TRANSDATE < toAccount->INITIALDATE)) {
                    toAccount->INITIALDATE = trx->TRANSDATE;
                }

                // Save Transaction Tags
                wxString tagStr = (entry.find(Category) != entry.end() ? entry.at(Category).AfterFirst('/') : "");
                Model_Taglink::Cache taglinks;
                if (!tagStr.IsEmpty())
                {
                    wxString reftype = Model_Attachment::REFTYPE_NAME_TRANSACTION;
                    wxStringTokenizer tagTokens = wxStringTokenizer(tagStr, ":");
                    while (tagTokens.HasMoreTokens())
                    {
                        wxString tagname = tagTokens.GetNextToken().Trim(false).Trim();
                        // make tag names single-word
                        tagname.Replace(" ", "_");
                        Model_Tag::Data* tag = Model_Tag::instance().get(tagname);
                        if (!tag)
                        {
                            tag = Model_Tag::instance().create();
                            tag->TAGNAME = tagname;
                            tag->ACTIVE = 1;
                            tag->TAGID = Model_Tag::instance().save(tag);
                        }
                        Model_Taglink::Data* taglink = Model_Taglink::instance().create();
                        taglink->REFTYPE = reftype;
                        taglink->TAGID = tag->TAGID;
                        // Just cache the new taglink since we don't know the TRANSID yet
                        taglinks.push_back(taglink);
                    }
                }

                // transactions are split into three groups, then merged
                // since txnIds are not yet created, we need to keep track of what tags go with each cached transaction.
                if (trx->TRANSCODE == transferStr && trx->TOTRANSAMOUNT > 0.0)
                {
                    // The "From" tags are stored with key <2, index of from transaction>
                    m_txnTaglinks[std::make_pair(2, transfer_from_data_set.size())] = taglinks;
                    transfer_from_data_set.push_back(trx);
                }
                else if (trx->TRANSCODE == transferStr && trx->TOTRANSAMOUNT <= 0.0)
                {
                    // The "To" tags are stored with key <1, index of 'to' transaction>
                    m_txnTaglinks[std::make_pair(1, transfer_to_data_set.size())] = taglinks;
                    transfer_to_data_set.push_back(trx);
                }
                else
                {
                    // The non-transfer tags are stored with key <0, index of transaction>
                    m_txnTaglinks[std::make_pair(0, trx_data_set.size())] = taglinks;
                    trx_data_set.push_back(trx);
                }
            }
            else
            {
                *log_field_ << wxString::Format(_t("Error: %s"), msg);

                wxString t = "";
                for (const auto&i : entry)
                    t << i.second << "|";
                t.RemoveLast(1);
                *log_field_ << wxString::Format("\n( %s )\n", t);
            }
            ++count;
        }

        progressDlg.Update(count, _t("Importing Transfers"));
        mergeTransferPair(transfer_to_data_set, transfer_from_data_set);
        appendTransfers(trx_data_set, transfer_to_data_set);

        //Search for duplicates for transfers
        for (auto &trx : trx_data_set)
        {
            if (!Model_Checking::is_transfer(trx))
                continue;
            wxDate dt;
            dt.ParseISODate(trx->TRANSDATE);
            const auto data = Model_Checking::instance().find(
                Model_Checking::TRANSDATE(dt)
                , Model_Checking::ACCOUNTID(trx->ACCOUNTID)
                , Model_Checking::TOACCOUNTID(trx->TOACCOUNTID)
                , Model_Checking::NOTES(trx->NOTES)
                , Model_Checking::TRANSACTIONNUMBER(trx->TRANSACTIONNUMBER)
                , Model_Checking::TRANSCODE(Model_Checking::TYPE_ID_TRANSFER)
                , Model_Checking::TRANSAMOUNT(trx->TRANSAMOUNT)
            );
            if (data.size() > 0)
                trx->STATUS = Model_Checking::STATUS_KEY_DUPLICATE;
        }
        // At this point all transactions and tags have been merged into single sets
        Model_Taglink::instance().Savepoint();
        for (int i = 0; i < static_cast<int>(trx_data_set.size()); i++)
        {
            if (!m_txnTaglinks[std::make_pair(0, i)].empty())
            {
                // we need to know the transid for the taglink, so save the transaction first
                int64 transid = Model_Checking::instance().save(trx_data_set[i]);
                // apply that transid to all associated tags
                for (const auto& taglink : m_txnTaglinks[std::make_pair(0, i)])
                    taglink->REFID = transid;
                // save the block of taglinks
                Model_Taglink::instance().save(m_txnTaglinks[std::make_pair(0, i)]);
            }
        }
        Model_Taglink::instance().ReleaseSavepoint();
        Model_Checking::instance().save(trx_data_set);
        progressDlg.Update(count, _t("Importing Split transactions"));
        joinSplit(trx_data_set, m_splitDataSets);
        saveSplit();

        sMsg = _t("Import finished successfully") + "\n" +
            wxString::Format(_t("Total Imported: %zu"), trx_data_set.size()) + "\n" +
            wxString::Format(_t("Duplicates Detected: %zu"), m_duplicateTransactions.size());

        trx_data_set.clear();
        vQIF_trxs_.clear();
        btnOK_->Enable(false);
        progressDlg.Destroy();

        save_file_name();
    }
    else
    {
        sMsg = _t("Imported transactions discarded by user!"); //TODO: strange message may be _t("Import has discarded by user!")
    }
    wxMessageDialog(this, sMsg, _t("Import from QIF file"), wxOK | wxICON_WARNING).ShowModal();
    *log_field_ << sMsg << "\n";

    refreshTabs(ACC_TAB | PAYEE_TAB | CAT_TAB);
}

void mmQIFImportDialog::saveSplit()
{
    if (m_splitDataSets.empty()) return;

    Model_Splittransaction::instance().Savepoint();
    Model_Taglink::instance().Savepoint();
    // Work through each group of splits 
    for (int i = 0; i < static_cast<int>(m_splitDataSets.size()); i++)
    {
        // and each split in the group
        for (int j = 0; j < static_cast<int>(m_splitDataSets[i].size()); j++)
        {
            // save the split
            int64 splitTransID = Model_Splittransaction::instance().save(m_splitDataSets[i][j]);
            // check if there are any taglinks for this split index in this group
            if (!m_splitTaglinks[i][j].empty())
            {
                // apply the SPLITTRANSID as the REFID for all the cached taglinks
                for (const auto& taglink : m_splitTaglinks[i][j])
                    taglink->REFID = splitTransID;
                // save cached taglinks
                Model_Taglink::instance().save(m_splitTaglinks[i][j]);
            }
        }
    }
    Model_Splittransaction::instance().ReleaseSavepoint();
    Model_Taglink::instance().ReleaseSavepoint();
}
void mmQIFImportDialog::joinSplit(Model_Checking::Cache &destination
    , std::vector<Model_Splittransaction::Cache> &target)
{
    if (target.empty()) // no splits in the file
        return;

    for (auto &item : destination)
    {
        if (item->CATEGID > 0) continue;
        for (auto &split_item : target.at(-1 * item->CATEGID.GetValue()))
            split_item->TRANSID = item->TRANSID;
        item->CATEGID = -1;
    }
}

void mmQIFImportDialog::appendTransfers(Model_Checking::Cache &destination, Model_Checking::Cache &target)
{
    // Here we are moving all the 'to' transfers into the normal transactions, so we also
    // need to keep track of the new index for the taglinks
    for (int i = 0; i < static_cast<int>(target.size()); i++)
    {
        m_txnTaglinks[std::make_pair(0, destination.size())] = m_txnTaglinks[std::make_pair(1, i)];
        destination.push_back(target[i]);
    }
}

bool mmQIFImportDialog::mergeTransferPair(Model_Checking::Cache& to, Model_Checking::Cache& from)
{
    if (to.empty() && from.empty()) return false; //Nothing to merge

    for (auto& refTrxTo : to)
    {
        int i = -1;
        bool pair_found = false;
        for (auto& refTrxFrom : from)
        {
            ++i;
            if (refTrxTo->ACCOUNTID != refTrxFrom->TOACCOUNTID) continue;
            if (refTrxTo->TOACCOUNTID != refTrxFrom->ACCOUNTID) continue;
            if (refTrxTo->TRANSACTIONNUMBER != refTrxFrom->TRANSACTIONNUMBER) continue;
            if (refTrxTo->NOTES != refTrxFrom->NOTES) continue;
            if (refTrxTo->TRANSDATE != refTrxFrom->TRANSDATE) continue;
            refTrxTo->TOTRANSAMOUNT = refTrxFrom->TRANSAMOUNT;
            from.erase(from.begin() + i);
            // a match is found so erase the 'from' taglinks
            m_txnTaglinks.erase(std::make_pair(2, i));
            pair_found = true;
            break;
        }

        if (!pair_found)
            refTrxTo->TOTRANSAMOUNT = refTrxTo->TRANSAMOUNT;
    }

    // now merge 'from' and 'to' transaction lists
    for (int i = 0; i < static_cast<int>(from.size()); i++)
    {
        std::swap(from[i]->ACCOUNTID, from[i]->TOACCOUNTID);
        // also need to move the 'from' taglinks to the 'to' taglinks list, keeping track
        // of the new transaction indices
        m_txnTaglinks[std::make_pair(1, to.size())] = m_txnTaglinks[std::make_pair(2, i)];
        to.push_back(from[i]);
    }

    return true;
}

bool mmQIFImportDialog::completeTransaction(/*in*/ const std::unordered_map <int, wxString> &i
    , /*out*/ Model_Checking::Data* trx, wxString& msg)
{
    auto t = i;
    trx->TRANSCODE = (t.find(TrxType) != t.end() ? t[TrxType] : "");
    if (trx->TRANSCODE.empty())
    {
        msg = _t("Transaction code is missing");
        return false;
    }
    bool transfer = Model_Checking::is_transfer(trx->TRANSCODE);

    if (!transfer)
    {
        wxString payee_name = t.find(Payee) != t.end() ? t.at(Payee) : "";
        if (!payee_name.empty())
        {
            if (m_QIFpayeeNames.find(payee_name) != m_QIFpayeeNames.end()) {
                trx->PAYEEID = std::get<0>(m_QIFpayeeNames[payee_name]);
                // NOTES haven't been filled yet, so we can just direct assign match details if necessary
                if (payeeMatchAddNotes_->IsChecked() && !std::get<2>(m_QIFpayeeNames[payee_name]).IsEmpty()) {
                    trx->NOTES =  wxString::Format(_t("%1$s matched by %2$s"), payee_name, std::get<2>(m_QIFpayeeNames[payee_name]));
                }
            } else trx->PAYEEID = -1;
        }
        else
        {
            trx->PAYEEID = -1;
        }
    }

    if (trx->PAYEEID == -1 && !transfer)
    {
        msg = _t("Transaction Payee is missing or incorrect");
        return false;
    }

    wxString dateStr = (t.find(Date) != t.end() ? t[Date] : "");
    if (!m_dateFormatStr.Contains(" ")) dateStr.Replace(" ", "");
    wxDateTime dtdt;
    wxString::const_iterator end;
    if (dtdt.ParseFormat(dateStr, m_dateFormatStr, &end))
        trx->TRANSDATE = dtdt.FormatISOCombined();
    else
    {
        *log_field_ << _t("Date format or date mask is incorrect") << "\n";
        return false;
    }

    int64 accountID = -1;
    wxString accountName = (t.find(AccountName) != t.end() ? t[AccountName] : "");
    if ((accountName.empty() || accountCheckBox_->IsChecked()) /*&& !transfer*/) {
        accountName = m_accountNameStr;
    }
    accountID = (m_QIFaccountsID.find(accountName) != m_QIFaccountsID.end() ? m_QIFaccountsID.at(accountName) : -1);
    if (accountID < 1)
    {
        msg = _t("Transaction Account is incorrect");
        return false;
    }
    trx->ACCOUNTID = accountID;
    trx->TOACCOUNTID = (t.find(ToAccountName) != t.end()
        ? (m_QIFaccountsID.find(t[ToAccountName]) != m_QIFaccountsID.end()
            ? m_QIFaccountsID[t[ToAccountName]] : -1) : -1);
    if (trx->ACCOUNTID == trx->TOACCOUNTID && transfer)
    {
        msg = _t("Transaction Account for transfer is incorrect");
        return false;
    }

    trx->TRANSACTIONNUMBER = (t.find(TransNumber) != t.end() ? t[TransNumber] : "");
    trx->NOTES.Prepend(!trx->NOTES.IsEmpty() ? "\n" : "").Prepend(t.find(Memo) != t.end() ? t[Memo] : ""); // add the actual NOTES before the payee match details
    wxString status = Model_Checking::STATUS_KEY_NONE;
    if (t.find(Status) != t.end())
    {
        wxString s = t[Status];
        if (s == "X" || s == "R")
            status = Model_Checking::STATUS_KEY_RECONCILED;
        /*else if (s == "*" || s == "c")
        {
            TODO: What does 'cleared' status mean?
            status = "c";
        }*/

    }
    trx->STATUS = status;

    int color_id = mmColorBtn_->GetColorId();
    trx->COLOR = -1;
    if (colorCheckBox_->IsChecked() && color_id > 0 && color_id < 8)
        trx->COLOR = color_id;

    const wxString value = mmTrimAmount(t.find(Amount) != t.end() ? t[Amount] : "", decimal_, ".");
    if (value.empty())
    {
        msg = _t("Transaction Amount is incorrect");
        return false;
    }

    double amt;
    value.ToCDouble(&amt);

    trx->TRANSAMOUNT = fabs(amt);
    trx->TOTRANSAMOUNT = transfer ? amt : trx->TRANSAMOUNT;
    wxString tagStr;
    wxRegEx regex(" ?: ?");
    if (t.find(CategorySplit) != t.end())
    {
        Model_Splittransaction::Cache split;       
        wxStringTokenizer categToken(t[CategorySplit], "\n");
        wxStringTokenizer amtToken((t.find(AmountSplit) != t.end() ? t[AmountSplit] : ""), "\n");
        wxString notes = t.find(MemoSplit) != t.end() ? t[MemoSplit] : "";
        int split_id = 1;

        while (categToken.HasMoreTokens())
        {
            const wxString c = categToken.GetNextToken().BeforeFirst('/', &tagStr);
            if (m_QIFcategoryNames.find(c) == m_QIFcategoryNames.end()) return false;
            int64 categID = m_QIFcategoryNames[c];
            if (categID <= 0)
            {
                msg = _t("Transaction Category is incorrect");
                return false;
            }
            Model_Splittransaction::Data* s = Model_Splittransaction::instance().create();
            s->CATEGID = categID;

            wxString amtSplit = amtToken.GetNextToken();
            amtSplit = mmTrimAmount(amtSplit, decimal_, ".");
            double amount;
            amtSplit.ToCDouble(&amount);

            wxString memo;
            while (!notes.empty()) {
                wxRegEx pattern(wxString::Format("^%d:(.*)", split_id), wxRE_NEWLINE);
                if (pattern.Matches(notes))
                {
                    memo += (!memo.IsEmpty() ? "\n" : "" ) + pattern.GetMatch(notes, 1);
                    pattern.ReplaceFirst(&notes, "");
                    notes.Replace("\n", "", false);
                }
                else
                    break;
            }

            s->SPLITTRANSAMOUNT = (Model_Checking::is_deposit(trx) ? amount : -amount);
            s->TRANSID = trx->TRANSID;
            s->NOTES = memo;
            split.push_back(s);
            // Save split tags
            if (!tagStr.IsEmpty())
            {
                Model_Taglink::Cache splitTaglinks;
                wxString reftype = Model_Attachment::REFTYPE_NAME_TRANSACTIONSPLIT;
                wxStringTokenizer tagTokens = wxStringTokenizer(tagStr, ":");
                while (tagTokens.HasMoreTokens())
                {
                    wxString tagname = tagTokens.GetNextToken().Trim(false).Trim();
                    // make tag names single-word
                    tagname.Replace(" ", "_");
                    Model_Tag::Data* tag = Model_Tag::instance().get(tagname);
                    if (!tag)
                    {
                        tag = Model_Tag::instance().create();
                        tag->TAGNAME = tagname;
                        tag->ACTIVE = 1;
                        tag->TAGID = Model_Tag::instance().save(tag);
                    }
                    Model_Taglink::Data* taglink = Model_Taglink::instance().create();
                    taglink->REFTYPE = reftype;
                    taglink->TAGID = tag->TAGID;
                    splitTaglinks.push_back(taglink);
                }
                // Here we keep track of which block of splits and which split in the block
                // each group of taglinks is associated with. Once we save the splits we can
                // record the SPLITTRANSID on the taglink
                m_splitTaglinks[m_splitDataSets.size()][split_id - 1] = splitTaglinks;
            }
            split_id++;
        }
        trx->CATEGID = -1 * static_cast<int>(m_splitDataSets.size());
        m_splitDataSets.push_back(split);
    }
    else
    {
        wxString categStr = (t.find(Category) != t.end() ? t.at(Category).BeforeFirst('/') : "");
        if (categStr.empty())
        {
            Model_Payee::Data* payee = Model_Payee::instance().get(trx->PAYEEID);
            if (payee)
            {
                trx->CATEGID = payee->CATEGID;
            }
            categStr = Model_Category::full_name(trx->CATEGID, ":");

            if (categStr.empty())
            {
                trx->CATEGID = (m_QIFcategoryNames[_t("Unknown")]);
            }
        }
        else
        {
            trx->CATEGID = (m_QIFcategoryNames[categStr]);
        }

    }

    // Check for duplicates according to user choice
    if (dupTransCheckBox_->IsChecked())
    {
        bool isDuplicate = false;
        int dupMethod = dupTransMethod_->GetSelection();
        int dupAction = dupTransAction_->GetSelection();

        if (dupMethod == 0) // By transaction number
        {
            if (!trx->TRANSACTIONNUMBER.empty())
            {
                const auto existing_transactions = Model_Checking::instance().find(
                    Model_Checking::TRANSACTIONNUMBER(trx->TRANSACTIONNUMBER),
                    Model_Checking::DELETEDTIME(wxEmptyString, EQUAL));

                isDuplicate = !existing_transactions.empty();
            }
        }
        else if (dupMethod == 1 || dupMethod == 2) // By amount and date (exact or nearby)
        {
            wxDateTime startDate, endDate;
            wxString trxDateStr = trx->TRANSDATE;

            if (dupMethod == 1) // exact date
            {
                startDate = endDate = wxDateTime();
                startDate.ParseISODate(trxDateStr);
                endDate = startDate;
            }
            else // nearby date
            {
                wxDateTime trxDate;
                trxDate.ParseISODate(trxDateStr);
                startDate = trxDate;
                endDate = trxDate;
                startDate.Subtract(wxDateSpan::Days(4));
                endDate.Add(wxDateSpan::Days(2));
            }

            wxString startDateStr = startDate.FormatISODate() + "T00:00:00";
            wxString endDateStr = mmDateRange::getDayEnd(endDate).FormatISOCombined();

            const auto potential_matches = Model_Checking::instance().find(
                Model_Checking::TRANSAMOUNT(trx->TRANSAMOUNT),
                Model_Checking::TRANSDATE(startDateStr, GREATER_OR_EQUAL),
                Model_Checking::TRANSDATE(endDateStr, LESS_OR_EQUAL),
                Model_Checking::DELETEDTIME(wxEmptyString, EQUAL));

            for (const auto& existingTrx : potential_matches)
            {
                bool alreadyMatched = m_duplicateTransactions.find(existingTrx.TRANSID) != m_duplicateTransactions.end();
                if (!alreadyMatched)
                {
                    isDuplicate = true;
                    m_duplicateTransactions.insert(existingTrx.TRANSID);
                    break;
                }
            }
        }

        if (isDuplicate)
        {
            if (dupAction == 0) // Skip
            {
                msg = _t("Transaction skipped as duplicate");
                return false;
            }
            else if (dupAction == 1) // Flag as duplicate
                trx->STATUS = Model_Checking::STATUS_KEY_DUPLICATE;
        }
    }

    return true;
}

void mmQIFImportDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_CANCEL);
}

void mmQIFImportDialog::OnQuit(wxCloseEvent& WXUNUSED(event))
{
    EndModal(wxID_CANCEL);
}

int64 mmQIFImportDialog::getOrCreateAccounts()
{
    m_QIFaccountsID.clear();

    for (auto &item : m_QIFaccounts)
    {
        int64 accountID = -1;
        Model_Account::Data* acc = (accountNumberCheckBox_->IsChecked())
            ? Model_Account::instance().getByAccNum(item.first)
            : Model_Account::instance().get(item.first);

        if (!acc)
        {
            Model_Account::Data *account = Model_Account::instance().create();

            account->FAVORITEACCT = "TRUE";
            account->STATUS = Model_Account::STATUS_NAME_OPEN;

            const auto type = item.second.find(AccountType) != item.second.end() ? item.second.at(AccountType) : "";
            account->ACCOUNTTYPE = mmExportTransaction::mm_acc_type(type);
            //Model_Account::TYPE_NAME_CHECKING;
            account->ACCOUNTNAME = item.first;
            account->INITIALBAL = 0;
            account->INITIALDATE = wxDate::Today().FormatISODate();

            account->CURRENCYID = Model_Currency::GetBaseCurrency()->CURRENCYID;
            const wxString c = (item.second.find(Description) == item.second.end() ? "" : item.second.at(Description));
            for (const auto& curr : Model_Currency::instance().all())
            {
                if (wxString::Format("[%s]", curr.CURRENCY_SYMBOL) == c) {
                    account->CURRENCYID = curr.CURRENCYID;
                    break;
                }
            }

            accountID = Model_Account::instance().save(account);
            wxString sMsg = wxString::Format(_t("Added account: %s"), item.first);
            *log_field_ << sMsg << "\n";
        }
        else
            accountID = acc->ACCOUNTID;

        m_QIFaccountsID[item.first] = accountID;
    }

    Model_Account::Data* acc = Model_Account::instance().get(m_accountNameStr);
    if (acc) {
        m_QIFaccountsID[m_accountNameStr] = acc->ACCOUNTID;
    }

    return m_QIFaccountsID.size();
}

void mmQIFImportDialog::getOrCreatePayees()
{
    Model_Payee::instance().Savepoint();
    
    for (const auto& item : m_payee_names)
    {
        // check if this payee exists
        if (m_QIFpayeeNames.find(item) != m_QIFpayeeNames.end() && std::get<0>(m_QIFpayeeNames[item]) != -1) continue;

        // the payee doesn't exist or match a pattern, so create one
        Model_Payee::Data* p = Model_Payee::instance().create();
        p->PAYEENAME = item;
        p->ACTIVE = 1;
        p->CATEGID = -1;
        wxString sMsg = wxString::Format(_t("Added payee: %s"), item);
        log_field_->AppendText(wxString() << sMsg << "\n");
        m_QIFpayeeNames[item] = std::make_tuple(Model_Payee::instance().save(p), p->PAYEENAME, "");
        
    }

    Model_Payee::instance().ReleaseSavepoint();
}

void mmQIFImportDialog::getOrCreateCategories()
{
    wxArrayString temp;
    for (const auto &item : m_QIFcategoryNames)
    {
        wxString categStr;
        wxStringTokenizer token(item.first, ":");
        int64 parentID = -1;
        while(token.HasMoreTokens()){
            categStr = token.GetNextToken().Trim(false).Trim();
            Model_Category::Data* c = Model_Category::instance().get(categStr, parentID);
            if (temp.Index(categStr + wxString::Format(":%lld", parentID)) == wxNOT_FOUND) {

                if (!c)
                {
                    c = Model_Category::instance().create();
                    c->CATEGNAME = categStr;
                    c->ACTIVE = 1;
                    c->PARENTID = parentID;
                    Model_Category::instance().save(c);
                }
                temp.Add(categStr + wxString::Format(":%lld", parentID));
            }
            parentID = c->CATEGID;

        }
        m_QIFcategoryNames[item.first] = parentID;
    }
}

int64 mmQIFImportDialog::get_last_imported_acc()
{
    int64 accID = -1;
    Model_Account::Data* acc = Model_Account::instance().get(m_accountNameStr);
    if (acc)
        accID = acc->ACCOUNTID;
    return accID;
}

void mmQIFImportDialog::OnDecimalChange(wxCommandEvent& event)
{
    int i = m_choiceDecimalSeparator->GetSelection();
    wxStringClientData* type_obj = static_cast<wxStringClientData*>(m_choiceDecimalSeparator->GetClientObject(i));
    if (type_obj) {
        decimal_ = type_obj->GetData();
    }

    event.Skip();
}

void mmQIFImportDialog::OnFileNameChanged(wxCommandEvent& WXUNUSED(event))
{
    const wxString file_name = file_name_ctrl_->GetValue();

    wxFileName file(file_name);
    if (file.FileExists()) {
        m_FileNameStr = file_name;
        log_field_->ChangeValue("");
        mmReadQIFFile();
    }
}

void mmQIFImportDialog::save_file_name()
{
    wxFileName file(m_FileNameStr);
    if (file.FileExists()) {
        Model_Setting::instance().prependArrayItem("RECENT_QIF_FILES", m_FileNameStr, 10);
    }
}

void mmQIFImportDialog::OnMenuSelected(wxCommandEvent& WXUNUSED(event))
{
    mmColorBtn_->Enable(false);
    colorCheckBox_->SetValue(false);
}
