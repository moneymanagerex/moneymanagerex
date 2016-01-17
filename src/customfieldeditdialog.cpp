/*******************************************************
Copyright (C) 2015 Gabriele-V

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
#include "mmtextctrl.h"
#include "paths.h"
#include "util.h"
#include "validators.h"

#include "model/Model_Attachment.h"
#include "model/Model_CustomField.h"
#include "model/Model_CustomFieldData.h"

#include <wx/valnum.h>

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
    , m_itemRegEx(nullptr)
    , m_itemAutocomplete(nullptr)
    , m_itemDefault(nullptr)
    , m_itemChoiches(nullptr)
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
    if (!this->m_field) return;

    m_itemDescription->SetValue(m_field->DESCRIPTION);
    m_itemType->SetSelection(Model_CustomField::type(m_field));
    m_itemRegEx->SetValue(Model_CustomField::getRegEx(m_field->PROPERTIES));
    m_itemAutocomplete->SetValue(Model_CustomField::getAutocomplete(m_field->PROPERTIES));
    m_itemDefault->SetValue(Model_CustomField::getDefault(m_field->PROPERTIES));

    wxString Choiches = wxEmptyString;
    wxArrayString ArrChoiches = Model_CustomField::getChoiches(m_field->PROPERTIES);
    for (int i = 0; i < ArrChoiches.size(); i++)
    {
        Choiches << ArrChoiches[i] << ";";
    }
    m_itemChoiches->SetValue(Choiches);

    wxCommandEvent evt;
    OnChangeType(evt);
}

void mmCustomFieldEditDialog::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    this->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(this, wxID_ANY, _("Custom Field Details"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static
        , wxVERTICAL);
    itemBoxSizer3->Add(itemStaticBoxSizer4, g_flags);

    wxPanel* itemPanel5 = new wxPanel(this, wxID_STATIC, wxDefaultPosition
        , wxDefaultSize, wxTAB_TRAVERSAL);
    itemStaticBoxSizer4->Add(itemPanel5, g_flags);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    itemPanel5->SetSizer(itemFlexGridSizer6);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Description")), g_flags);
    m_itemDescription = new wxTextCtrl(itemPanel5, wxID_ANY, wxGetEmptyString());
    m_itemDescription->SetToolTip(_("Enter the name of the custom field"));
    itemFlexGridSizer6->Add(m_itemDescription, g_flagsExpand);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Field Type")), g_flags);
    m_itemType = new wxChoice(itemPanel5, wxID_HIGHEST, wxDefaultPosition, wxSize(150, -1), Model_CustomField::all_type());
    m_itemType->SetToolTip(_("Select type of custom field"));
    m_itemType->SetSelection(Model_CustomField::STRING);
    itemFlexGridSizer6->Add(m_itemType, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("RegEx")), g_flags);
    m_itemRegEx = new wxTextCtrl(itemPanel5, wxID_ANY, "");
    m_itemRegEx->SetToolTip(_("Enter the RegEx to validate field"));
    itemFlexGridSizer6->Add(m_itemRegEx, g_flagsExpand);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Autocomplete")), g_flags);
    m_itemAutocomplete = new wxCheckBox(itemPanel5, wxID_STATIC, "", wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
    m_itemAutocomplete->SetValue(FALSE);
    m_itemAutocomplete->SetToolTip(_("Enables autocomplete on custom field"));
    itemFlexGridSizer6->Add(m_itemAutocomplete, g_flagsExpand);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Default")), g_flags);
    m_itemDefault = new wxTextCtrl(itemPanel5, wxID_ANY, "");
    m_itemDefault->SetToolTip(_("Enter the default for this field"));
    itemFlexGridSizer6->Add(m_itemDefault, g_flagsExpand);

    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Choiches")), g_flags);
    m_itemChoiches = new wxTextCtrl(itemPanel5, wxID_ANY, "");
    m_itemChoiches->SetToolTip(_("Enter the choiches for this field separated with a semi-colon"));
    itemFlexGridSizer6->Add(m_itemChoiches, g_flagsExpand);

    wxPanel* itemPanel27 = new wxPanel(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemBoxSizer3->Add(itemPanel27, wxSizerFlags(g_flags).Center());

    wxBoxSizer* itemBoxSizer28 = new wxBoxSizer(wxHORIZONTAL);
    itemPanel27->SetSizer(itemBoxSizer28);

    wxButton* itemButton29 = new wxButton(itemPanel27, wxID_OK, _("&OK "));
    itemBoxSizer28->Add(itemButton29, g_flags);

    wxButton* itemButton30 = new wxButton(itemPanel27, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    itemBoxSizer28->Add(itemButton30, g_flags);
    itemButton30->SetFocus();
}

void mmCustomFieldEditDialog::OnOk(wxCommandEvent& /*event*/)
{
    const wxString name = m_itemDescription->GetValue().Trim();
    if (name.empty()) {
        mmErrorDialogs::InvalidName(m_itemDescription);
        return;
    }

    if (!this->m_field)
        this->m_field = Model_CustomField::instance().create();
    else if (m_field->REFTYPE != m_fieldRefType)
    {
        auto DataSet = Model_CustomFieldData::instance().find(Model_CustomFieldData::FIELDID(m_field->FIELDID));
        if (DataSet.size() > 0)
        {
            int DeleteResponse = wxMessageBox(
                _("Changing field type will delete all content.") + "\n"
                + _("Do you want to proceed?") << "\n"
                , _("Confirm Custom Field Change")
                , wxYES_NO | wxNO_DEFAULT | wxICON_ERROR);
            if (DeleteResponse != wxYES)
                return;

            Model_CustomFieldData::instance().Savepoint();
            for (auto &data : DataSet)
            {
                data.CONTENT = wxEmptyString;
            }
            Model_CustomFieldData::instance().save(DataSet);
            Model_CustomFieldData::instance().ReleaseSavepoint();
        }
    }

    wxArrayString ArrChoiches;
    wxString Choiches = m_itemChoiches->GetValue();
    wxStringTokenizer token(Choiches, ";");
    while (token.HasMoreTokens())
    {
        ArrChoiches.Add(token.GetNextToken());
    }

    m_field->REFTYPE = m_fieldRefType;
    m_field->DESCRIPTION = name;
    m_field->TYPE = m_itemType->GetStringSelection();
    m_field->PROPERTIES = Model_CustomField::formatProperties(
        m_itemRegEx->GetValue(),
        m_itemAutocomplete->GetValue(),
        m_itemDefault->GetValue(),
        ArrChoiches
        );
    
    Model_CustomField::instance().save(m_field);
    EndModal(wxID_OK);
}

void mmCustomFieldEditDialog::OnCancel(wxCommandEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void mmCustomFieldEditDialog::OnQuit(wxCloseEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

void mmCustomFieldEditDialog::OnChangeType(wxCommandEvent& /*event*/)
{
    switch (m_itemType->GetSelection())
    {
    case Model_CustomField::STRING:
        {
            m_itemRegEx->Enable(false);
            m_itemAutocomplete->Enable(true);
            m_itemChoiches->Enable(false);
            m_itemChoiches->SetValue(wxEmptyString);
        }
        break;
    case Model_CustomField::SINGLECHOICHE:
        {
            m_itemRegEx->Enable(false);
            m_itemAutocomplete->Enable(false);
            m_itemAutocomplete->SetValue(false);
            m_itemChoiches->Enable(true);
        }
        break;
    default:
        {
            m_itemRegEx->Enable(false);
            m_itemAutocomplete->Enable(false);
            m_itemAutocomplete->SetValue(false);
            m_itemChoiches->Enable(false);
            m_itemChoiches->SetValue(wxEmptyString);
        }
        break;
    }
}