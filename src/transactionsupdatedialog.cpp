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

// Used to determine if we need to refresh the tag text ctrl after
// accelerator hints are shown which only occurs once.
static bool altRefreshDone;

transactionsUpdateDialog::transactionsUpdateDialog(wxWindow* parent
    , std::vector<int>& transaction_id)
    : m_transaction_id(transaction_id)
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

    m_custom_fields = new mmCustomDataTransaction(this, 0, ID_CUSTOMFIELDS);

    this->SetFont(parent->GetFont());
    Create(parent);
}

bool transactionsUpdateDialog::Create(wxWindow* parent
    , wxWindowID id
    , const wxString& caption
    , const wxPoint& pos
    , const wxSize& size, long style)
{
    altRefreshDone = false; // reset the ALT refresh indicator on new dialog creation
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
    return true;
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
    grid_sizer->Add(m_dpc->mmGetLayout(false), wxSizerFlags(g_flagsH).Border(wxLEFT, 0));
    m_dpc->Enable(false);

    if (Option::instance().UseTransDateTime())
    {
        // Time --------------------------------------------
        m_time_checkbox = new wxCheckBox(this, wxID_ANY, _("Time"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
        m_time_ctrl = new wxTimePickerCtrl(this, wxID_ANY);
        grid_sizer->Add(m_time_checkbox, g_flagsH);
        grid_sizer->Add(m_time_ctrl, g_flagsH);
        m_time_ctrl->Enable(false);
    }

    // Status --------------------------------------------
    m_status_checkbox = new wxCheckBox(this, wxID_ANY, _("Status")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);

    m_status_choice = new wxChoice(this, wxID_ANY
        , wxDefaultPosition, wxDefaultSize);
    for (const auto& i : Model_Checking::STATUS_STR)
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
    for (const auto& i : Model_Checking::TYPE_STR)
    {
        if (!(m_hasSplits && (Model_Checking::TYPE_STR_TRANSFER == i)))
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

    cbPayee_ = new mmComboBoxPayee(this, mmID_PAYEE, wxDefaultSize, -1, true);
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

    cbCategory_ = new mmComboBoxCategory(this, mmID_CATEGORY, wxDefaultSize, -1, true);
    cbCategory_->Enable(false);

    grid_sizer->Add(m_categ_checkbox, g_flagsH);
    grid_sizer->Add(cbCategory_, g_flagsExpand);

    // Tags ------------------------------------------------------
    tag_checkbox_ = new wxCheckBox(this, wxID_ANY, _("Tags"));
    tag_append_checkbox_ = new wxCheckBox(this, wxID_ANY, _("Append"));
    tag_append_checkbox_->SetValue(true);
    tag_append_checkbox_->Enable(false);

    tagTextCtrl_ = new mmTagTextCtrl(this);
    tagTextCtrl_->Enable(false);
    grid_sizer->Add(tag_checkbox_, g_flagsH);
    grid_sizer->Add(tag_append_checkbox_, g_flagsH);
    grid_sizer->AddSpacer(0);
    grid_sizer->Add(tagTextCtrl_, g_flagsExpand);

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
        , ID_BTN_CUSTOMFIELDS, mmBitmapBundle(png::RIGHTARROW, mmBitmapButtonSize));
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
            status = Model_Checking::status_key(status_obj->GetData());
        else
            return;
    }

    wxString type = "";
    if (m_type_checkbox->IsChecked())
    {
        wxStringClientData* type_obj = static_cast<wxStringClientData*>(m_type_choice->GetClientObject(m_type_choice->GetSelection()));
        type = type_obj->GetData();
        if (Model_Checking::TYPE_STR_TRANSFER == type)
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
                payee->ACTIVE = 1;
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

    if (tag_checkbox_->IsChecked() && !tagTextCtrl_->IsValid())
        return  mmErrorDialogs::ToolTip4Object(tagTextCtrl_, _("Invalid value"), _("Tags"), wxICON_ERROR);

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

    const auto split = Model_Splittransaction::instance().get_all();

    std::vector<int> skip_trx;
    Model_Checking::instance().Savepoint();
    Model_Taglink::instance().Savepoint();
    for (const auto& id : m_transaction_id)
    {
        Model_Checking::Data *trx = Model_Checking::instance().get(id);
        bool is_locked = Model_Checking::is_locked(trx);

        if (is_locked) {
            skip_trx.push_back(trx->TRANSID);
            continue;
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

        if (m_date_checkbox->IsChecked() || (m_time_ctrl && m_time_checkbox->IsChecked()))
        {
            wxString date = trx->TRANSDATE;
            if (m_date_checkbox->IsChecked())
            {
                date.replace(0, 10, m_dpc->GetValue().FormatISODate());
                const Model_Account::Data* account = Model_Account::instance().get(trx->ACCOUNTID);
                const Model_Account::Data* to_account = Model_Account::instance().get(trx->TOACCOUNTID);
                if ((date < account->INITIALDATE) ||
                    (to_account && (date < to_account->INITIALDATE)))
                {
                    skip_trx.push_back(trx->TRANSID);
                    continue;
                }
            }

            if (m_time_ctrl && m_time_checkbox->IsChecked())
            {
                if (date.Length() > 10)
                    date.replace(11, 8, m_time_ctrl->GetValue().FormatISOTime());
                else
                    date.Append("T" + m_time_ctrl->GetValue().FormatISOTime());
            }

            trx->TRANSDATE = date;
        }

        if (m_color_checkbox->IsChecked()) {
            int color_id = bColours_->GetColorId();
            if (color_id < 0 || color_id > 7) {
                return mmErrorDialogs::ToolTip4Object(bColours_, _("Color"), _("Invalid value"), wxICON_ERROR);
            }
            trx->COLOR = color_id == 0 ? -1 : color_id ; 
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

        // Update tags
        if (tag_checkbox_->IsChecked()) {
            Model_Taglink::Data_Set taglinks;
            const wxString& refType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
            wxArrayInt tagIds = tagTextCtrl_->GetTagIDs();

            if (tag_append_checkbox_->IsChecked()) {
                // Since we are appending, start with the existing tags
                taglinks = Model_Taglink::instance().find(Model_Taglink::REFTYPE(refType), Model_Taglink::REFID(trx->TRANSID));
                // Remove existing tags from the new list to avoid duplicates
                for (const auto& link : taglinks)
                {
                    int index = tagIds.Index(link.TAGID);
                    if (index != wxNOT_FOUND)
                        tagIds.RemoveAt(index);
                }
            }
            // Create new taglinks for each tag ID
            for (const auto& tagId : tagIds)
            {
                Model_Taglink::Data* t = Model_Taglink::instance().create();
                t->REFTYPE = refType;
                t->REFID = trx->TRANSID;
                t->TAGID = tagId;
                taglinks.push_back(*t);
            }
            // Update the links for the transaction
            Model_Taglink::instance().update(taglinks, refType, trx->TRANSID);
        }

        if (m_amount_checkbox->IsChecked()) {
            trx->TRANSAMOUNT = amount;
        }

        if (m_categ_checkbox->IsChecked()) {
            trx->CATEGID = categ_id;
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
    Model_Taglink::instance().ReleaseSavepoint();
    Model_Checking::instance().ReleaseSavepoint();
    if (!skip_trx.empty())
    {
        const wxString detail = wxString::Format("%s\n%s: %zu\n%s: %zu"
                        , _("This is due to some elements of the transaction or account detail not allowing the update")
                        , _("Updated"), m_transaction_id.size() - skip_trx.size()
                        , _("Not updated"), skip_trx.size());
        mmErrorDialogs::MessageWarning(this
            , detail
            , _("Some transactions could not be updated"));
    }
    //TODO: be able to report detail on transactions that could not be updated

    EndModal(wxID_OK);
}

void transactionsUpdateDialog::SetPayeeTransferControls()
{
    wxStringClientData* trans_obj = static_cast<wxStringClientData*>(m_type_choice->GetClientObject(m_type_choice->GetSelection()));
    bool transfer = (Model_Checking::TYPE_STR_TRANSFER == trans_obj->GetData());

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
    tagTextCtrl_->Enable(tag_checkbox_->IsChecked());
    tag_append_checkbox_->Enable(tag_checkbox_->IsChecked());

    if (m_type_checkbox->IsChecked()) {
        SetPayeeTransferControls();
    } else {
        m_payee_checkbox->Enable(!m_hasTransfers);
        m_transferAcc_checkbox->Enable(!m_hasNonTransfers);
    }

    if (m_time_ctrl)
        m_time_ctrl->Enable(m_time_checkbox->IsChecked());

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
        button->SetBitmap(mmBitmapBundle(m_custom_fields->IsCustomPanelShown() ? png::RIGHTARROW : png::LEFTARROW, mmBitmapButtonSize));

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
                if (dlg.getRefreshRequested())
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
                mmCategDialog dlg(this, true, -1);
                dlg.ShowModal();
                if (dlg.getRefreshRequested())
                    cbCategory_->mmDoReInitialize();
                category = Model_Category::full_name(dlg.getCategId());
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

    // The first time the ALT key is pressed accelerator hints are drawn, but custom painting on the tags button
    // is not applied. We need to refresh the tag ctrl to redraw the drop button with the correct image.
    if (event.AltDown() && !altRefreshDone)
    {
        tagTextCtrl_->Refresh();
        altRefreshDone = true;
    }

    event.Skip();
}
