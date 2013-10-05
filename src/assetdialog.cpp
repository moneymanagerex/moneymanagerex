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
#include "paths.h"
#include "util.h"
#include "mmCurrencyFormatter.h"
#include "validators.h"
#include "model/Model_Asset.h"
#include <wx/valnum.h>

namespace
{

enum
{
    IDC_COMBO_TYPE = wxID_HIGHEST + 1,
    IDC_NOTES,
};

} // namespace


IMPLEMENT_DYNAMIC_CLASS( mmAssetDialog, wxDialog )

BEGIN_EVENT_TABLE( mmAssetDialog, wxDialog )
    EVT_BUTTON(wxID_OK, mmAssetDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, mmAssetDialog::OnCancel)
    EVT_CHOICE(IDC_COMBO_TYPE, mmAssetDialog::OnChangeAppreciationType)
    EVT_CHILD_FOCUS(mmAssetDialog::changeFocus)
END_EVENT_TABLE()

mmAssetDialog::mmAssetDialog()
{}

mmAssetDialog::mmAssetDialog(wxWindow* parent, Model_Asset::Data* asset)
    : m_asset(asset)
{
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;

    Create(parent, wxID_ANY, _("New/Edit Asset"), wxDefaultPosition, wxSize(400, 300), style);
}

bool mmAssetDialog::Create(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
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

    wxString valueChangeTypeStr = m_asset->VALUECHANGE;
    m_valueChange->SetStringSelection(wxGetTranslation(m_asset->VALUECHANGE));
    enableDisableRate(Model_Asset::rate(m_asset) != Model_Asset::RATE_NONE);
    m_assetType->SetStringSelection(wxGetTranslation(m_asset->ASSETTYPE));
}

void mmAssetDialog::CreateControls()
{
    wxSizerFlags flags, flagsExpand;
    flags.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 5);
    flagsExpand.Align(wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL).Border(wxALL, 5).Expand();

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    this->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(this, wxID_ANY, _("Asset Details"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static,
        wxVERTICAL);
    itemBoxSizer3->Add(itemStaticBoxSizer4, flags);

    wxPanel* itemPanel5 = new wxPanel( this, wxID_STATIC, wxDefaultPosition,
        wxDefaultSize, wxTAB_TRAVERSAL );
    itemStaticBoxSizer4->Add(itemPanel5, flags);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    itemPanel5->SetSizer(itemFlexGridSizer6);

    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Name")), flags);

    m_assetName = new mmTextCtrl( itemPanel5, wxID_ANY, wxGetEmptyString(), wxDefaultPosition, wxDefaultSize, 0 );
    m_assetName->SetToolTip(_("Enter the name of the asset"));
    itemFlexGridSizer6->Add(m_assetName, flagsExpand);

    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Date")), flags);

    m_dpc = new wxDatePickerCtrl( itemPanel5, wxID_ANY, wxDefaultDateTime,
              wxDefaultPosition, wxSize(120, -1), wxDP_DROPDOWN|wxDP_SHOWCENTURY);
    itemFlexGridSizer6->Add(m_dpc, flags);
    m_dpc->SetToolTip(_("Specify the date of purchase of asset"));

    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Asset Type")), flags);

    m_assetType = new wxChoice( itemPanel5, wxID_STATIC, wxDefaultPosition, wxSize(150,-1));
    for (const auto& a : Model_Asset::all_type())
        m_assetType->Append(wxGetTranslation(a), new wxStringClientData(a));

    m_assetType->SetToolTip(_("Select type of asset"));
    m_assetType->SetSelection(Model_Asset::TYPE_PROPERTY);
    itemFlexGridSizer6->Add(m_assetType, 0,
        wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, 5);

    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Value")), flags);

    m_value = new mmTextCtrl(itemPanel5, wxID_ANY, wxGetEmptyString()
        , wxDefaultPosition, wxSize(150,-1), wxALIGN_RIGHT|wxTE_PROCESS_ENTER
        , mmCalcValidator() );
    m_value->SetToolTip(_("Enter the current value of the asset"));
    itemFlexGridSizer6->Add(m_value, flags);
    m_value->Connect(wxID_ANY, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmAssetDialog::onTextEntered), NULL, this);

    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Change in Value")), flags);

    m_valueChange = new wxChoice( itemPanel5, IDC_COMBO_TYPE, wxDefaultPosition, wxSize(150,-1));
    for(const auto& a : Model_Asset::all_rate())
        m_valueChange->Append(wxGetTranslation(a), new wxStringClientData(a));

    m_valueChange->SetToolTip(_("Specify if the value of the asset changes over time"));
    m_valueChange->SetSelection(Model_Asset::RATE_NONE);
    itemFlexGridSizer6->Add(m_valueChange, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    m_valueChangeRateLabel = new wxStaticText( itemPanel5, wxID_STATIC, _("% Rate"));
    itemFlexGridSizer6->Add(m_valueChangeRateLabel, flags);

    m_valueChangeRate = new mmTextCtrl( itemPanel5, wxID_STATIC, wxGetEmptyString()
        , wxDefaultPosition, wxSize(150,-1), wxALIGN_RIGHT|wxTE_PROCESS_ENTER
        , mmDoubleValidator() );
    m_valueChangeRate->SetToolTip(_("Enter the rate at which the asset changes its value in % per year"));
    itemFlexGridSizer6->Add(m_valueChangeRate, flags);
    enableDisableRate(false);

    itemFlexGridSizer6->Add(new wxStaticText( itemPanel5, wxID_STATIC, _("Notes")), flags);

    itemFlexGridSizer6->AddSpacer(1);

    m_notes = new mmTextCtrl( this, IDC_NOTES, wxGetEmptyString(), wxDefaultPosition, wxSize(220, 170), wxTE_MULTILINE );
    m_notes->SetToolTip(_("Enter notes associated with this asset"));
    itemStaticBoxSizer4->Add(m_notes, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 10);

    wxPanel* itemPanel27 = new wxPanel( this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
    itemBoxSizer3->Add(itemPanel27, flags.Right());

    wxBoxSizer* itemBoxSizer28 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel27->SetSizer(itemBoxSizer28);

    wxButton* itemButton29 = new wxButton( itemPanel27, wxID_OK);
    itemBoxSizer28->Add(itemButton29, flags);

    wxButton* itemButton30 = new wxButton( itemPanel27, wxID_CANCEL);
    itemBoxSizer28->Add(itemButton30, flags);
    itemButton30->SetFocus();
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
    wxString valueStr = m_value->GetValue().Trim();
    if (valueStr.IsEmpty())
    {
        wxMessageBox(_("Value"), _("Invalid Entry"), wxOK|wxICON_ERROR);
        return;
    }
    double value = 0;
    if (! CurrencyFormatter::formatCurrencyToDouble(valueStr, value))
    {
        wxMessageBox(_("Invalid Value "), _("Invalid Entry"), wxOK|wxICON_ERROR);
        return;
    }

    int valueChangeType = m_valueChange->GetSelection();
    wxString valueChangeTypeStr= "";
    wxStringClientData* value_change_obj = (wxStringClientData *)m_valueChange->GetClientObject(m_valueChange->GetSelection());
    if (value_change_obj) valueChangeTypeStr = value_change_obj->GetData();

    wxString valueChangeRateStr = m_valueChangeRate->GetValue().Trim();
    if (valueChangeRateStr.IsEmpty() && valueChangeType != Model_Asset::RATE_NONE)
    {
        wxMessageBox(_("Rate of Change in Value"), _("Invalid Entry"), wxOK|wxICON_ERROR);
        return;
    }
    double valueChangeRate = 0;
    if(!valueChangeRateStr.ToDouble(&valueChangeRate))
    {
        valueChangeRate = -1.0;
    }
    //This should be unnecessary with hidden controls
    if ((valueChangeType != Model_Asset::RATE_NONE) && (valueChangeRate < 0.0))
    {
        wxMessageBox(_("Invalid Value "), _("Invalid Entry"), wxOK|wxICON_ERROR);
        return;
    }

    wxString asset_type = "";
    wxStringClientData* type_obj = (wxStringClientData *)m_assetType->GetClientObject(m_assetType->GetSelection());
    if (type_obj) asset_type = type_obj->GetData();

    if (!this->m_asset) this->m_asset = Model_Asset::instance().create();

    m_asset->STARTDATE        = m_dpc->GetValue().FormatISODate();
    m_asset->NOTES            = m_notes->GetValue().Trim();
    m_asset->ASSETNAME        = m_assetName->GetValue().Trim();
    m_asset->VALUE            = value;
    m_asset->VALUECHANGE      = valueChangeTypeStr;
    m_asset->VALUECHANGERATE  = valueChangeRate;
    m_asset->ASSETTYPE        = asset_type;

    Model_Asset::instance().save(m_asset);

    EndModal(wxID_OK);
}

void mmAssetDialog::OnCancel(wxCommandEvent& /*event*/)
{
    if (assetRichText)
        return;
    else
        EndModal(wxID_CANCEL);
}

void mmAssetDialog::changeFocus(wxChildFocusEvent& event)
{
    wxWindow *w = event.GetWindow();
    if ( w ) assetRichText = (w->GetId() == IDC_NOTES ? true : false);
}

void mmAssetDialog::onTextEntered(wxCommandEvent& event)
{
    wxString sAmount = "";

    if (mmCalculator(m_value->GetValue(), sAmount))
        m_value->SetValue(sAmount);
    m_value->SetInsertionPoint(m_value->GetValue().Len());

    event.Skip();
}
