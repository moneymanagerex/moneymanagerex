
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "filtertransdialog.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "filtertransdialog.h"
#include "constants.h"
#include "util.h"
#include "mmCurrencyFormatter.h"
#include "categdialog.h"
#include "paths.h"
#include "validators.h"
#include <wx/valnum.h>

IMPLEMENT_DYNAMIC_CLASS( mmFilterTransactionsDialog, wxDialog )

BEGIN_EVENT_TABLE( mmFilterTransactionsDialog, wxDialog )

    EVT_CHECKBOX(wxID_ANY,    mmFilterTransactionsDialog::OnCheckboxClick )
    EVT_BUTTON  (wxID_OK,     mmFilterTransactionsDialog::OnButtonokClick )
    EVT_BUTTON  (wxID_CANCEL, mmFilterTransactionsDialog::OnButtoncancelClick )
    EVT_BUTTON  (wxID_SAVE,   mmFilterTransactionsDialog::OnButtonSaveClick )
    EVT_BUTTON  (wxID_CLEAR,  mmFilterTransactionsDialog::OnButtonClearClick )
    EVT_MENU    (wxID_ANY,    mmFilterTransactionsDialog::datePresetMenuSelected )

END_EVENT_TABLE()

mmFilterTransactionsDialog::mmFilterTransactionsDialog( )
{
}

mmFilterTransactionsDialog::mmFilterTransactionsDialog(
    mmCoreDB* core,
    wxWindow* parent, wxWindowID id, const wxString& caption,
    const wxPoint& pos, const wxSize& size,long style)
: core_(core)
, categID_(-1)
, subcategID_(-1)
, payeeID_(-1)
, refAccountID_(-1)
{
    Create(parent, id, caption, pos, size, style);
}

bool mmFilterTransactionsDialog::Create( wxWindow* parent, wxWindowID id,
                                        const wxString& caption, const wxPoint& pos,
                                        const wxSize& size, long style )
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    GetStoredSettings(-1);

    dataToControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());

    Centre();
    return true;
}

void mmFilterTransactionsDialog::dataToControls()
{
    wxStringTokenizer tkz(settings_string_, ";", wxTOKEN_RET_EMPTY_ALL);
    wxString value = "";
    bool status;

    status = get_next_value(tkz, value);
    accountCheckBox_ ->SetValue(status);
    accountDropDown_ ->Enable(status);
    accountDropDown_ ->SetStringSelection(value);

    status = get_next_value(tkz, value);
    dateRangeCheckBox_ ->SetValue(status);
    fromDateCtrl_ ->Enable(status);
    fromDateCtrl_ ->SetValue(mmGetStorageStringAsDate(value));
    get_next_value(tkz, value);
    toDateControl_ ->Enable(status);
    toDateControl_ ->SetValue(mmGetStorageStringAsDate(value));

    status = get_next_value(tkz, value);
    cbPayee_ ->Enable(status);
    cbPayee_ ->SetValue(value);
    payeeCheckBox_->SetValue(status);

    status = get_next_value(tkz, value);
    categoryCheckBox_ ->SetValue(status);
    btnCategory_ ->Enable(status);

    wxStringTokenizer categ_token(value, ":", wxTOKEN_RET_EMPTY_ALL);
    categID_ = core_->categoryList_.GetCategoryId(categ_token.GetNextToken().Trim());
    wxString subcateg_name = categ_token.GetNextToken().Trim(false);
    if (!subcateg_name.IsEmpty())
        subcategID_ = core_->categoryList_.GetSubCategoryID(categID_, subcateg_name);
    btnCategory_ ->SetLabel(core_->categoryList_.GetFullCategoryString(categID_, subcategID_));
    bExpandStaus_ = true;

    status = get_next_value(tkz, value);
    statusCheckBox_ ->SetValue(status);
    choiceStatus_ ->Enable(status);
    choiceStatus_ ->SetStringSelection(value);

    status = get_next_value(tkz, value);
    typeCheckBox_ ->SetValue(status);
    cbTypeWithdrawal_ ->SetValue(value.Contains("W"));
    cbTypeWithdrawal_ ->Enable(status);
    cbTypeDeposit_ ->SetValue(value.Contains("D"));
    cbTypeDeposit_ ->Enable(status);
    cbTypeTransfer_ ->SetValue(value.Contains("T"));
    cbTypeTransfer_ ->Enable(status);

    status = get_next_value(tkz, value);
    amountRangeCheckBox_ ->SetValue(status);
    amountMinEdit_ ->Enable(status);
    amountMinEdit_ ->SetValue(value);
    get_next_value(tkz, value);
    amountMaxEdit_ ->Enable(status);
    amountMaxEdit_ ->SetValue(value);

    status = get_next_value(tkz, value);
    transNumberCheckBox_ ->SetValue(status);
    transNumberEdit_ ->Enable(status);
    transNumberEdit_ ->SetValue(value);

    status = get_next_value(tkz, value);
    notesCheckBox_ ->SetValue(status);
    notesEdit_ ->Enable(status);
    notesEdit_ ->SetValue(value);

}
void mmFilterTransactionsDialog::CreateControls()
{
    wxSizerFlags flags, flagsExpand;
    flags.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 0);
    flagsExpand.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxEXPAND).Border(wxALL, 0).Proportion(1);

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, flagsExpand);

    wxStaticBox* static_box_sizer = new wxStaticBox(this, wxID_ANY, _(" Specify "));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(static_box_sizer, wxVERTICAL);
    itemBoxSizer3->Add(itemStaticBoxSizer4, 1, wxGROW|wxALL, 10);

    this->SetSizer(itemBoxSizer2);

    /******************************************************************************
     Items Panel
    *******************************************************************************/
    wxPanel* itemPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemStaticBoxSizer4->Add(itemPanel, 1, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* itemPanelSizer = new wxFlexGridSizer(0, 2, 10, 5);
    itemPanelSizer->AddGrowableCol(1, 1);

    itemPanel->SetSizer(itemBoxSizer4);
    itemBoxSizer4->Add(itemPanelSizer, flagsExpand);
    //--Start of Row --------------------------------------------------------
    accountCheckBox_ = new wxCheckBox( itemPanel, wxID_ANY, _("Account"),
                                      wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    itemPanelSizer->Add(accountCheckBox_, flags);

    wxArrayString as = core_->accountList_.getAccountsName();

    accountDropDown_ = new wxChoice( itemPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, as, 0 );
    itemPanelSizer->Add(accountDropDown_, flagsExpand);

    //--End of Row --------------------------------------------------------

    dateRangeCheckBox_ = new wxCheckBox( itemPanel, wxID_ANY, _("Date Range"),
                                        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    itemPanelSizer->Add(dateRangeCheckBox_, flags);

    fromDateCtrl_ = new wxDatePickerCtrl( itemPanel, wxID_ANY, wxDefaultDateTime,
                                         wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
    toDateControl_ = new wxDatePickerCtrl( itemPanel, wxID_ANY, wxDefaultDateTime,
                                          wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
    dateRangeCheckBox_->Connect(wxID_ANY, wxEVT_RIGHT_DOWN
        , wxMouseEventHandler(mmFilterTransactionsDialog::datePresetMenu), NULL, this);

    wxBoxSizer* dateSizer = new wxBoxSizer(wxHORIZONTAL);
    dateSizer->Add(fromDateCtrl_, flagsExpand);
    dateSizer->AddSpacer(5);
    dateSizer->Add(toDateControl_, flagsExpand);
    itemPanelSizer->Add(dateSizer, flagsExpand);
    //--End of Row --------------------------------------------------------

    payeeCheckBox_ = new wxCheckBox( itemPanel, wxID_ANY, _("Payee"),
                                    wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    itemPanelSizer->Add(payeeCheckBox_, flags);

    cbPayee_ = new wxComboBox(itemPanel, wxID_ANY, "",
        wxDefaultPosition, wxDefaultSize,
        core_->payeeList_.FilterPayees("") /*, wxTE_PROCESS_ENTER*/);
    cbPayee_->Connect(wxID_ANY, wxEVT_COMMAND_TEXT_UPDATED,
        wxCommandEventHandler(mmFilterTransactionsDialog::OnPayeeUpdated), NULL, this);

    cbPayee_->AutoComplete(core_->payeeList_.FilterPayees(""));

    itemPanelSizer->Add(cbPayee_, flagsExpand);
    //--End of Row --------------------------------------------------------

    categoryCheckBox_ = new wxCheckBox( itemPanel, wxID_ANY, _("Category"),
                                       wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    //categoryCheckBox_->SetValue(false);
    itemPanelSizer->Add(categoryCheckBox_, flags);

    btnCategory_ = new wxButton( itemPanel, wxID_ANY, "",
                                wxDefaultPosition, wxDefaultSize);
    btnCategory_->Connect(wxID_ANY, wxEVT_COMMAND_BUTTON_CLICKED,
        wxCommandEventHandler(mmFilterTransactionsDialog::OnCategs), NULL, this);

    itemPanelSizer->Add(btnCategory_, flagsExpand);
    //--End of Row --------------------------------------------------------

    statusCheckBox_ = new wxCheckBox( itemPanel, wxID_ANY, _("Status"),
                                     wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    statusCheckBox_->SetValue(false);
    itemPanelSizer->Add(statusCheckBox_, flags);

    choiceStatus_ = new wxChoice(itemPanel, wxID_ANY);

    for(const auto i : TRANSACTION_STATUSES)
        choiceStatus_->Append(wxGetTranslation(i), new wxStringClientData(i));

    itemPanelSizer->Add(choiceStatus_, flagsExpand);
    choiceStatus_->SetToolTip(_("Specify the status for the transaction"));
    //--End of Row --------------------------------------------------------

    typeCheckBox_ = new wxCheckBox( itemPanel, wxID_ANY, _("Type"),
                                   wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );

    wxFlexGridSizer* typeSizer = new wxFlexGridSizer(0, 2, 0, 0);
    typeSizer->AddGrowableCol(0, 1);
    typeSizer->AddGrowableCol(1, 1);
    cbTypeWithdrawal_ = new wxCheckBox( itemPanel, wxID_ANY, _("Withdrawal"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    cbTypeDeposit_ = new wxCheckBox( itemPanel, wxID_ANY, _("Deposit"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    cbTypeTransfer_ = new wxCheckBox( itemPanel, wxID_ANY, _("Transfer"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );

    itemPanelSizer->Add(typeCheckBox_, flags);
    itemPanelSizer->Add(typeSizer, flagsExpand);
    typeSizer ->Add(cbTypeWithdrawal_, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 2);
    typeSizer ->Add(cbTypeDeposit_, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 2);
    typeSizer ->Add(cbTypeTransfer_, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 2);
    typeSizer ->AddSpacer(2);

    //--End of Row --------------------------------------------------------

    amountRangeCheckBox_ = new wxCheckBox( itemPanel, wxID_ANY, _("Amount Range"),
                                          wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    itemPanelSizer->Add(amountRangeCheckBox_, flags);

    amountMinEdit_ = new wxTextCtrl( itemPanel, wxID_ANY, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT|wxTE_PROCESS_ENTER
        , mmDoubleValidator2() );
    amountMaxEdit_ = new wxTextCtrl( itemPanel, wxID_ANY, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT|wxTE_PROCESS_ENTER
        , mmDoubleValidator2() );

    wxBoxSizer* amountSizer = new wxBoxSizer(wxHORIZONTAL);
    amountSizer->Add(amountMinEdit_, flagsExpand);
    amountSizer->AddSpacer(5);
    amountSizer->Add(amountMaxEdit_, flagsExpand);
    itemPanelSizer->Add(amountSizer, flagsExpand);
    //--End of Row --------------------------------------------------------

    transNumberCheckBox_ = new wxCheckBox( itemPanel, wxID_ANY, _("Number"),
        wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    itemPanelSizer->Add(transNumberCheckBox_, flags);

    transNumberEdit_ = new wxTextCtrl( itemPanel, wxID_ANY);
    itemPanelSizer->Add(transNumberEdit_, flagsExpand);
    //--End of Row --------------------------------------------------------

    notesCheckBox_ = new wxCheckBox( itemPanel, wxID_ANY, _("Notes"),
                                    wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    itemPanelSizer->Add(notesCheckBox_, flags);

    notesEdit_ = new wxTextCtrl( itemPanel, wxID_ANY);
    itemPanelSizer->Add(notesEdit_, flagsExpand);
    //--End of Row --------------------------------------------------------

    wxBoxSizer* settings_box_sizer = new wxBoxSizer(wxHORIZONTAL);

    wxString choices[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
    int num = sizeof(choices) / sizeof(wxString);
    m_radio_box_ = new wxRadioBox(this, wxID_APPLY, "",
        wxDefaultPosition, wxDefaultSize, num, choices, num, wxRA_SPECIFY_COLS);
    m_radio_box_->Connect(wxID_APPLY, wxEVT_COMMAND_RADIOBOX_SELECTED,
        wxCommandEventHandler(mmFilterTransactionsDialog::OnSettingsSelected), NULL, this);

    int view_no = core_->dbInfoSettings_->GetIntSetting("TRANSACTIONS_FILTER_VIEW_NO", 0);
    m_radio_box_->SetSelection(view_no);
    m_radio_box_->Show(true);

    itemBoxSizer3->Add(settings_box_sizer, flags.Center());
    settings_box_sizer->Add( m_radio_box_, flags);

    /******************************************************************************
     Button Panel with OK/Cancel buttons
    *******************************************************************************/
    wxPanel* buttonPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemBoxSizer3->Add(buttonPanel, flags.Border(wxALL, 5));

    wxBoxSizer* buttonPanelSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonPanel->SetSizer(buttonPanelSizer);

    wxButton* itemButtonOK = new wxButton( buttonPanel, wxID_OK);

    wxButton* itemButtonCancel = new wxButton( buttonPanel, wxID_CANCEL);
    itemButtonCancel->SetFocus();

    wxButton* itemButtonClear = new wxButton( buttonPanel, wxID_CLEAR);

    wxBitmapButton* save_button = new wxBitmapButton( buttonPanel,
        wxID_SAVE, wxBitmap(save_xpm), wxDefaultPosition,
        wxSize(itemButtonOK->GetSize().GetHeight(), itemButtonOK->GetSize().GetHeight()));

    save_button->Show(true);

    buttonPanelSizer->Add(save_button, flags.Border(wxALL, 5));
    buttonPanelSizer->Add(itemButtonOK, flags);
    buttonPanelSizer->Add(itemButtonCancel, flags);
    buttonPanelSizer->Add(itemButtonClear, flags);
}

/*!
 * Should we show tooltips?
 */

bool mmFilterTransactionsDialog::ShowToolTips()
{
    return TRUE;
}

void mmFilterTransactionsDialog::OnCheckboxClick( wxCommandEvent& event )
{
    btnCategory_->Enable(categoryCheckBox_->IsChecked());
    accountDropDown_->Enable(accountCheckBox_->IsChecked());
    fromDateCtrl_->Enable(dateRangeCheckBox_->IsChecked());
    toDateControl_->Enable(dateRangeCheckBox_->IsChecked());
    cbPayee_->Enable(payeeCheckBox_->IsChecked());
    btnCategory_->Enable(categoryCheckBox_->IsChecked());
    choiceStatus_->Enable(statusCheckBox_->IsChecked());
    cbTypeWithdrawal_->Enable(typeCheckBox_->IsChecked());
    cbTypeDeposit_->Enable(typeCheckBox_->IsChecked());
    cbTypeTransfer_->Enable(typeCheckBox_->IsChecked());
    amountMinEdit_->Enable(amountRangeCheckBox_->IsChecked());
    amountMaxEdit_->Enable(amountRangeCheckBox_->IsChecked());
    notesEdit_->Enable(notesCheckBox_->IsChecked());
    transNumberEdit_->Enable(transNumberCheckBox_->IsChecked());
    accountDropDown_->Enable(accountCheckBox_->IsChecked());

    event.Skip();
}

void mmFilterTransactionsDialog::OnButtonokClick( wxCommandEvent& /*event*/ )
{
    if (accountCheckBox_->IsChecked())
    {
        refAccountStr_ = accountDropDown_->GetStringSelection();
        refAccountID_ = core_->accountList_.GetAccountId(refAccountStr_);
    }

    if (payeeCheckBox_->IsChecked())
    {
        payeeID_ = core_->payeeList_.GetPayeeId(cbPayee_->GetValue());
    }

    if (amountRangeCheckBox_->IsChecked())
    {
        wxString minamt = amountMinEdit_->GetValue();
        wxString maxamt = amountMaxEdit_->GetValue();
        if (!minamt.IsEmpty())
        {
            double amount;
            if (! CurrencyFormatter::formatCurrencyToDouble(minamt, amount) || (amount < 0.0))
            {
                mmShowErrorMessage(this, _("Invalid Amount Entered "), _("Error"));
                return;
            }
        }

        if (!maxamt.IsEmpty())
        {
            double amount;
            if (! CurrencyFormatter::formatCurrencyToDouble(maxamt, amount) || (amount < 0.0))
            {
                mmShowErrorMessage(this, _("Invalid Amount Entered "), _("Error"));
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
    mmCategDialog dlg(core_, this);

    dlg.setTreeSelection(core_->categoryList_.GetCategoryName(categID_)
    , core_->categoryList_.GetSubCategoryName(categID_,subcategID_));

    if (dlg.ShowModal() == wxID_OK)
    {
        categID_ = dlg.getCategId();
        subcategID_ = dlg.getSubCategId();
        btnCategory_->SetLabel(core_->categoryList_.GetFullCategoryString(categID_, subcategID_));
        bExpandStaus_ = dlg.getExpandStatus();
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
    || getAmountRangeCheckBox()
    || getNumberCheckBox()
    || getNotesCheckBox();
}

wxString mmFilterTransactionsDialog::getAccountName()
{
    wxString accountName = core_->accountList_.GetAccountName(refAccountID_);
    return accountName;
}

bool mmFilterTransactionsDialog::getDateRange(wxDateTime& startDate, wxDateTime& endDate) const
{
    if (dateRangeCheckBox_->IsChecked())
    {
        startDate = fromDateCtrl_->GetValue();
        endDate = toDateControl_->GetValue();
        return true;
    }
    return false;
}

wxString mmFilterTransactionsDialog::userDateRangeStr() const
{
    wxString dateStr;
    if (dateRangeCheckBox_->IsChecked())
    {
        wxString dtBegin = mmGetDateForDisplay(fromDateCtrl_->GetValue());
        wxString dtEnd = mmGetDateForDisplay(toDateControl_->GetValue());
        dateStr << wxString::Format(_("From %s till %s"), dtBegin, dtEnd);
    }
    return dateStr;
}

int mmFilterTransactionsDialog::getPayeeID() const
{
    wxString payeeStr = cbPayee_->GetValue();
    int payeeID = core_->payeeList_.GetPayeeId(payeeStr);
    return payeeID;

}
wxString mmFilterTransactionsDialog::userPayeeStr() const
{
    if (payeeCheckBox_->IsChecked())
        return cbPayee_->GetValue();
    return "";
}

wxString mmFilterTransactionsDialog::userCategoryStr() const
{
    if (categoryCheckBox_->IsChecked())
        return btnCategory_->GetLabelText();
    return "";
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

wxString mmFilterTransactionsDialog::getType() const
{
    wxString withdraval = "";
    wxString deposit = "";
    wxString transfer = "";
    if (cbTypeWithdrawal_->GetValue())
        withdraval = TRANS_TYPE_WITHDRAWAL_STR;
    if (cbTypeDeposit_->GetValue())
        deposit = TRANS_TYPE_DEPOSIT_STR;
    if (cbTypeTransfer_->GetValue())
        transfer = TRANS_TYPE_TRANSFER_STR;

    return withdraval+";"+deposit+";"+transfer;
}

wxString mmFilterTransactionsDialog::userTypeStr() const
{
    wxString transCode = wxEmptyString;
    if (typeCheckBox_->IsChecked())
    {
        if (cbTypeWithdrawal_->GetValue())
            transCode = wxGetTranslation(TRANS_TYPE_WITHDRAWAL_STR);
        if (cbTypeDeposit_->GetValue())
            transCode << (transCode.IsEmpty() ? "" : ", ") << wxGetTranslation(TRANS_TYPE_DEPOSIT_STR);
        if (cbTypeTransfer_->GetValue())
            transCode << (transCode.IsEmpty() ? "" : ", ") << wxGetTranslation(TRANS_TYPE_TRANSFER_STR);
    }
    return transCode;
}

wxString mmFilterTransactionsDialog::userStatusStr() const
{
    if (statusCheckBox_->IsChecked())
        return choiceStatus_->GetStringSelection();
    return "";
}

double mmFilterTransactionsDialog::getAmountMin()
{
    double amount = 0;
    if (! CurrencyFormatter::formatCurrencyToDouble(amountMinEdit_->GetValue(), amount) || (amount < 0.0))
        amount = 0;

    return amount;
}

double mmFilterTransactionsDialog::getAmountMax()
{
    double amount = 0;
    if (! CurrencyFormatter::formatCurrencyToDouble(amountMaxEdit_->GetValue(), amount) || (amount < 0.0))
        amount = 0;

    return amount;
}

wxString mmFilterTransactionsDialog::userAmountRangeStr() const
{
    wxString amountRangeStr;
    if (amountRangeCheckBox_->IsChecked())
    {
        wxString minamt = amountMinEdit_->GetValue();
        wxString maxamt = amountMaxEdit_->GetValue();
        amountRangeStr << _("Min: ") << minamt << " " << _("Max: ") << maxamt;
    }
    return amountRangeStr;
}

void mmFilterTransactionsDialog::OnButtonSaveClick( wxCommandEvent& /*event*/ )
{
    int i = m_radio_box_->GetSelection();
    settings_string_ = GetCurrentSettings();
    core_->dbInfoSettings_->SetStringSetting(wxString::Format("TRANSACTIONS_FILTER_%d", i), settings_string_);
    core_->dbInfoSettings_->SetIntSetting("TRANSACTIONS_FILTER_VIEW_NO", i);
    wxLogDebug(wxString::Format("Settings Saled to registry %i\n %s", i, settings_string_));
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
        id = core_->dbInfoSettings_->GetIntSetting("TRANSACTIONS_FILTER_VIEW_NO", 0);
    } else {
        core_->iniSettings_->SetIntSetting("TRANSACTIONS_FILTER_VIEW_NO", id);
    }
    settings_string_ = core_->dbInfoSettings_->GetStringSetting(
                              wxString::Format("TRANSACTIONS_FILTER_%d", id),
                              "0;;0;;0;;0;;0;;0;;0;;0;;0;;0;;0;;");
    return settings_string_;
}

bool mmFilterTransactionsDialog::get_next_value( wxStringTokenizer& tkz, wxString& value)
{
    value = "";
    bool on = "1" == tkz.GetNextToken();
    value = tkz.GetNextToken();
    return on;
}

wxString mmFilterTransactionsDialog::GetCurrentSettings()
{
    settings_string_.clear();

    settings_string_ << accountCheckBox_->GetValue() << ";";
    settings_string_ << accountDropDown_->GetStringSelection() << ";";

    settings_string_ << dateRangeCheckBox_->GetValue() << ";";
    settings_string_ << fromDateCtrl_->GetValue().FormatISODate() << ";";
    settings_string_ << dateRangeCheckBox_->GetValue() << ";";
    settings_string_ << toDateControl_->GetValue().FormatISODate() << ";";

    settings_string_ << payeeCheckBox_->GetValue() << ";";
    settings_string_ << cbPayee_->GetValue() << ";";

    settings_string_ << categoryCheckBox_->GetValue() << ";";
    settings_string_ << btnCategory_ ->GetLabel() << ";";

    settings_string_ << statusCheckBox_->GetValue() << ";";
    settings_string_ << choiceStatus_ ->GetStringSelection() << ";";

    settings_string_ << typeCheckBox_->GetValue() << ";"
    << (cbTypeWithdrawal_->GetValue() && typeCheckBox_->GetValue() ? "W" : "")
    << (cbTypeDeposit_->GetValue() && typeCheckBox_->GetValue() ? "D" : "")
    << (cbTypeTransfer_->GetValue() && typeCheckBox_->GetValue() ? "T" : "")
    << ";";

    settings_string_ << amountRangeCheckBox_->GetValue() << ";";
    settings_string_ << amountMinEdit_->GetValue() << ";";
    settings_string_ << amountRangeCheckBox_->GetValue() << ";";
    settings_string_ << amountMaxEdit_->GetValue() << ";";

    settings_string_ << transNumberCheckBox_->GetValue() << ";";
    settings_string_ << transNumberEdit_->GetValue() << ";";

    settings_string_ << notesCheckBox_->GetValue() << ";";
    settings_string_ << notesEdit_->GetValue() << ";";

    return settings_string_;
}

void mmFilterTransactionsDialog::setAccountToolTip(const wxString& tip) const
{
    accountDropDown_->SetToolTip(tip);
}

void mmFilterTransactionsDialog::clearSettings()
{
    settings_string_ = "0;;0;;0;;0;;0;;0;;0;;0;;0;;0;;0;;";
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
        date_range_ = new mmLastMonth;//ToDO:
    else if (view == VIEW_TRANS_CURRENT_YEAR_STR)
        date_range_ = new mmCurrentYear;
    else if (view == VIEW_TRANS_LAST_365_DAYS)//ToDO:
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
    const wxString value = cbPayee_->GetValue().Lower();
    payeeID_ = core_->payeeList_.GetPayeeId(value);
    event.Skip();
}
