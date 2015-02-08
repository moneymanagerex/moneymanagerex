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
#include "mmSimpleDialogs.h"
#include "mmtextctrl.h"
#include "paths.h"
#include "util.h"
#include "validators.h"

#include "model/Model_Attachment.h"
#include "mmUserPanelTrans.h"

#include "../resources/attachment.xpm"

#include <wx/valnum.h>

wxIMPLEMENT_DYNAMIC_CLASS(mmAssetDialog, wxDialog);

wxBEGIN_EVENT_TABLE( mmAssetDialog, wxDialog )
    EVT_BUTTON(wxID_OK, mmAssetDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, mmAssetDialog::OnCancel)
	EVT_BUTTON(wxID_FILE, mmAssetDialog::OnAttachments)
    EVT_CHOICE(IDC_COMBO_TYPE, mmAssetDialog::OnChangeAppreciationType)
    EVT_CHILD_FOCUS(mmAssetDialog::changeFocus)
    EVT_CLOSE(mmAssetDialog::OnQuit)
wxEND_EVENT_TABLE()

mmAssetDialog::mmAssetDialog(wxWindow* parent, Model_Asset::Data* asset)
    : m_asset(asset)
    , m_assetName()
    , m_dpc()
    , m_notes()
    , m_value()
    , m_valueChangeRate()
    , m_assetType()
    , m_valueChange()
    , m_valueChangeRateLabel()
{
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, _("New/Edit Asset"), wxDefaultPosition, wxSize(400, 300), style);
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
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());

    dataToControls();

    Centre();
    return true;
}

void mmAssetDialog::dataToControls()
{
    if (!this->m_asset) return;

    m_assetName->SetValue(m_asset->ASSETNAME);
    m_notes->SetValue(m_asset->NOTES);
    m_dpc->SetValue(Model_Asset::STARTDATE(m_asset));
    m_value->SetValue(m_asset->VALUE);

    wxString valueChangeRate;
    valueChangeRate.Printf("%.3f", m_asset->VALUECHANGERATE);
    m_valueChangeRate->SetValue(valueChangeRate);

    m_valueChange->SetSelection(Model_Asset::rate(m_asset));
    enableDisableRate(Model_Asset::rate(m_asset) != Model_Asset::RATE_NONE);
    m_assetType->SetSelection(Model_Asset::type(m_asset));
}

void mmAssetDialog::CreateControls()
{
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(main_sizer);

    wxBoxSizer* panel_sizer = new wxBoxSizer(wxHORIZONTAL);
    main_sizer->Add(panel_sizer, wxSizerFlags(g_flags));

    wxBoxSizer* left_sizer = new wxBoxSizer(wxVERTICAL);
    panel_sizer->Add(left_sizer, 0);

    wxStaticBox* details_frame = new wxStaticBox(this, wxID_ANY, _("Asset Details"));
    wxStaticBoxSizer* details_frame_sizer = new wxStaticBoxSizer(details_frame, wxVERTICAL);
    left_sizer->Add(details_frame_sizer, g_flags);

    wxBoxSizer* right_sizer = new wxBoxSizer(wxVERTICAL);
    panel_sizer->Add(right_sizer, 0);

    wxStaticBox* transaction_frame = new wxStaticBox(this, wxID_ANY, _("Asset Transaction Details"));
    wxStaticBoxSizer* transaction_frame_sizer = new wxStaticBoxSizer(transaction_frame, wxVERTICAL);
    right_sizer->Add(transaction_frame_sizer, g_flags);

    mmUserPanelTrans* checking_panel = new mmUserPanelTrans(this, wxID_STATIC
        , wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    transaction_frame_sizer->Add(checking_panel, g_flags);

    /********************************************************************
     Asset Details Panel
    *********************************************************************/
    wxPanel* itemPanel5 = new wxPanel(this, wxID_STATIC, wxDefaultPosition
        , wxDefaultSize, wxTAB_TRAVERSAL );
    details_frame_sizer->Add(itemPanel5, g_flags);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    itemPanel5->SetSizer(itemFlexGridSizer6);

    wxStaticText* n = new wxStaticText(itemPanel5, wxID_STATIC, _("Name"));
    itemFlexGridSizer6->Add(n, g_flags);
    n->SetFont(this->GetFont().Bold());

    m_assetName = new mmTextCtrl(itemPanel5, wxID_ANY, wxGetEmptyString());
    m_assetName->SetToolTip(_("Enter the name of the asset"));
    itemFlexGridSizer6->Add(m_assetName, g_flagsExpand);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Date")), g_flags);

    m_dpc = new wxDatePickerCtrl( itemPanel5, wxID_ANY, wxDefaultDateTime,
              wxDefaultPosition, wxSize(150, -1), wxDP_DROPDOWN|wxDP_SHOWCENTURY);
    itemFlexGridSizer6->Add(m_dpc, g_flags);
    m_dpc->SetToolTip(_("Specify the date of purchase of asset"));

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Asset Type")), g_flags);

    m_assetType = new wxChoice(itemPanel5, wxID_STATIC, wxDefaultPosition, wxSize(150, -1));
    for (const auto& a : Model_Asset::all_type())
        m_assetType->Append(wxGetTranslation(a), new wxStringClientData(a));

    m_assetType->SetToolTip(_("Select type of asset"));
    m_assetType->SetSelection(Model_Asset::TYPE_PROPERTY);
    itemFlexGridSizer6->Add(m_assetType, 0,
        wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* v = new wxStaticText(itemPanel5, wxID_STATIC, _("Value"));
    itemFlexGridSizer6->Add(v, g_flags);
    v->SetFont(this->GetFont().Bold());

    m_value = new mmTextCtrl(itemPanel5, IDC_VALUE, wxGetEmptyString()
        , wxDefaultPosition, wxSize(150,-1), wxALIGN_RIGHT|wxTE_PROCESS_ENTER
        , mmCalcValidator() );
    m_value->SetToolTip(_("Enter the current value of the asset"));
    itemFlexGridSizer6->Add(m_value, g_flags);
    m_value->Connect(IDC_VALUE, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmAssetDialog::onTextEntered), nullptr, this);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Change in Value")), g_flags);

    m_valueChange = new wxChoice(itemPanel5, IDC_COMBO_TYPE, wxDefaultPosition, wxSize(150, -1));
    for(const auto& a : Model_Asset::all_rate())
        m_valueChange->Append(wxGetTranslation(a));

    m_valueChange->SetToolTip(_("Specify if the value of the asset changes over time"));
    m_valueChange->SetSelection(Model_Asset::RATE_NONE);
    itemFlexGridSizer6->Add(m_valueChange, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_valueChangeRateLabel = new wxStaticText( itemPanel5, wxID_STATIC, _("% Rate"));
    itemFlexGridSizer6->Add(m_valueChangeRateLabel, g_flags);

    m_valueChangeRate = new mmTextCtrl(itemPanel5, IDC_RATE, wxGetEmptyString()
        , wxDefaultPosition, wxSize(150,-1), wxALIGN_RIGHT|wxTE_PROCESS_ENTER
        , mmCalcValidator());
    m_valueChangeRate->SetToolTip(_("Enter the rate at which the asset changes its value in % per year"));
    itemFlexGridSizer6->Add(m_valueChangeRate, g_flags);
    m_valueChangeRate->Connect(IDC_RATE, IDC_RATE
        , wxCommandEventHandler(mmAssetDialog::onTextEntered), nullptr, this);
    enableDisableRate(false);

    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Notes")), g_flags);

	bAttachments_ = new wxBitmapButton(itemPanel5, wxID_FILE
		, wxBitmap(attachment_xpm), wxDefaultPosition
		, wxSize(m_valueChange->GetSize().GetY(), m_valueChange->GetSize().GetY()));
	itemFlexGridSizer6->Add(bAttachments_, wxSizerFlags(g_flags).Align(wxALIGN_RIGHT));
	bAttachments_->SetToolTip(_("Organize attachments of this asset"));

    m_notes = new mmTextCtrl(this, IDC_NOTES, wxGetEmptyString(), wxDefaultPosition, wxSize(220, 170), wxTE_MULTILINE);
    m_notes->SetToolTip(_("Enter notes associated with this asset"));
    details_frame_sizer->Add(m_notes, 0, wxGROW | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    /********************************************************************
    Separation Line
    *********************************************************************/
    wxStaticLine* separation_line = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    main_sizer->Add(separation_line, 0, wxEXPAND | wxALL, 1);

    /********************************************************************
    Button Panel
    *********************************************************************/
    wxPanel* button_panel = new wxPanel(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    main_sizer->Add(button_panel, wxSizerFlags(g_flags).Center());

    wxBoxSizer* button_panel_sizer = new wxBoxSizer(wxHORIZONTAL);
    button_panel->SetSizer(button_panel_sizer);

    wxButton* ok_button = new wxButton(button_panel, wxID_OK, _("&OK "));
    button_panel_sizer->Add(ok_button, g_flags);

    wxButton* cancel_button = new wxButton(button_panel, wxID_CANCEL, _("&Cancel "));
    button_panel_sizer->Add(cancel_button, g_flags);
    cancel_button->SetFocus();
}

void mmAssetDialog::OnChangeAppreciationType(wxCommandEvent& /*event*/)
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

void mmAssetDialog::OnOk(wxCommandEvent& /*event*/)
{
    const wxString name = m_assetName->GetValue().Trim();
    if (name.empty()){
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

    if (!this->m_asset) this->m_asset = Model_Asset::instance().create();

    m_asset->STARTDATE        = m_dpc->GetValue().FormatISODate();
    m_asset->NOTES            = m_notes->GetValue().Trim();
    m_asset->ASSETNAME        = name;
    m_asset->VALUE            = value;
    m_asset->VALUECHANGE      = Model_Asset::all_rate()[valueChangeType];
    m_asset->VALUECHANGERATE  = valueChangeRate;
    m_asset->ASSETTYPE        = asset_type;

	int OldAssetId = m_asset->ASSETID;
    int NewAssetId = Model_Asset::instance().save(m_asset);

	if (OldAssetId < 0)
	{
		const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::ASSET);
		mmAttachmentManage::RelocateAllAttachments(RefType, 0, NewAssetId);
	}

    EndModal(wxID_OK);
}

void mmAssetDialog::OnCancel(wxCommandEvent& /*event*/)
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

void mmAssetDialog::OnQuit(wxCloseEvent& /*event*/)
{
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::ASSET);
    if (!this->m_asset)
        mmAttachmentManage::DeleteAllAttachments(RefType, 0);
    EndModal(wxID_CANCEL);
}

void mmAssetDialog::OnAttachments(wxCommandEvent& /*event*/)
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
    Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();
    if (event.GetId() == m_value->GetId())
    {
        m_value->Calculate(currency);
    }
    else if (event.GetId() == m_valueChangeRate->GetId())
    {
        mmCalculator calc;
        if (calc.is_ok(Model_Currency::fromString2Default(m_valueChangeRate->GetValue(), currency)))
            m_valueChangeRate->SetValue(wxString::Format("%.3f", calc.get_result()));
        m_valueChangeRate->SetInsertionPoint(m_valueChangeRate->GetValue().Len());
    }

    event.Skip();
}
