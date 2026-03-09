/*******************************************************
Copyright (C) 2016 Gabriele-V
Copyright (C) 2020 - 2022 Nikolay Akimov
Copyright (C) 2025  Mark Whalley (mark@ipx.co.uk)


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
#include <wx/spinctrl.h>

#include "base/constants.h"
#include "base/paths.h"
#include "base/images_list.h"
#include "util/_util.h"
#include "util/_simple.h"
#include "util/mmTextCtrl.h"

#include "FieldDialog.h"
#include "AttachmentDialog.h"

#include "model/FieldModel.h"
#include "model/FieldValueModel.h"

wxIMPLEMENT_DYNAMIC_CLASS(FieldDialog, wxDialog);

wxBEGIN_EVENT_TABLE(FieldDialog, wxDialog)
    EVT_BUTTON(wxID_OK, FieldDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, FieldDialog::OnCancel)
    EVT_CHOICE(wxID_HIGHEST, FieldDialog::OnChangeType)
    EVT_CLOSE(FieldDialog::OnQuit)
wxEND_EVENT_TABLE()

FieldDialog::FieldDialog(wxWindow* parent, FieldData* field) :
    m_field_n(field),
    m_ref_type(TrxModel::s_ref_type)
{
    this->SetFont(parent->GetFont());
    Create(parent);
    Fit();
}

bool FieldDialog::Create(
    wxWindow* parent,
    wxWindowID id,
    const wxString& caption,
    const wxPoint& pos,
    const wxSize& size,
    long style
) {
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    if (!wxDialog::Create(parent, id, caption, pos, size, style))
        return false;

    CreateControls();
    dataToControls();
    this->SetInitialSize();
    GetSizer()->SetSizeHints(this);
    SetIcon(mmex::getProgramIcon());
    Centre();
    mmThemeAutoColour(this);
    return true;
}

void FieldDialog::dataToControls()
{
    if (m_field_n) {
        m_itemDescription->SetValue(m_field_n->m_description);
        m_itemType->SetSelection(m_field_n->m_type_n.id_n());
        m_itemReference->SetSelection(m_field_n->m_ref_type.id_n());
        m_itemTooltip->SetValue(FieldModel::getTooltip(m_field_n->m_properties));
        m_itemRegEx->SetValue(FieldModel::getRegEx(m_field_n->m_properties));
        m_itemAutocomplete->SetValue(FieldModel::getAutocomplete(m_field_n->m_properties));
        m_itemDefault->SetValue(FieldModel::getDefault(m_field_n->m_properties));
        m_itemDigitScale->SetValue(FieldModel::getDigitScale(m_field_n->m_properties));
        m_itemUDFC->SetStringSelection(FieldModel::getUDFC(m_field_n->m_properties));


        wxString choices = wxEmptyString;
        for (const auto& arrChoices : FieldModel::getChoices(m_field_n->m_properties)) {
            choices += (choices.empty() ? "": ";") + arrChoices;
        }
        m_itemChoices->ChangeValue(choices);
    }
    else {
        m_itemReference->SetSelection(m_ref_type.id_n());
        m_itemType->SetSelection(FieldTypeN::e_string);
        m_itemUDFC->SetSelection(0);
    }
    wxCommandEvent evt;
    OnChangeType(evt, true);
}

void FieldDialog::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    this->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, g_flagsExpand);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(this, wxID_ANY, _t("Custom Field Details"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    itemBoxSizer3->Add(itemStaticBoxSizer4, g_flagsExpand);

    wxPanel* itemPanel5 = new wxPanel(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemStaticBoxSizer4->Add(itemPanel5, g_flagsExpand);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    itemFlexGridSizer6->AddGrowableCol(1, 1);
    itemPanel5->SetSizer(itemFlexGridSizer6);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _t("Attribute of")), g_flagsH);
    m_itemReference = new wxChoice(itemPanel5, wxID_HIGHEST);
    for (int type_id = 0; type_id < RefTypeN::size; ++type_id) {
        if (RefTypeN::field_id_n(type_id) != type_id)
            continue;
        wxString type_name = RefTypeN(type_id).name_n();
        m_itemReference->Append(
            wxGetTranslation(type_name),
            new wxStringClientData(type_name)
        );
    }
    mmToolTip(m_itemReference, _t("Select the item that the custom field is associated with"));
    itemFlexGridSizer6->Add(m_itemReference, g_flagsExpand);
    m_itemReference->Enable(false);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _t("Name")), g_flagsExpand);
    m_itemDescription = new wxTextCtrl(itemPanel5, wxID_ANY);
    m_itemDescription->SetMinSize(wxSize(150, -1));

    mmToolTip(m_itemDescription, _t("Enter the name of the custom field"));
    itemFlexGridSizer6->Add(m_itemDescription, g_flagsExpand);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _t("Field Type")), g_flagsH);
    m_itemType = new wxChoice(itemPanel5, wxID_HIGHEST);
    for (int type_id = 0; type_id < FieldTypeN::size; ++type_id) {
        wxString type_name = FieldTypeN(type_id).name_n();
        m_itemType->Append(
            wxGetTranslation(type_name),
            new wxStringClientData(type_name)
        );
    }
    mmToolTip(m_itemType, _t("Select the custom field type"));
    itemFlexGridSizer6->Add(m_itemType, g_flagsExpand);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _t("Tooltip")), g_flagsH);
    m_itemTooltip = new wxTextCtrl(itemPanel5, wxID_ANY, "");
    mmToolTip(m_itemTooltip, _t("Enter the tooltip that will be shown"));
    itemFlexGridSizer6->Add(m_itemTooltip, g_flagsExpand);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _t("RegEx")), g_flagsH);
    m_itemRegEx = new wxTextCtrl(itemPanel5, wxID_ANY, R"(^.+$)");
    mmToolTip(m_itemRegEx, _t("Enter the RegEx to validate field"));
    itemFlexGridSizer6->Add(m_itemRegEx, g_flagsExpand);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _t("Autocomplete")), g_flagsH);
    m_itemAutocomplete = new wxCheckBox(itemPanel5, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_itemAutocomplete->SetValue(FALSE);
    mmToolTip(m_itemAutocomplete, _t("Enables autocomplete on custom field"));
    itemFlexGridSizer6->Add(m_itemAutocomplete, g_flagsExpand);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _t("Default")), g_flagsH);
    m_itemDefault = new wxTextCtrl(itemPanel5, wxID_ANY, "");
    mmToolTip(m_itemDefault, _t("Enter the default for this field"));
    itemFlexGridSizer6->Add(m_itemDefault, g_flagsExpand);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _t("Choices")), g_flagsH);
    m_itemChoices = new wxTextCtrl(itemPanel5, wxID_ANY, "");
    mmToolTip(m_itemChoices, _t("Enter the choices for this field separated with a semicolon"));
    itemFlexGridSizer6->Add(m_itemChoices, g_flagsExpand);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _t("Digits scale")), g_flagsH);
    m_itemDigitScale = new wxSpinCtrl(itemPanel5, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 20, 0);
    mmToolTip(m_itemDigitScale, _t("Enter the decimal digits scale allowed"));
    itemFlexGridSizer6->Add(m_itemDigitScale, g_flagsExpand);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _t("Panel's column")), g_flagsH);
    m_itemUDFC = new wxChoice(itemPanel5, wxID_APPLY);
    for (const auto& udfc : FieldModel::instance().get_data_udfc_a(m_field_n)) {
        m_itemUDFC->Append(wxGetTranslation(udfc), new wxStringClientData(udfc));
    }
    mmToolTip(m_itemUDFC, _t("Select a value to represent the item on a panel"));
    itemFlexGridSizer6->Add(m_itemUDFC, g_flagsExpand);

    wxPanel* itemPanel27 = new wxPanel(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemBoxSizer3->Add(itemPanel27, wxSizerFlags(g_flagsV).Center());

    wxBoxSizer* itemBoxSizer28 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel27->SetSizer(itemBoxSizer28);

    wxButton* itemButton29 = new wxButton(itemPanel27, wxID_OK, _t("&OK "));
    itemBoxSizer28->Add(itemButton29, g_flagsH);

    wxButton* itemButton30 = new wxButton(itemPanel27, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    itemBoxSizer28->Add(itemButton30, g_flagsH);
    itemButton30->SetFocus();
}

void FieldDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    const wxString name = m_itemDescription->GetValue().Trim();
    if (name.empty()) {
        return mmErrorDialogs::InvalidName(m_itemDescription);
    }

    wxArrayString ArrChoices;
    wxString Choices = m_itemChoices->GetValue();
    wxStringTokenizer token(Choices, ";");
    while (token.HasMoreTokens()) {
        ArrChoices.Add(token.GetNextToken());
    }

    int itemType = m_itemType->GetSelection();
    if (ArrChoices.IsEmpty() && (
        itemType == FieldTypeN::e_single_choice ||
        itemType == FieldTypeN::e_multi_choice)
    ) {
        return mmErrorDialogs::ToolTip4Object(m_itemChoices, _t("Empty value"), _t("Choices"));
    }

    if (!m_field_n) {
        m_field_d = FieldData();
        m_field_n = &m_field_d;
    }
    else if (m_field_n->m_type_n.id_n() != m_itemType->GetSelection()) {
        auto fv_a = FieldValueModel::instance().find(
            FieldValueCol::FIELDID(m_field_n->m_id)
        );
        if (fv_a.size() > 0) {
            int DeleteResponse = wxMessageBox(
                _t("Changing field type will delete all content!") + "\n"
                    + _t("Do you want to continue?") << "\n",
                _t("Custom Field Change"),
                wxYES_NO | wxNO_DEFAULT | wxICON_ERROR
            );
            if (DeleteResponse != wxYES)
                return;

            // CHECK: What is the intention behind removing and adding back all items?
            // Is removal of CONTENT missing?
            FieldValueModel::instance().db_savepoint();
            for (auto& fv_d : fv_a) {
                FieldValueModel::instance().purge_id(fv_d.id());
            }
            FieldValueModel::instance().save_data_a(fv_a);
            FieldValueModel::instance().db_release_savepoint();
        }
    }
    else if (FieldModel::getChoices(m_field_n->m_properties) != ArrChoices) {
        auto fv_a = FieldValueModel::instance().find(
            FieldValueCol::FIELDID(m_field_n->m_id)
        );
        if (fv_a.size() > 0) {
            int DeleteResponse = wxMessageBox(
                _t("Modified choices available: ones removed will be cleaned!") + "\n"
                    + _t("Do you want to continue?") << "\n",
                _t("Custom Field Change"),
                wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION
            );
            if (DeleteResponse != wxYES)
                return;

            FieldValueModel::instance().db_savepoint();
            for (auto& fv_d : fv_a) {
                if (ArrChoices.Index(fv_d.m_content) == wxNOT_FOUND)
                    FieldValueModel::instance().purge_id(fv_d.id());
            }
            FieldValueModel::instance().save_data_a(fv_a);
            FieldValueModel::instance().db_release_savepoint();
        }
    }

    const wxString regexp = m_itemRegEx->GetValue();
    if (!regexp.empty()) {
        wxRegEx pattern(regexp);
        if (!pattern.IsValid())
            return;
    }

    m_field_n->m_ref_type = m_ref_type;
    m_field_n->m_description = name;
    m_field_n->m_type_n = FieldTypeN(m_itemType->GetSelection());
    m_field_n->m_properties = FieldModel::formatProperties(
        m_itemTooltip->GetValue(),
        regexp,
        m_itemAutocomplete->GetValue(),
        m_itemDefault->GetValue(),
        ArrChoices,
        m_itemDigitScale->GetValue(),
        m_itemUDFC->GetString(m_itemUDFC->GetSelection())
    );
    FieldModel::instance().unsafe_save_data_n(m_field_n);
    EndModal(wxID_OK);
}

void FieldDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_CANCEL);
}

void FieldDialog::OnQuit(wxCloseEvent& WXUNUSED(event))
{
    EndModal(wxID_CANCEL);
}

void FieldDialog::OnChangeType(wxCommandEvent& event)
{
    OnChangeType(event, false);
}


void FieldDialog::OnChangeType(wxCommandEvent& WXUNUSED(event), bool OnDataToControls)
{
    //Disable everything
    m_itemRegEx->Enable(false);
    m_itemAutocomplete->Enable(false);
    m_itemDefault->Enable(false);
    m_itemChoices->Enable(false);
    m_itemDigitScale->Enable(false);

    //Reset if not OnDataToControls
    if (!OnDataToControls)
    {
        m_itemRegEx->SetValue(wxEmptyString);
        m_itemAutocomplete->SetValue(false);
        m_itemDefault->SetValue(wxEmptyString);
        m_itemChoices->SetValue(wxEmptyString);
        m_itemDigitScale->SetValue(0);
    }

    //Enable specific fields
    switch (m_itemType->GetSelection())
    {
    case FieldTypeN::e_string:
    {
        m_itemDefault->Enable(true);
        m_itemRegEx->Enable(true);
        m_itemAutocomplete->Enable(true);
        break;
    }
    case FieldTypeN::e_single_choice:
    {
        m_itemChoices->Enable(true);
        m_itemDefault->Enable(true);
        break;
    }
    case FieldTypeN::e_multi_choice:
    {
        m_itemChoices->Enable(true);
        break;
    }
    case FieldTypeN::e_integer:
    {
        m_itemDefault->Enable(true);
        m_itemRegEx->Enable(true);
        break;
    }
    case FieldTypeN::e_decimal:
    {
        m_itemDefault->Enable(true);
        m_itemRegEx->Enable(true);
        m_itemDigitScale->Enable(true);
        break;
    }
    default:
    {
        break;
    }
    }
}
