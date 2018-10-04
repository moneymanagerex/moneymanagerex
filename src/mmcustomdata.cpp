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
#include "Model_CustomFieldData.h"
#include "Model_Attachment.h"

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
    , m_static_box(nullptr)
    , m_ref_type(ref_type)
    , m_ref_id(ref_id)
{
    m_dialog = dialog;
    m_fields = Model_CustomField::instance().find(Model_CustomField::DB_Table_CUSTOMFIELD::REFTYPE(m_ref_type));
    std::sort(m_fields.begin(), m_fields.end(), SorterByDESCRIPTION());
    m_data_changed.clear();
}

mmCustomDataTransaction::mmCustomDataTransaction(wxDialog* dialog, int ref_id, wxWindowID base_id)
    : mmCustomData(dialog
        , Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION)
        , ref_id)
{
    SetBaseID(base_id);
    SetLabelID(base_id + GetCustomFieldsCount());
}

bool mmCustomData::FillCustomFields(wxBoxSizer* box_sizer)
{
    m_static_box = new wxStaticBox(m_dialog, wxID_ANY, _("Custom fields"));
    wxStaticBoxSizer* box_sizer_right = new wxStaticBoxSizer(m_static_box, wxVERTICAL);
    box_sizer->Add(box_sizer_right, g_flagsExpand);

    wxScrolledWindow* scrolled_window = new wxScrolledWindow(m_static_box, wxID_ANY);
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

        wxWindowID controlID = GetBaseID() + (wxWindowID)field.FIELDID;
        wxWindowID labelID = GetLabelID() + (wxWindowID)field.FIELDID;
        
        wxCheckBox* Description = new wxCheckBox(scrolled_window
            , labelID, field.DESCRIPTION
            , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE
            , wxDefaultValidator, field.TYPE);
        Description->Connect(labelID, wxEVT_CHECKBOX
            , wxCommandEventHandler(mmCustomData::OnCheckBoxActivated), nullptr, this);

        grid_sizer_custom->Add(Description, g_flagsH);

        switch (Model_CustomField::type(field))
        {
        case Model_CustomField::STRING:
        {
            const auto& data = fieldData->CONTENT;
            wxTextCtrl* CustomString = new wxTextCtrl(scrolled_window, controlID, data, wxDefaultPosition, wxDefaultSize);
            CustomString->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
            if (Model_CustomField::getAutocomplete(field.PROPERTIES))
            {
                const wxArrayString& values = Model_CustomFieldData::instance().allValue(field.FIELDID);
                CustomString->AutoComplete(values);
            }
            grid_sizer_custom->Add(CustomString, g_flagsExpand);

            if (!data.empty())
            {
                Description->SetValue(true);
            }

            CustomString->Connect(controlID, wxEVT_TEXT, wxCommandEventHandler(mmCustomData::OnStringChanged), nullptr, this);
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
                value = (int)test;
                SetWidgetChanged(controlID, wxString::Format("%i", value));
                Description->SetValue(true);
            }

            wxSpinCtrl* CustomInteger = new wxSpinCtrl(scrolled_window, controlID,
                wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -2147483647, 2147483647, value);
            CustomInteger->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
            grid_sizer_custom->Add(CustomInteger, g_flagsExpand);

            CustomInteger->Connect(controlID, wxEVT_SPINCTRL, wxCommandEventHandler(mmCustomData::OnIntegerChanged), nullptr, this);

            break;
        }
        case Model_CustomField::DECIMAL:
        {
            double value;
            if (!fieldData->CONTENT.ToDouble(&value)) {
                value = 0;
            }
            else {
                const auto& data = wxString::Format("%f", value);
                SetWidgetChanged(controlID, data);
                Description->SetValue(true);
            }

            int DigitScale = Model_CustomField::getDigitScale(field.PROPERTIES);
            wxSpinCtrlDouble* CustomDecimal = new wxSpinCtrlDouble(scrolled_window, controlID
                , wxEmptyString, wxDefaultPosition, wxDefaultSize
                , wxSP_ARROW_KEYS, -2147483647, 2147483647, value, 1 / pow(10, DigitScale));
            CustomDecimal->SetDigits(DigitScale);
            CustomDecimal->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
            grid_sizer_custom->Add(CustomDecimal, g_flagsExpand);

            CustomDecimal->Connect(controlID, wxEVT_SPINCTRLDOUBLE, wxCommandEventHandler(mmCustomData::OnDoubleChanged), nullptr, this);

            break;
        }
        case Model_CustomField::BOOLEAN:
        {
            wxCheckBox* CustomBoolean = new wxCheckBox(scrolled_window, controlID,
                wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxCHK_2STATE);

            const auto& data = fieldData->CONTENT;
            if (!data.empty())
            {
                CustomBoolean->SetValue(data == "TRUE");
                SetWidgetChanged(controlID, data);
                Description->SetValue(true);
            }

            CustomBoolean->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
            grid_sizer_custom->Add(CustomBoolean, g_flagsExpand);

            CustomBoolean->Connect(controlID, wxEVT_CHECKBOX, wxCommandEventHandler(mmCustomData::OnCheckBoxChanged), nullptr, this);

            break;
        }
        case Model_CustomField::DATE:
        {
            wxDate value;
            if (!value.ParseDate(fieldData->CONTENT)) {
                value = wxDate::Today();
            }
            else {
                SetWidgetChanged(controlID, value.FormatISODate());
                Description->SetValue(true);
            }

            wxDatePickerCtrl* CustomDate = new wxDatePickerCtrl(scrolled_window, controlID
                , value, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN | wxDP_SHOWCENTURY);
            CustomDate->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
            grid_sizer_custom->Add(CustomDate, g_flagsExpand);

            CustomDate->Connect(controlID, wxEVT_DATE_CHANGED, wxDateEventHandler(mmCustomData::OnDateChanged), nullptr, this);

            break;
        }
        case Model_CustomField::TIME:
        {
            wxDateTime value;
            if (!value.ParseTime(fieldData->CONTENT)) {
                value.ParseTime("00:00:00");
            }
            else {
                SetWidgetChanged(controlID, value.FormatISOTime());
                Description->SetValue(true);
            }

            wxTimePickerCtrl* CustomTime = new wxTimePickerCtrl(scrolled_window, controlID
                , value, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
            CustomTime->SetToolTip(Model_CustomField::getTooltip(field.PROPERTIES));
            grid_sizer_custom->Add(CustomTime, g_flagsExpand);

            CustomTime->Connect(controlID, wxEVT_TIME_CHANGED, wxDateEventHandler(mmCustomData::OnTimeChanged), nullptr, this);

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

            const auto& data = fieldData->CONTENT;
            if (!data.empty())
            {
                CustomChoice->SetStringSelection(data);
                SetWidgetChanged(controlID, data);
                Description->SetValue(true);
            }
            
            CustomChoice->Connect(controlID, wxEVT_CHOICE, wxCommandEventHandler(mmCustomData::OnSingleChoice), nullptr, this);
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

            if (!content.empty()) {
                SetWidgetChanged(controlID, content);
                Description->SetValue(true);
            }

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
    m_static_box->Hide();

    return true;
}

void mmCustomData::OnMultiChoice(wxCommandEvent& event)
{
    long controlID = event.GetId();
    wxButton* button = (wxButton*)m_dialog->FindWindow(controlID);
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

    wxString data = label;
    wxMultiChoiceDialog* MultiChoice = new wxMultiChoiceDialog(this, _("Please select"), _("Multi Choice"), all_choices);
    MultiChoice->SetSelections(arr_selections);

    if (MultiChoice->ShowModal() == wxID_OK)
    {
        data.clear();
        for (const auto &s : MultiChoice->GetSelections()) {
            data += all_choices[s] + ";";
        }
        data.RemoveLast();
    }

    delete MultiChoice;
    button->SetLabel(data);
    SetWidgetChanged(controlID, data);
}

size_t mmCustomData::GetActiveCustomFieldsCount()
{ 
    const auto& data_set = Model_CustomFieldData::instance().find(Model_CustomFieldData::REFID(m_ref_id));
    return data_set.size();
}

std::map<wxString, wxString> mmCustomData::GetActiveCustomFields()
{
    wxString data;
    std::map<wxString, wxString> values;
    for (const auto& entry : m_data_changed)
    {
        int id = entry.first - GetBaseID();
        Model_CustomField::Data *item = Model_CustomField::instance().get(id);
        if (item) {
            data = item->DESCRIPTION;
        }
        values[data] = entry.second;
    }
    
    return values;
}

const wxString mmCustomData::GetWidgetData(wxWindowID controlID)
{
    wxString data;
    if (m_data_changed.find(controlID) != m_data_changed.end()) 
    {
        data = m_data_changed.at(controlID);
    }
    else
    {
        wxWindow* w = m_dialog->FindWindowById(controlID);
        if (w)
        {
            const wxString class_name = w->GetEventHandler()->GetClassInfo()->GetClassName();
            if (class_name == "wxDatePickerCtrl")
            { 
                wxDatePickerCtrl* d = (wxDatePickerCtrl*)w;
                data = d->GetValue().FormatISODate();
            }
            else if (class_name == "wxTimePickerCtrl")
            {
                wxTimePickerCtrl* d = (wxTimePickerCtrl*)w;
                data = d->GetValue().FormatISOTime();
            }
            else if (class_name == "wxSpinCtrlDouble")
            {
                wxSpinCtrlDouble* d = (wxSpinCtrlDouble*)w;
                data = wxString::Format("%f", d->GetValue());
            }
            else if (class_name == "wxSpinCtrl")
            {
                wxSpinCtrl* d = (wxSpinCtrl*)w;
                data = wxString::Format("%i", d->GetValue());
            }
            else if (class_name == "wxChoice")
            {
                wxChoice* d = (wxChoice*)w;
                data = d->GetStringSelection();
            }
            else if (class_name == "wxButton")
            {
                wxButton* d = (wxButton*)w;
                data = d->GetLabel();
            }
            else if (class_name == "wxTextCtrl")
            {
                wxTextCtrl* d = (wxTextCtrl*)w;
                data = d->GetValue();
            }
            else if (class_name == "wxCheckBox")
            {
                wxCheckBox* d = (wxCheckBox*)w;
                data = (d->GetValue() ? "TRUE" : "FALSE");
            }
        }
    }
    return data;
}

bool mmCustomData::SaveCustomValues(int ref_id)
{
    Model_CustomFieldData::instance().Savepoint();

    for (const auto &field : m_fields)
    {
        wxWindowID controlID = GetBaseID() + (wxWindowID)field.FIELDID;
        const auto& data = IsWidgetChanged(controlID) ? GetWidgetData(controlID) : "";

        Model_CustomFieldData::Data* fieldData = Model_CustomFieldData::instance().get(field.FIELDID, ref_id);
        if (!data.empty())
        {
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
        else if (fieldData)
        {
            Model_CustomFieldData::instance().remove(fieldData->FIELDATADID);
        }
    }

    Model_CustomFieldData::instance().ReleaseSavepoint();
    return true;
}

void mmCustomData::OnStringChanged(wxCommandEvent& event)
{
    int controlID = event.GetId();
    wxString data = event.GetString();

    if (data.empty()) {
        ResetWidgetChanged(controlID);
    }
    else
    {
        SetWidgetChanged(controlID, data);
    }
}

void mmCustomData::ResetWidgetChanged(wxWindowID id)
{
    m_data_changed.erase(id);
}

void mmCustomData::ResetWidgetsChanged()
{
    for (const auto& entry : m_data_changed)
    {
        auto label_id = entry.first - GetBaseID() + GetLabelID();
        wxCheckBox* Description = (wxCheckBox*)m_dialog->FindWindow(label_id);
        if (Description) {
            Description->SetValue(false);
            wxLogDebug("Description %i value = %s", label_id, "FALSE");
        }
    }

    m_data_changed.clear();
}

void mmCustomData::ClearSettings()
{
    for (const auto &field : m_fields)
    {
        //wxWindowID controlID = GetBaseID() + (wxWindowID)field.FIELDID;
        wxWindowID labelID = GetLabelID() + (wxWindowID)field.FIELDID;
        wxCheckBox* cb = (wxCheckBox*)FindWindowById(labelID);
        if (cb) cb->SetValue(false);
    }
}

void mmCustomData::OnSingleChoice(wxCommandEvent& event)
{
    const wxString& data = event.GetString();
    SetWidgetChanged(event.GetId(), data);
}

void mmCustomData::OnDoubleChanged(wxCommandEvent& event)
{
    const auto& data = event.GetString();
    SetWidgetChanged(event.GetId(), data);
}

void mmCustomData::OnIntegerChanged(wxCommandEvent& event)
{
    auto data = event.GetInt();
    SetWidgetChanged(event.GetId(), wxString::Format("%i", data));
}

void mmCustomData::OnCheckBoxChanged(wxCommandEvent& event)
{
    const auto& data = event.IsChecked() ? "TRUE" : "FALSE";
    SetWidgetChanged(event.GetId(), data);
}

int mmCustomData::GetWidgetType(wxWindowID controlID)
{
    Model_CustomField::Data_Set fields = Model_CustomField::instance().find(Model_CustomField::DB_Table_CUSTOMFIELD::REFTYPE(m_ref_type));
    int control_id = controlID - GetBaseID();
    for (const auto& entry : fields)
    {
        if (entry.FIELDID == control_id)
        {
            return Model_CustomField::type(entry);
        }
    }
    wxASSERT(false);
    return -1;

}

void mmCustomData::OnCheckBoxActivated(wxCommandEvent& event)
{
    auto id = event.GetId();
    auto widget_id = id - GetLabelID() + GetBaseID();
    auto checked = event.IsChecked();

    if (checked) {
        //TODO: 
        const auto& data = GetWidgetData(widget_id);
        SetWidgetChanged(widget_id, data);
    }
    else {
        this->ResetWidgetChanged(widget_id);
    }

}

void mmCustomData::OnDateChanged(wxDateEvent& event)
{
    const auto data = event.GetDate();
    SetWidgetChanged(event.GetId(), data.FormatISODate());
}

void mmCustomData::OnTimeChanged(wxDateEvent& event)
{
    auto data = event.GetDate();
    SetWidgetChanged(event.GetId(), data.FormatISOTime());
}

bool mmCustomData::IsWidgetChanged(wxWindowID id)
{
    const wxString& value = m_data_changed.find(id) == m_data_changed.end()
        ? wxString(wxEmptyString) : m_data_changed.at(id);
    return !value.empty();
}

bool mmCustomData::IsSomeWidgetChanged()
{
    for (const auto& entry : m_data_changed)
    {
        if (!entry.second.empty()) return true;
    }
    return false;
}

void mmCustomData::SetWidgetChanged(wxWindowID id, const wxString& data)
{ 
    m_data_changed[id] = data;

    auto label_id = id - GetBaseID() + GetLabelID();
    wxCheckBox* Description = (wxCheckBox*)m_dialog->FindWindow(label_id);
    if (Description) {
        Description->SetValue(true);
        wxLogDebug("Description %i value = %i", label_id, 1);
    }
}

bool mmCustomData::IsDataFound(const Model_Checking::Full_Data &tran)
{
    const auto& data_set = Model_CustomFieldData::instance().find(Model_CustomFieldData::REFID(tran.TRANSID));
    for (const auto& filter : m_data_changed)
    {
        for (const auto& item : data_set)
        {
            if (filter.second == item.CONTENT)
            {
                return true;
            }
        }

    }
    return false;
}

bool mmCustomData::IsCustomPanelShown()
{
    return m_static_box->IsShown();
}

void mmCustomData::ShowHideCustomPanel()
{
    if (IsCustomPanelShown()) {
        m_static_box->Hide();
    }
    else {
        m_static_box->Show();
    }
}
