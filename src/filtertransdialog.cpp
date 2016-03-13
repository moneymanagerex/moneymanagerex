/*******************************************************
Copyright (C) 2006-2012

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

#include "model/Model_Account.h"
#include "model/Model_Billsdeposits.h"
#include "model/Model_Category.h"
#include "model/Model_Infotable.h"
#include "model/Model_Payee.h"
#include "model/Model_Setting.h"

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
    VIEW_TRANS_CURRENT_YEAR_STR
};

wxIMPLEMENT_DYNAMIC_CLASS(mmFilterTransactionsDialog, wxDialog);

wxBEGIN_EVENT_TABLE( mmFilterTransactionsDialog, wxDialog )
    EVT_CHECKBOX(wxID_ANY,    mmFilterTransactionsDialog::OnCheckboxClick )
    EVT_BUTTON  (wxID_OK,     mmFilterTransactionsDialog::OnButtonokClick )
    EVT_BUTTON  (wxID_CANCEL, mmFilterTransactionsDialog::OnButtoncancelClick )
    EVT_BUTTON  (wxID_SAVE,   mmFilterTransactionsDialog::OnButtonSaveClick )
    EVT_BUTTON  (wxID_CLEAR,  mmFilterTransactionsDialog::OnButtonClearClick )
    EVT_MENU    (wxID_ANY,    mmFilterTransactionsDialog::datePresetMenuSelected )
wxEND_EVENT_TABLE()

mmFilterTransactionsDialog::mmFilterTransactionsDialog( )
{
}

mmFilterTransactionsDialog::mmFilterTransactionsDialog(wxWindow* parent)
    : categID_(-1)
    , subcategID_(-1)
    , payeeID_(-1)
    , refAccountID_(-1)
    , refAccountStr_("")
    , date_range_(0)
    , m_settingLabel()
    , bSimilarCategoryStatus_(false)
{
    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX;
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

    dataToControls();
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

    wxStaticBox* static_box_sizer = new wxStaticBox(this, wxID_ANY, _("Specify "));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(static_box_sizer, wxVERTICAL);
    itemBoxSizer3->Add(itemStaticBoxSizer4, 1, wxGROW|wxALL, 10);

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
    itemPanelSizer->Add(accountCheckBox_, g_flags);

    accountDropDown_ = new wxChoice(itemPanel
        , wxID_ANY, wxDefaultPosition, wxSize(220, -1)
        , Model_Account::instance().all_checking_account_names(), 0);
    itemPanelSizer->Add(accountDropDown_, g_flagsExpand);

    //--End of Row --------------------------------------------------------

    dateRangeCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Date Range")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(dateRangeCheckBox_, g_flags);

    fromDateCtrl_ = new wxDatePickerCtrl(itemPanel, wxID_ANY, wxDefaultDateTime
        , wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
    toDateControl_ = new wxDatePickerCtrl(itemPanel, wxID_ANY, wxDefaultDateTime
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
    itemPanelSizer->Add(payeeCheckBox_, g_flags);

    cbPayee_ = new wxComboBox(itemPanel, wxID_ANY, "", wxDefaultPosition,  wxSize(220, -1));
    cbPayee_->Connect(wxID_ANY, wxEVT_COMMAND_TEXT_UPDATED
        , wxCommandEventHandler(mmFilterTransactionsDialog::OnPayeeUpdated), nullptr, this);

    itemPanelSizer->Add(cbPayee_, g_flagsExpand);
    //--End of Row --------------------------------------------------------

    categoryCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Category")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);

    wxFlexGridSizer* categSizer = new wxFlexGridSizer(0, 1, 0, 0);
    categSizer->AddGrowableCol(0, 1);

    itemPanelSizer->Add(categoryCheckBox_, g_flags);
    itemPanelSizer->Add(categSizer, wxSizerFlags(g_flagsExpand).Border(0));

    btnCategory_ = new wxButton(itemPanel, wxID_ANY, ""
        , wxDefaultPosition, wxDefaultSize);
    btnCategory_->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(mmFilterTransactionsDialog::OnCategs), nullptr, this);
    similarCategCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Include Similar"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    similarCategCheckBox_->SetToolTip(_("Include all subcategories for the selected category."));

    categSizer->Add(btnCategory_, g_flagsExpand);
    categSizer->Add(similarCategCheckBox_, wxSizerFlags(g_flags).Center().Border(0));
    categSizer->AddSpacer(5);
    //--End of Row --------------------------------------------------------

    statusCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Status")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(statusCheckBox_, g_flags);

    choiceStatus_ = new wxChoice(itemPanel, wxID_ANY);

    for(const auto& i : TRANSACTION_STATUSES)
        choiceStatus_->Append(wxGetTranslation(i), new wxStringClientData(i));

    itemPanelSizer->Add(choiceStatus_, g_flagsExpand);
    choiceStatus_->SetToolTip(_("Specify the status for the transaction"));
    //--End of Row --------------------------------------------------------

    typeCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Type")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);

    wxFlexGridSizer* typeSizer = new wxFlexGridSizer(0, 2, 0, 0);
    typeSizer->AddGrowableCol(0, 1);
    typeSizer->AddGrowableCol(1, 1);
    cbTypeWithdrawal_ = new wxCheckBox( itemPanel, wxID_ANY, _("Withdrawal")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    cbTypeDeposit_ = new wxCheckBox( itemPanel, wxID_ANY, _("Deposit")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    cbTypeTransferTo_ = new wxCheckBox( itemPanel, wxID_ANY, _("Transfer To")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    cbTypeTransferFrom_ = new wxCheckBox(itemPanel, wxID_ANY, _("Transfer From")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);

    itemPanelSizer->Add(typeCheckBox_, g_flags);
    itemPanelSizer->Add(typeSizer, g_flagsExpand);
    typeSizer->Add(cbTypeWithdrawal_, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    typeSizer->Add(cbTypeDeposit_, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    typeSizer->Add(cbTypeTransferTo_, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    typeSizer->Add(cbTypeTransferFrom_, 1, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 2);
    typeSizer->AddSpacer(2);

    //--End of Row --------------------------------------------------------

    amountRangeCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Amount Range")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(amountRangeCheckBox_, g_flags);

    amountMinEdit_ = new mmTextCtrl(itemPanel, wxID_ANY, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT|wxTE_PROCESS_ENTER
        , mmCalcValidator());
    amountMinEdit_->Connect(wxID_ANY, wxEVT_COMMAND_TEXT_ENTER,
        wxCommandEventHandler(mmFilterTransactionsDialog::OnTextEntered), nullptr, this);
    amountMaxEdit_ = new mmTextCtrl(itemPanel, wxID_ANY, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT|wxTE_PROCESS_ENTER
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
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    itemPanelSizer->Add(transNumberCheckBox_, g_flags);

    transNumberEdit_ = new wxTextCtrl(itemPanel, wxID_ANY);
    itemPanelSizer->Add(transNumberEdit_, g_flagsExpand);
    //--End of Row --------------------------------------------------------

    notesCheckBox_ = new wxCheckBox(itemPanel, wxID_ANY, _("Notes")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    itemPanelSizer->Add(notesCheckBox_, g_flags);

    notesEdit_ = new wxTextCtrl( itemPanel, wxID_ANY);
    itemPanelSizer->Add(notesEdit_, g_flagsExpand);
    //--End of Row --------------------------------------------------------

    wxBoxSizer* settings_box_sizer = new wxBoxSizer(wxVERTICAL);

    m_settingLabel = new wxTextCtrl(this, wxID_INFO, "");
    wxString choices[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
    int num = sizeof(choices) / sizeof(wxString);
    m_radio_box_ = new wxRadioBox(this, wxID_APPLY, ""
        , wxDefaultPosition, wxDefaultSize, num, choices, num, wxRA_SPECIFY_COLS);
    m_radio_box_->Connect(wxID_APPLY, wxEVT_COMMAND_RADIOBOX_SELECTED
        , wxCommandEventHandler(mmFilterTransactionsDialog::OnSettingsSelected), nullptr, this);

    int view_no = Model_Infotable::instance().GetIntInfo("TRANSACTIONS_FILTER_VIEW_NO", 0);
    m_radio_box_->SetSelection(view_no);
    m_radio_box_->Show(true);

    itemBoxSizer3->Add(settings_box_sizer, wxSizerFlags(g_flags).Center());
    settings_box_sizer->Add(m_settingLabel, wxSizerFlags(g_flagsExpand).Border(0));
    settings_box_sizer->Add(m_radio_box_, g_flags);

    /******************************************************************************
     Button Panel with OK/Cancel buttons
    *******************************************************************************/
    wxPanel* buttonPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemBoxSizer3->Add(buttonPanel, wxSizerFlags(g_flags).Center());

    wxBoxSizer* buttonPanelSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonPanel->SetSizer(buttonPanelSizer);

    wxButton* itemButtonOK = new wxButton( buttonPanel, wxID_OK, _("&OK "));

    wxButton* itemButtonCancel = new wxButton( buttonPanel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    itemButtonCancel->SetFocus();

    wxButton* itemButtonClear = new wxButton( buttonPanel, wxID_CLEAR, _("&Clear "));

    wxBitmapButton* save_button = new wxBitmapButton(buttonPanel
        , wxID_SAVE, mmBitmap(png::SAVE));

    save_button->Show(true);

    buttonPanelSizer->Add(save_button,g_flags);
    buttonPanelSizer->Add(itemButtonOK, g_flags);
    buttonPanelSizer->Add(itemButtonCancel, g_flags);
    buttonPanelSizer->Add(itemButtonClear, g_flags);
}

void mmFilterTransactionsDialog::OnCheckboxClick( wxCommandEvent& event )
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
        fromDateCtrl_->Enable(dateRangeCheckBox_->IsChecked());
        toDateControl_->Enable(dateRangeCheckBox_->IsChecked());
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
        accountDropDown_->Enable(accountCheckBox_->IsChecked());
    }

    event.Skip();
}

void mmFilterTransactionsDialog::OnButtonokClick( wxCommandEvent& /*event*/ )
{
    if (accountCheckBox_->IsChecked())
    {
        refAccountStr_ = accountDropDown_->GetStringSelection();
        Model_Account::Data* account = Model_Account::instance().get(refAccountStr_);
        if (account) refAccountID_ = account->ACCOUNTID;
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
        Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();
        Model_Account::Data *account = Model_Account::instance().get(refAccountID_);
        if (account) currency = Model_Account::currency(account);

        wxString minamt = amountMinEdit_->GetValue().Trim();
        wxString maxamt = amountMaxEdit_->GetValue().Trim();
        if (!minamt.IsEmpty())
        {
            double amount;
            if (!Model_Currency::fromString(minamt, amount, currency) || amount < 0)
            {
                mmErrorDialogs::MessageError(this, _("Invalid Amount Entered "), _("Error"));
                return;
            }
        }

        if (!maxamt.IsEmpty())
        {
            double amount;
            if (!Model_Currency::fromString(maxamt, amount, currency) || amount < 0)
            {
                mmErrorDialogs::MessageError(this, _("Invalid Amount Entered "), _("Error"));
                return;
            }
        }
    }

    EndModal(wxID_OK);
}

void mmFilterTransactionsDialog::OnButtoncancelClick( wxCommandEvent& /*event*/ )
{
    EndModal(wxID_CANCEL);
}

void mmFilterTransactionsDialog::OnCategs(wxCommandEvent& /*event*/)
{
    mmCategDialog dlg(this);

    Model_Category::Data* category = Model_Category::instance().get(categID_);
    Model_Subcategory::Data* sub_category = Model_Subcategory::instance().get(subcategID_);
    int categID = category ? category->CATEGID : -1;
    int subcategID = sub_category ? sub_category->SUBCATEGID : -1;
    dlg.setTreeSelection(categID, subcategID);

    if (dlg.ShowModal() == wxID_OK)
    {
        categID_ = dlg.getCategId();
        subcategID_ = dlg.getSubCategId();
        category = Model_Category::instance().get(categID_);
        sub_category = Model_Subcategory::instance().get(subcategID_);

        btnCategory_->SetLabelText(Model_Category::full_name(category, sub_category));
    }
}

bool mmFilterTransactionsDialog::somethingSelected()
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
        (wxStringClientData *)choiceStatus_->GetClientObject(choiceStatus_->GetSelection());
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

void mmFilterTransactionsDialog::OnButtonSaveClick( wxCommandEvent& /*event*/ )
{
    int i = m_radio_box_->GetSelection();
    settings_string_ = to_json();
    Model_Infotable::instance().Set(wxString::Format("TRANSACTIONS_FILTER_%d", i), settings_string_);
    Model_Infotable::instance().Set("TRANSACTIONS_FILTER_VIEW_NO", i);
    wxLogDebug("Settings Saved to registry %i\n %s", i, settings_string_);
}

void mmFilterTransactionsDialog::OnButtonClearClick( wxCommandEvent& /*event*/ )
{
    clearSettings();
    wxCommandEvent evt(/*wxEVT_CHECKBOX*/ wxID_ANY, wxID_ANY);
    OnCheckboxClick(evt);
}

void mmFilterTransactionsDialog::OnSettingsSelected( wxCommandEvent& event )
{
    GetStoredSettings(event.GetSelection());
    dataToControls();
}

wxString mmFilterTransactionsDialog::GetStoredSettings(int id)
{
    if (id < 0) {
        id = Model_Infotable::instance().GetIntInfo("TRANSACTIONS_FILTER_VIEW_NO", 0);
    } else {
        Model_Setting::instance().Set("TRANSACTIONS_FILTER_VIEW_NO", id);
    }
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
    settings_string_.Clear();
    dataToControls();
}
void mmFilterTransactionsDialog::datePresetMenuSelected( wxCommandEvent& event )
{
    int id =  event.GetId();
    setPresettings(DATE_PRESETTINGS[id]);
}

void mmFilterTransactionsDialog::datePresetMenu( wxMouseEvent& event )
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
    if (date_range_) delete date_range_;
    date_range_ = new mmCurrentMonth;
    dateRangeCheckBox_->SetValue(true);

    if (view == VIEW_TRANS_ALL_STR)
        dateRangeCheckBox_->SetValue(false);
    else if (view == VIEW_TRANS_TODAY_STR)
        date_range_ = new mmToday;
    else if (view == VIEW_TRANS_CURRENT_MONTH_STR)
        date_range_ = new mmCurrentMonth;
    else if (view == VIEW_TRANS_LAST_30_DAYS_STR)
        date_range_ = new mmLast30Days;
    else if (view == VIEW_TRANS_LAST_90_DAYS_STR)
        date_range_ = new mmLast90Days;
    else if (view == VIEW_TRANS_LAST_MONTH_STR)
        date_range_ = new mmLastMonth;
    else if (view == VIEW_TRANS_LAST_3MONTHS_STR)
        date_range_ = new mmLast3Months;
    else if (view == VIEW_TRANS_CURRENT_YEAR_STR)
        date_range_ = new mmCurrentYear;
    else if (view == VIEW_TRANS_LAST_12MONTHS_STR)
        date_range_ = new mmLast12Months;

    if (dateRangeCheckBox_->IsChecked())
    {
        fromDateCtrl_->SetValue(date_range_->start_date());
        toDateControl_->SetValue(date_range_->end_date());
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

bool mmFilterTransactionsDialog::checkAll(const Model_Checking::Data &tran, const int accountID, const std::map<int, Model_Splittransaction::Data_Set>& split)
{
    bool ok = true;
    //wxLogDebug("Check date? %i trx date:%s %s %s", getDateRangeCheckBox(), tran.TRANSDATE, getFromDateCtrl().GetDateOnly().FormatISODate(), getToDateControl().GetDateOnly().FormatISODate());
    if (getAccountCheckBox() && (getAccountID() != tran.ACCOUNTID && getAccountID() != tran.TOACCOUNTID))
        ok = false;
    else if 
    (   getDateRangeCheckBox()
        && !Model_Checking::TRANSDATE(tran).IsBetween(
            getFromDateCtrl().GetDateOnly(), getToDateControl().GetDateOnly())
    )
        ok = false;
    else if (getPayeeCheckBox() && !checkPayee<Model_Checking>(tran)) ok = false;
    else if (getCategoryCheckBox() && !checkCategory<Model_Checking>(tran, split)) ok = false;
    else if (getStatusCheckBox() && !compareStatus(tran.STATUS)) ok = false;
    else if (getTypeCheckBox() && !allowType(tran.TRANSCODE, accountID == tran.ACCOUNTID)) ok = false;
    else if (getAmountRangeCheckBoxMin() && getAmountMin() > tran.TRANSAMOUNT) ok = false;
    else if (getAmountRangeCheckBoxMax() && getAmountMax() < tran.TRANSAMOUNT) ok = false;
    else if (getNumberCheckBox() && getNumber() != tran.TRANSACTIONNUMBER) ok = false;
    else if (getNotesCheckBox() && !tran.NOTES.Lower().Contains(getNotes().Lower())) ok = false;
    return ok;
}
bool mmFilterTransactionsDialog::checkAll(const Model_Billsdeposits::Data &tran, const std::map<int, Model_Budgetsplittransaction::Data_Set>& split)
{
    bool ok = true;
    if (getAccountCheckBox() && (getAccountID() != tran.ACCOUNTID && getAccountID() != tran.TOACCOUNTID)) ok = false;
    else if (getDateRangeCheckBox()
        && !Model_Billsdeposits::TRANSDATE(tran)
            .IsBetween(getFromDateCtrl().GetDateOnly()
            , getToDateControl().GetDateOnly()
        )
    ) ok = false;
    else if (getPayeeCheckBox() && !checkPayee<Model_Billsdeposits>(tran)) ok = false;
    else if (getCategoryCheckBox() && !checkCategory<Model_Billsdeposits>(tran, split)) ok = false;
    else if (getStatusCheckBox() && !compareStatus(tran.STATUS)) ok = false;
    else if (getTypeCheckBox() && !allowType(tran.TRANSCODE, true)) ok = false;
    else if (getAmountRangeCheckBoxMin() && getAmountMin() > tran.TRANSAMOUNT) ok = false;
    else if (getAmountRangeCheckBoxMax() && getAmountMax() < tran.TRANSAMOUNT) ok = false;
    else if (getNumberCheckBox() && getNumber() != tran.TRANSACTIONNUMBER) ok = false;
    else if (getNotesCheckBox() && !tran.NOTES.Lower().Contains(getNotes().Lower())) ok = false;
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
    wxString filterDetails = to_json();
    filterDetails.Replace(",\n", "<br>");
    filterDetails.replace(0, 1, ' ');
    filterDetails.RemoveLast(1);
    hb.addText(filterDetails);
}

wxString mmFilterTransactionsDialog::to_json()
{
    json::Object o;
    o.Clear();
    const wxString label = m_settingLabel->GetValue().Trim();
    if (!label.empty()) o[L"LABEL"] = json::String(label.ToStdWstring());
    if (accountCheckBox_->IsChecked())
    {
        const wxString acc = accountDropDown_->GetStringSelection();
        if (!acc.empty()) o[L"ACCOUNT"] = json::String(acc.ToStdWstring());
    }
    if (dateRangeCheckBox_->IsChecked())
    {
        o[L"DATE1"] = json::String(fromDateCtrl_->GetValue().FormatISODate().ToStdWstring());
        o[L"DATE2"] = json::String(toDateControl_->GetValue().FormatISODate().ToStdWstring());
    }
    if (payeeCheckBox_->IsChecked())
    {
        wxString wxpayee = cbPayee_->GetValue();
        //TODO: Here is big problem for UTF8 usage!!! wxString::ToStdString() does not working
        //for some strings like Kubalíková
        //but cyrillic working Николай = Николай (РќРёРєРѕР»Р°Р№)
        //Kubalíková ----> KubalГ­kovГЎ
        wxCharBuffer buffer = wxpayee.ToUTF8();
        std::string str_std(buffer.data(), strlen(buffer.data()));
        std::wstring test = wxpayee.ToStdWstring();
        wxLogDebug("utf8: %s|to_chars %s|from_chars %s|std::string: %s"
            , wxpayee, str_std, wxString(str_std.c_str(), wxConvUTF8), test);

        if (!test.empty()) o[L"PAYEE"] = json::String(test);
    }
    if (categoryCheckBox_->IsChecked())
    {
        o[L"SIMILAR_YN"] = json::Boolean(bSimilarCategoryStatus_);
        o[L"CATEGORY"] = json::String(btnCategory_->GetLabel().ToStdWstring());
    }
    if (statusCheckBox_->IsChecked())
    {
        int item = choiceStatus_->GetSelection();
        wxString status;
        if (0 <= item && item < (int)Model_Checking::all_status().size())
            status = Model_Checking::all_status()[item];
        if (!status.empty()) o[L"STATUS"] = json::String(status.ToStdWstring());
    }
    if (typeCheckBox_->IsChecked())
    {
        wxString type = wxString()
            << (cbTypeWithdrawal_->GetValue() && typeCheckBox_->GetValue() ? "W" : "")
            << (cbTypeDeposit_->GetValue() && typeCheckBox_->GetValue() ? "D" : "")
            << (cbTypeTransferTo_->GetValue() && typeCheckBox_->GetValue() ? "T" : "")
            << (cbTypeTransferFrom_->GetValue() && typeCheckBox_->GetValue() ? "F" : "");
        if (!type.empty()) o[L"TYPE"] = json::String(type.ToStdWstring());
    }

    if (amountRangeCheckBox_->IsChecked())
    {
        if (!amountMinEdit_->GetValue().empty())
        {
            double amount_min;
            amountMinEdit_->GetDouble(amount_min);
            o[L"AMOUNT_MIN"] = json::Number(amount_min);
        }
        if (!amountMaxEdit_->GetValue().empty())
        {
            double amount_max;
            amountMaxEdit_->GetDouble(amount_max);
            o[L"AMOUNT_MAX"] = json::Number(amount_max);
        }
    }
    if (transNumberCheckBox_->IsChecked())
    {
        const wxString num = transNumberEdit_->GetValue();
        if (!num.empty()) o[L"NUMBER"] = json::String(num.ToStdWstring());
    }
    if (notesCheckBox_->IsChecked())
    {
        const wxString notes = notesEdit_->GetValue();
        if (!notes.empty()) o[L"NOTES"] = json::String(notes.ToStdWstring());
    }

    std::wstringstream ss;
    json::Writer::Write(o, ss);
    return ss.str();
}

void mmFilterTransactionsDialog::from_json(const wxString &data)
{
    wxString str = data;
    if (!(str.StartsWith("{") && str.EndsWith("}"))) str = "{}";
    std::wstringstream ss;
    ss << str.ToStdWstring();
    json::Object o;
    json::Reader::Read(o, ss);
    
    //Label
    m_settingLabel->ChangeValue(wxString(json::String(o[L"LABEL"])));
    
    //Account
    accountCheckBox_->SetValue(!wxString(json::String(o[L"ACCOUNT"])).empty());
    accountDropDown_->Enable(accountCheckBox_->IsChecked());
    accountDropDown_->SetStringSelection(wxString(json::String(o[L"ACCOUNT"])));

    //Dates
    dateRangeCheckBox_->SetValue(!wxString(json::String(o[L"DATE"])).empty() || !wxString(json::String(o[L"DATE2"])).empty());
    fromDateCtrl_->Enable(dateRangeCheckBox_->IsChecked());
    fromDateCtrl_->SetValue(mmGetStorageStringAsDate(wxString(json::String(o[L"DATE1"]))));
    toDateControl_->Enable(dateRangeCheckBox_->IsChecked());
    toDateControl_->SetValue(mmGetStorageStringAsDate(wxString(json::String(o[L"DATE2"]))));

    //Payee
    payeeCheckBox_->SetValue(!wxString(json::String(o[L"PAYEE"])).empty());
    cbPayee_->Enable(payeeCheckBox_->IsChecked());
    cbPayee_->SetValue(wxString(json::String(o[L"PAYEE"])));

    //Category
    wxString value = wxString(json::String(o[L"CATEGORY"]));
    categoryCheckBox_->SetValue(!value.empty());
    btnCategory_->Enable(categoryCheckBox_->IsChecked());
    bSimilarCategoryStatus_ = json::Boolean(o[L"SIMILAR_YN"]);
    similarCategCheckBox_->SetValue(bSimilarCategoryStatus_);
    similarCategCheckBox_->Enable(categoryCheckBox_->IsChecked());
    wxStringTokenizer categ_token(value, ":", wxTOKEN_RET_EMPTY_ALL);
    Model_Category::Data* category = Model_Category::instance().get(categ_token.GetNextToken().Trim());
    if (category)
        categID_ = category->CATEGID;
    Model_Subcategory::Data* sub_category = 0;
    wxString subcateg_name = categ_token.GetNextToken().Trim(false);
    if (!subcateg_name.IsEmpty())
    {
        sub_category = Model_Subcategory::instance().get(subcateg_name, categID_);
        if (sub_category)
            subcategID_ = sub_category->SUBCATEGID;
    }
    btnCategory_->SetLabelText(Model_Category::full_name(categID_, subcategID_));

    //Status
    statusCheckBox_->SetValue(!wxString(json::String(o[L"STATUS"])).empty());
    choiceStatus_->Enable(statusCheckBox_->IsChecked());
    choiceStatus_->SetStringSelection(wxGetTranslation(wxString(json::String(o[L"STATUS"]))));

    //Type
    wxString type = wxString(json::String(o[L"TYPE"]));
    typeCheckBox_->SetValue(!type.empty());
    cbTypeWithdrawal_->SetValue(type.Contains("W"));
    cbTypeWithdrawal_->Enable(typeCheckBox_->IsChecked());
    cbTypeDeposit_->SetValue(type.Contains("D"));
    cbTypeDeposit_->Enable(typeCheckBox_->IsChecked());
    cbTypeTransferTo_->SetValue(type.Contains("T"));
    cbTypeTransferTo_->Enable(typeCheckBox_->IsChecked());
    cbTypeTransferFrom_->SetValue(type.Contains("F"));
    cbTypeTransferFrom_->Enable(typeCheckBox_->IsChecked());

    //Amounts
    bool amt1 = 0.0 != json::Number(o[L"AMOUNT_MIN"]);
    bool amt2 = 0.0 != json::Number(o[L"AMOUNT_MAX"]);
    amountRangeCheckBox_->SetValue(amt1 || amt2);
    amountMinEdit_->Enable(amountRangeCheckBox_->IsChecked());
    amountMaxEdit_->Enable(amountRangeCheckBox_->IsChecked());
    if (amt1)
        amountMinEdit_->SetValue(json::Number(o[L"AMOUNT_MIN"]));
    else
        amountMinEdit_->ChangeValue("");
    if (amt2)
        amountMaxEdit_->SetValue(json::Number(o[L"AMOUNT_MAX"]));
    else
        amountMaxEdit_->ChangeValue("");

    //Number
    transNumberCheckBox_->SetValue(!wxString(json::String(o[L"NUMBER"])).empty());
    transNumberEdit_->Enable(transNumberCheckBox_->IsChecked());
    transNumberEdit_->ChangeValue(wxString(json::String(o[L"NUMBER"])));

    //Notes
    notesCheckBox_->SetValue(!wxString(json::String(o[L"NOTES"])).empty());
    notesEdit_->Enable(notesCheckBox_->IsChecked());
    notesEdit_->ChangeValue(wxString(json::String(o[L"NOTES"])));
}
