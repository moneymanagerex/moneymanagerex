/*******************************************************
Copyright (C) 2006 Madhan Kanagavel
Copyright (C) 2016 - 2018 Nikolay Akimov
Copyright (C) 2018 Stefano Giorgio

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
#include "mmcustomdata.h"
#include "mmTextCtrl.h"
#include "option.h"
#include "reports/htmlbuilder.h"
#include "reports/mmDateRange.h"

#include "Model_Payee.h"
#include "Model_Setting.h"
#include "Model_Infotable.h"

static const wxString TRANSACTION_STATUSES[] =
{
    wxTRANSLATE("None"),
    wxTRANSLATE("Reconciled"),
    wxTRANSLATE("Void"),
    wxTRANSLATE("Follow up"),
    wxTRANSLATE("Duplicate"),
    wxTRANSLATE("Un-Reconciled"),
    wxTRANSLATE("All Except Reconciled")
};

wxIMPLEMENT_DYNAMIC_CLASS(mmFilterTransactionsDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmFilterTransactionsDialog, wxDialog)
EVT_CHECKBOX(wxID_ANY, mmFilterTransactionsDialog::OnCheckboxClick)
EVT_CHOICE(ID_CHOICE_DATE_RANGE, mmFilterTransactionsDialog::OnDateRangeChanged)
EVT_BUTTON(wxID_OK, mmFilterTransactionsDialog::OnButtonokClick)
EVT_BUTTON(wxID_CANCEL, mmFilterTransactionsDialog::OnButtoncancelClick)
EVT_BUTTON(wxID_CLEAR, mmFilterTransactionsDialog::OnButtonClearClick)
EVT_BUTTON(wxID_MORE, mmFilterTransactionsDialog::OnMoreFields)
wxEND_EVENT_TABLE()

mmFilterTransactionsDialog::mmFilterTransactionsDialog()
{
}

mmFilterTransactionsDialog::~mmFilterTransactionsDialog()
{
    if (m_custom_fields)
        delete m_custom_fields;
    
    std::for_each(m_all_date_ranges.begin(), m_all_date_ranges.end(), std::mem_fun(&mmDateRange::destroy));
    m_all_date_ranges.clear();
}

mmFilterTransactionsDialog::mmFilterTransactionsDialog(wxWindow* parent, int account_id)
    : categID_(-1)
    , subcategID_(-1)
    , bSimilarCategoryStatus_(false)
    , payeeID_(-1)
    , refAccountID_(account_id)
    , refAccountStr_("")
    , m_filterStatus("")
    , m_min_amount(0)
    , m_max_amount(0)
    , m_settings_id(-1)
{
    int day = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_DAY", 1);
    int month = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_MONTH", 7);

    m_all_date_ranges.push_back(new mmCurrentMonth());
    m_all_date_ranges.push_back(new mmCurrentMonthToDate());
    m_all_date_ranges.push_back(new mmLastMonth());
    m_all_date_ranges.push_back(new mmLast30Days());
    m_all_date_ranges.push_back(new mmLast90Days());
    m_all_date_ranges.push_back(new mmLast3Months());
    m_all_date_ranges.push_back(new mmLast12Months());
    m_all_date_ranges.push_back(new mmCurrentYear());
    m_all_date_ranges.push_back(new mmCurrentYearToDate());
    m_all_date_ranges.push_back(new mmLastYear());
    m_all_date_ranges.push_back(new mmCurrentFinancialYear(day, month));
    m_all_date_ranges.push_back(new mmCurrentFinancialYearToDate(day, month));
    m_all_date_ranges.push_back(new mmLastFinancialYear(day, month));
    m_all_date_ranges.push_back(new mmAllTime());
    m_all_date_ranges.push_back(new mmLast365Days());
    m_all_date_ranges.push_back(new mmSpecifiedRange(wxDate::Today(), wxDate::Today()));

    SetSettingsID(Model_Infotable::instance().GetIntInfo("TRANSACTIONS_FILTER_VIEW_NO", 0));
    m_custom_fields = new mmCustomDataTransaction(this, 0, wxID_HIGHEST + 11600);
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, _("Transaction Filter"), wxDefaultPosition, wxSize(400, 300), style);
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
    GetStoredSettings(-1);

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
    dataToControls();
    // rebuild the payee list as it may have changed
    BuildPayeeList();

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
    BuildPayeeList();
    from_json(settings_string_);
}

void mmFilterTransactionsDialog::CreateControls()
{

    wxBoxSizer* box_sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(box_sizer);

    wxBoxSizer* box_sizer1 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* box_sizer2 = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* box_sizer3 = new wxBoxSizer(wxVERTICAL);
    box_sizer->Add(box_sizer1, g_flagsExpand);
    box_sizer1->Add(box_sizer2, g_flagsExpand);
    box_sizer1->Add(box_sizer3, g_flagsExpand);

    wxStaticBox* static_box_sizer = new wxStaticBox(this, wxID_ANY, _("Specify"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(static_box_sizer, wxVERTICAL);
    box_sizer2->Add(itemStaticBoxSizer4, 1, wxGROW | wxALL, 10);

    /******************************************************************************
     Items Panel
    *******************************************************************************/
    wxPanel* itemPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemStaticBoxSizer4->Add(itemPanel, g_flagsExpand);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* itemPanelSizer = new wxFlexGridSizer(0, 2, 0, 0);
    itemPanelSizer->AddGrowableCol(1, 1);

    itemPanel->SetSizer(itemBoxSizer4);
    itemBoxSizer4->Add(itemPanelSizer, g_flagsExpand);

    //--Start of Row --------------------------------------------------------
    accountCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Account")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(accountCheckBox_, g_flagsH);

    accountDropDown_ = new wxChoice(itemPanel
        , wxID_ANY, wxDefaultPosition, wxSize(220, -1)
        , Model_Account::instance().all_checking_account_names(), 0);
    itemPanelSizer->Add(accountDropDown_, g_flagsExpand);

    //--End of Row --------------------------------------------------------

    m_dateRangeCheckBox = new wxCheckBox(itemPanel, wxID_ANY, _("Date Range")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(m_dateRangeCheckBox, g_flagsH);

    m_date_ranges = new wxChoice(itemPanel, ID_CHOICE_DATE_RANGE);
    for (const auto & date_range : m_all_date_ranges)
    {
        m_date_ranges->Append(date_range->local_title(), date_range);
    }
    itemPanelSizer->Add(m_date_ranges, g_flagsH);
    itemPanelSizer->AddSpacer(1);

    m_fromDateCtrl = new wxDatePickerCtrl(itemPanel, wxID_FIRST, wxDefaultDateTime
        , wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
    m_toDateControl = new wxDatePickerCtrl(itemPanel, wxID_LAST, wxDefaultDateTime
        , wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);

    wxBoxSizer* dateSizer = new wxBoxSizer(wxHORIZONTAL);
    dateSizer->Add(m_fromDateCtrl, g_flagsExpand);
    dateSizer->AddSpacer(5);
    dateSizer->Add(m_toDateControl, g_flagsExpand);
    itemPanelSizer->Add(dateSizer, wxSizerFlags(g_flagsExpand).Border(0));
    //--End of Row --------------------------------------------------------

    payeeCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Payee")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(payeeCheckBox_, g_flagsH);

    cbPayee_ = new wxComboBox(itemPanel, wxID_ANY, "", wxDefaultPosition, wxSize(220, -1));
    cbPayee_->Connect(wxID_ANY, wxEVT_COMMAND_TEXT_UPDATED
        , wxCommandEventHandler(mmFilterTransactionsDialog::OnPayeeUpdated), nullptr, this);

    itemPanelSizer->Add(cbPayee_, g_flagsExpand);
    //--End of Row --------------------------------------------------------

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
    similarCategCheckBox_->SetToolTip(_("Include all subcategories for the selected category."));

    categSizer->Add(btnCategory_, g_flagsExpand);
    categSizer->Add(similarCategCheckBox_, wxSizerFlags(g_flagsH).Center().Border(0));
    categSizer->AddSpacer(5);
    //--End of Row --------------------------------------------------------

    statusCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Status")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(statusCheckBox_, g_flagsH);

    choiceStatus_ = new wxChoice(itemPanel, wxID_ANY);

    for (const auto& i : TRANSACTION_STATUSES)
        choiceStatus_->Append(wxGetTranslation(i), new wxStringClientData(i));

    itemPanelSizer->Add(choiceStatus_, g_flagsExpand);
    choiceStatus_->SetToolTip(_("Specify the status for the transaction"));
    //--End of Row --------------------------------------------------------

    typeCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Type")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);

    wxFlexGridSizer* typeSizer = new wxFlexGridSizer(0, 2, 0, 0);
    typeSizer->AddGrowableCol(0, 1);
    typeSizer->AddGrowableCol(1, 1);
    cbTypeWithdrawal_ = new wxCheckBox(itemPanel, wxID_ANY, _("Withdrawal")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cbTypeDeposit_ = new wxCheckBox(itemPanel, wxID_ANY, _("Deposit")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cbTypeTransferTo_ = new wxCheckBox(itemPanel, wxID_ANY, _("Transfer To")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    cbTypeTransferFrom_ = new wxCheckBox(itemPanel, wxID_ANY, _("Transfer From")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);

    itemPanelSizer->Add(typeCheckBox_, g_flagsH);
    itemPanelSizer->Add(typeSizer, g_flagsExpand);
    typeSizer->Add(cbTypeWithdrawal_, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    typeSizer->Add(cbTypeDeposit_, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    typeSizer->Add(cbTypeTransferTo_, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    typeSizer->Add(cbTypeTransferFrom_, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    typeSizer->AddSpacer(2);

    //--End of Row --------------------------------------------------------

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
    //--End of Row --------------------------------------------------------

    transNumberCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Number")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(transNumberCheckBox_, g_flagsH);

    transNumberEdit_ = new wxTextCtrl(itemPanel, wxID_ANY);
    itemPanelSizer->Add(transNumberEdit_, g_flagsExpand);
    //--End of Row --------------------------------------------------------

    notesCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Notes")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(notesCheckBox_, g_flagsH);

    notesEdit_ = new wxTextCtrl(itemPanel, wxID_ANY);
    itemPanelSizer->Add(notesEdit_, g_flagsExpand);
    //--End of Row --------------------------------------------------------

    wxBoxSizer* settings_sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* settings_box_sizer = new wxBoxSizer(wxHORIZONTAL);
    settings_sizer->Add(settings_box_sizer, wxSizerFlags(g_flagsExpand).Border(wxALL, 0));
    
    wxStaticText* settings = new wxStaticText(this, wxID_ANY, _("Settings"));
    settings_box_sizer->Add(settings, g_flagsH);
    settings_box_sizer->AddSpacer(5);
    
    m_setting_name = new wxChoice(this, wxID_APPLY);
    settings_box_sizer->Add(m_setting_name, g_flagsExpand);

    for (int i = 0 ; i < 10 ; i++)
    {
        const wxString& data = Model_Infotable::instance().GetStringInfo(
            wxString::Format("TRANSACTIONS_FILTER_%d", i)
            , "");
        Document j_doc;
        if (j_doc.Parse(data.c_str()).HasParseError()) {
            j_doc.Parse("{}");
        }

        //Label
        Value& j_label = GetValueByPointerWithDefault(j_doc, "/LABEL", "");
        const wxString& s_label = j_label.IsString() ? j_label.GetString() : "";

        m_setting_name->AppendString(s_label.empty() ? wxString::Format(_("%i: Empty"), i + 1) : s_label);
    }
    m_setting_name->Select(0);
    m_setting_name->Connect(wxID_APPLY, wxEVT_COMMAND_CHOICE_SELECTED
        , wxCommandEventHandler(mmFilterTransactionsDialog::OnSettingsSelected), nullptr, this);

    settings_box_sizer->AddSpacer(5);
    m_btnSaveAs = new wxBitmapButton(this, wxID_SAVEAS, mmBitmap(png::SAVE));
    settings_box_sizer->Add(m_btnSaveAs, g_flagsH);
    m_btnSaveAs->SetToolTip(_("Save active values into current Preset selection"));
    m_btnSaveAs->Connect(wxID_SAVEAS, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmFilterTransactionsDialog::OnSaveSettings), nullptr, this);

    wxBitmapButton* itemButtonClear = new wxBitmapButton(this, wxID_CLEAR, mmBitmap(png::CLEAR));
    itemButtonClear->SetToolTip(_("Clear all fields for current Preset selection"));
    settings_box_sizer->Add(itemButtonClear, g_flagsH);

    box_sizer2->Add(settings_sizer, wxSizerFlags(g_flagsExpand).Border(wxALL, 0).Proportion(0));

    /******************************************************************************
     Button Panel with OK/Cancel buttons
    *******************************************************************************/
    wxPanel* buttonPanel = new wxPanel(this, wxID_ANY);
    box_sizer2->Add(buttonPanel, wxSizerFlags(g_flagsV).Center());

    wxBoxSizer* buttonPanelSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonPanel->SetSizer(buttonPanelSizer);

    wxButton* itemButtonOK = new wxButton(buttonPanel, wxID_OK, _("&Activate "));
    itemButtonOK->SetToolTip(_("Activate the filter with these settings"));

    wxButton* itemButtonCancel = new wxButton(buttonPanel, wxID_CANCEL, _("&Deactivate "));
    itemButtonCancel->SetToolTip(_("Deactivate the filter"));
    itemButtonCancel->SetFocus();

    wxBitmapButton* itemButtonHide = new wxBitmapButton(buttonPanel
        , wxID_MORE, mmBitmap(png::RIGHTARROWSIMPLE));
    itemButtonHide->SetToolTip(_("Show/Hide custom fields window"));
    if (m_custom_fields->GetCustomFieldsCount() == 0) {
        itemButtonHide->Hide();
    }

    buttonPanelSizer->Add(itemButtonOK, g_flagsH);
    buttonPanelSizer->Add(itemButtonCancel, g_flagsH);
    buttonPanelSizer->Add(itemButtonHide, g_flagsH);

    // Custom fields -----------------------------------

    m_custom_fields->FillCustomFields(box_sizer3);
    if (m_custom_fields->GetActiveCustomFieldsCount() > 0) {
        wxCommandEvent evt(wxEVT_BUTTON, wxID_MORE);
        this->GetEventHandler()->AddPendingEvent(evt);
    }

    Center();
}

void mmFilterTransactionsDialog::OnCheckboxClick(wxCommandEvent& event)
{
    if (event.GetId() == similarCategCheckBox_->GetId())
    {
        bSimilarCategoryStatus_ = similarCategCheckBox_->IsChecked();
    }
    else if (event.GetId() != cbTypeWithdrawal_->GetId() &&
        event.GetId() != cbTypeDeposit_->GetId() &&
        event.GetId() != cbTypeTransferTo_->GetId() &&
        event.GetId() != cbTypeTransferFrom_->GetId())
    {
        accountDropDown_->Enable(accountCheckBox_->IsChecked());
        cbPayee_->Enable(payeeCheckBox_->IsChecked());
        btnCategory_->Enable(categoryCheckBox_->IsChecked());
        similarCategCheckBox_->Enable(categoryCheckBox_->IsChecked());
        choiceStatus_->Enable(statusCheckBox_->IsChecked());
        cbTypeWithdrawal_->Enable(typeCheckBox_->IsChecked());
        cbTypeDeposit_->Enable(typeCheckBox_->IsChecked());
        cbTypeTransferTo_->Enable(typeCheckBox_->IsChecked());
        cbTypeTransferFrom_->Enable(typeCheckBox_->IsChecked());
        amountMinEdit_->Enable(amountRangeCheckBox_->IsChecked());
        amountMaxEdit_->Enable(amountRangeCheckBox_->IsChecked());
        notesEdit_->Enable(notesCheckBox_->IsChecked());
        transNumberEdit_->Enable(transNumberCheckBox_->IsChecked());
        m_date_ranges->Enable(m_dateRangeCheckBox->IsChecked());
    }

    if (accountCheckBox_->IsChecked() && accountDropDown_->GetSelection() < 0) {
        accountDropDown_->SetSelection(0);
    }

    event.Skip();
}

void mmFilterTransactionsDialog::OnButtonokClick(wxCommandEvent& WXUNUSED(event))
{
    if (accountCheckBox_->IsChecked())
    {
        refAccountStr_ = accountDropDown_->GetStringSelection();
        Model_Account::Data* account = Model_Account::instance().get(refAccountStr_);
        if (account)
            refAccountID_ = account->ACCOUNTID;
    }
    else
    {
        refAccountID_ = -1;
    }

    if (payeeCheckBox_->IsChecked())
    {
        Model_Payee::Data* payee = Model_Payee::instance().get(cbPayee_->GetValue());
        if (payee)
        {
            payeeID_ = payee->PAYEEID;
            payeeStr_ = payee->PAYEENAME;
        }
    }

    if (amountRangeCheckBox_->IsChecked())
    {
        m_min_amount = m_max_amount = 0;

        if (!amountMinEdit_->GetValue().Trim().IsEmpty()
            && !amountMinEdit_->checkValue(m_min_amount, true))
        {
            return mmErrorDialogs::ToolTip4Object(amountMinEdit_
                , _("Invalid value"), _("Amount"));
        }

        if (!amountMaxEdit_->GetValue().Trim().IsEmpty()
            && !amountMaxEdit_->checkValue(m_max_amount, true))

        {
            return mmErrorDialogs::ToolTip4Object(amountMaxEdit_
                , _("Invalid value"), _("Amount"));
        }
    }

    if (m_dateRangeCheckBox->IsChecked())
    {
        m_begin_date = m_fromDateCtrl->GetValue().FormatISODate();
        m_end_date = m_toDateControl->GetValue().FormatISODate();
        if (m_begin_date > m_end_date)
        {
            const auto today = wxDate::Today().FormatISODate();
            int id = m_begin_date >= today
                ? m_fromDateCtrl->GetId() : m_toDateControl->GetId();
            return mmErrorDialogs::ToolTip4Object(FindWindow(id)
                , _("Invalid value"), _("Date"));
        }
    }

    if (statusCheckBox_->IsChecked())
    {
        if (choiceStatus_->GetSelection() < 0)
        {
            int id = choiceStatus_->GetId();
            return mmErrorDialogs::ToolTip4Object(FindWindow(id)
                , _("Invalid value"), _("Status"));
        }
        getFilterStatus();
        wxLogDebug(m_filterStatus);
    }
    //SaveSettings();
    EndModal(wxID_OK);
}

void mmFilterTransactionsDialog::OnButtoncancelClick(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_CANCEL);
}

void mmFilterTransactionsDialog::OnCategs(wxCommandEvent& WXUNUSED(event))
{
    Model_Category::Data* category = Model_Category::instance().get(categID_);
    Model_Subcategory::Data* sub_category = Model_Subcategory::instance().get(subcategID_);
    int categID = category ? category->CATEGID : -1;
    int subcategID = sub_category ? sub_category->SUBCATEGID : -1;
    mmCategDialog dlg(this, categID, subcategID, false);

    if (dlg.ShowModal() == wxID_OK)
    {
        categID_ = dlg.getCategId();
        subcategID_ = dlg.getSubCategId();
        category = Model_Category::instance().get(categID_);
        sub_category = Model_Subcategory::instance().get(subcategID_);

        btnCategory_->SetLabelText(Model_Category::full_name(category, sub_category));
    }
}

void mmFilterTransactionsDialog::ResetFilterStatus()
{
    m_custom_fields->ResetWidgetsChanged();
    wxCommandEvent evt(wxEVT_BUTTON, wxID_MORE);
    this->GetEventHandler()->AddPendingEvent(evt);
}

bool mmFilterTransactionsDialog::SomethingSelected()
{
    return
        getAccountCheckBox()
        || getDateRangeCheckBox()
        || getPayeeCheckBox()
        || getCategoryCheckBox()
        || getStatusCheckBox()
        || getTypeCheckBox()
        || getAmountRangeCheckBox()
        || getNumberCheckBox()
        || getNotesCheckBox()
        || m_custom_fields->IsSomeWidgetChanged();
}

void mmFilterTransactionsDialog::getFilterStatus()
{
    m_filterStatus.clear();
    int item = choiceStatus_->GetSelection();
    if (!getStatusCheckBox() || item < 0) return;
    wxStringClientData* status_obj =
        (wxStringClientData*)choiceStatus_->GetClientObject(item);
    if (status_obj) {
        m_filterStatus = status_obj->GetData().Left(1);
    }
}

bool mmFilterTransactionsDialog::compareStatus(const wxString& itemStatus, const wxString& filterStatus) const
{

    if ((itemStatus.empty() || itemStatus.Contains("N")) && filterStatus == "N")
    {
        return true;
    }
    else if ((itemStatus.empty() || "N" == itemStatus || "F" == itemStatus) && filterStatus == "U") // Un-Reconciled
    {
        return true;
    }
    else if ((itemStatus != "R" && itemStatus != "RR" ) && filterStatus == "A") // All Except Reconciled
    {
        return true;
    }
    return itemStatus.Contains(filterStatus);
}

bool mmFilterTransactionsDialog::allowType(const wxString& typeState, bool sameAccount) const
{
    bool result = false;
    if (typeState == Model_Checking::all_type()[Model_Checking::TRANSFER]
        && cbTypeTransferTo_->GetValue() && sameAccount)
    {
        result = true;
    }
    else if (typeState == Model_Checking::all_type()[Model_Checking::TRANSFER]
        && cbTypeTransferFrom_->GetValue() && !sameAccount)
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

template<class MODEL, class FULL_DATA>
bool mmFilterTransactionsDialog::checkAmount(const FULL_DATA& tran)
{
    bool ok = true, split_ok = false;
    if (!amountMinEdit_->GetValue().IsEmpty() && m_min_amount > tran.TRANSAMOUNT)
        ok = false;
    else if (!amountMaxEdit_->GetValue().IsEmpty() && m_max_amount < tran.TRANSAMOUNT)
        ok = false;

    if (tran.has_split())
    {
        for (const auto s : tran.m_splits)
        {
            if ((amountMinEdit_->GetValue().IsEmpty() || m_min_amount <= s.SPLITTRANSAMOUNT)
                && (amountMaxEdit_->GetValue().IsEmpty() || m_max_amount >= s.SPLITTRANSAMOUNT))
            {
                split_ok = true;
                break;
            }
        }
    }

    return ok || split_ok;
}

void mmFilterTransactionsDialog::OnButtonClearClick(wxCommandEvent& WXUNUSED(event))
{
    clearSettings();
    wxCommandEvent evt(/*wxEVT_CHECKBOX*/ wxID_ANY, wxID_ANY);
    OnCheckboxClick(evt);
}

wxString mmFilterTransactionsDialog::GetStoredSettings(int id)
{
    if (id < 0) id = 0;
    Model_Setting::instance().Set("TRANSACTIONS_FILTER_VIEW_NO", id);
    settings_string_ = Model_Infotable::instance().GetStringInfo(
        wxString::Format("TRANSACTIONS_FILTER_%d", id)
        , "");
    return settings_string_;
}

void mmFilterTransactionsDialog::setAccountToolTip(const wxString& tip) const
{
    accountDropDown_->SetToolTip(tip);
}

void mmFilterTransactionsDialog::clearSettings()
{
    int i = m_setting_name->GetSelection();
    const wxString& default_label = wxString::Format(_("%i: Empty"), i + 1);
    m_setting_name->SetString(i, default_label);

    settings_string_.Clear();
    m_custom_fields->ClearSettings();
    dataToControls();

    // Clear the settings for the allocated position
    SaveSettings();
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

template<class MODEL, class FULL_DATA>
bool mmFilterTransactionsDialog::checkPayee(const FULL_DATA &tran)
{
    const Model_Payee::Data* payee = Model_Payee::instance().get(tran.PAYEEID);
    if (payee)
        return cbPayee_->GetValue().Lower() == (payee->PAYEENAME).Lower();
    return false;
}

template<class MODEL, class FULL_DATA>
bool mmFilterTransactionsDialog::checkCategory(const FULL_DATA& tran)
{
    if (!tran.has_split())
    {
        if (categID_ != tran.CATEGID) return false;
        if (subcategID_ != tran.SUBCATEGID && !bSimilarCategoryStatus_) return false;
    }
    else
    {
        bool bMatching = false;
        for (const auto &split : tran.m_splits)
        {
            if (split.CATEGID != categID_) continue;
            if (split.SUBCATEGID != subcategID_ && !bSimilarCategoryStatus_) continue;

            bMatching = true;
            break;
        }
        if (!bMatching) return false;
    }
    return true;
}

bool mmFilterTransactionsDialog::checkAll(const Model_Checking::Full_Data &tran, int accountID)
{
    bool ok = true;
    //wxLogDebug("Check date? %i trx date:%s %s %s", getDateRangeCheckBox(), tran.TRANSDATE, getFromDateCtrl().GetDateOnly().FormatISODate(), getToDateControl().GetDateOnly().FormatISODate());
    if (getAccountCheckBox() && (getAccountID() != tran.ACCOUNTID && getAccountID() != tran.TOACCOUNTID))
        ok = false;
    else if (getDateRangeCheckBox() && !(tran.TRANSDATE >= m_begin_date && tran.TRANSDATE <= m_end_date))
        ok = false;
    else if (getPayeeCheckBox() && !checkPayee<Model_Checking>(tran))
        ok = false;
    else if (getCategoryCheckBox() && !checkCategory<Model_Checking>(tran))
        ok = false;
    else if (getStatusCheckBox() && !compareStatus(tran.STATUSFD, m_filterStatus))
        ok = false;
    else if (getTypeCheckBox() && !allowType(tran.TRANSCODE, accountID == tran.ACCOUNTID))
        ok = false;
    else if (getAmountRangeCheckBox() && !checkAmount<Model_Checking>(tran))
        ok = false;
    else if (getNumberCheckBox() && getNumber() != tran.TRANSACTIONNUMBER)
        ok = false;
    else if (getNotesCheckBox() && !tran.NOTES.Lower().Contains(getNotes().Lower()))
        ok = false;
    else if (m_custom_fields->IsSomeWidgetChanged() && !m_custom_fields->IsDataFound(tran))
        ok = false;
    return ok;
}
bool mmFilterTransactionsDialog::checkAll(const Model_Billsdeposits::Full_Data &tran)
{
    bool ok = true;
    if (getAccountCheckBox() && (getAccountID() != tran.ACCOUNTID && getAccountID() != tran.TOACCOUNTID))
        ok = false;
    else if (getDateRangeCheckBox() && (tran.TRANSDATE < m_begin_date && tran.TRANSDATE > m_end_date))
        ok = false;
    else if (getPayeeCheckBox() && !checkPayee<Model_Billsdeposits>(tran))
        ok = false;
    else if (getCategoryCheckBox() && !checkCategory<Model_Billsdeposits>(tran))
        ok = false;
    else if (getStatusCheckBox() && !compareStatus(tran.STATUS, m_filterStatus))
        ok = false;
    else if (getTypeCheckBox() && !allowType(tran.TRANSCODE, true))
        ok = false;
    else if (getAmountRangeCheckBox() && !checkAmount<Model_Billsdeposits>(tran))
        ok = false;
    else if (getNumberCheckBox() && getNumber() != tran.TRANSACTIONNUMBER)
        ok = false;
    else if (getNotesCheckBox() && !tran.NOTES.Lower().Contains(getNotes().Lower()))
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

void mmFilterTransactionsDialog::getDescription(mmHTMLBuilder &hb)
{
    hb.addHorizontalLine();
    hb.addHeader(3, _("Filtering Details: "));
    // Extract the parameters from the transaction dialog and add them to the report.
    wxString filterDetails = to_json(true);
    filterDetails.Replace(",\n", "<br>");
    filterDetails.Replace("\"", "");
    filterDetails.replace(0, 1, ' ');
    filterDetails.RemoveLast(1);
    hb.addText(filterDetails);
}

wxString mmFilterTransactionsDialog::to_json(bool i18n)
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();

    if (accountCheckBox_->IsChecked())
    {
        const wxString acc = accountDropDown_->GetStringSelection();
        if (!acc.empty())
        {
            json_writer.Key(wxString(i18n ? _("Account") : "ACCOUNT").c_str());
            json_writer.String(acc.c_str());
        }
    }

    if (m_dateRangeCheckBox->IsChecked())
    {
        int i = m_date_ranges->GetSelection();
        const auto& title = m_all_date_ranges[i]->title();
        if (!title.empty()) {
            json_writer.Key(wxString(i18n ? _("Date") : "DATE").c_str());
            json_writer.String(wxString(i18n ? wxGetTranslation(title) : title).c_str());
        }

        json_writer.Key(wxString(i18n ? _("Since") : "DATE1").c_str());
        json_writer.String(m_fromDateCtrl->GetValue().FormatISODate().c_str());
        json_writer.Key(wxString(i18n ? _("Before") : "DATE2").c_str());
        json_writer.String(m_toDateControl->GetValue().FormatISODate().c_str());
    }

    if (payeeCheckBox_->IsChecked())
    {
        json_writer.Key(wxString(i18n ? _("Payee") : "PAYEE").c_str());
        json_writer.String(cbPayee_->GetValue().c_str());
    }

    if (categoryCheckBox_->IsChecked())
    {
        json_writer.Key(wxString(i18n ? _("Include Similar") : "SIMILAR_YN").c_str());
        json_writer.Bool(bSimilarCategoryStatus_);
        json_writer.Key(wxString(i18n ? _("Category") : "CATEGORY").c_str());
        json_writer.String(btnCategory_->GetLabel().c_str());
    }

    if (statusCheckBox_->IsChecked())
    {
        wxArrayString s = Model_Checking::all_status();
        s.Add(wxTRANSLATE("Un-Reconciled"));
        s.Add(wxTRANSLATE("All Except Reconciled"));
        int item = choiceStatus_->GetSelection();
        wxString status;
        if (0 <= item && item < (int)s.size())
            status = s[item];
        if (!status.empty())
        {
            json_writer.Key(wxString(i18n ? _("Status") : "STATUS").c_str());
            json_writer.String(wxString(i18n ? wxGetTranslation(status) : status).c_str());
        }
    }

    if (typeCheckBox_->IsChecked())
    {
        wxString type = wxString()
            << (cbTypeWithdrawal_->GetValue() && typeCheckBox_->GetValue() ? "W" : "")
            << (cbTypeDeposit_->GetValue() && typeCheckBox_->GetValue() ? "D" : "")
            << (cbTypeTransferTo_->GetValue() && typeCheckBox_->GetValue() ? "T" : "")
            << (cbTypeTransferFrom_->GetValue() && typeCheckBox_->GetValue() ? "F" : "");
        if (!type.empty())
        {
            json_writer.Key(wxString(i18n ? _("Type") : "TYPE").c_str());
            json_writer.String(type.c_str());
        }
    }

    if (amountRangeCheckBox_->IsChecked())
    {
        if (!amountMinEdit_->GetValue().empty())
        {
            double amount_min;
            amountMinEdit_->GetDouble(amount_min);
            json_writer.Key(wxString(i18n ? _("Amount Min.") : "AMOUNT_MIN").c_str());
            json_writer.Double(amount_min);
        }

        if (!amountMaxEdit_->GetValue().empty())
        {
            double amount_max;
            amountMaxEdit_->GetDouble(amount_max);
            json_writer.Key(wxString(i18n ? _("Amount Max.") : "AMOUNT_MAX").c_str());
            json_writer.Double(amount_max);
        }
    }

    if (transNumberCheckBox_->IsChecked())
    {
        const wxString num = transNumberEdit_->GetValue();
        if (!num.empty())
        {
            json_writer.Key(wxString(i18n ? _("Number") : "NUMBER").c_str());
            json_writer.String(num.c_str());
        }
    }

    if (notesCheckBox_->IsChecked())
    {
        wxString notes = notesEdit_->GetValue();
        if (!notes.empty())
        {
            json_writer.Key(wxString(i18n ? _("Notes") : "NOTES").c_str());
            json_writer.String(notes.c_str());
        }
    }

    for (const auto& entry : m_custom_fields->GetActiveCustomFields())
    {
        json_writer.Key(entry.first.c_str());
        json_writer.Key(entry.second.c_str());
    }

    const wxString label = m_setting_name->GetStringSelection();
    const wxString default_label = wxString::Format(_("%i: Empty"), m_setting_name->GetSelection() + 1);
    if (!label.empty() && label != default_label)
    {
        json_writer.Key(wxString(i18n ? _("Label") : "LABEL").c_str());
        json_writer.String(label.c_str());
    }

    json_writer.EndObject();

    return json_buffer.GetString();
}

void mmFilterTransactionsDialog::from_json(const wxString &data)
{
    Document j_doc;
    if (j_doc.Parse(data.c_str()).HasParseError()) {
        j_doc.Parse("{}");
    }

    //Label
    Value& j_label = GetValueByPointerWithDefault(j_doc, "/LABEL", "");
    const wxString& s_label = j_label.IsString() ? j_label.GetString() : "";
    m_setting_name->SetStringSelection(s_label);

    //Account
    Value& j_account = GetValueByPointerWithDefault(j_doc, "/ACCOUNT", "");
    const wxString& s_account = j_account.IsString() ? j_account.GetString() : "";
    accountCheckBox_->SetValue(!s_account.empty());
    accountDropDown_->Enable(accountCheckBox_->IsChecked());
    accountDropDown_->SetStringSelection(s_account);

    //Dates
    Value& j_date = GetValueByPointerWithDefault(j_doc, "/DATE", "");
    const wxString& s_date = j_date.IsString() ? j_date.GetString() : "";
    m_date_ranges->SetStringSelection(wxGetTranslation(s_date));
    if (!s_date.empty())
    {
        wxCommandEvent evt;
        OnDateRangeChanged(evt);
    }

    bool custom = (s_date == "Custom");
    if (custom)
    {
        Value& j_date1 = GetValueByPointerWithDefault(j_doc, "/DATE1", "");
        m_begin_date = j_date1.IsString() ? j_date1.GetString() : "";
        Value& j_date2 = GetValueByPointerWithDefault(j_doc, "/DATE2", "");
        m_end_date = j_date2.IsString() ? j_date2.GetString() : "";
        m_fromDateCtrl->SetValue(mmParseISODate(m_begin_date));
        m_toDateControl->SetValue(mmParseISODate(m_end_date));
    }
    m_dateRangeCheckBox->SetValue(!s_date.empty());
    m_fromDateCtrl->Enable(m_dateRangeCheckBox->IsChecked() && custom);
    m_toDateControl->Enable(m_dateRangeCheckBox->IsChecked() && custom);
    m_date_ranges->Enable(m_dateRangeCheckBox->IsChecked());


    //Payee
    Value& j_payee = GetValueByPointerWithDefault(j_doc, "/PAYEE", "");
    const wxString& s_payee = j_payee.IsString() ? j_payee.GetString() : "";
    payeeCheckBox_->SetValue(!s_payee.empty());
    cbPayee_->Enable(payeeCheckBox_->IsChecked());
    cbPayee_->SetValue(s_payee);

    //Category
    Value& j_category = GetValueByPointerWithDefault(j_doc, "/CATEGORY", "");
    const wxString& s_category = j_category.IsString() ? j_category.GetString() : "";
    categoryCheckBox_->SetValue(!s_category.empty());
    btnCategory_->Enable(categoryCheckBox_->IsChecked());

    bSimilarCategoryStatus_ = false;
    if (j_doc.HasMember("SIMILAR_YN") && j_doc["SIMILAR_YN"].IsBool())
    {
        bSimilarCategoryStatus_ = j_doc["SIMILAR_YN"].GetBool();
    }
    similarCategCheckBox_->SetValue(bSimilarCategoryStatus_);
    similarCategCheckBox_->Enable(categoryCheckBox_->IsChecked());

    wxStringTokenizer categ_token(s_category, ":", wxTOKEN_RET_EMPTY_ALL);
    Model_Category::Data* category = Model_Category::instance().get(categ_token.GetNextToken().Trim());
    if (category)
    {
        categID_ = category->CATEGID;
    }
    Model_Subcategory::Data* sub_category = 0;
    const wxString& subcateg_name = categ_token.GetNextToken().Trim(false);
    if (!subcateg_name.IsEmpty())
    {
        sub_category = Model_Subcategory::instance().get(subcateg_name, categID_);
        if (sub_category)
            subcategID_ = sub_category->SUBCATEGID;
    }
    btnCategory_->SetLabelText(Model_Category::full_name(categID_, subcategID_));

    //Status
    Value& j_status = GetValueByPointerWithDefault(j_doc, "/STATUS", "");
    const wxString& s_status = j_status.IsString() ? j_status.GetString() : "";
    statusCheckBox_->SetValue(!s_status.empty());
    choiceStatus_->Enable(statusCheckBox_->IsChecked());
    choiceStatus_->SetStringSelection(wxGetTranslation(s_status));

    //Type
    Value& j_type = GetValueByPointerWithDefault(j_doc, "/TYPE", "");
    const wxString& s_type = j_type.IsString() ? j_type.GetString() : "";
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
    Value& j_number = GetValueByPointerWithDefault(j_doc, "/NUMBER", "");
    const wxString& s_number = j_number.IsString() ? j_number.GetString() : "";
    transNumberCheckBox_->SetValue(!s_number.empty());
    transNumberEdit_->Enable(transNumberCheckBox_->IsChecked());
    transNumberEdit_->ChangeValue(s_number);

    //Notes
    Value& j_notes = GetValueByPointerWithDefault(j_doc, "/NOTES", "");
    const wxString& s_notes = j_notes.IsString() ? j_notes.GetString() : "";

    notesCheckBox_->SetValue(!s_notes.empty());
    notesEdit_->Enable(notesCheckBox_->IsChecked());
    notesEdit_->ChangeValue(s_notes);
}

int mmFilterTransactionsDialog::getAccountID()
{
    return refAccountID_;
}

bool mmFilterTransactionsDialog::getStatusCheckBox()
{
    return statusCheckBox_->IsChecked();
}

bool mmFilterTransactionsDialog::getAccountCheckBox()
{
    const auto s = accountDropDown_->GetStringSelection();
    return accountCheckBox_->GetValue() && !s.empty();
}

bool mmFilterTransactionsDialog::getCategoryCheckBox()
{
    return categoryCheckBox_->IsChecked();
}

bool mmFilterTransactionsDialog::getSimilarStatus()
{
    return bSimilarCategoryStatus_;
}

int mmFilterTransactionsDialog::getCategId()
{
    return categID_;
}

int mmFilterTransactionsDialog::getSubCategId()
{
    return subcategID_;
}

bool mmFilterTransactionsDialog::getDateRangeCheckBox()
{
    return m_dateRangeCheckBox->GetValue();
}

wxString mmFilterTransactionsDialog::getNumber()
{
    return transNumberEdit_->GetValue();
}

wxString mmFilterTransactionsDialog::getNotes()
{
    return notesEdit_->GetValue();
}

bool mmFilterTransactionsDialog::getAmountRangeCheckBox()
{
    return amountRangeCheckBox_->GetValue();
}

bool mmFilterTransactionsDialog::getTypeCheckBox()
{
    return typeCheckBox_->IsChecked();
}

bool mmFilterTransactionsDialog::getPayeeCheckBox()
{
    return payeeCheckBox_->IsChecked();
}

bool mmFilterTransactionsDialog::getNumberCheckBox()
{
    return transNumberCheckBox_->IsChecked();
}

bool mmFilterTransactionsDialog::getNotesCheckBox()
{
    return notesCheckBox_->IsChecked();
}

void mmFilterTransactionsDialog::OnMoreFields(wxCommandEvent& WXUNUSED(event))
{
    wxBitmapButton* button = (wxBitmapButton*)FindWindow(wxID_MORE);

    if (m_custom_fields->IsCustomPanelShown())
    {
        if (button) button->SetBitmap(mmBitmap(png::RIGHTARROWSIMPLE));
    }
    else
    {
        if (button) button->SetBitmap(mmBitmap(png::LEFTARROWSIMPLE));
    }

    m_custom_fields->ShowHideCustomPanel();

    this->SetMinSize(wxSize(0, 0));
    this->Fit();
}

void mmFilterTransactionsDialog::OnDateRangeChanged(wxCommandEvent& WXUNUSED(event))
{
    bool user_date = false;
    int i = this->m_date_ranges->GetSelection();
    if (i >= 0 && i < (int)m_date_ranges->GetCount())
    {
        const mmDateRange* date_range = static_cast<mmDateRange*>
            (m_date_ranges->GetClientData(i));

        if (date_range)
        {
            m_begin_date = date_range->start_date().FormatISODate();
            m_end_date = date_range->end_date().FormatISODate();
            m_fromDateCtrl->SetValue(date_range->start_date());
            m_toDateControl->SetValue(date_range->end_date());

            user_date = (date_range->title() == wxString("Custom"));
        }
    }
    m_fromDateCtrl->Enable(user_date);
    m_toDateControl->Enable(user_date);
}

void mmFilterTransactionsDialog::OnSaveSettings(wxCommandEvent& WXUNUSED(event))
{
    int i = m_setting_name->GetSelection();
    //m_custom_fields->SaveCustomValues(i);
    const wxString& default_label = wxString::Format(_("%i: Empty"), i + 1);
    wxString label = m_setting_name->GetStringSelection();
    label = wxGetTextFromUser(_("Please Enter"), _("Setting Name"), label);

    if (label.empty() || label == default_label) {
        return mmErrorDialogs::ToolTip4Object(m_setting_name
            , _("Could not save settings"), _("Empty value"));
    }

    m_setting_name->SetString(i, label);

    SaveSettings();

}

void mmFilterTransactionsDialog::SaveSettings()
{
    int i = m_setting_name->GetSelection();
    settings_string_ = to_json();
    Model_Infotable::instance().Set(wxString::Format("TRANSACTIONS_FILTER_%d", i), settings_string_);
    Model_Infotable::instance().Set("TRANSACTIONS_FILTER_VIEW_NO", i);
    wxLogDebug("========== Settings Saved to registry %i ==========\n %s", i, settings_string_);

    m_custom_fields->SaveCustomValues(0); //TODO: how to save it?
}

void mmFilterTransactionsDialog::OnSettingsSelected(wxCommandEvent& event)
{
    int i = event.GetSelection();
    GetStoredSettings(i);
    m_custom_fields->SetRefID(0); //TODO: 
    dataToControls();
}