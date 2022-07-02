/*******************************************************
Copyright (C) 2013-2020 Nikolay Akimov

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
#include "export.h"
#include "constants.h"
#include "mmSimpleDialogs.h"
#include "paths.h"
#include "util.h"
#include "webapp.h"
#include "option.h"

#include "model/Model_Category.h"
#include "model/Model_Subcategory.h"
#include "model/Model_Payee.h"

#include <wx/progdlg.h>
#include <wx/dataview.h>

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

mmQIFImportDialog::mmQIFImportDialog(wxWindow* parent, int account_id)
    : m_userDefinedDateMask(false)
    , m_today(wxDate::Today())
    , m_fresh(wxDate::Today().Subtract(wxDateSpan::Months(1)))
    , dataListBox_(nullptr)
    , accListBox_(nullptr)
    , payeeListBox_(nullptr)
    , categoryListBox_(nullptr)
    , button_search_(nullptr)
    , file_name_ctrl_(nullptr)
    , m_choiceEncoding(nullptr)
    , log_field_(nullptr)
    , dateFromCheckBox_(nullptr)
    , dateToCheckBox_(nullptr)
    , fromDateCtrl_(nullptr)
    , toDateCtrl_(nullptr)
    , choiceDateFormat_(nullptr)
    , accountCheckBox_(nullptr)
    , accountDropDown_(nullptr)
    , btnOK_(nullptr)
    , m_choiceDecimalSeparator(nullptr)
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

    ColName_[COL_ID] = "#";
    ColName_[COL_ACCOUNT] = _("Account");
    ColName_[COL_DATE] = _("Date");
    ColName_[COL_NUMBER] = _("Number");
    ColName_[COL_PAYEE] = _("Payee");
    ColName_[COL_TYPE] = _("Type");
    ColName_[COL_CATEGORY] = _("Category");
    ColName_[COL_VALUE] = _("Value");
    ColName_[COL_NOTES] = _("Notes");

    CreateControls();
    fillControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    this->SetInitialSize();
    SetIcon(mmex::getProgramIcon());
    Centre();

    return TRUE;
}

void mmQIFImportDialog::CreateControls()
{
    wxSizerFlags flagsExpand;
    flagsExpand.Align(wxALIGN_LEFT | wxALIGN_TOP).Border(wxLEFT | wxRIGHT | wxTOP, 5);

    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(main_sizer);
    wxBoxSizer* left_sizer = new wxBoxSizer(wxVERTICAL);

    //File to import, file path and browse button
    wxPanel* file_panel = new wxPanel(this
        , wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    main_sizer->Add(file_panel, 0, wxEXPAND | wxALL, 1);

    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer(wxHORIZONTAL);
    file_panel->SetSizer(itemBoxSizer7);

    wxStaticText* file_name_label = new wxStaticText(file_panel, wxID_ANY, _("File Name:"));
    itemBoxSizer7->Add(file_name_label, g_flagsH);

    wxArrayString files = Model_Setting::instance().GetArrayStringSetting("RECENT_QIF_FILES");
    file_name_ctrl_ = new  wxComboBox(file_panel, wxID_FILE, "", wxDefaultPosition, wxDefaultSize, files, wxTE_PROCESS_ENTER);
    file_name_ctrl_->SetMinSize(wxSize(300, -1));
    itemBoxSizer7->Add(file_name_ctrl_, 1, wxALL | wxGROW, 5);
    file_name_ctrl_->Connect(wxID_FILE
        , wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(mmQIFImportDialog::OnFileNameChanged), nullptr, this);
    file_name_ctrl_->Connect(wxID_FILE
        , wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(mmQIFImportDialog::OnFileNameChanged), nullptr, this);

    button_search_ = new wxButton(file_panel, wxID_OPEN, _("&Browse"));
    itemBoxSizer7->Add(button_search_, g_flagsH);
    button_search_->Connect(wxID_OPEN, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmQIFImportDialog::OnFileSearch), nullptr, this);


    wxFlexGridSizer* flex_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    //flex_sizer->AddGrowableCol(1);
    left_sizer->Add(flex_sizer, g_flagsExpand);

    //Encoding
    wxStaticText* itemStaticText88 = new wxStaticText(this, wxID_STATIC, wxString(_("Encoding:")));
    //itemStaticText88->SetFont(staticBoxFontSetting);
    flex_sizer->Add(itemStaticText88, g_flagsH);

    m_choiceEncoding = new wxChoice(this, wxID_ANY);
    for (const auto i : g_encoding)
        m_choiceEncoding->Append(wxGetTranslation(i.second.second), new wxStringClientData(i.second.second));
    m_choiceEncoding->SetSelection(0);

    flex_sizer->Add(m_choiceEncoding, g_flagsH);

    //Account
    accountCheckBox_ = new wxCheckBox(this, wxID_FILE5, _("Account")
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
    wxStaticBox* static_box = new wxStaticBox(this, wxID_ANY, _("Filtering Details:"));
    wxStaticBoxSizer* filter_sizer = new wxStaticBoxSizer(static_box, wxVERTICAL);
    wxFlexGridSizer* flex_sizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    filter_sizer->Add(flex_sizer2, g_flagsExpand);

    // From Date
    dateFromCheckBox_ = new wxCheckBox(static_box, wxID_FILE8, _("From Date")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    fromDateCtrl_ = new mmDatePickerCtrl(static_box, wxID_STATIC, wxDefaultDateTime
        , wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
    fromDateCtrl_->SetMinSize(wxSize(150, -1));
    fromDateCtrl_->Enable(false);
    flex_sizer2->Add(dateFromCheckBox_, g_flagsH);
    flex_sizer2->Add(fromDateCtrl_, g_flagsH);

    // To Date
    dateToCheckBox_ = new wxCheckBox(static_box, wxID_FILE9, _("To Date")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    toDateCtrl_ = new mmDatePickerCtrl(static_box, wxID_STATIC, wxDefaultDateTime
        , wxDefaultPosition, wxSize(150, -1), wxDP_DROPDOWN);
    toDateCtrl_->Enable(false);
    flex_sizer2->Add(dateToCheckBox_, g_flagsH);
    flex_sizer2->Add(toDateCtrl_, g_flagsH);

    //Data viewer ----------------------------------------------
    wxNotebook* qif_notebook = new wxNotebook(this
        , wxID_FILE9, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE);

    wxPanel* log_tab = new wxPanel(qif_notebook, wxID_ANY);
    qif_notebook->AddPage(log_tab, _("Log"));
    wxBoxSizer* log_sizer = new wxBoxSizer(wxVERTICAL);
    log_tab->SetSizer(log_sizer);

    log_field_ = new wxTextCtrl(log_tab, wxID_ANY, ""
        , wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxHSCROLL);
    log_field_->SetMinSize(wxSize(500, -1));
    log_sizer->Add(log_field_, g_flagsExpand);

    wxPanel* data_tab = new wxPanel(qif_notebook, wxID_ANY);
    qif_notebook->AddPage(data_tab, _("Data"));
    wxBoxSizer* data_sizer = new wxBoxSizer(wxVERTICAL);
    data_tab->SetSizer(data_sizer);

    dataListBox_ = new wxDataViewListCtrl(data_tab, wxID_ANY);
    dataListBox_->SetMinSize(wxSize(100, 200));
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

    accListBox_ = new wxDataViewListCtrl(acc_tab, wxID_ANY);
    accListBox_->AppendTextColumn(_("Name"), wxDATAVIEW_CELL_INERT, 250, wxALIGN_LEFT);
    accListBox_->AppendTextColumn(_("Type"), wxDATAVIEW_CELL_INERT, 50, wxALIGN_LEFT);
    accListBox_->AppendTextColumn(_("Currency"), wxDATAVIEW_CELL_INERT, 50, wxALIGN_LEFT);
    accListBox_->AppendTextColumn(_("Status"), wxDATAVIEW_CELL_INERT, 150, wxALIGN_LEFT);
    acc_sizer->Add(accListBox_, g_flagsExpand);

    //Payees
    wxPanel* payee_tab = new wxPanel(qif_notebook, wxID_ANY);
    qif_notebook->AddPage(payee_tab, _("Payee"));
    wxBoxSizer* payee_sizer = new wxBoxSizer(wxHORIZONTAL);
    payee_tab->SetSizer(payee_sizer);

    payeeListBox_ = new wxDataViewListCtrl(payee_tab, wxID_ANY);
    payeeListBox_->AppendTextColumn(_("Name"), wxDATAVIEW_CELL_INERT, 250, wxALIGN_LEFT);
    payeeListBox_->AppendTextColumn(_("Status"), wxDATAVIEW_CELL_INERT, 150, wxALIGN_LEFT);
    payee_sizer->Add(payeeListBox_, g_flagsExpand);

    //Category
    wxPanel* categ_tab = new wxPanel(qif_notebook, wxID_ANY);
    qif_notebook->AddPage(categ_tab, _("Category"));
    wxBoxSizer* category_sizer = new wxBoxSizer(wxHORIZONTAL);
    categ_tab->SetSizer(category_sizer);
    categoryListBox_ = new wxDataViewListCtrl(categ_tab, wxID_ANY);
    categoryListBox_->AppendTextColumn(_("Name"), wxDATAVIEW_CELL_INERT, 250, wxALIGN_LEFT);
    categoryListBox_->AppendTextColumn(_("Status"), wxDATAVIEW_CELL_INERT, 150, wxALIGN_LEFT);
    category_sizer->Add(categoryListBox_, g_flagsExpand);

    //Compose all sizers togethe
    wxBoxSizer* top_sizer = new wxBoxSizer(wxHORIZONTAL);
    top_sizer->Add(left_sizer, g_flagsH);
    top_sizer->Add(filter_sizer, g_flagsH);

    //Use account number instead of account name :
    accountNumberCheckBox_ = new wxCheckBox(this, wxID_FILE6, _("Use account number instead of account name")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);

    //Use payee as desc :
    payeeIsNotesCheckBox_ = new wxCheckBox(this, wxID_FILE7, _("Include payee field in notes")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    payeeIsNotesCheckBox_->SetValue(payeeIsNotes_);


    // Date Format Settings
    m_dateFormatStr = Option::instance().getDateFormat();

    wxStaticText* dateFormat = new wxStaticText(this, wxID_STATIC, _("Date Format"));
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
    flex_sizer_b->Add(payeeIsNotesCheckBox_, g_flagsBorder1H);
    flex_sizer_b->AddSpacer(1);

    wxBoxSizer* date_sizer = new wxBoxSizer(wxHORIZONTAL);
    date_sizer->Add(dateFormat, g_flagsBorder1H);
    date_sizer->Add(choiceDateFormat_, g_flagsBorder1H);
    flex_sizer_b->Add(date_sizer, g_flagsBorder1H);


    wxStaticText* decamalCharText = new wxStaticText(this, wxID_STATIC, _("Decimal Char"));
    m_choiceDecimalSeparator = new mmChoiceAmountMask(this, wxID_ANY);
    wxBoxSizer* decamalCharSizer = new wxBoxSizer(wxHORIZONTAL);
    decamalCharSizer->Add(decamalCharText, g_flagsBorder1H);
    decamalCharSizer->Add(m_choiceDecimalSeparator, g_flagsBorder1H);
    m_choiceDecimalSeparator->SetDecimalChar(decimal_);
    m_choiceDecimalSeparator->Connect(wxID_ANY, wxEVT_COMMAND_CHOICE_SELECTED
        , wxCommandEventHandler(mmQIFImportDialog::OnDecimalChange), nullptr, this);

    flex_sizer_b->Add(decamalCharSizer, g_flagsH);
    //

    colorCheckBox_ = new wxCheckBox(this, wxID_PASTE, _("Color")
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

    btnOK_ = new wxButton(buttons_panel, wxID_OK, _("&OK "));
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
    m_QIFcategoryNames[_("Unknown")] = std::make_pair(-1, -1);
    m_QIFpayeeNames.clear();
    m_payee_names.clear();
    m_payee_names.Add(_("Unknown"));

    wxFileInputStream input(m_FileNameStr);
    wxConvAuto conv = g_encoding.at(m_choiceEncoding->GetSelection()).first;
    wxTextInputStream text(input, "\x09", conv);

    wxProgressDialog progressDlg(_("Please wait"), _("Scanning")
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
            if (!progressDlg.Pulse(wxString::Format(_("Reading line %zu, %lld ms")
                , numLines, interval)))
                break;
        }
        if (numLines <= 50)
        {
            *log_field_ << wxString::Format(_("Line %zu \t %s\n"), numLines, lineStr);
            if (numLines == 50)
                *log_field_ << "-------------------------------------- 8< --------------------------------------\n";
        }

        const qifLineType lineType = mmQIFImport::lineType(lineStr);
        const auto data = mmQIFImport::getLineData(lineStr);
        if (lineType == EOTLT)
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

            if (completeTransaction(trx, m_accountNameStr)) {
                vQIF_trxs_.push_back(trx);
            }
            trx.clear();
            continue;
        }

        //Parse Categories
        const wxString& s = trx.find(CategorySplit) != trx.end() ? trx[CategorySplit] : "";
        if (!s.empty())
        {
            wxStringTokenizer token(s, "\n");
            while (token.HasMoreTokens())
            {
                wxString c = token.GetNextToken();
                qif_api->getFinancistoProject(c);
                if (m_QIFcategoryNames.find(c) == m_QIFcategoryNames.end())
                    m_QIFcategoryNames[c] = std::make_pair(-1, -1);
            }
        }

        //Parse date format
        if (!m_userDefinedDateMask && lineType == Date && (data.Mid(0, 1) != "["))
        {
            dParser->doHandleStatistics(data);
        }

        //Parse numbers
        if (lineType == Amount)
        {
            comma["."] += data.Contains(".") ? data.find(".") + 1 : 0;
            comma[","] += data.Contains(",") ? data.find(",") + 1 : 0;
        }

        if (trx[lineType].empty() || lineType == AcctType)
            trx[lineType] = data;
        else
            trx[lineType] += "\n" + data;

    }
    log_field_->ScrollLines(log_field_->GetNumberOfLines());

    if (comma[","] > comma["."]) {
        m_choiceDecimalSeparator->SetDecimalChar(",");
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
    wxString sMsg = wxString::Format(_("Number of lines read from QIF file: %zu in %lld ms")
        , numLines, interval);
    *log_field_ << sMsg << "\n";

    if (!m_QIFaccounts.empty()) {
        sMsg = _("Accounts:");
        for (const auto& i : m_QIFaccounts) {
            sMsg += ("\n" + i.first);
        }
        *log_field_ << sMsg << "\n";
    }

    sMsg = _("Date Format: ") + "\n" + choiceDateFormat_->GetValue();
    *log_field_ << sMsg << "\n";

    sMsg = wxString::Format(_("Press OK Button to continue"));
    *log_field_ << sMsg << "\n";

    return true;
}

bool mmQIFImportDialog::completeTransaction(std::unordered_map <int, wxString> &trx, const wxString &accName)
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
            if (m_QIFcategoryNames.find(c) == m_QIFcategoryNames.end())
                m_QIFcategoryNames[c] = std::make_pair(-1, -1);
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
        if (trx[Category].Mid(0, 1) == "[" && trx[Category].Last() == ']')
        {
            wxString toAccName = trx[Category].SubString(1, trx[Category].length() - 2);

            if (toAccName == m_accountNameStr)
            {
                trx[Category] = trx[Payee];
                trx[Payee] = toAccName;
            }
            else
            {
                isTransfer = true;
                trx[Category] = "Transfer";
                trx[TrxType] = Model_Checking::TRANSFER_STR;
                trx[ToAccountName] = toAccName;
                trx[Memo] += (trx[Memo].empty() ? "" : "\n") + trx[Payee];
                if (m_QIFaccounts.find(toAccName) == m_QIFaccounts.end())
                {
                    std::unordered_map <int, wxString> a;
                    a[Description] = "[" + Model_Currency::GetBaseCurrency()->CURRENCY_SYMBOL + "]";
                    a[AccountType] = (trx.find(Description) != trx.end() ? trx.at(Description) : "");
                    m_QIFaccounts[toAccName] = a;
                }
            }
        }

        //Cut non standard info after /
        const wxString categ_suffix = qif_api->getFinancistoProject(trx[Category]);
        if (!categ_suffix.empty()) trx[Memo] += (trx[Memo].empty() ? "" : "/n") + categ_suffix;

        //Add the full Category name if missing to the map with undefind category ID and Subcategory ID
        if (m_QIFcategoryNames.find(trx[Category]) == m_QIFcategoryNames.end())
            m_QIFcategoryNames[trx[Category]] = std::make_pair(-1, -1);
    }

    if (!isTransfer)
    {
        wxString payee_name = trx.find(Payee) != trx.end() ? trx[Payee] : "";
        if (payee_name.empty() && trx[AcctType] != "Account" )
        {
            payee_name = trx.find(AccountName) != trx.end() ? trx[AccountName] : _("Unknown");
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
        }
    }

    if (payeeIsNotes_) {
        trx[Memo] += (trx[Memo].empty() ? "" : "\n") + trx[Payee];
    }

    wxString amtStr = (trx.find(Amount) == trx.end() ? "" : trx[Amount]);
    if (!isTransfer) {
        if (amtStr.Mid(0, 1) == "-")
            trx[TrxType] = Model_Checking::WITHDRAWAL_STR;
        else if (!amtStr.empty())
            trx[TrxType] = Model_Checking::DEPOSIT_STR;
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
                    dateStr = mmGetDateForDisplay(dtdt.FormatISODate());
                else
                    dateStr.Prepend("!");
            }

            data.push_back(wxVariant(dateStr));
            data.push_back(wxVariant(trx.find(TransNumber) != trx.end() ? trx.at(TransNumber) : ""));
            const wxString type = (trx.find(TrxType) != trx.end() ? trx.at(TrxType) : "");
            if (type == Model_Checking::TRANSFER_STR)
                data.push_back(wxVariant(trx.find(ToAccountName) != trx.end() ? trx.at(ToAccountName) : ""));
            else
                data.push_back(wxVariant(trx.find(Payee) != trx.end() ? trx.at(Payee) : ""));
            data.push_back(wxVariant(trx.find(TrxType) != trx.end() ? trx.at(TrxType) : ""));

            wxString category;
            if (trx.find(CategorySplit) != trx.end()) {
                category = trx.at(CategorySplit);
                category.Prepend("*").Replace("\n", "|");
            }
            else
                category = (trx.find(Category) != trx.end() ? trx.at(Category) : "");
            data.push_back(wxVariant(category));

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
                    status = _("OK");
                else
                    status = _("Warning");
                if (account->ACCOUNTTYPE != mmExportTransaction::mm_acc_type(type))
                {
                    status = _("Warning");
                }
                data.push_back(wxVariant(account->ACCOUNTNAME));
            }
            else {
                status = _("Missing");
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
        payeeListBox_->DeleteAllItems();
        for (const auto& payee : m_payee_names)
        {
            wxVector<wxVariant> data;
            data.push_back(wxVariant(payee));
            Model_Payee::Data* p = Model_Payee::instance().get(payee);
            data.push_back(wxVariant(p ? _("OK") : _("Missing")));
            payeeListBox_->AppendItem(data, static_cast<wxUIntPtr>(num++));
        }
    }

    if (tabs & CAT_TAB)
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
            categoryListBox_->AppendItem(data, static_cast<wxUIntPtr>(num++));
        }
    }
}

void mmQIFImportDialog::OnFileSearch(wxCommandEvent& WXUNUSED(event))
{
    m_FileNameStr = file_name_ctrl_->GetValue();

    m_FileNameStr = wxFileSelector(_("Choose QIF data file to Import")
        , wxEmptyString, m_FileNameStr, wxEmptyString
        , _("QIF Files (*.qif)") + "|*.qif;*.QIF"
        , wxFD_OPEN | wxFD_CHANGE_DIR | wxFD_FILE_MUST_EXIST, this); //TODO: Remove UI Blinking

    if (!m_FileNameStr.IsEmpty()) {
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
        wxString sMsg = _("Date Format: ") + "\n" + choiceDateFormat_->GetValue();
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
    case wxID_FILE6:
        t = t | PAYEE_TAB;
        break;
    case wxID_FILE7:
        t = t | PAYEE_TAB;
        payeeIsNotes_ = payeeIsNotesCheckBox_->IsChecked();
        if (!m_FileNameStr.IsEmpty())
            mmReadQIFFile(); //TODO: 1:Why read file again? 2:In future may be def payee in settings
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
    wxMessageDialog msgDlg(this, _("Do you want to import all transaction ?")
        + "\n" + _("All missing account, payees and categories will be created.")
        , _("Confirm Import")
        , wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
    if (msgDlg.ShowModal() == wxID_YES)
    {
        getOrCreateAccounts();
        int nTransactions = vQIF_trxs_.size();
        wxProgressDialog progressDlg(_("Please wait"), _("Importing")
            , nTransactions + 1, this, wxPD_APP_MODAL | wxPD_CAN_ABORT | wxPD_AUTO_HIDE);
        progressDlg.Update(1, _("Importing Accounts"));

        mmWebApp::MMEX_WebApp_UpdateAccount();
        progressDlg.Update(1, _("Importing Payees"));
        getOrCreatePayees();
        mmWebApp::MMEX_WebApp_UpdatePayee();
        progressDlg.Update(1, _("Importing Categories"));
        getOrCreateCategories();
        mmWebApp::MMEX_WebApp_UpdateCategory();

        Model_Checking::Cache trx_data_set;
        Model_Checking::Cache transfer_to_data_set;
        Model_Checking::Cache transfer_from_data_set;
        int count = 0;
        const wxString& transferStr = Model_Checking::TRANSFER_STR;

        const auto begin_date = toDateCtrl_->GetValue().FormatISODate();
        const auto end_date = fromDateCtrl_->GetValue().FormatISODate();
        for (const auto& entry : vQIF_trxs_)
        {
            if (count % 100 == 0 || count == nTransactions)
            {
                if (!progressDlg.Update(count
                    , wxString::Format(_("Importing transaction %i of %i"), count, nTransactions))) // if cancel clicked
                    break; // abort processing
            }
            //
            Model_Checking::Data *trx = Model_Checking::instance().create();
            wxString msg;
            if (completeTransaction(entry, trx, msg))
            {
                if (dateFromCheckBox_->IsChecked() && trx->TRANSDATE < begin_date)
                    continue;
                if (dateToCheckBox_->IsChecked() && trx->TRANSDATE > end_date)
                    continue;

                if (trx->TRANSCODE == transferStr && trx->TOTRANSAMOUNT > 0.0)
                    transfer_from_data_set.push_back(trx);
                else if (trx->TRANSCODE == transferStr && trx->TOTRANSAMOUNT <= 0.0)
                    transfer_to_data_set.push_back(trx);
                else
                    trx_data_set.push_back(trx);
            }
            else
            {
                *log_field_ << wxString::Format(_("Error: %s"), msg);

                wxString t = "";
                for (const auto&i : entry)
                    t << i.second << "|";
                t.RemoveLast(1);
                *log_field_ << wxString::Format("\n( %s )\n", t);
            }
            ++count;
        }

        progressDlg.Update(count, _("Importing Transfers"));
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
                , Model_Checking::TRANSCODE(Model_Checking::TRANSFER)
                , Model_Checking::TRANSAMOUNT(trx->TRANSAMOUNT)
            );
            if (data.size() > 0)
                trx->STATUS = "D";
        }

        Model_Checking::instance().save(trx_data_set);
        progressDlg.Update(count, _("Importing Split transactions"));
        joinSplit(trx_data_set, m_splitDataSets);
        saveSplit();

        sMsg = _("Import finished successfully") + "\n" + wxString::Format(_("Total Imported: %zu"), trx_data_set.size());
        trx_data_set.clear();
        vQIF_trxs_.clear();
        btnOK_->Enable(false);
        progressDlg.Destroy();

        save_file_name();
    }
    else
    {
        sMsg = _("Imported transactions discarded by user!"); //TODO: strange message may be _("Import has discarded by user!")
    }
    wxMessageDialog(this, sMsg, _("QIF Import"), wxOK | wxICON_WARNING).ShowModal();
    *log_field_ << sMsg << "\n";

    refreshTabs(ACC_TAB | PAYEE_TAB | CAT_TAB);
}

void mmQIFImportDialog::saveSplit()
{
    if (m_splitDataSets.empty()) return;

    Model_Splittransaction::instance().Savepoint();
    while (!m_splitDataSets.empty()) {
        Model_Splittransaction::instance().save(m_splitDataSets.back());
        m_splitDataSets.pop_back();
    }
    Model_Splittransaction::instance().ReleaseSavepoint();
}
void mmQIFImportDialog::joinSplit(Model_Checking::Cache &destination
    , std::vector<Model_Splittransaction::Cache> &target)
{
    for (auto &item : destination)
    {
        if (item->CATEGID != -1) continue;
        if (item->SUBCATEGID == -1) continue;
        for (auto &split_item : target.at(item->SUBCATEGID))
            split_item->TRANSID = item->TRANSID;
        item->SUBCATEGID = -1;
    }
}

void mmQIFImportDialog::appendTransfers(Model_Checking::Cache &destination, Model_Checking::Cache &target)
{
    while (!target.empty()) {
        destination.push_back(target.back());
        target.pop_back();
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
            pair_found = true;
            break;
        }

        if (!pair_found)
            refTrxTo->TOTRANSAMOUNT = refTrxTo->TRANSAMOUNT;
    }

    while (!from.empty()) {
        std::swap(from.back()->ACCOUNTID, from.back()->TOACCOUNTID);
        to.push_back(from.back());
        from.pop_back();
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
        msg = _("Transaction code is missing");
        return false;
    }
    bool transfer = Model_Checking::is_transfer(trx->TRANSCODE);

    if (!transfer)
    {
        wxString payee_name = t.find(Payee) != t.end() ? t.at(Payee) : "";
        if (!payee_name.empty())
        {
            trx->PAYEEID = m_QIFpayeeNames.find(payee_name) != m_QIFpayeeNames.end() ? m_QIFpayeeNames[payee_name] : -1;
        }
        else
        {
            trx->PAYEEID = -1;
        }
    }

    if (trx->PAYEEID == -1 && !transfer)
    {
        msg = _("Transaction Payee is missing or incorrect");
        return false;
    }

    wxString dateStr = (t.find(Date) != t.end() ? t[Date] : "");
    if (!m_dateFormatStr.Contains(" ")) dateStr.Replace(" ", "");
    wxDateTime dtdt;
    wxString::const_iterator end;
    if (dtdt.ParseFormat(dateStr, m_dateFormatStr, &end))
        trx->TRANSDATE = dtdt.FormatISODate();
    else
    {
        *log_field_ << _("Date format or date mask is incorrect") << "\n";
        return false;
    }

    int accountID = -1;
    wxString accountName = (t.find(AccountName) != t.end() ? t[AccountName] : "");
    if ((accountName.empty() || accountCheckBox_->IsChecked()) /*&& !transfer*/) {
        accountName = m_accountNameStr;
    }
    accountID = (m_QIFaccountsID.find(accountName) != m_QIFaccountsID.end() ? m_QIFaccountsID.at(accountName) : -1);
    if (accountID < 1)
    {
        msg = _("Transaction Account is incorrect");
        return false;
    }
    trx->ACCOUNTID = accountID;
    trx->TOACCOUNTID = (t.find(ToAccountName) != t.end()
        ? (m_QIFaccountsID.find(t[ToAccountName]) != m_QIFaccountsID.end()
            ? m_QIFaccountsID[t[ToAccountName]] : -1) : -1);
    if (trx->ACCOUNTID == trx->TOACCOUNTID && transfer)
    {
        msg = _("Transaction Account for transfer is incorrect");
        return false;
    }

    trx->TRANSACTIONNUMBER = (t.find(TransNumber) != t.end() ? t[TransNumber] : "");
    trx->NOTES = (t.find(Memo) != t.end() ? t[Memo] : "");

    wxString status = "";
    if (t.find(Status) != t.end())
    {
        wxString s = t[Status];
        if (s == "X" || s == "R")
            status = "R";
        /*else if (s == "*" || s == "c")
        {
            TODO: What does 'cleared' status mean?
            status = "c";
        }*/

    }
    trx->STATUS = status;

    int color_id = mmColorBtn_->GetColorId();
    trx->FOLLOWUPID = -1;
    if (colorCheckBox_->IsChecked() && color_id > 0 && color_id < 8)
        trx->FOLLOWUPID = color_id;

    const wxString value = mmTrimAmount(t.find(Amount) != t.end() ? t[Amount] : "", decimal_, ".");
    if (value.empty())
    {
        msg = _("Transaction Amount is incorrect");
        return false;
    }

    double amt;
    value.ToCDouble(&amt);

    trx->TRANSAMOUNT = fabs(amt);
    trx->TOTRANSAMOUNT = transfer ? amt : trx->TRANSAMOUNT;

    if (t.find(CategorySplit) != t.end())
    {
        Model_Splittransaction::Cache split;
        wxStringTokenizer token(t[CategorySplit], "\n");
        wxStringTokenizer amtToken(t.find(AmountSplit) != t.end() ? t[AmountSplit] : "", "\n");
        while (token.HasMoreTokens())
        {
            const wxString c = token.GetNextToken();
            if (m_QIFcategoryNames.find(c) == m_QIFcategoryNames.end()) return false;
            int categID = m_QIFcategoryNames[c].first;
            if (categID <= 0)
            {
                msg = _("Transaction Category is incorrect");
                return false;
            }
            Model_Splittransaction::Data* s = Model_Splittransaction::instance().create();
            s->CATEGID = categID;
            s->SUBCATEGID = m_QIFcategoryNames[c].second;

            wxString amtSplit = amtToken.GetNextToken();
            amtSplit = mmTrimAmount(amtSplit, decimal_, ".");
            double amount;
            amtSplit.ToCDouble(&amount);
            s->SPLITTRANSAMOUNT = (Model_Checking::is_deposit(trx) ? amount : -amount);
            s->TRANSID = trx->TRANSID;
            split.push_back(s);
        }
        trx->SUBCATEGID = m_splitDataSets.size();
        m_splitDataSets.push_back(split);
    }
    else
    {
        wxString categStr = (t.find(Category) != t.end() ? t.at(Category) : "");
        if (categStr.empty())
        {
            Model_Payee::Data* payee = Model_Payee::instance().get(trx->PAYEEID);
            if (payee)
            {
                trx->CATEGID = payee->CATEGID;
                trx->SUBCATEGID = payee->SUBCATEGID;
            }
            categStr = Model_Category::full_name(trx->CATEGID, trx->SUBCATEGID, ":");

            if (categStr.empty())
            {
                trx->CATEGID = (m_QIFcategoryNames[_("Unknown")].first);
                trx->SUBCATEGID = -1;
            }
        }
        else
        {
            trx->CATEGID = (m_QIFcategoryNames[categStr].first);
            trx->SUBCATEGID = (m_QIFcategoryNames[categStr].second);
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

int mmQIFImportDialog::getOrCreateAccounts()
{
    m_QIFaccountsID.clear();

    for (auto &item : m_QIFaccounts)
    {
        int accountID = -1;
        Model_Account::Data* acc = (accountNumberCheckBox_->IsChecked())
            ? Model_Account::instance().getByAccNum(item.first)
            : Model_Account::instance().get(item.first);

        if (!acc)
        {
            Model_Account::Data *account = Model_Account::instance().create();

            account->FAVORITEACCT = "TRUE";
            account->STATUS = Model_Account::all_status()[Model_Account::OPEN];

            const auto type = item.second.find(AccountType) != item.second.end() ? item.second.at(AccountType) : "";
            account->ACCOUNTTYPE = mmExportTransaction::mm_acc_type(type);
            //Model_Account::all_type()[Model_Account::CHECKING];
            account->ACCOUNTNAME = item.first;
            account->INITIALBAL = 0;

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
            wxString sMsg = wxString::Format(_("Added account: %s"), item.first);
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

    for (const auto &item : m_payee_names)
    {
        bool is_exists = false;
        wxString payee_name;
        for (const auto& payee : Model_Payee::instance().all())
        {
            if (item.CmpNoCase(payee.PAYEENAME) == 0)
            {
                is_exists = true;
                payee_name = payee.PAYEENAME;
                continue;
            }
        }

        if (!is_exists)
        {
            Model_Payee::Data* p = Model_Payee::instance().create();
            p->PAYEENAME = item;
            p->CATEGID = -1;
            p->SUBCATEGID = -1;
            wxString sMsg = wxString::Format(_("Added payee: %s"), item);
            log_field_->AppendText(wxString() << sMsg << "\n");
            int id = Model_Payee::instance().save(p);
            m_QIFpayeeNames[item] = id;
        }
        else
        {
            Model_Payee::Data* p = Model_Payee::instance().get(payee_name);
            m_QIFpayeeNames[item] = p->PAYEEID;
        }
    }

    Model_Payee::instance().ReleaseSavepoint();
}

void mmQIFImportDialog::getOrCreateCategories()
{
    Model_Category::Cache data_set;
    wxArrayString temp;
    for (const auto &item : m_QIFcategoryNames)
    {
        wxStringTokenizer token(item.first, ":");
const wxString categStr = token.GetNextToken();
Model_Category::Data* c = Model_Category::instance().get(categStr);
//if (c && std::find(data_set.begin(), data_set.end(), c) != data_set.end()) continue;
if (temp.Index(categStr) != wxNOT_FOUND) continue;

if (!c)
{
    c = Model_Category::instance().create();
    c->CATEGNAME = categStr;
    c->CATEGID = -1;
}
data_set.push_back(c);
temp.Add(categStr);
    }
    Model_Category::instance().save(data_set);

    Model_Subcategory::Cache sub_data_set;
    for (const auto& item : m_QIFcategoryNames)
    {
        wxStringTokenizer token(item.first, ":");
        const wxString categStr = token.GetNextToken();
        wxString subcategStr = token.GetNextToken();
        if (subcategStr.empty()) {
            continue;
        }

        if (subcategStr.Contains(":")) {
            subcategStr.Replace(":", "|");
        }

        Model_Category::Data* c = Model_Category::instance().get(categStr);
        wxASSERT(c);

        Model_Subcategory::Data* sc = Model_Subcategory::instance().get(subcategStr, c->CATEGID);
        if (!sc)
        {
            sc = Model_Subcategory::instance().create();
            sc->SUBCATEGNAME = subcategStr;
            sc->CATEGID = c->CATEGID;
        }
        sub_data_set.push_back(sc);
    }
    Model_Subcategory::instance().save(sub_data_set);

    for (const auto& item : m_QIFcategoryNames) {
        int subcategID = -1, categID = -1;
        wxStringTokenizer token(item.first, ":");
        const wxString categStr = token.GetNextToken();
        const wxString subcategStr = token.GetNextToken();
        const auto c = Model_Category::instance().get(categStr);
        if (c) {
            categID = c->CATEGID;
            const auto s = Model_Subcategory::instance().get(subcategStr, categID);
            if (s) subcategID = s->SUBCATEGID;
        }
        m_QIFcategoryNames[item.first] = std::make_pair(categID, subcategID);
    }
}

int mmQIFImportDialog::get_last_imported_acc()
{
    int accID = -1;
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

    wxFileName csv_file(file_name);
    if (csv_file.FileExists()) {
        m_FileNameStr = file_name;
        log_field_->ChangeValue("");
        mmReadQIFFile();
    }
}

void mmQIFImportDialog::save_file_name()
{
    Model_Setting::instance().Prepend("RECENT_QIF_FILES", m_FileNameStr, 10);
}

void mmQIFImportDialog::OnMenuSelected(wxCommandEvent& event)
{
    mmColorBtn_->Enable(false);
    colorCheckBox_->SetValue(false);
}