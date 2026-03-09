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

#include "base/defs.h"
#include <wx/timectrl.h>
#include <wx/collpane.h>
#include <wx/spinctrl.h>

#include "base/constants.h"
#include "util/_util.h"
#include "util/_simple.h"
#include "util/mmCalcValidator.h"

#include "model/CurrencyModel.h"
#include "model/FieldValueModel.h"

#include "FieldValueDialog.h"

FieldValueDialog::FieldValueDialog() :
    wxDialog()
{
}

FieldValueDialog::FieldValueDialog(
    wxDialog* dialog,
    RefTypeN ref_type,
    int64 ref_id
) :
    wxDialog(),
    m_ref_type(ref_type),
    m_ref_id(ref_id)
{
    m_dialog = dialog;
    m_field_a = FieldModel::instance().find(
        FieldCol::REFTYPE(RefTypeN::field_ref_type_n(m_ref_type).name_n())
    );
    std::sort(m_field_a.begin(), m_field_a.end(), FieldData::SorterByDESCRIPTION());
    m_data_changed.clear();
}

FieldValueDialog::~FieldValueDialog()
{
    wxLogDebug("~FieldValueDialog");
}

// TODO: refactor to FieldValueDialog()
mmCustomDataTransaction::mmCustomDataTransaction(
    wxDialog* dialog, RefTypeN ref_type, int64 ref_id, wxWindowID base_id
) :
    FieldValueDialog(dialog, ref_type, ref_id)
{
    SetBaseID(base_id);
}

bool FieldValueDialog::FillCustomFields(wxBoxSizer* box_sizer)
{
    m_static_box = new wxStaticBox(m_dialog, wxID_ANY, _t("Custom fields"));
    wxStaticBoxSizer* box_sizer_right = new wxStaticBoxSizer(m_static_box, wxVERTICAL);
    box_sizer->Add(box_sizer_right, g_flagsExpand);

    wxScrolledWindow* scrolled_window = new wxScrolledWindow(m_static_box, wxID_ANY);
    wxBoxSizer *custom_sizer = new wxBoxSizer(wxVERTICAL);
    scrolled_window->SetScrollbar(wxSB_VERTICAL, wxALIGN_RIGHT, 1, -1);
    scrolled_window->SetSizer(custom_sizer);

    wxFlexGridSizer* grid_sizer_custom = new wxFlexGridSizer(0, 2, 0, 0);
    grid_sizer_custom->AddGrowableCol(1, 1);
    custom_sizer->Add(grid_sizer_custom, g_flagsExpand);

    int field_index = 0;
    for (const auto& field_d : m_field_a) {
        bool nonDefaultData = true;
        const FieldValueData* fv_n = FieldValueModel::instance().get_key_data_n(
            field_d.m_id, m_ref_type, m_ref_id
        );
        FieldValueData fv_d;
        if (fv_n) {
            fv_d = *fv_n;
        }
        else {
            fv_d = FieldValueData();
            fv_d.m_field_id = field_d.m_id;
            fv_d.m_ref_type = m_ref_type;
            fv_d.m_ref_id   = m_ref_id;
            fv_d.m_content  = FieldModel::getDefault(field_d.m_properties);
            nonDefaultData = false;
        }

        wxWindowID controlID = GetBaseID() + field_index++ * FIELDMULTIPLIER;
        wxWindowID labelID = controlID + CONTROLOFFSET;

        wxCheckBox* Description = new wxCheckBox(
            scrolled_window,
            labelID, field_d.m_description,
            wxDefaultPosition, wxDefaultSize, wxCHK_2STATE,
            wxDefaultValidator, field_d.m_type_n.name_n()
        );
        Description->Connect(labelID, wxEVT_CHECKBOX,
            wxCommandEventHandler(FieldValueDialog::OnCheckBoxActivated), nullptr, this
        );

        grid_sizer_custom->Add(Description, g_flagsH);

        switch (field_d.m_type_n.id_n()) {
        case FieldTypeN::e_string:
        {
            const auto& data = fv_d.m_content;
            wxTextCtrl* CustomString = new wxTextCtrl(scrolled_window, controlID, data, wxDefaultPosition, wxDefaultSize);
            mmToolTip(CustomString, FieldModel::getTooltip(field_d.m_properties));
            if (FieldModel::getAutocomplete(field_d.m_properties)) {
                const wxArrayString& values = FieldModel::instance().find_id_value_a(field_d.m_id);
                CustomString->AutoComplete(values);
            }
            grid_sizer_custom->Add(CustomString, g_flagsExpand);

            if (!data.empty()) {
                if (nonDefaultData) 
                    SetWidgetChanged(controlID, data);
            }

            CustomString->Connect(controlID, wxEVT_TEXT, wxCommandEventHandler(FieldValueDialog::OnStringChanged), nullptr, this);
            break;
        }
        case FieldTypeN::e_integer:
        case FieldTypeN::e_decimal:
        {
            int digitScale = FieldModel::getDigitScale(field_d.m_properties);
            wxString content = cleanseNumberString(fv_d.m_content, digitScale > 0);

            double value;
            if (!content.ToCDouble(&value)) {
                value = 0;
            }
            else {
                if (nonDefaultData) 
                    SetWidgetChanged(controlID, CurrencyModel::toString(value, nullptr, digitScale));
            }
            
            mmTextCtrl* CustomDecimal = new mmTextCtrl(scrolled_window, controlID,
                wxEmptyString, wxDefaultPosition, wxDefaultSize,
                wxALIGN_RIGHT | wxTE_PROCESS_ENTER,
                mmCalcValidator()
            );
            CustomDecimal->SetAltPrecision(digitScale);
            CustomDecimal->SetValue(value, digitScale);
            CustomDecimal->Connect(wxID_ANY, wxEVT_TEXT,
                wxCommandEventHandler(FieldValueDialog::OnStringChanged), nullptr, this
            );

            mmToolTip(CustomDecimal, FieldModel::getTooltip(field_d.m_properties));
            grid_sizer_custom->Add(CustomDecimal, g_flagsExpand);

            break;
        }
        case FieldTypeN::e_boolean:
        {
            wxRadioButton* CustomBooleanF = new wxRadioButton(scrolled_window, controlID
                , _t("False"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
            wxRadioButton* CustomBooleanT = new wxRadioButton(scrolled_window, controlID + 1
                , _t("True"), wxDefaultPosition, wxDefaultSize);

            const auto& data = fv_d.m_content;
            if (!data.empty()) {
                data == "TRUE" ? CustomBooleanT->SetValue(true) : CustomBooleanF->SetValue(true);
                if (nonDefaultData) 
                    SetWidgetChanged(controlID, data);
            }

            mmToolTip(CustomBooleanF, FieldModel::getTooltip(field_d.m_properties));
            mmToolTip(CustomBooleanT, FieldModel::getTooltip(field_d.m_properties));
            wxBoxSizer* boolsizer = new wxBoxSizer(wxHORIZONTAL);
            boolsizer->Add(CustomBooleanF);
            boolsizer->Add(CustomBooleanT);
            grid_sizer_custom->Add(boolsizer, g_flagsExpand);

            CustomBooleanF->Connect(controlID, wxEVT_RADIOBUTTON, wxCommandEventHandler(FieldValueDialog::OnRadioButtonChanged), nullptr, this);
            CustomBooleanT->Connect(controlID + 1, wxEVT_RADIOBUTTON, wxCommandEventHandler(FieldValueDialog::OnRadioButtonChanged), nullptr, this);

            break;
        }
        case FieldTypeN::e_date:
        {
            wxDate value;
            if (!value.ParseDate(fv_d.m_content)) {
                value = wxDate::Today();
            }
            else {
                if (nonDefaultData) 
                    SetWidgetChanged(controlID, value.FormatISODate());
            }

            mmDatePickerCtrl* CustomDate = new mmDatePickerCtrl(scrolled_window, controlID, value);
            mmToolTip(CustomDate, FieldModel::getTooltip(field_d.m_properties));
            grid_sizer_custom->Add(CustomDate->mmGetLayout(false));

            CustomDate->Connect(controlID, wxEVT_DATE_CHANGED, wxDateEventHandler(FieldValueDialog::OnDateChanged), nullptr, this);

            break;
        }
        case FieldTypeN::e_time:
        {
            wxDateTime value;
            if (!value.ParseTime(fv_d.m_content)) {
                value.ParseTime("00:00:00");
            }
            else {
                if (nonDefaultData) 
                    SetWidgetChanged(controlID, value.FormatISOTime());
            }

            wxTimePickerCtrl* CustomTime = new wxTimePickerCtrl(scrolled_window, controlID
                , value, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN);
            mmToolTip(CustomTime, FieldModel::getTooltip(field_d.m_properties));
            grid_sizer_custom->Add(CustomTime, g_flagsExpand);

            CustomTime->Connect(controlID, wxEVT_TIME_CHANGED, wxDateEventHandler(FieldValueDialog::OnTimeChanged), nullptr, this);

            break;
        }
        case FieldTypeN::e_single_choice:
        {
            wxArrayString Choices = FieldModel::getChoices(field_d.m_properties);
            Choices.Sort();

            wxChoice* CustomChoice = new wxChoice(scrolled_window, controlID
                , wxDefaultPosition, wxDefaultSize, Choices);
            mmToolTip(CustomChoice, FieldModel::getTooltip(field_d.m_properties));
            grid_sizer_custom->Add(CustomChoice, g_flagsExpand);

            if (Choices.empty()) {
                CustomChoice->Enable(false);
            }

            const auto& data = fv_d.m_content;
            if (!data.empty())
            {
                CustomChoice->SetStringSelection(data);
                if (nonDefaultData) 
                    SetWidgetChanged(controlID, data);
            }

            CustomChoice->Connect(controlID, wxEVT_CHOICE, wxCommandEventHandler(FieldValueDialog::OnSingleChoice), nullptr, this);
            break;
        }
        case FieldTypeN::e_multi_choice:
        {
            const auto& content = fv_d.m_content;
            const auto& name = field_d.m_description;

            wxButton* multi_choice_button = new wxButton(scrolled_window, controlID, content
                , wxDefaultPosition, wxDefaultSize, 0L, wxDefaultValidator, name);
            mmToolTip(multi_choice_button, FieldModel::getTooltip(field_d.m_properties));
            grid_sizer_custom->Add(multi_choice_button, g_flagsExpand);

            if (!content.empty()) {
                if (nonDefaultData) 
                    SetWidgetChanged(controlID, content);
            }

            multi_choice_button->Connect(controlID, wxEVT_COMMAND_BUTTON_CLICKED
                , wxCommandEventHandler(FieldValueDialog::OnMultiChoice), nullptr, this);

            break;
        }
        default: break;
        }
    }

    scrolled_window->FitInside();
    scrolled_window->SetScrollRate(6, 6);
    box_sizer_right->Add(scrolled_window, g_flagsExpand);
    const TrxData* ref_trx_n = TrxModel::instance().get_id_data_n(m_ref_id);
    if (ref_trx_n && !ref_trx_n->DELETEDTIME.IsEmpty())
        scrolled_window->Disable();
    m_static_box->Hide();
    mmThemeAutoColour(scrolled_window);
    return true;
}

void FieldValueDialog::OnMultiChoice(wxCommandEvent& event)
{
    long controlID = event.GetId();
    auto init = event.GetInt();
    wxButton* button = static_cast<wxButton*>(m_dialog->FindWindow(controlID));
    if (!button) {
        return;
    }

    const auto& name = button->GetName();

    FieldModel::DataA field_a = FieldModel::instance().find(
        FieldCol::REFTYPE(m_ref_type.name_n()),
        FieldCol::TYPE(FieldTypeN(FieldTypeN::e_multi_choice).name_n()),
        FieldCol::DESCRIPTION(name)
    );
    wxArrayString all_choices = FieldModel::getChoices(field_a.begin()->m_properties);

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
        wxSharedPtr<wxMultiChoiceDialog> MultiChoice(new wxMultiChoiceDialog(this, _t("Please select"), _t("Multi Choice"), all_choices));
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

size_t FieldValueDialog::GetActiveCustomFieldsCount() const
{
    return FieldValueModel::instance().find(
        FieldValueCol::REFID(m_ref_id)
    ).size();
}

std::map<int64, wxString> FieldValueDialog::GetActiveCustomFields() const
{
    std::map<int64, wxString> values;
    for (const auto& entry : m_data_changed) {
        int id = (entry.first - GetBaseID()) / FIELDMULTIPLIER;
        const FieldData* field_n = FieldModel::instance().get_id_data_n(m_field_a[id].m_id);
        if (field_n) {
            values[field_n->m_id] = entry.second;
        }
    }

    return values;
}

void FieldValueDialog::SetWidgetData(wxWindowID controlID, const wxString& value)
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
    else if (class_name == "wxRadioButton")
    {
        wxRadioButton* dF = static_cast<wxRadioButton*>(w);
        wxWindow* w2 = m_dialog->FindWindowById(controlID + 1, m_dialog);
        wxRadioButton* dT = static_cast<wxRadioButton*>(w2);
        bool v = wxString("TRUE|true|1").Contains(value);
        v ? dT->SetValue(true) : dF->SetValue(true);
        wxCommandEvent evt(wxEVT_RADIOBUTTON, controlID);
        dF->GetEventHandler()->AddPendingEvent(evt);
    }
}

const wxString FieldValueDialog::GetWidgetData(wxWindowID controlID) const
{
    wxString data;
    if (m_data_changed.find(controlID) != m_data_changed.end()) {
        data = m_data_changed.at(controlID);
    }
    else {
        wxWindow* w = FindWindowById(controlID, m_dialog);
        if (w) {
            wxString class_name = w->GetEventHandler()->GetClassInfo()->GetClassName();

            if (class_name == "wxPanel") {
                wxWindow* child = w->GetChildren()[0];
                if (child)
                    class_name = child->GetEventHandler()->GetClassInfo()->GetClassName();
            }

            if (class_name == "wxDatePickerCtrl") {
                mmDatePickerCtrl* d = static_cast<mmDatePickerCtrl*>(w);
                data = d->GetValue().FormatISODate();
            }
            else if (class_name == "wxTimePickerCtrl") {
                wxTimePickerCtrl* d = static_cast<wxTimePickerCtrl*>(w);
                data = d->GetValue().FormatISOTime();
            }
            else if (class_name == "wxChoice") {
                wxChoice* d = static_cast<wxChoice*>(w);
                data = d->GetStringSelection();
            }
            else if (class_name == "wxButton") {
                wxButton* d = static_cast<wxButton*>(w);
                data = d->GetLabel();
            }
            else if (class_name == "wxTextCtrl") {
                wxTextCtrl* d = static_cast<wxTextCtrl*>(w);
                data = d->GetValue();
            }
            else if (class_name == "wxRadioButton") {
                wxRadioButton* d = static_cast<wxRadioButton*>(w);
                data = (d->GetValue() ? "FALSE" : "TRUE");
            }
        }
    }
    return data;
}

bool FieldValueDialog::SaveCustomValues(RefTypeN ref_type, int64 ref_id)
{
    bool changed = false;
    FieldValueModel::instance().db_savepoint();
    int field_index = 0;
    for (const auto& field_d : m_field_a) {
        wxWindowID controlID = GetBaseID() + (field_index++) * FIELDMULTIPLIER;
        const wxString data = IsWidgetChanged(controlID) ? GetWidgetData(controlID) : "";

        const FieldValueData* fv_n = FieldValueModel::instance().get_key_data_n(
            field_d.m_id, ref_type, ref_id
        );
        if (!data.empty()) {
            FieldValueData old_fv_d = fv_n ? *fv_n : FieldValueData();
            FieldValueData fv_d = fv_n ? *fv_n : FieldValueData();
            fv_d.m_field_id = field_d.m_id;
            fv_d.m_ref_type = ref_type;
            fv_d.m_ref_id   = ref_id;
            fv_d.m_content  = data;
            wxLogDebug("Control:%i Type:%s Value:%s",
                controlID,
                field_d.m_type_n.name_n(),
                data
            );

            if (fv_n && !fv_d.equals(&old_fv_d))
                changed = true;

            FieldValueModel::instance().save_data_n(fv_d);
        }
        else if (fv_n) {
            FieldValueModel::instance().purge_id(fv_n->m_id);
            changed = true;
        }
    }
    FieldValueModel::instance().db_release_savepoint();

    if (ref_type.id_n() == TrxModel::s_ref_type.id_n() && changed)
        TrxModel::instance().save_timestamp(ref_id);        

    return true;
}

void FieldValueDialog::UpdateCustomValues(RefTypeN ref_type, int64 ref_id)
{
    bool changed = false;
    FieldValueModel::instance().db_savepoint();
    int field_index = 0;
    for (const auto& field_d : m_field_a) {
        wxWindowID controlID = GetBaseID() + (field_index++) * FIELDMULTIPLIER;
        auto label_id = controlID + CONTROLOFFSET;
        wxCheckBox* label_cb = static_cast<wxCheckBox*>(m_dialog->FindWindow(label_id));
        if (!label_cb || !label_cb->GetValue())
            continue;

        const wxString data = GetWidgetData(controlID);
        const FieldValueData* fv_n = FieldValueModel::instance().get_key_data_n(
            field_d.m_id, ref_type, ref_id
        );
        if (!data.empty()) {
            FieldValueData old_fv_d = fv_n ? *fv_n : FieldValueData();
            FieldValueData fv_d = fv_n ? *fv_n : FieldValueData();
            fv_d.m_field_id = field_d.m_id;
            fv_d.m_ref_type = ref_type;
            fv_d.m_ref_id   = ref_id;
            fv_d.m_content  = data;

            if (!fv_d.equals(&old_fv_d))
                changed = true;

            FieldValueModel::instance().save_data_n(fv_d);
        }
        else if (fv_n) {
            FieldValueModel::instance().purge_id(fv_n->m_id);
            changed = true;
        }
    }
    FieldValueModel::instance().db_release_savepoint();

    if (ref_type.id_n() == TrxModel::s_ref_type.id_n() && changed)
        TrxModel::instance().save_timestamp(ref_id);        
}

void FieldValueDialog::OnStringChanged(wxCommandEvent& event)
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

void FieldValueDialog::ResetWidgetChanged(wxWindowID id)
{
    auto label_id = id + CONTROLOFFSET;
    wxCheckBox* check_box = static_cast<wxCheckBox*>(m_dialog->FindWindow(label_id));
    if (check_box) {
        check_box->SetValue(false);
    }
    m_data_changed.erase(id);
}

void FieldValueDialog::ResetWidgetsChanged()
{
    for (const auto& entry : m_data_changed)
    {
        auto label_id = entry.first + CONTROLOFFSET;
        wxCheckBox* check_box = static_cast<wxCheckBox*>(m_dialog->FindWindow(label_id));
        if (check_box) {
            check_box->SetValue(false);
            wxLogDebug("Description %i value = %s", label_id, "FALSE");
        }
    }

    m_data_changed.clear();
}

void FieldValueDialog::ClearSettings()
{
    for (unsigned int field_index = 0 ; field_index < m_field_a.size() ; field_index++ )
    {
        SetStringValue(field_index, "");
        wxWindowID labelID = GetBaseID() + field_index * FIELDMULTIPLIER + CONTROLOFFSET;
        wxCheckBox* cb = static_cast<wxCheckBox*>(FindWindowById(labelID, m_dialog));
        if (cb)
            cb->SetValue(false);
    }
}

void FieldValueDialog::OnSingleChoice(wxCommandEvent& event)
{
    const wxString& data = event.GetString();
    SetWidgetChanged(event.GetId(), data);
}

void FieldValueDialog::OnRadioButtonChanged(wxCommandEvent& event)
{
    int winID = event.GetId();
    int winOffset = (winID - GetBaseID()) % FIELDMULTIPLIER;
    wxRadioButton* button = static_cast<wxRadioButton*>(m_dialog->FindWindow(winID));
    const auto& data = ((winOffset == 0) && button->GetValue()) ? "FALSE" : "TRUE";
    SetWidgetChanged(winID - winOffset, data);
}

int FieldValueDialog::GetWidgetType(wxWindowID controlID) const
{
    int control_id = (controlID - GetBaseID()) / FIELDMULTIPLIER;
    for (const auto& field_d : FieldModel::instance().find(
        FieldCol::REFTYPE(m_ref_type.name_n())
    )) {
        if (field_d.m_id == m_field_a[control_id].m_id) {
            return field_d.m_type_n.id_n();
        }
    }
    wxFAIL_MSG("unknown custom field type");
    return -1;
}

int FieldValueDialog::GetPrecision(wxWindowID controlID) const
{
    int control_id = (controlID - GetBaseID()) / FIELDMULTIPLIER;
    for (const auto& field_d : m_field_a)
    if (field_d.m_id == m_field_a[control_id].m_id)
            return (FieldModel::getDigitScale(field_d.m_properties));
    wxFAIL_MSG("No field found");
    return -1;
}

void FieldValueDialog::OnCheckBoxActivated(wxCommandEvent& event)
{
    auto id = event.GetId();
    auto widget_id = id - CONTROLOFFSET;
    auto checked = event.IsChecked();

    if (checked) {
        //TODO:
        const wxString data = GetWidgetData(widget_id);
        SetWidgetChanged(widget_id, data);
    }
    else {
        this->ResetWidgetChanged(widget_id);
    }
}

void FieldValueDialog::OnDateChanged(wxDateEvent& event)
{
    const auto data = event.GetDate();
    SetWidgetChanged(event.GetId(), data.FormatISODate());
    event.Skip();
}

void FieldValueDialog::OnTimeChanged(wxDateEvent& event)
{
    auto data = event.GetDate();
    SetWidgetChanged(event.GetId(), data.FormatISOTime());
}

bool FieldValueDialog::IsWidgetChanged(wxWindowID id)
{
    const wxString& value = (m_data_changed.find(id) == m_data_changed.end())
        ? "" : m_data_changed.at(id);
    return !value.empty();
}

bool FieldValueDialog::IsSomeWidgetChanged() const
{
    for (const auto& entry : m_data_changed)
    {
        if (!entry.second.empty()) return true;
    }
    return false;
}

void FieldValueDialog::SetWidgetChanged(wxWindowID id, const wxString& data)
{
    m_data_changed[id] = data;

    auto label_id = id + CONTROLOFFSET;
    wxCheckBox* check_box = static_cast<wxCheckBox*>(m_dialog->FindWindow(label_id));
    if (check_box) {
        check_box->SetValue(true);
        wxLogDebug("[V] %s = %s", check_box->GetLabel(), data);
    }
}

bool FieldValueDialog::IsDataFound(const TrxModel::Full_Data& trx_xd)
{
    const auto& fv_a = FieldValueModel::instance().find(
        FieldValueCol::REFID(trx_xd.m_id)
    );
    for (const auto& filter : m_data_changed) {
        for (const auto& fv_d : fv_a) {
            if (filter.second == fv_d.m_content) {
                return true;
            }
        }

    }
    return false;
}

bool FieldValueDialog::IsCustomPanelShown() const
{
    return m_static_box->IsShown();
}

void FieldValueDialog::ShowHideCustomPanel() const
{
    if (IsCustomPanelShown()) {
        m_static_box->Hide();
    }
    else {
        if (!m_field_a.empty())
            m_static_box->Show();
    }
}

void FieldValueDialog::ShowCustomPanel() const
{
    m_static_box->Show();
}

void FieldValueDialog::SetStringValue(int fieldIndex, const wxString& value, bool hasChanged)
{
    wxWindowID widget_id = GetBaseID() + fieldIndex * FIELDMULTIPLIER;
    SetWidgetData(widget_id, value);
    if (hasChanged)
         SetWidgetChanged(widget_id, value);
}

bool FieldValueDialog::ValidateCustomValues(int64)
{
    bool is_valid = true;
    int field_index = 0;
    for (const auto& field_d : m_field_a) {
        wxWindowID controlID = GetBaseID() + (field_index++) * FIELDMULTIPLIER;
        wxWindowID labelID = controlID + CONTROLOFFSET;

        wxCheckBox* cb = static_cast<wxCheckBox*>(FindWindowById(labelID, m_dialog));
        if (!cb || !cb->GetValue())
            continue;

        if (GetWidgetType(controlID) == FieldTypeN::e_decimal ||
            GetWidgetType(controlID) == FieldTypeN::e_integer
        ) {
            wxWindow* w = FindWindowById(controlID, m_dialog);
            if (w) {
                mmTextCtrl* d = static_cast<mmTextCtrl*>(w);
                double value;
                if (d->checkValue(value, false))
                    SetWidgetChanged(controlID,
                        CurrencyModel::toString(value, nullptr,
                            FieldModel::getDigitScale(field_d.m_properties)
                        )
                    );
                else
                    is_valid = false;
            }
        }

        const wxString regExStr = FieldModel::getRegEx(field_d.m_properties);
        if (!regExStr.empty()) {
            const wxString data = GetWidgetData(controlID);
            wxRegEx regEx(regExStr, wxRE_EXTENDED);

            if (!regEx.Matches(data)) {
                mmErrorDialogs::MessageError(this,
                    wxString::Format(
                        _t("Unable to save custom field \"%1$s\":\nvalue \"%2$s\" "
                            "does not match RegEx validation \"%3$s\""
                        ),
                        field_d.m_description, data, regExStr
                    ),
                    _t("CustomField validation error")
                );
                is_valid = false;
                continue;
            }
        }
    }

    return is_valid;
}
