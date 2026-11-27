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

#include "base/_defs.h"
#include <wx/collpane.h>

#include "base/_constants.h"
#include "util/mmDatePicker.h"
#include "util/mmCalcValidator.h"
#include "util/_util.h"
#include "util/_simple.h"

#include "model/CurrencyModel.h"
#include "model/FieldValueModel.h"

#include "FieldValueDialog.h"

// -- constructor

FieldValueDialog::FieldValueDialog() :
    wxDialog()
{
}

FieldValueDialog::FieldValueDialog(
    wxDialog* dlg,
    RefTypeN ref_type,
    int64 ref_id
) :
    wxDialog(),
    m_ref_type(ref_type),
    m_ref_id(ref_id),
    w_dlg(dlg)
{
    m_field_a = FieldModel::instance().find_data_a(
        FieldCol::WHERE_REFTYPE(OP_EQ, RefTypeN::field_ref_type_n(m_ref_type).key_n()),
        TableClause::ORDERBY(FieldCol::NAME_DESCRIPTION)
    );
    m_data_changed.clear();
}

FieldValueDialog::FieldValueDialog(
    wxDialog* dlg,
    RefTypeN ref_type,
    int64 ref_id,
    wxWindowID base_id
) :
    FieldValueDialog(dlg, ref_type, ref_id)
{
    m_base_id = base_id;
}

FieldValueDialog::~FieldValueDialog()
{
    wxLogDebug("~FieldValueDialog");
}

// -- methods

bool FieldValueDialog::FillCustomFields(wxBoxSizer* box_sizer)
{
    w_box = new wxStaticBox(w_dlg, wxID_ANY, _t("Custom fields"));
    wxStaticBoxSizer* box_sizer_right = new wxStaticBoxSizer(w_box, wxVERTICAL);
    box_sizer->Add(box_sizer_right, g_flagsExpand);

    wxScrolledWindow* scrolled_window = new wxScrolledWindow(w_box, wxID_ANY);
    wxBoxSizer *custom_sizer = new wxBoxSizer(wxVERTICAL);
    scrolled_window->SetScrollbar(wxSB_VERTICAL, wxALIGN_RIGHT, 1, -1);
    scrolled_window->SetSizer(custom_sizer);

    wxFlexGridSizer* grid_sizer_custom = new wxFlexGridSizer(0, 2, 0, 0);
    grid_sizer_custom->AddGrowableCol(1, 1);
    custom_sizer->Add(grid_sizer_custom, g_flagsExpand);

    int field_index = 0;
    for (const auto& field_d : m_field_a) {
        wxWindowID win_id = m_base_id + (field_index++) * FIELDMULTIPLIER;

        const FieldValueData* fv_n = FieldValueModel::instance().get_key_data_n(
            field_d.m_id, m_ref_type, m_ref_id
        );
        wxString content = fv_n
            ? fv_n->m_content
            : FieldModel::getDefault(field_d.m_properties);

        // TODO: field_d.m_type_n: use translated name_n() instead of key_n()
        wxCheckBox* Description = new wxCheckBox(
            scrolled_window,
            win_id + CONTROLOFFSET,
            field_d.m_description,
            wxDefaultPosition, wxDefaultSize, wxCHK_2STATE,
            wxDefaultValidator,
            field_d.m_type_n.key_n()
        );
        Description->Connect(
            win_id + CONTROLOFFSET,
            wxEVT_CHECKBOX,
            wxCommandEventHandler(FieldValueDialog::OnCheckBoxActivated),
            nullptr, this
        );

        grid_sizer_custom->Add(Description, g_flagsH);

        switch (field_d.m_type_n.id_n())
        {
        case FieldTypeN::e_string: {
            wxTextCtrl* CustomString = new wxTextCtrl(
                scrolled_window, win_id,
                content,
                wxDefaultPosition, wxDefaultSize
            );
            mmToolTip(CustomString, FieldModel::getTooltip(field_d.m_properties));
            if (FieldModel::getAutocomplete(field_d.m_properties)) {
                const wxArrayString& value_a = FieldModel::instance().find_id_value_a(
                    field_d.m_id
                );
                CustomString->AutoComplete(value_a);
            }
            grid_sizer_custom->Add(CustomString, g_flagsExpand);

            if (!content.empty() && fv_n) {
                SetWidgetChanged(win_id, content);
            }

            CustomString->Connect(win_id, wxEVT_TEXT,
                wxCommandEventHandler(FieldValueDialog::OnStringChanged),
                nullptr, this
            );
            break;
        }
        case FieldTypeN::e_integer:
        case FieldTypeN::e_decimal: {
            int digitScale = FieldModel::getDigitScale(field_d.m_properties);
            content = cleanseNumberString(content, digitScale > 0);

            double value;
            if (!content.ToCDouble(&value)) {
                value = 0;
            }
            else if (fv_n) {
                SetWidgetChanged(win_id, CurrencyModel::instance().toString(
                    value, nullptr, digitScale
                ));
            }

            mmTextCtrl* text = new mmTextCtrl(scrolled_window, win_id,
                wxEmptyString,
                wxDefaultPosition, wxDefaultSize,
                wxALIGN_RIGHT | wxTE_PROCESS_ENTER,
                mmCalcValidator()
            );
            text->SetAltPrecision(digitScale);
            text->SetValue(value, digitScale);
            text->Connect(wxID_ANY, wxEVT_TEXT,
                wxCommandEventHandler(FieldValueDialog::OnStringChanged),
                nullptr, this
            );
            mmToolTip(text, FieldModel::getTooltip(field_d.m_properties));
            grid_sizer_custom->Add(text, g_flagsExpand);

            break;
        }
        case FieldTypeN::e_boolean: {
            wxRadioButton* false_btn = new wxRadioButton(scrolled_window, win_id,
                _t("False"),
                wxDefaultPosition, wxDefaultSize, wxRB_GROUP
            );
            wxRadioButton* true_btn = new wxRadioButton(scrolled_window, win_id + 1,
                _t("True"),
                wxDefaultPosition, wxDefaultSize
            );

            if (!content.empty()) {
                content == "TRUE"
                    ? true_btn->SetValue(true)
                    : false_btn->SetValue(true);
                if (fv_n)
                    SetWidgetChanged(win_id, content);
            }

            mmToolTip(false_btn, FieldModel::getTooltip(field_d.m_properties));
            mmToolTip(true_btn, FieldModel::getTooltip(field_d.m_properties));
            wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->Add(false_btn);
            sizer->Add(true_btn);
            grid_sizer_custom->Add(sizer, g_flagsExpand);

            false_btn->Connect(
                win_id,
                wxEVT_RADIOBUTTON,
                wxCommandEventHandler(FieldValueDialog::OnRadioButtonChanged),
                nullptr, this
            );
            true_btn->Connect(
                win_id + 1,
                wxEVT_RADIOBUTTON,
                wxCommandEventHandler(FieldValueDialog::OnRadioButtonChanged),
                nullptr, this
            );

            break;
        }
        case FieldTypeN::e_date: {
            mmDateN date_n = mmDateN(content);
            if (date_n.has_value() && fv_n) {
                SetWidgetChanged(win_id, date_n.value().isoDate());
            }

            mmDatePicker* CustomDate = new mmDatePicker(
                scrolled_window, win_id,
                date_n.value_or(mmDate::today()).dateTime()
            );
            mmToolTip(CustomDate, FieldModel::getTooltip(field_d.m_properties));
            grid_sizer_custom->Add(CustomDate->mmGetLayout(false));

            CustomDate->Connect(win_id,
                wxEVT_DATE_CHANGED,
                wxDateEventHandler(FieldValueDialog::OnDateChanged),
                nullptr, this
            );

            break;
        }
        case FieldTypeN::e_time: {
            wxDateTime value;
            if (!value.ParseTime(content)) {
                value.ParseTime("00:00:00");
            }
            else if (fv_n) {
                SetWidgetChanged(win_id, value.FormatISOTime());
            }

            wxTimePickerCtrl* CustomTime = new wxTimePickerCtrl(
                scrolled_window, win_id,
                value,
                wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN
            );
            mmToolTip(CustomTime, FieldModel::getTooltip(field_d.m_properties));
            grid_sizer_custom->Add(CustomTime, g_flagsExpand);

            CustomTime->Connect(win_id,
                wxEVT_TIME_CHANGED,
                wxDateEventHandler(FieldValueDialog::OnTimeChanged),
                nullptr, this
            );

            break;
        }
        case FieldTypeN::e_single_choice: {
            wxArrayString choice_a = FieldModel::getChoices(field_d.m_properties);
            choice_a.Sort();

            wxChoice* single_choice = new wxChoice(scrolled_window, win_id,
                wxDefaultPosition, wxDefaultSize,
                choice_a
            );
            mmToolTip(single_choice, FieldModel::getTooltip(field_d.m_properties));
            grid_sizer_custom->Add(single_choice, g_flagsExpand);

            if (choice_a.empty()) {
                single_choice->Enable(false);
            }

            if (!content.empty()) {
                single_choice->SetStringSelection(content);
                if (fv_n)
                    SetWidgetChanged(win_id, content);
            }

            single_choice->Connect(win_id,
                wxEVT_CHOICE,
                wxCommandEventHandler(FieldValueDialog::OnSingleChoice),
                nullptr, this
            );
            break;
        }
        case FieldTypeN::e_multi_choice: {
            wxButton* multi_btn = new wxButton(
                scrolled_window, win_id,
                content,
                wxDefaultPosition, wxDefaultSize,
                0L,
                wxDefaultValidator,
                field_d.m_description
            );
            mmToolTip(multi_btn, FieldModel::getTooltip(field_d.m_properties));
            grid_sizer_custom->Add(multi_btn, g_flagsExpand);

            if (!content.empty() && fv_n) {
                SetWidgetChanged(win_id, content);
            }

            multi_btn->Connect(win_id,
                wxEVT_COMMAND_BUTTON_CLICKED,
                wxCommandEventHandler(FieldValueDialog::OnMultiChoice),
                nullptr, this
            );

            break;
        }
        default:
            break;
        }
    }

    scrolled_window->FitInside();
    scrolled_window->SetScrollRate(6, 6);
    box_sizer_right->Add(scrolled_window, g_flagsExpand);
    const TrxData* ref_trx_n = TrxModel::instance().get_idN_data_n(m_ref_id);
    if (ref_trx_n && ref_trx_n->is_deleted())
        scrolled_window->Disable();
    w_box->Hide();
    mmThemeAutoColour(scrolled_window);
    return true;
}

void FieldValueDialog::OnMultiChoice(wxCommandEvent& event)
{
    long win_id = event.GetId();
    auto init = event.GetInt();
    wxButton* button = static_cast<wxButton*>(w_dlg->FindWindow(win_id));
    if (!button) {
        return;
    }

    const auto& description = button->GetName();
    wxArrayString all_choices;
    for (const FieldData& field_d : FieldModel::instance().find_data_a(
        FieldCol::WHERE_REFTYPE(OP_EQ, RefTypeN::field_ref_type_n(m_ref_type).key_n()),
        FieldCol::WHERE_DESCRIPTION(OP_EQ, description),
        FieldCol::WHERE_TYPE(OP_EQ, FieldTypeN(FieldTypeN::e_multi_choice).key_n())
    )) {
        all_choices = FieldModel::getChoices(field_d.m_properties);
    }

    const wxString& label = button->GetLabelText();
    wxArrayInt arr_selections;
    int i = 0;
    for (const wxString& choice : all_choices) {
        if (label.Contains(choice)) {
            arr_selections.Add(i);
        }
        i++;
    }

    wxString data = label;
    if (init != -1) {
        wxSharedPtr<wxMultiChoiceDialog> MultiChoice(new wxMultiChoiceDialog(this,
            _t("Please select"),
            _t("Multi Choice"),
            all_choices
        ));
        MultiChoice->SetSelections(arr_selections);

        if (MultiChoice->ShowModal() == wxID_OK) {
            data.clear();
            for (const auto& s : MultiChoice->GetSelections()) {
                data += all_choices[s] + ";";
            }
            data.RemoveLast();
        }
    }
    button->SetLabel(data);
    SetWidgetChanged(win_id, data);
}

size_t FieldValueDialog::GetActiveCustomFieldsCount() const
{
    return FieldValueModel::instance().find_count(
        FieldValueModel::WHERE_REFTYPEID(m_ref_type, m_ref_id)
    );
}

std::map<int64, wxString> FieldValueDialog::GetActiveCustomFields() const
{
    std::map<int64, wxString> values;
    for (const auto& entry : m_data_changed) {
        int i = (entry.first - m_base_id) / FIELDMULTIPLIER;
        const FieldData* field_n = FieldModel::instance().get_idN_data_n(
            m_field_a[i].m_id
        );
        if (field_n) {
            values[field_n->m_id] = entry.second;
        }
    }

    return values;
}

void FieldValueDialog::SetWidgetData(wxWindowID win_id, const wxString& value)
{
    wxWindow* w = w_dlg->FindWindowById(win_id, w_dlg);
    if (!w)
        return;

    if (value.empty()) {
        ResetWidgetChanged(win_id);
        return;
    }

    const wxString class_name = w->GetEventHandler()->GetClassInfo()->GetClassName();

    if (class_name == "wxDatePickerCtrl") {
        mmDatePicker* d = static_cast<mmDatePicker*>(w);
        wxDateTime date;
        mmParseISODate(value, date);
        d->setValue(date);
        wxDateEvent evt(d, date, wxEVT_DATE_CHANGED);
        d->GetEventHandler()->AddPendingEvent(evt);
    }
    else if (class_name == "wxTimePickerCtrl") {
        wxTimePickerCtrl* d = static_cast<wxTimePickerCtrl*>(w);
        wxDateTime time;
        time.ParseTime(value);
        d->SetValue(time);
        wxDateEvent evt(d, time, wxEVT_TIME_CHANGED);
        d->GetEventHandler()->AddPendingEvent(evt);
    }
    else if (class_name == "wxChoice") {
        wxChoice* d = static_cast<wxChoice*>(w);
        d->SetStringSelection(value);
        wxCommandEvent evt(wxEVT_CHOICE, win_id);
        evt.SetString(value);
        d->GetEventHandler()->AddPendingEvent(evt);
    }
    else if (class_name == "wxButton") {
        wxButton* d = static_cast<wxButton*>(w);
        d->SetLabel(value);

        wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, win_id);
        evt.SetInt(-1);
        d->GetEventHandler()->AddPendingEvent(evt);

    }
    else if (class_name == "wxTextCtrl") {
        wxTextCtrl* d = static_cast<wxTextCtrl*>(w);
        d->SetValue(value);
    }
    else if (class_name == "wxRadioButton") {
        wxRadioButton* dF = static_cast<wxRadioButton*>(w);
        wxWindow* w2 = w_dlg->FindWindowById(win_id + 1, w_dlg);
        wxRadioButton* dT = static_cast<wxRadioButton*>(w2);
        if (wxString("TRUE|true|1").Contains(value))
            dT->SetValue(true);
        else
            dF->SetValue(true);
        wxCommandEvent evt(wxEVT_RADIOBUTTON, win_id);
        dF->GetEventHandler()->AddPendingEvent(evt);
    }
}

const wxString FieldValueDialog::GetWidgetData(wxWindowID win_id) const
{
    if (m_data_changed.find(win_id) != m_data_changed.end())
        return m_data_changed.at(win_id);

    wxString data;
    wxWindow* w = FindWindowById(win_id, w_dlg);
    if (!w)
        return data;

    wxString class_name = w->GetEventHandler()->GetClassInfo()->GetClassName();

    if (class_name == "wxPanel") {
        wxWindow* child = w->GetChildren()[0];
        if (child)
            class_name = child->GetEventHandler()->GetClassInfo()->GetClassName();
    }

    if (class_name == "wxDatePickerCtrl") {
        data = static_cast<mmDatePicker*>(w)->GetValue().FormatISODate();
    }
    else if (class_name == "wxTimePickerCtrl") {
        data = static_cast<wxTimePickerCtrl*>(w)->GetValue().FormatISOTime();
    }
    else if (class_name == "wxChoice") {
        data = static_cast<wxChoice*>(w)->GetStringSelection();
    }
    else if (class_name == "wxButton") {
        data = static_cast<wxButton*>(w)->GetLabel();
    }
    else if (class_name == "wxTextCtrl") {
        data = static_cast<wxTextCtrl*>(w)->GetValue();
    }
    else if (class_name == "wxRadioButton") {
        data = static_cast<wxRadioButton*>(w)->GetValue() ? "FALSE" : "TRUE";
    }

    return data;
}

bool FieldValueDialog::SaveCustomValues(RefTypeN ref_type, int64 ref_id)
{
    bool changed = false;

    FieldValueModel::instance().db_savepoint();
    int field_index = 0;
    for (const auto& field_d : m_field_a) {
        const FieldValueData* fv_n = FieldValueModel::instance().get_key_data_n(
            field_d.m_id, ref_type, ref_id
        );

        wxWindowID win_id = m_base_id + (field_index++) * FIELDMULTIPLIER;
        const wxString content = IsWidgetChanged(win_id) ? GetWidgetData(win_id) : "";
        if (!content.empty()) {
            FieldValueData fv_d = fv_n ? *fv_n : FieldValueData();
            fv_d.m_field_id = field_d.m_id;
            fv_d.m_ref_type = ref_type;
            fv_d.m_ref_id   = ref_id;
            fv_d.m_content  = content;
            wxLogDebug("Control:%i Type:%s Value:%s",
                win_id,
                field_d.m_type_n.key_n(),
                content
            );

            if (!fv_n || !fv_d.equals(fv_n))
                changed = true;

            FieldValueModel::instance().save_data_n(fv_d);
        }
        else if (fv_n) {
            FieldValueModel::instance().purge_id(fv_n->m_id);
            changed = true;
        }
    }
    FieldValueModel::instance().db_release_savepoint();

    if (ref_type == TrxModel::s_ref_type && changed)
        TrxModel::instance().save_timestamp(ref_id);

    return true;
}

void FieldValueDialog::UpdateCustomValues(RefTypeN ref_type, int64 ref_id)
{
    bool changed = false;

    FieldValueModel::instance().db_savepoint();
    int field_index = 0;
    for (const auto& field_d : m_field_a) {
        const FieldValueData* fv_n = FieldValueModel::instance().get_key_data_n(
            field_d.m_id, ref_type, ref_id
        );

        wxWindowID win_id = m_base_id + (field_index++) * FIELDMULTIPLIER;
        wxCheckBox* field_cb = static_cast<wxCheckBox*>(w_dlg->FindWindow(
            win_id + CONTROLOFFSET
        ));
        if (!field_cb || !field_cb->GetValue())
            continue;

        const wxString content = GetWidgetData(win_id);
        if (!content.empty()) {
            FieldValueData fv_d = fv_n ? *fv_n : FieldValueData();
            fv_d.m_field_id = field_d.m_id;
            fv_d.m_ref_type = ref_type;
            fv_d.m_ref_id   = ref_id;
            fv_d.m_content  = content;

            if (!fv_n || !fv_d.equals(fv_n))
                changed = true;

            FieldValueModel::instance().save_data_n(fv_d);
        }
        else if (fv_n) {
            FieldValueModel::instance().purge_id(fv_n->m_id);
            changed = true;
        }
    }
    FieldValueModel::instance().db_release_savepoint();

    if (ref_type == TrxModel::s_ref_type && changed)
        TrxModel::instance().save_timestamp(ref_id);
}

void FieldValueDialog::OnStringChanged(wxCommandEvent& event)
{
    int win_id = event.GetId();
    wxString data = event.GetString();

    if (data.empty()) {
        ResetWidgetChanged(win_id);
    }
    else {
        SetWidgetChanged(win_id, data);
    }
}

void FieldValueDialog::ResetWidgetChanged(wxWindowID win_id)
{
    wxCheckBox* check_box = static_cast<wxCheckBox*>(w_dlg->FindWindow(
        win_id + CONTROLOFFSET
    ));
    if (check_box) {
        check_box->SetValue(false);
    }
    m_data_changed.erase(win_id);
}

void FieldValueDialog::ResetWidgetsChanged()
{
    for (const auto& entry : m_data_changed) {
        auto label_id = entry.first + CONTROLOFFSET;
        wxCheckBox* check_box = static_cast<wxCheckBox*>(w_dlg->FindWindow(label_id));
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
        wxWindowID labelID = m_base_id + field_index * FIELDMULTIPLIER + CONTROLOFFSET;
        wxCheckBox* cb = static_cast<wxCheckBox*>(FindWindowById(labelID, w_dlg));
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
    int win_id = event.GetId();
    int win_offset = (win_id - m_base_id) % FIELDMULTIPLIER;
    wxRadioButton* button = static_cast<wxRadioButton*>(w_dlg->FindWindow(win_id));
    const wxString& data = (win_offset == 0 && button->GetValue()) ? "FALSE" : "TRUE";
    SetWidgetChanged(win_id - win_offset, data);
}

int FieldValueDialog::GetWidgetType(wxWindowID win_id) const
{
    int field_index = (win_id - m_base_id) / FIELDMULTIPLIER;
    int64 field_id = m_field_a[field_index].m_id;
    // TODO: FieldModel::instance().get_id_data_n()
    for (const FieldData& field_d : FieldModel::instance().find_data_a(
        FieldCol::WHERE_FIELDID(OP_EQ, field_id)
    )) {
        return field_d.m_type_n.id_n();
    }
    wxFAIL_MSG("unknown custom field type");
    return -1;
}

int FieldValueDialog::GetPrecision(wxWindowID controlID) const
{
    int control_id = (controlID - m_base_id) / FIELDMULTIPLIER;
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
    return (
        m_data_changed.find(id) != m_data_changed.end() &&
        !m_data_changed.at(id).IsEmpty()
    );
}

bool FieldValueDialog::IsSomeWidgetChanged() const
{
    for (const auto& winId_content : m_data_changed) {
        if (!winId_content.second.IsEmpty())
            return true;
    }
    return false;
}

void FieldValueDialog::SetWidgetChanged(wxWindowID win_id, const wxString& data)
{
    m_data_changed[win_id] = data;

    wxCheckBox* check_box = static_cast<wxCheckBox*>(w_dlg->FindWindow(
        win_id + CONTROLOFFSET
    ));
    if (check_box) {
        check_box->SetValue(true);
        wxLogDebug("[V] %s = %s", check_box->GetLabel(), data);
    }
}

bool FieldValueDialog::IsDataFound(const TrxModel::DataExt& trx_dx)
{
    const FieldValueModel::DataA fv_a = FieldValueModel::instance().find_data_a(
        FieldValueModel::WHERE_REFTYPEID(TrxModel::s_ref_type, trx_dx.m_id)
    );
    for (const auto& filter : m_data_changed) {
        for (const FieldValueData& fv_d : fv_a) {
            if (filter.second == fv_d.m_content) {
                return true;
            }
        }
    }
    return false;
}

bool FieldValueDialog::IsCustomPanelShown() const
{
    return w_box->IsShown();
}

void FieldValueDialog::ShowHideCustomPanel() const
{
    if (IsCustomPanelShown()) {
        w_box->Hide();
    }
    else {
        if (!m_field_a.empty())
            w_box->Show();
    }
}

void FieldValueDialog::ShowCustomPanel() const
{
    w_box->Show();
}

void FieldValueDialog::SetStringValue(int fieldIndex, const wxString& value, bool hasChanged)
{
    wxWindowID widget_id = m_base_id + fieldIndex * FIELDMULTIPLIER;
    SetWidgetData(widget_id, value);
    if (hasChanged)
         SetWidgetChanged(widget_id, value);
}

bool FieldValueDialog::ValidateCustomValues()
{
    bool is_valid = true;
    int field_index = 0;
    for (const auto& field_d : m_field_a) {
        wxWindowID controlID = m_base_id + (field_index++) * FIELDMULTIPLIER;
        wxWindowID labelID = controlID + CONTROLOFFSET;

        wxCheckBox* cb = static_cast<wxCheckBox*>(FindWindowById(labelID, w_dlg));
        if (!cb || !cb->GetValue())
            continue;

        if (GetWidgetType(controlID) == FieldTypeN::e_decimal ||
            GetWidgetType(controlID) == FieldTypeN::e_integer
        ) {
            wxWindow* w = FindWindowById(controlID, w_dlg);
            if (w) {
                mmTextCtrl* d = static_cast<mmTextCtrl*>(w);
                double value;
                if (d->checkValue(value, false))
                    SetWidgetChanged(controlID,
                        CurrencyModel::instance().toString(value, nullptr,
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
