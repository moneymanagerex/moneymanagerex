/*******************************************************
 Copyright (C) 2017 - 2021 Nikolay Akimov
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

#include "categdialog.h"
#include "constants.h"
#include "paths.h"
#include "mmTextCtrl.h"
#include "transactionsupdatedialog.h"
#include "validators.h"
#include "mmSimpleDialogs.h"
#include "Model_Account.h"
#include "Model_Checking.h"
#include "Model_Payee.h"
#include <wx/statline.h>

wxIMPLEMENT_DYNAMIC_CLASS(transactionsUpdateDialog, wxDialog);

wxBEGIN_EVENT_TABLE(transactionsUpdateDialog, wxDialog)
    EVT_BUTTON(wxID_OK, transactionsUpdateDialog::OnOk)
    EVT_BUTTON(wxID_VIEW_DETAILS, transactionsUpdateDialog::OnCategChange)
    EVT_CHECKBOX(wxID_ANY, transactionsUpdateDialog::OnCheckboxClick)
    EVT_CHILD_FOCUS(transactionsUpdateDialog::onFocusChange)
    EVT_COMBOBOX(ID_PAYEE, transactionsUpdateDialog::OnPayeeUpdated)
    EVT_COMBOBOX(ID_TRANS_ACC, transactionsUpdateDialog::OnAccountUpdated)
    EVT_CHOICE(ID_TRANS_TYPE, transactionsUpdateDialog::OnTransTypeChanged)
wxEND_EVENT_TABLE()

void transactionsUpdateDialog::SetEventHandlers()
{
    m_payee->Connect(ID_PAYEE, wxEVT_COMMAND_TEXT_UPDATED
        , wxCommandEventHandler(transactionsUpdateDialog::OnPayeeUpdated), nullptr, this);
    m_transferAcc->Connect(ID_TRANS_ACC, wxEVT_COMMAND_TEXT_UPDATED
        , wxCommandEventHandler(transactionsUpdateDialog::OnAccountUpdated), nullptr, this);
}

transactionsUpdateDialog::transactionsUpdateDialog()
{
}

transactionsUpdateDialog::~transactionsUpdateDialog()
{
}

transactionsUpdateDialog::transactionsUpdateDialog(wxWindow* parent
    , std::vector<int>& transaction_id)
    : m_payee_checkbox(nullptr)
    , m_payee(nullptr)
    , m_date_checkbox(nullptr)
    , m_dpc(nullptr)
    , m_status_checkbox(nullptr)
    , m_status_choice(nullptr)
    , m_categ_checkbox(nullptr)
    , m_categ_btn(nullptr)
    , m_type_checkbox(nullptr)
    , m_type_choice(nullptr)
    , m_amount_checkbox(nullptr)
    , m_amount_ctrl(nullptr)
    , m_notes_checkbox(nullptr)
    , m_append_checkbox(nullptr)
    , m_notes_ctrl(nullptr)
    , m_transaction_id(transaction_id)
{
    m_currency = Model_Currency::GetBaseCurrency(); // base currency if we need it

    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_STATIC, _("Multi Transactions Update")
        , wxDefaultPosition, wxSize(500, 300), style);
}

bool transactionsUpdateDialog::Create(wxWindow* parent, wxWindowID id
    , const wxString& caption, const wxPoint& pos
    , const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    CreateControls();

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    this->SetInitialSize();
    SetMinSize(wxSize(300, 400));
    SetIcon(mmex::getProgramIcon());
    Centre();

    SetEventHandlers();
    SetEvtHandlerEnabled(true);
    return TRUE;
}

void transactionsUpdateDialog::CreateControls()
{
    wxBoxSizer* box_sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(box_sizer);

    wxStaticBox* static_box = new wxStaticBox(this, wxID_ANY, _("Specify"));

    wxFlexGridSizer* grid_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    grid_sizer->AddGrowableCol(1, 1);

    // Date --------------------------------------------
    m_date_checkbox = new wxCheckBox(this, wxID_ANY, _("Date")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);

    long date_style = wxDP_DROPDOWN | wxDP_SHOWCENTURY;
    m_dpc = new wxDatePickerCtrl(this
        , wxID_ANY, wxDateTime::Today()
        , wxDefaultPosition, wxDefaultSize, date_style);
    m_dpc->Enable(false);

    grid_sizer->Add(m_date_checkbox, g_flagsH);
    grid_sizer->Add(m_dpc, g_flagsH);

    // Status --------------------------------------------
    m_status_checkbox = new wxCheckBox(this, wxID_ANY, _("Status")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_status_choice = new wxChoice(this, wxID_ANY
        , wxDefaultPosition, wxDefaultSize);
    for (const auto& i : Model_Checking::all_status())
        m_status_choice->Append(wxGetTranslation(i), new wxStringClientData(i));

    m_status_choice->Enable(false);
    m_status_choice->Select(0);

    grid_sizer->Add(m_status_checkbox, g_flagsH);
    grid_sizer->Add(m_status_choice, g_flagsH);

    // Type --------------------------------------------
    m_type_checkbox = new wxCheckBox(this, wxID_ANY, _("Type")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_type_choice = new wxChoice(this, ID_TRANS_TYPE
        , wxDefaultPosition, wxDefaultSize);
    m_type_choice->Enable(false);
    m_type_choice->Select(0);

    for (const auto& i : Model_Checking::all_type())
    {
        m_type_choice->Append(wxGetTranslation(i), new wxStringClientData(i));
    }

    grid_sizer->Add(m_type_checkbox, g_flagsH);
    grid_sizer->Add(m_type_choice, g_flagsH);

    // Amount Field --------------------------------------------
    m_amount_checkbox = new wxCheckBox(this, wxID_ANY, _("Amount")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_amount_ctrl = new mmTextCtrl(this, wxID_ANY, ""
        , wxDefaultPosition, wxDefaultSize
        , wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    m_amount_ctrl->Enable(false);

    grid_sizer->Add(m_amount_checkbox, g_flagsH);
    grid_sizer->Add(m_amount_ctrl, g_flagsH);

    // Payee --------------------------------------------
    m_payee_checkbox = new wxCheckBox(this, wxID_ANY, _("Payee")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);

    m_payee = new wxComboBox(this, ID_PAYEE);
       wxArrayString all_payees = Model_Payee::instance().all_payee_names();
    if (!all_payees.empty()) {
        m_payee->Insert(all_payees, 0);
        m_payee->AutoComplete(all_payees);
    }
    m_payee->Enable(false);

    grid_sizer->Add(m_payee_checkbox, g_flagsH);
    grid_sizer->Add(m_payee, g_flagsExpand);

    // Transfer to account --------------------------------------------
    m_transferAcc_checkbox = new wxCheckBox(this, wxID_ANY, _("Transfer To")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);

    m_transferAcc = new wxComboBox(this, ID_TRANS_ACC);
    wxArrayString account_names = Model_Account::instance().all_checking_account_names(true);
    m_transferAcc->Insert(account_names, 0);
    m_transferAcc->AutoComplete(account_names);
    m_transferAcc->Enable(false);

    grid_sizer->Add(m_transferAcc_checkbox, g_flagsH);
    grid_sizer->Add(m_transferAcc, g_flagsExpand);

    // Category -------------------------------------------------
    m_categ_checkbox = new wxCheckBox(this, wxID_VIEW_DETAILS, _("Category")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);

    m_categ_btn = new wxButton(this, wxID_VIEW_DETAILS, _("Select Category")
        , wxDefaultPosition);
    m_categ_btn->Enable(false);

    grid_sizer->Add(m_categ_checkbox, g_flagsH);
    grid_sizer->Add(m_categ_btn, g_flagsExpand);

    // Notes --------------------------------------------
    m_notes_checkbox = new wxCheckBox(this, wxID_ANY, _("Notes")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_append_checkbox = new wxCheckBox(this, wxID_ANY, _("Append")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_append_checkbox->SetValue(true);

    m_notes_ctrl = new wxTextCtrl(this, wxID_ANY, ""
        , wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
    m_notes_ctrl->Enable(false);
    m_append_checkbox->Enable(false);

    grid_sizer->Add(m_notes_checkbox, g_flagsH);
    grid_sizer->Add(m_append_checkbox, g_flagsH);

    wxStaticBoxSizer* static_box_sizer = new wxStaticBoxSizer(static_box, wxVERTICAL);
    box_sizer->Add(static_box_sizer, 1, wxGROW | wxALL, 10);
    static_box_sizer->Add(grid_sizer, g_flagsExpand);
    static_box_sizer->Add(m_notes_ctrl, 0, wxEXPAND | wxALL, 5);

    /*************************************************************
     Button Panel with OK/Cancel buttons
    *************************************************************/
    wxPanel* button_panel = new wxPanel(this, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    box_sizer->Add(button_panel, wxSizerFlags(g_flagsV).Center());

    wxBoxSizer* button_panel_sizer = new wxBoxSizer(wxHORIZONTAL);
    button_panel->SetSizer(button_panel_sizer);

    wxButton* button_ok = new wxButton(button_panel, wxID_OK, _("&OK "));
    wxButton* button_cancel = new wxButton(button_panel
        , wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    button_cancel->SetFocus();

    button_panel_sizer->Add(button_ok, g_flagsH);
    button_panel_sizer->Add(button_cancel, g_flagsH);
}

void transactionsUpdateDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    double amount = 0;
    if (m_amount_checkbox->IsChecked() && !m_amount_ctrl->checkValue(amount))
        return;

    wxString status = "";
    if (m_status_checkbox->IsChecked())
    {
        wxStringClientData* status_obj = static_cast<wxStringClientData*>(m_status_choice->GetClientObject(m_status_choice->GetSelection()));
        if (status_obj)
            status = Model_Checking::toShortStatus(status_obj->GetData());
        else
            return;
    }

    wxString type = "";
    bool transfer;
    if (m_type_checkbox->IsChecked())
    {
        wxStringClientData* type_obj = static_cast<wxStringClientData*>(m_type_choice->GetClientObject(m_type_choice->GetSelection()));
        type = type_obj->GetData();
        transfer = Model_Checking::is_transfer(type);
    }

    if (m_categ_checkbox->IsChecked())
    {
        const auto c = Model_Category::instance().full_name(m_categ_id, m_subcateg_id);
        if (c.empty())
            return mmErrorDialogs::InvalidCategory(m_categ_btn);
    }

    const auto split = Model_Splittransaction::instance().get_all();

    std::vector<int> skip_trx;
    Model_Checking::instance().Savepoint();
    for (const auto& id : m_transaction_id)
    {
        Model_Checking::Data *trx = Model_Checking::instance().get(id);
        bool is_locked = Model_Checking::is_locked(trx);

        if (m_date_checkbox->IsChecked())
        {
            if (!is_locked) {
                trx->TRANSDATE = m_dpc->GetValue().FormatISODate();
            }
            else {
                skip_trx.push_back(trx->TRANSID);
            }
        }

        if (m_status_checkbox->IsChecked())
        {
            if (!is_locked) {
                trx->STATUS = status;
            }
            else {
                skip_trx.push_back(trx->TRANSID);
            }
        }

        if (m_payee_checkbox->IsChecked())
        {
            wxString payee_name = m_payee->GetValue();
            if (payee_name.IsEmpty())
                return mmErrorDialogs::InvalidPayee(m_payee);
            
            // Get payee string from populated list to address issues with case compare differences between autocomplete and payee list
            int payee_loc = m_payee->FindString(payee_name);
            if (payee_loc != wxNOT_FOUND)
                payee_name = m_payee->GetString(payee_loc);
            
            Model_Payee::Data* payee = Model_Payee::instance().get(payee_name);
            if (!payee)
            {
                wxMessageDialog msgDlg( this
                    , wxString::Format(_("Do you want to add new payee: \n%s?"), payee_name)
                    , _("Confirm to add new payee")
                    , wxYES_NO | wxYES_DEFAULT | wxICON_WARNING);
                if (Option::instance().TransCategorySelection() == Option::UNUSED || msgDlg.ShowModal() == wxID_YES)
                {
                    payee = Model_Payee::instance().create();
                    payee->PAYEENAME = payee_name;
                    Model_Payee::instance().save(payee);
                    mmWebApp::MMEX_WebApp_UpdatePayee();
                }
                else
                    return;
            }

            if (!Model_Checking::is_transfer(trx))
                trx->PAYEEID = payee->PAYEEID;
        }

        if (m_transferAcc_checkbox->IsChecked() && !is_locked)
        {
            wxString account_name = m_transferAcc->GetValue();
            if (account_name.IsEmpty())
                return mmErrorDialogs::InvalidAccount(m_transferAcc, true);
            
            // Get account string from populated list to address issues with case compare differences between autocomplete and account list
            int account_loc = m_transferAcc->FindString(account_name);
            if (account_loc != wxNOT_FOUND)
                account_name = m_transferAcc->GetString(account_loc);
            
            Model_Account::Data* account = Model_Account::instance().get(account_name);
 
            if (Model_Checking::is_transfer(trx) && (trx->ACCOUNTID != account->ACCOUNTID))
                trx->TOACCOUNTID = account->ACCOUNTID;
        }


        if (m_notes_checkbox->IsChecked())
        {
            if (m_append_checkbox->IsChecked()) {
                trx->NOTES += (trx->NOTES.Right(1) == "\n" || trx->NOTES.empty()
                    ? "" : "\n")
                    + m_notes_ctrl->GetValue();
            }
            else {
                trx->NOTES = m_notes_ctrl->GetValue();
            }
        }

        if (m_amount_checkbox->IsChecked())
        {
            if (!is_locked)
            {

                if (Model_Checking::is_transfer(trx))
                {
                    const auto acc = Model_Account::instance().get(trx->ACCOUNTID);
                    const auto curr = Model_Currency::instance().get(acc->CURRENCYID);
                    const auto acc2 = Model_Account::instance().get(trx->TOACCOUNTID);
                    const auto curr2 = Model_Currency::instance().get(acc2->CURRENCYID);
                    if (curr == curr2) {
                        trx->TRANSAMOUNT = amount;
                        trx->TOTRANSAMOUNT = amount;
                    }
                }
                else
                {
                    if (split.find(trx->TRANSID) == split.end()) {
                        trx->TRANSAMOUNT = amount;
                    }
                }
            }
            else {
                skip_trx.push_back(trx->TRANSID);
            }
        }

        if (m_categ_checkbox->IsChecked() && (split.find(trx->TRANSID) == split.end()))
        {
            trx->CATEGID = m_categ_id;
            trx->SUBCATEGID = m_subcateg_id;
        }

        if (m_type_checkbox->IsChecked() && !is_locked)
        {
            if (!Model_Checking::is_transfer(trx->TRANSCODE) && !transfer) {
                trx->TRANSCODE = type;
            } else {
                skip_trx.push_back(trx->TRANSID);
            }
        }

        Model_Checking::instance().save(trx);
    }
    Model_Checking::instance().ReleaseSavepoint();

    skip_trx; //TODO: resume

    EndModal(wxID_OK);
}

void transactionsUpdateDialog::SetPayeeTransferControls()
{
    wxStringClientData* trans_obj = static_cast<wxStringClientData*>(m_type_choice->GetClientObject(m_type_choice->GetSelection()));
    bool transfer = (Model_Checking::is_transfer(trans_obj->GetData()));

    m_payee_checkbox->Enable(!transfer);
    m_transferAcc_checkbox->Enable(transfer);
    if (transfer)
    {
        m_payee_checkbox->SetValue(false);
        m_payee->Enable(false);
    } else
    {
        m_transferAcc_checkbox->SetValue(false);
        m_transferAcc->Enable(false);        
    }
}
void transactionsUpdateDialog::OnTransTypeChanged(wxCommandEvent& event)
{
    SetPayeeTransferControls();
}

void transactionsUpdateDialog::OnCheckboxClick(wxCommandEvent& event)
{

    m_dpc->Enable(m_date_checkbox->IsChecked());
    m_status_choice->Enable(m_status_checkbox->IsChecked());
    m_type_choice->Enable(m_type_checkbox->IsChecked());
    m_payee->Enable(m_payee_checkbox->IsChecked());
    m_transferAcc->Enable(m_transferAcc_checkbox->IsChecked());
    m_categ_btn->Enable(m_categ_checkbox->IsChecked());
    m_amount_ctrl->Enable(m_amount_checkbox->IsChecked());
    m_notes_ctrl->Enable(m_notes_checkbox->IsChecked());
    m_append_checkbox->Enable(m_notes_checkbox->IsChecked());

    if (m_type_checkbox->IsChecked())
    {
        SetPayeeTransferControls();
    } else 
    {
        m_payee_checkbox->Enable(true);
        m_transferAcc_checkbox->Enable(true);
    }

    event.Skip();
}

void transactionsUpdateDialog::onFocusChange(wxChildFocusEvent& event)
{
    int object_in_focus = -1;

    wxWindow *w = event.GetWindow();
    if (w)
    {
        object_in_focus = w->GetId();
    }

    if (object_in_focus == m_amount_ctrl->GetId())
    {
        m_amount_ctrl->SelectAll();
    }
    else
    {
        m_amount_ctrl->Calculate();
    }

    event.Skip();
}

void transactionsUpdateDialog::OnCategChange(wxCommandEvent& WXUNUSED(event))
{
    mmCategDialog dlg(this, true, -1, -1);
    if (dlg.ShowModal() == wxID_OK)
    {
        m_categ_id = dlg.getCategId();
        m_subcateg_id = dlg.getSubCategId();
        m_categ_btn->SetLabelText(Model_Category::full_name(m_categ_id, m_subcateg_id));
    }

}

#if defined (__WXMAC__)
void transactionsUpdateDialog::OnPayeeUpdated(wxCommandEvent& event)
{
    // Filtering the combobox as the user types because on Mac autocomplete function doesn't work
    // PLEASE DO NOT REMOVE!!!

    wxString payeeName = event.GetString();
    if (m_payee->GetSelection() == -1) // make sure nothing is selected (ex. user presses down arrow)
    {
        m_payee->SetEvtHandlerEnabled(false); // things will crash if events are handled during Clear
        m_payee->Clear();      
        Model_Payee::Data_Set filtd = Model_Payee::instance().FilterPayees(payeeName);        
        std::sort(filtd.rbegin(), filtd.rend(), SorterByPAYEENAME());
        for (const auto &payee : filtd) {
            m_payee->Insert(payee.PAYEENAME, 0);
        }
        m_payee->ChangeValue(payeeName);
        m_payee->SetInsertionPointEnd();
        m_payee->Popup();
        m_payee->SetEvtHandlerEnabled(true);
    }
#else
void transactionsUpdateDialog::OnPayeeUpdated(wxCommandEvent& WXUNUSED(event))
{
#endif
    wxChildFocusEvent evt;
    onFocusChange(evt);
}

#if defined (__WXMAC__)
void transactionsUpdateDialog::OnAccountUpdated(wxCommandEvent& event)
{
    // Filtering the combobox as the user types because on Mac autocomplete function doesn't work
    // PLEASE DO NOT REMOVE!!
    wxString accountName = event.GetString();
    if (m_transferAcc->GetSelection() == -1) // make sure nothing is selected (ex. user presses down arrow)
    {
        m_transferAcc->SetEvtHandlerEnabled(false); // things will crash if events are handled during Clear
        m_transferAcc->Clear();
        
        Model_Account::Data_Set filtd = Model_Account::instance().FilterAccounts(accountName, true);
        std::sort(filtd.rbegin(), filtd.rend(), SorterByACCOUNTNAME());
        for (const auto &account : filtd)
            m_transferAcc->Insert(account.ACCOUNTNAME, 0);
  
        m_transferAcc->ChangeValue(accountName);
        m_transferAcc->SetInsertionPointEnd();
        m_transferAcc->Popup();
        m_transferAcc->SetEvtHandlerEnabled(true);
    }
#else
void transactionsUpdateDialog::OnAccountUpdated(wxCommandEvent& WXUNUSED(event))
{
#endif
    wxChildFocusEvent evt;
    onFocusChange(evt);
}
