/*******************************************************
Copyright (C) 2006 Madhan Kanagavel
Copyright (C) 2016 - 2020 Nikolay Akimov
Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)

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
EVT_BUTTON(wxID_SAVE, mmFilterTransactionsDialog::OnButtonSaveClick)
EVT_BUTTON(wxID_CLEAR, mmFilterTransactionsDialog::OnButtonClearClick)
EVT_MENU(ID_DIALOG_DATEPRESET, mmFilterTransactionsDialog::datePresetMenuSelected)
EVT_DATE_CHANGED(wxID_ANY, mmFilterTransactionsDialog::OnDateChanged)
EVT_BUTTON(ID_DIALOG_COLOUR, mmFilterTransactionsDialog::OnColourButton)
EVT_MENU_RANGE(wxID_HIGHEST , wxID_HIGHEST + 8, mmFilterTransactionsDialog::OnColourSelected)
wxEND_EVENT_TABLE()

mmFilterTransactionsDialog::mmFilterTransactionsDialog()
{
}

mmFilterTransactionsDialog::mmFilterTransactionsDialog(wxWindow* parent)
    : categID_(-1)
    , subcategID_(-1)
    , payeeID_(-1)
    , bSimilarCategoryStatus_(false)
{
    Create(parent);
    isValuesCorrect();
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
    SetStoredSettings(-1);
    wxCommandEvent* evt = new wxCommandEvent(wxEVT_CHECKBOX, wxID_ANY);
    AddPendingEvent(*evt);
    delete evt;

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
    m_accounts_name.clear();
    selected_accounts_id_.clear();

    for (const auto& acc : Model_Account::instance().all()) {
        m_accounts_name.push_back(acc.ACCOUNTNAME);
    }
    m_accounts_name.Sort();

    BuildPayeeList();
    from_json(settings_string_);
}
void mmFilterTransactionsDialog::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, g_flagsExpand);

    wxStaticBox* static_box_sizer = new wxStaticBox(this, wxID_ANY, _("Specify"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(static_box_sizer, wxVERTICAL);
    itemBoxSizer3->Add(itemStaticBoxSizer4, 1, wxGROW | wxALL, 10);

    this->SetSizer(itemBoxSizer2);

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
    
    // Account
    accountCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Account")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(accountCheckBox_, g_flagsH);

    bSelectedAccounts_ = new wxButton(itemPanel, wxID_STATIC, _("All"));
    bSelectedAccounts_->SetMinSize(wxSize(180, -1));
    bSelectedAccounts_->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmFilterTransactionsDialog::OnAccountsButton), nullptr, this);

    itemPanelSizer->Add(bSelectedAccounts_, g_flagsExpand);

    // From Date
    startDateCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("From Date")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(startDateCheckBox_, g_flagsH);

    startDateDropDown_ = new wxChoice(itemPanel, wxID_ANY);
    for (const auto& i : FROM_DATES)
        startDateDropDown_->Append(wxGetTranslation(i), new wxStringClientData(i));
    itemPanelSizer->Add(startDateDropDown_, g_flagsExpand);

    // Date Range
    dateRangeCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Date Range")
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

    // Colour
    colourCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Color")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(colourCheckBox_, g_flagsH);

    colourButton_ = new wxButton(itemPanel, ID_DIALOG_COLOUR, _("Select the Color"));
    colourValue_ = 0;
    itemPanelSizer->Add(colourButton_, g_flagsExpand);

    // Settings
    wxBoxSizer* settings_box_sizer = new wxBoxSizer(wxHORIZONTAL);

    wxBoxSizer* settings_sizer = new wxBoxSizer(wxVERTICAL);
    settings_sizer->Add(settings_box_sizer, wxSizerFlags(g_flagsExpand).Border(wxALL, 0));

    wxStaticText* settings = new wxStaticText(this, wxID_ANY, _("Settings"));
    settings_box_sizer->Add(settings, g_flagsH);
    settings_box_sizer->AddSpacer(5);

    m_setting_name = new wxChoice(this, wxID_APPLY);
    settings_box_sizer->Add(m_setting_name, g_flagsExpand);

    for (int i = 0; i < 10; i++)
    {
        const wxString& data = Model_Infotable::instance().GetStringInfo(
            wxString::Format("TRANSACTIONS_FILTER_%d", i)
            , "");
        Document j_doc;
        if (j_doc.Parse(data.utf8_str()).HasParseError()) {
            j_doc.Parse("{}");
        }

        //Label
        Value& j_label = GetValueByPointerWithDefault(j_doc, "/LABEL", "");
        const wxString& s_label = j_label.IsString() ? wxString::FromUTF8(j_label.GetString()) : "";

        m_setting_name->AppendString(s_label.empty() ? wxString::Format(_("%i: Empty"), i + 1) : s_label);
    }

    m_setting_name->Connect(wxID_APPLY, wxEVT_COMMAND_CHOICE_SELECTED
        , wxCommandEventHandler(mmFilterTransactionsDialog::OnSettingsSelected), nullptr, this);

    settings_box_sizer->AddSpacer(5);
    m_btnSaveAs = new wxBitmapButton(this, wxID_SAVEAS, mmBitmap(png::SAVE));
    settings_box_sizer->Add(m_btnSaveAs, g_flagsH);
    mmToolTip(m_btnSaveAs, _("Save active values into current Preset selection"));
    m_btnSaveAs->Connect(wxID_SAVEAS, wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmFilterTransactionsDialog::OnSaveSettings), nullptr, this);

    wxBitmapButton* itemButtonClear = new wxBitmapButton(this, wxID_CLEAR, mmBitmap(png::CLEAR));
    mmToolTip(itemButtonClear, _("Clear all fields for current Preset selection"));
    settings_box_sizer->Add(itemButtonClear, g_flagsH);

    itemBoxSizer3->Add(settings_sizer, wxSizerFlags(g_flagsExpand).Border(wxALL, 0).Proportion(0));

    /******************************************************************************
     Button Panel with OK/Cancel buttons
    *******************************************************************************/
    wxPanel* buttonPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemBoxSizer3->Add(buttonPanel, wxSizerFlags(g_flagsV).Center());

    wxBoxSizer* buttonPanelSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonPanel->SetSizer(buttonPanelSizer);

    wxButton* itemButtonOK = new wxButton(buttonPanel, wxID_OK, _("&OK "));

    wxButton* itemButtonCancel = new wxButton(buttonPanel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    itemButtonCancel->SetFocus();

    buttonPanelSizer->Add(itemButtonOK, g_flagsH);
    buttonPanelSizer->Add(itemButtonCancel, g_flagsH);

}

void mmFilterTransactionsDialog::OnCheckboxClick(wxCommandEvent& event)
{
    if (event.GetId() == similarCategCheckBox_->GetId())
    {
        bSimilarCategoryStatus_ = similarCategCheckBox_->IsChecked();
    } else if (event.GetId() != cbTypeWithdrawal_->GetId() &&
        event.GetId() != cbTypeDeposit_->GetId() &&
        event.GetId() != cbTypeTransferTo_->GetId() &&
        event.GetId() != cbTypeTransferFrom_->GetId())

    {
        if ((event.GetId() == startDateCheckBox_->GetId()) && dateRangeCheckBox_->IsChecked())
            dateRangeCheckBox_->SetValue(false);
        if ((event.GetId() == dateRangeCheckBox_->GetId()) && startDateCheckBox_->IsChecked())
            startDateCheckBox_->SetValue(false);

        bSelectedAccounts_->Enable(accountCheckBox_->IsChecked());
        cbPayee_->Enable(payeeCheckBox_->IsChecked());
        btnCategory_->Enable(categoryCheckBox_->IsChecked());
        similarCategCheckBox_->Enable(categoryCheckBox_->IsChecked());
        choiceStatus_->Enable(statusCheckBox_->IsChecked());
        cbTypeWithdrawal_->Enable(typeCheckBox_->IsChecked());
        cbTypeDeposit_->Enable(typeCheckBox_->IsChecked());
        cbTypeTransferTo_->Enable(accountCheckBox_->IsChecked() && typeCheckBox_->IsChecked());
        cbTypeTransferFrom_->Enable(typeCheckBox_->IsChecked());
        amountMinEdit_->Enable(amountRangeCheckBox_->IsChecked());
        amountMaxEdit_->Enable(amountRangeCheckBox_->IsChecked());
        notesEdit_->Enable(notesCheckBox_->IsChecked());
        transNumberEdit_->Enable(transNumberCheckBox_->IsChecked());
        startDateDropDown_->Enable(startDateCheckBox_->IsChecked());
        fromDateCtrl_->Enable(dateRangeCheckBox_->IsChecked());
        toDateControl_->Enable(dateRangeCheckBox_->IsChecked());
        colourButton_->Enable(colourCheckBox_->IsChecked());
    }

    if (accountCheckBox_->IsChecked() && selected_accounts_id_.size() <= 0)
        bSelectedAccounts_->SetLabelText("");

    event.Skip();
}

bool mmFilterTransactionsDialog::isValuesCorrect()
{
    if (accountCheckBox_->IsChecked())
    {
        //TODO
    }
    else
    {
        selected_accounts_id_.clear();
    }

    if (payeeCheckBox_->IsChecked())
    {
        Model_Payee::Data* payee = Model_Payee::instance().get(cbPayee_->GetValue());
        if (payee)
        {
            payeeID_ = payee->PAYEEID;
            payeeStr_ = payee->PAYEENAME;
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

    if (startDateCheckBox_->IsChecked())
    {
        mmDateRange* date_range = NULL;
        switch (startDateDropDown_->GetSelection())
        {
            case FROM_FIN_YEAR:
                date_range = new mmCurrentFinancialYear(wxAtoi(Option::instance().FinancialYearStartDay())
                                        , wxAtoi(Option::instance().FinancialYearStartMonth()));
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
        delete date_range;
    }

    if (statusCheckBox_->IsChecked() && choiceStatus_->GetSelection() < 0)
    {
        int id = choiceStatus_->GetId();
        mmErrorDialogs::ToolTip4Object(FindWindow(id)
            , _("Invalid value"), _("Status"));
        return false;
    }

    return true;
}

void mmFilterTransactionsDialog::OnButtonOkClick(wxCommandEvent& /*event*/)
{
    if (isValuesCorrect()) {
        settings_string_ = to_json();
        int id = m_setting_name->GetSelection(); //Model_Infotable::instance().GetIntInfo("TRANSACTIONS_FILTER_VIEW_NO", 0);
        Model_Infotable::instance().Set("TRANSACTIONS_FILTER_VIEW_NO", id);
        Model_Infotable::instance().Set(wxString::Format("TRANSACTIONS_FILTER_%d", id), settings_string_);
        wxLogDebug("Settings Saved to registry %i\n %s", id, settings_string_);
        EndModal(wxID_OK);
    }
}

void mmFilterTransactionsDialog::OnButtonCancelClick(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void mmFilterTransactionsDialog::OnCategs(wxCommandEvent& /*event*/)
{
    Model_Category::Data* category = Model_Category::instance().get(categID_);
    Model_Subcategory::Data* sub_category = Model_Subcategory::instance().get(subcategID_);
    int categID = category ? category->CATEGID : -1;
    int subcategID = sub_category ? sub_category->SUBCATEGID : -1;
    mmCategDialog dlg(this, true, categID, subcategID);

    if (dlg.ShowModal() == wxID_OK)
    {
        categID_ = dlg.getCategId();
        subcategID_ = dlg.getSubCategId();
        category = Model_Category::instance().get(categID_);
        sub_category = Model_Subcategory::instance().get(subcategID_);

        btnCategory_->SetLabelText(Model_Category::full_name(category, sub_category));
    }
}

bool mmFilterTransactionsDialog::isSomethingSelected()
{
    return
        getAccountCheckBox()
        || getDateRangeCheckBox()
        || getStartDateCheckBox()
        || getPayeeCheckBox()
        || getCategoryCheckBox()
        || getStatusCheckBox()
        || getTypeCheckBox()
        || getAmountRangeCheckBoxMin()
        || getAmountRangeCheckBoxMax()
        || getNumberCheckBox()
        || getNotesCheckBox()
        || getColourCheckBox();
}

wxString mmFilterTransactionsDialog::getStatus() const
{
    wxString status;
    wxStringClientData* status_obj =
        static_cast<wxStringClientData*>(choiceStatus_->GetClientObject(choiceStatus_->GetSelection()));
    if (status_obj) status = status_obj->GetData().Left(1);
    status.Replace("N", "");
    return status;
}

bool mmFilterTransactionsDialog::compareStatus(const wxString& itemStatus) const
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
    else if ("U" == filterStatus) // Un-Reconciled
    {
        return itemStatus.empty() || "N" == itemStatus;
    }
    return false;
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

double mmFilterTransactionsDialog::getAmountMin()
{
    Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();

    wxString amountStr = amountMinEdit_->GetValue().Trim();
    double amount = 0;
    if (!Model_Currency::fromString(amountStr, amount, currency) || amount < 0)
        amount = 0;

    return amount;
}

double mmFilterTransactionsDialog::getAmountMax()
{
    Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();

    wxString amountStr = amountMaxEdit_->GetValue().Trim();
    double amount = 0;
    if (!Model_Currency::fromString(amountStr, amount, currency) || amount < 0)
        amount = 0;

    return amount;
}

void mmFilterTransactionsDialog::OnButtonSaveClick(wxCommandEvent& /*event*/)
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

    SaveSettings(i);
}

void mmFilterTransactionsDialog::OnButtonClearClick(wxCommandEvent& /*event*/)
{
    clearSettings();
    wxCommandEvent evt(/*wxEVT_CHECKBOX*/ wxID_ANY, wxID_ANY);
    OnCheckboxClick(evt);
}

void mmFilterTransactionsDialog::SetStoredSettings(int id)
{
    if (id < 0) {
        id = Model_Infotable::instance().GetIntInfo("TRANSACTIONS_FILTER_VIEW_NO", 0);
    }
    else {
        Model_Setting::instance().Set("TRANSACTIONS_FILTER_VIEW_NO", id);
    }
    settings_string_ = Model_Infotable::instance().GetStringInfo(
        wxString::Format("TRANSACTIONS_FILTER_%d", id)
        , "");
    dataToControls();
}

void mmFilterTransactionsDialog::clearSettings()
{
    settings_string_ = "{}";

    int i = m_setting_name->GetSelection();
    wxString s_label = wxString::Format(_("%i: Empty"), i + 1);
    m_setting_name->SetString(i, s_label);
    Model_Infotable::instance().Set(wxString::Format("TRANSACTIONS_FILTER_%d", i), settings_string_);

    dataToControls();
}
void mmFilterTransactionsDialog::datePresetMenuSelected(wxCommandEvent& event)
{
    int id = event.GetId();
    setPresettings(DATE_PRESETTINGS[id]);
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
    mmDateRange* date_range = NULL;
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
        date_range = new mmCurrentFinancialYear(wxAtoi(Option::instance().FinancialYearStartDay())
            , wxAtoi(Option::instance().FinancialYearStartMonth()));
    else if (view == VIEW_TRANS_LAST_YEAR_STR)
        date_range = new mmLastYear;
    else if (view == VIEW_TRANS_LAST_FIN_YEAR_STR)
        date_range = new mmLastFinancialYear(wxAtoi(Option::instance().FinancialYearStartDay())
            , wxAtoi(Option::instance().FinancialYearStartMonth()));
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
    delete date_range;
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
bool mmFilterTransactionsDialog::checkPayee(const DATA &tran)
{
    const Model_Payee::Data* payee = Model_Payee::instance().get(tran.PAYEEID);
    if (payee)
        return cbPayee_->GetValue().Lower() == (payee->PAYEENAME).Lower();
    return false;
}

template<class MODEL, class DATA>
bool mmFilterTransactionsDialog::checkCategory(const DATA& tran, const std::map<int, typename MODEL::Split_Data_Set> & splits)
{
    const auto it = splits.find(tran.id());
    if (it == splits.end())
    {
        if (categID_ != tran.CATEGID) return false;
        if (subcategID_ != tran.SUBCATEGID && !bSimilarCategoryStatus_) return false;
    }
    else
    {
        bool bMatching = false;
        for (const auto &split : it->second)
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

bool mmFilterTransactionsDialog::checkAll(const Model_Checking::Data &tran
    , int accountID, const std::map<int, Model_Splittransaction::Data_Set>& split)
{
    bool ok = true;
    //wxLogDebug("Check date? %i trx date:%s %s %s", getDateRangeCheckBox(), tran.TRANSDATE, getFromDateCtrl().GetDateOnly().FormatISODate(), getToDateControl().GetDateOnly().FormatISODate());
    if (getAccountCheckBox()
           && selected_accounts_id_.Index(tran.ACCOUNTID) == wxNOT_FOUND
           && selected_accounts_id_.Index(tran.TOACCOUNTID) == wxNOT_FOUND)
        ok = false;
    else if (getDateRangeCheckBox() && (tran.TRANSDATE < m_begin_date || tran.TRANSDATE > m_end_date))
        ok = false;
    else if (getStartDateCheckBox() && (tran.TRANSDATE < m_begin_date)) ok = false;
    else if (getPayeeCheckBox() && !checkPayee<Model_Checking>(tran)) ok = false;
    else if (getCategoryCheckBox() && !checkCategory<Model_Checking>(tran, split)) ok = false;
    else if (getStatusCheckBox() && !compareStatus(tran.STATUS)) ok = false;
    else if (getTypeCheckBox() && !allowType(tran.TRANSCODE, accountID == tran.ACCOUNTID)) ok = false;
    else if (getAmountRangeCheckBoxMin() && getAmountMin() > tran.TRANSAMOUNT) ok = false;
    else if (getAmountRangeCheckBoxMax() && getAmountMax() < tran.TRANSAMOUNT) ok = false;
    else if (getNumberCheckBox() && (getNumber().empty() ? !tran.TRANSACTIONNUMBER.empty()
        : tran.TRANSACTIONNUMBER.empty() || !tran.TRANSACTIONNUMBER.Lower().Matches(getNumber().Lower())))
        ok = false;
    else if (getNotesCheckBox() && (getNotes().empty() ? !tran.NOTES.empty()
        : tran.NOTES.empty() || !tran.NOTES.Lower().Matches(getNotes().Lower())))
        ok = false;
    else if (getColourCheckBox() && (colourValue_ != tran.FOLLOWUPID))
        ok = false;
    return ok;
}
bool mmFilterTransactionsDialog::checkAll(const Model_Billsdeposits::Data &tran, const std::map<int, Model_Budgetsplittransaction::Data_Set>& split)
{
    bool ok = true;
    if (getAccountCheckBox()
            && selected_accounts_id_.Index(tran.ACCOUNTID) == wxNOT_FOUND
            && selected_accounts_id_.Index(tran.TOACCOUNTID) == wxNOT_FOUND)
        ok = false;
    else if (getDateRangeCheckBox() && (tran.TRANSDATE < m_begin_date && tran.TRANSDATE > m_end_date))
        ok = false;
    else if (getStartDateCheckBox() && (tran.TRANSDATE < m_begin_date)) ok = false;
    else if (getPayeeCheckBox() && !checkPayee<Model_Billsdeposits>(tran)) ok = false;
    else if (getCategoryCheckBox() && !checkCategory<Model_Billsdeposits>(tran, split)) ok = false;
    else if (getStatusCheckBox() && !compareStatus(tran.STATUS)) ok = false;
    else if (getTypeCheckBox() && !allowType(tran.TRANSCODE, true)) ok = false;
    else if (getAmountRangeCheckBoxMin() && getAmountMin() > tran.TRANSAMOUNT) ok = false;
    else if (getAmountRangeCheckBoxMax() && getAmountMax() < tran.TRANSAMOUNT) ok = false;
    else if (getNumberCheckBox() && (getNumber().empty()
        ? !tran.TRANSACTIONNUMBER.empty()
        : tran.TRANSACTIONNUMBER.empty() || !tran.TRANSACTIONNUMBER.Lower().Matches(getNumber().Lower())))
        ok = false;
    else if (getNotesCheckBox() && (getNotes().empty()
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

const wxString mmFilterTransactionsDialog::getDescriptionToolTip()
{
    wxString filterDetails = to_json(true);
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
    wxString filterDetails = to_json(true);
    filterDetails.Replace("\n", "<br>");
    filterDetails.Replace(R"("")", _("Empty value"));
    filterDetails.Replace("\"", "");
    filterDetails.replace(0, 1, ' ');
    filterDetails.RemoveLast(1);
    hb.addText(filterDetails);
}

const wxString mmFilterTransactionsDialog::to_json(bool i18n)
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();


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

    if (accountCheckBox_->IsChecked() && !m_accounts_name.empty())
    {
        json_writer.Key((i18n ? _("Account") : "ACCOUNT").utf8_str());
        json_writer.StartArray();
        for (const auto& acc : selected_accounts_id_)
        {
            Model_Account::Data* a = Model_Account::instance().get(acc);
            json_writer.String(a->ACCOUNTNAME.utf8_str());
        }
        json_writer.EndArray();
    }

    if (dateRangeCheckBox_->IsChecked())
    {
        json_writer.Key((i18n ? _("Since") : "DATE1").utf8_str());
        json_writer.String(fromDateCtrl_->GetValue().FormatISODate().utf8_str());
        json_writer.Key((i18n ? _("Before") : "DATE2").utf8_str());
        json_writer.String(toDateControl_->GetValue().FormatISODate().utf8_str());
    }

    if (startDateCheckBox_->IsChecked())
    {
        const wxString startPoint = startDateDropDown_->GetStringSelection();
        if (!startPoint.empty())
        {
            json_writer.Key((i18n ? _("From") : "FROM").utf8_str());
            json_writer.String(startPoint.utf8_str());
        }
    }

   if (payeeCheckBox_->IsChecked())
    {
        json_writer.Key((i18n ? _("Payee") : "PAYEE").utf8_str());
        json_writer.String(cbPayee_->GetValue().utf8_str());
    }

    if (categoryCheckBox_->IsChecked())
    {
        json_writer.Key((i18n ? _("Include Similar") : "SIMILAR_YN").utf8_str());
        json_writer.Bool(bSimilarCategoryStatus_);
        json_writer.Key((i18n ? _("Category") : "CATEGORY").utf8_str());
        auto categ = Model_Category::full_name(categID_, subcategID_);
        wxLogDebug("%s", categ);
        json_writer.String(categ.utf8_str());
    }

    if (statusCheckBox_->IsChecked())
    {
        wxArrayString s = Model_Checking::all_status();
        s.Add(wxTRANSLATE("Un-Reconciled"));
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

    if (transNumberCheckBox_->IsChecked())
    {
        const wxString num = transNumberEdit_->GetValue();
        json_writer.Key((i18n ? _("Number") : "NUMBER").utf8_str());
        json_writer.String(num.utf8_str());
    }

    if (notesCheckBox_->IsChecked())
    {
        wxString notes = notesEdit_->GetValue();
        json_writer.Key((i18n ? _("Notes") : "NOTES").utf8_str());
        json_writer.String(notes.utf8_str());
    }

    if (colourCheckBox_->IsChecked())
    {
        json_writer.Key((i18n ? _("Color") : "COLOR").utf8_str());
        json_writer.Int(colourValue_);
    }

    json_writer.EndObject();

    return wxString::FromUTF8(json_buffer.GetString());
}

void mmFilterTransactionsDialog::from_json(const wxString &data)
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
    if (s_label.empty())
    {
        int i = m_setting_name->GetSelection();
        if (i < 0 )
            i = Model_Infotable::instance().GetIntInfo("TRANSACTIONS_FILTER_VIEW_NO", 0);
        s_label = wxString::Format(_("%i: Empty"), i + 1);
    }
    m_setting_name->SetStringSelection(s_label);

    //Account
    Value& j_account = GetValueByPointerWithDefault(j_doc, "/ACCOUNT", "");
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
                selected_accounts_id_.Add(a.ACCOUNTID);
                baloon += (baloon.empty() ? "" : "\n") + a.ACCOUNTNAME;
            }
        }
        if (selected_accounts_id_.size() == 1)
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

    subcategID_ = -1;
    categID_ = -1;

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
            categID_ = entry.second.first;
            subcategID_ = entry.second.second;
            break;
        }
    }

    btnCategory_->SetLabelText(Model_Category::full_name(categID_, subcategID_));

    bSimilarCategoryStatus_ = false;
    if (j_doc.HasMember("SIMILAR_YN") && j_doc["SIMILAR_YN"].IsBool())
    {
        bSimilarCategoryStatus_ = j_doc["SIMILAR_YN"].GetBool();
    }
    similarCategCheckBox_->SetValue(bSimilarCategoryStatus_);
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
    colourValue_ = 0;
    colourCheckBox_->SetValue(false);
    if (j_doc.HasMember("COLOR") && j_doc["COLOR"].IsInt()) {
        colourCheckBox_->SetValue(true);
        colourValue_ = j_doc["COLOR"].GetInt();
    }
    colourButton_->Enable(colourCheckBox_->IsChecked());
    colourButton_->SetBackgroundColour(getUDColour(colourValue_));
    colourButton_->Refresh(); // Needed as setting the background colour does not cause an immediate refresh
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
    return selected_accounts_id_;
}

bool mmFilterTransactionsDialog::getStatusCheckBox()
{
    return statusCheckBox_->IsChecked();
}

bool mmFilterTransactionsDialog::getAccountCheckBox()
{
    return accountCheckBox_->GetValue() && !selected_accounts_id_.empty();
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
    return dateRangeCheckBox_->GetValue();
}

bool mmFilterTransactionsDialog::getStartDateCheckBox()
{
    return startDateCheckBox_->GetValue();
}

bool mmFilterTransactionsDialog::getAmountRangeCheckBoxMin()
{
    return amountRangeCheckBox_->GetValue() && !amountMinEdit_->GetValue().IsEmpty();
}

bool mmFilterTransactionsDialog::getAmountRangeCheckBoxMax()
{
    return amountRangeCheckBox_->GetValue() && !amountMaxEdit_->GetValue().IsEmpty();
}

wxString mmFilterTransactionsDialog::getNumber()
{
    return transNumberEdit_->GetValue();
}

wxString mmFilterTransactionsDialog::getNotes()
{
    return notesEdit_->GetValue();
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

bool mmFilterTransactionsDialog::getColourCheckBox()
{
    return colourCheckBox_->IsChecked();
}

void mmFilterTransactionsDialog::ResetFilterStatus()
{
    //m_custom_fields->ResetWidgetsChanged();
}

void mmFilterTransactionsDialog::SaveSettings(int menu_item)
{
    settings_string_ = to_json();
    Model_Infotable::instance().Set(wxString::Format("TRANSACTIONS_FILTER_%d", menu_item), settings_string_);
    wxLogDebug("========== Settings Saved to registry %i ==========\n %s", menu_item, settings_string_);

}

void mmFilterTransactionsDialog::OnSettingsSelected(wxCommandEvent& event)
{
    int i = event.GetSelection();
    SetStoredSettings(i);
    dataToControls();
}

void mmFilterTransactionsDialog::OnSaveSettings(wxCommandEvent& WXUNUSED(event))
{
    int i = m_setting_name->GetSelection();
    if (i < 0)
        return mmErrorDialogs::ToolTip4Object(m_setting_name
            , _("Could not save settings"), _("Empty value"));

    const wxString& default_label = wxString::Format(_("%i: Empty"), i + 1);
    wxString label = m_setting_name->GetStringSelection();
    label = wxGetTextFromUser(_("Please Enter"), _("Setting Name"), label);

    if (label.empty() || label == default_label) {
        return mmErrorDialogs::ToolTip4Object(m_setting_name
            , _("Could not save settings"), _("Empty value"));
    }

    Model_Infotable::instance().Set("TRANSACTIONS_FILTER_VIEW_NO", i);
    m_setting_name->SetString(i, label);

    SaveSettings(i);
}

void mmFilterTransactionsDialog::OnAccountsButton(wxCommandEvent& WXUNUSED(event))
{
    bSelectedAccounts_->UnsetToolTip();
    wxMultiChoiceDialog s_acc(this, _("Choose Accounts")
       , "" , m_accounts_name);

    wxButton* ok = static_cast<wxButton*>(s_acc.FindWindow(wxID_OK));
    if (ok) ok->SetLabel(_("&OK "));
    wxButton* ca = static_cast<wxButton*>(s_acc.FindWindow(wxID_CANCEL));
    if (ca) ca->SetLabel(wxGetTranslation(g_CancelLabel));

    wxString baloon = "";
    wxArrayInt selected_items;

    for (const auto& acc : selected_accounts_id_)
    {
        Model_Account::Data* a = Model_Account::instance().get(acc);
        if (a && m_accounts_name.Index(a->ACCOUNTNAME) != wxNOT_FOUND)
            selected_items.Add(m_accounts_name.Index(a->ACCOUNTNAME));
    }
    s_acc.SetSelections(selected_items);

    selected_accounts_id_.Clear();
    bSelectedAccounts_->UnsetToolTip();

    if (s_acc.ShowModal() == wxID_OK)
    {
        selected_items = s_acc.GetSelections();
        for (const auto &entry : selected_items)
        {
            int index = entry;
            const wxString accounts_name = m_accounts_name[index];
            const auto account = Model_Account::instance().get(accounts_name);
            if (account) selected_accounts_id_.Add(account->ACCOUNTID);
            baloon += accounts_name + "\n";
        }
    }

    if (selected_accounts_id_.GetCount() == 0)
    {
        bSelectedAccounts_->SetLabelText("");
        accountCheckBox_->SetValue(false);
        bSelectedAccounts_->Disable();
    }
    else if (selected_accounts_id_.GetCount() == 1)
    {
        const Model_Account::Data* account = Model_Account::instance().get(*selected_accounts_id_.begin());
        if (account)
            bSelectedAccounts_->SetLabelText(account->ACCOUNTNAME);
    }
    else if (selected_accounts_id_.GetCount() > 1)
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
        wxBitmap bitmap(mmBitmap(png::EMPTY).GetSize());
        wxMemoryDC memoryDC(bitmap);
        wxRect rect(memoryDC.GetSize());

        memoryDC.SetBackground(wxBrush(getUDColour(i)));
        memoryDC.Clear();
        memoryDC.DrawBitmap(mmBitmap(png::EMPTY), 0, 0, true);
        memoryDC.SelectObject(wxNullBitmap);
        menuItem->SetBitmap(bitmap);

        mainMenu->Append(menuItem);
    }

    PopupMenu(mainMenu);
    delete mainMenu;
}

void mmFilterTransactionsDialog::OnColourSelected(wxCommandEvent& event)
{
    int selected_nemu_item = event.GetId() - wxID_HIGHEST;
    colourButton_->SetBackgroundColour(getUDColour(selected_nemu_item));
    colourValue_ = selected_nemu_item;
}