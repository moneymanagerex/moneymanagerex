/*******************************************************
Copyright (C) 2006 Madhan Kanagavel
Copyright (C) 2016 - 2020 Nikolay Akimov

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
EVT_MENU(wxID_ANY, mmFilterTransactionsDialog::datePresetMenuSelected)
EVT_DATE_CHANGED(wxID_ANY, mmFilterTransactionsDialog::OnDateChanged)
wxEND_EVENT_TABLE()

mmFilterTransactionsDialog::mmFilterTransactionsDialog()
{
}

mmFilterTransactionsDialog::mmFilterTransactionsDialog(wxWindow* parent)
    : categID_(-1)
    , subcategID_(-1)
    , payeeID_(-1)
    , refAccountID_(-1)
    , refAccountStr_("")
    , bSimilarCategoryStatus_(false)
    , m_settingLabel(nullptr)
{
    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, _("Transaction Filter"), wxDefaultPosition, wxDefaultSize, style);
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
    //--Start of Row --------------------------------------------------------
    accountCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Account")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(accountCheckBox_, g_flagsH);

    accountDropDown_ = new wxChoice(itemPanel
        , wxID_ANY, wxDefaultPosition, wxSize(220, -1)
        , Model_Account::instance().all_checking_account_names(), 0);
    itemPanelSizer->Add(accountDropDown_, g_flagsExpand);

    //--End of Row --------------------------------------------------------

    dateRangeCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Date Range")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(dateRangeCheckBox_, g_flagsH);

    fromDateCtrl_ = new wxDatePickerCtrl(itemPanel, wxID_FIRST, wxDefaultDateTime
        , wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
    toDateControl_ = new wxDatePickerCtrl(itemPanel, wxID_LAST, wxDefaultDateTime
        , wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
    dateRangeCheckBox_->Connect(wxID_ANY, wxEVT_RIGHT_DOWN
        , wxMouseEventHandler(mmFilterTransactionsDialog::datePresetMenu), nullptr, this);

    wxBoxSizer* dateSizer = new wxBoxSizer(wxHORIZONTAL);
    dateSizer->Add(fromDateCtrl_, g_flagsExpand);
    dateSizer->AddSpacer(5);
    dateSizer->Add(toDateControl_, g_flagsExpand);
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

    wxBoxSizer* settings_box_sizer = new wxBoxSizer(wxVERTICAL);

    m_settingLabel = new wxTextCtrl(this, wxID_INFO, "");
    wxString choices[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };
    int num = sizeof(choices) / sizeof(wxString);
    m_radio_box_ = new wxRadioBox(this, wxID_APPLY, ""
        , wxDefaultPosition, wxDefaultSize, num, choices, num, wxRA_SPECIFY_COLS);
    m_radio_box_->Connect(wxID_APPLY, wxEVT_COMMAND_RADIOBOX_SELECTED
        , wxCommandEventHandler(mmFilterTransactionsDialog::OnSettingsSelected), nullptr, this);

    int view_no = Model_Infotable::instance().GetIntInfo("TRANSACTIONS_FILTER_VIEW_NO", 0);
    m_radio_box_->SetSelection(view_no);
    m_radio_box_->Show(true);

    itemBoxSizer3->Add(settings_box_sizer, wxSizerFlags(g_flagsV).Center());
    settings_box_sizer->Add(m_settingLabel, wxSizerFlags(g_flagsExpand).Border(0));
    settings_box_sizer->Add(m_radio_box_, g_flagsV);

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

    wxButton* itemButtonClear = new wxButton(buttonPanel, wxID_CLEAR, _("&Clear "));

    wxBitmapButton* save_button = new wxBitmapButton(buttonPanel
        , wxID_SAVE, mmBitmap(png::SAVE));

    save_button->Show(true);

    buttonPanelSizer->Add(save_button, g_flagsH);
    buttonPanelSizer->Add(itemButtonOK, g_flagsH);
    buttonPanelSizer->Add(itemButtonCancel, g_flagsH);
    buttonPanelSizer->Add(itemButtonClear, g_flagsH);
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
        fromDateCtrl_->Enable(dateRangeCheckBox_->IsChecked());
        toDateControl_->Enable(dateRangeCheckBox_->IsChecked());
    }

    if (accountCheckBox_->IsChecked() && accountDropDown_->GetSelection() < 0)
        accountDropDown_->SetSelection(0);

    event.Skip();
}

bool mmFilterTransactionsDialog::isValuesCorrect()
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
        Model_Account::Data *account = Model_Account::instance().get(refAccountID_);
        if (account) currency = Model_Account::currency(account);
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
        || getPayeeCheckBox()
        || getCategoryCheckBox()
        || getStatusCheckBox()
        || getTypeCheckBox()
        || getAmountRangeCheckBoxMin()
        || getAmountRangeCheckBoxMax()
        || getNumberCheckBox()
        || getNotesCheckBox();
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
    else if ("U" == filterStatus) // Un-Reconciled
    {
        return "" == itemStatus || "F" == itemStatus;
    }
    else if ("A" == filterStatus) // All Except Reconciled
    {
        return "R" != itemStatus;
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
    Model_Account::Data *account = Model_Account::instance().get(refAccountID_);
    if (account) currency = Model_Account::currency(account);

    wxString amountStr = amountMinEdit_->GetValue().Trim();
    double amount = 0;
    if (!Model_Currency::fromString(amountStr, amount, currency) || amount < 0)
        amount = 0;

    return amount;
}

double mmFilterTransactionsDialog::getAmountMax()
{
    Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();
    Model_Account::Data *account = Model_Account::instance().get(refAccountID_);
    if (account) currency = Model_Account::currency(account);

    wxString amountStr = amountMaxEdit_->GetValue().Trim();
    double amount = 0;
    if (!Model_Currency::fromString(amountStr, amount, currency) || amount < 0)
        amount = 0;

    return amount;
}

void mmFilterTransactionsDialog::OnButtonSaveClick(wxCommandEvent& /*event*/)
{
    if (isValuesCorrect())
    {
        int i = m_radio_box_->GetSelection();
        settings_string_ = to_json();
        Model_Infotable::instance().Set(wxString::Format("TRANSACTIONS_FILTER_%d", i), settings_string_);
        Model_Infotable::instance().Set("TRANSACTIONS_FILTER_VIEW_NO", i);
        wxLogDebug("Settings Saved to registry %i\n %s", i, settings_string_);
    }
}

void mmFilterTransactionsDialog::OnButtonClearClick(wxCommandEvent& /*event*/)
{
    clearSettings();
    wxCommandEvent evt(/*wxEVT_CHECKBOX*/ wxID_ANY, wxID_ANY);
    OnCheckboxClick(evt);
}

void mmFilterTransactionsDialog::OnSettingsSelected(wxCommandEvent& event)
{
    SetStoredSettings(event.GetSelection());
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

void mmFilterTransactionsDialog::setAccountToolTip(const wxString& tip) const
{
    accountDropDown_->SetToolTip(tip);
}

void mmFilterTransactionsDialog::clearSettings()
{
    settings_string_ = "{}";
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
    , const int accountID, const std::map<int, Model_Splittransaction::Data_Set>& split)
{
    bool ok = true;
    //wxLogDebug("Check date? %i trx date:%s %s %s", getDateRangeCheckBox(), tran.TRANSDATE, getFromDateCtrl().GetDateOnly().FormatISODate(), getToDateControl().GetDateOnly().FormatISODate());
    if (getAccountCheckBox() && (getAccountID() != tran.ACCOUNTID && getAccountID() != tran.TOACCOUNTID))
        ok = false;
    else if (getDateRangeCheckBox() && (tran.TRANSDATE < m_begin_date || tran.TRANSDATE > m_end_date))
        ok = false;
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
    return ok;
}
bool mmFilterTransactionsDialog::checkAll(const Model_Billsdeposits::Data &tran, const std::map<int, Model_Budgetsplittransaction::Data_Set>& split)
{
    bool ok = true;
    if (getAccountCheckBox() && (getAccountID() != tran.ACCOUNTID && getAccountID() != tran.TOACCOUNTID)) ok = false;
    else if (getDateRangeCheckBox() && (tran.TRANSDATE < m_begin_date && tran.TRANSDATE > m_end_date))
        ok = false;
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
    filterDetails.replace(0, 1, ' ');
    filterDetails.RemoveLast(1);
    filterDetails.Append("\n ");
    return filterDetails;
}

void mmFilterTransactionsDialog::getDescription(mmHTMLBuilder &hb)
{
    hb.addHorizontalLine();
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


    auto s_name = m_settingLabel->GetValue();
    s_name.Trim();
    if (!s_name.empty())
    {
        json_writer.Key((i18n ? _("Name") : "LABEL").utf8_str());
        json_writer.String(s_name.utf8_str());
    }

    if (accountCheckBox_->IsChecked())
    {
        const wxString acc = accountDropDown_->GetStringSelection();
        if (!acc.empty())
        {
            json_writer.Key((i18n ? _("Account") : "ACCOUNT").utf8_str());
            json_writer.String(acc.utf8_str());
        }
    }

    if (dateRangeCheckBox_->IsChecked())
    {
        json_writer.Key((i18n ? _("Since") : "DATE1").utf8_str());
        json_writer.String(fromDateCtrl_->GetValue().FormatISODate().utf8_str());
        json_writer.Key((i18n ? _("Before") : "DATE2").utf8_str());
        json_writer.String(toDateControl_->GetValue().FormatISODate().utf8_str());
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
        json_writer.String(btnCategory_->GetLabel().utf8_str());
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
    const wxString& s_label = j_label.IsString() ? wxString::FromUTF8(j_label.GetString()) : "";
    m_settingLabel->SetValue(s_label);

    //Account
    Value& j_account = GetValueByPointerWithDefault(j_doc, "/ACCOUNT", "");
    const wxString& s_account = j_account.IsString() ? wxString::FromUTF8(j_account.GetString()) : "";
    accountCheckBox_->SetValue(!s_account.empty());
    accountDropDown_->Enable(accountCheckBox_->IsChecked());
    accountDropDown_->SetStringSelection(s_account);

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

    //Payee
    Value& j_payee = GetValueByPointerWithDefault(j_doc, "/PAYEE", "");
    const wxString& s_payee = j_payee.IsString() ? wxString::FromUTF8(j_payee.GetString()) : "";
    payeeCheckBox_->SetValue(!s_payee.empty());
    cbPayee_->Enable(payeeCheckBox_->IsChecked());
    cbPayee_->SetValue(s_payee);

    //Category
    Value& j_category = GetValueByPointerWithDefault(j_doc, "/CATEGORY", "");
    const wxString& s_category = j_category.IsString() ? wxString::FromUTF8(j_category.GetString()) : "";
    categoryCheckBox_->SetValue(!s_category.empty());
    btnCategory_->Enable(categoryCheckBox_->IsChecked());

    subcategID_ = -1;
    categID_ = -1;
    wxStringTokenizer categ_token(s_category, ":", wxTOKEN_RET_EMPTY_ALL);
    const auto categ_name = categ_token.GetNextToken().Trim();
    Model_Category::Data* category = Model_Category::instance().get(categ_name);
    if (category)
    {
        categID_ = category->CATEGID;

        Model_Subcategory::Data* sub_category = nullptr;
        const auto subcateg_name = categ_token.GetNextToken().Trim(false);
        if (!subcateg_name.IsEmpty())
        {
            sub_category = Model_Subcategory::instance().get(subcateg_name, categID_);
            if (sub_category)
                subcategID_ = sub_category->SUBCATEGID;
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
}

void mmFilterTransactionsDialog::OnDateChanged(wxDateEvent& event)
{
    switch (event.GetId())
    {
    case wxID_FIRST: m_begin_date = event.GetDate().FormatISODate(); break;
    case wxID_LAST: m_end_date = event.GetDate().FormatISODate(); break;
    }

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
    return dateRangeCheckBox_->GetValue();
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

void mmFilterTransactionsDialog::ResetFilterStatus()
{
    //m_custom_fields->ResetWidgetsChanged();
}
