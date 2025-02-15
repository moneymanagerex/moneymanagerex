/*******************************************************
Copyright (C) 2006 Madhan Kanagavel
Copyright (C) 2013 - 2022 Nikolay Akimov
Copyright (C) 2021 - 2023 Mark Whalley (mark@ipx.co.uk)

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

#include "filtertransdialog.h"
#include "categdialog.h"
#include "constants.h"
#include "images_list.h"
#include "mmSimpleDialogs.h"
#include "paths.h"
#include "payeedialog.h"
#include "util.h"
#include "validators.h"

#include "model/allmodel.h"
#include <wx/display.h>
#include <wx/regex.h>
#include <wx/valnum.h>

static const wxString COLUMN_NAMES[] = { "ID", "Color", "Date", "Number", "Account", "Payee", "Status", "Category", "Type", "Amount",
                                         "Notes", "UDFC01", "UDFC02", "UDFC03", "UDFC04", "UDFC05", "Tags", "FX Rate", "Time" };

static const wxString TRANSACTION_STATUSES[] = { wxTRANSLATE("Unreconciled"), wxTRANSLATE("Reconciled"), wxTRANSLATE("Void"),
                                                 wxTRANSLATE("Follow Up"),    wxTRANSLATE("Duplicate"),  wxTRANSLATE("All Except Reconciled") };

static const wxString GROUPBY_OPTIONS[] = { wxTRANSLATE("Account"), wxTRANSLATE("Payee"), wxTRANSLATE("Category"), wxTRANSLATE("Type"),
                                            wxTRANSLATE("Day"),     wxTRANSLATE("Month"), wxTRANSLATE("Year") };

static const wxString CHART_OPTIONS[] = { wxTRANSLATE("Bar"), wxTRANSLATE("Line"), wxTRANSLATE("Line DateTime"),
                                            wxTRANSLATE("Pie"),    wxTRANSLATE("Donut"), wxTRANSLATE("Radar"),
                                            wxTRANSLATE("Bar Line"), wxTRANSLATE("Stacked Bar Line"), wxTRANSLATE("Stacked Area")};
// Keep options aligned with HtmlBuilder GraphData::type

// Used to determine if we need to refresh the tag text ctrl after
// accelerator hints are shown which only occurs once.
static bool altRefreshDone;

wxIMPLEMENT_DYNAMIC_CLASS(mmFilterTransactionsDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmFilterTransactionsDialog, wxDialog)
EVT_CHAR_HOOK(mmFilterTransactionsDialog::OnComboKey)
EVT_CHECKBOX(wxID_ANY, mmFilterTransactionsDialog::OnCheckboxClick)
EVT_BUTTON(wxID_OK, mmFilterTransactionsDialog::OnButtonOkClick)
EVT_BUTTON(wxID_CLEAR, mmFilterTransactionsDialog::OnButtonClearClick)
EVT_BUTTON(ID_BTN_CUSTOMFIELDS, mmFilterTransactionsDialog::OnMoreFields)
EVT_MENU(wxID_ANY, mmFilterTransactionsDialog::OnMenuSelected)
EVT_DATE_CHANGED(wxID_ANY, mmFilterTransactionsDialog::OnDateChanged)
EVT_TIME_CHANGED(wxID_ANY, mmFilterTransactionsDialog::OnDateChanged)
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
    if (isReportMode_)
        Model_Infotable::instance().setSize("TRANSACTION_FILTER_SIZE", GetSize());
}

mmFilterTransactionsDialog::mmFilterTransactionsDialog(wxWindow* parent, int64 accountID, bool isReport, wxString selected)
    : isMultiAccount_(accountID == -1), accountID_(accountID), isReportMode_(isReport),
      m_filter_key(isReport ? "TRANSACTIONS_FILTER" : "ALL_TRANSACTIONS_FILTER")
{
    this->SetFont(parent->GetFont());
    mmDoInitVariables();
    Create(parent);
    if (!selected.empty())
        m_settings_json = selected;

    mmDoDataToControls(m_settings_json);
}

mmFilterTransactionsDialog::mmFilterTransactionsDialog(wxWindow* parent, const wxString& json)
    : isMultiAccount_(true), accountID_(-1), isReportMode_(true), m_filter_key("TRANSACTIONS_FILTER")
{
    this->SetFont(parent->GetFont());
    mmDoInitVariables();
    Create(parent);
    mmDoDataToControls(json);
}

void mmFilterTransactionsDialog::mmDoInitVariables()
{
    m_custom_fields = new mmCustomDataTransaction(this, 0, ID_CUSTOMFIELDS + (isReportMode_ ? 100 : 0));

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
    const auto accounts = Model_Account::instance().find(
        Model_Account::ACCOUNTTYPE(Model_Account::TYPE_STR_INVESTMENT, NOT_EQUAL));
    for (const auto& acc : accounts)
    {
        m_accounts_name.push_back(acc.ACCOUNTNAME);
    }
    m_accounts_name.Sort();
}

bool mmFilterTransactionsDialog::Create(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style,
                                        const wxString& name)
{
    altRefreshDone = false; // reset the ALT refresh indicator on new dialog creation
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style | wxRESIZE_BORDER, name);

    mmDoCreateControls();
    wxCommandEvent evt(wxEVT_CHECKBOX, wxID_ANY);
    AddPendingEvent(evt);
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
    Document j_doc;
    if (j_doc.Parse(json.utf8_str()).HasParseError())
    {
        j_doc.Parse("{}");
    }

    // Label
    Value& j_label = GetValueByPointerWithDefault(j_doc, "/LABEL", "");
    wxString s_label = j_label.IsString() ? wxString::FromUTF8(j_label.GetString()) : "";
    m_setting_name->SetStringSelection(s_label);

    // Account
    m_selected_accounts_id.clear();
    Value& j_account = GetValueByPointerWithDefault(j_doc, "/ACCOUNT", "");
    if (isMultiAccount_)
    {
        if (j_account.IsArray())
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
                    m_selected_accounts_id.push_back(a.ACCOUNTID);
                    baloon += (baloon.empty() ? "" : "\n") + a.ACCOUNTNAME;
                }
            }
            if (m_selected_accounts_id.size() == 1)
                bSelectedAccounts_->SetLabelText(acc_name);
            else
            {
                mmToolTip(bSelectedAccounts_, baloon);
                bSelectedAccounts_->SetLabelText("...");
            }
        }

        // If no accounts are explicitly selected, turn off the Account filter and set selection to "All"
        if (m_selected_accounts_id.empty())
        {
            bSelectedAccounts_->SetLabelText(_("All"));
            accountCheckBox_->SetValue(false);
            bSelectedAccounts_->Disable();
        }
    }

    // Dates
    const wxString begin_date_str = wxString::FromUTF8(GetValueByPointerWithDefault(j_doc, "/DATE1", "").GetString());
    wxString end_date_str = wxString::FromUTF8(GetValueByPointerWithDefault(j_doc, "/DATE2", "").GetString());

    // Default end date to end of today
    if (end_date_str.IsEmpty())
        end_date_str = wxDateTime(23, 59, 59, 999).FormatISOCombined();

    wxDateTime begin_date, end_date;
    bool is_begin_date_valid = mmParseISODate(begin_date_str, begin_date);
    bool is_end_date_valid = mmParseISODate(end_date_str, end_date);
    dateRangeCheckBox_->SetValue(is_begin_date_valid && is_end_date_valid);
    fromDateCtrl_->Enable(dateRangeCheckBox_->IsChecked());
    toDateControl_->Enable(dateRangeCheckBox_->IsChecked());
    fromDateCtrl_->SetValue(begin_date);
    toDateControl_->SetValue(end_date);

    wxDateEvent date_event;
    date_event.SetId(wxID_FIRST);
    date_event.SetDate(begin_date);
    OnDateChanged(date_event);
    date_event.SetId(wxID_LAST);
    date_event.SetDate(end_date);
    OnDateChanged(date_event);

    // Date Period Range
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

    // Payee
    Value& j_payee = GetValueByPointerWithDefault(j_doc, "/PAYEE", "");
    const wxString& s_payee = j_payee.IsString() ? wxString::FromUTF8(j_payee.GetString()) : "";
    payeeCheckBox_->SetValue(!s_payee.empty());
    cbPayee_->Enable(payeeCheckBox_->IsChecked());
    cbPayee_->ChangeValue(s_payee);

    // Category
    Value& j_category = GetValueByPointerWithDefault(j_doc, "/CATEGORY", "");
    wxString s_category = j_category.IsString() ? wxString::FromUTF8(j_category.GetString()) : "";

    const wxString& delimiter = Model_Infotable::instance().getString("CATEG_DELIMITER", ":");
    if (delimiter != ":" && s_category.Contains(":"))
    {
        for (const auto& category : Model_Category::all_categories())
        {
            wxString full_name = category.first;
            wxRegEx regex(delimiter);
            regex.Replace(&full_name, ":");
            if (s_category == full_name)
            {
                s_category = category.first;
                break;
            }
        }
    }

    categoryCheckBox_->SetValue(!s_category.IsEmpty());
    categoryComboBox_->Enable(categoryCheckBox_->IsChecked());
    categoryComboBox_->ChangeValue(s_category);

    // Sub Category inclusion
    Value& j_categorySubCat = GetValueByPointerWithDefault(j_doc, "/SUBCATEGORYINCLUDE", "");
    bool subCatCheck = j_categorySubCat.IsBool() ? j_categorySubCat.GetBool() : false;
    categorySubCatCheckBox_->SetValue(subCatCheck);
    categorySubCatCheckBox_->Enable(categoryCheckBox_->IsChecked());

    wxCommandEvent evt(wxEVT_COMBOBOX, mmID_CATEGORY);
    OnCategoryChange(evt);

    // Status
    Value& j_status = GetValueByPointerWithDefault(j_doc, "/STATUS", "");
    const wxString& s_status = j_status.IsString() ? wxString::FromUTF8(j_status.GetString()) : "";
    choiceStatus_->SetStringSelection(wxGetTranslation(s_status));
    statusCheckBox_->SetValue(choiceStatus_->GetSelection() != wxNOT_FOUND && !s_status.empty());
    choiceStatus_->Enable(statusCheckBox_->IsChecked());

    // Type
    Value& j_type = GetValueByPointerWithDefault(j_doc, "/TYPE", "");
    const wxString& s_type = j_type.IsString() ? wxString::FromUTF8(j_type.GetString()) : "";
    typeCheckBox_->SetValue(!s_type.empty());
    cbTypeWithdrawal_->SetValue(s_type.Contains("W"));
    cbTypeWithdrawal_->Enable(typeCheckBox_->IsChecked());
    cbTypeDeposit_->SetValue(s_type.Contains("D"));
    cbTypeDeposit_->Enable(typeCheckBox_->IsChecked());
    cbTypeTransferTo_->SetValue(s_type.Contains("T"));
    // cbTypeTransferTo_->Enable(typeCheckBox_->IsChecked());
    cbTypeTransferFrom_->SetValue(s_type.Contains("F"));
    // cbTypeTransferFrom_->Enable(typeCheckBox_->IsChecked());

    setTransferTypeCheckBoxes();

    bool amt1 = (j_doc.HasMember("AMOUNT_MIN") && j_doc["AMOUNT_MIN"].IsDouble());
    bool amt2 = (j_doc.HasMember("AMOUNT_MAX") && j_doc["AMOUNT_MAX"].IsDouble());

    amountRangeCheckBox_->SetValue(amt1 || amt2);
    amountMinEdit_->Enable(amt1);
    amountMaxEdit_->Enable(amt2);

    if (amt1)
    {
        amountMinEdit_->SetValue(j_doc["AMOUNT_MIN"].GetDouble());
    }
    else
    {
        amountMinEdit_->ChangeValue("");
    }

    if (amt2)
    {
        amountMaxEdit_->SetValue(j_doc["AMOUNT_MAX"].GetDouble());
    }
    else
    {
        amountMaxEdit_->ChangeValue("");
    }

    // Number
    wxString s_number;
    if (j_doc.HasMember("NUMBER") && j_doc["NUMBER"].IsString())
    {
        transNumberCheckBox_->SetValue(true);
        Value& s = j_doc["NUMBER"];
        s_number = wxString::FromUTF8(s.GetString());
    }
    else
    {
        transNumberCheckBox_->SetValue(false);
    }
    transNumberEdit_->Enable(transNumberCheckBox_->IsChecked());
    transNumberEdit_->ChangeValue(s_number);

    // Tags
    wxString s_tag;
    Value& j_tags = GetValueByPointerWithDefault(j_doc, "/TAGS", "");
    if (j_tags.IsArray())
    {
        for (rapidjson::SizeType i = 0; i < j_tags.Size(); i++)
        {
            if (j_tags[i].IsInt64())
            {
                // Retrieve TAGNAME from TAGID
                Model_Tag::Data* tag = Model_Tag::instance().get(int64(j_tags[i].GetInt64()));
                if (tag)
                {
                    s_tag.Append(tag->TAGNAME + " ");
                }
            }
            else
            {
                s_tag.Append(wxString(j_tags[i].GetString()).Append(" ").Prepend(" "));
            }
        }
        tagTextCtrl_->SetText(s_tag);
        tagTextCtrl_->Validate();
        tagCheckBox_->SetValue(true);
    }
    else
        tagCheckBox_->SetValue(false);
    tagTextCtrl_->Enable(tagCheckBox_->IsChecked());

    // Notes
    wxString s_notes;
    if (j_doc.HasMember("NOTES") && j_doc["NOTES"].IsString())
    {
        notesCheckBox_->SetValue(true);
        Value& s = j_doc["NOTES"];
        s_notes = wxString::FromUTF8(s.GetString());
    }
    else
    {
        notesCheckBox_->SetValue(false);
    }
    notesEdit_->Enable(notesCheckBox_->IsChecked());
    notesEdit_->ChangeValue(s_notes);

    // Colour
    m_color_value = -1;
    colorCheckBox_->SetValue(false);
    if (j_doc.HasMember("COLOR") && j_doc["COLOR"].IsInt())
    {
        colorCheckBox_->SetValue(true);
        m_color_value = j_doc["COLOR"].GetInt();
    }
    colorButton_->Enable(colorCheckBox_->IsChecked());
    colorButton_->SetBackgroundColor(m_color_value);
    colorButton_->Refresh(); // Needed as setting the background color does not cause an immediate refresh

    // Custom Fields
    bool is_custom_found = false;
    const wxString RefType = Model_Attachment::REFTYPE_STR_TRANSACTION;
    for (const auto& i : Model_CustomField::instance().find(Model_CustomField::DB_Table_CUSTOMFIELD_V1::REFTYPE(RefType)))
    {
        const auto entry = wxString::Format("CUSTOM%lld", i.FIELDID);
        if (j_doc.HasMember(entry.c_str()))
        {
            const auto value = j_doc[const_cast<char*>(static_cast<const char*>(entry.mb_str()))].GetString();
            m_custom_fields->SetStringValue(i.FIELDID, value, true);
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

    // Hide Columns
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

    // Remove the time column from the hidden columns list if not enabled in options
    if (!Option::instance().UseTransDateTime() && m_selected_columns_id.Index(COL_TIME) != wxNOT_FOUND)
    {
        m_selected_columns_id.Remove(COL_TIME);
        if (m_selected_columns_id.IsEmpty())
        {
            showColumnsCheckBox_->SetValue(false);
            bHideColumns_->SetLabelText("");
        }
    }

    bHideColumns_->Enable(showColumnsCheckBox_->IsChecked());

    // Group By
    Value& j_groupBy = GetValueByPointerWithDefault(j_doc, "/GROUPBY", "");
    const wxString& s_groupBy = j_groupBy.IsString() ? wxString::FromUTF8(j_groupBy.GetString()) : "";
    groupByCheckBox_->SetValue(!s_groupBy.empty());
    bGroupBy_->Enable(groupByCheckBox_->IsChecked() && isReportMode_);
    bGroupBy_->SetStringSelection(s_groupBy);

    // Chart
    Value& j_chart = GetValueByPointerWithDefault(j_doc, "/CHART", "");
    const wxString& s_chart = j_chart.IsString() ? wxString::FromUTF8(j_chart.GetString()) : "";
    chartCheckBox_->SetValue(!s_chart.empty());
    bChart_->Enable(chartCheckBox_->IsChecked() && isReportMode_);
    bChart_->SetStringSelection(s_chart);

    // Combine splits
    Value& j_combineSplits = GetValueByPointerWithDefault(j_doc, "/COMBINE_SPLITS", "");
    const bool& b_combineSplits = j_combineSplits.IsBool() ? j_combineSplits.GetBool() : false;
    combineSplitsCheckBox_->SetValue(b_combineSplits);

    if (is_custom_found)
    {
        m_custom_fields->ShowCustomPanel();
    }
}

void mmFilterTransactionsDialog::mmDoInitSettingNameChoice(wxString sel) const
{
    m_setting_name->Clear();

    if (isMultiAccount_)
    {
        // Add a blank setting at the beginning. This clears all selections if the user chooses it.
        m_setting_name->Append("", new wxStringClientData("{}"));
        // Add the 'Last Used' setting which was the last setting used that wasn't saved
        m_setting_name->Append(_("Last Unsaved Filter"), new wxStringClientData(Model_Infotable::instance().getString(m_filter_key + "_LAST_USED", "")));
        wxArrayString filter_settings = Model_Infotable::instance().getArrayString(m_filter_key, true);
        for (const auto& data : filter_settings)
        {
            Document j_doc;
            if (j_doc.Parse(data.utf8_str()).HasParseError())
            {
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

    if (m_setting_name->GetCount() > 0)
    {
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
    box_sizer2->Add(itemStaticBoxSizer4, g_flagsExpand);

    wxScrolledWindow* scroll_window = new wxScrolledWindow(static_box_sizer, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL);
    scroll_window->SetScrollRate(5, 5);
    itemStaticBoxSizer4->Add(scroll_window, 0, wxEXPAND | wxALL, 0);
    wxBoxSizer* scrollWindowSizer = new wxBoxSizer(wxVERTICAL);
    scroll_window->SetSizer(scrollWindowSizer);

    wxPanel* itemPanel = new wxPanel(scroll_window, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    scrollWindowSizer->Add(itemPanel, 0, wxEXPAND | wxALL, 10);

    wxFlexGridSizer* itemPanelSizer = new wxFlexGridSizer(0, 2, 0, 0);
    itemPanelSizer->AddGrowableCol(1, 1);

    // Account
    accountCheckBox_ = new wxCheckBox(itemPanel, ID_ACCOUNT_CB, _("Account"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(accountCheckBox_, g_flagsH);

    bSelectedAccounts_ = new wxButton(itemPanel, wxID_STATIC, _("All"));
    bSelectedAccounts_->SetMinSize(wxSize(180, -1));
    bSelectedAccounts_->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(mmFilterTransactionsDialog::OnAccountsButton), nullptr, this);
    itemPanelSizer->Add(bSelectedAccounts_, g_flagsExpand);

    // Period Range
    datesCheckBox_ = new wxCheckBox(itemPanel, ID_PERIOD_CB, _("Period Range"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(datesCheckBox_, g_flagsH);

    rangeChoice_ = new wxChoice(itemPanel, ID_DATE_RANGE);
    rangeChoice_->SetName("DateRanges");
    for (const auto& date_range : m_all_date_ranges)
    {
        rangeChoice_->Append(date_range.get()->local_title());
    }
    itemPanelSizer->Add(rangeChoice_, g_flagsExpand);

    // Date Range
    dateRangeCheckBox_ = new wxCheckBox(itemPanel, ID_DATE_RANGE_CB, _("Date Range"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(dateRangeCheckBox_, g_flagsH);

    fromDateCtrl_ = new mmDatePickerCtrl(itemPanel, wxID_FIRST, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
    toDateControl_ = new mmDatePickerCtrl(itemPanel, wxID_LAST, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);

    wxBoxSizer* dateSizer = new wxBoxSizer(wxHORIZONTAL);
    dateSizer->Add(fromDateCtrl_->mmGetLayoutWithTime(), g_flagsExpand);
    dateSizer->AddSpacer(5);
    dateSizer->Add(toDateControl_->mmGetLayoutWithTime(), g_flagsExpand);
    itemPanelSizer->Add(dateSizer, wxSizerFlags(g_flagsExpand).Border(0));

    // Payee
    payeeCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Payee"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(payeeCheckBox_, g_flagsH);

    cbPayee_ = new mmComboBoxPayee(itemPanel, mmID_PAYEE);
    cbPayee_->SetMinSize(wxSize(220, -1));

    itemPanelSizer->Add(cbPayee_, g_flagsExpand);

    // Category
    categoryCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Category"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(categoryCheckBox_, g_flagsH);

    categoryComboBox_ = new mmComboBoxCategory(itemPanel, mmID_CATEGORY);
    categoryComboBox_->Bind(wxEVT_COMBOBOX, &mmFilterTransactionsDialog::OnCategoryChange, this);
    categoryComboBox_->Bind(wxEVT_KILL_FOCUS, &mmFilterTransactionsDialog::OnCategoryChange, this);
    itemPanelSizer->Add(categoryComboBox_, g_flagsExpand);

    // Category sub-category checkbox
    categorySubCatCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Include all subcategories"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    categorySubCatCheckBox_->Bind(wxEVT_CHECKBOX, &mmFilterTransactionsDialog::OnCategoryChange, this);

    itemPanelSizer->AddSpacer(1);
    itemPanelSizer->Add(categorySubCatCheckBox_, g_flagsExpand);

    // Tags
    tagCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Tags"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(tagCheckBox_, g_flagsH);

    tagTextCtrl_ = new mmTagTextCtrl(itemPanel, wxID_ANY, true);
    itemPanelSizer->Add(tagTextCtrl_, g_flagsExpand);

    // Status
    statusCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Status"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(statusCheckBox_, g_flagsH);

    choiceStatus_ = new wxChoice(itemPanel, wxID_ANY);

    for (const auto& i : TRANSACTION_STATUSES)
        choiceStatus_->Append(wxGetTranslation(i), new wxStringClientData(i));

    itemPanelSizer->Add(choiceStatus_, g_flagsExpand);
    mmToolTip(choiceStatus_, _("Specify the status for the transaction"));

    // Type
    typeCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Type"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);

    wxFlexGridSizer* typeSizer = new wxFlexGridSizer(0, 2, 0, 0);
    typeSizer->AddGrowableCol(0, 1);
    typeSizer->AddGrowableCol(1, 1);
    cbTypeWithdrawal_ = new wxCheckBox(itemPanel, wxID_ANY, _("Withdrawal"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cbTypeDeposit_ = new wxCheckBox(itemPanel, wxID_ANY, _("Deposit"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cbTypeTransferTo_ = new wxCheckBox(itemPanel, wxID_ANY, _("Transfer Out"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cbTypeTransferFrom_ = new wxCheckBox(itemPanel, wxID_ANY, _("Transfer In"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);

    itemPanelSizer->Add(typeCheckBox_, g_flagsH);
    itemPanelSizer->Add(typeSizer, g_flagsExpand);
    typeSizer->Add(cbTypeWithdrawal_, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    typeSizer->Add(cbTypeDeposit_, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    typeSizer->Add(cbTypeTransferTo_, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    typeSizer->Add(cbTypeTransferFrom_, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    typeSizer->AddSpacer(2);

    // Amount
    amountRangeCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Amount Range"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(amountRangeCheckBox_, g_flagsH);

    amountMinEdit_ = new mmTextCtrl(itemPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    amountMinEdit_->SetMinSize(wxSize(105, -1));
    amountMaxEdit_ = new mmTextCtrl(itemPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    amountMaxEdit_->SetMinSize(wxSize(105, -1));
    wxBoxSizer* amountSizer = new wxBoxSizer(wxHORIZONTAL);
    amountSizer->Add(amountMinEdit_, g_flagsExpand);
    amountSizer->Add(amountMaxEdit_, g_flagsExpand);
    itemPanelSizer->Add(amountSizer, wxSizerFlags(g_flagsExpand).Border(0));

    // Number
    transNumberCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Number"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(transNumberCheckBox_, g_flagsH);

    transNumberEdit_ = new wxTextCtrl(itemPanel, wxID_ANY);
    itemPanelSizer->Add(transNumberEdit_, g_flagsExpand);

    // Notes
    notesCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Notes"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(notesCheckBox_, g_flagsH);

    notesEdit_ = new wxTextCtrl(itemPanel, wxID_ANY);
    itemPanelSizer->Add(notesEdit_, g_flagsExpand);
    mmToolTip(notesEdit_, _("Enter any string to find it in transaction notes") + "\n\n" +
        _u("Tips: Wildcard characters—question mark (?), asterisk (*)—can be used in search criteria.") + "\n" +
        _u("Use the question mark (?) to find any single character—for example, “s?t” finds “sat” and “set”.") + "\n" +
        _u("Use the asterisk (*) to find any number of characters—for example, “s*d” finds “sad” and “started”.") + "\n" +
        _u("Use the asterisk (*) at the beginning to find any string in the middle of the sentence.") + "\n" +
        _("Use regex: to match using regular expressions."));

    // Colour
    colorCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Color"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(colorCheckBox_, g_flagsH);

    colorButton_ = new mmColorButton(itemPanel, wxID_HIGHEST);
    itemPanelSizer->Add(colorButton_, g_flagsExpand);

    itemPanel->SetSizerAndFit(itemPanelSizer);
    scroll_window->SetSizerAndFit(scrollWindowSizer);
    Fit();
    wxSize min_scroll_size = scroll_window->GetSize();
    scroll_window->SetSizeHints(min_scroll_size.GetWidth(), min_scroll_size.GetHeight(), -1, min_scroll_size.GetHeight());

    /******************************************************************************
     Presentation Panel
    *******************************************************************************/

    wxPanel* presPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    if (isReportMode_)
    {
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

    // Hide columns
    showColumnsCheckBox_ = new wxCheckBox(presPanel, wxID_ANY, _("Hide Columns"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    presPanelSizer->Add(showColumnsCheckBox_, g_flagsH);

    bHideColumns_ = new wxButton(presPanel, ID_DIALOG_COLUMNS, "");
    bHideColumns_->SetMinSize(wxSize(180, -1));
    bHideColumns_->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(mmFilterTransactionsDialog::OnShowColumnsButton), nullptr, this);
    presPanelSizer->Add(bHideColumns_, g_flagsExpand);

    // Group By
    groupByCheckBox_ = new wxCheckBox(presPanel, wxID_ANY, _("Group By"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    presPanelSizer->Add(groupByCheckBox_, g_flagsH);

    bGroupBy_ = new wxChoice(presPanel, wxID_ANY);
    for (const auto& i : GROUPBY_OPTIONS)
    {
        bGroupBy_->Append(wxGetTranslation(i), new wxStringClientData(i));
    }
    presPanelSizer->Add(bGroupBy_, g_flagsExpand);
    mmToolTip(bGroupBy_, _("Specify how the report should be grouped"));

    // Chart
    chartCheckBox_ = new wxCheckBox(presPanel, wxID_ANY, _("Chart"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    presPanelSizer->Add(chartCheckBox_, g_flagsH);

    bChart_ = new wxChoice(presPanel, wxID_ANY);
    for (const auto& i : CHART_OPTIONS)
    {
        bChart_->Append(wxGetTranslation(i), new wxStringClientData(i));
    }
    presPanelSizer->Add(bChart_, g_flagsExpand);
    mmToolTip(bChart_, _("Specify which chart will be included in the report"));

    // Compress Splits
    combineSplitsCheckBox_ = new wxCheckBox(presPanel, wxID_ANY, _("Combine Splits"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    combineSplitsCheckBox_->SetMinSize(wxSize(-1, bGroupBy_->GetSize().GetHeight()));
    presPanelSizer->Add(combineSplitsCheckBox_, g_flagsH);
    mmToolTip(combineSplitsCheckBox_, _("Display split transactions as a single row"));

    // Settings
    wxBoxSizer* settings_box_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* settings_sizer = new wxBoxSizer(wxVERTICAL);
    settings_sizer->Add(settings_box_sizer, wxSizerFlags(g_flagsExpand).Border(wxALL, 0));

    if (isMultiAccount_)
    {
        wxStaticText* settings = new wxStaticText(this, wxID_ANY, _("Settings"));
        settings_box_sizer->Add(settings, g_flagsH);
        settings_box_sizer->AddSpacer(5);
    }

    m_setting_name = new wxChoice(this, wxID_APPLY);
    settings_box_sizer->Add(m_setting_name, g_flagsExpand);
    mmDoInitSettingNameChoice();
    m_setting_name->Connect(wxID_APPLY, wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(mmFilterTransactionsDialog::OnSettingsSelected), nullptr, this);

    settings_box_sizer->AddSpacer(5);
    m_btnSaveAs = new wxBitmapButton(this, wxID_SAVEAS, mmBitmapBundle(png::SAVE, mmBitmapButtonSize));
    settings_box_sizer->Add(m_btnSaveAs, g_flagsH);
    mmToolTip(m_btnSaveAs, _("Save active values into current Preset selection"));
    m_btnSaveAs->Connect(wxID_SAVEAS, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(mmFilterTransactionsDialog::OnSaveSettings), nullptr, this);

    m_itemButtonClear = new wxBitmapButton(this, wxID_CLEAR, mmBitmapBundle(png::CLEAR, mmBitmapButtonSize));
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

    wxBitmapButton* button_hide = new wxBitmapButton(button_panel, ID_BTN_CUSTOMFIELDS, mmBitmapBundle(png::RIGHTARROW, mmBitmapButtonSize));
    mmToolTip(button_hide, _("Show/Hide custom fields window"));
    if (m_custom_fields->GetCustomFieldsCount() == 0)
    {
        button_hide->Hide();
    }

    button_sizer->Add(button_ok, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));
    button_sizer->Add(button_cancel, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));
    button_sizer->Add(button_hide, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));

    // Custom fields -----------------------------------

    m_custom_fields->FillCustomFields(custom_fields_box_sizer);
    auto cf_count = m_custom_fields->GetCustomFieldsCount();
    if (cf_count > 0)
    {
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
    if (!isReportMode_)
    {
        groupByCheckBox_->Disable();
        bGroupBy_->Disable();
        chartCheckBox_->Disable();
        bChart_->Disable();
        showColumnsCheckBox_->Disable();
        bHideColumns_->Disable();
        groupByCheckBox_->Hide();
        bGroupBy_->Hide();
        chartCheckBox_->Hide();
        bChart_->Hide();
        showColumnsCheckBox_->Hide();
        bHideColumns_->Hide();
        combineSplitsCheckBox_->Hide();
    }

    wxCommandEvent e(wxID_APPLY);
    OnSettingsSelected(e);

    Center();
    wxSize max_size = wxDisplay(GetParent()).GetGeometry().GetSize();
    wxSize best_size = GetBestSize();
    SetSizeHints(std::min({ best_size.GetWidth(), max_size.GetWidth() }), -1, max_size.GetWidth(), std::min({ max_size.GetHeight(), best_size.GetHeight() }));
    Fit();
    this->SetSizer(box_sizer);
    if (isReportMode_)
        mmSetSize(this);
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
    {
        if (dateRangeCheckBox_->IsChecked())
            dateRangeCheckBox_->SetValue(false);
        wxCommandEvent evt(wxID_ANY, ID_DATE_RANGE);
        evt.SetInt(rangeChoice_->GetSelection());
        OnChoice(evt);
        break;
    }
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
    categorySubCatCheckBox_->Enable(categoryCheckBox_->IsChecked());
    choiceStatus_->Enable(statusCheckBox_->IsChecked());
    cbTypeWithdrawal_->Enable(typeCheckBox_->IsChecked());
    cbTypeDeposit_->Enable(typeCheckBox_->IsChecked());
    cbTypeTransferTo_->Enable(typeCheckBox_->IsChecked());
    cbTypeTransferFrom_->Enable(typeCheckBox_->IsChecked());
    amountMinEdit_->Enable(amountRangeCheckBox_->IsChecked());
    amountMaxEdit_->Enable(amountRangeCheckBox_->IsChecked());
    notesEdit_->Enable(notesCheckBox_->IsChecked());
    transNumberEdit_->Enable(transNumberCheckBox_->IsChecked());
    tagTextCtrl_->Enable(tagCheckBox_->IsChecked());
    rangeChoice_->Enable(datesCheckBox_->IsChecked());
    fromDateCtrl_->Enable(dateRangeCheckBox_->IsChecked());
    toDateControl_->Enable(dateRangeCheckBox_->IsChecked());
    colorButton_->Enable(colorCheckBox_->IsChecked());
    bHideColumns_->Enable(showColumnsCheckBox_->IsChecked());
    bGroupBy_->Enable(groupByCheckBox_->IsChecked() && isReportMode_);
    bChart_->Enable(chartCheckBox_->IsChecked() && isReportMode_);

    event.Skip();
}

bool mmFilterTransactionsDialog::mmIsValuesCorrect() const
{
    if (accountCheckBox_->IsChecked() && m_selected_accounts_id.empty())
    {
        mmErrorDialogs::ToolTip4Object(bSelectedAccounts_, _("Account"), _("Invalid value"), wxICON_ERROR);
        return false;
    }

    if (dateRangeCheckBox_->IsChecked())
    {
        if (m_begin_date > m_end_date)
        {
            const auto today = wxDate::Today().FormatISOCombined();
            int id = m_begin_date >= today ? fromDateCtrl_->GetId() : toDateControl_->GetId();
            mmErrorDialogs::ToolTip4Object(FindWindow(id), _("Date"), _("Invalid value"), wxICON_ERROR);
            return false;
        }
    }

    if (datesCheckBox_->IsChecked() && rangeChoice_->GetSelection() == wxNOT_FOUND)
    {
        mmErrorDialogs::ToolTip4Object(rangeChoice_, _("Date"), _("Invalid value"), wxICON_ERROR);
        return false;
    }

    if (mmIsPayeeChecked())
    {
        bool ok = false;
        const auto& value = cbPayee_->mmGetPattern();

        if (value.empty())
        {
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
                if (pattern.Matches(payee.PAYEENAME))
                {
                    ok = true;
                    break;
                }
            }
            if (ok == false)
            {
                if (wxMessageBox(wxString::Format(_("This name does not currently match any payees.\n"
                                                    "Do you want to continue to use it?\n%s"),
                                                  value),
                                 _("Invalid value"), wxYES_NO | wxICON_INFORMATION) == wxNO)
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
        const auto& value = categoryComboBox_->mmGetPattern();
        if (value.empty())
        {
            mmErrorDialogs::ToolTip4Object(categoryComboBox_, _("Empty value"), _("Category"), wxICON_ERROR);
            return false;
        }
        wxRegEx pattern(value, wxRE_ADVANCED);
        if (!pattern.IsValid() || m_selected_categories_id.empty())
        {
            mmErrorDialogs::ToolTip4Object(categoryComboBox_, _("Invalid value"), _("Category"), wxICON_ERROR);
            return false;
        }
    }

    if (mmIsStatusChecked() && choiceStatus_->GetSelection() == wxNOT_FOUND)
    {
        mmErrorDialogs::ToolTip4Object(choiceStatus_, _("Invalid value"), _("Status"), wxICON_ERROR);
        return false;
    }

    if (mmIsTypeChecked() && mmGetTypes().empty())
    {
        mmErrorDialogs::ToolTip4Object(cbTypeWithdrawal_, _("Invalid value"), _("Type"), wxICON_ERROR);
        return false;
    }

    if (mmIsTagsChecked())
    {
        if (!tagTextCtrl_->IsValid())
        {
            mmErrorDialogs::ToolTip4Object(tagTextCtrl_, _("Invalid value"), _("Tags"), wxICON_ERROR);
            return false;
        }
        else if (tagTextCtrl_->GetTagIDs().empty())
        {
            mmErrorDialogs::ToolTip4Object(tagTextCtrl_, _("Empty value"), _("Tags"), wxICON_ERROR);
            return false;
        }
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
            if (max_amount < min_amount)
            {
                mmErrorDialogs::ToolTip4Object(amountMaxEdit_, _("Invalid value"), _("Amount"), wxICON_ERROR);
                return false;
            }
        }
    }

    if (mmIsColorChecked())
    {
        if (m_color_value < 1 || m_color_value > 7)
        {
            mmErrorDialogs::ToolTip4Object(colorButton_, _("Invalid value"), _("Color"), wxICON_ERROR);
            return false;
        }
    }

    if (showColumnsCheckBox_->IsChecked())
    {
        if (m_selected_columns_id.empty())
        {
            mmErrorDialogs::ToolTip4Object(bHideColumns_, _("Invalid value"), _("Hide Columns"), wxICON_ERROR);
            return false;
        }
    }

    if (groupByCheckBox_->IsChecked() && bGroupBy_->GetSelection() == wxNOT_FOUND)
    {
        mmErrorDialogs::ToolTip4Object(bGroupBy_, _("Invalid value"), _("Group By"), wxICON_ERROR);
        return false;
    }

    if (chartCheckBox_->IsChecked() && bChart_->GetSelection() == wxNOT_FOUND)
    {
        mmErrorDialogs::ToolTip4Object(bChart_, _("Invalid value"), _("Chart"), wxICON_ERROR);
        return false;
    }

    if (!m_custom_fields->ValidateCustomValues(0))
    {
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

void mmFilterTransactionsDialog::OnButtonCancelClick(wxCommandEvent& WXUNUSED(event))
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

    bool useDateTime = Option::instance().UseTransDateTime();

    wxArrayString column_names;
    for (const auto& name : COLUMN_NAMES)
    {
        // Suppress the 'Time' column if the option is turned off
        if (!useDateTime && name == "Time")
        {
            if (m_selected_columns_id.Index(COL_TIME) != wxNOT_FOUND)
                m_selected_columns_id.Remove(COL_TIME);
        }
        else
            column_names.Add(wxGetTranslation(name));
    }

    wxArrayInt hiddenCols = m_selected_columns_id;

    if (!useDateTime)
    {
        // We removed the 'Time' column from the list of names
        // We need to reduce the index of any columns to the right to realign the indexes to the names in the dialog
        for (unsigned int i = 0; i < hiddenCols.GetCount(); i++)
        {
            if (hiddenCols[i] > COL_TIME)
                hiddenCols[i] -= 1;
        }
    }

    mmMultiChoiceDialog s_col(this, _("Hide Report Columns"), "", column_names);
    s_col.SetSelections(hiddenCols);

    wxString baloon = "";
    wxArrayInt selected_items;

    bHideColumns_->UnsetToolTip();

    if (s_col.ShowModal() == wxID_OK)
    {
        m_selected_columns_id.Clear();
        selected_items = s_col.GetSelections();
        for (const auto& entry : selected_items)
        {
            int index = entry;

            // If we shifted the names when displaying the dialog, shift them back
            if (!useDateTime && index >= COL_TIME)
                index += 1;

            const wxString column_name = COLUMN_NAMES[index];
            m_selected_columns_id.Add(index);
            baloon += wxGetTranslation(column_name) + "\n";
        }
    }

    if (m_selected_columns_id.empty() ||
        (!useDateTime && m_selected_columns_id.size() == 1 && m_selected_columns_id[0] == COL_TIME))
    {
        bHideColumns_->SetLabelText("");
        showColumnsCheckBox_->SetValue(false);
        bHideColumns_->Disable();
    }
    else if (! m_selected_columns_id.empty())
    {
        bHideColumns_->SetLabelText("...");
        mmToolTip(bHideColumns_, baloon);
    }
}

bool mmFilterTransactionsDialog::mmIsSomethingChecked() const
{
    return mmIsAccountChecked() || mmIsRangeChecked() || mmIsDateRangeChecked() || mmIsPayeeChecked() || mmIsCategoryChecked() || mmIsStatusChecked() ||
           mmIsTypeChecked() || mmIsAmountRangeMinChecked() || mmIsAmountRangeMaxChecked() || mmIsNumberChecked() || mmIsTagsChecked() || mmIsNotesChecked() ||
           mmIsColorChecked() || mmIsCustomFieldChecked();
}

const wxString mmFilterTransactionsDialog::mmGetStatus() const
{
    wxString status;
    wxStringClientData* status_obj = static_cast<wxStringClientData*>(choiceStatus_->GetClientObject(choiceStatus_->GetSelection()));
    if (status_obj)
        status = status_obj->GetData().Left(1);
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
        return Model_Checking::STATUS_KEY_RECONCILED != itemStatus;
    }
    return false;
}

bool mmFilterTransactionsDialog::mmIsTypeMaches(const wxString& typeState, int64 accountid, int64 toaccountid) const
{
    bool result = false;
    if (typeState == Model_Checking::TYPE_STR_TRANSFER && cbTypeTransferTo_->GetValue() &&
        (!mmIsAccountChecked() || std::find(m_selected_accounts_id.begin(), m_selected_accounts_id.end(), accountid) != m_selected_accounts_id.end()))
    {
        result = true;
    }
    else if (typeState == Model_Checking::TYPE_STR_TRANSFER && cbTypeTransferFrom_->GetValue() &&
             (!mmIsAccountChecked() || std::find(m_selected_accounts_id.begin(), m_selected_accounts_id.end(), toaccountid) != m_selected_accounts_id.end()))
    {
        result = true;
    }
    else if (typeState == Model_Checking::TYPE_STR_WITHDRAWAL && cbTypeWithdrawal_->IsChecked())
    {
        result = true;
    }
    else if (typeState == Model_Checking::TYPE_STR_DEPOSIT && cbTypeDeposit_->IsChecked())
    {
        result = true;
    }

    return result;
}

double mmFilterTransactionsDialog::mmGetAmountMin() const
{
    Model_Currency::Data* currency = Model_Currency::GetBaseCurrency();

    wxString amountStr = amountMinEdit_->GetValue().Trim();
    double amount = 0;
    if (!Model_Currency::fromString(amountStr, amount, currency) || amount < 0)
        amount = 0;

    return amount;
}

double mmFilterTransactionsDialog::mmGetAmountMax() const
{
    Model_Currency::Data* currency = Model_Currency::GetBaseCurrency();

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
        if (wxMessageBox(_("The selected item will be deleted") + "\n\n" + _("Do you want to continue?"), _("Settings item deletion"),
                         wxYES_NO | wxICON_WARNING) == wxNO)
        {
            return;
        }

        int sel_json = Model_Infotable::instance().findArrayItem(m_filter_key, mmGetLabelString());
        if (sel_json != wxNOT_FOUND)
            Model_Infotable::instance().eraseArrayItem(m_filter_key, sel_json);

        m_setting_name->Delete(sel--);
        m_settings_json.clear();

        m_setting_name->SetSelection(sel < 0 ? 0 : sel);
        wxCommandEvent evt(wxID_APPLY);
        OnSettingsSelected(evt);
        mmDoDataToControls(m_settings_json);
    }
}

bool mmFilterTransactionsDialog::mmIsPayeeMatches(int64 payeeID)
{
    const Model_Payee::Data* payee = Model_Payee::instance().get(payeeID);
    if (payee)
    {
        const wxString value = cbPayee_->mmGetPattern();
        if (!value.empty())
        {
            wxRegEx pattern("^(" + value + ")$", wxRE_ICASE | wxRE_ADVANCED);
            if (pattern.IsValid() && pattern.Matches(payee->PAYEENAME))
            {
                return true;
            }
        }
    }
    return false;
}

bool mmFilterTransactionsDialog::mmIsNoteMatches(const wxString& note)
{
    const wxString value = mmGetNotes();
    if (!value.empty())
    {
        if (value.StartsWith("regex:"))
        {
            wxRegEx pattern("^(" + value.Right(value.length() - 6).ToStdString() + ")$", wxRE_ICASE | wxRE_EXTENDED);
            return (pattern.IsValid() && pattern.Matches(note));
        }
        else
            return note.Lower().Matches(value.Lower());
    }
    else
        return note.IsEmpty();
}

bool mmFilterTransactionsDialog::mmIsCategoryMatches(int64 categid)
{
    return std::find(m_selected_categories_id.begin(), m_selected_categories_id.end(), categid) != m_selected_categories_id.end();
}

bool mmFilterTransactionsDialog::mmIsTagMatches(const wxString& refType, int64 refId, bool mergeSplitTags)
{
    std::map<wxString, int64> tagnames = Model_Taglink::instance().get(refType, refId);

    // If we have a split, merge the transaciton tags so that an AND condition captures cases
    // where one tag is on the base txn and the other is on the split
    std::map<wxString, int64> txnTagnames;
    if (refType == Model_Attachment::REFTYPE_STR_TRANSACTIONSPLIT)
        txnTagnames = Model_Taglink::instance().get(Model_Attachment::REFTYPE_STR_TRANSACTION,
                                                    Model_Splittransaction::instance().get(refId)->TRANSID);
    else if (refType == Model_Attachment::REFTYPE_STR_BILLSDEPOSITSPLIT)
        txnTagnames = Model_Taglink::instance().get(Model_Attachment::REFTYPE_STR_BILLSDEPOSIT,
                                                    Model_Budgetsplittransaction::instance().get(refId)->TRANSID);

    if (mergeSplitTags)
    {
        // Merge transaction tags and split tags. This is necessary when checking
        // if a split record matches the filter since we are using mmIsRecordMatches
        // to validate the split which gives it the wrong refType & refId
        if (refType == Model_Attachment::REFTYPE_STR_TRANSACTION)
        {
            // Loop through checking splits and merge tags for each SPLITTRANSID
            for (const auto& split : Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(refId)))
            {
                std::map<wxString, int64> splitTagnames =
                    Model_Taglink::instance().get(Model_Attachment::REFTYPE_STR_TRANSACTIONSPLIT, split.SPLITTRANSID);
                txnTagnames.insert(splitTagnames.begin(), splitTagnames.end());
            }
        }
        else if (refType == Model_Attachment::REFTYPE_STR_BILLSDEPOSIT)
        {
            // Loop through scheduled txn splits and merge tags for each SPLITTRANSID
            for (const auto& split : Model_Budgetsplittransaction::instance().find(Model_Budgetsplittransaction::TRANSID(refId)))
            {
                std::map<wxString, int64> splitTagnames =
                    Model_Taglink::instance().get(Model_Attachment::REFTYPE_STR_BILLSDEPOSITSPLIT, split.SPLITTRANSID);
                txnTagnames.insert(splitTagnames.begin(), splitTagnames.end());
            }
        }
    }

    tagnames.insert(txnTagnames.begin(), txnTagnames.end());
    if (tagnames.empty())
        return false;

    bool match = true;

    wxArrayString tags = tagTextCtrl_->GetTagStrings();
    for (int i = 0; i < static_cast<int>(tags.GetCount()); i++)
    {
        wxString tag = tags.Item(i);
        // if the tag is the "OR" operator, fetch the next tag and compare with OR
        if (tags.Item(i) == "|" && i++ < static_cast<int>(tags.GetCount()) - 1)
            match |= tagnames.find(tags.Item(i)) != tagnames.end();
        // if the tag is the "AND" operator, fetch the next tag and compare with AND
        else if (tags.Item(i) == "&" && i++ < static_cast<int>(tags.GetCount()) - 1)
            match &= tagnames.find(tags.Item(i)) != tagnames.end();
        // default compare with AND operator
        else if (tags.Item(i) != "&" && tags.Item(i) != "|")
            match &= tagnames.find(tags.Item(i)) != tagnames.end();
    }

    return match;
}

template <class MODEL, class DATA> bool mmFilterTransactionsDialog::mmIsRecordMatches(const DATA& tran, bool mergeSplitTags)
{
    bool ok = true;

    // wxLogDebug("Check date? %i trx date:%s %s %s", getDateRangeCheckBox(), tran.TRANSDATE, getFromDateCtrl().GetDateOnly().FormatISODate(),
    if (mmIsAccountChecked() && std::find(m_selected_accounts_id.begin(), m_selected_accounts_id.end(), tran.ACCOUNTID) == m_selected_accounts_id.end() && std::find(m_selected_accounts_id.begin(), m_selected_accounts_id.end(), tran.TOACCOUNTID) == m_selected_accounts_id.end())
        ok = false;
    else if ((mmIsDateRangeChecked() || mmIsRangeChecked()) && (tran.TRANSDATE < m_begin_date.Mid(0, tran.TRANSDATE.length()) || tran.TRANSDATE > m_end_date.Mid(0, tran.TRANSDATE.length())))
        ok = false;
    else if (mmIsPayeeChecked() && !mmIsPayeeMatches(tran.PAYEEID))
        ok = false;
    else if (mmIsCategoryChecked() && !mmIsCategoryMatches(tran.CATEGID))
        ok = false;
    else if (mmIsStatusChecked() && !mmIsStatusMatches(tran.STATUS))
        ok = false;
    else if (mmIsTypeChecked() && !mmIsTypeMaches(tran.TRANSCODE, tran.ACCOUNTID, tran.TOACCOUNTID))
        ok = false;
    else if (mmIsAmountRangeMinChecked() && mmGetAmountMin() > tran.TRANSAMOUNT)
        ok = false;
    else if (mmIsAmountRangeMaxChecked() && mmGetAmountMax() < tran.TRANSAMOUNT)
        ok = false;
    else if (mmIsNumberChecked() && (mmGetNumber().empty() ? !tran.TRANSACTIONNUMBER.empty()
                                                           : tran.TRANSACTIONNUMBER.empty() || !tran.TRANSACTIONNUMBER.Lower().Matches(mmGetNumber().Lower())))
        ok = false;
    else if (mmIsNotesChecked() && !mmIsNoteMatches(tran.NOTES))
        ok = false;
    else if (mmIsColorChecked() && (m_color_value != tran.COLOR))
        ok = false;
    else if (mmIsCustomFieldChecked() && !mmIsCustomFieldMatches(tran.id()))
        ok = false;
    else if (mmIsTagsChecked())
    {
        wxString refType;
        // Check the Data type to determine the tag RefType
        if (typeid(tran).hash_code() == typeid(Model_Checking::Data).hash_code())
            refType = Model_Attachment::REFTYPE_STR_TRANSACTION;
        else if (typeid(tran).hash_code() == typeid(Model_Billsdeposits::Data).hash_code())
            refType = Model_Attachment::REFTYPE_STR_BILLSDEPOSIT;
        if (!mmIsTagMatches(refType, tran.id(), mergeSplitTags))
            ok = false;
    }
    return ok;
}

template <class MODEL, class DATA> bool mmFilterTransactionsDialog::mmIsSplitRecordMatches(const DATA& split)
{
    wxString refType;

    if (typeid(split).hash_code() == typeid(Model_Splittransaction::Data).hash_code())
    {
        refType = Model_Attachment::REFTYPE_STR_TRANSACTIONSPLIT;
    }
    else if (typeid(split).hash_code() == typeid(Model_Budgetsplittransaction::Data).hash_code())
    {
        refType = Model_Attachment::REFTYPE_STR_BILLSDEPOSITSPLIT;
    }

    if (mmIsTagsChecked() && !mmIsTagMatches(refType, split.SPLITTRANSID))
        return false;

    return true;
}

template bool mmFilterTransactionsDialog::mmIsSplitRecordMatches<Model_Splittransaction>(const Model_Splittransaction::Data& split);
template bool mmFilterTransactionsDialog::mmIsSplitRecordMatches<Model_Budgetsplittransaction>(const Model_Budgetsplittransaction::Data& split);

int mmFilterTransactionsDialog::mmIsRecordMatches(const Model_Checking::Data& tran, const Model_Splittransaction::Data_Set& splits)
{
    int ok = mmIsRecordMatches<Model_Checking>(tran);
    for (const auto& split : splits)
    {
        // Need to check if the split matches using the transaction Notes & Tags as well
        Model_Checking::Data splitWithTxnNotes(tran);
        splitWithTxnNotes.CATEGID = split.CATEGID;
        splitWithTxnNotes.TRANSAMOUNT = split.SPLITTRANSAMOUNT;
        Model_Checking::Data splitWithSplitNotes = splitWithTxnNotes;
        splitWithSplitNotes.NOTES = split.NOTES;
        ok += (mmIsRecordMatches<Model_Checking>(splitWithSplitNotes, true) || mmIsRecordMatches<Model_Checking>(splitWithTxnNotes, true));
    }
    return ok;
}

int mmFilterTransactionsDialog::mmIsRecordMatches(const Model_Checking::Data& tran, const std::map<int64, Model_Splittransaction::Data_Set>& splits)
{
    const Model_Splittransaction::Data_Set* split = nullptr;
    const auto& it = splits.find(tran.id());
    if (it != splits.end())
        split = &(it->second);
    return mmIsRecordMatches(tran, *split);
}

int mmFilterTransactionsDialog::mmIsRecordMatches(const Model_Billsdeposits::Data& tran, const std::map<int64, Model_Budgetsplittransaction::Data_Set>& splits)
{
    int ok = mmIsRecordMatches<Model_Billsdeposits>(tran);
    const auto& it = splits.find(tran.id());
    if (it != splits.end())
    {
        for (const auto& split : it->second)
        {
            Model_Billsdeposits::Data splitWithTxnNotes = tran;
            splitWithTxnNotes.CATEGID = split.CATEGID;
            splitWithTxnNotes.TRANSAMOUNT = split.SPLITTRANSAMOUNT;
            Model_Billsdeposits::Data splitWithSplitNotes = splitWithTxnNotes;
            splitWithSplitNotes.NOTES = split.NOTES;
            ok += (mmIsRecordMatches<Model_Billsdeposits>(splitWithSplitNotes, true) || mmIsRecordMatches<Model_Billsdeposits>(splitWithTxnNotes, true));
        }
    }
    return ok;
}

const wxString mmFilterTransactionsDialog::mmGetDescriptionToolTip() const
{
    wxString buffer;
    wxString data = mmGetJsonSetings(true);
    Document j_doc;
    if (j_doc.Parse(data.utf8_str()).HasParseError())
    {
        j_doc.Parse("{}");
    }
    for (Value::ConstMemberIterator itr = j_doc.MemberBegin(); itr != j_doc.MemberEnd(); ++itr)
    {
        wxString value;
        buffer += wxString::FromUTF8(itr->name.GetString()).Append(": ");
        switch (itr->value.GetType())
        {
        case kTrueType:
            value = itr->value.GetBool() ? "TRUE" : "FALSE";
            break;
        case kStringType:
        {
            value = wxString::FromUTF8(itr->value.GetString());
            wxRegEx pattern_date(R"(^[0-9]{4}-[0-9]{2}-[0-9]{2}$)");
            wxRegEx pattern_type(R"(^(W?D?T?F?)$)");
            if (pattern_date.Matches(value))
            {
                wxDateTime dt;
                if (mmParseISODate(value, dt))
                    value = mmGetDateTimeForDisplay(value);
            }
            else if (pattern_type.Matches(value))
            {
                wxString temp;
                if (value.Contains("W"))
                    temp += (temp.empty() ? "" : ", ") + _("Withdrawal");
                if (value.Contains("D"))
                    temp += (temp.empty() ? "" : ", ") + _("Deposit");
                if (value.Contains("F"))
                    temp += (temp.empty() ? "" : ", ") + _("Transfer In");
                if (value.Contains("T"))
                    temp += (temp.empty() ? "" : ", ") + (mmGetAccountsID().empty() ? _("Transfer") : _("Transfer Out"));
                value = temp;
            }
            break;
        }
        case kNumberType:
        {
            wxString temp;
            double d = itr->value.GetDouble();
            if (static_cast<int>(d) == d)
                value = wxString::Format("%i", static_cast<int>(d));
            else
                value = wxString::Format("%f", d);
            break;
        }
        case kArrayType:
        {
            buffer.Append("\n    ");
            auto valArray = itr->value.GetArray();
            for (unsigned int i = 0; i < valArray.Size(); i++)
            {
                if (valArray[i].GetType() == kNumberType)
                {
                    if (wxGetTranslation("Tags").IsSameAs(wxString::FromUTF8(itr->name.GetString())))
                    {
                        value += (value.empty() ? "" : " ") + Model_Tag::instance().get(int64(valArray[i].GetInt64()))->TAGNAME;
                        // don't add a newline between tag operators
                        if (valArray.Size() > 1 && i < valArray.Size() - 2 && valArray[i + 1].GetType() == kStringType)
                            continue;
                    }
                    else
                        value += wxString::Format("%i", valArray[i].GetInt());
                }
                else if (valArray[i].GetType() == kStringType)
                {
                    if (wxGetTranslation("Tags").IsSameAs(itr->name.GetString()))
                    {
                        value += (value.empty() ? "" : " " + wxString::FromUTF8(valArray[i].GetString()));
                        continue;
                    }
                    else
                        value += wxString::FromUTF8(valArray[i].GetString());
                }
                if (i < valArray.Size() - 1)
                    value.Append("\n    ");
            }
            break;
        }
        default:
            break;
        }
        if (value.IsEmpty())
            value = _("Empty value");
        buffer += value.Append("\n");
    }

    return buffer;
}

void mmFilterTransactionsDialog::mmGetDescription(mmHTMLBuilder& hb)
{
    hb.addHeader(4, _("Filtering Details: "));
    // Extract the parameters from the transaction dialog and add them to the report.
    wxString data = mmGetJsonSetings(true);
    Document j_doc;
    if (j_doc.Parse(data.utf8_str()).HasParseError())
    {
        j_doc.Parse("{}");
    }

    wxString buffer;

    for (Value::ConstMemberIterator itr = j_doc.MemberBegin(); itr != j_doc.MemberEnd(); ++itr)
    {
        const auto name = wxGetTranslation(wxString::FromUTF8(itr->name.GetString()));
        switch (itr->value.GetType())
        {
        case kTrueType:
            buffer += wxString::Format("<kbd><samp><b>%s:</b> %s</samp></kbd>\n", name, L"\u2713");
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
                    value = mmGetDateTimeForDisplay(value);
            }
            else if (pattern_type.Matches(value))
            {
                wxString temp;
                if (value.Contains("W"))
                    temp += (temp.empty() ? "" : ", ") + _("Withdrawal");
                if (value.Contains("D"))
                    temp += (temp.empty() ? "" : ", ") + _("Deposit");
                if (value.Contains("F"))
                    temp += (temp.empty() ? "" : ", ") + _("Transfer In");
                if (value.Contains("T"))
                    temp += (temp.empty() ? "" : ", ") + (mmGetAccountsID().empty() ? _("Transfer") : _("Transfer Out"));
                value = temp;
            }
            buffer += wxString::Format("<kbd><samp><b>%s:</b> %s</samp></kbd>\n", name, wxGetTranslation(value));
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
            bool appendOperator = false;
            for (const auto& a : itr->value.GetArray())
            {
                if (a.GetType() == kNumberType)
                {
                    // wxLogDebug("%s", wxString::FromUTF8(itr->name.GetString()));
                    if (wxGetTranslation("Tags").IsSameAs(name))
                    {
                        temp += (temp.empty() ? "" : (appendOperator ? " & " : " ")) + Model_Tag::instance().get(int64(a.GetInt64()))->TAGNAME;
                        appendOperator = true;
                    }
                    else if (wxGetTranslation("Hide Columns").IsSameAs(name) &&
                        !(!Option::instance().UseTransDateTime() && a.GetInt() == COL_TIME))
                    {
                        temp += (temp.empty() ? "" : ", ") + wxGetTranslation(COLUMN_NAMES[a.GetInt()]);
                    }
                    else
                        temp += (temp.empty() ? "" : ", ") + wxString::Format("%i", a.GetInt());
                }
                else if (a.GetType() == kStringType)
                {
                    if (wxGetTranslation("Tags").IsSameAs(name))
                    {
                        temp += (temp.empty() ? "" : " " + wxString::FromUTF8(a.GetString()) + " ");
                        appendOperator = false;
                    }
                    else
                        temp += (temp.empty() ? "" : ", ") + wxString::FromUTF8(a.GetString());
                }
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

const wxString mmFilterTransactionsDialog::mmGetTypes() const
{
    wxString type;
    if (cbTypeWithdrawal_->IsChecked())
        type += "W";
    if (cbTypeDeposit_->IsChecked())
        type += "D";
    if (cbTypeTransferTo_->IsChecked())
        type += "T";
    if (cbTypeTransferFrom_->IsThisEnabled() && cbTypeTransferFrom_->IsChecked())
        type += "F";
    return type;
}

const wxString mmFilterTransactionsDialog::mmGetJsonSetings(bool i18n) const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();

    // Label
    wxString label = mmGetLabelString();
    if (m_setting_name->GetSelection() == wxNOT_FOUND)
    {
        label = "";
    }

    if (!label.empty())
    {
        json_writer.Key((i18n ? _("Label") : "LABEL").utf8_str());
        json_writer.String(label.utf8_str());
    }

    // Account
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

    // Dates
    if (dateRangeCheckBox_->IsChecked())
    {
        wxString from_date, to_date;
        if (Option::instance().UseTransDateTime())
        {
            from_date = fromDateCtrl_->GetValue().FormatISOCombined(' ');
            to_date = toDateControl_->GetValue().FormatISOCombined(' ');
        }
        else
        {
            from_date = fromDateCtrl_->GetValue().FormatISODate();
            to_date = toDateControl_->GetValue().FormatISODate();
        }
        json_writer.Key((i18n ? _("Since") : "DATE1").utf8_str());
        json_writer.String(from_date.utf8_str());
        json_writer.Key((i18n ? _("Before") : "DATE2").utf8_str());
        json_writer.String(to_date.utf8_str());
    }

    // Date Period Range
    else if (datesCheckBox_->IsChecked())
    {
        int sel = rangeChoice_->GetSelection();
        if (sel != wxNOT_FOUND)
        {
            const wxSharedPtr<mmDateRange> date_range = m_all_date_ranges.at(sel);
            if (date_range)
            {
                json_writer.Key((i18n ? _("Period") : "PERIOD").utf8_str());
                json_writer.String(date_range->title().utf8_str());
            }
        }
    }

    // Payee
    if (payeeCheckBox_->IsChecked())
    {
        json_writer.Key((i18n ? _("Payee") : "PAYEE").utf8_str());
        json_writer.String(cbPayee_->GetValue().utf8_str());
    }

    // Category
    if (categoryCheckBox_->IsChecked())
    {
        json_writer.Key((i18n ? _("Category") : "CATEGORY").utf8_str());
        if (categoryComboBox_->mmIsValid())
        {
            int64 categ = categoryComboBox_->mmGetCategoryId();
            const auto& full_name = Model_Category::full_name(categ, ":");
            json_writer.String(full_name.utf8_str());
        }
        else
        {
            json_writer.String(categoryComboBox_->GetValue().utf8_str());
        }
    }

    // Sub Category inclusion
    if (categoryCheckBox_->IsChecked())
    {
        json_writer.Key((i18n ? _("Include all subcategories") : "SUBCATEGORYINCLUDE").utf8_str());
        json_writer.Bool(categorySubCatCheckBox_->GetValue());
    }

    // Status
    if (statusCheckBox_->IsChecked())
    {
        wxArrayString s = Model_Checking::STATUS_STR;
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

    // Type
    if (typeCheckBox_->IsChecked())
    {
        wxString type = mmGetTypes();
        if (!type.empty())
        {
            json_writer.Key((i18n ? _("Type") : "TYPE").utf8_str());
            json_writer.String(type.utf8_str());
        }
    }

    // Amounts
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

    // Number
    if (transNumberCheckBox_->IsChecked())
    {
        const wxString num = transNumberEdit_->GetValue();
        json_writer.Key((i18n ? _("Number") : "NUMBER").utf8_str());
        json_writer.String(num.utf8_str());
    }

    // Tags
    if (tagCheckBox_->IsChecked() && !tagTextCtrl_->IsEmpty())
    {
        json_writer.Key((i18n ? _("Tags") : "TAGS").utf8_str());
        json_writer.StartArray();

        for (const auto& tag : tagTextCtrl_->GetTagStrings())
        {
            if (tag == "&" || tag == "|")
                json_writer.String(tag.utf8_str());
            else
                json_writer.Int64(Model_Tag::instance().get(tag)->TAGID.GetValue());
        }

        json_writer.EndArray();
    }

    // Notes
    if (notesCheckBox_->IsChecked())
    {
        wxString notes = notesEdit_->GetValue();
        json_writer.Key((i18n ? _("Notes") : "NOTES").utf8_str());
        json_writer.String(notes.utf8_str());
    }

    // Colour
    if (colorCheckBox_->IsChecked())
    {
        json_writer.Key((i18n ? _("Color") : "COLOR").utf8_str());
        json_writer.Int(m_color_value);
    }

    // Custom Fields
    const auto cf = m_custom_fields->GetActiveCustomFields();
    if (cf.size() > 0)
    {
        for (const auto& i : cf)
        {
            if (!i.second.empty())
            {
                const auto field = Model_CustomField::instance().get(i.first);
                json_writer.Key(wxString::Format("CUSTOM%lld", field->FIELDID).utf8_str());
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

    // Chart
    if (chartCheckBox_->IsChecked())
    {
        const wxString chart = bChart_->GetStringSelection();
        if (!chart.empty())
        {
            json_writer.Key((i18n ? _("Chart") : "CHART").utf8_str());
            json_writer.String(chart.utf8_str());
        }
    }

    // Compress Splits
    const bool combineSplits = combineSplitsCheckBox_->IsChecked();
    if (combineSplits)
    {
        json_writer.Key((i18n ? _("Combine Splits") : "COMBINE_SPLITS").utf8_str());
        json_writer.Bool(combineSplits);
    }

    json_writer.EndObject();

    return wxString::FromUTF8(json_buffer.GetString());
}

void mmFilterTransactionsDialog::OnCategoryChange(wxEvent& event)
{
    m_selected_categories_id.clear();
    if (!categoryComboBox_->GetValue().IsEmpty())
    {
        wxRegEx pattern("^(" + categoryComboBox_->mmGetPattern() + ")$", wxRE_ICASE | wxRE_ADVANCED);
        if (pattern.IsValid())
            for (const auto& category : Model_Category::instance().all_categories())
                if (pattern.Matches(category.first))
                {
                    m_selected_categories_id.push_back(category.second);
                    if (mmIsCategorySubCatChecked())
                        for (const auto& subcat : Model_Category::instance().sub_tree(Model_Category::instance().get(category.second)))
                            m_selected_categories_id.push_back(subcat.CATEGID);
                }
    }
    event.Skip();
}

void mmFilterTransactionsDialog::OnDateChanged(wxDateEvent& event)
{
    switch (event.GetId())
    {
    case wxID_FIRST:
        m_begin_date = event.GetDate().FormatISOCombined();
        break;
    case wxID_LAST:
        if (Option::instance().UseTransDateTime())
            m_end_date = event.GetDate().FormatISOCombined();
        else
            m_end_date = mmDateRange::getDayEnd(event.GetDate()).FormatISOCombined();
        break;
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

bool mmFilterTransactionsDialog::mmIsCustomFieldMatches(int64 transid) const
{
    const auto cf = m_custom_fields->GetActiveCustomFields();
    int matched = 0;
    for (const auto& i : cf)
    {
        auto DataSet = Model_CustomFieldData::instance().find(Model_CustomFieldData::REFID(transid));
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

int mmFilterTransactionsDialog::mmGetChart() const
{
    int by = -1;
    if (chartCheckBox_->IsChecked())
        by = bChart_->GetSelection();
    return by;
}

void mmFilterTransactionsDialog::mmDoResetFilterStatus()
{
    // m_custom_fields->ResetWidgetsChanged();
}

void mmFilterTransactionsDialog::OnSettingsSelected(wxCommandEvent& event)
{
    if (!isMultiAccount_)
        return;
    int sel = event.GetSelection();
    int count = m_setting_name->GetCount();
    if (count > 0)
    {
        wxStringClientData* settings_obj = static_cast<wxStringClientData*>(m_setting_name->GetClientObject(sel));
        if (settings_obj)
            m_settings_json = settings_obj->GetData();

        mmDoDataToControls(m_settings_json);
    }
}

void mmFilterTransactionsDialog::mmDoUpdateSettings()
{
    if (isMultiAccount_ && m_setting_name->GetSelection() != wxNOT_FOUND) {
        int i = Model_Infotable::instance().findArrayItem(m_filter_key, mmGetLabelString());
        if (i != wxNOT_FOUND) {
            m_settings_json = mmGetJsonSetings();
            Model_Infotable::instance().updateArrayItem(m_filter_key, i, m_settings_json);
        }
    }
    if (!isReportMode_) {
        Model_Infotable::instance().setString(
            wxString::Format("CHECK_FILTER_ID_ADV_%lld", accountID_),
            mmGetJsonSetings()
        );
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
        for (unsigned int i = 0; i < m_setting_name->GetCount(); i++)
        {
            label_names.Add(m_setting_name->GetString(i));
        }

        if (label_names.Index(user_label) == wxNOT_FOUND)
        {
            m_setting_name->Append(user_label);
            m_setting_name->SetStringSelection(user_label);
            m_settings_json = mmGetJsonSetings();
            Model_Infotable::instance().prependArrayItem(m_filter_key, m_settings_json, -1);
        }
        else if (label == user_label)
        {
            mmDoUpdateSettings();
        }
        else
        {
            if (wxMessageBox(_("The entered name is already in use"), _("Warning"), wxOK | wxICON_WARNING) == wxOK)
            {
            }
        }
        mmDoInitSettingNameChoice(user_label);
    }
    else
    {
        bool updateLastUsed = false;
        if (!isReportMode_)
        {
            mmDoUpdateSettings();
            // If no filter name is selected, save as the "Last Used"
            // Named filters are updated automatically in the "All Transactions" panel
            if (m_setting_name->GetStringSelection() == "")
            {
                m_settings_json = mmGetJsonSetings();
                updateLastUsed = true;
            }
        }
        else
        {
            const auto& filter_settings = Model_Infotable::instance().getArrayString(m_filter_key);
            const auto& l = mmGetLabelString();
            int sel_json = Model_Infotable::instance().findArrayItem(m_filter_key, l);
            const auto& json = sel_json != wxNOT_FOUND ? filter_settings[sel_json] : "";
            m_settings_json = mmGetJsonSetings();
            if (isMultiAccount_ && json != m_settings_json && !label.empty())
            {
                if (wxMessageBox(_("Filter settings have changed") + "\n" + _("Do you want to save them before continuing?") + "\n\n", _("Please confirm"),
                                 wxYES_NO | wxICON_WARNING) == wxYES)
                {
                    mmDoUpdateSettings();
                }
                else // User changed a preset but didnt save changes
                updateLastUsed = true;
            }
            else if (isMultiAccount_ && label.empty()) // the preset name field is empty 
                updateLastUsed = true;
        }

        // If the filter was changed and not saved by the user, save it as the 'Last Used' filter
        if (updateLastUsed)
        {
            // Save the JSON (without label) as the "Last Used" settings
            Document j_doc;
            if (j_doc.Parse(m_settings_json.utf8_str()).HasParseError())
            {
                j_doc.Parse("{}");
            }
            // Remove the 'LABEL' key which is present if the user changed an existing setting but didn't save the changes
            if (j_doc.HasMember("LABEL"))
            {
                j_doc.RemoveMember("LABEL");
            }
            StringBuffer buffer;
            Writer<StringBuffer> writer(buffer);
            j_doc.Accept(writer);
            Model_Infotable::instance().setString(
                m_filter_key + "_LAST_USED",
                wxString::FromUTF8(buffer.GetString())
            );
            // Update the settings list with the new data
            mmDoInitSettingNameChoice();
        }
    }
    Model_Infotable::instance().setString("TRANSACTIONS_FILTER_LAST_USED", m_settings_json);
}

void mmFilterTransactionsDialog::OnSaveSettings(wxCommandEvent& WXUNUSED(event))
{
    if (mmIsValuesCorrect())
    {
        mmDoSaveSettings(true);
    }
}

void mmFilterTransactionsDialog::OnAccountsButton(wxCommandEvent& WXUNUSED(event))
{
    mmMultiChoiceDialog s_acc(this, _("Choose Accounts"), "", m_accounts_name);

    wxString baloon = "";
    wxArrayInt selected_items;

    for (const auto& acc : m_selected_accounts_id)
    {
        Model_Account::Data* a = Model_Account::instance().get(acc);
        if (a && m_accounts_name.Index(a->ACCOUNTNAME) != wxNOT_FOUND)
            selected_items.Add(m_accounts_name.Index(a->ACCOUNTNAME));
    }
    s_acc.SetSelections(selected_items);

    m_selected_accounts_id.clear();
    bSelectedAccounts_->UnsetToolTip();

    if (s_acc.ShowModal() == wxID_OK)
    {
        selected_items = s_acc.GetSelections();
        for (const auto& entry : selected_items)
        {
            int index = entry;
            const wxString accounts_name = m_accounts_name[index];
            const auto account = Model_Account::instance().get(accounts_name);
            if (account)
                m_selected_accounts_id.push_back(account->ACCOUNTID);
            baloon += accounts_name + "\n";
        }
    }

    if (m_selected_accounts_id.empty())
    {
        bSelectedAccounts_->SetLabelText(_("All"));
        accountCheckBox_->SetValue(false);
        bSelectedAccounts_->Disable();
    }
    else if (m_selected_accounts_id.size() == 1)
    {
        const Model_Account::Data* account = Model_Account::instance().get(*m_selected_accounts_id.begin());
        if (account)
            bSelectedAccounts_->SetLabelText(account->ACCOUNTNAME);
    }
    else if (m_selected_accounts_id.size() > 1)
    {
        bSelectedAccounts_->SetLabelText("...");
        mmToolTip(bSelectedAccounts_, baloon);
    }
}

void mmFilterTransactionsDialog::OnMoreFields(wxCommandEvent& WXUNUSED(event))
{
    wxBitmapButton* button = static_cast<wxBitmapButton*>(FindWindow(ID_BTN_CUSTOMFIELDS));

    if (button)
        button->SetBitmap(mmBitmapBundle(m_custom_fields->IsCustomPanelShown() ? png::RIGHTARROW : png::LEFTARROW, mmBitmapButtonSize));

    m_custom_fields->ShowHideCustomPanel();

    this->SetMinSize(wxSize(0, 0));
    wxSize max_size = wxDisplay(GetParent()).GetGeometry().GetSize();
    wxSize best_size = GetBestSize();
    SetSizeHints(std::min({ best_size.GetWidth(), max_size.GetWidth() }), -1, max_size.GetWidth(), std::min({ max_size.GetHeight(), best_size.GetHeight() }));
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
            m_begin_date = dates->start_date().FormatISOCombined();
            m_end_date = dates->end_date().FormatISOCombined();
            wxLogDebug("%s %s", m_begin_date, m_end_date);
            m_startDay = dates->startDay();
            m_futureIgnored = dates->isFutureIgnored();
            fromDateCtrl_->SetValue(dates->start_date());
            toDateControl_->SetValue(dates->end_date());
        }
        break;
    }
    }
}

void mmFilterTransactionsDialog::OnMenuSelected(wxCommandEvent&)
{
    m_color_value = colorButton_->GetColorId();
    if (!m_color_value)
    {
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
                if (dlg.getRefreshRequested())
                    cbPayee_->mmDoReInitialize();
                int64 payee_id = dlg.getPayeeId();
                Model_Payee::Data* payee = Model_Payee::instance().get(payee_id);
                if (payee)
                {
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
                mmCategDialog dlg(this, true, -1);
                dlg.ShowModal();
                if (dlg.getRefreshRequested())
                    categoryComboBox_->mmDoReInitialize();
                category = Model_Category::full_name(dlg.getCategId());
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

    // The first time the ALT key is pressed accelerator hints are drawn, but custom painting on the tags button
    // is not applied. We need to refresh the tag ctrl to redraw the drop button with the correct image.
    if (event.AltDown() && !altRefreshDone)
    {
        tagTextCtrl_->Refresh();
        altRefreshDone = true;
    }

    event.Skip();
}
