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

#include "base/defs.h"
#include <wx/statline.h>

#include "base/constants.h"
#include "base/paths.h"
#include "base/images_list.h"
#include "util/_simple.h"
#include "util/mmTextCtrl.h"
#include "util/mmCalcValidator.h"

#include "model/AccountModel.h"
#include "model/CurrencyHistoryModel.h"
#include "model/CurrencyModel.h"
#include "model/PayeeModel.h"
#include "model/TrxModel.h"

#include "manager/CategoryManager.h"
#include "manager/PayeeManager.h"
#include "TrxUpdateDialog.h"

wxIMPLEMENT_DYNAMIC_CLASS(TrxUpdateDialog, wxDialog);

wxBEGIN_EVENT_TABLE(TrxUpdateDialog, wxDialog)
    EVT_BUTTON(wxID_OK,             TrxUpdateDialog::OnOk)
    EVT_BUTTON(ID_BTN_CUSTOMFIELDS, TrxUpdateDialog::OnMoreFields)
    EVT_CHECKBOX(wxID_ANY,          TrxUpdateDialog::OnCheckboxClick)
    EVT_CHILD_FOCUS(                TrxUpdateDialog::onFocusChange)
    EVT_CHAR_HOOK(                  TrxUpdateDialog::OnComboKey)
    EVT_CHOICE(ID_TRANS_TYPE,       TrxUpdateDialog::OnTransTypeChanged)
wxEND_EVENT_TABLE()

TrxUpdateDialog::TrxUpdateDialog()
{
}

TrxUpdateDialog::~TrxUpdateDialog()
{
}

// Used to determine if we need to refresh the tag text ctrl after
// accelerator hints are shown which only occurs once.
static bool altRefreshDone;

TrxUpdateDialog::TrxUpdateDialog(
    wxWindow* parent,
    std::vector<int64>& trx_id_a
) :
    m_trx_id_a(trx_id_a)
{
    m_currency_n = CurrencyModel::instance().get_base_data_n(); // base currency if we need it

    // Determine the mix of transaction that have been selected
    for (const auto& trx_id : m_trx_id_a) {
        const TrxData* trx_n = TrxModel::instance().get_id_data_n(trx_id);
        const bool isTransfer = trx_n->is_transfer();

        if (!m_hasSplits) {
            TrxSplitModel::DataA tp_a = TrxSplitModel::instance().find(
                TrxSplitCol::TRANSID(trx_id)
            );
            if (!tp_a.empty())
                m_hasSplits = true;
        }

        if (!m_hasTransfers && isTransfer)
            m_hasTransfers = true;

        if (!m_hasNonTransfers && !isTransfer)
            m_hasNonTransfers = true;
    }

    m_custom_fields = new mmCustomDataTransaction(this, TrxModel::s_ref_type, 0, ID_CUSTOMFIELDS);

    this->SetFont(parent->GetFont());
    Create(parent);
}

bool TrxUpdateDialog::Create(
    wxWindow* parent,
    wxWindowID id,
    const wxString& caption,
    const wxPoint& pos,
    const wxSize& size, long style
) {
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

void TrxUpdateDialog::CreateControls()
{
    wxBoxSizer* box_sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* box_sizer1 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* box_sizer2 = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* custom_fields_box_sizer = new wxBoxSizer(wxVERTICAL);
    box_sizer->Add(box_sizer1, g_flagsExpand);
    box_sizer1->Add(box_sizer2, g_flagsExpand);
    box_sizer1->Add(custom_fields_box_sizer, g_flagsExpand);

    wxStaticBox* static_box = new wxStaticBox(this, wxID_ANY, _t("Specify"));
    wxStaticBoxSizer* box_sizer_left = new wxStaticBoxSizer(static_box, wxVERTICAL);
    wxFlexGridSizer* grid_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    grid_sizer->AddGrowableCol(1, 1);
    box_sizer_left->Add(grid_sizer, g_flagsV);
    box_sizer2->Add(box_sizer_left, g_flagsExpand);

    // Date --------------------------------------------
    m_date_checkbox = new wxCheckBox(this, wxID_ANY, _t("Date")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_dpc = new mmDatePickerCtrl(this, wxID_ANY);
    grid_sizer->Add(m_date_checkbox, g_flagsH);
    grid_sizer->Add(m_dpc->mmGetLayout(false), wxSizerFlags(g_flagsH).Border(wxLEFT, 0));
    m_dpc->Enable(false);

    if (PrefModel::instance().UseTransDateTime())
    {
        // Time --------------------------------------------
        m_time_checkbox = new wxCheckBox(this, wxID_ANY, _t("Time"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
        m_time_ctrl = new wxTimePickerCtrl(this, wxID_ANY);
        grid_sizer->Add(m_time_checkbox, g_flagsH);
        grid_sizer->Add(m_time_ctrl, g_flagsH);
        m_time_ctrl->Enable(false);
    }

    // Status --------------------------------------------
    m_status_checkbox = new wxCheckBox(this, wxID_ANY, _t("Status")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);

    m_status_choice = new wxChoice(this, wxID_ANY
        , wxDefaultPosition, wxDefaultSize);
    for (int i = 0; i < TrxStatus::size; ++i) {
        wxString status = TrxStatus(i).name();
        m_status_choice->Append(wxGetTranslation(status), new wxStringClientData(status));
    }

    m_status_choice->Enable(false);
    m_status_choice->Select(0);

    grid_sizer->Add(m_status_checkbox, g_flagsH);
    grid_sizer->Add(m_status_choice, g_flagsExpand);

    // Type --------------------------------------------
    m_type_checkbox = new wxCheckBox(this, wxID_ANY, _t("Type")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);

    m_type_choice = new wxChoice(this, ID_TRANS_TYPE
        , wxDefaultPosition, wxDefaultSize);
    for (int i = 0; i < TrxType::size; ++i) {
        if (!(m_hasSplits && i == TrxType::e_transfer)) {
            wxString type = TrxType(i).name();
            m_type_choice->Append(wxGetTranslation(type), new wxStringClientData(type));
        }
    }
    m_type_choice->Enable(false);
    m_type_choice->Select(0);


    grid_sizer->Add(m_type_checkbox, g_flagsH);
    grid_sizer->Add(m_type_choice, g_flagsExpand);

    // Amount Field --------------------------------------------
    m_amount_checkbox = new wxCheckBox(this, wxID_ANY, _t("Amount")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_amount_checkbox->Enable(!m_hasSplits);

    m_amount_ctrl = new mmTextCtrl(this, wxID_ANY, ""
        , wxDefaultPosition, wxDefaultSize
        , wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    m_amount_ctrl->Enable(false);

    grid_sizer->Add(m_amount_checkbox, g_flagsH);
    grid_sizer->Add(m_amount_ctrl, g_flagsExpand);

    // Payee --------------------------------------------
    m_payee_checkbox = new wxCheckBox(this, wxID_ANY, _t("Payee")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_payee_checkbox->Enable(!m_hasTransfers);

    cbPayee_ = new mmComboBoxPayee(this, mmID_PAYEE, wxDefaultSize, -1, true);
    cbPayee_->Enable(false);
    cbPayee_->SetMinSize(wxSize(200, -1));

    grid_sizer->Add(m_payee_checkbox, g_flagsH);
    grid_sizer->Add(cbPayee_, g_flagsExpand);

    // Transfer to account --------------------------------------------
    m_transferAcc_checkbox = new wxCheckBox(this, wxID_ANY, _t("Transfer To")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_transferAcc_checkbox->Enable(!m_hasNonTransfers);

    cbAccount_ = new mmComboBoxAccount(this, ID_TRANS_ACC);
    cbAccount_->SetMaxSize(cbPayee_->GetSize());
    cbAccount_->Enable(false);

    grid_sizer->Add(m_transferAcc_checkbox, g_flagsH);
    grid_sizer->Add(cbAccount_, g_flagsExpand);

    // Category -------------------------------------------------
    m_categ_checkbox = new wxCheckBox(this, wxID_VIEW_DETAILS, _t("Category")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_categ_checkbox->Enable(!m_hasSplits);

    cbCategory_ = new mmComboBoxCategory(this, mmID_CATEGORY, wxDefaultSize, -1, true);
    cbCategory_->Enable(false);

    grid_sizer->Add(m_categ_checkbox, g_flagsH);
    grid_sizer->Add(cbCategory_, g_flagsExpand);

    // Tags ------------------------------------------------------
    tag_checkbox_ = new wxCheckBox(this, wxID_ANY, _t("Tags"));
    tag_append_checkbox_ = new wxCheckBox(this, wxID_ANY, _t("Append"));
    tag_append_checkbox_->SetValue(true);
    tag_append_checkbox_->Enable(false);

    tagTextCtrl_ = new mmTagTextCtrl(this);
    tagTextCtrl_->Enable(false);
    grid_sizer->Add(tag_checkbox_, g_flagsH);
    grid_sizer->Add(tag_append_checkbox_, g_flagsH);
    grid_sizer->AddSpacer(0);
    grid_sizer->Add(tagTextCtrl_, g_flagsExpand);

    // Colours --------------------------------------------
    m_color_checkbox = new wxCheckBox(this, wxID_VIEW_DETAILS, _t("Color")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    bColours_ = new mmColorButton(this, wxID_HIGHEST, cbCategory_->GetSize());
    mmToolTip(bColours_, _t("User Colors"));
    grid_sizer->Add(m_color_checkbox, g_flagsH);
    grid_sizer->Add(bColours_, g_flagsExpand);
    bColours_->Enable(false);

    // Notes --------------------------------------------
    m_notes_checkbox = new wxCheckBox(this, wxID_ANY, _t("Notes")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_append_checkbox = new wxCheckBox(this, wxID_ANY, _t("Append")
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

    wxButton* button_ok = new wxButton(button_panel, wxID_OK, _t("&OK "));
    wxButton* button_cancel = new wxButton(button_panel
        , wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    button_cancel->SetFocus();

    wxBitmapButton* button_hide = new wxBitmapButton(button_panel
        , ID_BTN_CUSTOMFIELDS, mmBitmapBundle(png::RIGHTARROW, mmBitmapButtonSize));
    mmToolTip(button_hide, _t("Show/Hide custom fields window"));
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

void TrxUpdateDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    double amount = 0;
    if (m_amount_checkbox->IsChecked() && !m_amount_ctrl->checkValue(amount))
        return;

    wxString status = "";
    if (m_status_checkbox->IsChecked()) {
        wxStringClientData* status_obj = static_cast<wxStringClientData*>(
            m_status_choice->GetClientObject(m_status_choice->GetSelection())
        );
        if (status_obj)
            status = TrxStatus(status_obj->GetData()).key();
        else
            return;
    }

    TrxType trx_type = TrxType();
    if (m_type_checkbox->IsChecked()) {
        wxStringClientData* type_obj = static_cast<wxStringClientData*>(
            m_type_choice->GetClientObject(m_type_choice->GetSelection())
        );
        trx_type = TrxType(type_obj->GetData());
        if (trx_type.id() == TrxType::e_transfer) {
            if  (m_hasNonTransfers && !m_transferAcc_checkbox->IsChecked())
                return mmErrorDialogs::InvalidAccount(m_transferAcc_checkbox, true);
        } else {
            if (m_hasTransfers && !m_payee_checkbox->IsChecked())
                return mmErrorDialogs::InvalidPayee(m_payee_checkbox);
        }
    }

    int64 payee_id = -1;
    if (m_payee_checkbox->IsChecked()) {
        wxString payee_name = cbPayee_->GetValue();
        if (payee_name.IsEmpty())
            return mmErrorDialogs::InvalidPayee(cbPayee_);

        if (!cbPayee_->mmIsValid()) {
            wxMessageDialog msgDlg(this
                , wxString::Format(_t("Payee name has not been used before. Is the name correct?\n%s"), cbPayee_->GetValue())
                , _t("Confirm payee name")
                , wxYES_NO | wxYES_DEFAULT | wxICON_WARNING);
            if (msgDlg.ShowModal() == wxID_YES) {
                PayeeData new_payee_d = PayeeData();
                new_payee_d.m_name = cbPayee_->GetValue();
                PayeeModel::instance().add_data_n(new_payee_d);
                mmWebApp::uploadPayee();
                payee_id = new_payee_d.m_id;
            }
            else
                return;
        }
        else
            payee_id = cbPayee_->mmGetId();
    }

    if (tag_checkbox_->IsChecked() && !tagTextCtrl_->IsValid())
        return mmErrorDialogs::ToolTip4Object(tagTextCtrl_,
            _t("Invalid value"),
            _t("Tags"),
            wxICON_ERROR
        );

    if (m_transferAcc_checkbox->IsChecked()) {
        if (!cbAccount_->mmIsValid())
            return mmErrorDialogs::InvalidAccount(cbAccount_);
    }

    if (m_categ_checkbox->IsChecked()) {
        if (!cbCategory_->mmIsValid())
            return mmErrorDialogs::InvalidCategory(cbCategory_);
    }
    int64 categ_id = cbCategory_->mmGetCategoryId();

    // const auto split = TrxSplitModel::instance().find_all_mTrxId();

    std::vector<int64> skip_trx;
    TrxModel::instance().db_savepoint();
    TagLinkModel::instance().db_savepoint();
    for (const auto& trx_id : m_trx_id_a) {
        TrxData* trx_n = TrxModel::instance().unsafe_get_id_data_n(trx_id);
        bool is_locked = TrxModel::instance().is_locked(*trx_n);

        if (is_locked) {
            skip_trx.push_back(trx_n->m_id);
            continue;
        }

        if (m_status_checkbox->IsChecked()) {
            trx_n->m_status = TrxStatus(status);
        }

        if (m_payee_checkbox->IsChecked()) {
            trx_n->m_payee_id_n = payee_id;
            trx_n->m_to_account_id_n = -1;
        }

        if (m_transferAcc_checkbox->IsChecked()) {
            trx_n->m_to_account_id_n = cbAccount_->mmGetId();
            trx_n->m_payee_id_n = -1;
        }

        if (m_date_checkbox->IsChecked() || (m_time_ctrl && m_time_checkbox->IsChecked())) {
            wxString date = trx_n->m_date_time.isoDateTime();
            if (m_date_checkbox->IsChecked()) {
                date.replace(0, 10, m_dpc->GetValue().FormatISODate());
                const AccountData* account = AccountModel::instance().get_id_data_n(
                    trx_n->m_account_id
                );
                const AccountData* to_account = AccountModel::instance().get_id_data_n(
                    trx_n->m_to_account_id_n
                );
                if ((mmDate(date) < account->m_open_date) ||
                    (to_account && (mmDate(date) < to_account->m_open_date))
                ) {
                    skip_trx.push_back(trx_n->m_id);
                    continue;
                }
            }

            if (m_time_ctrl && m_time_checkbox->IsChecked()) {
                if (date.Length() > 10)
                    date.replace(11, 8, m_time_ctrl->GetValue().FormatISOTime());
                else
                    date.Append("T" + m_time_ctrl->GetValue().FormatISOTime());
            }

            trx_n->m_date_time = mmDateTime(date);
        }

        if (m_color_checkbox->IsChecked()) {
            int color_id = bColours_->GetColorId();
            if (color_id < 0 || color_id > 7) {
                return mmErrorDialogs::ToolTip4Object(bColours_,
                    _t("Color"), _t("Invalid value"), wxICON_ERROR
                );
            }
            trx_n->m_color = color_id == 0 ? -1 : color_id ; 
        }

        if (m_notes_checkbox->IsChecked()) {
            if (m_append_checkbox->IsChecked()) {
                trx_n->m_notes += (trx_n->m_notes.Right(1) == "\n" || trx_n->m_notes.empty()
                    ? "" : "\n")
                    + m_notes_ctrl->GetValue();
            }
            else {
                trx_n->m_notes = m_notes_ctrl->GetValue();
            }
        }

        // Update tags
        if (tag_checkbox_->IsChecked()) {
            TagLinkModel::DataA gl_a;
            wxArrayInt64 tag_id_a = tagTextCtrl_->GetTagIDs();

            if (tag_append_checkbox_->IsChecked()) {
                // Since we are appending, start with the existing tags
                gl_a = TagLinkModel::instance().find(
                    TagLinkCol::REFTYPE(TrxModel::s_ref_type.name_n()),
                    TagLinkCol::REFID(trx_n->m_id)
                );
                // Remove existing tags from the new list to avoid duplicates
                for (const auto& gl_d : gl_a) {
                    auto index = std::find(tag_id_a.begin(), tag_id_a.end(), gl_d.m_tag_id);
                    if (index != tag_id_a.end())
                        tag_id_a.erase(index);
                }
            }
            // Create new taglinks for each tag ID
            for (const auto& tag_id : tag_id_a) {
                TagLinkData new_gl_d = TagLinkData();
                new_gl_d.m_tag_id   = tag_id;
                new_gl_d.m_ref_type = TrxModel::s_ref_type;
                new_gl_d.m_ref_id   = trx_n->m_id;
                gl_a.push_back(new_gl_d);
            }
            // Update the links for the transaction
            TagLinkModel::instance().update(
                TrxModel::s_ref_type, trx_n->m_id,
                gl_a
            );
        }

        if (m_amount_checkbox->IsChecked()) {
            trx_n->m_amount = amount;
        }

        if (m_categ_checkbox->IsChecked()) {
            trx_n->m_category_id_n = categ_id;
        }

        if (m_type_checkbox->IsChecked()) {
            trx_n->m_type = trx_type;
        }

        // Need to consider m_to_amount if material transaction change
        if (m_amount_checkbox->IsChecked() || m_type_checkbox->IsChecked() ||
            m_transferAcc_checkbox->IsChecked()
        ) {
            if (!trx_n->is_transfer()) {
                trx_n->m_to_amount = trx_n->m_amount;
            }
            else {
                const auto acc = AccountModel::instance().get_id_data_n(trx_n->m_account_id);
                const auto curr = CurrencyModel::instance().get_id_data_n(acc->m_currency_id);
                const auto to_acc = AccountModel::instance().get_id_data_n(trx_n->m_to_account_id_n);
                const auto to_curr = CurrencyModel::instance().get_id_data_n(to_acc->m_currency_id);
                if (curr == to_curr) {
                    trx_n->m_to_amount = trx_n->m_amount;
                }
                else {
                    double exch = 1;
                    const double convRateTo = CurrencyHistoryModel::instance().get_id_date_rate(
                        to_curr->m_id,
                        trx_n->m_date()
                    );
                    if (convRateTo > 0) {
                        const double convRate = CurrencyHistoryModel::instance().get_id_date_rate(
                            curr->m_id,
                            trx_n->m_date()
                        );
                        exch = convRate / convRateTo;
                    }
                    trx_n->m_to_amount = trx_n->m_amount * exch;
                }
            }
        }

        m_custom_fields->UpdateCustomValues(TrxModel::s_ref_type, trx_id);

        TrxModel::instance().unsafe_save_trx_n(trx_n);
    }
    TagLinkModel::instance().db_release_savepoint();
    TrxModel::instance().db_release_savepoint();
    if (!skip_trx.empty()) {
        const wxString detail = wxString::Format("%s\n%s: %zu\n%s: %zu",
            _t("This is due to some elements of the transaction or account detail not allowing the update"),
            _t("Updated"),
            m_trx_id_a.size() - skip_trx.size(),
            _t("Not updated"), skip_trx.size()
        );
        mmErrorDialogs::MessageWarning(this,
            detail,
            _t("Unable to update some transactions.")
        );
    }
    //TODO: enable report to detail transactions that are unable to be updated

    EndModal(wxID_OK);
}

void TrxUpdateDialog::SetPayeeTransferControls()
{
    wxStringClientData* trans_obj = static_cast<wxStringClientData*>(
        m_type_choice->GetClientObject(m_type_choice->GetSelection())
    );
    bool transfer = (TrxType(trans_obj->GetData()).id() == TrxType::e_transfer);

    m_payee_checkbox->Enable(!transfer);
    m_transferAcc_checkbox->Enable(transfer);
    if (transfer) {
        m_payee_checkbox->SetValue(false);
        cbPayee_->Enable(false);
    }
    else {
        m_transferAcc_checkbox->SetValue(false);
        cbAccount_->Enable(false);
    }
}

void TrxUpdateDialog::OnTransTypeChanged(wxCommandEvent&)
{
    SetPayeeTransferControls();
}

void TrxUpdateDialog::OnCheckboxClick(wxCommandEvent& event)
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

void TrxUpdateDialog::onFocusChange(wxChildFocusEvent& event)
{
    cbCategory_->SetValue(cbCategory_->GetValue());


    int object_in_focus = -1;
    wxWindow *w = event.GetWindow();
    if (w) {
        object_in_focus = w->GetId();
    }

    if (object_in_focus == m_amount_ctrl->GetId()) {
        m_amount_ctrl->SelectAll();
    }
    else {
        m_amount_ctrl->Calculate();
    }

    event.Skip();
}

void TrxUpdateDialog::OnMoreFields(wxCommandEvent& WXUNUSED(event))
{
    wxBitmapButton* button = static_cast<wxBitmapButton*>(FindWindow(ID_BTN_CUSTOMFIELDS));

    if (button)
        button->SetBitmap(mmBitmapBundle(
            m_custom_fields->IsCustomPanelShown()
                ? png::RIGHTARROW
                : png::LEFTARROW,
            mmBitmapButtonSize
        ));

    m_custom_fields->ShowHideCustomPanel();

    this->SetMinSize(wxSize(0, 0));
    this->Fit();
}

void TrxUpdateDialog::OnComboKey(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_RETURN) {
        auto id = event.GetId();
        switch (id) {
        case mmID_PAYEE:
        {
            const auto payeeName = cbPayee_->GetValue();
            if (payeeName.empty()) {
                mmPayeeDialog dlg(this, true);
                dlg.ShowModal();
                if (dlg.getRefreshRequested())
                    cbPayee_->mmDoReInitialize();
                int64 payee_id = dlg.getPayeeId();
                const PayeeData* payee_n = PayeeModel::instance().get_id_data_n(payee_id);
                if (payee_n) {
                    cbPayee_->ChangeValue(payee_n->m_name);
                    cbPayee_->SelectAll();
                }
                return;
            }
        }
        break;
        case mmID_CATEGORY:
        {
            auto category = cbCategory_->GetValue();
            if (category.empty()) {
                CategoryManager dlg(this, true, -1);
                dlg.ShowModal();
                if (dlg.getRefreshRequested())
                    cbCategory_->mmDoReInitialize();
                category = CategoryModel::instance().get_id_fullname(dlg.getCategId());
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
    if (event.AltDown() && !altRefreshDone) {
        tagTextCtrl_->Refresh();
        altRefreshDone = true;
    }

    event.Skip();
}
