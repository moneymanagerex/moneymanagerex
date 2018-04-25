/*******************************************************
Copyright (C) 2017 Gabriele-V
Copyright (C) 2018 Nikolay Akimov

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

#include "mmcustomdata.h"
#include "constants.h"
#include "model/Model_CustomFieldData.h"
#include <wx/numformatter.h>
#include <wx/timectrl.h>
#include <wx/collpane.h>
#include <wx/spinctrl.h>


mmCustomData::mmCustomData()
    :wxDialog()
{
}

mmCustomData::mmCustomData(wxDialog* dialog, const wxString& ref_type, int ref_id)
    : m_ref_type(ref_type)
    , m_ref_id(ref_id)
{
    m_dialog = dialog;
    m_fields = Model_CustomField::instance()
        .find(Model_CustomField::DB_Table_CUSTOMFIELD_V1::REFTYPE(m_ref_type));
    std::sort(m_fields.begin(), m_fields.end(), SorterByDESCRIPTION());
}

bool mmCustomData::FillCustomFields(wxBoxSizer* box_sizer)
{
    wxStaticBox* static_box2 = new wxStaticBox(m_dialog, wxID_FILEDLGG, _("Custom"));
    static_box2->Hide();
    wxStaticBoxSizer* box_sizer_right = new wxStaticBoxSizer(static_box2, wxVERTICAL);
    box_sizer->Add(box_sizer_right, g_flagsExpand);

    wxScrolledWindow* custom_tab = new wxScrolledWindow(static_box2, wxID_ANY);
    wxBoxSizer *custom_sizer = new wxBoxSizer(wxVERTICAL);
    custom_tab->SetScrollbar(wxSB_VERTICAL, wxALIGN_RIGHT, 1, -1);
    custom_tab->SetSizer(custom_sizer);

    wxFlexGridSizer* grid_sizer_custom = new wxFlexGridSizer(0, 2, 0, 0);
    grid_sizer_custom->AddGrowableCol(1, 1);
    custom_sizer->Add(grid_sizer_custom, g_flagsExpand);

    for (const auto &field : m_fields)
    {
        Model_CustomFieldData::Data* fieldData = Model_CustomFieldData::instance().get(field.FIELDID, m_ref_id);
        if (!fieldData)
        {
            fieldData = Model_CustomFieldData::instance().create();
            fieldData->FIELDID = field.FIELDID;
            fieldData->REFID = m_ref_id;
            fieldData->CONTENT = Model_CustomField::getDefault(field.PROPERTIES);
            Model_CustomFieldData::instance().save(fieldData);
        }

        int controlID = ID_CUSTOMFIELD + field.FIELDID;
        wxStaticText* Description = new wxStaticText(custom_tab, wxID_STATIC, field.DESCRIPTION);
        grid_sizer_custom->Add(Description, g_flagsH);

        switch (Model_CustomField::type(field))
        {
        case Model_CustomField::STRING:
        {
            wxTextCtrl* CustomString = new wxTextCtrl(custom_tab, controlID
                , fieldData->CONTENT, wxDefaultPosition, wxDefaultSize);
            CustomString->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
            if (Model_CustomField::getAutocomplete(field.PROPERTIES))
            {
                const wxArrayString& values = Model_CustomFieldData::instance().allValue(field.FIELDID);
                CustomString->AutoComplete(values);
            }
            grid_sizer_custom->Add(CustomString, g_flagsExpand);
            break;
        }
        case Model_CustomField::INTEGER:
        {
            int value = (wxAtoi(fieldData->CONTENT)) ? wxAtoi(fieldData->CONTENT) : 0;
            wxSpinCtrl* CustomInteger = new wxSpinCtrl(custom_tab, controlID,
                wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -2147483647, 2147483647, value);
            CustomInteger->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
            grid_sizer_custom->Add(CustomInteger, g_flagsExpand);
            break;
        }
        case Model_CustomField::DECIMAL:
        {
            double value;
            if (!fieldData->CONTENT.ToDouble(&value)) value = 0;
            wxSpinCtrlDouble* CustomDecimal = new wxSpinCtrlDouble(custom_tab, controlID
                , wxEmptyString, wxDefaultPosition, wxDefaultSize
                , wxSP_ARROW_KEYS, -2147483647, 2147483647, value, 0.01);
            CustomDecimal->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
            grid_sizer_custom->Add(CustomDecimal, g_flagsExpand);
            break;
        }
        case Model_CustomField::BOOLEAN:
        {
            wxCheckBox* CustomBoolean = new wxCheckBox(custom_tab, controlID
                , wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
            CustomBoolean->SetValue((fieldData->CONTENT == "TRUE") ? TRUE : FALSE);
            CustomBoolean->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
            grid_sizer_custom->Add(CustomBoolean, g_flagsExpand);
            break;
        }
        case Model_CustomField::DATE:
        {
            wxDate value = wxDate::Today();
            if (fieldData->CONTENT.CmpNoCase("Now") == 0)
                value = wxDate::Today();
            else
                value.ParseDate(fieldData->CONTENT);

            wxDatePickerCtrl* CustomDate = new wxDatePickerCtrl(custom_tab, controlID
                , value, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN | wxDP_SHOWCENTURY);
            CustomDate->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
            grid_sizer_custom->Add(CustomDate, g_flagsExpand);
            break;
        }
        case Model_CustomField::TIME:
        {
            wxDateTime value;
            if (fieldData->CONTENT.CmpNoCase("Now") == 0)
                value = wxDateTime::Now();
            else if (!value.ParseTime(fieldData->CONTENT))
                value.ParseTime("00:00:00");
            wxTimePickerCtrl* CustomDate = new wxTimePickerCtrl(custom_tab, controlID
                , value, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
            CustomDate->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
            grid_sizer_custom->Add(CustomDate, g_flagsExpand);
            break;
        }
        case Model_CustomField::SINGLECHOICE:
        {
            wxArrayString Choices = Model_CustomField::getChoices(field.PROPERTIES);
            Choices.Sort();

            wxChoice* CustomChoice = new wxChoice(custom_tab, controlID
                , wxDefaultPosition, wxDefaultSize, Choices);
            CustomChoice->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
            grid_sizer_custom->Add(CustomChoice, g_flagsExpand);

            if (Choices.size() == 0)
                CustomChoice->Enable(false);

            CustomChoice->SetStringSelection(fieldData->CONTENT);
            break;
        }
        case Model_CustomField::MULTICHOICE:
        {
            const auto& content = fieldData->CONTENT;
            const auto& name = field.DESCRIPTION;

            wxButton* multi_choice_button = new wxButton(custom_tab, controlID, content
                , wxDefaultPosition, wxDefaultSize, 0L, wxDefaultValidator, name);
            multi_choice_button->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
            grid_sizer_custom->Add(multi_choice_button, g_flagsExpand);

            multi_choice_button->Connect(controlID, wxEVT_COMMAND_BUTTON_CLICKED
                , wxCommandEventHandler(mmCustomData::OnMultiChoice), nullptr, m_dialog);

            break;
        }
        default: break;
        }
    }

    custom_tab->FitInside();
    custom_tab->SetScrollRate(5, 5);
    box_sizer_right->Add(custom_tab, g_flagsExpand);

    return true;
}

void mmCustomData::OnMultiChoice(wxCommandEvent& event)
{
    long id = event.GetId();
    wxButton* button = (wxButton*)FindWindow(id);
    if (!button) {
        return;
    }

    const auto& name = button->GetName();
    const wxString& type = Model_CustomField::FIELDTYPE_CHOICES[Model_CustomField::MULTICHOICE].second;

    Model_CustomField::Data_Set fields = Model_CustomField::instance()
        .find(Model_CustomField::REFTYPE(/*GetRefTypr()*/ /*m_ref_type*/ "Transaction")
            , Model_CustomField::TYPE(type)
            , Model_CustomField::DESCRIPTION(name));
    wxArrayString all_choices = Model_CustomField::getChoices(fields.begin()->PROPERTIES);

    const wxString& label = button->GetLabelText();
    wxArrayInt arr_selections;
    int i = 0;
    for (const auto& entry : all_choices) {
        if (label.Contains(entry)) {
            arr_selections.Add(i);
        }
        i++;
    }

    wxString info = label;
    wxMultiChoiceDialog* MultiChoice = new wxMultiChoiceDialog(this
        , _("Please select"), _("Multi Choice"), all_choices);
    MultiChoice->SetSelections(arr_selections);

    if (MultiChoice->ShowModal() == wxID_OK)
    {
        info.clear();
        for (const auto &i : MultiChoice->GetSelections()) {
            info += all_choices[i] + ";";
        }
        info.RemoveLast();
    }

    button->SetLabel(info);
}


bool mmCustomData::SaveCustomValues()
{
    bool ok = true;

    Model_CustomFieldData::instance().Savepoint();

    for (const auto &field : m_fields)
    {
        Model_CustomFieldData::Data* fieldData = Model_CustomFieldData::instance().get(field.FIELDID, m_ref_id);

        if (!fieldData) {
            fieldData = Model_CustomFieldData::instance().create();
        }

        int controlID = ID_CUSTOMFIELD + field.FIELDID;
        wxString data = wxEmptyString;

        switch (Model_CustomField::type(field))
        {
        case Model_CustomField::STRING:
        {
            wxTextCtrl* CustomString = (wxTextCtrl*)m_dialog->FindWindow(controlID);
            if (CustomString != nullptr) data = CustomString->GetValue().Trim();
            break;
        }
        case Model_CustomField::INTEGER:
        {
            wxSpinCtrl* CustomInteger = (wxSpinCtrl*)m_dialog->FindWindow(controlID);
            if (CustomInteger) data = wxString::Format("%i", CustomInteger->GetValue());
            break;
        }
        case Model_CustomField::DECIMAL:
        {
            wxSpinCtrlDouble* CustomDecimal = (wxSpinCtrlDouble*)m_dialog->FindWindow(controlID);
            if (CustomDecimal) data = wxString::Format("%f", CustomDecimal->GetValue());
            break;
        }
        case Model_CustomField::BOOLEAN:
        {
            wxCheckBox* CustomBoolean = (wxCheckBox*)m_dialog->FindWindow(controlID);
            if (CustomBoolean) data = (CustomBoolean->GetValue()) ? "TRUE" : "FALSE";
            break;
        }
        case Model_CustomField::DATE:
        {
            wxDatePickerCtrl* CustomDate = (wxDatePickerCtrl*)m_dialog->FindWindow(controlID);
            if (CustomDate) data = CustomDate->GetValue().FormatISODate();
            break;
        }
        case Model_CustomField::TIME:
        {
            wxTimePickerCtrl* CustomTime = (wxTimePickerCtrl*)m_dialog->FindWindow(controlID);
            if (CustomTime) data = CustomTime->GetValue().FormatISOTime();
            break;
        }
        case Model_CustomField::SINGLECHOICE:
        {
            wxChoice* CustomSingleChoice = (wxChoice*)m_dialog->FindWindow(controlID);
            if (CustomSingleChoice) data = CustomSingleChoice->GetStringSelection();
            break;
        }
        case Model_CustomField::MULTICHOICE:
        {
            wxButton* CustomMultiChoice = (wxButton*)m_dialog->FindWindow(controlID);
            if (CustomMultiChoice) {
                data = CustomMultiChoice->GetLabelText();
            }
            break;
        }
        default: break;
        }

        if (!data.empty())
        {
            fieldData->FIELDID = field.FIELDID;
            fieldData->CONTENT = data;
            wxLogDebug("%s", data);
            Model_CustomFieldData::instance().save(fieldData);
        }
        else {
            ok = false;
        }
    }

    Model_CustomFieldData::instance().ReleaseSavepoint();
    return ok;
}
