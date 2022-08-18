/*******************************************************
Copyright (C) 2017 Gabriele-V
Copyright (C) 2018, 2021, 2022 Nikolay Akimov
Copyright (C) 2021, 2022 Mark Whalley (mark@ipx.co.uk)

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
#include "util.h"
#include "mmSimpleDialogs.h"
#include "validators.h"
#include "Model_Currency.h"
#include "Model_CustomFieldData.h"
#include "Model_Attachment.h"

#include <wx/timectrl.h>
#include <wx/collpane.h>
#include <wx/spinctrl.h>

mmCustomData::~mmCustomData()
{
    wxLogDebug("~mmCustomData");
}

mmCustomData::mmCustomData()
    : wxDialog()
    , m_ref_id(-1)
{
}

mmCustomData::mmCustomData(wxDialog* dialog, const wxString& ref_type, int ref_id)
    : wxDialog()
    , m_static_box(nullptr)
    , m_ref_type(ref_type)
    , m_ref_id(ref_id)
{
    m_dialog = dialog;
    m_fields = Model_CustomField::instance().find(Model_CustomField::DB_Table_CUSTOMFIELD_V1::REFTYPE(m_ref_type));
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
        bool nonDefaultData = true;
        Model_CustomFieldData::Data* fieldData = Model_CustomFieldData::instance().get(field.FIELDID, m_ref_id);
        if (!fieldData)
        {
            fieldData = Model_CustomFieldData::instance().create();
            fieldData->FIELDID = field.FIELDID;
            fieldData->REFID = m_ref_id;
            fieldData->CONTENT = Model_CustomField::getDefault(field.PROPERTIES);
            nonDefaultData = false;
        }

        wxWindowID controlID = GetBaseID() + field.FIELDID * 2;
        wxWindowID labelID = controlID + 1;

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
            mmToolTip(CustomString, Model_CustomField::getTooltip(field.PROPERTIES));
            if (Model_CustomField::getAutocomplete(field.PROPERTIES))
            {
                const wxArrayString& values = Model_CustomFieldData::instance().allValue(field.FIELDID);
                CustomString->AutoComplete(values);
            }
            grid_sizer_custom->Add(CustomString, g_flagsExpand);

            if (!data.empty()) {
                if (nonDefaultData) 
                    SetWidgetChanged(controlID, data);
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
                value = trunc(test);
                if (nonDefaultData) 
                    SetWidgetChanged(controlID, wxString::Format("%i", value));
            }

            wxSpinCtrl* CustomInteger = new wxSpinCtrl(scrolled_window, controlID,
                wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, -2147483647, 2147483647, value);
            mmToolTip(CustomInteger, Model_CustomField::getTooltip(field.PROPERTIES));
            grid_sizer_custom->Add(CustomInteger, g_flagsExpand);

            CustomInteger->Connect(controlID, wxEVT_SPINCTRL, wxCommandEventHandler(mmCustomData::OnIntegerChanged), nullptr, this);

            break;
        }
        case Model_CustomField::DECIMAL:
        {
            // Strip any thousands separators and make sure decimal is "."
            // earlier implementations may have different formats so we need to cleanse
            wxString content = fieldData->CONTENT;
            wxRegEx pattern(R"([\., ](?=\d*[\., ]))");
            pattern.ReplaceAll(&content, wxEmptyString);
            content.Replace(",",".");

            double value;
            int digitScale = Model_CustomField::getDigitScale(field.PROPERTIES);
            if (!content.ToCDouble(&value)) {
                value = 0;
            }
            else {
                if (nonDefaultData) 
                    SetWidgetChanged(controlID, Model_Currency::toString(value, NULL, digitScale));
            }
            
            mmTextCtrl* CustomDecimal = new mmTextCtrl(scrolled_window, controlID
                , wxEmptyString, wxDefaultPosition, wxDefaultSize
                , wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
            CustomDecimal->SetAltPrecision(digitScale);
            CustomDecimal->SetValue(value, digitScale);
            CustomDecimal->Connect(wxID_ANY, wxEVT_TEXT
                , wxCommandEventHandler(mmCustomData::OnStringChanged), nullptr, this);

            mmToolTip(CustomDecimal, Model_CustomField::getTooltip(field.PROPERTIES));
            grid_sizer_custom->Add(CustomDecimal, g_flagsExpand);

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
                if (nonDefaultData) 
                    SetWidgetChanged(controlID, data);
            }

            mmToolTip(CustomBoolean, Model_CustomField::getTooltip(field.PROPERTIES));
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
                if (nonDefaultData) 
                    SetWidgetChanged(controlID, value.FormatISODate());
            }

            mmDatePickerCtrl* CustomDate = new mmDatePickerCtrl(scrolled_window, controlID, value);
            mmToolTip(CustomDate, Model_CustomField::getTooltip(field.PROPERTIES));
            grid_sizer_custom->Add(CustomDate->mmGetLayout());

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
                if (nonDefaultData) 
                    SetWidgetChanged(controlID, value.FormatISOTime());
            }

            wxTimePickerCtrl* CustomTime = new wxTimePickerCtrl(scrolled_window, controlID
                , value, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
            mmToolTip(CustomTime, Model_CustomField::getTooltip(field.PROPERTIES));
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
            mmToolTip(CustomChoice, Model_CustomField::getTooltip(field.PROPERTIES));
            grid_sizer_custom->Add(CustomChoice, g_flagsExpand);

            if (Choices.empty()) {
                CustomChoice->Enable(false);
            }

            const auto& data = fieldData->CONTENT;
            if (!data.empty())
            {
                CustomChoice->SetStringSelection(data);
                if (nonDefaultData) 
                    SetWidgetChanged(controlID, data);
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
            mmToolTip(multi_choice_button, Model_CustomField::getTooltip(field.PROPERTIES));
            grid_sizer_custom->Add(multi_choice_button, g_flagsExpand);

            if (!content.empty()) {
                if (nonDefaultData) 
                    SetWidgetChanged(controlID, content);
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
    auto init = event.GetInt();
    wxButton* button = static_cast<wxButton*>(m_dialog->FindWindow(controlID));
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
    if (init != -1)
    {
        wxSharedPtr<wxMultiChoiceDialog> MultiChoice(new wxMultiChoiceDialog(this, _("Please select"), _("Multi Choice"), all_choices));
        MultiChoice->SetSelections(arr_selections);

        if (MultiChoice->ShowModal() == wxID_OK)
        {
            data.clear();
            for (const auto& s : MultiChoice->GetSelections()) {
                data += all_choices[s] + ";";
            }
            data.RemoveLast();
        }
    }
    button->SetLabel(data);
    SetWidgetChanged(controlID, data);
}

size_t mmCustomData::GetActiveCustomFieldsCount() const
{
    const auto& data_set = Model_CustomFieldData::instance().find(Model_CustomFieldData::REFID(m_ref_id));
    return data_set.size();
}

std::map<int, wxString> mmCustomData::GetActiveCustomFields() const
{
    std::map<int, wxString> values;
    for (const auto& entry : m_data_changed)
    {
        int id = (entry.first - GetBaseID()) / 2;
        Model_CustomField::Data *item = Model_CustomField::instance().get(id);
        if (item) {
            values[item->FIELDID] = entry.second;
        }
    }

    return values;
}

void mmCustomData::SetWidgetData(wxWindowID controlID, const wxString& value)
{
    wxWindow* w = m_dialog->FindWindowById(controlID, m_dialog);
    if (!w)
        return;

    if (value.empty())
    {
        ResetWidgetChanged(controlID);
        return;
    }

    const wxString class_name = w->GetEventHandler()->GetClassInfo()->GetClassName();

    if (class_name == "wxDatePickerCtrl")
    {
        mmDatePickerCtrl* d = static_cast<mmDatePickerCtrl*>(w);
        wxDateTime date;
        date.ParseDate(value);
        d->SetValue(date);
        wxDateEvent evt(d, date, wxEVT_DATE_CHANGED);
        d->GetEventHandler()->AddPendingEvent(evt);
    }
    else if (class_name == "wxTimePickerCtrl")
    {
        wxTimePickerCtrl* d = static_cast<wxTimePickerCtrl*>(w);
        wxDateTime time;
        time.ParseTime(value);
        d->SetValue(time);
        wxDateEvent evt(d, time, wxEVT_TIME_CHANGED);
        d->GetEventHandler()->AddPendingEvent(evt);
    }  
    else if (class_name == "wxSpinCtrl")
    {
        wxSpinCtrl* d = static_cast<wxSpinCtrl*>(w);
        int v = wxAtoi(value);
        d->SetValue(v);
        wxCommandEvent evt(wxEVT_SPINCTRL, controlID);
        evt.SetInt(v);
        d->GetEventHandler()->AddPendingEvent(evt);
    }
    else if (class_name == "wxChoice")
    {
        wxChoice* d = static_cast<wxChoice*>(w);
        d->SetStringSelection(value);
        wxCommandEvent evt(wxEVT_CHOICE, controlID);
        evt.SetString(value);
        d->GetEventHandler()->AddPendingEvent(evt);
    }
    else if (class_name == "wxButton")
    {
        wxButton* d = static_cast<wxButton*>(w);
        d->SetLabel(value);

        wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, controlID);
        evt.SetInt(-1);
        d->GetEventHandler()->AddPendingEvent(evt);

    }
    else if (class_name == "wxTextCtrl")
    {
        wxTextCtrl* d = static_cast<wxTextCtrl*>(w);
        d->SetValue(value);
    }
    else if (class_name == "wxCheckBox")
    {
        wxCheckBox* d = static_cast<wxCheckBox*>(w);
        bool v = wxString("TRUE|true|1").Contains(value);
        d->SetValue(v);
        wxCommandEvent evt(wxEVT_CHECKBOX, controlID);
        evt.SetInt(v);
        d->GetEventHandler()->AddPendingEvent(evt);
    }
}

const wxString mmCustomData::GetWidgetData(wxWindowID controlID) const
{
    wxString data;
    if (m_data_changed.find(controlID) != m_data_changed.end())
    {
        data = m_data_changed.at(controlID);
    }
    else
    {
        wxWindow* w = FindWindowById(controlID, m_dialog);
        if (w)
        {
            const wxString class_name = w->GetEventHandler()->GetClassInfo()->GetClassName();
            if (class_name == "wxDatePickerCtrl")
            {
                mmDatePickerCtrl* d = static_cast<mmDatePickerCtrl*>(w);
                data = d->GetValue().FormatISODate();
            }
            else if (class_name == "wxTimePickerCtrl")
            {
                wxTimePickerCtrl* d = static_cast<wxTimePickerCtrl*>(w);
                data = d->GetValue().FormatISOTime();
            }
            else if (class_name == "wxSpinCtrl")
            {
                wxSpinCtrl* d = static_cast<wxSpinCtrl*>(w);
                data = wxString::Format("%i", d->GetValue());
            }
            else if (class_name == "wxChoice")
            {
                wxChoice* d = static_cast<wxChoice*>(w);
                data = d->GetStringSelection();
            }
            else if (class_name == "wxButton")
            {
                wxButton* d = static_cast<wxButton*>(w);
                data = d->GetLabel();
            }
            else if (class_name == "wxTextCtrl")
            {
                wxTextCtrl* d = static_cast<wxTextCtrl*>(w);
                data = d->GetValue();
            }
            else if (class_name == "wxCheckBox")
            {
                wxCheckBox* d = static_cast<wxCheckBox*>(w);
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
        wxWindowID controlID = GetBaseID() + field.FIELDID * 2;
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

void mmCustomData::UpdateCustomValues(int ref_id)
{
    Model_CustomFieldData::instance().Savepoint();

    for (const auto& field : m_fields)
    {
        bool is_changed = false;

        wxWindowID controlID = GetBaseID() + field.FIELDID * 2;
        auto label_id = controlID + 1;
        wxCheckBox* Description = static_cast<wxCheckBox*>(m_dialog->FindWindow(label_id));
        if (Description) {
            is_changed = Description->GetValue();
        }

        if (is_changed)
        {
            const auto& data = GetWidgetData(controlID);
            Model_CustomFieldData::Data* fieldData = Model_CustomFieldData::instance().get(field.FIELDID, ref_id);
            if (!data.empty())
            {
                if (!fieldData) {
                    fieldData = Model_CustomFieldData::instance().create();
                }

                fieldData->REFID = ref_id;
                fieldData->FIELDID = field.FIELDID;
                fieldData->CONTENT = data;
                Model_CustomFieldData::instance().save(fieldData);
            }
            else if (fieldData) {
                Model_CustomFieldData::instance().remove(fieldData->FIELDATADID);
            }
        }
    }

    Model_CustomFieldData::instance().ReleaseSavepoint();
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
    auto label_id = id - GetBaseID() + 1;
    wxCheckBox* check_box = static_cast<wxCheckBox*>(m_dialog->FindWindow(label_id));
    if (check_box) {
        check_box->SetValue(false);
    }
    m_data_changed.erase(id);
}

void mmCustomData::ResetWidgetsChanged()
{
    for (const auto& entry : m_data_changed)
    {
        auto label_id = entry.first - GetBaseID() + 1;
        wxCheckBox* check_box = static_cast<wxCheckBox*>(m_dialog->FindWindow(label_id));
        if (check_box) {
            check_box->SetValue(false);
            wxLogDebug("Description %i value = %s", label_id, "FALSE");
        }
    }

    m_data_changed.clear();
}

void mmCustomData::ClearSettings()
{
    for (const auto &field : m_fields)
    {
        SetStringValue(field.FIELDID * 2, "");
        wxWindowID labelID = GetBaseID() + field.FIELDID * 2 + 1;
        wxCheckBox* cb = static_cast<wxCheckBox*>(FindWindowById(labelID, m_dialog));
        if (cb)
            cb->SetValue(false);
    }
}

void mmCustomData::OnSingleChoice(wxCommandEvent& event)
{
    const wxString& data = event.GetString();
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

int mmCustomData::GetWidgetType(wxWindowID controlID) const
{
    Model_CustomField::Data_Set fields = Model_CustomField::instance().find(Model_CustomField::DB_Table_CUSTOMFIELD_V1::REFTYPE(m_ref_type));
    int control_id = (controlID - GetBaseID()) / 2;
    for (const auto& entry : fields)
    {
        if (entry.FIELDID == control_id)
        {
            return Model_CustomField::type(entry);
        }
    }
    wxFAIL_MSG("unknown custom field type");
    return -1;
}

int mmCustomData::GetPrecision(wxWindowID controlID) const
{
    int control_id = (controlID - GetBaseID()) / 2;
    for (const auto &field : m_fields)
        if (field.FIELDID == control_id)
            return (Model_CustomField::getDigitScale(field.PROPERTIES));
    wxFAIL_MSG("No field found");
    return -1;
}

void mmCustomData::OnCheckBoxActivated(wxCommandEvent& event)
{
    auto id = event.GetId();
    auto widget_id = id - 1;
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
    event.Skip();
}

void mmCustomData::OnTimeChanged(wxDateEvent& event)
{
    auto data = event.GetDate();
    SetWidgetChanged(event.GetId(), data.FormatISOTime());
}

bool mmCustomData::IsWidgetChanged(wxWindowID id)
{
    const wxString& value = (m_data_changed.find(id) == m_data_changed.end())
        ? "" : m_data_changed.at(id);
    return !value.empty();
}

bool mmCustomData::IsSomeWidgetChanged() const
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

    auto label_id = id + 1;
    wxCheckBox* check_box = static_cast<wxCheckBox*>(m_dialog->FindWindow(label_id));
    if (check_box) {
        check_box->SetValue(true);
        wxLogDebug("[V] %s = %s", check_box->GetLabel(), data);
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

bool mmCustomData::IsCustomPanelShown() const
{
    return m_static_box->IsShown();
}

void mmCustomData::ShowHideCustomPanel() const
{
    if (IsCustomPanelShown()) {
        m_static_box->Hide();
    }
    else {
        if (!m_fields.empty())
            m_static_box->Show();
    }
}

void mmCustomData::ShowCustomPanel() const
{
    m_static_box->Show();
}

void mmCustomData::SetStringValue(int fieldId, const wxString& value)
{
    wxWindowID widget_id = GetBaseID() + fieldId;
    SetWidgetData(widget_id, value);
}

bool mmCustomData::ValidateCustomValues(int ref_id)
{
    bool is_valid = true;
    for (const auto &field : m_fields)
    {
        wxWindowID controlID = GetBaseID() + field.FIELDID * 2;
        wxWindowID labelID = controlID + 1;

        wxCheckBox* cb = static_cast<wxCheckBox*>(FindWindowById(labelID, m_dialog));
        if (!cb || !cb->GetValue())
            continue;

        const wxString regExStr = Model_CustomField::getRegEx(field.PROPERTIES);
        if (!regExStr.empty())
        {
            const auto& data = GetWidgetData(controlID);
            wxRegEx regEx(regExStr, wxRE_EXTENDED);

            if (!regEx.Matches(data))
            {
                mmErrorDialogs::MessageError(this, wxString::Format(_("Unable to save custom field \"%s\":\nvalue \"%s\" "
                    "does not match RegEx validation \"%s\"")
                    , field.DESCRIPTION, data, regExStr)
                    , _("CustomField validation error"));
                is_valid = false;
                continue;
            }
        }

        if (GetWidgetType(controlID) == Model_CustomField::DECIMAL)
        {
            wxWindow* w = FindWindowById(controlID, m_dialog);
            if (w)
            {
                mmTextCtrl* d = static_cast<mmTextCtrl*>(w);
                double value;
                if (d->checkValue(value, false))
                    SetWidgetChanged(controlID, Model_Currency::toString(value, NULL
                                                , Model_CustomField::getDigitScale(field.PROPERTIES)));
                else
                    is_valid = false;
            }
        }
    }

    return is_valid;
}
