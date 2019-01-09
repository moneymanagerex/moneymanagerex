/*******************************************************
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

#include "Model_Attachment.h"
#include "usertransactionpanel.h"
#include "accountdialog.h"
#include "mmframe.h"

wxIMPLEMENT_DYNAMIC_CLASS(mmAssetDialog, wxDialog);

wxBEGIN_EVENT_TABLE( mmAssetDialog, wxDialog )
    EVT_BUTTON(wxID_OK, mmAssetDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, mmAssetDialog::OnCancel)
    EVT_BUTTON(wxID_FILE, mmAssetDialog::OnAttachments)
    EVT_CHOICE(IDC_COMBO_TYPE, mmAssetDialog::OnChangeAppreciationType)
    EVT_CHILD_FOCUS(mmAssetDialog::changeFocus)
    EVT_CLOSE(mmAssetDialog::OnQuit)
wxEND_EVENT_TABLE()

mmAssetDialog::mmAssetDialog(wxWindow* parent, mmGUIFrame* gui_frame, Model_Asset::Data* asset, bool trans_data)
    : m_asset(asset)
    , m_hidden_trans_entry(true)
    , m_transfer_entry(nullptr)
    , m_checking_entry(nullptr)
    , m_assetName()
    , m_dpc()
    , m_notes()
    , m_value()
    , m_valueChangeRate()
    , m_assetType()
    , m_valueChange()
    , m_valueChangeRateLabel()
    , m_dialog_heading (_("New Asset"))
    , m_gui_frame(gui_frame)
{
    if (m_asset || trans_data)
    {
        m_dialog_heading = _("Edit Asset");
        if (trans_data)
        {
            m_hidden_trans_entry = false;
            m_dialog_heading = _("Add Asset Transaction");
        }
    }

    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, m_dialog_heading, wxDefaultPosition, wxSize(400, 300), style);
}

mmAssetDialog::mmAssetDialog(wxWindow* parent, mmGUIFrame* gui_frame, Model_Translink::Data* transfer_entry, Model_Checking::Data* checking_entry)
    : m_asset(nullptr)
    , m_hidden_trans_entry(false)
    , m_transfer_entry(transfer_entry)
    , m_checking_entry(checking_entry)
    , m_assetName()
    , m_dpc()
    , m_notes()
    , m_value()
    , m_valueChangeRate()
    , m_assetType()
    , m_valueChange()
    , m_valueChangeRateLabel()
    , m_dialog_heading (_("Add Asset Transaction"))
    , m_gui_frame(gui_frame)
{
    if (transfer_entry)
    {
        m_dialog_heading = _("Edit Asset Transaction");
        m_asset = Model_Asset::instance().get(transfer_entry->LINKRECORDID);
    }

    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, m_dialog_heading, wxDefaultPosition, wxSize(400, 300), style);
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
    {
        m_assetName->Enable(false);
    }

    m_notes->SetValue(m_asset->NOTES);
    m_dpc->SetValue(Model_Asset::STARTDATE(m_asset));
    m_value->SetValue(std::abs(m_asset->VALUE));

    if (!Model_Translink::TranslinkList(Model_Attachment::ASSET, m_asset->ASSETID).empty())
    {
        m_value->Enable(false);
    }

    m_valueChangeRate->SetValue(m_asset->VALUECHANGERATE, 3);

    m_valueChange->SetSelection(Model_Asset::rate(m_asset));
    enableDisableRate(Model_Asset::rate(m_asset) != Model_Asset::RATE_NONE);
    m_assetType->SetSelection(Model_Asset::type(m_asset));

    // Set up the transaction if this is the first entry.
    if (Model_Translink::TranslinkList(Model_Attachment::ASSET, m_asset->ASSETID).empty())
    {
        m_transaction_panel->SetTransactionValue(m_asset->VALUE, 0);
    }

    if (!m_hidden_trans_entry)
    {
        m_assetName->Enable(false);
        m_assetType->Enable(false);
        m_dpc->Enable(false);
        m_value->Enable(false);
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
    wxStaticBox* details_frame = new wxStaticBox(this, wxID_ANY, _("Asset Details"));
    wxStaticBoxSizer* details_frame_sizer = new wxStaticBoxSizer(details_frame, wxVERTICAL);
    left_sizer->Add(details_frame_sizer, g_flagsV);

    wxPanel* asset_details_panel = new wxPanel(this, wxID_STATIC);
    details_frame_sizer->Add(asset_details_panel, g_flagsV);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    asset_details_panel->SetSizer(itemFlexGridSizer6);

    wxStaticText* n = new wxStaticText(asset_details_panel, wxID_STATIC, _("Name"));
    itemFlexGridSizer6->Add(n, g_flagsV);
    n->SetFont(this->GetFont().Bold());

    m_assetName = new mmTextCtrl(asset_details_panel, wxID_ANY, wxGetEmptyString());
    m_assetName->SetToolTip(_("Enter the name of the asset"));
    itemFlexGridSizer6->Add(m_assetName, g_flagsExpand);

    itemFlexGridSizer6->Add(new wxStaticText(asset_details_panel, wxID_STATIC, _("Date")), g_flagsH);

    m_dpc = new wxDatePickerCtrl( asset_details_panel, wxID_ANY, wxDefaultDateTime,
              wxDefaultPosition, wxSize(150, -1), wxDP_DROPDOWN|wxDP_SHOWCENTURY);
    itemFlexGridSizer6->Add(m_dpc, g_flagsV);
    m_dpc->SetToolTip(_("Specify the date of purchase of asset"));

    itemFlexGridSizer6->Add(new wxStaticText(asset_details_panel, wxID_STATIC, _("Asset Type")), g_flagsH);

    m_assetType = new wxChoice(asset_details_panel, wxID_STATIC, wxDefaultPosition, wxSize(150, -1));
    for (const auto& a : Model_Asset::all_type())
        m_assetType->Append(wxGetTranslation(a), new wxStringClientData(a));

    m_assetType->SetToolTip(_("Select type of asset"));
    m_assetType->SetSelection(Model_Asset::TYPE_PROPERTY);
    itemFlexGridSizer6->Add(m_assetType, 0,
        wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* v = new wxStaticText(asset_details_panel, wxID_STATIC, _("Value"));
    itemFlexGridSizer6->Add(v, g_flagsV);
    v->SetFont(this->GetFont().Bold());

    m_value = new mmTextCtrl(asset_details_panel, IDC_VALUE, wxGetEmptyString()
        , wxDefaultPosition, wxSize(150,-1), wxALIGN_RIGHT|wxTE_PROCESS_ENTER
        , mmCalcValidator() );
    m_value->SetToolTip(_("Enter the current value of the asset"));
    itemFlexGridSizer6->Add(m_value, g_flagsV);
    m_value->Connect(IDC_VALUE, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmAssetDialog::onTextEntered), nullptr, this);

    itemFlexGridSizer6->Add(new wxStaticText(asset_details_panel, wxID_STATIC, _("Change in Value")), g_flagsH);

    m_valueChange = new wxChoice(asset_details_panel, IDC_COMBO_TYPE, wxDefaultPosition, wxSize(150, -1));
    for(const auto& a : Model_Asset::all_rate())
        m_valueChange->Append(wxGetTranslation(a));

    m_valueChange->SetToolTip(_("Specify if the value of the asset changes over time"));
    m_valueChange->SetSelection(Model_Asset::RATE_NONE);
    itemFlexGridSizer6->Add(m_valueChange, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_valueChangeRateLabel = new wxStaticText(asset_details_panel, wxID_STATIC, _("% Rate"));
    itemFlexGridSizer6->Add(m_valueChangeRateLabel, g_flagsV);

    m_valueChangeRate = new mmTextCtrl(asset_details_panel, IDC_RATE, wxGetEmptyString()
        , wxDefaultPosition, wxSize(150,-1), wxALIGN_RIGHT|wxTE_PROCESS_ENTER
        , mmCalcValidator());
    m_valueChangeRate->SetToolTip(_("Enter the rate at which the asset changes its value in percentage per year"));
    itemFlexGridSizer6->Add(m_valueChangeRate, g_flagsV);
    m_valueChangeRate->Connect(IDC_RATE, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmAssetDialog::onTextEntered), nullptr, this);
    enableDisableRate(false);

    itemFlexGridSizer6->Add(new wxStaticText( asset_details_panel, wxID_STATIC, _("Notes")), g_flagsV);

    bAttachments_ = new wxBitmapButton(asset_details_panel, wxID_FILE
        , mmBitmap(png::CLIP), wxDefaultPosition
        , wxSize(m_valueChange->GetSize().GetY(), m_valueChange->GetSize().GetY()));
    itemFlexGridSizer6->Add(bAttachments_, wxSizerFlags(g_flagsV).Align(wxALIGN_RIGHT));
    bAttachments_->SetToolTip(_("Organize attachments of this asset"));

    m_notes = new mmTextCtrl(this, IDC_NOTES, wxGetEmptyString(), wxDefaultPosition, wxSize(220, 170), wxTE_MULTILINE);
    m_notes->SetToolTip(_("Enter notes associated with this asset"));
    details_frame_sizer->Add(m_notes, 0, wxGROW | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    /********************************************************************
    Asset Transaction Panel
    *********************************************************************/
    wxString trans_frame_heading = _("New Transaction Details");
    if (m_checking_entry)
    {
        trans_frame_heading = _("Edit Transaction Details");
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
            m_transaction_panel->SetTransactionNumber(m_asset->ASSETNAME);
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
    wxButton* ok_button = new wxButton(button_panel, wxID_OK, _("&OK "));
    wxButton* cancel_button = new wxButton(button_panel, wxID_CANCEL, _("&Cancel "));

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

void mmAssetDialog::OnChangeAppreciationType(wxCommandEvent& WXUNUSED(event))
{
    int selection = m_valueChange->GetSelection();
    // Disable for "None", Enable for "Appreciates" or "Depreciates"
    enableDisableRate(selection != Model_Asset::RATE_NONE);
}

void mmAssetDialog::enableDisableRate(bool en)
{
    if (en)
    {
        m_valueChangeRate->SetEditable(true);
        m_valueChangeRate->Enable(true);
        m_valueChangeRateLabel->Enable(true);
    }
    else
    {
        //m_valueChangeRate->SetValue("0");
        m_valueChangeRate->SetEditable(false);
        m_valueChangeRate->Enable(false);
        m_valueChangeRateLabel->Enable(false);
    }
}

void mmAssetDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    const wxString name = m_assetName->GetValue().Trim();
    if (name.empty())
    {
        mmErrorDialogs::InvalidName(m_assetName);
        return;
    }

    double value = 0, valueChangeRate = 0;
    if (!m_value->checkValue(value))
    {
        return;
    }

    int valueChangeType = m_valueChange->GetSelection();
    if (valueChangeType != Model_Asset::RATE_NONE && !m_valueChangeRate->checkValue(valueChangeRate))
    {
        return;
    }

    wxString asset_type = "";
    wxStringClientData* type_obj = (wxStringClientData *)m_assetType->GetClientObject(m_assetType->GetSelection());
    if (type_obj) asset_type = type_obj->GetData();

    bool is_new = !m_asset;
    if (is_new) this->m_asset = Model_Asset::instance().create();

    m_asset->STARTDATE        = m_dpc->GetValue().FormatISODate();
    m_asset->NOTES            = m_notes->GetValue().Trim();
    m_asset->ASSETNAME        = name;
    m_asset->VALUE            = value;
    m_asset->VALUECHANGE      = Model_Asset::all_rate()[valueChangeType];
    m_asset->VALUECHANGERATE  = valueChangeRate;
    m_asset->ASSETTYPE        = asset_type;

	int old_asset_id = m_asset->ASSETID;
    int new_asset_id = Model_Asset::instance().save(m_asset);

    if (old_asset_id < 0)
    {
        const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::ASSET);
        mmAttachmentManage::RelocateAllAttachments(RefType, 0, new_asset_id);
    }

    UserTransactionPanel::GUI_ERROR g_err = UserTransactionPanel::GUI_ERROR::NONE;
    if (m_transaction_panel->ValidCheckingAccountEntry(g_err))
    {
        int checking_id = m_transaction_panel->SaveChecking();
        if (!m_transfer_entry)
        {
            Model_Translink::SetAssetTranslink(new_asset_id
                , checking_id, m_transaction_panel->CheckingType());
        }
        Model_Translink::UpdateAssetValue(m_asset);
    }
    else if (!m_hidden_trans_entry)
    {
        if (g_err == UserTransactionPanel::GUI_ERROR::ACCOUNT)
        {
            mmErrorDialogs::InvalidAccount((wxWindow*)m_transaction_panel->m_account, false, mmErrorDialogs::MESSAGE_POPUP_BOX);
        }
        else if (g_err == UserTransactionPanel::GUI_ERROR::PAYEE)
        {
            mmErrorDialogs::InvalidPayee((wxWindow*)m_transaction_panel->m_payee, mmErrorDialogs::MESSAGE_POPUP_BOX);
        }
        else if (g_err == UserTransactionPanel::GUI_ERROR::CATEGORY)
        {
            mmErrorDialogs::InvalidCategory((wxWindow*)m_transaction_panel->m_category, true);
        }
        else if (g_err == UserTransactionPanel::GUI_ERROR::ENTRY)
        {
            mmErrorDialogs::InvalidAmount((wxWindow*)m_transaction_panel->m_entered_amount);
        }

        return;
    }

    Model_Account::Data* asset_account = Model_Account::instance().get(name);
    if (is_new && !asset_account)
    {
        if (wxMessageBox(_("An asset account having the same name has not been found.\n\n"
            "A single transaction for this asset can be associated with any account.\n"
            "Multiple asset transactions need to be associated with an asset account.\n\n"
            "Would you want to create one?\n\n"
        )
            , _("New Asset"), wxYES_NO | wxICON_INFORMATION) == wxYES)
        {
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
    asset_account->ACCOUNTTYPE = Model_Account::all_type()[Model_Account::ASSET];
    asset_account->FAVORITEACCT = "TRUE";
    asset_account->STATUS = Model_Account::all_status()[Model_Account::OPEN];
    asset_account->INITIALBAL = 0;
    asset_account->CURRENCYID = Model_Currency::GetBaseCurrency()->CURRENCYID;
    Model_Account::instance().save(asset_account);

    mmNewAcctDialog account_dialog(asset_account, this);
    account_dialog.ShowModal();
    m_gui_frame->RefreshNavigationTree();

    mmAssetDialog asset_dialog(this, m_gui_frame, m_asset, true);
    asset_dialog.SetTransactionAccountName(m_asset->ASSETNAME);
    asset_dialog.SetTransactionDate();
    asset_dialog.ShowModal();
}

void mmAssetDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    if (assetRichText)
        return;
    else
    {
        const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::ASSET);
        if (!this->m_asset)
            mmAttachmentManage::DeleteAllAttachments(RefType, 0);
        EndModal(wxID_CANCEL);
    }
}

void mmAssetDialog::OnQuit(wxCloseEvent& WXUNUSED(event))
{
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::ASSET);
    if (!this->m_asset)
        mmAttachmentManage::DeleteAllAttachments(RefType, 0);
    EndModal(wxID_CANCEL);
}

void mmAssetDialog::OnAttachments(wxCommandEvent& WXUNUSED(event))
{
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::ASSET);
    int RefId;
    
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

void mmAssetDialog::onTextEntered(wxCommandEvent& event)
{
    if (event.GetId() == m_value->GetId())
    {
        m_value->Calculate();
    }
    else if (event.GetId() == m_valueChangeRate->GetId())
    {
        m_valueChangeRate->Calculate(3);
    }

    event.Skip();
}
