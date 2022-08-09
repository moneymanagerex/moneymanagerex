/*******************************************************
 Copyright (C) 2017 - 2022 Nikolay Akimov
 Copyright (C) 2021-2022 Mark Whalley (mark@ipx.co.uk)

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
#include "payeedialog.h"
#include "transactionsupdatedialog.h"
#include "validators.h"
#include "Model_Account.h"
#include "Model_Checking.h"
#include "Model_Payee.h"
#include <wx/statline.h>

wxIMPLEMENT_DYNAMIC_CLASS(transactionsUpdateDialog, wxDialog);

wxBEGIN_EVENT_TABLE(transactionsUpdateDialog, wxDialog)
    EVT_BUTTON(wxID_OK, transactionsUpdateDialog::OnOk)
    EVT_BUTTON(ID_BTN_CUSTOMFIELDS, transactionsUpdateDialog::OnMoreFields)
    EVT_CHECKBOX(wxID_ANY, transactionsUpdateDialog::OnCheckboxClick)
    EVT_CHILD_FOCUS(transactionsUpdateDialog::onFocusChange)
    EVT_CHAR_HOOK(transactionsUpdateDialog::OnComboKey)
    EVT_CHOICE(ID_TRANS_TYPE, transactionsUpdateDialog::OnTransTypeChanged)
wxEND_EVENT_TABLE()

transactionsUpdateDialog::transactionsUpdateDialog()
{
}

transactionsUpdateDialog::~transactionsUpdateDialog()
{
}

transactionsUpdateDialog::transactionsUpdateDialog(wxWindow* parent
    , std::vector<int>& transaction_id)
    : m_payee_checkbox(nullptr)
    , cbPayee_(nullptr)
    , m_date_checkbox(nullptr)
    , m_dpc(nullptr)
    , m_status_checkbox(nullptr)
    , m_status_choice(nullptr)
    , m_categ_checkbox(nullptr)
    , cbCategory_(nullptr)
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

    this->SetFont(parent->GetFont());
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
    m_dpc = new mmDatePickerCtrl(this, wxID_ANY);
    grid_sizer->Add(m_date_checkbox, g_flagsH);
    grid_sizer->Add(m_dpc->mmGetLayout(), g_flagsH);
    m_dpc->Enable(false);

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
    grid_sizer->Add(m_amount_ctrl, g_flagsExpand);

    // Payee --------------------------------------------
    m_payee_checkbox = new wxCheckBox(this, wxID_ANY, _("Payee")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_payee_checkbox->Enable(!m_hasTransfers);

    cbPayee_ = new mmComboBoxPayee(this, mmID_PAYEE);
    cbPayee_->Enable(false);
    cbPayee_->SetMinSize(wxSize(200, -1));

    grid_sizer->Add(m_payee_checkbox, g_flagsH);
    grid_sizer->Add(cbPayee_, g_flagsExpand);

    // Transfer to account --------------------------------------------
    m_transferAcc_checkbox = new wxCheckBox(this, wxID_ANY, _("Transfer To")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_transferAcc_checkbox->Enable(!m_hasNonTransfers);

    cbAccount_ = new mmComboBoxAccount(this, ID_TRANS_ACC);
    cbAccount_->SetMaxSize(cbPayee_->GetSize());
    cbAccount_->Enable(false);

    grid_sizer->Add(m_transferAcc_checkbox, g_flagsH);
    grid_sizer->Add(cbAccount_, g_flagsExpand);

    // Category -------------------------------------------------
    m_categ_checkbox = new wxCheckBox(this, wxID_VIEW_DETAILS, _("Category")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_categ_checkbox->Enable(!m_hasSplits);

    cbCategory_ = new mmComboBoxCategory(this, mmID_CATEGORY);
    cbCategory_->Enable(false);

    grid_sizer->Add(m_categ_checkbox, g_flagsH);
    grid_sizer->Add(cbCategory_, g_flagsExpand);

    // Colours --------------------------------------------
    m_color_checkbox = new wxCheckBox(this, wxID_VIEW_DETAILS, _("Color")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    bColours_ = new mmColorButton(this, wxID_HIGHEST, cbCategory_->GetSize());
    mmToolTip(bColours_, _("User Colors"));
    grid_sizer->Add(m_color_checkbox, g_flagsH);
    grid_sizer->Add(bColours_, g_flagsExpand);
    bColours_->Enable(false);

    // Notes --------------------------------------------
    m_notes_checkbox = new wxCheckBox(this, wxID_ANY, _("Notes")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_append_checkbox = new wxCheckBox(this, wxID_ANY, _("Append")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_append_checkbox->SetValue(true);

    m_notes_ctrl = new wxTextCtrl(this, wxID_ANY, ""
        , wxDefaultPosition, wxSize(-1, cbCategory_->GetSize().GetHeight() * 5), wxTE_MULTILINE);
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
    if (m_type_checkbox->IsChecked())
    {
        wxStringClientData* type_obj = static_cast<wxStringClientData*>(m_type_choice->GetClientObject(m_type_choice->GetSelection()));
        type = type_obj->GetData();
        if (Model_Checking::TRANSFER_STR == type)
        {
            if  (m_hasNonTransfers && !m_transferAcc_checkbox->IsChecked())
                return mmErrorDialogs::InvalidAccount(m_transferAcc_checkbox, true);
        } else {
            if (m_hasTransfers && !m_payee_checkbox->IsChecked())
                return mmErrorDialogs::InvalidPayee(m_payee_checkbox);
        }
    }

    int payee_id = -1;
    if (m_payee_checkbox->IsChecked())
    {
        wxString payee_name = cbPayee_->GetValue();
        if (payee_name.IsEmpty())
            return mmErrorDialogs::InvalidPayee(cbPayee_);

        if (!cbPayee_->mmIsValid()) {
            wxMessageDialog msgDlg(this
                , wxString::Format(_("You have not used this payee name before. Is the name correct?\n%s"), cbPayee_->GetValue())
                , _("Confirm payee name")
                , wxYES_NO | wxYES_DEFAULT | wxICON_WARNING);
            if (msgDlg.ShowModal() == wxID_YES)
            {
                Model_Payee::Data* payee = Model_Payee::instance().create();
                payee->PAYEENAME = cbPayee_->GetValue();
                Model_Payee::instance().save(payee);
                mmWebApp::MMEX_WebApp_UpdatePayee();
                payee_id = payee->PAYEEID;
            }
            else
                return;
        }
        else
            payee_id = cbPayee_->mmGetId();
    }

    if (m_transferAcc_checkbox->IsChecked())
    {
        if (!cbAccount_->mmIsValid())
            return mmErrorDialogs::InvalidAccount(cbAccount_);
    }

    if (m_categ_checkbox->IsChecked())
    {
        if (!cbCategory_->mmIsValid())
            return mmErrorDialogs::InvalidCategory(cbCategory_);
    }
    int categ_id = cbCategory_->mmGetCategoryId();
    int m_subcateg_id = cbCategory_->mmGetSubcategoryId();

    const auto split = Model_Splittransaction::instance().get_all();

    std::vector<int> skip_trx;
    Model_Checking::instance().Savepoint();
    for (const auto& id : m_transaction_id)
    {
        Model_Checking::Data *trx = Model_Checking::instance().get(id);
        bool is_locked = Model_Checking::is_locked(trx);

        if (is_locked) {
            skip_trx.push_back(trx->TRANSID);
            continue;
        }

        if (m_date_checkbox->IsChecked()) {
            trx->TRANSDATE = m_dpc->GetValue().FormatISODate();
        }

        if (m_status_checkbox->IsChecked()) {
            trx->STATUS = status;
        }

        if (m_payee_checkbox->IsChecked()) {
            trx->PAYEEID = payee_id;
            trx->TOACCOUNTID = -1;
        }

        if (m_transferAcc_checkbox->IsChecked()) {
            trx->TOACCOUNTID = cbAccount_->mmGetId();
            trx->PAYEEID = -1;
        }

        if (m_color_checkbox->IsChecked()) {
            int color_id = bColours_->GetColorId();
            if (color_id < 0 || color_id > 7) {
                return mmErrorDialogs::ToolTip4Object(bColours_, _("Color"), _("Invalid value"), wxICON_ERROR);
            }
            trx->FOLLOWUPID = color_id == 0 ? -1 : color_id ;
        }

        if (m_notes_checkbox->IsChecked()) {
            if (m_append_checkbox->IsChecked()) {
                trx->NOTES += (trx->NOTES.Right(1) == "\n" || trx->NOTES.empty()
                    ? "" : "\n")
                    + m_notes_ctrl->GetValue();
            }
            else {
                trx->NOTES = m_notes_ctrl->GetValue();
            }
        }

        if (m_amount_checkbox->IsChecked()) {
            trx->TRANSAMOUNT = amount;
        }

        if (m_categ_checkbox->IsChecked()) {
            trx->CATEGID = categ_id;
            trx->SUBCATEGID = m_subcateg_id;
        }

        if (m_type_checkbox->IsChecked()) {
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
        cbPayee_->Enable(false);
    } else
    {
        m_transferAcc_checkbox->SetValue(false);
        cbAccount_->Enable(false);        
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
    cbPayee_->Enable(m_payee_checkbox->IsChecked());
    cbAccount_->Enable(m_transferAcc_checkbox->IsChecked());
    cbCategory_->Enable(m_categ_checkbox->IsChecked());
    m_amount_ctrl->Enable(m_amount_checkbox->IsChecked());
    bColours_->Enable(m_color_checkbox->IsChecked());
    m_notes_ctrl->Enable(m_notes_checkbox->IsChecked());
    m_append_checkbox->Enable(m_notes_checkbox->IsChecked());

    if (m_type_checkbox->IsChecked()) {
        SetPayeeTransferControls();
    } else {
        m_payee_checkbox->Enable(!m_hasTransfers);
        m_transferAcc_checkbox->Enable(!m_hasNonTransfers);
    }

    event.Skip();
}

void transactionsUpdateDialog::onFocusChange(wxChildFocusEvent& event)
{
    cbCategory_->SetValue(cbCategory_->GetValue());


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

void transactionsUpdateDialog::OnMoreFields(wxCommandEvent& WXUNUSED(event))
{
    wxBitmapButton* button = static_cast<wxBitmapButton*>(FindWindow(ID_BTN_CUSTOMFIELDS));

    if (button)
        button->SetBitmap(mmBitmap(m_custom_fields->IsCustomPanelShown() ? png::RIGHTARROW : png::LEFTARROW, mmBitmapButtonSize));

    m_custom_fields->ShowHideCustomPanel();

    this->SetMinSize(wxSize(0, 0));
    this->Fit();
}

void transactionsUpdateDialog::OnComboKey(wxKeyEvent& event)
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
            auto category = cbCategory_->GetValue();
            if (category.empty())
            {
                mmCategDialog dlg(this, true, -1, -1);
                dlg.ShowModal();
                cbCategory_->mmDoReInitialize();
                category = Model_Category::full_name(dlg.getCategId(), dlg.getSubCategId());
                cbCategory_->ChangeValue(category);
                cbCategory_->SelectAll();
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
