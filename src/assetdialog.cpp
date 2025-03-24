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

#include "assetdialog.h"
#include "attachmentdialog.h"
#include "constants.h"
#include "images_list.h"
#include "mmSimpleDialogs.h"
#include "mmTextCtrl.h"
#include "paths.h"
#include "util.h"
#include "validators.h"

#include "model/Model_Attachment.h"
#include "usertransactionpanel.h"
#include "accountdialog.h"
#include "mmframe.h"

#include <wx/valnum.h>

wxIMPLEMENT_DYNAMIC_CLASS(mmAssetDialog, wxDialog);

wxBEGIN_EVENT_TABLE( mmAssetDialog, wxDialog )
    EVT_BUTTON(wxID_OK, mmAssetDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, mmAssetDialog::OnCancel)
    EVT_BUTTON(wxID_FILE, mmAssetDialog::OnAttachments)
    EVT_CHOICE(IDC_COMBO_TYPE, mmAssetDialog::OnChangeAppreciationType)
    EVT_CHOICE(IDC_COMPOUNDING, mmAssetDialog::OnChangeCompounding)
    EVT_CHILD_FOCUS(mmAssetDialog::changeFocus)
    EVT_CLOSE(mmAssetDialog::OnQuit)
wxEND_EVENT_TABLE()

mmAssetDialog::mmAssetDialog(wxWindow* parent, Model_Asset::Data* asset, const bool trans_data)
    : m_asset(asset)
    , m_dialog_heading (_t("New Asset"))
{
    if (m_asset || trans_data)
    {
        m_dialog_heading = _t("Edit Asset");
        if (trans_data)
        {
            m_hidden_trans_entry = false;
            m_dialog_heading = _t("Add Asset Transaction");
        }
    }

    this->SetFont(parent->GetFont());
    Create(parent, wxID_ANY, m_dialog_heading);
}

mmAssetDialog::mmAssetDialog(wxWindow* parent, mmGUIFrame* gui_frame, Model_Translink::Data* transfer_entry, Model_Checking::Data* checking_entry)
    : m_gui_frame(gui_frame)
    , m_transfer_entry(transfer_entry)
    , m_checking_entry(checking_entry)
    , m_dialog_heading (_t("Add Asset Transaction"))
    , m_hidden_trans_entry(false)
{
    if (transfer_entry)
    {
        m_dialog_heading = _t("Edit Asset Transaction");
        m_asset = Model_Asset::instance().get(transfer_entry->LINKRECORDID);
    }

    Create(parent, wxID_ANY, m_dialog_heading);
}

bool mmAssetDialog::Create(wxWindow* parent
    , wxWindowID id
    , const wxString& caption
    , const wxPoint& pos
    , const wxSize& size
    , long style)
{
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
    return true;
}

void mmAssetDialog::dataToControls()
{
    if (!this->m_asset) return;

    m_assetName->SetValue(m_asset->ASSETNAME);
    if (Model_Account::instance().get(m_asset->ASSETNAME))
        m_assetName->Enable(false);
    m_dpc->SetValue(Model_Asset::STARTDATE(m_asset));
    m_assetType->SetSelection(Model_Asset::type_id(m_asset));

    auto bal = Model_Asset::value(m_asset);
    m_value->SetValue(bal.first);
    m_curr_val->SetValue(bal.second);

    int valueChangeType = Model_Asset::change_id(m_asset);
    m_valueChange->SetSelection(valueChangeType);

    // m_asset->VALUECHANGERATE is the rate with daily compounding
    double valueChangeRate = m_asset->VALUECHANGERATE;
    if (valueChangeType != Model_Asset::CHANGE_ID_NONE &&
        m_compounding != Option::COMPOUNDING_ID_DAY
    ) {
        valueChangeRate = convertRate(valueChangeType, valueChangeRate, Option::COMPOUNDING_ID_DAY, m_compounding);
    }
    m_valueChangeRate->SetValue(valueChangeRate, 3);
    enableDisableRate(valueChangeType != Model_Asset::CHANGE_ID_NONE);

    m_notes->SetValue(m_asset->NOTES);

    Model_Translink::Data_Set translink = Model_Translink::TranslinkList(
        Model_Attachment::REFTYPE_ID_ASSET, m_asset->ASSETID
    );
    if (!translink.empty())
        m_value->Enable(false);

    // Set up the transaction if this is the first entry.
    if (translink.empty())
        m_transaction_panel->SetTransactionValue(bal.first);

    if (!m_hidden_trans_entry) {
        m_assetName->Enable(false);
        m_dpc->Enable(false);
        m_assetType->Enable(false);
        m_value->Enable(false);
        m_valueChange->Enable(false);
        m_valueChange->Enable(false);
    }

    if (m_checking_entry && !m_checking_entry->DELETEDTIME.IsEmpty()) {
        m_valueChange->Enable(false);
        m_compoundingChoice->Enable(false);
        m_valueChangeRate->Enable(false);
        m_notes->Enable(false);
        bAttachments_->Enable(false);
    }
}

void mmAssetDialog::CreateControls()
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

    wxPanel* asset_details_panel = new wxPanel(this, wxID_STATIC);
    details_frame_sizer->Add(asset_details_panel, g_flagsV);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    asset_details_panel->SetSizer(itemFlexGridSizer6);

    wxStaticText* n = new wxStaticText(asset_details_panel, wxID_STATIC, _t("Name"));
    itemFlexGridSizer6->Add(n, g_flagsH);
    n->SetFont(this->GetFont().Bold());

    m_assetName = new wxTextCtrl(asset_details_panel, wxID_ANY, wxGetEmptyString());
    mmToolTip(m_assetName, _t("Enter the name of the asset"));
    itemFlexGridSizer6->Add(m_assetName, g_flagsExpand);

    itemFlexGridSizer6->Add(new wxStaticText(asset_details_panel, wxID_STATIC, _t("Date")), g_flagsH);
    m_dpc = new mmDatePickerCtrl(asset_details_panel, wxID_ANY);
    itemFlexGridSizer6->Add(m_dpc->mmGetLayout(false));
    mmToolTip(m_dpc, _t("Specify the date of purchase of asset"));

    itemFlexGridSizer6->Add(new wxStaticText(asset_details_panel, wxID_STATIC, _t("Asset Type")), g_flagsH);

    m_assetType = new wxChoice(asset_details_panel, wxID_STATIC);
    for (int i = 0; i < Model_Asset::TYPE_ID_size; ++i) {
        wxString type = Model_Asset::type_name(i);
        m_assetType->Append(wxGetTranslation(type), new wxStringClientData(type));
    }

    mmToolTip(m_assetType, _t("Select type of asset"));
    m_assetType->SetSelection(Model_Asset::TYPE_ID_PROPERTY);
    itemFlexGridSizer6->Add(m_assetType, g_flagsExpand);

    wxStaticText* v = new wxStaticText(asset_details_panel, wxID_STATIC, _t("Initial Value"));
    itemFlexGridSizer6->Add(v, g_flagsH);
    v->SetFont(this->GetFont().Bold());

    m_value = new mmTextCtrl(
        asset_details_panel, IDC_VALUE, wxGetEmptyString(),
        wxDefaultPosition, wxSize(150,-1), wxALIGN_RIGHT|wxTE_PROCESS_ENTER,
        mmCalcValidator()
    );
    mmToolTip(m_value, _t("Enter the current value of the asset"));
    itemFlexGridSizer6->Add(m_value, g_flagsExpand);

    wxStaticText* c = new wxStaticText(asset_details_panel, wxID_STATIC, _t("Current Value"));
    itemFlexGridSizer6->Add(c, g_flagsH);
    c->SetFont(this->GetFont().Bold());

    m_curr_val = new mmTextCtrl(
        asset_details_panel, IDC_CURR_VAL, wxGetEmptyString(),
        wxDefaultPosition, wxSize(150,-1), wxALIGN_RIGHT|wxTE_PROCESS_ENTER,
        mmCalcValidator()
    );
    m_curr_val->Enable(false);
    mmToolTip(m_curr_val, _t("The current value of the asset"));
    itemFlexGridSizer6->Add(m_curr_val, g_flagsExpand);

    itemFlexGridSizer6->Add(new wxStaticText(asset_details_panel, wxID_STATIC, _t("Change in Value")), g_flagsH);

    m_valueChange = new wxChoice(asset_details_panel, IDC_COMBO_TYPE);
    for (int i = 0; i < Model_Asset::CHANGE_ID_size; ++i) {
        wxString change = Model_Asset::change_name(i);
        m_valueChange->Append(wxGetTranslation(change));
    }

    mmToolTip(m_valueChange, _t("Specify if the value of the asset changes over time"));
    m_valueChange->SetSelection(Model_Asset::CHANGE_ID_NONE);
    itemFlexGridSizer6->Add(m_valueChange, g_flagsExpand);

    m_compoundingLabel = new wxStaticText(asset_details_panel, wxID_STATIC, _t("Compounding Period"));
    itemFlexGridSizer6->Add(m_compoundingLabel, g_flagsH);
    m_compoundingChoice = new wxChoice(asset_details_panel, IDC_COMPOUNDING);
    for(const auto& a : Option::COMPOUNDING_NAME)
        m_compoundingChoice->Append(wxGetTranslation(a.second));
    mmToolTip(m_compoundingChoice, _t("Select the compounding period for the appreciation/depreciation rate"));
    m_compounding = static_cast<Option::COMPOUNDING_ID>(Option::instance().getAssetCompounding());
    m_compoundingChoice->SetSelection(m_compounding);
    itemFlexGridSizer6->Add(m_compoundingChoice, g_flagsExpand);

    m_valueChangeRateLabel = new wxStaticText(asset_details_panel, wxID_STATIC, _t("% Rate"));
    itemFlexGridSizer6->Add(m_valueChangeRateLabel, g_flagsH);
    m_valueChangeRate = new mmTextCtrl(
        asset_details_panel, IDC_RATE, wxGetEmptyString(),
        wxDefaultPosition, wxSize(150,-1), wxALIGN_RIGHT|wxTE_PROCESS_ENTER,
        mmCalcValidator()
    );
    m_valueChangeRate->SetAltPrecision(3);
    mmToolTip(m_valueChangeRate, _t("Enter the rate at which the asset changes its value in percentage per year"));
    itemFlexGridSizer6->Add(m_valueChangeRate, g_flagsExpand);
    enableDisableRate(false);

    itemFlexGridSizer6->Add(new wxStaticText( asset_details_panel, wxID_STATIC, _t("Notes")), g_flagsH);

    bAttachments_ = new wxBitmapButton(asset_details_panel, wxID_FILE
        , mmBitmapBundle(png::CLIP, mmBitmapButtonSize), wxDefaultPosition
        , wxSize(m_valueChange->GetSize().GetY(), m_valueChange->GetSize().GetY()));
    itemFlexGridSizer6->Add(bAttachments_, wxSizerFlags(g_flagsV).Align(wxALIGN_RIGHT));
    mmToolTip(bAttachments_, _t("Organize attachments of this asset"));

    m_notes = new wxTextCtrl(this, IDC_NOTES, wxGetEmptyString(), wxDefaultPosition, wxSize(220, 170), wxTE_MULTILINE);
    mmToolTip(m_notes, _t("Enter notes associated with this asset"));
    details_frame_sizer->Add(m_notes, 0, wxGROW | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    /********************************************************************
    Asset Transaction Panel
    *********************************************************************/
    wxString trans_frame_heading = _t("New Transaction Details");
    if (m_checking_entry)
    {
        trans_frame_heading = _t("Edit Transaction Details");
    }

    m_transaction_frame = new wxStaticBox(this, wxID_ANY, trans_frame_heading);
    wxStaticBoxSizer* transaction_frame_sizer = new wxStaticBoxSizer(m_transaction_frame, wxVERTICAL);
    right_sizer->Add(transaction_frame_sizer, g_flagsV);

    m_transaction_panel = new UserTransactionPanel(this, m_checking_entry, wxID_STATIC);
    transaction_frame_sizer->Add(m_transaction_panel, g_flagsV);
    if (m_transfer_entry && m_checking_entry)
    {
        m_transaction_panel->CheckingType(Model_Translink::type_checking(m_checking_entry->TOACCOUNTID));
    }
    else
    {
        if (m_asset)
        {
            m_transaction_panel->SetTransactionNumber(m_asset->ASSETID.ToString() + "_" + m_asset->ASSETNAME);
            m_transaction_panel->CheckingType(Model_Translink::AS_INCOME_EXPENSE);
        }
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

void mmAssetDialog::HideTransactionPanel()
{
    m_transaction_frame->Hide();
    m_transaction_panel->Hide();
}

void mmAssetDialog::enableDisableRate(bool en)
{
    m_valueChangeRateLabel->Enable(en);
    // if (!en) m_valueChangeRate->SetValue("0");
    m_valueChangeRate->SetEditable(en);
    m_valueChangeRate->Enable(en);
    m_compoundingLabel->Enable(en);
    m_compoundingChoice->Enable(en);
}

double mmAssetDialog::convertRate(int changeType, double xRate, int xCompounding, int yCompounding)
{
    int sign = changeType == Model_Asset::CHANGE_ID_DEPRECIATE ? -1 : 1;
    int xN = Option::COMPOUNDING_N[xCompounding].second;
    int yN = Option::COMPOUNDING_N[yCompounding].second;
    // solve (1.0 + sign*xRate/(xN*100.0))^xN = (1.0 + sign*yRate/(yN*100.0))^yN
    double xMult = 1.0 + sign * xRate / (xN * 100.0);
    double yMult = (xMult > 0.0) ? pow(xMult, double(xN)/double(yN)) : 0.0;
    double yRate = sign * (yMult - 1.0) * (yN * 100.0);
    return (yRate > 0.0) ? yRate : 0.0;
}

void mmAssetDialog::OnChangeAppreciationType(wxCommandEvent& /*event*/)
{
    int selection = m_valueChange->GetSelection();
    // Disable for "None", Enable for "Appreciates" or "Depreciates"
    enableDisableRate(selection != Model_Asset::CHANGE_ID_NONE);
}

void mmAssetDialog::OnChangeCompounding(wxCommandEvent& /*event*/)
{
    int selection = m_compoundingChoice->GetSelection();
    if (selection == m_compounding)
        return;

    int valueChangeType = m_valueChange->GetSelection();
    double valueChangeRate = 0;
    if (valueChangeType != Model_Asset::CHANGE_ID_NONE &&
        m_valueChangeRate->checkValue(valueChangeRate)
    ) {
        valueChangeRate = convertRate(valueChangeType, valueChangeRate, m_compounding, selection);
        m_valueChangeRate->SetValue(valueChangeRate, 3);
    }

    m_compounding = static_cast<Option::COMPOUNDING_ID>(selection);
}

void mmAssetDialog::OnOk(wxCommandEvent& /*event*/)
{
    const wxString name = m_assetName->GetValue().Trim();
    if (name.empty()) {
        mmErrorDialogs::InvalidName(m_assetName);
        return;
    }

    double value = 0;
    if (!m_value->checkValue(value))
        return;

    int valueChangeType = m_valueChange->GetSelection();
    double valueChangeRate = 0.0;
    if (valueChangeType != Model_Asset::CHANGE_ID_NONE) {
        if (!m_valueChangeRate->checkValue(valueChangeRate))
            return;
        if (m_compounding != Option::COMPOUNDING_ID_DAY) {
            valueChangeRate = convertRate(valueChangeType, valueChangeRate, m_compounding);
        }
    }

    wxString asset_type = "";
    wxStringClientData* type_obj = static_cast<wxStringClientData *>(m_assetType->GetClientObject(m_assetType->GetSelection()));
    if (type_obj) asset_type = type_obj->GetData();

    bool is_new = !m_asset;
    if (is_new) this->m_asset = Model_Asset::instance().create();

    m_asset->STARTDATE        = m_dpc->GetValue().FormatISODate();
    m_asset->NOTES            = m_notes->GetValue().Trim();
    m_asset->ASSETNAME        = name;
    m_asset->ASSETSTATUS      = Model_Asset::status_name(Model_Asset::STATUS_ID_OPEN);
    m_asset->VALUECHANGEMODE  = Model_Asset::changemode_name(Model_Asset::CHANGEMODE_ID_PERCENTAGE);  
    m_asset->CURRENCYID       = -1; 
    m_asset->VALUE            = value;
    m_asset->VALUECHANGE      = Model_Asset::change_name(valueChangeType);
    m_asset->VALUECHANGERATE  = valueChangeRate;
    m_asset->ASSETTYPE        = asset_type;

    int64 old_asset_id = m_asset->ASSETID;
    int64 new_asset_id = Model_Asset::instance().save(m_asset);

    if (old_asset_id < 0) {
        const wxString& RefType = Model_Attachment::REFTYPE_NAME_ASSET;
        mmAttachmentManage::RelocateAllAttachments(RefType, 0, RefType, new_asset_id);
    }
    if (m_transaction_panel->ValidCheckingAccountEntry()) {
        int64 checking_id = m_transaction_panel->SaveChecking();
        if (checking_id < 0)
            return;

        if (!m_transfer_entry) {
            Model_Translink::SetAssetTranslink(
                new_asset_id, checking_id, m_transaction_panel->CheckingType()
            );
        }
        Model_Translink::UpdateAssetValue(m_asset);
    }
    else if (!m_hidden_trans_entry) {
        mmErrorDialogs::MessageWarning(this, _t("Invalid Transaction"), m_dialog_heading);
        return;
    }

    Model_Account::Data* asset_account = Model_Account::instance().get(name);
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

void mmAssetDialog::SetTransactionAccountName(const wxString& account_name)
{
    m_transaction_panel->SetTransactionAccount(account_name);
}

void mmAssetDialog::SetTransactionDate()
{
    m_transaction_panel->TransactionDate(m_dpc->GetValue());
}

void mmAssetDialog::CreateAssetAccount()
{
    Model_Account::Data* asset_account = Model_Account::instance().create();
    asset_account->ACCOUNTNAME = m_asset->ASSETNAME;
    asset_account->ACCOUNTTYPE = Model_Account::TYPE_NAME_ASSET;
    asset_account->FAVORITEACCT = "FALSE";
    asset_account->STATUS = Model_Account::STATUS_NAME_OPEN;
    asset_account->INITIALBAL = 0;
    asset_account->INITIALDATE = m_asset->STARTDATE;
    asset_account->CURRENCYID = Model_Currency::GetBaseCurrency()->CURRENCYID;
    Model_Account::instance().save(asset_account);

    mmAssetDialog asset_dialog(this, m_asset, true);
    asset_dialog.SetTransactionAccountName(m_asset->ASSETNAME);
    asset_dialog.SetTransactionDate();
    asset_dialog.ShowModal();
}

void mmAssetDialog::OnCancel(wxCommandEvent& /*event*/)
{
    if (assetRichText)
        return;
    else
    {
        const wxString& RefType = Model_Attachment::REFTYPE_NAME_ASSET;
        if (!this->m_asset)
            mmAttachmentManage::DeleteAllAttachments(RefType, 0);
        EndModal(wxID_CANCEL);
    }
}

void mmAssetDialog::OnQuit(wxCloseEvent& /*event*/)
{
    const wxString& RefType = Model_Attachment::REFTYPE_NAME_ASSET;
    if (!this->m_asset)
        mmAttachmentManage::DeleteAllAttachments(RefType, 0);
    EndModal(wxID_CANCEL);
}

void mmAssetDialog::OnAttachments(wxCommandEvent& /*event*/)
{
    const wxString& RefType = Model_Attachment::REFTYPE_NAME_ASSET;
    int64 RefId;
    
    if (!this->m_asset)
        RefId = 0;
    else
        RefId= m_asset->ASSETID;

    mmAttachmentDialog dlg(this, RefType, RefId);
    dlg.ShowModal();
}

void mmAssetDialog::changeFocus(wxChildFocusEvent& event)
{
    wxWindow *w = event.GetWindow();
    if (w) assetRichText = (w->GetId() == IDC_NOTES ? true : false);
}
