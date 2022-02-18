/*******************************************************
 Copyright (C) 2017 - 2022 Nikolay Akimov
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
#include "images_list.h"
#include "mmSimpleDialogs.h"
#include "mmTextCtrl.h"
#include "paths.h"
#include "transactionsupdatedialog.h"
#include "validators.h"
#include "Model_Account.h"
#include "Model_Checking.h"
#include "Model_Payee.h"
#include <wx/statline.h>

wxIMPLEMENT_DYNAMIC_CLASS(transactionsUpdateDialog, wxDialog);

wxBEGIN_EVENT_TABLE(transactionsUpdateDialog, wxDialog)
    EVT_BUTTON(wxID_OK, transactionsUpdateDialog::OnOk)
    EVT_BUTTON(wxID_VIEW_DETAILS, transactionsUpdateDialog::OnCategChange)
    EVT_BUTTON(ID_BTN_CUSTOMFIELDS, transactionsUpdateDialog::OnMoreFields)
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
    , m_hasTransfers(false)
    , m_hasNonTransfers(false)
    , m_hasSplits(false)
{
    m_currency = Model_Currency::GetBaseCurrency(); // base currency if we need it

    // Determine the mix of transaction that have been selected
    for (const auto& id : m_transaction_id)
    {
        Model_Checking::Data *trx = Model_Checking::instance().get(id);
        const bool isTransfer = Model_Checking::is_transfer(trx);
        
        if (!m_hasSplits)
        {
            Model_Splittransaction::Data_Set split = Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(id));
            if (!split.empty())
                m_hasSplits = true;
        }
  
        if (!m_hasTransfers && isTransfer)
            m_hasTransfers = true;

        if (!m_hasNonTransfers && !isTransfer)    
            m_hasNonTransfers = true;
    }

    m_custom_fields = new mmCustomDataTransaction(this, NULL, ID_CUSTOMFIELDS);

    Create(parent);
}

bool transactionsUpdateDialog::Create(wxWindow* parent
    , wxWindowID id
    , const wxString& caption
    , const wxPoint& pos
    , const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, wxGetTranslation(caption), pos, size, style);

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
    wxBoxSizer* box_sizer1 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* box_sizer2 = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* custom_fields_box_sizer = new wxBoxSizer(wxVERTICAL);
    box_sizer->Add(box_sizer1, g_flagsExpand);
    box_sizer1->Add(box_sizer2, g_flagsExpand);
    box_sizer1->Add(custom_fields_box_sizer, g_flagsExpand);

    wxStaticBox* static_box = new wxStaticBox(this, wxID_ANY, _("Specify"));
    wxStaticBoxSizer* box_sizer_left = new wxStaticBoxSizer(static_box, wxVERTICAL);
    wxFlexGridSizer* grid_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    grid_sizer->AddGrowableCol(1, 1);
    box_sizer_left->Add(grid_sizer, g_flagsV);
    box_sizer2->Add(box_sizer_left, g_flagsExpand);

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
    for (const auto& i : Model_Checking::all_type())
    {
        if (!(m_hasSplits && (Model_Checking::TRANSFER_STR == i)))
            m_type_choice->Append(wxGetTranslation(i), new wxStringClientData(i));
    }
    m_type_choice->Enable(false);
    m_type_choice->Select(0);


    grid_sizer->Add(m_type_checkbox, g_flagsH);
    grid_sizer->Add(m_type_choice, g_flagsH);

    // Amount Field --------------------------------------------
    m_amount_checkbox = new wxCheckBox(this, wxID_ANY, _("Amount")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_amount_checkbox->Enable(!m_hasSplits);

    m_amount_ctrl = new mmTextCtrl(this, wxID_ANY, ""
        , wxDefaultPosition, wxDefaultSize
        , wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    m_amount_ctrl->Enable(false);

    grid_sizer->Add(m_amount_checkbox, g_flagsH);
    grid_sizer->Add(m_amount_ctrl, g_flagsH);

    // Payee --------------------------------------------
    m_payee_checkbox = new wxCheckBox(this, wxID_ANY, _("Payee")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_payee_checkbox->Enable(!m_hasTransfers);

    m_payee = new wxComboBox(this, ID_PAYEE);
    m_payee->SetMaxSize(wxSize(m_amount_ctrl->GetSize().GetX() * 2, -1));

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
    m_transferAcc_checkbox->Enable(!m_hasNonTransfers);

    m_transferAcc = new wxComboBox(this, ID_TRANS_ACC);
    m_transferAcc->SetMaxSize(m_payee->GetSize());
    wxArrayString account_names = Model_Account::instance().all_checking_account_names(true);
    m_transferAcc->Insert(account_names, 0);
    m_transferAcc->AutoComplete(account_names);
    m_transferAcc->Enable(false);

    grid_sizer->Add(m_transferAcc_checkbox, g_flagsH);
    grid_sizer->Add(m_transferAcc, g_flagsExpand);

    // Category -------------------------------------------------
    m_categ_checkbox = new wxCheckBox(this, wxID_VIEW_DETAILS, _("Category")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_categ_checkbox->Enable(!m_hasSplits);

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
        , wxDefaultPosition, wxSize(-1, m_categ_btn->GetSize().GetHeight() * 5), wxTE_MULTILINE);
    m_notes_ctrl->Enable(false);
    m_append_checkbox->Enable(false);

    grid_sizer->Add(m_notes_checkbox, g_flagsH);
    grid_sizer->Add(m_append_checkbox, g_flagsH);
    box_sizer_left->Add(m_notes_ctrl, wxSizerFlags(g_flagsExpand).Border(wxLEFT | wxRIGHT | wxBOTTOM, 10));

    /*************************************************************
     Button Panel with OK/Cancel buttons
    *************************************************************/
    wxPanel* button_panel = new wxPanel(this, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    box_sizer_left->Add(button_panel, wxSizerFlags(g_flagsV).Center());

    wxStdDialogButtonSizer* button_sizer = new wxStdDialogButtonSizer;
    button_panel->SetSizer(button_sizer);

    wxButton* button_ok = new wxButton(button_panel, wxID_OK, _("&OK "));
    wxButton* button_cancel = new wxButton(button_panel
        , wxID_CANCEL, wxGetTranslation(g_CancelLabel));
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
    button_sizer->Realize();

    // Custom fields -----------------------------------

    m_custom_fields->FillCustomFields(custom_fields_box_sizer);
    if (m_custom_fields->GetActiveCustomFieldsCount() > 0) {
        wxCommandEvent evt(wxEVT_BUTTON, ID_BTN_CUSTOMFIELDS);
        this->GetEventHandler()->AddPendingEvent(evt);
    }

    Center();
    this->SetSizer(box_sizer);
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
        transfer = (Model_Checking::TRANSFER_STR == type);
        if (transfer)
        {
            if  (m_hasNonTransfers && !m_transferAcc_checkbox->IsChecked())
                return mmErrorDialogs::InvalidAccount(m_transferAcc_checkbox, true);
        } else {
            if (m_hasTransfers && !m_payee_checkbox->IsChecked())
                return mmErrorDialogs::InvalidPayee(m_payee_checkbox);
        }
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

        if (is_locked)
        {
            skip_trx.push_back(trx->TRANSID);
            continue;
        }

        if (m_date_checkbox->IsChecked())
        {
            trx->TRANSDATE = m_dpc->GetValue().FormatISODate();
        }

        if (m_status_checkbox->IsChecked())
        {
            trx->STATUS = status;
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
                    , wxString::Format(_("You have not used this payee name before. Is the name correct?\n%s"), payee_name)
                    , _("Confirm payee name")
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

            trx->PAYEEID = payee->PAYEEID;
            trx->TOACCOUNTID = -1;
        }

        if (m_transferAcc_checkbox->IsChecked())
        {
            wxString account_name = m_transferAcc->GetValue();
            if (account_name.IsEmpty())
                return mmErrorDialogs::InvalidAccount(m_transferAcc, true);
            
            // Get account string from populated list to address issues with case compare differences between autocomplete and account list
            int account_loc = m_transferAcc->FindString(account_name);
            if (account_loc != wxNOT_FOUND)
                account_name = m_transferAcc->GetString(account_loc);
            
            Model_Account::Data* account = Model_Account::instance().get(account_name);
            trx->TOACCOUNTID = account->ACCOUNTID;
            trx->PAYEEID = -1;
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
            trx->TRANSAMOUNT = amount;
        }

        if (m_categ_checkbox->IsChecked())
        {
            trx->CATEGID = m_categ_id;
            trx->SUBCATEGID = m_subcateg_id;
        }

        if (m_type_checkbox->IsChecked())
        {
            trx->TRANSCODE = type;
        }
 
        // Need to consider TOTRANSAMOUNT if material transaction change
        if (m_amount_checkbox->IsChecked() || m_type_checkbox->IsChecked() || m_transferAcc_checkbox->IsChecked())
        {
            if (!Model_Checking::is_transfer(trx))
            {
                trx->TOTRANSAMOUNT = trx->TRANSAMOUNT;
            } else
            {
                const auto acc = Model_Account::instance().get(trx->ACCOUNTID);
                const auto curr = Model_Currency::instance().get(acc->CURRENCYID);
                const auto to_acc = Model_Account::instance().get(trx->TOACCOUNTID);
                const auto to_curr = Model_Currency::instance().get(to_acc->CURRENCYID);
                if (curr == to_curr)
                {
                    trx->TOTRANSAMOUNT = trx->TRANSAMOUNT;                
                } else
                {
                    double exch = 1;
                    const double convRateTo = Model_CurrencyHistory::getDayRate(to_curr->CURRENCYID, trx->TRANSDATE);
                    if (convRateTo > 0)
                    {
                        const double convRate = Model_CurrencyHistory::getDayRate(curr->CURRENCYID, trx->TRANSDATE);
                        exch = convRate / convRateTo;
                    }
                    trx->TOTRANSAMOUNT = trx->TRANSAMOUNT * exch;
                }
            }
        }

        m_custom_fields->UpdateCustomValues(id);

        Model_Checking::instance().save(trx);
    }
    Model_Checking::instance().ReleaseSavepoint();

    skip_trx; //TODO: resume

    EndModal(wxID_OK);
}

void transactionsUpdateDialog::SetPayeeTransferControls()
{
    wxStringClientData* trans_obj = static_cast<wxStringClientData*>(m_type_choice->GetClientObject(m_type_choice->GetSelection()));
    bool transfer = (Model_Checking::TRANSFER_STR == trans_obj->GetData());

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
        m_payee_checkbox->Enable(!m_hasTransfers);
        m_transferAcc_checkbox->Enable(!m_hasNonTransfers);
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

void transactionsUpdateDialog::OnMoreFields(wxCommandEvent& WXUNUSED(event))
{
    wxBitmapButton* button = static_cast<wxBitmapButton*>(FindWindow(ID_BTN_CUSTOMFIELDS));

    if (button)
        button->SetBitmap(mmBitmap(m_custom_fields->IsCustomPanelShown() ? png::RIGHTARROW : png::LEFTARROW, mmBitmapButtonSize));

    m_custom_fields->ShowHideCustomPanel();

    this->SetMinSize(wxSize(0, 0));
    this->Fit();
}
