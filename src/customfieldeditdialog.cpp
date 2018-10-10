/*******************************************************
Copyright (C) 2016 Gabriele-V

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

#include "customfieldeditdialog.h"
#include "attachmentdialog.h"
#include "constants.h"
#include "images_list.h"
#include "mmSimpleDialogs.h"
#include "mmTextCtrl.h"
#include "paths.h"
#include "util.h"

#include "Model_Attachment.h"
#include "Model_CustomField.h"
#include "Model_CustomFieldData.h"

#include <wx/spinctrl.h>

wxIMPLEMENT_DYNAMIC_CLASS(mmCustomFieldEditDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmCustomFieldEditDialog, wxDialog)
    EVT_BUTTON(wxID_OK, mmCustomFieldEditDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, mmCustomFieldEditDialog::OnCancel)
    EVT_CHOICE(wxID_HIGHEST, mmCustomFieldEditDialog::OnChangeType)
    EVT_CLOSE(mmCustomFieldEditDialog::OnQuit)
wxEND_EVENT_TABLE()

mmCustomFieldEditDialog::mmCustomFieldEditDialog(wxWindow* parent, Model_CustomField::Data* field, const wxString& fieldRefType)
    : m_field(field)
    , m_fieldRefType(fieldRefType)
    , m_itemDescription(nullptr)
    , m_itemType(nullptr)
    , m_itemUDFC(nullptr)
    , m_itemTooltip(nullptr)
    , m_itemRegEx(nullptr)
    , m_itemAutocomplete(nullptr)
    , m_itemDefault(nullptr)
    , m_itemChoices(nullptr)
    , m_itemDigitScale(nullptr)
{
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, _("New/Edit Custom Field"), wxDefaultPosition, wxSize(400, 300), style);
}

bool mmCustomFieldEditDialog::Create(wxWindow* parent
    , wxWindowID id
    , const wxString& caption
    , const wxPoint& pos
    , const wxSize& size
    , long style)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

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

void mmCustomFieldEditDialog::dataToControls()
{
    if (this->m_field)
    {
        m_itemDescription->SetValue(m_field->DESCRIPTION);
        m_itemType->SetSelection(Model_CustomField::type(m_field));
        m_itemTooltip->SetValue(Model_CustomField::getTooltip(m_field->PROPERTIES));
        m_itemRegEx->SetValue(Model_CustomField::getRegEx(m_field->PROPERTIES));
        m_itemAutocomplete->SetValue(Model_CustomField::getAutocomplete(m_field->PROPERTIES));
        m_itemDefault->SetValue(Model_CustomField::getDefault(m_field->PROPERTIES));
        m_itemDigitScale->SetValue(Model_CustomField::getDigitScale(m_field->PROPERTIES));
        m_itemUDFC->SetStringSelection(Model_CustomField::getUDFC(m_field->PROPERTIES));


        wxString choices = wxEmptyString;
        for (const auto& arrChoices : Model_CustomField::getChoices(m_field->PROPERTIES))
        {
            choices << arrChoices << ";";
        }
        choices.RemoveLast(1);
        m_itemChoices->ChangeValue(choices);
    }
    else
    {
        m_itemType->SetSelection(Model_CustomField::STRING);
    }
    wxCommandEvent evt;
    OnChangeType(evt, true);
}

void mmCustomFieldEditDialog::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    this->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(this, wxID_ANY, _("Custom Field Details"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    itemBoxSizer3->Add(itemStaticBoxSizer4, g_flagsV);

    wxPanel* itemPanel5 = new wxPanel(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemStaticBoxSizer4->Add(itemPanel5, g_flagsV);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    itemPanel5->SetSizer(itemFlexGridSizer6);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Name")), g_flagsH);
    m_itemDescription = new wxTextCtrl(itemPanel5, wxID_ANY, wxGetEmptyString());
    m_itemDescription->SetToolTip(_("Enter the name of the custom field"));
    itemFlexGridSizer6->Add(m_itemDescription, g_flagsExpand);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Field Type")), g_flagsH);
    m_itemType = new wxChoice(itemPanel5, wxID_HIGHEST, wxDefaultPosition, wxSize(150, -1));
    for (const auto& type : Model_CustomField::all_type())
        m_itemType->Append(wxGetTranslation(type), new wxStringClientData(type));
    m_itemType->SetToolTip(_("Select type of custom field"));
    itemFlexGridSizer6->Add(m_itemType, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("ToolTip")), g_flagsH);
    m_itemTooltip = new wxTextCtrl(itemPanel5, wxID_ANY, "");
    m_itemTooltip->SetToolTip(_("Enter the tooltip that will be shown"));
    itemFlexGridSizer6->Add(m_itemTooltip, g_flagsExpand);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("RegEx")), g_flagsH);
    m_itemRegEx = new wxTextCtrl(itemPanel5, wxID_ANY, "");
    m_itemRegEx->SetToolTip(_("Enter the RegEx to validate field"));
    itemFlexGridSizer6->Add(m_itemRegEx, g_flagsExpand);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Autocomplete")), g_flagsH);
    m_itemAutocomplete = new wxCheckBox(itemPanel5, wxID_STATIC, "", wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_itemAutocomplete->SetValue(FALSE);
    m_itemAutocomplete->SetToolTip(_("Enables autocomplete on custom field"));
    itemFlexGridSizer6->Add(m_itemAutocomplete, g_flagsExpand);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Default")), g_flagsH);
    m_itemDefault = new wxTextCtrl(itemPanel5, wxID_ANY, "");
    m_itemDefault->SetToolTip(_("Enter the default for this field\n"
        "For date/time insert 'Now' to make current date/time as default"));
    itemFlexGridSizer6->Add(m_itemDefault, g_flagsExpand);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Choices")), g_flagsH);
    m_itemChoices = new wxTextCtrl(itemPanel5, wxID_ANY, "");
    m_itemChoices->SetToolTip(_("Enter the choices for this field separated with a semicolon"));
    itemFlexGridSizer6->Add(m_itemChoices, g_flagsExpand);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Digits scale")), g_flagsH);
    m_itemDigitScale = new wxSpinCtrl(itemPanel5, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 20, 0);
    m_itemDigitScale->SetToolTip(_("Enter the decimal digits scale allowed"));
    itemFlexGridSizer6->Add(m_itemDigitScale, g_flagsExpand);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Panel's column")), g_flagsH);
    m_itemUDFC = new wxChoice(itemPanel5, wxID_APPLY, wxDefaultPosition, wxSize(150, -1));
    for (const auto& type : Model_CustomField::getUDFCList(m_field)) {
        m_itemUDFC->Append(wxGetTranslation(type), new wxStringClientData(type));
    }
    m_itemUDFC->SetToolTip(_("Select a value to represent the item on a panel"));
    itemFlexGridSizer6->Add(m_itemUDFC, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxPanel* itemPanel27 = new wxPanel(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemBoxSizer3->Add(itemPanel27, wxSizerFlags(g_flagsV).Center());

    wxBoxSizer* itemBoxSizer28 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel27->SetSizer(itemBoxSizer28);

    wxButton* itemButton29 = new wxButton(itemPanel27, wxID_OK, _("&OK "));
    itemBoxSizer28->Add(itemButton29, g_flagsH);

    wxButton* itemButton30 = new wxButton(itemPanel27, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    itemBoxSizer28->Add(itemButton30, g_flagsH);
    itemButton30->SetFocus();
}

void mmCustomFieldEditDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    const wxString name = m_itemDescription->GetValue().Trim();
    if (name.empty()) {
        return mmErrorDialogs::InvalidName(m_itemDescription);
    }

    wxArrayString ArrChoices;
    wxString Choices = m_itemChoices->GetValue();
    wxStringTokenizer token(Choices, ";");
    while (token.HasMoreTokens())
    {
        ArrChoices.Add(token.GetNextToken());
    }

    if (!this->m_field)
    {
        this->m_field = Model_CustomField::instance().create();
    }
    else if (m_field->TYPE != Model_CustomField::fieldtype_desc(m_itemType->GetSelection()))
    {
        auto DataSet = Model_CustomFieldData::instance().find(Model_CustomFieldData::FIELDID(m_field->FIELDID));
        if (DataSet.size() > 0)
        {
            int DeleteResponse = wxMessageBox(
                _("Changing field type will delete all content!") + "\n"
                + _("Do you want to proceed?") << "\n"
                , _("Custom Field Change")
                , wxYES_NO | wxNO_DEFAULT | wxICON_ERROR);
            if (DeleteResponse != wxYES)
                return;

            Model_CustomFieldData::instance().Savepoint();
            for (auto &data : DataSet)
            {
                Model_CustomFieldData::instance().remove(data.id());
            }
            Model_CustomFieldData::instance().save(DataSet);
            Model_CustomFieldData::instance().ReleaseSavepoint();
        }
    }
    else if (Model_CustomField::getChoices(m_field->PROPERTIES) != ArrChoices)
    {
        auto DataSet = Model_CustomFieldData::instance().find(Model_CustomFieldData::FIELDID(m_field->FIELDID));
        if (DataSet.size() > 0)
        {
            int DeleteResponse = wxMessageBox(
                _("You have modified choices: ones removed will be cleaned!") + "\n"
                + _("Do you want to proceed?") << "\n"
                , _("Custom Field Change")
                , wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION);
            if (DeleteResponse != wxYES)
                return;

            Model_CustomFieldData::instance().Savepoint();
            for (auto &data : DataSet)
            {
                if(ArrChoices.Index(data.CONTENT) == wxNOT_FOUND)
                    Model_CustomFieldData::instance().remove(data.id());
            }
            Model_CustomFieldData::instance().save(DataSet);
            Model_CustomFieldData::instance().ReleaseSavepoint();
        }
    }


    m_field->REFTYPE = m_fieldRefType;
    m_field->DESCRIPTION = name;
    m_field->TYPE = Model_CustomField::fieldtype_desc(m_itemType->GetSelection());
    m_field->PROPERTIES = Model_CustomField::formatProperties(
        m_itemTooltip->GetValue(),
        m_itemRegEx->GetValue(),
        m_itemAutocomplete->GetValue(),
        m_itemDefault->GetValue(),
        ArrChoices,
        m_itemDigitScale->GetValue(),
        m_itemUDFC->GetString(m_itemUDFC->GetSelection())
        );
    
    Model_CustomField::instance().save(m_field);
    EndModal(wxID_OK);
}

void mmCustomFieldEditDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_CANCEL);
}

void mmCustomFieldEditDialog::OnQuit(wxCloseEvent& WXUNUSED(event))
{
    EndModal(wxID_CANCEL);
}

void mmCustomFieldEditDialog::OnChangeType(wxCommandEvent& event)
{
    OnChangeType(event, false);
}


void mmCustomFieldEditDialog::OnChangeType(wxCommandEvent& WXUNUSED(event), bool OnDataToControls)
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
    case Model_CustomField::STRING:
    {
        m_itemDefault->Enable(true);
        m_itemRegEx->Enable(true);
        m_itemAutocomplete->Enable(true);
        break;
    }
    case Model_CustomField::SINGLECHOICE:
    {
        m_itemChoices->Enable(true);
        m_itemDefault->Enable(true);
        break;
    }
    case Model_CustomField::MULTICHOICE:
    {
        m_itemChoices->Enable(true);
        break;
    }
    case Model_CustomField::INTEGER:
    {
        m_itemDefault->Enable(true);
        m_itemRegEx->Enable(true);
        break;
    }
    case Model_CustomField::DECIMAL:
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
