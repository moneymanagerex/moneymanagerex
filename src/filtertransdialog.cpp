/*******************************************************
Copyright (C) 2006 Madhan Kanagavel
Copyright (C) 2013 - 2022 Nikolay Akimov
Copyright (C) 2021, 2022 Mark Whalley (mark@ipx.co.uk)

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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "filtertransdialog.h"
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "categdialog.h"
#include "filtertransdialog.h"
#include "images_list.h"
#include "categdialog.h"
#include "constants.h"
#include "mmSimpleDialogs.h"
#include "paths.h"
#include "payeedialog.h"
#include "util.h"
#include "validators.h"

#include "model/allmodel.h"

#include <wx/valnum.h>

constexpr auto DATE_MAX = 253402214400   /* Dec 31, 9999 */;

static const wxString TRANSACTION_STATUSES[] =
{
    wxTRANSLATE("Unreconciled"),
    wxTRANSLATE("Reconciled"),
    wxTRANSLATE("Void"),
    wxTRANSLATE("Follow Up"),
    wxTRANSLATE("Duplicate"),
    wxTRANSLATE("All Except Reconciled")
};

static const wxString GROUPBY_OPTIONS[] =
{
    wxTRANSLATE("Account"),
    wxTRANSLATE("Payee"),
    wxTRANSLATE("Category")
};


wxIMPLEMENT_DYNAMIC_CLASS(mmFilterTransactionsDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmFilterTransactionsDialog, wxDialog)
EVT_CHAR_HOOK(mmFilterTransactionsDialog::OnComboKey)
EVT_CHECKBOX(wxID_ANY, mmFilterTransactionsDialog::OnCheckboxClick)
EVT_BUTTON(wxID_OK, mmFilterTransactionsDialog::OnButtonOkClick)
EVT_BUTTON(wxID_CLEAR, mmFilterTransactionsDialog::OnButtonClearClick)
EVT_BUTTON(ID_BTN_CUSTOMFIELDS, mmFilterTransactionsDialog::OnMoreFields)
EVT_MENU(wxID_ANY, mmFilterTransactionsDialog::OnMenuSelected)
EVT_DATE_CHANGED(wxID_ANY, mmFilterTransactionsDialog::OnDateChanged)
EVT_CHOICE(wxID_ANY, mmFilterTransactionsDialog::OnChoice)
EVT_BUTTON(wxID_CANCEL, mmFilterTransactionsDialog::OnButtonCancelClick)
EVT_CLOSE(mmFilterTransactionsDialog::OnQuit)
wxEND_EVENT_TABLE()

mmFilterTransactionsDialog::mmFilterTransactionsDialog()
{
}
mmFilterTransactionsDialog::~mmFilterTransactionsDialog()
{
    wxLogDebug("~mmFilterTransactionsDialog");
}

mmFilterTransactionsDialog::mmFilterTransactionsDialog(wxWindow* parent, int accountID, bool isReport, wxString selected)
    : isMultiAccount_(accountID == -1)
    , accountID_(accountID)
    , isReportMode_(isReport)
    , m_color_value(-1)
    , m_filter_key(isReport ? "TRANSACTIONS_FILTER" : "ALL_TRANSACTIONS_FILTER")
{
    this->SetFont(parent->GetFont());
    mmDoInitVariables();
    Create(parent);
    if (!selected.empty())
        m_settings_json = selected;

    mmDoDataToControls(m_settings_json);
}

mmFilterTransactionsDialog::mmFilterTransactionsDialog(wxWindow* parent, const wxString& json)
    : isMultiAccount_(true)
    , accountID_(-1)
    , isReportMode_(true)
    , m_filter_key("TRANSACTIONS_FILTER")
{
    this->SetFont(parent->GetFont());
    mmDoInitVariables();
    Create(parent);
    mmDoDataToControls(json);
}

void mmFilterTransactionsDialog::mmDoInitVariables()
{
    m_custom_fields = new mmCustomDataTransaction(this, NULL, ID_CUSTOMFIELDS + (isReportMode_ ? 100 : 0));

    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmToday()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmCurrentMonth()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmCurrentMonthToDate()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLastMonth()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLast30Days()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLast90Days()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLast3Months()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLast12Months()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmCurrentYear()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmCurrentYearToDate()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLastYear()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmCurrentFinancialYear()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmCurrentFinancialYearToDate()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLastFinancialYear()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLast365Days()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmAllTime()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmSinseToday()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmSinse30days()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmSinse90days()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmSinseCurrentYear()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmSinseCurrentFinancialYear()));

    m_accounts_name.clear();
    const auto accounts = Model_Account::instance().find(Model_Account::ACCOUNTTYPE(Model_Account::all_type()[Model_Account::INVESTMENT], NOT_EQUAL));
    for (const auto& acc : accounts) {
        m_accounts_name.push_back(acc.ACCOUNTNAME);
    }
    m_accounts_name.Sort();
}

bool mmFilterTransactionsDialog::Create(wxWindow* parent
    , wxWindowID id
    , const wxString& caption
    , const wxPoint& pos
    , const wxSize& size
    , long style)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    mmDoCreateControls();
    wxCommandEvent evt(wxEVT_CHECKBOX, wxID_ANY);
    AddPendingEvent(evt);

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    this->SetInitialSize();
    SetMinSize(wxSize(400, 480));
    SetIcon(mmex::getProgramIcon());

    Centre();
    return true;
}

int mmFilterTransactionsDialog::ShowModal()
{
    return wxDialog::ShowModal();
}

void mmFilterTransactionsDialog::mmDoDataToControls(const wxString& json)
{
    if (json.empty()) return;

    Document j_doc;
    if (j_doc.Parse(json.utf8_str()).HasParseError())
    {
        j_doc.Parse("{}");
    }

    //Label
    Value& j_label = GetValueByPointerWithDefault(j_doc, "/LABEL", "");
    wxString s_label = j_label.IsString() ? wxString::FromUTF8(j_label.GetString()) : "";
    m_setting_name->SetStringSelection(s_label);

    //Account
    m_selected_accounts_id.clear();
    Value& j_account = GetValueByPointerWithDefault(j_doc, "/ACCOUNT", "");
    if (isMultiAccount_ && j_account.IsArray())
    {
        wxString baloon = "";
        wxString acc_name;
        for (rapidjson::SizeType i = 0; i < j_account.Size(); i++)
        {
            wxASSERT(j_account[i].IsString());
            acc_name = wxString::FromUTF8(j_account[i].GetString());
            wxLogDebug("%s", acc_name);
            accountCheckBox_->SetValue(true);
            for (const auto& a : Model_Account::instance().find(Model_Account::ACCOUNTNAME(acc_name)))
            {
                m_selected_accounts_id.Add(a.ACCOUNTID);
                baloon += (baloon.empty() ? "" : "\n") + a.ACCOUNTNAME;
            }
        }
        if (m_selected_accounts_id.size() == 1)
            bSelectedAccounts_->SetLabelText(acc_name);
        else {
            mmToolTip(bSelectedAccounts_, baloon);
            bSelectedAccounts_->SetLabelText("...");
        }
    }
    else
    {
        Model_Account::Data* acc = Model_Account::instance().get(accountID_);
        accountCheckBox_->SetValue(acc);
        bSelectedAccounts_->SetLabelText(acc ? acc->ACCOUNTNAME : _("All"));
        bSelectedAccounts_->Enable(acc);
        m_selected_accounts_id.Add(accountID_);
    }

    //Dates
    const wxString& begin_date_str = wxString::FromUTF8(GetValueByPointerWithDefault(j_doc, "/DATE1", "").GetString());
    const wxString& end_date_str = wxString::FromUTF8(GetValueByPointerWithDefault(j_doc, "/DATE2", "").GetString());
    wxDateTime begin_date, end_date;
    bool is_begin_date_valid = mmParseISODate(begin_date_str, begin_date);
    bool is_end_date_valid = mmParseISODate(end_date_str, end_date);
    dateRangeCheckBox_->SetValue(is_begin_date_valid && is_end_date_valid);
    fromDateCtrl_->Enable(dateRangeCheckBox_->IsChecked());
    toDateControl_->Enable(dateRangeCheckBox_->IsChecked());
    fromDateCtrl_->SetValue(begin_date);
    toDateControl_->SetValue(end_date);
    if (dateRangeCheckBox_->IsChecked())
    {
        wxDateEvent date_event;
        date_event.SetId(wxID_FIRST);
        date_event.SetDate(begin_date);
        OnDateChanged(date_event);
        date_event.SetId(wxID_LAST);
        date_event.SetDate(end_date);
        OnDateChanged(date_event);
    }

    //Date Period Range
    Value& j_period = GetValueByPointerWithDefault(j_doc, "/PERIOD", "");
    const wxString& s_range = j_period.IsString() ? wxString::FromUTF8(j_period.GetString()) : "";
    rangeChoice_->SetStringSelection(wxGetTranslation(s_range));
    datesCheckBox_->SetValue(rangeChoice_->GetSelection() != wxNOT_FOUND && !s_range.empty());
    rangeChoice_->Enable(datesCheckBox_->IsChecked());
    if (datesCheckBox_->IsChecked())
    {
        wxCommandEvent evt(wxID_ANY, ID_DATE_RANGE);
        evt.SetInt(rangeChoice_->GetSelection());
        OnChoice(evt);
    }

    //Payee
    Value& j_payee = GetValueByPointerWithDefault(j_doc, "/PAYEE", "");
    const wxString& s_payee = j_payee.IsString() ? wxString::FromUTF8(j_payee.GetString()) : "";
    payeeCheckBox_->SetValue(!s_payee.empty());
    cbPayee_->Enable(payeeCheckBox_->IsChecked());
    cbPayee_->ChangeValue(s_payee);

    //Category
    Value& j_category = GetValueByPointerWithDefault(j_doc, "/CATEGORY", "");
    wxString s_category = j_category.IsString() ? wxString::FromUTF8(j_category.GetString()) : "";

    const wxString& delimiter = Model_Infotable::instance().GetStringInfo("CATEG_DELIMITER", ":");
    if (delimiter != ":" && s_category.Contains(":"))
    {
        wxStringTokenizer categ_token(s_category, ":", wxTOKEN_RET_EMPTY_ALL);
        const auto& categ_name = categ_token.GetNextToken();
        Model_Category::Data_Set categs = Model_Category::instance().all();
        for (const auto& categ : categs) {
            if (categ.CATEGNAME == categ_name) {
                s_category.Replace(categ_name + ":", categ_name + delimiter);
                break;
            }
        }
    }

    categoryCheckBox_->SetValue(!s_category.IsEmpty());
    categoryComboBox_->Enable(categoryCheckBox_->IsChecked());
    categoryComboBox_->SetLabelText(s_category);

    // Sub Category inclusion
    Value& j_categorySubCat = GetValueByPointerWithDefault(j_doc, "/SUBCATEGORYINCLUDE", "");
    bool subCatCheck = j_categorySubCat.IsBool() ? j_categorySubCat.GetBool() : false;
    categorySubCatCheckBox_->SetValue(subCatCheck);

    //Status
    Value& j_status = GetValueByPointerWithDefault(j_doc, "/STATUS", "");
    const wxString& s_status = j_status.IsString() ? wxString::FromUTF8(j_status.GetString()) : "";
    choiceStatus_->SetStringSelection(wxGetTranslation(s_status));
    statusCheckBox_->SetValue(choiceStatus_->GetSelection() != wxNOT_FOUND && !s_status.empty());
    choiceStatus_->Enable(statusCheckBox_->IsChecked());

    //Type
    Value& j_type = GetValueByPointerWithDefault(j_doc, "/TYPE", "");
    const wxString& s_type = j_type.IsString() ? wxString::FromUTF8(j_type.GetString()) : "";
    typeCheckBox_->SetValue(!s_type.empty());
    cbTypeWithdrawal_->SetValue(s_type.Contains("W"));
    cbTypeWithdrawal_->Enable(typeCheckBox_->IsChecked());
    cbTypeDeposit_->SetValue(s_type.Contains("D"));
    cbTypeDeposit_->Enable(typeCheckBox_->IsChecked());
    cbTypeTransferTo_->SetValue(s_type.Contains("T"));
    //cbTypeTransferTo_->Enable(typeCheckBox_->IsChecked());
    cbTypeTransferFrom_->SetValue(s_type.Contains("F"));
    //cbTypeTransferFrom_->Enable(typeCheckBox_->IsChecked());

    setTransferTypeCheckBoxes();

    bool amt1 = (j_doc.HasMember("AMOUNT_MIN") && j_doc["AMOUNT_MIN"].IsDouble());
    bool amt2 = (j_doc.HasMember("AMOUNT_MAX") && j_doc["AMOUNT_MAX"].IsDouble());

    amountRangeCheckBox_->SetValue(amt1 || amt2);
    amountMinEdit_->Enable(amt1);
    amountMaxEdit_->Enable(amt2);

    if (amt1) {
        amountMinEdit_->SetValue(j_doc["AMOUNT_MIN"].GetDouble());
    }
    else {
        amountMinEdit_->ChangeValue("");
    }

    if (amt2) {
        amountMaxEdit_->SetValue(j_doc["AMOUNT_MAX"].GetDouble());
    }
    else {
        amountMaxEdit_->ChangeValue("");
    }

    //Number
    wxString s_number;
    if (j_doc.HasMember("NUMBER") && j_doc["NUMBER"].IsString()) {
        transNumberCheckBox_->SetValue(true);
        Value& s = j_doc["NUMBER"];
        s_number = wxString::FromUTF8(s.GetString());
    }
    else {
        transNumberCheckBox_->SetValue(false);
    }
    transNumberEdit_->Enable(transNumberCheckBox_->IsChecked());
    transNumberEdit_->ChangeValue(s_number);

    //Notes
    wxString s_notes;
    if (j_doc.HasMember("NOTES") && j_doc["NOTES"].IsString()) {
        notesCheckBox_->SetValue(true);
        Value& s = j_doc["NOTES"];
        s_notes = wxString::FromUTF8(s.GetString());
    }
    else {
        notesCheckBox_->SetValue(false);
    }
    notesEdit_->Enable(notesCheckBox_->IsChecked());
    notesEdit_->ChangeValue(s_notes);

    //Colour
    m_color_value = -1;
    colorCheckBox_->SetValue(false);
    if (j_doc.HasMember("COLOR") && j_doc["COLOR"].IsInt()) {
        colorCheckBox_->SetValue(true);
        m_color_value = j_doc["COLOR"].GetInt();
    }
    colorButton_->Enable(colorCheckBox_->IsChecked());
    colorButton_->SetBackgroundColor(m_color_value);
    colorButton_->Refresh(); // Needed as setting the background color does not cause an immediate refresh

    //Custom Fields
    bool is_custom_found = false;
    const wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
    for (const auto& i : Model_CustomField::instance().find(Model_CustomField::DB_Table_CUSTOMFIELD_V1::REFTYPE(RefType)))
    {
        const auto entry = wxString::Format("CUSTOM%i", i.FIELDID);
        if (j_doc.HasMember(entry.c_str())) {
            const auto value = j_doc[const_cast<char*>(static_cast<const char*>(entry.mb_str()))].GetString();
            m_custom_fields->SetStringValue(i.FIELDID, value);
            is_custom_found = true;
        }
        else
        {
            m_custom_fields->SetStringValue(i.FIELDID, "");
        }
    }

    /*******************************************************
     Presentation Options
    *******************************************************/

    //Hide Columns
    Value& j_columns = GetValueByPointerWithDefault(j_doc, "/COLUMN", "");
    m_selected_columns_id.Clear();
    if (j_columns.IsArray())
    {
        for (rapidjson::SizeType i = 0; i < j_columns.Size(); i++)
        {
            wxASSERT(j_columns[i].IsInt());
            const int colID = j_columns[i].GetInt();

            m_selected_columns_id.Add(colID);
        }
        showColumnsCheckBox_->SetValue(true);
        bHideColumns_->SetLabelText("...");
    }
    else
    {
        showColumnsCheckBox_->SetValue(false);
        bHideColumns_->SetLabelText("");
    }
    bHideColumns_->Enable(showColumnsCheckBox_->IsChecked());

    //Group By
    Value& j_groupBy = GetValueByPointerWithDefault(j_doc, "/GROUPBY", "");
    const wxString& s_groupBy = j_groupBy.IsString() ? wxString::FromUTF8(j_groupBy.GetString()) : "";
    groupByCheckBox_->SetValue(!s_groupBy.empty());
    bGroupBy_->Enable(groupByCheckBox_->IsChecked() && isReportMode_);
    bGroupBy_->SetStringSelection(s_groupBy);

    if (is_custom_found) {
        m_custom_fields->ShowCustomPanel();
    }
}

void mmFilterTransactionsDialog::mmDoInitSettingNameChoice(wxString sel) const
{
    m_setting_name->Clear();
    if (isMultiAccount_)
    {
        wxArrayString filter_settings = Model_Infotable::instance().GetArrayStringSetting(m_filter_key, true);
        for (const auto& data : filter_settings)
        {
            Document j_doc;
            if (j_doc.Parse(data.utf8_str()).HasParseError()) {
                j_doc.Parse("{}");
            }

            Value& j_label = GetValueByPointerWithDefault(j_doc, "/LABEL", "");
            const wxString& s_label = j_label.IsString() ? wxString::FromUTF8(j_label.GetString()) : "";
            m_setting_name->Append(s_label, new wxStringClientData(data));
        }
    }
    else
    {
        Model_Account::Data* acc = Model_Account::instance().get(accountID_);
        wxString account_name = acc ? acc->ACCOUNTNAME : "";
        m_setting_name->Append(account_name, new wxStringClientData(account_name));
        sel = "";
    }

    if (m_setting_name->GetCount() > 0) {
        if (sel.empty())
            m_setting_name->SetSelection(0);
        else
            m_setting_name->SetStringSelection(sel);
    }
}

void mmFilterTransactionsDialog::mmDoCreateControls()
{
    wxBoxSizer* box_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* box_sizer1 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* box_sizer2 = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* custom_fields_box_sizer = new wxBoxSizer(wxVERTICAL);
    box_sizer->Add(box_sizer1, g_flagsExpand);
    box_sizer1->Add(box_sizer2, g_flagsExpand);
    box_sizer1->Add(custom_fields_box_sizer, g_flagsExpand);

    this->SetSizer(box_sizer);

    /******************************************************************************
     Items Panel
    *******************************************************************************/
    wxStaticBox* static_box_sizer = new wxStaticBox(this, wxID_ANY, _("Specify"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(static_box_sizer, wxVERTICAL);
    box_sizer2->Add(itemStaticBoxSizer4, 1, wxGROW | wxALL, 5);

    wxPanel* itemPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemStaticBoxSizer4->Add(itemPanel, g_flagsExpand);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* itemPanelSizer = new wxFlexGridSizer(0, 2, 0, 0);
    itemPanelSizer->AddGrowableCol(1, 1);

    itemPanel->SetSizer(itemBoxSizer4);
    itemBoxSizer4->Add(itemPanelSizer, g_flagsExpand);

    // Account
    accountCheckBox_ = new wxCheckBox(itemPanel, ID_ACCOUNT_CB, _("Account")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(accountCheckBox_, g_flagsH);

    bSelectedAccounts_ = new wxButton(itemPanel, wxID_STATIC, _("All"));
    bSelectedAccounts_->SetMinSize(wxSize(180, -1));
    bSelectedAccounts_->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmFilterTransactionsDialog::OnAccountsButton), nullptr, this);
    itemPanelSizer->Add(bSelectedAccounts_, g_flagsExpand);

    // Period Range
    datesCheckBox_ = new wxCheckBox(itemPanel, ID_PERIOD_CB, _("Period Range")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(datesCheckBox_, g_flagsH);

    rangeChoice_ = new wxChoice(itemPanel, ID_DATE_RANGE);
    rangeChoice_->SetName("DateRanges");
    for (const auto& date_range : m_all_date_ranges) {
        rangeChoice_->Append(date_range.get()->local_title());
    }
    itemPanelSizer->Add(rangeChoice_, g_flagsExpand);

    // Date Range
    dateRangeCheckBox_ = new wxCheckBox(itemPanel, ID_DATE_RANGE_CB, _("Date Range")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(dateRangeCheckBox_, g_flagsH);

    fromDateCtrl_ = new mmDatePickerCtrl(itemPanel, wxID_FIRST, wxDefaultDateTime
        , wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
    toDateControl_ = new mmDatePickerCtrl(itemPanel, wxID_LAST, wxDefaultDateTime
        , wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);

    wxBoxSizer* dateSizer = new wxBoxSizer(wxHORIZONTAL);
    dateSizer->Add(fromDateCtrl_, g_flagsExpand);
    dateSizer->AddSpacer(5);
    dateSizer->Add(toDateControl_, g_flagsExpand);
    itemPanelSizer->Add(dateSizer, wxSizerFlags(g_flagsExpand).Border(0));

    // Payee
    payeeCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Payee")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(payeeCheckBox_, g_flagsH);

    cbPayee_ = new mmComboBoxPayee(itemPanel, mmID_PAYEE);
    cbPayee_->SetMinSize(wxSize(220, -1));

    itemPanelSizer->Add(cbPayee_, g_flagsExpand);

    // Category
    categoryCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Category")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(categoryCheckBox_, g_flagsH);

    categoryComboBox_ = new mmComboBoxCategory(itemPanel, mmID_CATEGORY);
    categoryComboBox_->Bind(wxEVT_COMBOBOX, &mmFilterTransactionsDialog::OnCategoryChange, this);
    categoryComboBox_->Bind(wxEVT_KILL_FOCUS, &mmFilterTransactionsDialog::OnCategoryChange, this);
    itemPanelSizer->Add(categoryComboBox_, g_flagsExpand);

    // Category sub-category checkbox
    categorySubCatCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Include all sub-categories")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);

    itemPanelSizer->AddSpacer(1);
    itemPanelSizer->Add(categorySubCatCheckBox_, g_flagsExpand);

    // Status
    statusCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Status")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(statusCheckBox_, g_flagsH);

    choiceStatus_ = new wxChoice(itemPanel, wxID_ANY);

    for (const auto& i : TRANSACTION_STATUSES)
        choiceStatus_->Append(wxGetTranslation(i), new wxStringClientData(i));

    itemPanelSizer->Add(choiceStatus_, g_flagsExpand);
    mmToolTip(choiceStatus_, _("Specify the status for the transaction"));

    // Type
    typeCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Type")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);

    wxFlexGridSizer* typeSizer = new wxFlexGridSizer(0, 2, 0, 0);
    typeSizer->AddGrowableCol(0, 1);
    typeSizer->AddGrowableCol(1, 1);
    cbTypeWithdrawal_ = new wxCheckBox(itemPanel, wxID_ANY, _("Withdrawal")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cbTypeDeposit_ = new wxCheckBox(itemPanel, wxID_ANY, _("Deposit")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cbTypeTransferTo_ = new wxCheckBox(itemPanel, wxID_ANY, _("Transfer Out")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cbTypeTransferFrom_ = new wxCheckBox(itemPanel, wxID_ANY, _("Transfer In")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);

    itemPanelSizer->Add(typeCheckBox_, g_flagsH);
    itemPanelSizer->Add(typeSizer, g_flagsExpand);
    typeSizer->Add(cbTypeWithdrawal_, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    typeSizer->Add(cbTypeDeposit_, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    typeSizer->Add(cbTypeTransferTo_, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    typeSizer->Add(cbTypeTransferFrom_, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    typeSizer->AddSpacer(2);

    // Amount
    amountRangeCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Amount Range")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(amountRangeCheckBox_, g_flagsH);

    amountMinEdit_ = new mmTextCtrl(itemPanel, wxID_ANY, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER
        , mmCalcValidator());
    amountMinEdit_->Connect(wxID_ANY, wxEVT_COMMAND_TEXT_ENTER,
        wxCommandEventHandler(mmFilterTransactionsDialog::OnTextEntered), nullptr, this);
    amountMaxEdit_ = new mmTextCtrl(itemPanel, wxID_ANY, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER
        , mmCalcValidator());
    amountMaxEdit_->Connect(wxID_ANY, wxEVT_COMMAND_TEXT_ENTER,
        wxCommandEventHandler(mmFilterTransactionsDialog::OnTextEntered), nullptr, this);

    wxBoxSizer* amountSizer = new wxBoxSizer(wxHORIZONTAL);
    amountSizer->Add(amountMinEdit_, g_flagsExpand);
    amountSizer->Add(amountMaxEdit_, g_flagsExpand);
    itemPanelSizer->Add(amountSizer, wxSizerFlags(g_flagsExpand).Border(0));

    // Number
    transNumberCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Number")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(transNumberCheckBox_, g_flagsH);

    transNumberEdit_ = new wxTextCtrl(itemPanel, wxID_ANY);
    itemPanelSizer->Add(transNumberEdit_, g_flagsExpand);

    // Notes
    notesCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Notes")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(notesCheckBox_, g_flagsH);

    notesEdit_ = new wxTextCtrl(itemPanel, wxID_ANY);
    itemPanelSizer->Add(notesEdit_, g_flagsExpand);
    notesEdit_->SetHint("*");
    mmToolTip(notesEdit_,
        _("Enter any string to find it in transaction notes") + "\n\n" +
        _("Tips: You can use wildcard characters - question mark (?), asterisk (*) - in your search criteria.") + "\n" +
        _("Use the question mark (?) to find any single character - for example, s?t finds 'sat' and 'set'.") + "\n" +
        _("Use the asterisk (*) to find any number of characters - for example, s*d finds 'sad' and 'started'.") + "\n" +
        _("Use the asterisk (*) in the begin to find any string in the middle of the sentence.")
    );

    // Colour
    colorCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Color")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(colorCheckBox_, g_flagsH);

    colorButton_ = new mmColorButton(itemPanel, wxID_HIGHEST);
    itemPanelSizer->Add(colorButton_, g_flagsExpand);

    /******************************************************************************
     Presentation Panel
    *******************************************************************************/

    wxPanel* presPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    if (isReportMode_) {
        wxStaticBox* static_box_sizer_pres = new wxStaticBox(this, wxID_ANY, _("Presentation Options"));
        wxStaticBoxSizer* itemStaticBoxSizer_pres = new wxStaticBoxSizer(static_box_sizer_pres, wxVERTICAL);
        box_sizer2->Add(itemStaticBoxSizer_pres, wxSizerFlags(g_flagsExpand).Proportion(0));
        itemStaticBoxSizer_pres->Add(presPanel, g_flagsExpand);
    }

    wxBoxSizer* presBoxSizer = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* presPanelSizer = new wxFlexGridSizer(0, 2, 0, 0);
    presPanelSizer->AddGrowableCol(1, 1);

    presPanel->SetSizer(presBoxSizer);
    presBoxSizer->Add(presPanelSizer, g_flagsExpand);

    //Hide columns
    showColumnsCheckBox_ = new wxCheckBox(presPanel, wxID_ANY, _("Hide Columns")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    presPanelSizer->Add(showColumnsCheckBox_, g_flagsH);

    bHideColumns_ = new wxButton(presPanel, ID_DIALOG_COLUMNS, "");
    bHideColumns_->SetMinSize(wxSize(180, -1));
    bHideColumns_->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmFilterTransactionsDialog::OnShowColumnsButton), nullptr, this);
    presPanelSizer->Add(bHideColumns_, g_flagsExpand);

    //Group By
    groupByCheckBox_ = new wxCheckBox(presPanel, wxID_ANY, _("Group By")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    presPanelSizer->Add(groupByCheckBox_, g_flagsH);

    bGroupBy_ = new wxChoice(presPanel, wxID_ANY);
    for (const auto& i : GROUPBY_OPTIONS) {
        bGroupBy_->Append(wxGetTranslation(i), new wxStringClientData(i));
    }
    presPanelSizer->Add(bGroupBy_, g_flagsExpand);
    mmToolTip(bGroupBy_, _("Specify how the report should be grouped"));

    // Settings
    wxBoxSizer* settings_box_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* settings_sizer = new wxBoxSizer(wxVERTICAL);
    settings_sizer->Add(settings_box_sizer, wxSizerFlags(g_flagsExpand).Border(wxALL, 0));

    if (isMultiAccount_) {
        wxStaticText* settings = new wxStaticText(this, wxID_ANY, _("Settings"));
        settings_box_sizer->Add(settings, g_flagsH);
        settings_box_sizer->AddSpacer(5);
    }

    m_setting_name = new wxChoice(this, wxID_APPLY);
    settings_box_sizer->Add(m_setting_name, g_flagsExpand);
    mmDoInitSettingNameChoice();
    m_setting_name->Connect(wxID_APPLY, wxEVT_COMMAND_CHOICE_SELECTED
        , wxCommandEventHandler(mmFilterTransactionsDialog::OnSettingsSelected), nullptr, this);

    settings_box_sizer->AddSpacer(5);
    m_btnSaveAs = new wxBitmapButton(this, wxID_SAVEAS, mmBitmap(png::SAVE, mmBitmapButtonSize));
    settings_box_sizer->Add(m_btnSaveAs, g_flagsH);
    mmToolTip(m_btnSaveAs, _("Save active values into current Preset selection"));
    m_btnSaveAs->Connect(wxID_SAVEAS, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmFilterTransactionsDialog::OnSaveSettings), nullptr, this);

    m_itemButtonClear = new wxBitmapButton(this, wxID_CLEAR, mmBitmap(png::CLEAR, mmBitmapButtonSize));
    mmToolTip(m_itemButtonClear, _("Delete current Preset selection"));
    settings_box_sizer->Add(m_itemButtonClear, g_flagsH);

    box_sizer2->Add(settings_sizer, wxSizerFlags(g_flagsExpand).Border(wxALL, 0).Proportion(0));

    /******************************************************************************
     Button Panel with OK/Cancel buttons
    *******************************************************************************/
    wxPanel* button_panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    box_sizer2->Add(button_panel, wxSizerFlags(g_flagsV).Center());

    wxBoxSizer* button_sizer = new wxBoxSizer(wxHORIZONTAL);
    button_panel->SetSizer(button_sizer);

    wxButton* button_ok = new wxButton(button_panel, wxID_OK, _("&OK "));

    wxButton* button_cancel = new wxButton(button_panel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    button_cancel->SetFocus();

    wxBitmapButton* button_hide = new wxBitmapButton(button_panel
        , ID_BTN_CUSTOMFIELDS, mmBitmap(png::RIGHTARROW, mmBitmapButtonSize));
    mmToolTip(button_hide, _("Show/Hide custom fields window"));
    if (m_custom_fields->GetCustomFieldsCount() == 0) {
        button_hide->Hide();
    }

    button_sizer->Add(button_ok, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));
    button_sizer->Add(button_cancel, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));
    button_sizer->Add(button_hide, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));

    // Custom fields -----------------------------------

    m_custom_fields->FillCustomFields(custom_fields_box_sizer);
    auto cf_count = m_custom_fields->GetCustomFieldsCount();
    if (cf_count > 0) {
        wxCommandEvent evt(wxEVT_BUTTON, ID_BTN_CUSTOMFIELDS);
        OnMoreFields(evt);
    }

    if (!isMultiAccount_)
    {
        bSelectedAccounts_->Disable();
        bSelectedAccounts_->Hide();
        accountCheckBox_->Disable();
        accountCheckBox_->Hide();
        m_setting_name->Disable();
        m_itemButtonClear->Hide();
        m_btnSaveAs->Hide();
        m_setting_name->Hide();
    }
    if (!isReportMode_) {
        groupByCheckBox_->Disable();
        bGroupBy_->Disable();
        showColumnsCheckBox_->Disable();
        bHideColumns_->Disable();
        groupByCheckBox_->Hide();
        bGroupBy_->Hide();
        showColumnsCheckBox_->Hide();
        bHideColumns_->Hide();
    }
    Fit();

    wxCommandEvent e(wxID_APPLY);
    OnSettingsSelected(e);

    Center();
    this->SetSizer(box_sizer);

}

void mmFilterTransactionsDialog::setTransferTypeCheckBoxes()
{
    if (accountCheckBox_->IsChecked())
    {
        cbTypeTransferFrom_->Show();
        cbTypeTransferFrom_->Enable();
        cbTypeTransferTo_->SetLabel(_("Transfer Out"));
        Layout();
        bSelectedAccounts_->Enable(accountCheckBox_->IsEnabled());
    }
    else
    {
        m_selected_accounts_id.clear();
        bSelectedAccounts_->SetLabelText(_("All"));
        cbTypeTransferFrom_->Hide();
        cbTypeTransferFrom_->Disable();
        cbTypeTransferTo_->SetLabel(_("Transfer"));
        Layout();
        bSelectedAccounts_->Disable();
    }    
}

void mmFilterTransactionsDialog::OnCheckboxClick(wxCommandEvent& event)
{
    int id = event.GetId();
    switch (id)
    {
    case ID_PERIOD_CB:
        if (dateRangeCheckBox_->IsChecked())
            dateRangeCheckBox_->SetValue(false);
        break;
    case ID_DATE_RANGE_CB:
        if (datesCheckBox_->IsChecked())
            datesCheckBox_->SetValue(false);
        break;
    case ID_ACCOUNT_CB:
    {
        setTransferTypeCheckBoxes();
        break;
    }
    }

    cbPayee_->Enable(payeeCheckBox_->IsChecked());
    categoryComboBox_->Enable(categoryCheckBox_->IsChecked());
    categorySubCatCheckBox_->Enable(categoryCheckBox_->IsChecked()
            && (categoryComboBox_->mmGetCategoryId() != -1) && (categoryComboBox_->mmGetSubcategoryId() == -1));
    if (!categorySubCatCheckBox_->IsEnabled()) categorySubCatCheckBox_->SetValue(false);
    choiceStatus_->Enable(statusCheckBox_->IsChecked());
    cbTypeWithdrawal_->Enable(typeCheckBox_->IsChecked());
    cbTypeDeposit_->Enable(typeCheckBox_->IsChecked());
    cbTypeTransferTo_->Enable(typeCheckBox_->IsChecked());
    cbTypeTransferFrom_->Enable(typeCheckBox_->IsChecked());
    amountMinEdit_->Enable(amountRangeCheckBox_->IsChecked());
    amountMaxEdit_->Enable(amountRangeCheckBox_->IsChecked());
    notesEdit_->Enable(notesCheckBox_->IsChecked());
    transNumberEdit_->Enable(transNumberCheckBox_->IsChecked());
    rangeChoice_->Enable(datesCheckBox_->IsChecked());
    fromDateCtrl_->Enable(dateRangeCheckBox_->IsChecked());
    toDateControl_->Enable(dateRangeCheckBox_->IsChecked());
    colorButton_->Enable(colorCheckBox_->IsChecked());
    bHideColumns_->Enable(showColumnsCheckBox_->IsChecked());
    bGroupBy_->Enable(groupByCheckBox_->IsChecked() && isReportMode_);

    event.Skip();
}

bool mmFilterTransactionsDialog::mmIsValuesCorrect() const
{
    if (accountCheckBox_->IsChecked() && m_selected_accounts_id.empty()) {
        mmErrorDialogs::ToolTip4Object(bSelectedAccounts_, _("Account"), _("Invalid value"), wxICON_ERROR);
        return false;
    }

    if (dateRangeCheckBox_->IsChecked())
    {
        if (m_begin_date > m_end_date)
        {
            const auto today = wxDate::Today().FormatISODate();
            int id = m_begin_date >= today ? fromDateCtrl_->GetId() : toDateControl_->GetId();
            mmErrorDialogs::ToolTip4Object(FindWindow(id), _("Date"), _("Invalid value"), wxICON_ERROR);
            return false;
        }
    }

    if (datesCheckBox_->IsChecked() && rangeChoice_->GetSelection() == wxNOT_FOUND) {
        mmErrorDialogs::ToolTip4Object(rangeChoice_, _("Date"), _("Invalid value"), wxICON_ERROR);
        return false;
    }

    if (mmIsPayeeChecked())
    {
        bool ok = false;
        const auto& value = cbPayee_->mmGetPattern();

        if (value.empty()) {
            mmErrorDialogs::ToolTip4Object(categoryComboBox_, _("Empty value"), _("Payee"), wxICON_ERROR);
            return false;
        }

        wxRegEx pattern(value);
        if (pattern.IsValid())
        {
            pattern.Compile("^(" + value + ")$", wxRE_ICASE | wxRE_ADVANCED);
            Model_Payee::Data_Set payees = Model_Payee::instance().all();
            for (const auto& payee : payees)
            {
                if (pattern.Matches(payee.PAYEENAME)) {
                    ok = true;
                    break;
                }
            }
            if (ok == false) {
                if (wxMessageBox(wxString::Format(_("This name does not currently match any payees.\n"
                    "Do you want to continue to use it?\n%s"), value)
                    , _("Invalid value"), wxYES_NO | wxICON_INFORMATION) == wxNO)
                {
                    return false;
                }
            }
        }
        else
            return false;
    }

    if (mmIsCategoryChecked())
    {
        const auto& value = categoryComboBox_->GetValue();
        if (value.empty()) {
            mmErrorDialogs::ToolTip4Object(categoryComboBox_, _("Empty value"), _("Category"), wxICON_ERROR);
            return false;
        }
        wxRegEx pattern(value, wxRE_ADVANCED);
        if (!pattern.IsValid()) {
            return false;
        }
    }

    if (mmIsStatusChecked() && choiceStatus_->GetSelection() == wxNOT_FOUND) {
        mmErrorDialogs::ToolTip4Object(choiceStatus_, _("Invalid value"), _("Status"), wxICON_ERROR);
        return false;
    }

    if (mmIsTypeChecked() && mmGetTypes().empty()) {
        mmErrorDialogs::ToolTip4Object(cbTypeWithdrawal_, _("Invalid value"), _("Type"), wxICON_ERROR);
        return false;
    }

    if (amountRangeCheckBox_->IsChecked())
    {
        Model_Currency::Data* currency = Model_Currency::GetBaseCurrency();
        int currency_precision = Model_Currency::precision(currency);
        double min_amount = 0;

        if (!amountMinEdit_->Calculate(currency_precision))
        {
            amountMinEdit_->GetDouble(min_amount);
            mmErrorDialogs::ToolTip4Object(amountMinEdit_, _("Invalid value"), _("Amount"), wxICON_ERROR);
            return false;
        }

        if (!amountMaxEdit_->Calculate(currency_precision))
        {
            double max_amount = 0;
            amountMaxEdit_->GetDouble(max_amount);
            if (max_amount < min_amount) {
                mmErrorDialogs::ToolTip4Object(amountMaxEdit_, _("Invalid value"), _("Amount"), wxICON_ERROR);
                return false;
            }
        }
    }

    if (mmIsColorChecked())
    {
        if (m_color_value < 1 || m_color_value > 7) {
            mmErrorDialogs::ToolTip4Object(colorButton_, _("Invalid value"), _("Color"), wxICON_ERROR);
            return false;
        }
    }

    if (showColumnsCheckBox_->IsChecked())
    {
        if (m_selected_columns_id.empty()) {
            mmErrorDialogs::ToolTip4Object(bHideColumns_, _("Invalid value"), _("Hide Columns"), wxICON_ERROR);
            return false;
        }
    }

    if (groupByCheckBox_->IsChecked() && bGroupBy_->GetSelection() == wxNOT_FOUND) {
        mmErrorDialogs::ToolTip4Object(bGroupBy_, _("Invalid value"), _("Group By"), wxICON_ERROR);
        return false;
    }

    if (!m_custom_fields->ValidateCustomValues(NULL)) {
        return false;
    }

    return true;
}

void mmFilterTransactionsDialog::OnButtonOkClick(wxCommandEvent& /*event*/)
{
    if (mmIsValuesCorrect())
    {
        mmDoSaveSettings();
        EndModal(wxID_OK);
    }
}

void mmFilterTransactionsDialog::OnButtonCancelClick(wxCommandEvent& event)
{
#ifdef __WXMSW__
    wxWindow* w = FindFocus();
    if (w && w->GetId() != wxID_CANCEL)
        return;
#endif

    EndModal(wxID_CANCEL);
}

void mmFilterTransactionsDialog::OnQuit(wxCloseEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void mmFilterTransactionsDialog::OnShowColumnsButton(wxCommandEvent& /*event*/)
{
    wxArrayString column_names;
    column_names.Add("ID");
    column_names.Add("Color");
    column_names.Add("Date");
    column_names.Add("Number");
    column_names.Add("Account");
    column_names.Add("Payee");
    column_names.Add("Status");
    column_names.Add("Category");
    column_names.Add("Type");
    column_names.Add("Amount");
    column_names.Add("Notes");
    column_names.Add("UDFC01");
    column_names.Add("UDFC02");
    column_names.Add("UDFC03");
    column_names.Add("UDFC04");
    column_names.Add("UDFC05");


    mmMultiChoiceDialog s_col(this, _("Hide Report Columns"), "", column_names);
    s_col.SetSelections(m_selected_columns_id);

    wxString baloon = "";
    wxArrayInt selected_items;

    bHideColumns_->UnsetToolTip();

    if (s_col.ShowModal() == wxID_OK)
    {
        m_selected_columns_id.Clear();
        selected_items = s_col.GetSelections();
        for (const auto &entry : selected_items)
        {
            int index = entry;
            const wxString column_name = column_names[index];
            m_selected_columns_id.Add(index);
            baloon += wxGetTranslation(column_name) + "\n";
        }
    }

    if (m_selected_columns_id.GetCount() == 0)
    {
        bHideColumns_->SetLabelText("");
        showColumnsCheckBox_->SetValue(false);
        bHideColumns_->Disable();
    }
    else if (m_selected_columns_id.GetCount() > 0)
    {
        bHideColumns_->SetLabelText("...");
        mmToolTip(bHideColumns_, baloon);
    }
}

bool mmFilterTransactionsDialog::mmIsSomethingChecked() const
{
    return
        mmIsAccountChecked()
        || mmIsRangeChecked()
        || mmIsDateRangeChecked()
        || mmIsPayeeChecked()
        || mmIsCategoryChecked()
        || mmIsStatusChecked()
        || mmIsTypeChecked()
        || mmIsAmountRangeMinChecked()
        || mmIsAmountRangeMaxChecked()
        || mmIsNumberChecked()
        || mmIsNotesChecked()
        || mmIsColorChecked()
        || mmIsCustomFieldChecked();
}

const wxString mmFilterTransactionsDialog::mmGetStatus() const
{
    wxString status;
    wxStringClientData* status_obj =
        static_cast<wxStringClientData*>(choiceStatus_->GetClientObject(choiceStatus_->GetSelection()));
    if (status_obj) status = status_obj->GetData().Left(1);
    status.Replace("U", "");
    return status;
}

bool mmFilterTransactionsDialog::mmIsStatusMatches(const wxString& itemStatus) const
{
    wxString filterStatus = mmGetStatus();
    if (itemStatus == filterStatus)
    {
        return true;
    }
    else if ("A" == filterStatus) // All Except Reconciled
    {
        return "R" != itemStatus;
    }
    return false;
}

bool mmFilterTransactionsDialog::mmIsTypeMaches(const wxString& typeState, int accountid, int toaccountid) const
{
    bool result = false;
    if (typeState == Model_Checking::all_type()[Model_Checking::TRANSFER]
        && cbTypeTransferTo_->GetValue() 
        && (!mmIsAccountChecked() || (m_selected_accounts_id.Index(accountid) != wxNOT_FOUND)))
    {
        result = true;
    }
    else if (typeState == Model_Checking::all_type()[Model_Checking::TRANSFER]
        && cbTypeTransferFrom_->GetValue()
        && (!mmIsAccountChecked() || (m_selected_accounts_id.Index(toaccountid) != wxNOT_FOUND)))
    {
        result = true;
    }
    else if (typeState == Model_Checking::all_type()[Model_Checking::WITHDRAWAL] && cbTypeWithdrawal_->IsChecked())
    {
        result = true;
    }
    else if (typeState == Model_Checking::all_type()[Model_Checking::DEPOSIT] && cbTypeDeposit_->IsChecked())
    {
        result = true;
    }

    return result;
}

double mmFilterTransactionsDialog::mmGetAmountMin() const
{
    Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();

    wxString amountStr = amountMinEdit_->GetValue().Trim();
    double amount = 0;
    if (!Model_Currency::fromString(amountStr, amount, currency) || amount < 0)
        amount = 0;

    return amount;
}

double mmFilterTransactionsDialog::mmGetAmountMax() const
{
    Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();

    wxString amountStr = amountMaxEdit_->GetValue().Trim();
    double amount = 0;
    if (!Model_Currency::fromString(amountStr, amount, currency) || amount < 0)
        amount = 0;

    return amount;
}

void mmFilterTransactionsDialog::OnButtonClearClick(wxCommandEvent& /*event*/)
{
    int sel = m_setting_name->GetSelection();
    int size = m_setting_name->GetCount();
    if (sel >= 0 && size > 0)
    {
        if (wxMessageBox(
            _("The selected item will be deleted") + "\n\n" +
            _("Do you wish to continue?")
            , _("Settings item deletion"), wxYES_NO | wxICON_WARNING) == wxNO)
        {
            return;
        }

        int sel_json = Model_Infotable::instance().FindLabelInJSON(m_filter_key, mmGetLabelString());
        if (sel_json != wxNOT_FOUND)
            Model_Infotable::instance().Erase(m_filter_key, sel_json);

        m_setting_name->Delete(sel--);
        m_settings_json.clear();

        m_setting_name->SetSelection(sel < 0 ? 0 : sel);
        wxCommandEvent evt(wxID_APPLY);
        OnSettingsSelected(evt);
        mmDoDataToControls(m_settings_json);
    }
}

template<class MODEL, class DATA>
bool mmFilterTransactionsDialog::mmIsPayeeMatches(const DATA &tran)
{
    const Model_Payee::Data* payee = Model_Payee::instance().get(tran.PAYEEID);
    if (payee) {
        const wxString value = cbPayee_->mmGetPattern();
        if (!value.empty()) {
            wxRegEx pattern("^(" + value + ")$", wxRE_ICASE | wxRE_ADVANCED);
            if (pattern.IsValid() && pattern.Matches(payee->PAYEENAME)) {
                return true;
            }
        }
    }
    return false;
}

template<class MODEL, class DATA>
bool mmFilterTransactionsDialog::mmIsCategoryMatches(const DATA& tran, const std::map<int, typename MODEL::Split_Data_Set> & splits)
{
    wxArrayString trx_categories;
    const auto& it = splits.find(tran.id());
    if (it == splits.end()) {
        trx_categories.Add(Model_Category::full_name(tran.CATEGID, tran.SUBCATEGID));
    }
    else {
        for (const auto& split : it->second) {
            trx_categories.Add(Model_Category::full_name(split.CATEGID, split.SUBCATEGID));
        }
    }

    auto value = categoryComboBox_->mmGetPattern();

    if (!value.empty()) {
        if (mmIsCategorySubCatChecked()) value = value + ".*";
        for (const auto& item : trx_categories)
        {
            wxRegEx pattern("^(" + value + ")$", wxRE_ICASE | wxRE_ADVANCED);
            if (pattern.IsValid() && pattern.Matches(item)) {
                return true;
            }
        }
    }

    return false;
}

bool mmFilterTransactionsDialog::mmIsRecordMatches(const Model_Checking::Data &tran
    , const std::map<int, Model_Splittransaction::Data_Set>& split)
{
    bool ok = true;
    //wxLogDebug("Check date? %i trx date:%s %s %s", getDateRangeCheckBox(), tran.TRANSDATE, getFromDateCtrl().GetDateOnly().FormatISODate(), getToDateControl().GetDateOnly().FormatISODate());
    if (mmIsAccountChecked()
        && m_selected_accounts_id.Index(tran.ACCOUNTID) == wxNOT_FOUND
        && m_selected_accounts_id.Index(tran.TOACCOUNTID) == wxNOT_FOUND)
        ok = false;
    else if ((mmIsDateRangeChecked() || mmIsRangeChecked()) && (tran.TRANSDATE < m_begin_date || tran.TRANSDATE > m_end_date))
        ok = false;
    else if (mmIsPayeeChecked() && !mmIsPayeeMatches<Model_Checking>(tran)) ok = false;
    else if (mmIsCategoryChecked() && !mmIsCategoryMatches<Model_Checking>(tran, split)) ok = false;
    else if (mmIsStatusChecked() && !mmIsStatusMatches(tran.STATUS)) ok = false;
    else if (mmIsTypeChecked() && !mmIsTypeMaches(tran.TRANSCODE, tran.ACCOUNTID, tran.TOACCOUNTID)) ok = false;
    else if (mmIsAmountRangeMinChecked() && mmGetAmountMin() > tran.TRANSAMOUNT) ok = false;
    else if (mmIsAmountRangeMaxChecked() && mmGetAmountMax() < tran.TRANSAMOUNT) ok = false;
    else if (mmIsNumberChecked() && (mmGetNumber().empty() ? !tran.TRANSACTIONNUMBER.empty()
        : tran.TRANSACTIONNUMBER.empty() || !tran.TRANSACTIONNUMBER.Lower().Matches(mmGetNumber().Lower())))
        ok = false;
    else if (mmIsNotesChecked() && (mmGetNotes().empty() ? !tran.NOTES.empty()
        : tran.NOTES.empty() || !tran.NOTES.Lower().Matches(mmGetNotes().Lower())))
        ok = false;
    else if (mmIsColorChecked() && (m_color_value != tran.FOLLOWUPID))
        ok = false;
    else if (mmIsCustomFieldChecked() && !mmIsCustomFieldMatches(tran))
        ok = false;
    return ok;
}
bool mmFilterTransactionsDialog::mmIsRecordMatches(const Model_Billsdeposits::Data &tran, const std::map<int, Model_Budgetsplittransaction::Data_Set>& split)
{
    bool ok = true;
    if (mmIsAccountChecked()
            && m_selected_accounts_id.Index(tran.ACCOUNTID) == wxNOT_FOUND
            && m_selected_accounts_id.Index(tran.TOACCOUNTID) == wxNOT_FOUND)
        ok = false;
    else if ((mmIsDateRangeChecked() || mmIsRangeChecked()) && (tran.TRANSDATE < m_begin_date && tran.TRANSDATE > m_end_date))
        ok = false;
    else if (mmIsPayeeChecked() && !mmIsPayeeMatches<Model_Billsdeposits>(tran)) ok = false;
    else if (mmIsCategoryChecked() && !mmIsCategoryMatches<Model_Billsdeposits>(tran, split)) ok = false;
    else if (mmIsStatusChecked() && !mmIsStatusMatches(tran.STATUS)) ok = false;
    else if (mmIsTypeChecked() && !mmIsTypeMaches(tran.TRANSCODE, tran.ACCOUNTID, tran.TOACCOUNTID)) ok = false;
    else if (mmIsAmountRangeMinChecked() && mmGetAmountMin() > tran.TRANSAMOUNT) ok = false;
    else if (mmIsAmountRangeMaxChecked() && mmGetAmountMax() < tran.TRANSAMOUNT) ok = false;
    else if (mmIsNumberChecked() && (mmGetNumber().empty()
        ? !tran.TRANSACTIONNUMBER.empty()
        : tran.TRANSACTIONNUMBER.empty() || !tran.TRANSACTIONNUMBER.Lower().Matches(mmGetNumber().Lower())))
        ok = false;
    else if (mmIsNotesChecked() && (mmGetNotes().empty()
        ? !tran.NOTES.empty()
        : tran.NOTES.empty() || !tran.NOTES.Lower().Matches(mmGetNotes().Lower())))
        ok = false;
    return ok;
}

void mmFilterTransactionsDialog::OnTextEntered(wxCommandEvent& event)
{
    if (event.GetId() == amountMinEdit_->GetId())
        amountMinEdit_->Calculate();
    else if (event.GetId() == amountMaxEdit_->GetId())
        amountMaxEdit_->Calculate();
}

const wxString mmFilterTransactionsDialog::mmGetDescriptionToolTip() const
{
    wxString buffer;
    wxStringTokenizer token(mmGetJsonSetings(true), "\n");
    while (token.HasMoreTokens())
    {
        wxString c = token.GetNextToken().Trim();
        c.Replace(R"("")", _("Empty value"));
        c.Replace("\"", "");
        c.Replace("[", "");
        c.Replace("]", "");
        c.Replace("{", "");
        c.Replace("}", "");
        if (c.EndsWith(",")) c.RemoveLast(1);
        if (!c.empty()) buffer += c + "\n";
    }

    return buffer;
}

void mmFilterTransactionsDialog::mmGetDescription(mmHTMLBuilder &hb)
{
    hb.addHeader(4, _("Filtering Details: "));
    // Extract the parameters from the transaction dialog and add them to the report.
    wxString data = mmGetJsonSetings(true);
    Document j_doc;
    if (j_doc.Parse(data.utf8_str()).HasParseError()) {
        j_doc.Parse("{}");
    }

    wxString buffer;

    for (Value::ConstMemberIterator itr = j_doc.MemberBegin();
        itr != j_doc.MemberEnd(); ++itr)
    {
        const auto& name = wxGetTranslation(wxString::FromUTF8(itr->name.GetString()));
        switch (itr->value.GetType())
        {
        case kTrueType:
            buffer += wxString::Format("<kbd><samp><b>%s:</b> %s</samp></kbd>\n",
                name, L"\u2713");
            break;
        case kStringType:
        {
            wxString value = wxString::FromUTF8(itr->value.GetString());
            wxRegEx pattern_date(R"(^[0-9]{4}-[0-9]{2}-[0-9]{2}$)");
            wxRegEx pattern_type(R"(^(W?D?T?F?)$)");
            if (pattern_date.Matches(value))
            {
                wxDateTime dt;
                if (mmParseISODate(value, dt))
                    value = mmGetDateForDisplay(value);
            }
            else if (pattern_type.Matches(value))
            {
                wxString temp;
                if (value.Contains("W")) temp += (temp.empty() ? "" : ", ") + _("Withdrawal");
                if (value.Contains("D")) temp += (temp.empty() ? "" : ", ") + _("Deposit");
                if (value.Contains("F")) temp += (temp.empty() ? "" : ", ") + _("Transfer In");
                if (value.Contains("T")) temp += (temp.empty() ? "" : ", ")
                    + (mmGetAccountsID().empty() ? _("Transfer") : _("Transfer Out"));
                value = temp;
            }
            buffer += wxString::Format("<kbd><samp><b>%s:</b> %s</samp></kbd>\n",
                name, wxGetTranslation(value));
            break;
        }
        case kNumberType:
        {
            wxString temp;
            double d = itr->value.GetDouble();
            if (static_cast<int>(d) == d)
                temp = wxString::Format("%i", static_cast<int>(d));
            else
                temp = wxString::Format("%.2f", d);
            buffer += wxString::Format("<kbd><samp><b>%s:</b> %s</samp></kbd>\n", name, temp);
            break;
        }
        case kArrayType:
        {
            wxString temp;
            for (const auto& a : itr->value.GetArray()) {
                if (a.GetType() == kNumberType)
                    temp += (temp.empty() ? "" : ", ") + wxString::Format("%i", a.GetInt());
                else if (a.GetType() == kStringType)
                    temp += (temp.empty() ? "" : ", ") + wxString::FromUTF8(a.GetString());
            }
            buffer += wxString::Format("<kbd><samp><b>%s:</b> %s</samp></kbd>\n", name, temp);
            break;
        }
        default:
            break;
        }
    }

    hb.addText(buffer);
}

const wxString  mmFilterTransactionsDialog::mmGetTypes() const
{
    wxString type;
    if (cbTypeWithdrawal_->IsChecked()) type += "W";
    if (cbTypeDeposit_->IsChecked()) type += "D";
    if (cbTypeTransferTo_->IsChecked()) type += "T";
    if (cbTypeTransferFrom_->IsThisEnabled() && cbTypeTransferFrom_->IsChecked()) type += "F";
    return type;
}

const wxString mmFilterTransactionsDialog::mmGetJsonSetings(bool i18n) const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();

    //Label
    wxString label = mmGetLabelString();
    if (m_setting_name->GetSelection() == wxNOT_FOUND) {
        label = "";
    }

    if (!label.empty())
    {
        json_writer.Key((i18n ? _("Label") : "LABEL").utf8_str());
        json_writer.String(label.utf8_str());
    }

    //Account
    if (accountCheckBox_->IsChecked() && !m_selected_accounts_id.empty())
    {
        json_writer.Key((i18n ? _("Account") : "ACCOUNT").utf8_str());
        json_writer.StartArray();
        for (const auto& acc : m_selected_accounts_id)
        {
            Model_Account::Data* a = Model_Account::instance().get(acc);
            json_writer.String(a->ACCOUNTNAME.utf8_str());
        }
        json_writer.EndArray();
    }

    //Dates
    if (dateRangeCheckBox_->IsChecked())
    {
        json_writer.Key((i18n ? _("Since") : "DATE1").utf8_str());
        json_writer.String(fromDateCtrl_->GetValue().FormatISODate().utf8_str());
        json_writer.Key((i18n ? _("Before") : "DATE2").utf8_str());
        json_writer.String(toDateControl_->GetValue().FormatISODate().utf8_str());
    }

    //Date Period Range
    else if (datesCheckBox_->IsChecked())
    {
        int sel = rangeChoice_->GetSelection();
        if (sel != wxNOT_FOUND)
        {
            const wxSharedPtr<mmDateRange> date_range = m_all_date_ranges.at(sel);
            if (date_range) {
                json_writer.Key((i18n ? _("Period") : "PERIOD").utf8_str());
                json_writer.String(date_range->title().utf8_str());
            }
        }
    }

    //Payee
   if (payeeCheckBox_->IsChecked())
    {
        json_writer.Key((i18n ? _("Payee") : "PAYEE").utf8_str());
        json_writer.String(cbPayee_->GetValue().utf8_str());
    }

    //Category
    if (categoryCheckBox_->IsChecked())
    {
        json_writer.Key((i18n ? _("Category") : "CATEGORY").utf8_str());
        if (categoryComboBox_->mmIsValid())
        {
            int categ = categoryComboBox_->mmGetCategoryId();
            int subcateg = categoryComboBox_->mmGetSubcategoryId();
            const auto& full_name = Model_Category::full_name(categ, subcateg, ":");
            json_writer.String(full_name.utf8_str());
        } else {
            json_writer.String(categoryComboBox_->GetValue().utf8_str());
        }
    }

    // Sub Category inclusion
    if (categoryCheckBox_->IsChecked() 
        && (categoryComboBox_->mmGetCategoryId() != -1) && (categoryComboBox_->mmGetSubcategoryId() == -1))
    {
        json_writer.Key((i18n ? _("Include all sub-categories") : "SUBCATEGORYINCLUDE").utf8_str());
        json_writer.Bool(categorySubCatCheckBox_->GetValue());
    }

    //Status
    if (statusCheckBox_->IsChecked())
    {
        wxArrayString s = Model_Checking::all_status();
        s.Add(wxTRANSLATE("All Except Reconciled"));
        int item = choiceStatus_->GetSelection();
        wxString status;
        if (0 <= item && static_cast<size_t>(item) < s.size())
            status = s[item];
        if (!status.empty())
        {
            json_writer.Key((i18n ? _("Status") : "STATUS").utf8_str());
            json_writer.String((i18n ? wxGetTranslation(status) : status).utf8_str());
        }
    }

    //Type
    if (typeCheckBox_->IsChecked())
    {
        wxString type = mmGetTypes();
        if (!type.empty())
        {
            json_writer.Key((i18n ? _("Type") : "TYPE").utf8_str());
            json_writer.String(type.utf8_str());
        }
    }

    //Amounts
    if (amountRangeCheckBox_->IsChecked())
    {
        if (!amountMinEdit_->IsEmpty())
        {
            double amount_min;
            amountMinEdit_->GetDouble(amount_min);
            json_writer.Key((i18n ? _("Amount Min.") : "AMOUNT_MIN").utf8_str());
            json_writer.Double(amount_min);
        }

        if (!amountMaxEdit_->IsEmpty())
        {
            double amount_max;
            amountMaxEdit_->GetDouble(amount_max);
            json_writer.Key((i18n ? _("Amount Max.") : "AMOUNT_MAX").utf8_str());
            json_writer.Double(amount_max);
        }
    }

    //Number
    if (transNumberCheckBox_->IsChecked())
    {
        const wxString num = transNumberEdit_->GetValue();
        json_writer.Key((i18n ? _("Number") : "NUMBER").utf8_str());
        json_writer.String(num.utf8_str());
    }

    //Notes
    if (notesCheckBox_->IsChecked())
    {
        wxString notes = notesEdit_->GetValue();
        json_writer.Key((i18n ? _("Notes") : "NOTES").utf8_str());
        json_writer.String(notes.utf8_str());
    }

    //Colour
    if (colorCheckBox_->IsChecked())
    {
        json_writer.Key((i18n ? _("Color") : "COLOR").utf8_str());
        json_writer.Int(m_color_value);
    }

    //Custom Fields
    const auto cf = m_custom_fields->GetActiveCustomFields();
    if (cf.size() > 0)
    {
        for (const auto& i : cf)
        {
            if (!i.second.empty())
            {
                const auto field = Model_CustomField::instance().get(i.first);
                json_writer.Key(wxString::Format("CUSTOM%i", field->FIELDID).utf8_str());
                json_writer.String(i.second.utf8_str());
            }

        }
    }

    /*******************************************************
     Presentation Options
    *******************************************************/

    // Hide Columns
    if (showColumnsCheckBox_->IsChecked() && !m_selected_columns_id.empty())
    {
        json_writer.Key((i18n ? _("Hide Columns") : "COLUMN").utf8_str());
        json_writer.StartArray();
        for (const auto& acc : m_selected_columns_id)
            json_writer.Int(acc);
        json_writer.EndArray();
    }

    // Group By
    if (groupByCheckBox_->IsChecked())
    {
        const wxString groupBy = bGroupBy_->GetStringSelection();
        if (!groupBy.empty())
        {
            json_writer.Key((i18n ? _("Group By") : "GROUPBY").utf8_str());
            json_writer.String(groupBy.utf8_str());
        }
    }

    json_writer.EndObject();

    return wxString::FromUTF8(json_buffer.GetString());
}

void mmFilterTransactionsDialog::OnCategoryChange(wxEvent& event)
{
    categorySubCatCheckBox_->Enable(categoryCheckBox_->IsChecked()
            && (categoryComboBox_->mmGetCategoryId() != -1) && (categoryComboBox_->mmGetSubcategoryId() == -1));
    if (!categorySubCatCheckBox_->IsEnabled()) categorySubCatCheckBox_->SetValue(false);
    event.Skip();  
}

void mmFilterTransactionsDialog::OnDateChanged(wxDateEvent& event)
{
    switch (event.GetId())
    {
    case wxID_FIRST: m_begin_date = event.GetDate().FormatISODate(); break;
    case wxID_LAST: m_end_date = event.GetDate().FormatISODate(); break;
    }
}

bool mmFilterTransactionsDialog::mmIsAccountChecked() const
{
    return accountCheckBox_->GetValue() && !m_selected_accounts_id.empty();
}

bool mmFilterTransactionsDialog::mmIsAmountRangeMinChecked() const
{
    return amountRangeCheckBox_->GetValue() && !amountMinEdit_->GetValue().IsEmpty();
}

bool mmFilterTransactionsDialog::mmIsAmountRangeMaxChecked() const
{
    return amountRangeCheckBox_->GetValue() && !amountMaxEdit_->GetValue().IsEmpty();
}

bool mmFilterTransactionsDialog::mmIsCustomFieldChecked() const
{
    const auto cf = m_custom_fields->GetActiveCustomFields();
    return (cf.size() > 0);
}

bool mmFilterTransactionsDialog::mmIsCustomFieldMatches(const Model_Checking::Data& tran) const
{
    const auto cf = m_custom_fields->GetActiveCustomFields();
    int matched = 0;
    for (const auto& i : cf)
    {
        auto DataSet = Model_CustomFieldData::instance().find(Model_CustomFieldData::REFID(tran.TRANSID));
        for (const auto& j : DataSet)
        {
            if (i.first == j.FIELDID)
            {
                if (j.CONTENT.Matches(i.second))
                    matched += 1;
                else
                    return false;
            }
        }
    }
    return matched == static_cast<int>(cf.size());
}

int mmFilterTransactionsDialog::mmGetGroupBy() const
{
    int by = -1;
    if (groupByCheckBox_->IsChecked())
        by = bGroupBy_->GetSelection();
    return by;
}

void mmFilterTransactionsDialog::mmDoResetFilterStatus()
{
    //m_custom_fields->ResetWidgetsChanged();
}

void mmFilterTransactionsDialog::OnSettingsSelected(wxCommandEvent& event)
{
    if (!isMultiAccount_)
        return;
    int sel = event.GetSelection();
    int count = m_setting_name->GetCount();
    if (count > 0)
    {
        wxStringClientData* settings_obj =
            static_cast<wxStringClientData*>(m_setting_name->GetClientObject(sel));
        if (settings_obj)
            m_settings_json = settings_obj->GetData();

        mmDoDataToControls(m_settings_json);
    }
}

void mmFilterTransactionsDialog::mmDoUpdateSettings()
{
    if (isMultiAccount_)
    {
        int sel = m_setting_name->GetSelection();
        if (sel != wxNOT_FOUND)
        {
            sel = Model_Infotable::instance().FindLabelInJSON(m_filter_key, mmGetLabelString());
            if (sel != wxNOT_FOUND) {
                m_settings_json = mmGetJsonSetings();
                Model_Infotable::instance().Update(m_filter_key, sel, m_settings_json);
            }
        }
    }
    if (!isReportMode_)
    {
        Model_Infotable::instance().Set(wxString::Format("CHECK_FILTER_ID_%d", accountID_), mmGetJsonSetings());
    }
}

void mmFilterTransactionsDialog::mmDoSaveSettings(bool is_user_request)
{
    const auto label = mmGetLabelString();
    wxString user_label;

    if (is_user_request)
    {
        user_label = wxGetTextFromUser(_("Setting Name"), _("Please Enter"), label);

        if (user_label.empty())
            return;

        wxArrayString label_names;
        for (unsigned int i = 0; i < m_setting_name->GetCount(); i++) {
            label_names.Add(m_setting_name->GetString(i));
        }

        if (label_names.Index(user_label) == wxNOT_FOUND)
        {
            m_setting_name->Append(user_label);
            m_setting_name->SetStringSelection(user_label);
            m_settings_json = mmGetJsonSetings();
            Model_Infotable::instance().Prepend(m_filter_key, m_settings_json, -1);
        }
        else if (label == user_label)
        {
            mmDoUpdateSettings();
        }
        else
        {
            if (wxMessageBox(_("The entered name is already in use"), _("Warning"), wxOK | wxICON_WARNING) == wxOK)
            { }
        }
        mmDoInitSettingNameChoice(user_label);
    }
    else
    {
        const auto& filter_settings = Model_Infotable::instance().GetArrayStringSetting(m_filter_key);
        const auto& l = mmGetLabelString();
        int sel_json = Model_Infotable::instance().FindLabelInJSON(m_filter_key, l);
        const auto& json = sel_json != wxNOT_FOUND ? filter_settings[sel_json] : "";
        const auto& test = mmGetJsonSetings();
        if (isMultiAccount_ && json != test && !label.empty())
        {
            if (wxMessageBox(
                _("Filter settings have changed") + "\n" +
                _("Do you want to save them before continuing?") + "\n\n"
                , _("Please confirm"), wxYES_NO | wxICON_WARNING) == wxYES)
            {
                mmDoUpdateSettings();
            }
        }
    }
}

void mmFilterTransactionsDialog::OnSaveSettings(wxCommandEvent& WXUNUSED(event))
{
    if (mmIsValuesCorrect()) {
        mmDoSaveSettings(true);
    }
}

void mmFilterTransactionsDialog::OnAccountsButton(wxCommandEvent& WXUNUSED(event))
{
    mmMultiChoiceDialog s_acc(this, _("Choose Accounts"), "" , m_accounts_name);

    wxString baloon = "";
    wxArrayInt selected_items;

    for (const auto& acc : m_selected_accounts_id)
    {
        Model_Account::Data* a = Model_Account::instance().get(acc);
        if (a && m_accounts_name.Index(a->ACCOUNTNAME) != wxNOT_FOUND)
            selected_items.Add(m_accounts_name.Index(a->ACCOUNTNAME));
    }
    s_acc.SetSelections(selected_items);

    m_selected_accounts_id.Clear();
    bSelectedAccounts_->UnsetToolTip();

    if (s_acc.ShowModal() == wxID_OK)
    {
        selected_items = s_acc.GetSelections();
        for (const auto &entry : selected_items)
        {
            int index = entry;
            const wxString accounts_name = m_accounts_name[index];
            const auto account = Model_Account::instance().get(accounts_name);
            if (account) m_selected_accounts_id.Add(account->ACCOUNTID);
            baloon += accounts_name + "\n";
        }
    }

    if (m_selected_accounts_id.GetCount() == 0)
    {
        bSelectedAccounts_->SetLabelText(_("All"));
        accountCheckBox_->SetValue(false);
        bSelectedAccounts_->Disable();
    }
    else if (m_selected_accounts_id.GetCount() == 1)
    {
        const Model_Account::Data* account = Model_Account::instance().get(*m_selected_accounts_id.begin());
        if (account)
            bSelectedAccounts_->SetLabelText(account->ACCOUNTNAME);
    }
    else if (m_selected_accounts_id.GetCount() > 1)
    {
        bSelectedAccounts_->SetLabelText("...");
        mmToolTip(bSelectedAccounts_, baloon);
    }
}

void mmFilterTransactionsDialog::OnMoreFields(wxCommandEvent& WXUNUSED(event))
{
    wxBitmapButton* button = static_cast<wxBitmapButton*>(FindWindow(ID_BTN_CUSTOMFIELDS));

    if (button)
        button->SetBitmap(mmBitmap(m_custom_fields->IsCustomPanelShown() ? png::RIGHTARROW : png::LEFTARROW, mmBitmapButtonSize));

    m_custom_fields->ShowHideCustomPanel();

    this->SetMinSize(wxSize(0, 0));
    this->Fit();
}

void mmFilterTransactionsDialog::OnChoice(wxCommandEvent& event)
{
    int id = event.GetId();
    int sel = event.GetInt();

    switch (id)
    {
    case ID_DATE_RANGE:
    {
        if (sel != wxNOT_FOUND)
        {
            wxSharedPtr<mmDateRange> dates(m_all_date_ranges.at(sel));
            wxLogDebug("%s %s", dates->start_date().FormatISODate(), dates.get()->end_date().FormatISODate());
            m_begin_date = dates->start_date().FormatISODate();
            m_end_date = dates->end_date().FormatISODate();
            m_startDay = dates->startDay();
            m_futureIgnored = dates->isFutureIgnored();
            fromDateCtrl_->SetValue(dates->start_date());
            toDateControl_->SetValue(dates->end_date());
        }
        break;
    }
    }
}

void mmFilterTransactionsDialog::OnMenuSelected(wxCommandEvent& event)
{
    m_color_value = colorButton_->GetColorId();
    if (!m_color_value) {
        colorButton_->Enable(false);
        colorCheckBox_->SetValue(false);
        colorButton_->SetLabel("");
    }
}

void mmFilterTransactionsDialog::OnComboKey(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_RETURN)
    {
        auto id = event.GetId();
        switch (id)
        {
        case mmID_PAYEE:
        {
            const auto payeeName = cbPayee_->GetValue();
            if (payeeName.empty())
            {
                mmPayeeDialog dlg(this, true);
                dlg.ShowModal();
                cbPayee_->mmDoReInitialize();
                int payee_id = dlg.getPayeeId();
                Model_Payee::Data* payee = Model_Payee::instance().get(payee_id);
                if (payee) {
                    cbPayee_->ChangeValue(payee->PAYEENAME);
                    cbPayee_->SelectAll();
                }
                return;
            }
        }
        break;
        case mmID_CATEGORY:
        {
            auto category = categoryComboBox_->GetValue();
            if (category.empty())
            {
                mmCategDialog dlg(this, true, -1, -1);
                dlg.ShowModal();
                categoryComboBox_->mmDoReInitialize();
                category = Model_Category::full_name(dlg.getCategId(), dlg.getSubCategId());
                categoryComboBox_->ChangeValue(category);
                categoryComboBox_->SelectAll();
                return;
            }
        }
        break;
        default:
            break;
        }
    }

    event.Skip();
}
