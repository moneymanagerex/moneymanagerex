/*******************************************************
 Copyright (C) 2013 - 2016, 2020, 2022 Nikolay Akimov
 Copyright (C) 2022  Mark Whalley (mark@ipx.co.uk)

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
#include <wx/valnum.h>

#include "base/constants.h"
#include "base/paths.h"
#include "base/images_list.h"
#include "util/_util.h"
#include "util/_simple.h"
#include "util/mmTextCtrl.h"
#include "util/mmCalcValidator.h"

#include "AssetDialog.h"
#include "AccountDialog.h"
#include "AttachmentDialog.h"
#include "TrxLinkDialog.h"
#include "uicontrols/navigatortypes.h"

wxIMPLEMENT_DYNAMIC_CLASS(AssetDialog, wxDialog);

wxBEGIN_EVENT_TABLE( AssetDialog, wxDialog )
    EVT_BUTTON(wxID_OK, AssetDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, AssetDialog::OnCancel)
    EVT_BUTTON(wxID_FILE, AssetDialog::OnAttachments)
    EVT_CHOICE(IDC_COMBO_TYPE, AssetDialog::OnChangeAppreciationType)
    EVT_CHOICE(IDC_COMPOUNDING, AssetDialog::OnChangeCompounding)
    EVT_CHILD_FOCUS(AssetDialog::changeFocus)
    EVT_CLOSE(AssetDialog::OnQuit)
wxEND_EVENT_TABLE()

AssetDialog::AssetDialog(
    wxWindow* parent, AssetData* asset, bool trans_data
) :
    m_asset_n(asset),
    m_dialog_heading(_t("New Asset"))
{
    if (m_asset_n || trans_data) {
        m_dialog_heading = _t("Edit Asset");
        if (trans_data) {
            m_hidden_trans_entry = false;
            m_dialog_heading = _t("Add Asset Transaction");
        }
    }

    this->SetFont(parent->GetFont());
    Create(parent, wxID_ANY, m_dialog_heading);
}

AssetDialog::AssetDialog(
    wxWindow* parent,
    const TrxLinkData* tl_d,
    TrxData* checking_entry
) :
    m_transfer_entry(tl_d),
    m_checking_entry(checking_entry),
    m_dialog_heading(_t("Add Asset Transaction")),
    m_hidden_trans_entry(false)
{
    if (tl_d) {
        m_dialog_heading = _t("Edit Asset Transaction");
        m_asset_n = AssetModel::instance().unsafe_get_id_data_n(tl_d->m_ref_id);
    }

    Create(parent, wxID_ANY, m_dialog_heading);
}

bool AssetDialog::Create(
    wxWindow* parent,
    wxWindowID id,
    const wxString& caption,
    const wxPoint& pos,
    const wxSize& size,
    long style
) {
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);

    if (!wxDialog::Create(parent, id, caption, pos, size, style))
        return false;

    CreateControls();
    dataToControls();
    GetSizer()->Fit(this);
    this->SetInitialSize();
    GetSizer()->SetSizeHints(this);
    SetIcon(mmex::getProgramIcon());
    Centre();
    mmThemeAutoColour(this);
    return true;
}

void AssetDialog::dataToControls()
{
    if (!m_asset_n)
        return;

    w_assetName->SetValue(m_asset_n->m_name);
    if (AccountModel::instance().get_name_data_n(m_asset_n->m_name))
        w_assetName->Enable(false);
    w_dpc->SetValue(m_asset_n->m_start_date.getDateTime());
    w_assetType->SetSelection(m_asset_n->m_type.id());
    if (AccountModel::instance().get_name_data_n(m_asset_n->m_type.name()))
        w_assetType->Enable(false);

    auto bal = AssetModel::instance().get_data_value(*m_asset_n);
    w_value->SetValue(bal.first);
    w_curr_val->SetValue(bal.second);

    mmChoiceId valueChangeType = m_asset_n->m_change.id();
    w_valueChange->SetSelection(valueChangeType);

    // m_asset_n->m_change_rate is the rate with daily compounding
    double valueChangeRate = m_asset_n->m_change_rate;
    if (valueChangeType != AssetChange::e_none &&
        m_compounding != PrefModel::COMPOUNDING_ID_DAY
    ) {
        valueChangeRate = convertRate(valueChangeType, valueChangeRate, PrefModel::COMPOUNDING_ID_DAY, m_compounding);
    }
    w_valueChangeRate->SetValue(valueChangeRate, 3);
    enableDisableRate(valueChangeType != AssetChange::e_none);

    w_notes->SetValue(m_asset_n->m_notes);

    TrxLinkModel::DataA tl_a = TrxLinkModel::instance().find_ref_data_a(
        AssetModel::s_ref_type, m_asset_n->m_id
    );
    if (!tl_a.empty())
        w_value->Enable(false);

    // Set up the transaction if this is the first entry.
    if (tl_a.empty())
        w_transaction_panel->SetTransactionValue(bal.first);

    if (!m_hidden_trans_entry) {
        w_assetName->Enable(false);
        w_dpc->Enable(false);
        w_assetType->Enable(false);
        w_value->Enable(false);
        w_valueChange->Enable(false);
        w_valueChange->Enable(false);
    }

    if (m_checking_entry && m_checking_entry->is_deleted()) {
        w_valueChange->Enable(false);
        w_compoundingChoice->Enable(false);
        w_valueChangeRate->Enable(false);
        w_notes->Enable(false);
        w_attachments->Enable(false);
    }
}

void AssetDialog::CreateControls()
{
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(main_sizer);

    wxBoxSizer* panel_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* left_sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* right_sizer = new wxBoxSizer(wxVERTICAL);

    main_sizer->Add(panel_sizer, wxSizerFlags(g_flagsV));
    panel_sizer->Add(left_sizer, 0);
    panel_sizer->Add(right_sizer, 0);

    /********************************************************************
    Asset Details Panel
    *********************************************************************/
    wxStaticBox* details_frame = new wxStaticBox(this, wxID_ANY, _t("Asset Details"));
    wxStaticBoxSizer* details_frame_sizer = new wxStaticBoxSizer(details_frame, wxVERTICAL);
    left_sizer->Add(details_frame_sizer, g_flagsV);

    wxPanel* asset_details_panel = new wxPanel(details_frame, wxID_STATIC);
    details_frame_sizer->Add(asset_details_panel, g_flagsV);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    asset_details_panel->SetSizer(itemFlexGridSizer6);

    wxStaticText* n = new wxStaticText(asset_details_panel, wxID_STATIC, _t("Name"));
    itemFlexGridSizer6->Add(n, g_flagsH);
    n->SetFont(this->GetFont().Bold());

    w_assetName = new wxTextCtrl(asset_details_panel, wxID_ANY, wxGetEmptyString());
    mmToolTip(w_assetName, _t("Enter the name of the asset"));
    itemFlexGridSizer6->Add(w_assetName, g_flagsExpand);

    itemFlexGridSizer6->Add(new wxStaticText(asset_details_panel, wxID_STATIC, _t("Date")), g_flagsH);
    w_dpc = new mmDatePickerCtrl(asset_details_panel, wxID_ANY);
    itemFlexGridSizer6->Add(w_dpc->mmGetLayout(false));
    mmToolTip(w_dpc, _t("Specify the date of purchase of asset"));

    itemFlexGridSizer6->Add(new wxStaticText(asset_details_panel, wxID_STATIC, _t("Asset Type")), g_flagsH);

    w_assetType = new wxChoice(asset_details_panel, wxID_STATIC);
    for (int i = 0; i < AssetType::size; ++i) {
        wxString type = AssetType(i).name();
        w_assetType->Append(wxGetTranslation(type), new wxStringClientData(type));
    }

    mmToolTip(w_assetType, _t("Select type of asset"));
    w_assetType->SetSelection(AssetType().id());
    itemFlexGridSizer6->Add(w_assetType, g_flagsExpand);

    wxStaticText* v = new wxStaticText(asset_details_panel, wxID_STATIC, _t("Initial Value"));
    itemFlexGridSizer6->Add(v, g_flagsH);
    v->SetFont(this->GetFont().Bold());

    w_value = new mmTextCtrl(
        asset_details_panel, IDC_VALUE, wxGetEmptyString(),
        wxDefaultPosition, wxSize(150,-1), wxALIGN_RIGHT|wxTE_PROCESS_ENTER,
        mmCalcValidator()
    );
    mmToolTip(w_value, _t("Enter the current value of the asset"));
    itemFlexGridSizer6->Add(w_value, g_flagsExpand);

    wxStaticText* c = new wxStaticText(asset_details_panel, wxID_STATIC, _t("Current Value"));
    itemFlexGridSizer6->Add(c, g_flagsH);
    c->SetFont(this->GetFont().Bold());

    w_curr_val = new mmTextCtrl(
        asset_details_panel, IDC_CURR_VAL, wxGetEmptyString(),
        wxDefaultPosition, wxSize(150,-1), wxALIGN_RIGHT|wxTE_PROCESS_ENTER,
        mmCalcValidator()
    );
    w_curr_val->Enable(false);
    mmToolTip(w_curr_val, _t("The current value of the asset"));
    itemFlexGridSizer6->Add(w_curr_val, g_flagsExpand);

    itemFlexGridSizer6->Add(new wxStaticText(asset_details_panel, wxID_STATIC, _t("Change in Value")), g_flagsH);

    w_valueChange = new wxChoice(asset_details_panel, IDC_COMBO_TYPE);
    for (mmChoiceId i = 0; i < AssetChange::size; ++i) {
        wxString change = AssetChange(i).name();
        w_valueChange->Append(wxGetTranslation(change));
    }

    mmToolTip(w_valueChange, _t("Specify if the value of the asset changes over time"));
    w_valueChange->SetSelection(AssetChange::e_none);
    itemFlexGridSizer6->Add(w_valueChange, g_flagsExpand);

    w_compoundingLabel = new wxStaticText(asset_details_panel, wxID_STATIC, _t("Compounding Period"));
    itemFlexGridSizer6->Add(w_compoundingLabel, g_flagsH);
    w_compoundingChoice = new wxChoice(asset_details_panel, IDC_COMPOUNDING);
    for(const auto& a : PrefModel::COMPOUNDING_NAME)
        w_compoundingChoice->Append(wxGetTranslation(a.second));
    mmToolTip(w_compoundingChoice, _t("Select the compounding period for the appreciation/depreciation rate"));
    m_compounding = static_cast<PrefModel::COMPOUNDING_ID>(PrefModel::instance().getAssetCompounding());
    w_compoundingChoice->SetSelection(m_compounding);
    itemFlexGridSizer6->Add(w_compoundingChoice, g_flagsExpand);

    w_valueChangeRateLabel = new wxStaticText(asset_details_panel, wxID_STATIC, _t("% Rate"));
    itemFlexGridSizer6->Add(w_valueChangeRateLabel, g_flagsH);
    w_valueChangeRate = new mmTextCtrl(
        asset_details_panel, IDC_RATE, wxGetEmptyString(),
        wxDefaultPosition, wxSize(150,-1), wxALIGN_RIGHT|wxTE_PROCESS_ENTER,
        mmCalcValidator()
    );
    w_valueChangeRate->SetAltPrecision(3);
    mmToolTip(w_valueChangeRate, _t("Enter the rate at which the asset changes its value in percentage per year"));
    itemFlexGridSizer6->Add(w_valueChangeRate, g_flagsExpand);
    enableDisableRate(false);

    itemFlexGridSizer6->Add(new wxStaticText( asset_details_panel, wxID_STATIC, _t("Notes")), g_flagsH);

    w_attachments = new wxBitmapButton(asset_details_panel, wxID_FILE
        , mmBitmapBundle(png::CLIP, mmBitmapButtonSize), wxDefaultPosition
        , wxSize(w_valueChange->GetSize().GetY(), w_valueChange->GetSize().GetY()));
    itemFlexGridSizer6->Add(w_attachments, wxSizerFlags(g_flagsV).Align(wxALIGN_RIGHT));
    mmToolTip(w_attachments, _t("Organize attachments of this asset"));

    w_notes = new wxTextCtrl(details_frame, IDC_NOTES, wxGetEmptyString(), wxDefaultPosition, wxSize(220, 170), wxTE_MULTILINE);
    mmToolTip(w_notes, _t("Enter notes associated with this asset"));
    details_frame_sizer->Add(w_notes, 0, wxGROW | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    /********************************************************************
    Asset Transaction Panel
    *********************************************************************/
    wxString trans_frame_heading = _t("New Transaction Details");
    if (m_checking_entry) {
        trans_frame_heading = _t("Edit Transaction Details");
    }

    w_transaction_frame = new wxStaticBox(this, wxID_ANY, trans_frame_heading);
    wxStaticBoxSizer* transaction_frame_sizer = new wxStaticBoxSizer(w_transaction_frame, wxVERTICAL);
    right_sizer->Add(transaction_frame_sizer, g_flagsV);

    w_transaction_panel = new TrxLinkDialog(w_transaction_frame, m_checking_entry, true, wxID_STATIC);
    transaction_frame_sizer->Add(w_transaction_panel, g_flagsV);
    if (m_transfer_entry && m_checking_entry) {
        w_transaction_panel->CheckingType(
            TrxLinkModel::type_checking(m_checking_entry->m_to_account_id_n)
        );
    }
    else if (m_asset_n) {
        w_transaction_panel->SetTransactionNumber(
            m_asset_n->m_id.ToString() + "_" + m_asset_n->m_name
        );
        w_transaction_panel->CheckingType(TrxLinkModel::AS_INCOME_EXPENSE);
    }

    if (m_hidden_trans_entry) HideTransactionPanel();
    /********************************************************************
    Separation Line
    *********************************************************************/
    wxStaticLine* separation_line = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    main_sizer->Add(separation_line, 0, wxEXPAND | wxALL, 1);

    /********************************************************************
    Button Panel
    *********************************************************************/
    wxPanel* button_panel = new wxPanel(this, wxID_STATIC);
    wxBoxSizer* button_panel_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* ok_button = new wxButton(button_panel, wxID_OK, _t("&OK "));
    wxButton* cancel_button = new wxButton(button_panel, wxID_CANCEL, _t("&Cancel "));

    main_sizer->Add(button_panel, wxSizerFlags(g_flagsV).Center());
    button_panel->SetSizer(button_panel_sizer);
    button_panel_sizer->Add(ok_button, g_flagsV);

    button_panel_sizer->Add(cancel_button, g_flagsV);
    //cancel_button->SetFocus();
}

void AssetDialog::HideTransactionPanel()
{
    w_transaction_frame->Hide();
    w_transaction_panel->Hide();
}

void AssetDialog::enableDisableRate(bool en)
{
    w_valueChangeRateLabel->Enable(en);
    // if (!en) w_valueChangeRate->SetValue("0");
    w_valueChangeRate->SetEditable(en);
    w_valueChangeRate->Enable(en);
    w_compoundingLabel->Enable(en);
    w_compoundingChoice->Enable(en);
}

double AssetDialog::convertRate(
    mmChoiceId changeType, double xRate, int xCompounding, int yCompounding
) {
    int sign = (changeType == AssetChange::e_depreciates) ? -1 : 1;
    int xN = PrefModel::COMPOUNDING_N[xCompounding].second;
    int yN = PrefModel::COMPOUNDING_N[yCompounding].second;
    // solve (1.0 + sign*xRate/(xN*100.0))^xN = (1.0 + sign*yRate/(yN*100.0))^yN
    double xMult = 1.0 + sign * xRate / (xN * 100.0);
    double yMult = (xMult > 0.0) ? pow(xMult, double(xN)/double(yN)) : 0.0;
    double yRate = sign * (yMult - 1.0) * (yN * 100.0);
    return (yRate > 0.0) ? yRate : 0.0;
}

void AssetDialog::OnChangeAppreciationType(wxCommandEvent& /*event*/)
{
    mmChoiceId selection = w_valueChange->GetSelection();
    // Disable for "None", Enable for "Appreciates" or "Depreciates"
    enableDisableRate(selection != AssetChange::e_none);
}

void AssetDialog::OnChangeCompounding(wxCommandEvent& /*event*/)
{
    int selection = w_compoundingChoice->GetSelection();
    if (selection == m_compounding)
        return;

    mmChoiceId change_id = w_valueChange->GetSelection();
    double change_rate = 0;
    if (change_id != AssetChange::e_none &&
        w_valueChangeRate->checkValue(change_rate)
    ) {
        change_rate = convertRate(change_id, change_rate, m_compounding, selection);
        w_valueChangeRate->SetValue(change_rate, 3);
    }

    m_compounding = static_cast<PrefModel::COMPOUNDING_ID>(selection);
}

void AssetDialog::OnOk(wxCommandEvent& /*event*/)
{
    AssetType asset_type = AssetType();
    wxStringClientData* type_obj = static_cast<wxStringClientData *>(
        w_assetType->GetClientObject(w_assetType->GetSelection())
    );
    if (type_obj)
        asset_type = AssetType(type_obj->GetData());

    const wxString asset_name = w_assetName->GetValue().Trim();
    if (asset_name.empty()) {
        mmErrorDialogs::InvalidName(w_assetName);
        return;
    }

    double asset_value = 0.0;
    if (!w_value->checkValue(asset_value))
        return;

    mmChoiceId asset_change_id = w_valueChange->GetSelection();
    double asset_change_rate = 0.0;
    if (asset_change_id != AssetChange::e_none) {
        if (!w_valueChangeRate->checkValue(asset_change_rate))
            return;
        if (m_compounding != PrefModel::COMPOUNDING_ID_DAY) {
            asset_change_rate = convertRate(asset_change_id, asset_change_rate, m_compounding);
        }
    }

    bool is_new = !m_asset_n;
    if (!m_asset_n) {
        m_asset_d = AssetData();
        m_asset_n = &m_asset_d;
    }

    m_asset_n->m_type          = asset_type;
    m_asset_n->m_status        = AssetStatus();
    m_asset_n->m_name          = asset_name;
    m_asset_n->m_start_date    = mmDate(w_dpc->GetValue());
    m_asset_n->m_currency_id_n = -1;
    m_asset_n->m_value         = asset_value;
    m_asset_n->m_change        = AssetChange(asset_change_id);
    m_asset_n->m_change_mode   = AssetChangeMode();
    m_asset_n->m_change_rate   = asset_change_rate;
    m_asset_n->m_notes         = w_notes->GetValue().Trim();

    int64 old_asset_id = m_asset_n->id();
    AssetModel::instance().unsafe_save_data_n(m_asset_n);
    int64 new_asset_id = m_asset_n->id();

    if (old_asset_id < 0) {
        mmAttachmentManage::RelocateAllAttachments(
            AssetModel::s_ref_type, 0,
            AssetModel::s_ref_type, new_asset_id
        );
    }
    if (w_transaction_panel->ValidCheckingAccountEntry()) {
        int64 trx_id = w_transaction_panel->SaveChecking();
        if (trx_id < 0)
            return;

        if (!m_transfer_entry) {
            TrxLinkModel::instance().save_asset_record(
                trx_id, new_asset_id,
                w_transaction_panel->CheckingType()
            );
        }
        TrxLinkModel::instance().update_asset_value(m_asset_n);
    }
    else if (!m_hidden_trans_entry) {
        mmErrorDialogs::MessageWarning(this, _t("Invalid Transaction"), m_dialog_heading);
        return;
    }

    const AccountData* asset_account = AccountModel::instance().get_name_data_n(asset_name);
    if (is_new && !asset_account) {
        if (wxMessageBox(
            _t("Asset account not found.") + "\n\n" + _t("Do you want to create one?"),
            _t("New Asset"), wxYES_NO | wxICON_INFORMATION
        ) == wxYES) {
            CreateAssetAccount();
        }
    }

    EndModal(wxID_OK);
}

void AssetDialog::SetTransactionAccountName(const wxString& account_name)
{
    w_transaction_panel->SetTransactionAccount(account_name);
}

void AssetDialog::SetTransactionDate()
{
    w_transaction_panel->TransactionDate(w_dpc->GetValue());
}

void AssetDialog::CreateAssetAccount()
{
    AccountData new_account_d = AccountData();
    new_account_d.m_name         = m_asset_n->m_type.name();
    new_account_d.m_type_        = NavigatorTypes::instance().getAssetAccountStr();
    new_account_d.m_open_balance = 0;
    new_account_d.m_open_date    = m_asset_n->m_start_date;
    new_account_d.m_currency_id  = CurrencyModel::instance().get_base_data_n()->m_id;
    AccountModel::instance().add_data_n(new_account_d);

    AssetDialog dlg(this, m_asset_n, true);
    dlg.SetTransactionAccountName(m_asset_n->m_type.name());
    dlg.SetTransactionDate();
    dlg.ShowModal();
}

void AssetDialog::OnCancel(wxCommandEvent& /*event*/)
{
    if (m_asset_rich_text)
        return;

    // FIXME: temporary records (with id <= 0) are not stored in database
    if (!m_asset_n)
        mmAttachmentManage::DeleteAllAttachments(AssetModel::s_ref_type, 0);
    EndModal(wxID_CANCEL);
}

void AssetDialog::OnQuit(wxCloseEvent& /*event*/)
{
    // FIXME: temporary records (with id <= 0) are not stored in database
    if (!m_asset_n)
        mmAttachmentManage::DeleteAllAttachments(AssetModel::s_ref_type, 0);
    EndModal(wxID_CANCEL);
}

void AssetDialog::OnAttachments(wxCommandEvent& /*event*/)
{
    int64 ref_id = m_asset_n ? m_asset_n->m_id : 0;
    AttachmentDialog dlg(this, AssetModel::s_ref_type, ref_id);
    dlg.ShowModal();
}

void AssetDialog::changeFocus(wxChildFocusEvent& event)
{
    wxWindow *w = event.GetWindow();
    if (w)
        m_asset_rich_text = (w->GetId() == IDC_NOTES);
}
