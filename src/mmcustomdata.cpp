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
#include "model/Model_Attachment.h"
#include <wx/numformatter.h>
#include <wx/timectrl.h>
#include <wx/collpane.h>
#include <wx/spinctrl.h>

mmCustomData::mmCustomData()
    :wxDialog()
{
}

mmCustomData::mmCustomData(wxDialog* dialog, const wxString& ref_type, int ref_id)
    : wxDialog()
    , m_ref_type(ref_type)
    , m_ref_id(ref_id)
{
    m_dialog = dialog;
    m_fields = Model_CustomField::instance()
        .find(Model_CustomField::DB_Table_CUSTOMFIELD_V1::REFTYPE(m_ref_type));
    std::sort(m_fields.begin(), m_fields.end(), SorterByDESCRIPTION());
    m_data_changed.clear();
}

mmCustomDataTransaction::mmCustomDataTransaction(wxDialog* dialog, int ref_id, wxWindowID base_id)
    : mmCustomData(dialog
        , Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION)
        , ref_id)
{
    SetBaseID(base_id);
}

bool mmCustomData::FillCustomFields(wxBoxSizer* box_sizer)
{
    wxStaticBox* static_box = new wxStaticBox(m_dialog, wxID_FILEDLGG, _("Custom fields"));
    static_box->Hide();
    wxStaticBoxSizer* box_sizer_right = new wxStaticBoxSizer(static_box, wxVERTICAL);
    box_sizer->Add(box_sizer_right, g_flagsExpand);

    wxScrolledWindow* scrolled_window = new wxScrolledWindow(static_box, wxID_ANY);
    wxBoxSizer *custom_sizer = new wxBoxSizer(wxVERTICAL);
    scrolled_window->SetScrollbar(wxSB_VERTICAL, wxALIGN_RIGHT, 1, -1);
    scrolled_window->SetSizer(custom_sizer);

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
        }

        wxWindowID controlID = m_init_control_id + (wxWindowID)field.FIELDID;
        wxStaticText* Description = new wxStaticText(scrolled_window, wxID_STATIC, field.DESCRIPTION);
        grid_sizer_custom->Add(Description, g_flagsH);

        switch (Model_CustomField::type(field))
        {
        case Model_CustomField::STRING:
        {
            wxTextCtrl* CustomString = new wxTextCtrl(scrolled_window, controlID
                , fieldData->CONTENT, wxDefaultPosition, wxDefaultSize);
            CustomString->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
            if (Model_CustomField::getAutocomplete(field.PROPERTIES))
            {
                const wxArrayString& values = Model_CustomFieldData::instance().allValue(field.FIELDID);
                CustomString->AutoComplete(values);
            }
            grid_sizer_custom->Add(CustomString, g_flagsExpand);

            CustomString->Connect(controlID, wxEVT_TEXT
                , wxCommandEventHandler(mmCustomData::OnStringChanged), nullptr, this);
            break;
        }
        case Model_CustomField::INTEGER:
        {
            int value;
            double test;
            if (!fieldData->CONTENT.ToDouble(&test)) {
                value = 0;
            }
            else {
                SetWidgetChanged(controlID);
                value = (int)test;
            }

            wxSpinCtrl* CustomInteger = new wxSpinCtrl(scrolled_window, controlID,
                wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -2147483647, 2147483647, value);
            CustomInteger->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
            grid_sizer_custom->Add(CustomInteger, g_flagsExpand);

            CustomInteger->Connect(controlID, wxEVT_SPINCTRL
                , wxCommandEventHandler(mmCustomData::OnIntegerChanged), nullptr, this);

            break;
        }
        case Model_CustomField::DECIMAL:
        {
            m_data_changed[controlID] = 0;

            double value;
            if (!fieldData->CONTENT.ToDouble(&value)) {
                value = 0;
            }
            else {
                SetWidgetChanged(controlID);
            }
            wxSpinCtrlDouble* CustomDecimal = new wxSpinCtrlDouble(scrolled_window, controlID
                , wxEmptyString, wxDefaultPosition, wxDefaultSize
                , wxSP_ARROW_KEYS, -2147483647, 2147483647, value, 0.01);
            CustomDecimal->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
            grid_sizer_custom->Add(CustomDecimal, g_flagsExpand);

            CustomDecimal->Connect(controlID, wxEVT_SPINCTRLDOUBLE
                , wxCommandEventHandler(mmCustomData::OnDoubleChanged), nullptr, this);

            break;
        }
        case Model_CustomField::BOOLEAN:
        {
            m_data_changed[controlID] = 0;

            wxCheckBox* CustomBoolean = new wxCheckBox(scrolled_window, controlID
                , wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);

            if (fieldData->CONTENT == "TRUE")
            {
                CustomBoolean->SetValue(true);
                SetWidgetChanged(controlID);
            }
            else if (fieldData->CONTENT == "FALSE")
            {
                CustomBoolean->SetValue(true);
                SetWidgetChanged(controlID);
            }

            CustomBoolean->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
            grid_sizer_custom->Add(CustomBoolean, g_flagsExpand);

            CustomBoolean->Connect(controlID, wxEVT_CHECKBOX
                , wxCommandEventHandler(mmCustomData::OnCheckBoxChanged), nullptr, this);

            break;
        }
        case Model_CustomField::DATE:
        {
            m_data_changed[controlID] = 0;

            wxDate value;
            if (!value.ParseDate(fieldData->CONTENT)) {
                value = wxDate::Today();
            }
            else {
                SetWidgetChanged(controlID);
            }

            wxDatePickerCtrl* CustomDate = new wxDatePickerCtrl(scrolled_window, controlID
                , value, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN | wxDP_SHOWCENTURY);
            CustomDate->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
            grid_sizer_custom->Add(CustomDate, g_flagsExpand);

            CustomDate->Connect(controlID, wxEVT_DATE_CHANGED
                , wxDateEventHandler(mmCustomData::OnDateChanged), nullptr, this);

            break;
        }
        case Model_CustomField::TIME:
        {
            m_data_changed[controlID] = 0;

            wxDateTime value;
            if (!value.ParseTime(fieldData->CONTENT)) {
                value.ParseTime("00:00:00");
            }
            else {
                SetWidgetChanged(controlID);
            }

            wxTimePickerCtrl* CustomTime = new wxTimePickerCtrl(scrolled_window, controlID
                , value, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
            CustomTime->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
            grid_sizer_custom->Add(CustomTime, g_flagsExpand);

            CustomTime->Connect(controlID, wxEVT_TIME_CHANGED
                , wxDateEventHandler(mmCustomData::OnTimeChanged), nullptr, this);

            break;
        }
        case Model_CustomField::SINGLECHOICE:
        {
            wxArrayString Choices = Model_CustomField::getChoices(field.PROPERTIES);
            Choices.Sort();

            wxChoice* CustomChoice = new wxChoice(scrolled_window, controlID
                , wxDefaultPosition, wxDefaultSize, Choices);
            CustomChoice->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
            grid_sizer_custom->Add(CustomChoice, g_flagsExpand);

            if (Choices.size() == 0) {
                CustomChoice->Enable(false);
            }

            CustomChoice->SetStringSelection(fieldData->CONTENT);
            
            
            CustomChoice->Connect(controlID, wxEVT_CHOICE
                , wxCommandEventHandler(mmCustomData::OnSingleChoice), nullptr, this);
            break;
        }
        case Model_CustomField::MULTICHOICE:
        {
            const auto& content = fieldData->CONTENT;
            const auto& name = field.DESCRIPTION;

            wxButton* multi_choice_button = new wxButton(scrolled_window, controlID, content
                , wxDefaultPosition, wxDefaultSize, 0L, wxDefaultValidator, name);
            multi_choice_button->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
            grid_sizer_custom->Add(multi_choice_button, g_flagsExpand);

            multi_choice_button->Connect(controlID, wxEVT_COMMAND_BUTTON_CLICKED
                , wxCommandEventHandler(mmCustomData::OnMultiChoice), nullptr, this);

            break;
        }
        default: break;
        }
    }

    scrolled_window->FitInside();
    scrolled_window->SetScrollRate(5, 5);
    box_sizer_right->Add(scrolled_window, g_flagsExpand);

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
        .find(Model_CustomField::REFTYPE(m_ref_type)
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

size_t mmCustomData::GetActiveCustomFieldsCount()
{ 
    const auto& data_set = Model_CustomFieldData::instance().find(Model_CustomFieldData::REFID(m_ref_id));
    return data_set.size();
}

bool mmCustomData::SaveCustomValues(int ref_id)
{
    Model_CustomFieldData::instance().Savepoint();

    for (const auto &field : m_fields)
    {
        wxWindowID controlID = m_init_control_id + (wxWindowID)field.FIELDID;
        wxString data = wxEmptyString;

        switch (Model_CustomField::type(field))
        {
        case Model_CustomField::STRING:
        {
            wxTextCtrl* CustomString = (wxTextCtrl*)m_dialog->FindWindow(controlID);
            if (CustomString != nullptr) {
                data = CustomString->GetValue().Trim();
            }
            break;
        }
        case Model_CustomField::INTEGER:
        {
            wxSpinCtrl* CustomInteger = (wxSpinCtrl*)m_dialog->FindWindow(controlID);
            if (CustomInteger && IsWidgetChanged(controlID)) {
                data = wxString::Format("%i", CustomInteger->GetValue());
            }
            break;
        }
        case Model_CustomField::DECIMAL:
        {
            wxSpinCtrlDouble* CustomDecimal = (wxSpinCtrlDouble*)m_dialog->FindWindow(controlID);
            if (CustomDecimal && IsWidgetChanged(controlID)) {
                data = wxString::Format("%f", CustomDecimal->GetValue()); 
            }
            break;
        }
        case Model_CustomField::BOOLEAN:
        {
            wxCheckBox* CustomBoolean = (wxCheckBox*)m_dialog->FindWindow(controlID);
            if (CustomBoolean && IsWidgetChanged(controlID)) {
                data = (CustomBoolean->GetValue()) ? "TRUE" : "FALSE";
            }
            break;
        }
        case Model_CustomField::DATE:
        {
            wxDatePickerCtrl* CustomDate = (wxDatePickerCtrl*)m_dialog->FindWindow(controlID);
            if (CustomDate && IsWidgetChanged(controlID)) {
                data = CustomDate->GetValue().FormatISODate();
            }
            break;
        }
        case Model_CustomField::TIME:
        {
            wxTimePickerCtrl* CustomTime = (wxTimePickerCtrl*)m_dialog->FindWindow(controlID);
            if (CustomTime && IsWidgetChanged(controlID)) {
                data = CustomTime->GetValue().FormatISOTime();
            }
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
            Model_CustomFieldData::Data* fieldData = Model_CustomFieldData::instance().get(field.FIELDID, m_ref_id);
            if (!fieldData) {
                fieldData = Model_CustomFieldData::instance().create();
            }

            fieldData->REFID = ref_id;
            fieldData->FIELDID = field.FIELDID;
            fieldData->CONTENT = data;
            wxLogDebug("Control:%i Type:%s Value:%s"
                , controlID
                , Model_CustomField::all_type()[Model_CustomField::type(field)]
                , data);
            Model_CustomFieldData::instance().save(fieldData);
        }
    }

    Model_CustomFieldData::instance().ReleaseSavepoint();
    return true;
}

void mmCustomData::OnStringChanged(wxCommandEvent& event)
{
    int controlID = event.GetId();
    wxString data = wxEmptyString;
    SetWidgetChanged(controlID);
    wxTextCtrl* CustomString = (wxTextCtrl*)m_dialog->FindWindow(controlID);
    if (CustomString != nullptr) {
        data = CustomString->GetValue().Trim();
    }
    if (data.empty()) {
        ResetWidgetChanged(controlID);
    }
}

void mmCustomData::ResetWidgetChanged(wxWindowID id)
{
    m_data_changed[id] = 0;
}

void mmCustomData::ResetWidgetsChanged()
{
    m_data_changed.clear();
}

void mmCustomData::OnSingleChoice(wxCommandEvent& event)
{
    SetWidgetChanged(event.GetId());
}

void mmCustomData::OnDoubleChanged(wxCommandEvent& event)
{
    SetWidgetChanged(event.GetId());
}

void mmCustomData::OnIntegerChanged(wxCommandEvent& event)
{
    SetWidgetChanged(event.GetId());
}

void mmCustomData::OnCheckBoxChanged(wxCommandEvent& event)
{
    SetWidgetChanged(event.GetId());
}

void mmCustomData::OnDateChanged(wxDateEvent& event)
{
    SetWidgetChanged(event.GetId());
}

void mmCustomData::OnTimeChanged(wxDateEvent& event)
{
    SetWidgetChanged(event.GetId());
}

bool mmCustomData::IsWidgetChanged(wxWindowID id)
{
    int count = m_data_changed.at(id);
    return count > 0;
}

bool mmCustomData::IsSomeWidgetChanged()
{
    for (const auto& entry : m_data_changed)
    {
        if (entry.second > 0) return true;
    }
    return false;
}

void mmCustomData::SetWidgetChanged(wxWindowID id)
{ 
    m_data_changed[id]++;
};

bool mmCustomData::IsDataFound(const Model_Checking::Full_Data &tran)
{
    return true;
    m_data_changed;
}
