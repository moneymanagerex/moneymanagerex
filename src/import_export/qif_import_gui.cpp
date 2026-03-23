/*******************************************************
Copyright (C) 2013-2020 Nikolay Akimov
Copyright (C) 2022  Mark Whalley (mark@ipx.co.uk)
Copyright (C) 2026  Klaus Wich

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

#include "base/defs.h"
#include <wx/progdlg.h>
#include <wx/dataview.h>

#include "base/constants.h"
#include "base/paths.h"
#include "util/_util.h"
#include "util/_simple.h"
#include "util/mmDateRange.h"

#include "model/CategoryModel.h"
#include "model/PayeeModel.h"
#include "model/SettingModel.h"
#include "model/TagModel.h"
#include "model/PrefModel.h"

#include "manager/CategoryManager.h"
#include "manager/PayeeManager.h"
#include "webapp.h"
#include "uicontrols/navigatortypes.h"

#include "qif_import_gui.h"
#include "qif_import.h"
#include "export.h"

enum tab_id {
    LOG_TAB = 1,
    TRX_TAB = 2,
    ACC_TAB = 4,
    PAYEE_TAB = 8,
    CAT_TAB = 16,
};

wxIMPLEMENT_DYNAMIC_CLASS(mmQIFImportDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmQIFImportDialog, wxDialog)
    EVT_CHECKBOX(wxID_ANY,  mmQIFImportDialog::OnCheckboxClick)
    EVT_BUTTON(wxID_OK,     mmQIFImportDialog::OnOk)
    EVT_MENU(wxID_HIGHEST,  mmQIFImportDialog::OnMenuSelected)
    EVT_BUTTON(wxID_CANCEL, mmQIFImportDialog::OnCancel)
    EVT_CHOICE(ID_ACCOUNT,  mmQIFImportDialog::OnAccountChanged)
    EVT_CLOSE(              mmQIFImportDialog::OnQuit)
wxEND_EVENT_TABLE()

mmQIFImportDialog::mmQIFImportDialog(
    wxWindow* parent,
    int64 account_id,
    const wxString& file_path
) :
    m_FileNameStr(file_path),
    m_today(wxDateTime::Today()),
    m_fresh(wxDateTime::Today().Subtract(wxDateSpan::Months(1)))
{
    decimal_ = CurrencyModel::instance().get_base_data_n()->m_decimal_point;
    payeeIsNotes_ = false;
    const AccountData* account_n = AccountModel::instance().get_id_data_n(account_id);
    if (account_n)
        m_accountNameStr = account_n->m_name;

    this->SetFont(parent->GetFont());
    Create(parent);
    mmThemeAutoColour(this);
    SetMinSize(wxSize(500, 300));
    SetSize(InfoModel::instance().getSize(DIALOG_SIZE));
}

mmQIFImportDialog::~mmQIFImportDialog()
{
    InfoModel::instance().setSize(DIALOG_SIZE, GetSize());
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

    ColName_[LIST_ID_ID]       = "#";
    ColName_[LIST_ID_ACCOUNT]  = _t("Account");
    ColName_[LIST_ID_DATE]     = _t("Date");
    ColName_[LIST_ID_NUMBER]   = _t("Number");
    ColName_[LIST_ID_PAYEE]    = _t("Payee");
    ColName_[LIST_ID_TYPE]     = _t("Type");
    ColName_[LIST_ID_CATEGORY] = _t("Category");
    ColName_[LIST_ID_TAGS]     = _t("Tags");
    ColName_[LIST_ID_VALUE]    = _t("Value");
    ColName_[LIST_ID_NOTES]    = _t("Notes");

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

    wxArrayString files = SettingModel::instance().getArrayString("RECENT_QIF_FILES");
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
    wxStaticText* itemStaticText88 = new wxStaticText(this, wxID_STATIC, _t("Encoding:"));
    //itemStaticText88->SetFont(staticBoxFontSetting);
    flex_sizer->Add(itemStaticText88, g_flagsH);

    m_choiceEncoding = new wxChoice(this, wxID_ANY);
    for (const auto &i : g_encoding)
        m_choiceEncoding->Append(wxGetTranslation(i.second.second), new wxStringClientData(i.second.second));
    m_choiceEncoding->SetSelection(0);
    m_choiceEncoding->Bind(wxEVT_CHOICE, &mmQIFImportDialog::OnFileNameChanged, this);

    flex_sizer->Add(m_choiceEncoding, g_flagsH);

    //Account
    accountCheckBox_ = new wxCheckBox(this, wxID_FILE5, _t("Account")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    accountDropDown_ = new wxChoice(this, ID_ACCOUNT);
    accountDropDown_->SetMinSize(wxSize(180, -1));
    accountDropDown_->Enable(false);

    for (const auto& a : AccountModel::instance().find_all_name_a()) {
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
    m_dateFormatStr = PrefModel::instance().getDateFormat();

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

    wxStaticText* decamalCharText = new wxStaticText(this, wxID_STATIC, _t("Decimal Char:"));
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
    wxString catDelimiter = InfoModel::instance().getString("CATEG_DELIMITER", ":");

    wxFileInputStream input(m_FileNameStr);
    wxTextInputStream* text;
    if (m_choiceEncoding->GetSelection() == 0) {
        text = new wxTextInputStream(input);
    }
    else {
        wxConvAuto conv = g_encoding.at(m_choiceEncoding->GetSelection()).first;
        text = new wxTextInputStream(input, "\x09", conv);
    }

    wxProgressDialog progressDlg(_tu("Please wait…"), _t("Scanning")
        , 0, this, wxPD_APP_MODAL | wxPD_CAN_ABORT);

    wxLongLong start = wxGetUTCTimeMillis();
    wxLongLong interval = wxGetUTCTimeMillis() - start;

    wxString accName = "";
    if (accountCheckBox_->IsChecked()) {
        accName = accountDropDown_->GetStringSelection();
        const AccountData* acc = AccountModel::instance().get_name_data_n(accName);
        if (acc) {
            m_accountNameStr = acc->m_name;
        }
    }

    std::unordered_map <int, wxString> trx;
    int64 split_id = 0;
    wxSharedPtr<mmDates> dParser(new mmDates);
    std::map<wxString, int> comma({ {".", 0}, {",", 0} });
    while (input.IsOk() && !input.Eof())
    {
        ++numLines;
        const wxString lineStr = text->ReadLine();
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
        if (lineType == QIF_ID_EOTLT || input.Eof()) {
            if (trx.find(QIF_ID_AcctType) != trx.end()) {
                if (trx[QIF_ID_AcctType] == "Account") {
                    accName = (trx.find(QIF_ID_TransNumber) == trx.end() ? "" : trx[QIF_ID_TransNumber]);
                    std::unordered_map <int, wxString> a;
                    a[QIF_ID_AccountType] = (trx.find(QIF_ID_Description) != trx.end() ? trx.at(QIF_ID_Description) : "");
                    a[QIF_ID_Description] = (trx.find(QIF_ID_AccountType) != trx.end() ? trx.at(QIF_ID_AccountType) : "");
                    m_QIFaccounts[accName] = a;
                    m_accountNameStr = accName;
                }
            }

            if (trx[QIF_ID_AcctType] != "Account" && completeTransaction(trx, m_accountNameStr)) {
                vQIF_trxs_.push_back(trx);
            }
            trx.clear();
            split_id = 0;
            continue;
        }

        //Parse Categories
        if (lineType == QIF_ID_CategorySplit || lineType == QIF_ID_Category) {
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
        if (!m_userDefinedDateMask && lineType == QIF_ID_Date && (data.Mid(0, 1) != "[")) {
            dParser->doHandleStatistics(data);
        }

        //Parse numbers
        if (lineType == QIF_ID_Amount || lineType == QIF_ID_AmountSplit)
        {
            comma["."] += data.Contains(".") ? data.find(".") + 1 : 0;
            comma[","] += data.Contains(",") ? data.find(",") + 1 : 0;
        }

        if (lineType == QIF_ID_CategorySplit)
            split_id++;

        if (lineType == QIF_ID_AcctType)
            trx[lineType] = data;
        else
        {
            wxString prefix;
            if (!trx[lineType].empty())
                prefix = "\n";

            if (lineType == QIF_ID_MemoSplit)
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

bool mmQIFImportDialog::completeTransaction(
    std::unordered_map<int, wxString>& trx,
    const wxString &accName
) {
    if (trx.find(QIF_ID_Date) == trx.end())
        return false;

    bool isTransfer = false;

    if (accName.empty()) {
        trx[QIF_ID_AccountName] = m_accountNameStr;
    }
    else {
        trx[QIF_ID_AccountName] = accName;
    }


    if (trx.find(QIF_ID_CategorySplit) != trx.end()) {
        //TODO:Dublicate code
        wxStringTokenizer token(trx[QIF_ID_CategorySplit], "\n");
        while (token.HasMoreTokens()) {
            wxString c = token.GetNextToken();
            const wxString project = qif_api->getFinancistoProject(c);
            if (!project.empty())
                trx[QIF_ID_TransNumber] += project + "\n"; //TODO: trx number or notes
        }
    }

    if (trx[QIF_ID_Payee] == "Opening Balance") {
        trx[QIF_ID_Memo] += (trx[QIF_ID_Memo].empty() ? "" : "\n") + trx[QIF_ID_Payee];
        trx[QIF_ID_Category] = trx[QIF_ID_Payee];
    }

    if (trx.find(QIF_ID_Category) != trx.end()) {
        wxString tags;
        wxString categname = trx[QIF_ID_Category].BeforeFirst('/', &tags);
        if (categname.Left(1) == "[" && categname.Last() == ']') {
            wxString toAccName = categname.SubString(1, categname.length() - 2);

            if (toAccName == m_accountNameStr) {
                trx[QIF_ID_Category] = trx[QIF_ID_Payee];
                trx[QIF_ID_Payee] = toAccName;
            }
            else {
                isTransfer = true;
                trx[QIF_ID_Category] = _t("Transfer") + (!tags.IsEmpty() ? "/" + tags : "");
                trx[QIF_ID_TrxType] = TrxType(TrxType::e_transfer).name();
                trx[QIF_ID_ToAccountName] = toAccName;
                trx[QIF_ID_Memo] += (trx[QIF_ID_Memo].empty() ? "" : "\n") + trx[QIF_ID_Payee];
                if (m_QIFaccounts.find(toAccName) == m_QIFaccounts.end()) {
                    std::unordered_map<int, wxString> a;
                    a[QIF_ID_Description] = "[" + CurrencyModel::instance().get_base_data_n()->m_symbol + "]";
                    a[QIF_ID_AccountType] = (trx.find(QIF_ID_Description) != trx.end() ? trx.at(QIF_ID_Description) : "");
                    m_QIFaccounts[toAccName] = a;
                }
            }
        }
    }

    if (!isTransfer) {
        wxString payee_name = trx.find(QIF_ID_Payee) != trx.end() ? trx[QIF_ID_Payee] : "";
        if (payee_name.empty() && trx[QIF_ID_AcctType] != "Account" ) {
            payee_name = trx.find(QIF_ID_AccountName) != trx.end() ? trx[QIF_ID_AccountName] : _t("Unknown");
            trx[QIF_ID_Payee] = payee_name;
        }

        if (!payee_name.empty()) {
            int i = m_payee_names.Index(payee_name, false);
            if (i == wxNOT_FOUND) {
                m_payee_names.Add(payee_name);
            }
            else
                trx[QIF_ID_Payee] = m_payee_names.Item(i);

            if (payee_name == "Opening Balance")
                m_QIFcategoryNames["Opening Balance"] = -1;

        }
    }

    if (payeeIsNotes_) {
        trx[QIF_ID_Memo] += (trx[QIF_ID_Memo].empty() ? "" : "\n") + trx[QIF_ID_Payee];
    }

    wxString amtStr = (trx.find(QIF_ID_Amount) == trx.end() ? "" : trx[QIF_ID_Amount]);
    if (!isTransfer) {
        if (amtStr.Mid(0, 1) == "-")
            trx[QIF_ID_TrxType] = TrxType(TrxType::e_withdrawal).name();
        else if (!amtStr.empty())
            trx[QIF_ID_TrxType] = TrxType(TrxType::e_deposit).name();
    }

    return !amtStr.empty();
}

void mmQIFImportDialog::refreshTabs(int tabs)
{
    int num = 0;
    if (tabs & TRX_TAB) {
        dataListBox_->DeleteAllItems();
        for (const auto& trx : vQIF_trxs_) {
            wxVector<wxVariant> data;
            const AccountData* account = AccountModel::instance().get_num_data_n(trx.at(QIF_ID_AccountName));
            data.push_back(wxVariant(wxString::Format("%i", num + 1)));
            data.push_back(
                wxVariant(
                    trx.find(QIF_ID_AccountName) != trx.end()
                    && (trx.at(QIF_ID_AccountName).empty() || accountCheckBox_->IsChecked())
                    ? m_accountNameStr
                    : ((accountNumberCheckBox_->IsChecked() && account)
                        ? account->m_name : trx.at(QIF_ID_AccountName)
                        )
                )
            );

            wxString dateStr = "";
            if (trx.find(QIF_ID_Date) != trx.end()) {
                dateStr = trx.at(QIF_ID_Date);
                dateStr.Replace(" ", "");
                wxDateTime dtdt;
                wxString::const_iterator end;
                if (dtdt.ParseFormat(dateStr, m_dateFormatStr, &end))
                    dateStr = mmGetDateTimeForDisplay(dtdt.FormatISODate());
                else
                    dateStr.Prepend("!");
            }

            data.push_back(wxVariant(dateStr));
            data.push_back(wxVariant(trx.find(QIF_ID_TransNumber) != trx.end() ? trx.at(QIF_ID_TransNumber) : ""));
            const wxString type = (trx.find(QIF_ID_TrxType) != trx.end() ? trx.at(QIF_ID_TrxType) : "");
            if (type == TrxType(TrxType::e_transfer).name())
                data.push_back(wxVariant(trx.find(QIF_ID_ToAccountName) != trx.end() ? trx.at(QIF_ID_ToAccountName) : ""));
            else
                data.push_back(wxVariant(trx.find(QIF_ID_Payee) != trx.end() ? trx.at(QIF_ID_Payee) : ""));
            data.push_back(wxVariant(trx.find(QIF_ID_TrxType) != trx.end() ? trx.at(QIF_ID_TrxType) : ""));

            wxString category;
            wxString tags;
            if (trx.find(QIF_ID_CategorySplit) != trx.end()) {
                wxStringTokenizer tokenizer = wxStringTokenizer(trx.at(QIF_ID_CategorySplit), "\n");
                while (tokenizer.HasMoreTokens()) {
                    wxString token = tokenizer.GetNextToken();
                    wxString splitTags;
                    category += (category.IsEmpty() ? "" : "|") + token.BeforeFirst('/', &splitTags);
                    if (!splitTags.IsEmpty())
                        tags += (tags.IsEmpty() ? "" : "|") + splitTags;
                }
                category.Prepend("*");
            }
            else {
                category = (trx.find(QIF_ID_Category) != trx.end() ? trx.at(QIF_ID_Category).BeforeFirst('/') : "");
            }
            wxString txnTags = trx.find(QIF_ID_Category) != trx.end() ? trx.at(QIF_ID_Category).AfterFirst('/') : "";
            if (!txnTags.IsEmpty())
                tags.Prepend(tags.IsEmpty() ? "" : "|").Prepend(txnTags);
            data.push_back(wxVariant(category));
            data.push_back(wxVariant(tags));
            data.push_back(wxVariant(trx.find(QIF_ID_Amount) != trx.end() ? trx.at(QIF_ID_Amount) : ""));
            data.push_back(wxVariant(trx.find(QIF_ID_Memo) != trx.end() ? trx.at(QIF_ID_Memo) : ""));

            dataListBox_->AppendItem(data, static_cast<wxUIntPtr>(num++));
        }
    }

    if (tabs & ACC_TAB) {
        num = 0;
        accListBox_->DeleteAllItems();
        for (const auto& acc : m_QIFaccounts) {
            wxVector<wxVariant> data;
            const auto &a = acc.second;

            wxString currencySymbol = a.find(QIF_ID_Description) == a.end() ? "" : a.at(QIF_ID_Description);
            currencySymbol = currencySymbol.SubString(1, currencySymbol.length() - 2);

            const AccountData* account = (accountNumberCheckBox_->IsChecked())
                ? AccountModel::instance().get_num_data_n(acc.first)
                : AccountModel::instance().get_name_data_n(acc.first);

            wxString status;
            const wxString type = acc.second.find(QIF_ID_AccountType) != acc.second.end()
                ? acc.second.at(QIF_ID_AccountType) : "";

            if (account) {
                const CurrencyData *currency_n = CurrencyModel::instance().get_id_data_n(account->m_currency_id);
                if (currency_n && currency_n->m_symbol == currencySymbol)
                    status = _t("OK");
                else
                    status = _t("Warning");
                if (account->m_type_ != mmExportTransaction::mm_acc_type(type)) {
                    status = _t("Warning");
                }
                data.push_back(wxVariant(account->m_name));
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

    if (tabs & PAYEE_TAB) {
        validatePayees();

        payeeListBox_->DeleteAllItems();
        for (const auto& payee : m_payee_names) {
            wxVector<wxVariant> data;
            data.push_back(wxVariant(payee));
            if (payee == _t("Unknown") || (m_QIFpayeeNames.find(payee) != m_QIFpayeeNames.end() && std::get<0>(m_QIFpayeeNames[payee]) != -1)) {
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

    if (tabs & CAT_TAB) {
        num = 0;
        const auto& cat_fullname_id_m = CategoryModel::instance().find_all_id_mFullname();
        categoryListBox_->DeleteAllItems();
        for (const auto& categ : m_QIFcategoryNames) {
            wxVector<wxVariant> data;
            data.push_back(wxVariant(categ.first));
            if (cat_fullname_id_m.find(categ.first) == cat_fullname_id_m.end() &&
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
            std::map<wxString, int64 > categories = CategoryModel::instance().find_all_id_mFullname();
            for (const auto& category : categories)
            {
                if (category.first.CmpNoCase(selectedCategname) <= 0) id = category.second;
                else break;
            }
        }
    }
    CategoryManager dlg(this, false, id);
    dlg.ShowModal();
    if (dlg.getRefreshRequested())
    {
        refreshTabs(CAT_TAB);
    }
}

void mmQIFImportDialog::OnFileSearch(wxCommandEvent& WXUNUSED(event))
{
    m_FileNameStr = file_name_ctrl_->GetValue();

    m_FileNameStr = wxFileSelector(_t("Choose QIF data file to import")
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
        [[fallthrough]];    // is this intended ??
    case wxID_FILE5:
    case mmID_PAYEE:
        payeeMatchAddNotes_->Enable(payeeMatchCheckBox_->IsChecked());
        payeeMatchAddNotes_->SetValue(false);
        t = t | PAYEE_TAB;
    //case wxID_FILE5:    // duplicated case and fall through => is this really the intention?
    //{
        t = t | ACC_TAB;
        if (accountCheckBox_->IsChecked()
            && !AccountModel::instance().find_all_name_a().empty())
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
    // }
    }

    refreshTabs(t);
}

void mmQIFImportDialog::compilePayeeRegEx() {

    // pre-compile all payee match strings if not already done
    if (!payeeMatchCheckBox_->IsChecked() || payeeRegExInitialized_)
        return;

    // only look at payees that have a match pattern set
    PayeeModel::DataA payee_a = PayeeModel::instance().find(
        PayeeCol::PATTERN(OP_NE, wxEmptyString)
    );
    for (const auto& payee_d : payee_a) {
        Document json_doc;
        if (json_doc.Parse(payee_d.m_pattern.utf8_str()).HasParseError()) {
            continue;
        }
        int key = -1;
        // loop over all keys in the pattern json data
        for (const auto& member : json_doc.GetObject()) {
            key++;
            const auto pattern = wxString::FromUTF8(member.value.GetString());
            // add the pattern string (for non-regex match, match notes, and the payee tab preview)
            payeeMatchPatterns_[std::make_pair(payee_d.m_id, payee_d.m_name)][key].first = pattern;
            // complie the regex if necessary
            if (pattern.StartsWith("regex:")) {
                payeeMatchPatterns_[std::make_pair(payee_d.m_id, payee_d.m_name)][key].second
                    .Compile(pattern.Right(pattern.length() - 6), wxRE_ICASE | wxRE_EXTENDED);
            }
        }
    }
    payeeRegExInitialized_ = true;
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
            const PayeeData* payee_n = PayeeModel::instance().get_name_data_n(payee_name);
            if (payee_n) {
                m_QIFpayeeNames[payee_name] = std::make_tuple(payee_n->m_id, payee_n->m_name, "");
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
    wxMessageDialog msgDlg(this, _t("Do you want to import all transactions?")
        + "\n" + _t("All missing account, payees and categories will be created.")
        , _t("Confirm Import")
        , wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
    if (msgDlg.ShowModal() == wxID_YES)
    {
        getOrCreateAccounts();
        int nTransactions = vQIF_trxs_.size();
        wxProgressDialog progressDlg(_tu("Please wait…"), _t("Importing")
            , nTransactions + 1, this, wxPD_APP_MODAL | wxPD_CAN_ABORT | wxPD_AUTO_HIDE);
        progressDlg.Update(1, _t("Importing Accounts"));
        bool is_webbapp_enabled = mmWebApp::uploadAccount();

        progressDlg.Update(1, _t("Importing Payees"));
        getOrCreatePayees();
        if (is_webbapp_enabled)
            is_webbapp_enabled = mmWebApp::uploadPayee();
        progressDlg.Update(1, _t("Importing Categories"));
        getOrCreateCategories();
        if (is_webbapp_enabled)
            mmWebApp::uploadCategory();

        TrxModel::DataA trx_a;
        TrxModel::DataA trx_to_a;
        TrxModel::DataA trx_from_a;
        int count = 0;

        const auto begin_date = toDateCtrl_->GetValue().FormatISODate();
        const auto end_date = fromDateCtrl_->GetValue().FormatISODate();

        wxCommandEvent evt;
        OnDecimalChange(evt);

        for (const auto& entry : vQIF_trxs_) {
            if (count % 100 == 0 || count == nTransactions) {
                if (!progressDlg.Update(count,
                    wxString::Format(_t("Importing transaction %1$i of %2$i"), count, nTransactions))) // if cancel clicked
                    break; // abort processing
            }
            //
            TrxData trx_d = TrxData();
            wxString msg;
            if (completeTransaction(entry, &trx_d, msg)) {
                wxString strDate = trx_d.m_date().isoDate();
                if (dateFromCheckBox_->IsChecked() && strDate < begin_date)
                    continue;
                if (dateToCheckBox_->IsChecked() && strDate > end_date)
                    continue;

                AccountData* account = AccountModel::instance().unsafe_get_id_data_n(
                    trx_d.m_account_id
                );
                AccountData* toAccount = AccountModel::instance().unsafe_get_id_data_n(
                    trx_d.m_to_account_id_n
                );

                if (account->is_locked_for(trx_d.m_date()) ||
                    (toAccount && toAccount->is_locked_for(trx_d.m_date()))
                )
                    continue;

                if (trx_d.m_date() < account->m_open_date) {
                    // FIXME: account is changed but not saved
                    account->m_open_date = trx_d.m_date();
                }
                if (toAccount && (trx_d.m_date() < toAccount->m_open_date)) {
                    // FIXME: toAccount is changed but not saved
                    toAccount->m_open_date = trx_d.m_date();
                }

                // Save Transaction Tags
                wxString tagStr = (entry.find(QIF_ID_Category) != entry.end()
                    ? entry.at(QIF_ID_Category).AfterFirst('/')
                    : ""
                );
                TagLinkModel::DataA gl_a;
                if (!tagStr.IsEmpty()) {
                    wxStringTokenizer tagTokens = wxStringTokenizer(tagStr, ":");
                    while (tagTokens.HasMoreTokens()) {
                        wxString tagname = tagTokens.GetNextToken().Trim(false).Trim();
                        // make tag names single-word
                        tagname.Replace(" ", "_");
                        const TagData* tag_n = TagModel::instance().get_name_data_n(tagname);
                        if (!tag_n) {
                            TagData new_tag_d = TagData();
                            new_tag_d.m_name = tagname;
                            TagModel::instance().add_data_n(new_tag_d);
                            tag_n = TagModel::instance().get_id_data_n(new_tag_d.m_id);
                        }
                        TagLinkData gl_d = TagLinkData();
                        gl_d.m_tag_id   = tag_n->m_id;
                        gl_d.m_ref_type = TrxModel::s_ref_type;
                        // Just cache the new taglink since we don't know the m_id yet
                        gl_a.push_back(gl_d);
                    }
                }

                // transactions are split into three groups, then merged
                // since txnIds are not yet created, we need to keep track of what tags go with each cached transaction.
                if (trx_d.is_transfer() && trx_d.m_to_amount > 0.0) {
                    // The "From" tags are stored with key <2, index of from transaction>
                    m_txnTaglinks[std::make_pair(2, trx_from_a.size())] = gl_a;
                    trx_from_a.push_back(trx_d);
                }
                else if (trx_d.is_transfer() && trx_d.m_to_amount <= 0.0) {
                    // The "To" tags are stored with key <1, index of 'to' transaction>
                    m_txnTaglinks[std::make_pair(1, trx_to_a.size())] = gl_a;
                    trx_to_a.push_back(trx_d);
                }
                else {
                    // The non-transfer tags are stored with key <0, index of transaction>
                    m_txnTaglinks[std::make_pair(0, trx_a.size())] = gl_a;
                    trx_a.push_back(trx_d);
                }
            }
            else {
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
        mergeTransferPair(trx_to_a, trx_from_a);
        appendTransfers(trx_a, trx_to_a);

        // Search for duplicates for transfers
        for (auto& trx_d : trx_a) {
            if (!trx_d.is_transfer())
                continue;
            const auto data = TrxModel::instance().find(
                TrxModel::DATE(            OP_EQ, trx_d.m_date()),
                TrxModel::TYPE(            OP_EQ, TrxType(TrxType::e_transfer)),
                TrxCol::ACCOUNTID(         OP_EQ, trx_d.m_account_id),
                TrxCol::TOACCOUNTID(       OP_EQ, trx_d.m_to_account_id_n),
                TrxCol::TRANSAMOUNT(       OP_EQ, trx_d.m_amount),
                TrxCol::TRANSACTIONNUMBER( OP_EQ, trx_d.m_number),
                TrxCol::NOTES(             OP_EQ, trx_d.m_notes)
            );
            if (data.size() > 0)
                trx_d.m_status = TrxStatus(TrxStatus::e_duplicate);
        }
        // At this point all transactions and tags have been merged into single sets
        TagLinkModel::instance().db_savepoint();
        for (int i = 0; i < static_cast<int>(trx_a.size()); i++) {
            if (!m_txnTaglinks[std::make_pair(0, i)].empty()) {
                // we need to know the transid for the taglink, so save the transaction first
                TrxModel::instance().save_trx_n(trx_a[i]);
                // apply that transid to all associated tags
                for (auto& gl_d : m_txnTaglinks[std::make_pair(0, i)])
                    gl_d.m_ref_id = trx_a[i].m_id;
                // save the block of taglinks
                TagLinkModel::instance().save_data_a(m_txnTaglinks[std::make_pair(0, i)]);
            }
        }
        TagLinkModel::instance().db_release_savepoint();
        TrxModel::instance().save_trx_a(trx_a);
        progressDlg.Update(count, _t("Importing Split transactions"));
        joinSplit(trx_a, m_tp_a_a);
        saveSplit();

        sMsg = _t("Import finished successfully.") + "\n" +
            wxString::Format(_t("Total Imported: %zu"), trx_a.size()) + "\n" +
            wxString::Format(_t("Duplicates Detected: %zu"), m_duplicateTransactions.size());

        trx_a.clear();
        vQIF_trxs_.clear();
        btnOK_->Enable(false);
        progressDlg.Destroy();
    }
    else {
        sMsg = _t("Imported transactions discarded by user!"); //TODO: strange message may be _t("Import has discarded by user!")
    }
    save_file_name();
    wxMessageDialog(this, sMsg, _t("Import from QIF file"), wxOK | wxICON_WARNING).ShowModal();
    *log_field_ << sMsg << "\n";

    refreshTabs(ACC_TAB | PAYEE_TAB | CAT_TAB);
}

void mmQIFImportDialog::saveSplit()
{
    if (m_tp_a_a.empty())
        return;

    TrxSplitModel::instance().db_savepoint();
    TagLinkModel::instance().db_savepoint();
    // Work through each group of splits
    for (int i = 0; i < static_cast<int>(m_tp_a_a.size()); i++) {
        // and each split in the group
        for (int j = 0; j < static_cast<int>(m_tp_a_a[i].size()); j++) {
            // save the split
            TrxSplitModel::instance().save_data_n(m_tp_a_a[i][j]);
            int64 tp_id = m_tp_a_a[i][j].m_id;
            // check if there are any taglinks for this split index in this group
            if (!m_gl_a_a[i][j].empty()) {
                // apply the SPLITTRANSID as the REFID for all the cached taglinks
                for (auto& gl_d : m_gl_a_a[i][j])
                    gl_d.m_ref_id = tp_id;
                // save cached taglinks
                TagLinkModel::instance().save_data_a(m_gl_a_a[i][j]);
            }
        }
    }
    TrxSplitModel::instance().db_release_savepoint();
    TagLinkModel::instance().db_release_savepoint();
}

void mmQIFImportDialog::joinSplit(
    TrxModel::DataA& dst_trx_a,
    std::vector<TrxSplitModel::DataA>& tp_a_a
) {
    // no splits in the file
    if (tp_a_a.empty())
        return;

    for (auto& dst_trx_d : dst_trx_a) {
        if (dst_trx_d.m_category_id_n > 0) continue;
        for (auto& tp_d : tp_a_a.at(-1 * dst_trx_d.m_category_id_n.GetValue()))
            tp_d.m_trx_id = dst_trx_d.m_id;
        dst_trx_d.m_category_id_n = -1;
    }
}

void mmQIFImportDialog::appendTransfers(
    TrxModel::DataA& destination,
    TrxModel::DataA& target
) {
    // Here we are moving all the 'to' transfers into the normal transactions, so we also
    // need to keep track of the new index for the taglinks
    for (int i = 0; i < static_cast<int>(target.size()); i++) {
        m_txnTaglinks[std::make_pair(0, destination.size())] = m_txnTaglinks[std::make_pair(1, i)];
        destination.push_back(target[i]);
    }
}

bool mmQIFImportDialog::mergeTransferPair(
    TrxModel::DataA& dst_trx_a,
    TrxModel::DataA& src_trx_a
) {
    // Nothing to merge
    if (dst_trx_a.empty() && src_trx_a.empty())
        return false;

    for (auto& dst_trx_d : dst_trx_a) {
        int i = -1;
        bool pair_found = false;
        for (auto& src_trx_d : src_trx_a) {
            ++i;
            if (dst_trx_d.m_account_id      != src_trx_d.m_to_account_id_n) continue;
            if (dst_trx_d.m_to_account_id_n != src_trx_d.m_account_id)      continue;
            if (dst_trx_d.m_number          != src_trx_d.m_number)          continue;
            if (dst_trx_d.m_notes           != src_trx_d.m_notes)           continue;
            if (dst_trx_d.m_date_time       != src_trx_d.m_date_time)       continue;
            dst_trx_d.m_to_amount = src_trx_d.m_amount;
            src_trx_a.erase(src_trx_a.begin() + i);
            // a match is found so erase the 'from' taglinks
            m_txnTaglinks.erase(std::make_pair(2, i));
            pair_found = true;
            break;
        }

        if (!pair_found)
            dst_trx_d.m_to_amount = dst_trx_d.m_amount;
    }

    // now merge 'from' and 'to' transaction lists
    for (int i = 0; i < static_cast<int>(src_trx_a.size()); i++) {
        std::swap(src_trx_a[i].m_account_id, src_trx_a[i].m_to_account_id_n);
        // also need to move the 'from' taglinks to the 'to' taglinks list, keeping track
        // of the new transaction indices
        m_txnTaglinks[std::make_pair(1, dst_trx_a.size())] = m_txnTaglinks[std::make_pair(2, i)];
        dst_trx_a.push_back(src_trx_a[i]);
    }

    return true;
}

bool mmQIFImportDialog::completeTransaction(
    /* in */ const std::unordered_map <int, wxString> &i,
    /* out */ TrxData* trx_n,
    wxString& msg
) {
    auto t = i;
    wxString trx_type = (t.find(QIF_ID_TrxType) != t.end() ? t[QIF_ID_TrxType] : "");
    if (trx_type.empty()) {
        msg = _t("Transaction code is missing");
        return false;
    }
    trx_n->m_type = TrxType(trx_type);
    bool transfer = trx_n->is_transfer();

    if (!transfer) {
        wxString payee_name = t.find(QIF_ID_Payee) != t.end() ? t.at(QIF_ID_Payee) : "";
        if (!payee_name.empty()) {
            if (m_QIFpayeeNames.find(payee_name) != m_QIFpayeeNames.end()) {
                trx_n->m_payee_id_n = std::get<0>(m_QIFpayeeNames[payee_name]);
                // NOTES haven't been filled yet, so we can just direct assign match details if necessary
                if (payeeMatchAddNotes_->IsChecked()
                    && !std::get<2>(m_QIFpayeeNames[payee_name]).IsEmpty()
                ) {
                    trx_n->m_notes =  wxString::Format(_t("%1$s matched by %2$s"),
                        payee_name, std::get<2>(m_QIFpayeeNames[payee_name])
                    );
                }
            }
            else
                trx_n->m_payee_id_n = -1;
        }
        else {
            trx_n->m_payee_id_n = -1;
        }
    }

    if (trx_n->m_payee_id_n == -1 && !transfer) {
        msg = _t("Transaction Payee is missing or incorrect");
        return false;
    }

    wxString dateStr = (t.find(QIF_ID_Date) != t.end() ? t[QIF_ID_Date] : "");
    if (!m_dateFormatStr.Contains(" "))
        dateStr.Replace(" ", "");
    wxDateTime dtdt;
    wxString::const_iterator end;
    if (dtdt.ParseFormat(dateStr, m_dateFormatStr, &end))
        trx_n->m_date_time = mmDateTime(dtdt);
    else {
        *log_field_ << _t("Date format or date mask is incorrect") << "\n";
        return false;
    }

    int64 accountID = -1;
    wxString accountName = (t.find(QIF_ID_AccountName) != t.end() ? t[QIF_ID_AccountName] : "");
    if ((accountName.empty() || accountCheckBox_->IsChecked()) /*&& !transfer*/) {
        accountName = m_accountNameStr;
    }
    accountID = (m_QIFaccountsID.find(accountName) != m_QIFaccountsID.end())
        ? m_QIFaccountsID.at(accountName)
        : -1;
    if (accountID < 1) {
        msg = _t("Transaction Account is incorrect");
        return false;
    }
    trx_n->m_account_id = accountID;
    trx_n->m_to_account_id_n = (t.find(QIF_ID_ToAccountName) != t.end())
        ? (m_QIFaccountsID.find(t[QIF_ID_ToAccountName]) != m_QIFaccountsID.end()
            ? m_QIFaccountsID[t[QIF_ID_ToAccountName]]
            : -1)
        : -1;
    if (trx_n->m_account_id == trx_n->m_to_account_id_n && transfer) {
        msg = _t("Transaction Account for transfer is incorrect");
        return false;
    }

    trx_n->m_number = (t.find(QIF_ID_TransNumber) != t.end() ? t[QIF_ID_TransNumber] : "");
    // add the actual NOTES before the payee match details
    trx_n->m_notes.
        Prepend(!trx_n->m_notes.IsEmpty() ? "\n" : "").
        Prepend(t.find(QIF_ID_Memo) != t.end() ? t[QIF_ID_Memo] : "");
    TrxStatus trx_status = TrxStatus(TrxStatus::e_unreconciled);
    if (t.find(QIF_ID_Status) != t.end()) {
        wxString s = t[QIF_ID_Status];
        if (s == "X" || s == "R")
            trx_status = TrxStatus(TrxStatus::e_reconciled);
        /*else if (s == "*" || s == "c") {
            TODO: What does 'cleared' trx_status mean?
            trx_status = TrxStatus();
        }*/
    }
    trx_n->m_status = trx_status;

    int color_id = mmColorBtn_->GetColorId();
    trx_n->m_color = -1;
    if (colorCheckBox_->IsChecked() && color_id > 0 && color_id < 8)
        trx_n->m_color = color_id;

    const wxString value = mmTrimAmount(
        t.find(QIF_ID_Amount) != t.end() ? t[QIF_ID_Amount] : "",
        decimal_,
        "."
    );
    if (value.empty()) {
        msg = _t("Transaction Amount is incorrect");
        return false;
    }

    double amt;
    value.ToCDouble(&amt);

    trx_n->m_amount = fabs(amt);
    trx_n->m_to_amount = transfer ? amt : trx_n->m_amount;
    wxString tagStr;
    wxRegEx regex(" ?: ?");
    if (t.find(QIF_ID_CategorySplit) != t.end()) {
        TrxSplitModel::DataA tp_a;
        wxStringTokenizer categToken(t[QIF_ID_CategorySplit], "\n");
        wxStringTokenizer amtToken(
            (t.find(QIF_ID_AmountSplit) != t.end() ? t[QIF_ID_AmountSplit] : ""),
            "\n"
        );
        wxString notes = t.find(QIF_ID_MemoSplit) != t.end() ? t[QIF_ID_MemoSplit] : "";
        int split_id = 1;

        while (categToken.HasMoreTokens()) {
            const wxString c = categToken.GetNextToken().BeforeFirst('/', &tagStr);
            if (m_QIFcategoryNames.find(c) == m_QIFcategoryNames.end()) return false;
            int64 categID = m_QIFcategoryNames[c];
            if (categID <= 0) {
                msg = _t("Transaction Category is incorrect");
                return false;
            }
            TrxSplitData tp_d = TrxSplitData();
            tp_d.m_category_id = categID;

            wxString amtSplit = amtToken.GetNextToken();
            amtSplit = mmTrimAmount(amtSplit, decimal_, ".");
            double amount;
            amtSplit.ToCDouble(&amount);

            wxString memo;
            while (!notes.empty()) {
                wxRegEx pattern(wxString::Format("^%d:(.*)", split_id), wxRE_NEWLINE);
                if (pattern.Matches(notes)) {
                    memo += (!memo.IsEmpty() ? "\n" : "" ) + pattern.GetMatch(notes, 1);
                    pattern.ReplaceFirst(&notes, "");
                    notes.Replace("\n", "", false);
                }
                else
                    break;
            }

            tp_d.m_amount = (trx_n->is_deposit() ? amount : -amount);
            tp_d.m_trx_id = trx_n->m_id;
            tp_d.m_notes  = memo;
            tp_a.push_back(tp_d);

            // Save split tags
            if (!tagStr.IsEmpty()) {
                TagLinkModel::DataA splitTaglinks;
                wxStringTokenizer tagTokens = wxStringTokenizer(tagStr, ":");
                while (tagTokens.HasMoreTokens()) {
                    wxString tagname = tagTokens.GetNextToken().Trim(false).Trim();
                    // make tag names single-word
                    tagname.Replace(" ", "_");
                    const TagData* tag_n = TagModel::instance().get_name_data_n(tagname);
                    if (!tag_n) {
                        TagData new_tag_d = TagData();
                        new_tag_d.m_name = tagname;
                        TagModel::instance().add_data_n(new_tag_d);
                        tag_n = TagModel::instance().get_id_data_n(new_tag_d.m_id);
                    }
                    TagLinkData gl_d = TagLinkData();
                    gl_d.m_tag_id   = tag_n->m_id;
                    gl_d.m_ref_type = TrxSplitModel::s_ref_type;
                    splitTaglinks.push_back(gl_d);
                }
                // Here we keep track of which block of splits and which split in the block
                // each group of taglinks is associated with. Once we save the splits we can
                // record the SPLITTRANSID on the taglink
                m_gl_a_a[m_tp_a_a.size()][split_id - 1] = splitTaglinks;
            }
            split_id++;
        }
        trx_n->m_category_id_n = -1 * static_cast<int>(m_tp_a_a.size());
        m_tp_a_a.push_back(tp_a);
    }
    else {
        wxString categStr = (t.find(QIF_ID_Category) != t.end()
            ? t.at(QIF_ID_Category).BeforeFirst('/')
            : ""
        );
        if (categStr.empty()) {
            const PayeeData* payee_n = PayeeModel::instance().get_id_data_n(trx_n->m_payee_id_n);
            if (payee_n) {
                trx_n->m_category_id_n = payee_n->m_category_id_n;
            }
            categStr = CategoryModel::instance().get_id_fullname(trx_n->m_category_id_n, ":");

            if (categStr.empty()) {
                trx_n->m_category_id_n = (m_QIFcategoryNames[_t("Unknown")]);
            }
        }
        else {
            trx_n->m_category_id_n = (m_QIFcategoryNames[categStr]);
        }
    }

    // Check for duplicates according to user choice
    if (dupTransCheckBox_->IsChecked()) {
        bool isDuplicate = false;
        int dupMethod = dupTransMethod_->GetSelection();
        int dupAction = dupTransAction_->GetSelection();

        // By transaction number
        if (dupMethod == 0) {
            if (!trx_n->m_number.empty()) {
                const auto existing_transactions = TrxModel::instance().find(
                    TrxCol::TRANSACTIONNUMBER(OP_EQ, trx_n->m_number),
                    TrxModel::IS_DELETED(false)
                );

                isDuplicate = !existing_transactions.empty();
            }
        }
        // By amount and date (exact or nearby)
        else if (dupMethod == 1 || dupMethod == 2) {
            mmDate startDate = trx_n->m_date();
            mmDate endDate = startDate;
            // nearby date
            if (dupMethod != 1) {
                startDate.addDateSpan(wxDateSpan::Days(-4));
                endDate.addDateSpan(wxDateSpan::Days(2));
            }

            const auto potential_matches = TrxModel::instance().find(
                TrxModel::DATE(OP_GE, startDate),
                TrxModel::DATE(OP_LE, endDate),
                TrxCol::TRANSAMOUNT(trx_n->m_amount),
                TrxModel::IS_DELETED(false)
            );

            for (const auto& existingTrx : potential_matches) {
                bool alreadyMatched = m_duplicateTransactions.find(existingTrx.m_id) != m_duplicateTransactions.end();
                if (!alreadyMatched) {
                    isDuplicate = true;
                    m_duplicateTransactions.insert(existingTrx.m_id);
                    break;
                }
            }
        }

        if (isDuplicate) {
            // Skip
            if (dupAction == 0) {
                msg = _t("Transaction skipped as duplicate");
                return false;
            }
            else if (dupAction == 1) // Flag as duplicate
                trx_n->m_status = TrxStatus(TrxStatus::e_duplicate);
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

    for (auto& item : m_QIFaccounts) {
        int64 account_id = -1;
        const AccountData* account_n = (accountNumberCheckBox_->IsChecked())
            ? AccountModel::instance().get_num_data_n(item.first)
            : AccountModel::instance().get_name_data_n(item.first);

        if (!account_n) {
            AccountData account_d = AccountData();
            account_d.m_favorite = AccountFavorite(true);

            const auto type = item.second.find(QIF_ID_AccountType) != item.second.end() ? item.second.at(QIF_ID_AccountType) : "";
            account_d.m_type_ = mmExportTransaction::mm_acc_type(type);
            //NavigatorTypes::TYPE_NAME_CHECKING;
            account_d.m_name         = item.first;
            account_d.m_open_balance = 0;
            account_d.m_open_date    = mmDate::today();
            account_d.m_currency_id  = CurrencyModel::instance().get_base_data_n()->m_id;
            const wxString c = (item.second.find(QIF_ID_Description) == item.second.end()
                ? ""
                : item.second.at(QIF_ID_Description)
            );
            for (const auto& curr : CurrencyModel::instance().find_all()) {
                if (wxString::Format("[%s]", curr.m_symbol) == c) {
                    account_d.m_currency_id = curr.m_id;
                    break;
                }
            }

            AccountModel::instance().add_data_n(account_d);
            account_id = account_d.m_id;
            wxString sMsg = wxString::Format(_t("Added account: %s"), item.first);
            *log_field_ << sMsg << "\n";
        }
        else
            account_id = account_n->m_id;

        m_QIFaccountsID[item.first] = account_id;
    }

    const AccountData* account_n = AccountModel::instance().get_name_data_n(m_accountNameStr);
    if (account_n) {
        m_QIFaccountsID[m_accountNameStr] = account_n->m_id;
    }

    return m_QIFaccountsID.size();
}

void mmQIFImportDialog::getOrCreatePayees()
{
    PayeeModel::instance().db_savepoint();

    for (const auto& item : m_payee_names) {
        // check if this payee exists
        if (m_QIFpayeeNames.find(item) != m_QIFpayeeNames.end() &&
            std::get<0>(m_QIFpayeeNames[item]) != -1
        )
            continue;

        // the payee doesn't exist or match a pattern, so create one
        PayeeData new_payee_d = PayeeData();
        new_payee_d.m_name = item;
        PayeeModel::instance().save_data_n(new_payee_d);
        wxString sMsg = wxString::Format(_t("Added payee: %s"), item);
        log_field_->AppendText(wxString() << sMsg << "\n");
        m_QIFpayeeNames[item] = std::make_tuple(new_payee_d.m_id, new_payee_d.m_name, "");
    }

    PayeeModel::instance().db_release_savepoint();
}

void mmQIFImportDialog::getOrCreateCategories()
{
    wxArrayString temp;
    for (const auto& item : m_QIFcategoryNames) {
        wxString categStr;
        wxStringTokenizer token(item.first, ":");
        int64 parentID = -1;
        while(token.HasMoreTokens()){
            categStr = token.GetNextToken().Trim(false).Trim();
            const CategoryData* cat_n = CategoryModel::instance().get_key_data_n(
                categStr, parentID
            );
            if (temp.Index(categStr + wxString::Format(":%lld", parentID)) == wxNOT_FOUND) {
                if (!cat_n) {
                    CategoryData new_cat_d = CategoryData();
                    new_cat_d.m_name        = categStr;
                    new_cat_d.m_parent_id_n = parentID;
                    CategoryModel::instance().add_data_n(new_cat_d);
                    cat_n = CategoryModel::instance().get_id_data_n(new_cat_d.m_id);
                }
                temp.Add(categStr + wxString::Format(":%lld", parentID));
            }
            parentID = cat_n->m_id;
        }
        m_QIFcategoryNames[item.first] = parentID;
    }
}

int64 mmQIFImportDialog::get_last_imported_acc()
{
    const AccountData* account_n = AccountModel::instance().get_name_data_n(m_accountNameStr);
    return account_n ? account_n->m_id : -1;
}

void mmQIFImportDialog::OnDecimalChange(wxCommandEvent& event)
{
    int i = m_choiceDecimalSeparator->GetSelection();
    wxStringClientData* type_obj = static_cast<wxStringClientData*>(
        m_choiceDecimalSeparator->GetClientObject(i)
    );
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
        SettingModel::instance().prependArrayItem("RECENT_QIF_FILES", m_FileNameStr, 10);
    }
}

void mmQIFImportDialog::OnMenuSelected(wxCommandEvent& WXUNUSED(event))
{
    mmColorBtn_->Enable(false);
    colorCheckBox_->SetValue(false);
}
