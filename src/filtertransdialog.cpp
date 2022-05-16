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

#include "filtertransdialog.h"
#include "images_list.h"
#include "categdialog.h"
#include "constants.h"
#include "mmSimpleDialogs.h"
#include "paths.h"
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

enum fromdates {
    FROM_FIN_YEAR,
    FROM_CAL_YEAR,
    FROM_90_DAYS,
    FROM_30_DAYS,
    FROM_TODAY
};

static const wxString FROM_DATES[] =
{
    wxTRANSLATE("Start of Financial Year"),
    wxTRANSLATE("Start of This Year"),
    wxTRANSLATE("Since 90 days ago"),
    wxTRANSLATE("Since 30 days ago"),
    wxTRANSLATE("Since Today")
};

static const wxString DATE_PRESETTINGS[] =
{
    VIEW_TRANS_ALL_STR,
    VIEW_TRANS_TODAY_STR,
    VIEW_TRANS_CURRENT_MONTH_STR,
    VIEW_TRANS_LAST_30_DAYS_STR,
    VIEW_TRANS_LAST_90_DAYS_STR,
    VIEW_TRANS_LAST_MONTH_STR,
    VIEW_TRANS_LAST_3MONTHS_STR,
    VIEW_TRANS_LAST_12MONTHS_STR,
    VIEW_TRANS_CURRENT_YEAR_STR,
    VIEW_TRANS_CRRNT_FIN_YEAR_STR,
    VIEW_TRANS_LAST_YEAR_STR,
    VIEW_TRANS_LAST_FIN_YEAR_STR
};

wxIMPLEMENT_DYNAMIC_CLASS(mmFilterTransactionsDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmFilterTransactionsDialog, wxDialog)
EVT_CHECKBOX(wxID_ANY, mmFilterTransactionsDialog::OnCheckboxClick)
EVT_BUTTON(wxID_OK, mmFilterTransactionsDialog::OnButtonOkClick)
EVT_BUTTON(wxID_CANCEL, mmFilterTransactionsDialog::OnButtonCancelClick)
EVT_BUTTON(wxID_CLEAR, mmFilterTransactionsDialog::OnButtonClearClick)
EVT_BUTTON(ID_BTN_CUSTOMFIELDS, mmFilterTransactionsDialog::OnMoreFields)
EVT_MENU(wxID_ANY, mmFilterTransactionsDialog::OnMenuSelected)
EVT_DATE_CHANGED(wxID_ANY, mmFilterTransactionsDialog::OnDateChanged)
EVT_BUTTON(ID_DIALOG_COLOUR, mmFilterTransactionsDialog::OnColourButton)
wxEND_EVENT_TABLE()

mmFilterTransactionsDialog::mmFilterTransactionsDialog()
{
}
mmFilterTransactionsDialog::~mmFilterTransactionsDialog()
{
    wxLogDebug("~mmFilterTransactionsDialog");
}

mmFilterTransactionsDialog::mmFilterTransactionsDialog(wxWindow* parent, bool showAccountFilter, bool isReportMode)
    : m_categ_id(-1)
    , m_subcateg_id(-1)
    , payeeID_(-1)
    , is_similar_category_status(false)
    , isMultiAccount_(showAccountFilter)
    , isReportMode_(isReportMode)
{
    m_custom_fields = new mmCustomDataTransaction(this, NULL, ID_CUSTOMFIELDS + (isReportMode_ ? 100 : 0));
    Create(parent);
    dataToControls();
    is_values_correct();
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

    CreateControls();
    wxCommandEvent evt(wxEVT_CHECKBOX, wxID_ANY);
    AddPendingEvent(evt);

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    this->SetInitialSize();
    SetMinSize(wxSize(400, 580));
    SetIcon(mmex::getProgramIcon());

    Centre();
    return true;
}

int mmFilterTransactionsDialog::ShowModal()
{
    return wxDialog::ShowModal();
}

void mmFilterTransactionsDialog::BuildPayeeList()
{
    wxArrayString all_payees = Model_Payee::instance().all_payee_names();
    wxString selected = cbPayee_->GetValue();
    cbPayee_->SetEvtHandlerEnabled(false);
    cbPayee_->Clear();
    if (!all_payees.empty()) {
        cbPayee_->Insert(all_payees, 0);
        cbPayee_->AutoComplete(all_payees);
        cbPayee_->SetValue(selected);
    }
    cbPayee_->SetEvtHandlerEnabled(true);
}

void mmFilterTransactionsDialog::dataToControls()
{
    m_accounts_name.clear();
    m_selected_accounts_id.clear();

    const auto accounts = Model_Account::instance().find(Model_Account::ACCOUNTTYPE(Model_Account::all_type()[Model_Account::INVESTMENT], NOT_EQUAL));
    for (const auto& acc : accounts) {
        m_accounts_name.push_back(acc.ACCOUNTNAME);
    }
    m_accounts_name.Sort();

    BuildPayeeList();
    SetJsonSettings(m_settings_json);
}
void mmFilterTransactionsDialog::SetSettingsLabel()
{
    m_setting_name->Clear();
    wxArrayString filter_settings = Model_Infotable::instance().GetArrayStringSetting("TRANSACTIONS_FILTER");
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

    if (m_setting_name->GetCount() > 0)
        m_setting_name->SetSelection(0);
}

void mmFilterTransactionsDialog::CreateControls()
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
    accountCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Account")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(accountCheckBox_, g_flagsH);

    bSelectedAccounts_ = new wxButton(itemPanel, wxID_STATIC, _("All"));
    bSelectedAccounts_->SetMinSize(wxSize(180, -1));
    bSelectedAccounts_->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmFilterTransactionsDialog::OnAccountsButton), nullptr, this);
    itemPanelSizer->Add(bSelectedAccounts_, g_flagsExpand);

    if (!isMultiAccount_) 
    {
            accountCheckBox_->Disable();
            bSelectedAccounts_->SetLabelText("");
            bSelectedAccounts_->Disable();
    }

    // From Date
    startDateCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("From Date")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(startDateCheckBox_, g_flagsH);

    startDateDropDown_ = new wxChoice(itemPanel, wxID_ANY);
    for (const auto& i : FROM_DATES)
        startDateDropDown_->Append(wxGetTranslation(i), new wxStringClientData(i));
    itemPanelSizer->Add(startDateDropDown_, g_flagsExpand);

   // Period Range
    rangeCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Period Range")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(rangeCheckBox_, g_flagsH);

    rangeChoice_ = new wxChoice(itemPanel, wxID_ANY);
    rangeChoice_->SetName("DateRanges");
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
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmAllTime()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmLast365Days()));
    m_all_date_ranges.push_back(wxSharedPtr<mmDateRange>(new mmSpecifiedRange(wxDate::Today().SetDay(1), wxDate::Today())));
    for (const auto & date_range : m_all_date_ranges) {
        rangeChoice_->Append(date_range.get()->local_title(), date_range.get());
    }
    itemPanelSizer->Add(rangeChoice_, g_flagsExpand);

    // Date Range
    dateRangeCheckBox_ = new wxCheckBox(itemPanel, ID_DIALOG_DATEPRESET, _("Date Range")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(dateRangeCheckBox_, g_flagsH);

    fromDateCtrl_ = new wxDatePickerCtrl(itemPanel, wxID_FIRST, wxDefaultDateTime
        , wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
    toDateControl_ = new wxDatePickerCtrl(itemPanel, wxID_LAST, wxDefaultDateTime
        , wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
    dateRangeCheckBox_->Connect(ID_DIALOG_DATEPRESET, wxEVT_RIGHT_DOWN
        , wxMouseEventHandler(mmFilterTransactionsDialog::datePresetMenu), nullptr, this);

    wxBoxSizer* dateSizer = new wxBoxSizer(wxHORIZONTAL);
    dateSizer->Add(fromDateCtrl_, g_flagsExpand);
    dateSizer->AddSpacer(5);
    dateSizer->Add(toDateControl_, g_flagsExpand);
    itemPanelSizer->Add(dateSizer, wxSizerFlags(g_flagsExpand).Border(0));

    // Payee
    payeeCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Payee")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(payeeCheckBox_, g_flagsH);

    cbPayee_ = new wxComboBox(itemPanel, wxID_ANY, "", wxDefaultPosition, wxSize(220, -1));
    cbPayee_->Connect(wxID_ANY, wxEVT_COMMAND_TEXT_UPDATED
        , wxCommandEventHandler(mmFilterTransactionsDialog::OnPayeeUpdated), nullptr, this);

    itemPanelSizer->Add(cbPayee_, g_flagsExpand);

    // Category
    categoryCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Category")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);

    wxFlexGridSizer* categSizer = new wxFlexGridSizer(0, 1, 0, 0);
    categSizer->AddGrowableCol(0, 1);

    itemPanelSizer->Add(categoryCheckBox_, g_flagsH);
    itemPanelSizer->Add(categSizer, wxSizerFlags(g_flagsExpand).Border(0));

    btnCategory_ = new wxButton(itemPanel, wxID_ANY, ""
        , wxDefaultPosition, wxDefaultSize);
    btnCategory_->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(mmFilterTransactionsDialog::OnCategs), nullptr, this);
    similarCategCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Include Similar"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    mmToolTip(similarCategCheckBox_, _("Include all subcategories for the selected category."));

    categSizer->Add(btnCategory_, g_flagsExpand);
    categSizer->Add(similarCategCheckBox_, wxSizerFlags(g_flagsH).Center().Border(0));
    categSizer->AddSpacer(5);

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
    amountSizer->AddSpacer(5);
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
    colourCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Color")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(colourCheckBox_, g_flagsH);

    colourButton_ = new wxButton(itemPanel, ID_DIALOG_COLOUR, _("Select the Color"));
    m_colour_value = 0;
    itemPanelSizer->Add(colourButton_, g_flagsExpand);

    /******************************************************************************
     Presentation Panel
    *******************************************************************************/
    wxStaticBox* static_box_sizer_pres = new wxStaticBox(this, wxID_ANY, _("Presentation Options"));
    wxStaticBoxSizer* itemStaticBoxSizer_pres = new wxStaticBoxSizer(static_box_sizer_pres, wxVERTICAL);
    box_sizer2->Add(itemStaticBoxSizer_pres, wxSizerFlags(g_flagsExpand).Proportion(0));

    wxPanel* presPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemStaticBoxSizer_pres->Add(presPanel, g_flagsExpand);

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
    for (const auto& i : GROUPBY_OPTIONS)
        bGroupBy_->Append(wxGetTranslation(i), new wxStringClientData(i));
    presPanelSizer->Add(bGroupBy_, g_flagsExpand);
    mmToolTip(bGroupBy_, _("Specify how the report should be grouped"));

    //Disable items that are only applicable to report mode
    if (!isReportMode_)
    {
        showColumnsCheckBox_->Disable();
        bHideColumns_->SetLabelText("");
        bHideColumns_->Disable();
        groupByCheckBox_->Disable();
        bGroupBy_->SetLabelText("");
        bGroupBy_->Disable();
    }

    // Settings
    wxBoxSizer* settings_box_sizer = new wxBoxSizer(wxHORIZONTAL);

    wxBoxSizer* settings_sizer = new wxBoxSizer(wxVERTICAL);
    settings_sizer->Add(settings_box_sizer, wxSizerFlags(g_flagsExpand).Border(wxALL, 0));

    wxStaticText* settings = new wxStaticText(this, wxID_ANY, _("Settings"));
    settings_box_sizer->Add(settings, g_flagsH);
    settings_box_sizer->AddSpacer(5);

    m_setting_name = new wxChoice(this, wxID_APPLY);
    settings_box_sizer->Add(m_setting_name, g_flagsExpand);
    SetSettingsLabel();
    m_setting_name->Connect(wxID_APPLY, wxEVT_COMMAND_CHOICE_SELECTED
        , wxCommandEventHandler(mmFilterTransactionsDialog::OnSettingsSelected), nullptr, this);

    settings_box_sizer->AddSpacer(5);
    m_btnSaveAs = new wxBitmapButton(this, wxID_SAVEAS, mmBitmap(png::SAVE, mmBitmapButtonSize));
    settings_box_sizer->Add(m_btnSaveAs, g_flagsH);
    mmToolTip(m_btnSaveAs, _("Save active values into current Preset selection"));
    m_btnSaveAs->Connect(wxID_SAVEAS, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmFilterTransactionsDialog::OnSaveSettings), nullptr, this);

    wxBitmapButton* itemButtonClear = new wxBitmapButton(this, wxID_CLEAR, mmBitmap(png::CLEAR, mmBitmapButtonSize));
    mmToolTip(itemButtonClear, _("Clear all fields for current Preset selection"));
    settings_box_sizer->Add(itemButtonClear, g_flagsH);

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
        AddPendingEvent(evt);
    }

    Center();
    this->SetSizer(box_sizer);

}

void mmFilterTransactionsDialog::OnCheckboxClick(wxCommandEvent& event)
{
    if (event.GetId() == similarCategCheckBox_->GetId())
    {
        is_similar_category_status = similarCategCheckBox_->IsChecked();
    } else if (event.GetId() != cbTypeWithdrawal_->GetId() &&
        event.GetId() != cbTypeDeposit_->GetId() &&
        event.GetId() != cbTypeTransferTo_->GetId() &&
        event.GetId() != cbTypeTransferFrom_->GetId())

    {
        if (event.GetId() == startDateCheckBox_->GetId())
        {
            rangeCheckBox_->SetValue(false);
            dateRangeCheckBox_->SetValue(false);

        }
        if (event.GetId() == rangeCheckBox_->GetId())
        {
            startDateCheckBox_->SetValue(false);
            dateRangeCheckBox_->SetValue(false);

        }
        if (event.GetId() == dateRangeCheckBox_->GetId())
        {
            startDateCheckBox_->SetValue(false);
            rangeCheckBox_->SetValue(false);

        }

        bSelectedAccounts_->Enable(accountCheckBox_->IsChecked());
        cbPayee_->Enable(payeeCheckBox_->IsChecked());
        btnCategory_->Enable(categoryCheckBox_->IsChecked());
        similarCategCheckBox_->Enable(categoryCheckBox_->IsChecked());
        choiceStatus_->Enable(statusCheckBox_->IsChecked());
        cbTypeWithdrawal_->Enable(typeCheckBox_->IsChecked());
        cbTypeDeposit_->Enable(typeCheckBox_->IsChecked());
        cbTypeTransferTo_->Enable(typeCheckBox_->IsChecked());
        cbTypeTransferFrom_->Enable(typeCheckBox_->IsChecked());
        if (!accountCheckBox_->IsChecked())
        {
            cbTypeTransferFrom_->Hide();
            cbTypeTransferTo_->SetLabel(_("Transfer"));
            Layout();
        } else
        {
            cbTypeTransferFrom_->Show();
            cbTypeTransferTo_->SetLabel(_("Transfer Out"));
            Layout();
        }
        amountMinEdit_->Enable(amountRangeCheckBox_->IsChecked());
        amountMaxEdit_->Enable(amountRangeCheckBox_->IsChecked());
        notesEdit_->Enable(notesCheckBox_->IsChecked());
        transNumberEdit_->Enable(transNumberCheckBox_->IsChecked());
        startDateDropDown_->Enable(startDateCheckBox_->IsChecked());
        rangeChoice_->Enable(rangeCheckBox_->IsChecked());
        fromDateCtrl_->Enable(dateRangeCheckBox_->IsChecked());
        toDateControl_->Enable(dateRangeCheckBox_->IsChecked());
        colourButton_->Enable(colourCheckBox_->IsChecked());
        bHideColumns_->Enable(showColumnsCheckBox_->IsChecked());
        bGroupBy_->Enable(groupByCheckBox_->IsChecked() && isReportMode_);
    }

    if (accountCheckBox_->IsChecked() && m_selected_accounts_id.size() <= 0)
        bSelectedAccounts_->SetLabelText("");

    event.Skip();
}

bool mmFilterTransactionsDialog::is_values_correct()
{
    if (accountCheckBox_->IsChecked())
    {
        if (m_selected_accounts_id.empty())
        {
            mmErrorDialogs::ToolTip4Object(bSelectedAccounts_
                , _("Invalid value"), _("Account"));
            return false;
        }
    }

    if (payeeCheckBox_->IsChecked())
    {
        Model_Payee::Data* payee = Model_Payee::instance().get(cbPayee_->GetValue());
        if (payee)
        {
            payeeID_ = payee->PAYEEID;
            m_payee_str = payee->PAYEENAME;
        }
        else
        {
            mmErrorDialogs::ToolTip4Object(cbPayee_
                , _("Invalid value"), _("Payee"));
            return false;
        }
    }

    if (amountRangeCheckBox_->IsChecked())
    {
        Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();
        int currency_precision = Model_Currency::precision(currency);
        double min_amount = 0;

        if (!amountMinEdit_->Calculate(currency_precision))
        {
            amountMinEdit_->GetDouble(min_amount);
            mmErrorDialogs::ToolTip4Object(amountMinEdit_
                , _("Invalid value"), _("Amount"));
            return false;
        }

        if (!amountMaxEdit_->Calculate(currency_precision))
        {
            double max_amount = 0;
            amountMaxEdit_->GetDouble(max_amount);
            if (max_amount < min_amount)
            {
                mmErrorDialogs::ToolTip4Object(amountMaxEdit_
                    , _("Invalid value"), _("Amount"));
                return false;
            }
        }
    }

    if (dateRangeCheckBox_->IsChecked())
    {
        m_begin_date = fromDateCtrl_->GetValue().FormatISODate(); 
        m_end_date = toDateControl_->GetValue().FormatISODate(); 
        if (m_begin_date > m_end_date)
        {
            const auto today = wxDate::Today().FormatISODate();
            int id = m_begin_date >= today
                ? fromDateCtrl_->GetId() : toDateControl_->GetId();
            mmErrorDialogs::ToolTip4Object(FindWindow(id)
                , _("Invalid value"), _("Date"));
            return false;
        }
    }

    if (rangeCheckBox_->IsChecked())
    {
        const wxSharedPtr<mmDateRange> date_range = m_all_date_ranges[rangeChoice_->GetSelection()];
        if (date_range)
        {
            m_begin_date = date_range->start_date().FormatISODate(); 
            m_end_date = date_range->end_date().FormatISODate(); 
            m_startDay = date_range->startDay();
            m_futureIgnored = date_range->isFutureIgnored();
        }        
    }

    if (startDateCheckBox_->IsChecked())
    {
        mmDateRange* date_range = NULL;
        switch (startDateDropDown_->GetSelection())
        {
            case FROM_FIN_YEAR:
                date_range = new mmCurrentFinancialYear();
                break;
            case FROM_CAL_YEAR:
                date_range = new mmCurrentYear;
                break;
            case FROM_90_DAYS:
                date_range = new mmLast90Days;
                break;
            case FROM_30_DAYS:
                date_range = new mmLast30Days;
                break;
            case FROM_TODAY:
                date_range = new mmToday;
                break;
            default:
                wxASSERT(FALSE);
        }
        m_begin_date = date_range->start_date().FormatISODate();
        m_futureIgnored = Option::instance().getIgnoreFutureTransactions();
        if (m_futureIgnored) 
            m_end_date = wxDateTime::Today().FormatISODate();
        else
            m_end_date = wxDateTime(DATE_MAX).FormatISODate();
        m_startDay = date_range->startDay();
        delete date_range;
    }

    if (statusCheckBox_->IsChecked() && choiceStatus_->GetSelection() < 0)
    {
        int id = choiceStatus_->GetId();
        mmErrorDialogs::ToolTip4Object(FindWindow(id)
            , _("Invalid value"), _("Status"));
        return false;
    }

    if (groupByCheckBox_->IsChecked() && bGroupBy_->GetSelection() < 0)
    {
        int id = bGroupBy_->GetId();
        mmErrorDialogs::ToolTip4Object(FindWindow(id)
            , _("Invalid value"), _("Group By"));
        return false;
    }

    if (!m_custom_fields->ValidateCustomValues(NULL)) {
        return false;
    }

    return true;
}

void mmFilterTransactionsDialog::OnButtonOkClick(wxCommandEvent& /*event*/)
{
    if (is_values_correct()) {
        auto label = m_setting_name->GetStringSelection();
        const wxString new_settings_string = GetJsonSetings();
        if (m_settings_json != new_settings_string && !label.empty())
        {
            wxArrayString label_names;
            for (unsigned int i = 0; i < m_setting_name->GetCount(); i++)
            {
                label_names.Add(m_setting_name->GetString(i));
            }

            while (label_names.Index(label) != wxNOT_FOUND)
            {
                label = wxGetTextFromUser(
                    _("Filter settings have changed") + "\n" +
                    _("Do you want to save them before continuing?") + "\n\n" +
                    _("Please Enter")
                    , _("Setting Name"), label);
            }

            if (!label.empty())
            {
                m_setting_name->SetString(m_setting_name->GetSelection(), label);
                m_settings_json = GetJsonSetings();
                Model_Infotable::instance().Prepend("TRANSACTIONS_FILTER", m_settings_json, -1);
            }
        }
        EndModal(wxID_OK);
    }
}

void mmFilterTransactionsDialog::OnButtonCancelClick(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void mmFilterTransactionsDialog::OnCategs(wxCommandEvent& /*event*/)
{
    Model_Category::Data* category = Model_Category::instance().get(m_categ_id);
    Model_Subcategory::Data* sub_category = Model_Subcategory::instance().get(m_subcateg_id);
    int categID = category ? category->CATEGID : -1;
    int subcategID = sub_category ? sub_category->SUBCATEGID : -1;
    mmCategDialog dlg(this, true, categID, subcategID);

    if (dlg.ShowModal() == wxID_OK)
    {
        m_categ_id = dlg.getCategId();
        m_subcateg_id = dlg.getSubCategId();
        category = Model_Category::instance().get(m_categ_id);
        sub_category = Model_Subcategory::instance().get(m_subcateg_id);

        btnCategory_->SetLabelText(Model_Category::full_name(category, sub_category));
    }
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


    wxMultiChoiceDialog s_col(this, _("Hide Report Columns")
        , "", column_names);
    s_col.SetSelections(m_selected_columns_id);

    wxButton* ok = static_cast<wxButton*>(s_col.FindWindow(wxID_OK));
    if (ok) ok->SetLabel(_("&OK "));
    wxButton* ca = static_cast<wxButton*>(s_col.FindWindow(wxID_CANCEL));
    if (ca) ca->SetLabel(wxGetTranslation(g_CancelLabel));

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

bool mmFilterTransactionsDialog::isSomethingSelected() const
{
    return
        is_account_cb_active()
        || getRangeCheckBox()
        || is_date_range_cb_active()
        || getStartDateCheckBox()
        || is_payee_cb_active()
        || is_category_cb_active()
        || is_status_cb_active()
        || is_type_cb_active()
        || is_amountrange_min_cb_active()
        || is_amount_range_max_cb_active()
        || is_number_cb_active()
        || is_notes_cb_active()
        || is_colour_cb_active()
        || is_custom_field_active();
}

const wxString mmFilterTransactionsDialog::getStatus() const
{
    wxString status;
    wxStringClientData* status_obj =
        static_cast<wxStringClientData*>(choiceStatus_->GetClientObject(choiceStatus_->GetSelection()));
    if (status_obj) status = status_obj->GetData().Left(1);
    status.Replace("U", "");
    return status;
}

bool mmFilterTransactionsDialog::is_status_matches(const wxString& itemStatus) const
{
    wxString filterStatus = getStatus();
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

bool mmFilterTransactionsDialog::is_type_maches(const wxString& typeState, int accountid, int toaccountid) const
{
    bool result = false;
    if (typeState == Model_Checking::all_type()[Model_Checking::TRANSFER]
        && cbTypeTransferTo_->GetValue() 
        && (!is_account_cb_active() || (m_selected_accounts_id.Index(accountid) != wxNOT_FOUND)))
    {
        result = true;
    }
    else if (typeState == Model_Checking::all_type()[Model_Checking::TRANSFER]
        && cbTypeTransferFrom_->GetValue()
        && (!is_account_cb_active() || (m_selected_accounts_id.Index(toaccountid) != wxNOT_FOUND)))
    {
        result = true;
    }
    else if (typeState == Model_Checking::all_type()[Model_Checking::WITHDRAWAL] && cbTypeWithdrawal_->GetValue())
    {
        result = true;
    }
    else if (typeState == Model_Checking::all_type()[Model_Checking::DEPOSIT] && cbTypeDeposit_->GetValue())
    {
        result = true;
    }

    return result;
}

double mmFilterTransactionsDialog::getAmountMin() const
{
    Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();

    wxString amountStr = amountMinEdit_->GetValue().Trim();
    double amount = 0;
    if (!Model_Currency::fromString(amountStr, amount, currency) || amount < 0)
        amount = 0;

    return amount;
}

double mmFilterTransactionsDialog::getAmountMax() const
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
    if (sel >= 0)
    {
        for (int i = 0; i < size; i++)
        {
            wxLogDebug("%i", i);
            wxStringClientData* settings_obj =
                static_cast<wxStringClientData*>(m_setting_name->GetClientObject(i));
            if (settings_obj) {
                Model_Infotable::instance().Prepend("TRANSACTIONS_FILTER", settings_obj->GetData(), size - 1);
            }
        }

        m_setting_name->Delete(sel--);
        m_settings_json.clear();

        m_setting_name->SetSelection(sel < 0 ? 0 : sel);
        wxCommandEvent evt(wxID_APPLY);
        OnSettingsSelected(evt);
        dataToControls();
    }
}

void mmFilterTransactionsDialog::OnMenuSelected(wxCommandEvent& event)
{
    auto selected_nemu_item = event.GetId();
    if (selected_nemu_item < wxID_HIGHEST)
    {
        setPresettings(DATE_PRESETTINGS[selected_nemu_item]);
    }
    else
    {
        selected_nemu_item -= wxID_HIGHEST;
        colourButton_->SetBackgroundColour(getUDColour(selected_nemu_item));
        m_colour_value = selected_nemu_item;
    }
}

void mmFilterTransactionsDialog::datePresetMenu(wxMouseEvent& event)
{
    wxMenu menu;
    int id = 0;
    for (const auto& i : DATE_PRESETTINGS)
    {
        menu.Append(id++, wxGetTranslation(i));
    }
    PopupMenu(&menu, event.GetPosition());
}

void mmFilterTransactionsDialog::setPresettings(const wxString& view)
{
    wxSharedPtr<mmDateRange> date_range;
    dateRangeCheckBox_->SetValue(true);

    if (view == VIEW_TRANS_ALL_STR)
        dateRangeCheckBox_->SetValue(false);
    else if (view == VIEW_TRANS_TODAY_STR)
        date_range = new mmToday;
    else if (view == VIEW_TRANS_CURRENT_MONTH_STR)
        date_range = new mmCurrentMonth;
    else if (view == VIEW_TRANS_LAST_30_DAYS_STR)
        date_range = new mmLast30Days;
    else if (view == VIEW_TRANS_LAST_90_DAYS_STR)
        date_range = new mmLast90Days;
    else if (view == VIEW_TRANS_LAST_MONTH_STR)
        date_range = new mmLastMonth;
    else if (view == VIEW_TRANS_LAST_3MONTHS_STR)
        date_range = new mmLast3Months;
    else if (view == VIEW_TRANS_LAST_12MONTHS_STR)
        date_range = new mmLast12Months;
    else if (view == VIEW_TRANS_CURRENT_YEAR_STR)
        date_range = new mmCurrentYear;
    else if (view == VIEW_TRANS_CRRNT_FIN_YEAR_STR)
        date_range = new mmCurrentFinancialYear();
    else if (view == VIEW_TRANS_LAST_YEAR_STR)
        date_range = new mmLastYear;
    else if (view == VIEW_TRANS_LAST_FIN_YEAR_STR)
        date_range = new mmLastFinancialYear();
    if (date_range == NULL)
        date_range = new mmCurrentMonth;

    m_begin_date = date_range->start_date().FormatISODate();
    m_end_date = date_range->end_date().FormatISODate();

    if (dateRangeCheckBox_->IsChecked())
    {
        fromDateCtrl_->SetValue(date_range->start_date());
        toDateControl_->SetValue(date_range->end_date());
        fromDateCtrl_->Enable();
        toDateControl_->Enable();
    }
}

void mmFilterTransactionsDialog::OnPayeeUpdated(wxCommandEvent& event)
{
    cbPayee_->SetEvtHandlerEnabled(false);
    Model_Payee::Data* payee = Model_Payee::instance().get(cbPayee_->GetValue());
    if (payee)
    {
        payeeID_ = payee->PAYEEID;
        cbPayee_->SetValue(payee->PAYEENAME);
    }
    cbPayee_->SetEvtHandlerEnabled(true);
    event.Skip();
}

template<class MODEL, class DATA>
bool mmFilterTransactionsDialog::is_payee_matches(const DATA &tran)
{
    const Model_Payee::Data* payee = Model_Payee::instance().get(tran.PAYEEID);
    if (payee)
        return cbPayee_->GetValue().Lower() == (payee->PAYEENAME).Lower();
    return false;
}

template<class MODEL, class DATA>
bool mmFilterTransactionsDialog::is_category_matches(const DATA& tran, const std::map<int, typename MODEL::Split_Data_Set> & splits)
{
    const auto it = splits.find(tran.id());
    if (it == splits.end())
    {
        if (m_categ_id != tran.CATEGID) return false;
        if (m_subcateg_id != tran.SUBCATEGID && !is_similar_category_status) return false;
    }
    else
    {
        bool bMatching = false;
        for (const auto &split : it->second)
        {
            if (split.CATEGID != m_categ_id) continue;
            if (split.SUBCATEGID != m_subcateg_id && !is_similar_category_status) continue;

            bMatching = true;
            break;
        }
        if (!bMatching) return false;
    }
    return true;
}

bool mmFilterTransactionsDialog::checkAll(const Model_Checking::Data &tran
    , const std::map<int, Model_Splittransaction::Data_Set>& split)
{
    bool ok = true;
    //wxLogDebug("Check date? %i trx date:%s %s %s", getDateRangeCheckBox(), tran.TRANSDATE, getFromDateCtrl().GetDateOnly().FormatISODate(), getToDateControl().GetDateOnly().FormatISODate());
    if (is_account_cb_active()
        && m_selected_accounts_id.Index(tran.ACCOUNTID) == wxNOT_FOUND
        && m_selected_accounts_id.Index(tran.TOACCOUNTID) == wxNOT_FOUND)
        ok = false;
    else if ((is_date_range_cb_active() || getRangeCheckBox()
        || getStartDateCheckBox()) && (tran.TRANSDATE < m_begin_date || tran.TRANSDATE > m_end_date))
        ok = false;
    else if (is_payee_cb_active() && !is_payee_matches<Model_Checking>(tran)) ok = false;
    else if (is_category_cb_active() && !is_category_matches<Model_Checking>(tran, split)) ok = false;
    else if (is_status_cb_active() && !is_status_matches(tran.STATUS)) ok = false;
    else if (is_type_cb_active() && !is_type_maches(tran.TRANSCODE, tran.ACCOUNTID, tran.TOACCOUNTID)) ok = false;
    else if (is_amountrange_min_cb_active() && getAmountMin() > tran.TRANSAMOUNT) ok = false;
    else if (is_amount_range_max_cb_active() && getAmountMax() < tran.TRANSAMOUNT) ok = false;
    else if (is_number_cb_active() && (getNumber().empty() ? !tran.TRANSACTIONNUMBER.empty()
        : tran.TRANSACTIONNUMBER.empty() || !tran.TRANSACTIONNUMBER.Lower().Matches(getNumber().Lower())))
        ok = false;
    else if (is_notes_cb_active() && (getNotes().empty() ? !tran.NOTES.empty()
        : tran.NOTES.empty() || !tran.NOTES.Lower().Matches(getNotes().Lower())))
        ok = false;
    else if (is_colour_cb_active() && (m_colour_value != tran.FOLLOWUPID))
        ok = false;
    else if (is_custom_field_active() && !is_custom_field_matches(tran))
        ok = false;
    return ok;
}
bool mmFilterTransactionsDialog::checkAll(const Model_Billsdeposits::Data &tran, const std::map<int, Model_Budgetsplittransaction::Data_Set>& split)
{
    bool ok = true;
    if (is_account_cb_active()
            && m_selected_accounts_id.Index(tran.ACCOUNTID) == wxNOT_FOUND
            && m_selected_accounts_id.Index(tran.TOACCOUNTID) == wxNOT_FOUND)
        ok = false;
    else if ((is_date_range_cb_active() || getRangeCheckBox()) && (tran.TRANSDATE < m_begin_date && tran.TRANSDATE > m_end_date))
        ok = false;
    else if (getStartDateCheckBox() && (tran.TRANSDATE < m_begin_date)) ok = false;
    else if (is_payee_cb_active() && !is_payee_matches<Model_Billsdeposits>(tran)) ok = false;
    else if (is_category_cb_active() && !is_category_matches<Model_Billsdeposits>(tran, split)) ok = false;
    else if (is_status_cb_active() && !is_status_matches(tran.STATUS)) ok = false;
    else if (is_type_cb_active() && !is_type_maches(tran.TRANSCODE, tran.ACCOUNTID, tran.TOACCOUNTID)) ok = false;
    else if (is_amountrange_min_cb_active() && getAmountMin() > tran.TRANSAMOUNT) ok = false;
    else if (is_amount_range_max_cb_active() && getAmountMax() < tran.TRANSAMOUNT) ok = false;
    else if (is_number_cb_active() && (getNumber().empty()
        ? !tran.TRANSACTIONNUMBER.empty()
        : tran.TRANSACTIONNUMBER.empty() || !tran.TRANSACTIONNUMBER.Lower().Matches(getNumber().Lower())))
        ok = false;
    else if (is_notes_cb_active() && (getNotes().empty()
        ? !tran.NOTES.empty()
        : tran.NOTES.empty() || !tran.NOTES.Lower().Matches(getNotes().Lower())))
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

const wxString mmFilterTransactionsDialog::getDescriptionToolTip() const
{
    wxString filterDetails = GetJsonSetings(true);
    filterDetails.Replace(R"("")", _("Empty value"));
    filterDetails.Replace("\"", "");
    filterDetails.Replace("[", "");
    filterDetails.Replace("]", "");
    filterDetails.replace(0, 1, ' ');
    filterDetails.RemoveLast(1);
    filterDetails.Append("\n ");
    return filterDetails;
}

void mmFilterTransactionsDialog::getDescription(mmHTMLBuilder &hb)
{
    hb.addHeader(3, _("Filtering Details: "));
    // Extract the parameters from the transaction dialog and add them to the report.
    wxString filterDetails = GetJsonSetings(true);
    filterDetails.Replace("\n", " ");
    filterDetails.Replace(R"("")", _("Empty value"));
    filterDetails.Replace("\"", "");
    filterDetails.replace(0, 1, ' ');
    filterDetails.RemoveLast(1);
    hb.addText(filterDetails);
}

const wxString mmFilterTransactionsDialog::GetJsonSetings(bool i18n) const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();

    //Label
    wxString label = m_setting_name->GetStringSelection();
    if (m_setting_name->GetSelection() < 0) {
        label = "";
    }

    const wxString default_label = wxString::Format(_("%i: Empty"), m_setting_name->GetSelection() + 1);
    if (!label.empty() && label != default_label)
    {
        json_writer.Key((i18n ? _("Label") : "LABEL").utf8_str());
        json_writer.String(label.utf8_str());
    }

    //Account
    if (accountCheckBox_->IsChecked() && !m_accounts_name.empty())
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
    if (rangeCheckBox_->IsChecked())
    {
        const wxString range = rangeChoice_->GetStringSelection();
        if (!range.empty())
        {
            json_writer.Key((i18n ? _("Period") : "PERIOD").utf8_str());
            json_writer.String(range.utf8_str());
        }
    }

    //From Date
    if (startDateCheckBox_->IsChecked())
    {
        const wxString startPoint = startDateDropDown_->GetStringSelection();
        if (!startPoint.empty())
        {
            json_writer.Key((i18n ? _("From") : "FROM").utf8_str());
            json_writer.String(startPoint.utf8_str());
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
        json_writer.Key((i18n ? _("Include Similar") : "SIMILAR_YN").utf8_str());
        json_writer.Bool(is_similar_category_status);
        json_writer.Key((i18n ? _("Category") : "CATEGORY").utf8_str());
        auto categ = Model_Category::full_name(m_categ_id, m_subcateg_id);
        wxLogDebug("%s", categ);
        json_writer.String(categ.utf8_str());
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
        wxString type = wxString()
            << (cbTypeWithdrawal_->GetValue() && typeCheckBox_->GetValue() ? "W" : "")
            << (cbTypeDeposit_->GetValue() && typeCheckBox_->GetValue() ? "D" : "")
            << (cbTypeTransferTo_->GetValue() && typeCheckBox_->GetValue() ? "T" : "")
            << (cbTypeTransferFrom_->GetValue() && typeCheckBox_->GetValue() ? "F" : "");
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
    if (colourCheckBox_->IsChecked())
    {
        json_writer.Key((i18n ? _("Color") : "COLOR").utf8_str());
        json_writer.Int(m_colour_value);
    }

    //Custom Fields
    const auto cf = m_custom_fields->GetActiveCustomFields();
    if (cf.size() > 0)
    {
        for (const auto& i : cf)
        {
            const auto field = Model_CustomField::instance().get(i.first);
            json_writer.Key(wxString::Format("CUSTOM%i",field->FIELDID).utf8_str());
            json_writer.String(i.second.utf8_str());

        }
    }

    /*******************************************************
     Presentation Options
    *******************************************************/

    // Hide Columns
    if (showColumnsCheckBox_->IsChecked() && !m_selected_columns_id.empty())
    {
        json_writer.Key((i18n ? _("Column") : "COLUMN").utf8_str());
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

void mmFilterTransactionsDialog::SetJsonSettings(const wxString &data)
{
    if (data.empty()) return;

    Document j_doc;
    if (j_doc.Parse(data.utf8_str()).HasParseError())
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
        accountCheckBox_->SetValue(false);

    bSelectedAccounts_->Enable(accountCheckBox_->IsChecked());

    //Dates
    Value& j_date1 = GetValueByPointerWithDefault(j_doc, "/DATE1", "");
    m_begin_date = j_date1.IsString() ? wxString::FromUTF8(j_date1.GetString()) : "";
    Value& j_date2 = GetValueByPointerWithDefault(j_doc, "/DATE2", "");
    m_end_date = j_date2.IsString() ? wxString::FromUTF8(j_date2.GetString()) : "";
    dateRangeCheckBox_->SetValue(!m_begin_date.empty() || !m_end_date.empty());
    fromDateCtrl_->Enable(dateRangeCheckBox_->IsChecked());
    toDateControl_->Enable(dateRangeCheckBox_->IsChecked());
    fromDateCtrl_->SetValue(mmParseISODate(m_begin_date));
    toDateControl_->SetValue(mmParseISODate(m_end_date));

    //Date Period Range
    Value& j_period = GetValueByPointerWithDefault(j_doc, "/PERIOD", "");
    const wxString& s_range = j_period.IsString() ? wxString::FromUTF8(j_period.GetString()) : "";
    rangeCheckBox_->SetValue(!s_range.empty());
    rangeChoice_->Enable(rangeCheckBox_->IsChecked());
    rangeChoice_->SetStringSelection(s_range);

    //From Date
    Value& j_from = GetValueByPointerWithDefault(j_doc, "/FROM", "");
    const wxString& s_startPoint = j_from.IsString() ? wxString::FromUTF8(j_from.GetString()) : "";
    startDateCheckBox_->SetValue(!s_startPoint.empty());
    startDateDropDown_->Enable(startDateCheckBox_->IsChecked());
    startDateDropDown_->SetStringSelection(s_startPoint);

    //Payee
    Value& j_payee = GetValueByPointerWithDefault(j_doc, "/PAYEE", "");
    const wxString& s_payee = j_payee.IsString() ? wxString::FromUTF8(j_payee.GetString()) : "";
    payeeCheckBox_->SetValue(!s_payee.empty());
    cbPayee_->Enable(payeeCheckBox_->IsChecked());
    cbPayee_->SetValue(s_payee);

    //Category
    Value& j_category = GetValueByPointerWithDefault(j_doc, "/CATEGORY", "");
    wxString s_category = j_category.IsString() ? wxString::FromUTF8(j_category.GetString()) : "";
    categoryCheckBox_->SetValue(!s_category.empty());
    btnCategory_->Enable(categoryCheckBox_->IsChecked());

    m_subcateg_id = -1;
    m_categ_id = -1;

    const wxString delimiter = Model_Infotable::instance().GetStringInfo("CATEG_DELIMITER", ":");
    wxStringTokenizer categ_token(s_category, ":", wxTOKEN_RET_EMPTY_ALL);
    const auto categ_name = categ_token.GetNextToken().Trim();
    const auto subcateg_name = categ_token.GetNextToken().Trim(false);
    s_category = categ_name + (s_category.Contains(":") ? delimiter + subcateg_name : "");

    for (const auto& entry : Model_Category::instance().all_categories())
    {
        //wxLogDebug("%s : %i %i", entry.first, entry.second.first, entry.second.second);
        if (s_category == entry.first)
        {
            m_categ_id = entry.second.first;
            m_subcateg_id = entry.second.second;
            break;
        }
    }

    btnCategory_->SetLabelText(Model_Category::full_name(m_categ_id, m_subcateg_id));

    is_similar_category_status = false;
    if (j_doc.HasMember("SIMILAR_YN") && j_doc["SIMILAR_YN"].IsBool())
    {
        is_similar_category_status = j_doc["SIMILAR_YN"].GetBool();
    }
    similarCategCheckBox_->SetValue(is_similar_category_status);
    similarCategCheckBox_->Enable(categoryCheckBox_->IsChecked());

    //Status
    Value& j_status = GetValueByPointerWithDefault(j_doc, "/STATUS", "");
    const wxString& s_status = j_status.IsString() ? wxString::FromUTF8(j_status.GetString()) : "";
    statusCheckBox_->SetValue(!s_status.empty());
    choiceStatus_->Enable(statusCheckBox_->IsChecked());
    choiceStatus_->SetStringSelection(wxGetTranslation(s_status));

    //Type
    Value& j_type = GetValueByPointerWithDefault(j_doc, "/TYPE", "");
    const wxString& s_type = j_type.IsString() ? wxString::FromUTF8(j_type.GetString()) : "";
    typeCheckBox_->SetValue(!s_type.empty());
    cbTypeWithdrawal_->SetValue(s_type.Contains("W"));
    cbTypeWithdrawal_->Enable(typeCheckBox_->IsChecked());
    cbTypeDeposit_->SetValue(s_type.Contains("D"));
    cbTypeDeposit_->Enable(typeCheckBox_->IsChecked());
    cbTypeTransferTo_->SetValue(s_type.Contains("T"));
    cbTypeTransferTo_->Enable(typeCheckBox_->IsChecked());
    cbTypeTransferFrom_->SetValue(s_type.Contains("F"));
    cbTypeTransferFrom_->Enable(typeCheckBox_->IsChecked());

    //Amounts
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
    m_colour_value = 0;
    colourCheckBox_->SetValue(false);
    if (j_doc.HasMember("COLOR") && j_doc["COLOR"].IsInt()) {
        colourCheckBox_->SetValue(true);
        m_colour_value = j_doc["COLOR"].GetInt();
    }
    colourButton_->Enable(colourCheckBox_->IsChecked());
    colourButton_->SetBackgroundColour(getUDColour(m_colour_value));
    colourButton_->Refresh(); // Needed as setting the background colour does not cause an immediate refresh

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
    } else
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

void mmFilterTransactionsDialog::OnDateChanged(wxDateEvent& event)
{
    switch (event.GetId())
    {
    case wxID_FIRST: m_begin_date = event.GetDate().FormatISODate(); break;
    case wxID_LAST: m_end_date = event.GetDate().FormatISODate(); break;
    }
}

const wxArrayInt mmFilterTransactionsDialog::getAccountsID() const
{
    return m_selected_accounts_id;
}

const wxArrayInt mmFilterTransactionsDialog::getHideColumnsID() const
{
    return m_selected_columns_id;
}

bool mmFilterTransactionsDialog::is_status_cb_active() const
{
    return statusCheckBox_->IsChecked();
}

bool mmFilterTransactionsDialog::is_account_cb_active() const
{
    return accountCheckBox_->GetValue() && !m_selected_accounts_id.empty();
}

bool mmFilterTransactionsDialog::getHideColumnsCheckBox() const
{
    return showColumnsCheckBox_->GetValue();
}

bool mmFilterTransactionsDialog::is_category_cb_active() const
{
    return categoryCheckBox_->IsChecked();
}

bool mmFilterTransactionsDialog::getSimilarStatus() const
{
    return is_similar_category_status;
}

int mmFilterTransactionsDialog::getCategId() const
{
    return m_categ_id;
}

int mmFilterTransactionsDialog::getSubCategId() const
{
    return m_subcateg_id;
}

bool mmFilterTransactionsDialog::is_date_range_cb_active() const
{
    return dateRangeCheckBox_->GetValue();
}

bool mmFilterTransactionsDialog::getRangeCheckBox() const
{
    return rangeCheckBox_->GetValue();
}

bool mmFilterTransactionsDialog::getStartDateCheckBox() const
{
    return startDateCheckBox_->GetValue();
}

bool mmFilterTransactionsDialog::is_amountrange_min_cb_active() const
{
    return amountRangeCheckBox_->GetValue() && !amountMinEdit_->GetValue().IsEmpty();
}

bool mmFilterTransactionsDialog::is_amount_range_max_cb_active() const
{
    return amountRangeCheckBox_->GetValue() && !amountMaxEdit_->GetValue().IsEmpty();
}

const wxString mmFilterTransactionsDialog::getNumber() const
{
    return transNumberEdit_->GetValue();
}

const wxString mmFilterTransactionsDialog::getNotes() const
{
    return notesEdit_->GetValue();
}

bool mmFilterTransactionsDialog::is_type_cb_active() const
{
    return typeCheckBox_->IsChecked();
}

bool mmFilterTransactionsDialog::is_payee_cb_active() const
{
    return payeeCheckBox_->IsChecked();
}

bool mmFilterTransactionsDialog::is_number_cb_active() const
{
    return transNumberCheckBox_->IsChecked();
}

bool mmFilterTransactionsDialog::is_notes_cb_active() const
{
    return notesCheckBox_->IsChecked();
}

bool mmFilterTransactionsDialog::is_colour_cb_active() const
{
    return colourCheckBox_->IsChecked();
}

bool mmFilterTransactionsDialog::is_custom_field_active() const
{
    const auto cf = m_custom_fields->GetActiveCustomFields();
    return (cf.size() > 0);
}

bool mmFilterTransactionsDialog::is_custom_field_matches(const Model_Checking::Data& tran) const
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

int mmFilterTransactionsDialog::getGroupBy() const
{
    int by = -1;
    if (groupByCheckBox_->IsChecked())
        by = bGroupBy_->GetSelection();
    return by;
}

void mmFilterTransactionsDialog::ResetFilterStatus()
{
    //m_custom_fields->ResetWidgetsChanged();
}

void mmFilterTransactionsDialog::OnSettingsSelected(wxCommandEvent& event)
{
    int sel = event.GetSelection();
    int count = m_setting_name->GetCount();
    if (count > 0)
    {
        wxStringClientData* settings_obj =
            static_cast<wxStringClientData*>(m_setting_name->GetClientObject(sel));
        if (settings_obj)
            m_settings_json = settings_obj->GetData();

        dataToControls();
    }
}

void mmFilterTransactionsDialog::OnSaveSettings(wxCommandEvent& WXUNUSED(event))
{
    auto label = m_setting_name->GetStringSelection();

    wxArrayString label_names;
    for (unsigned int i = 0; i < m_setting_name->GetCount(); i++)
    {
        label_names.Add(m_setting_name->GetString(i));
    }

    if (label_names.empty() && label.empty())
    {
        label_names.Add("");
        label = "";
    }

    while (label_names.Index(label) != wxNOT_FOUND)
    {
        label = wxGetTextFromUser(_("Please Enter"), _("Setting Name"), label);
    }

    if (!label.empty())
    {
        m_setting_name->Append(label);
        m_setting_name->SetStringSelection(label);
        m_settings_json = GetJsonSetings();
        Model_Infotable::instance().Prepend("TRANSACTIONS_FILTER", m_settings_json, -1);

        SetSettingsLabel();
    }
}

void mmFilterTransactionsDialog::OnAccountsButton(wxCommandEvent& WXUNUSED(event))
{
    wxMultiChoiceDialog s_acc(this, _("Choose Accounts")
       , "" , m_accounts_name);

    wxButton* ok = static_cast<wxButton*>(s_acc.FindWindow(wxID_OK));
    if (ok) ok->SetLabel(_("&OK "));
    wxButton* ca = static_cast<wxButton*>(s_acc.FindWindow(wxID_CANCEL));
    if (ca) ca->SetLabel(wxGetTranslation(g_CancelLabel));

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
        bSelectedAccounts_->SetLabelText("");
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

void mmFilterTransactionsDialog::OnColourButton(wxCommandEvent& /*event*/)
{
    wxMenu* mainMenu = new wxMenu;

    wxMenuItem* menuItem = new wxMenuItem(mainMenu, wxID_HIGHEST, wxString::Format(_("Not colored"), 0));
    mainMenu->Append(menuItem);

    for (int i = 1; i <= 7; ++i)
    {
        menuItem = new wxMenuItem(mainMenu, wxID_HIGHEST + i, wxString::Format(_("Color #%i"), i));
#ifdef __WXMSW__
        menuItem->SetBackgroundColour(getUDColour(i)); //only available for the wxMSW port.
#endif
        wxBitmap bitmap(mmBitmap(png::EMPTY, mmBitmapButtonSize).GetSize());
        wxMemoryDC memoryDC(bitmap);
        wxRect rect(memoryDC.GetSize());

        memoryDC.SetBackground(wxBrush(getUDColour(i)));
        memoryDC.Clear();
        memoryDC.DrawBitmap(mmBitmap(png::EMPTY, mmBitmapButtonSize), 0, 0, true);
        memoryDC.SelectObject(wxNullBitmap);
        menuItem->SetBitmap(bitmap);

        mainMenu->Append(menuItem);
    }

    PopupMenu(mainMenu);
    delete mainMenu;
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
