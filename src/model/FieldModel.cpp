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

#include "base/defs.h"
#include <wx/string.h>

#include "FieldModel.h"
#include "FieldValueModel.h"
#include "TrxModel.h"

FieldModel::FieldModel() :
    TableFactory<FieldTable, FieldData>()
{
}

FieldModel::~FieldModel()
{
}

// Initialize the global FieldModel table.
// Reset the FieldModel table or create the table if it does not exist.
FieldModel& FieldModel::instance(wxSQLite3Database* db)
{
    FieldModel& ins = Singleton<FieldModel>::instance();
    ins.reset_cache();
    ins.m_db = db;
    ins.ensure_table();

    return ins;
}

// Return the static instance of FieldModel table
FieldModel& FieldModel::instance()
{
    return Singleton<FieldModel>::instance();
}

const wxArrayString FieldModel::UDFC_FIELDS()
{
    wxArrayString choices;
    choices.Add("");
    choices.Add("UDFC01");
    choices.Add("UDFC02");
    choices.Add("UDFC03");
    choices.Add("UDFC04");
    choices.Add("UDFC05");
    return choices;
}

const wxString FieldModel::getUDFC(const wxString& properties)
{
    Document json_doc;
    if (!json_doc.Parse(properties.utf8_str()).HasParseError() &&
        json_doc.HasMember("UDFC") && json_doc["UDFC"].IsString()
    ) {
        Value& s = json_doc["UDFC"];
        return s.GetString();
    }
    return "";
}

const wxString FieldModel::getRegEx(const wxString& properties)
{
    Document json_doc;
    if (!json_doc.Parse(properties.utf8_str()).HasParseError() &&
        json_doc.HasMember("RegEx") && json_doc["RegEx"].IsString()
    ) {
        Value& s = json_doc["RegEx"];
        return wxString::FromUTF8Unchecked(s.GetString());
    }
    return "";
}

const wxString FieldModel::getTooltip(const wxString& properties)
{
    Document json_doc;
    if (!json_doc.Parse(properties.utf8_str()).HasParseError()) {
        if (json_doc.HasMember("Tooltip") && json_doc["Tooltip"].IsString()) {
            Value& s = json_doc["Tooltip"];
            return wxString::FromUTF8Unchecked(s.GetString());
        }
    }
    return "";
}

const wxString FieldModel::getDefault(const wxString& properties)
{
    Document json_doc;
    if (!json_doc.Parse(properties.utf8_str()).HasParseError() &&
        json_doc.HasMember("Default") && json_doc["Default"].IsString()
    ) {
        Value& s = json_doc["Default"];
        return wxString::FromUTF8Unchecked(s.GetString());
    }
    return "";
}

int FieldModel::getDigitScale(const wxString& properties)
{
    Document json_doc;
    if (!json_doc.Parse(properties.utf8_str()).HasParseError() &&
        json_doc.HasMember("DigitScale") && json_doc["DigitScale"].IsInt()
    ) {
        Value& s = json_doc["DigitScale"];
        return s.GetInt();
    }
    return 0;
}

bool FieldModel::getAutocomplete(const wxString& properties)
{
    Document json_doc;
    if (!json_doc.Parse(properties.utf8_str()).HasParseError() &&
        json_doc.HasMember("Autocomplete") && json_doc["Autocomplete"].IsBool()
    ) {
        Value& b = json_doc["Autocomplete"];
        return b.GetBool();
    }
    return false;
}

const wxArrayString FieldModel::getChoices(const wxString& properties)
{
    wxArrayString choices;
    Document json_doc;
    if (!json_doc.Parse(properties.utf8_str()).HasParseError() &&
        json_doc.HasMember("Choice") && json_doc["Choice"].IsArray()
    ) {
        Value& sa = json_doc["Choice"];
        for (const auto& entry : sa.GetArray()) {
            choices.Add(wxString::FromUTF8Unchecked(entry.GetString()));
        }
    }
    return choices;
}

const wxString FieldModel::formatProperties(
    const wxString& tooltip,
    const wxString& regEx,
    bool autocomplete,
    const wxString& default_value,
    const wxArrayString& choice_a,
    const int digitScale,
    const wxString& udfc_str
) {
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();

    if (!tooltip.empty()) {
        json_writer.Key("Tooltip");
        json_writer.String(tooltip.ToUTF8());
    }

    if (!regEx.empty()) {
        json_writer.Key("RegEx");
        json_writer.String(regEx.ToUTF8());
    }

    if (autocomplete) {
        json_writer.Key("Autocomplete");
        json_writer.Bool(autocomplete);
    }

    if (!default_value.empty()) {
        json_writer.Key("Default");
        json_writer.String(default_value.ToUTF8());
    }

    if (!choice_a.empty()) {
        json_writer.Key("Choice");
        json_writer.StartArray();
        for (const auto &choice : choice_a) {
            json_writer.String(choice.ToUTF8());
        }
        json_writer.EndArray();
    }

    if (digitScale) {
        json_writer.Key("DigitScale");
        json_writer.Int(digitScale);
    }

    if (!udfc_str.empty()) {
        json_writer.Key("UDFC");
        json_writer.String(udfc_str.c_str());
    }

    json_writer.EndObject();

    return wxString::FromUTF8(json_buffer.GetString());
}

// Delete a field and all his data
bool FieldModel::purge_id(int64 field_id)
{
    db_savepoint();
    for (const auto& fv_d : FieldValueModel::instance().find(
        FieldValueCol::FIELDID(field_id)
    )) {
        FieldValueModel::instance().purge_id(fv_d.m_id);
    }
    db_release_savepoint();
    return unsafe_remove_id(field_id);
}

// Return all values
// CHECK: chenge wxArrayString to std::set<wxString>
wxArrayString FieldModel::find_id_value_a(const int64 field_id)
{
    wxArrayString value_a;
    wxString prev_value;

    FieldValueModel::DataA fv_a = FieldValueModel::instance().find(
        FieldValueCol::FIELDID(field_id)
    );
    std::sort(fv_a.begin(), fv_a.end(), FieldValueData::SorterByCONTENT());

    for (const auto& fv_d : fv_a) {
        if (fv_d.m_content != prev_value) {
            value_a.Add(fv_d.m_content);
            prev_value = fv_d.m_content;
        }
    }
    return value_a;
}


const FieldData* FieldModel::get_udfc_data_n(RefTypeN ref_type, const wxString& udfc)
{
    Document json_doc;
    for (const auto& field_d : find(
        FieldCol::REFTYPE(ref_type.name_n())
    )) {
        if (!json_doc.Parse(field_d.m_properties.utf8_str()).HasParseError() &&
            json_doc.HasMember("UDFC") &&
            json_doc["UDFC"].IsString() &&
            json_doc["UDFC"].GetString() == udfc
        ) {
            return get_id_data_n(field_d.m_id);
        }
    }
    return nullptr;
}

int64 FieldModel::get_udfc_id_n(RefTypeN ref_type, const wxString& udfc)
{
    const Data* field_n = get_udfc_data_n(ref_type, udfc);
    return field_n ? field_n->m_id : -1;
}

const wxString FieldModel::get_udfc_name_n(RefTypeN ref_type, const wxString& udfc)
{
    const Data* field_n = get_udfc_data_n(ref_type, udfc);
    return field_n ? field_n->m_description : "";
}

FieldTypeN FieldModel::get_udfc_type_n(RefTypeN ref_type, const wxString& udfc)
{
    const Data* field_n = get_udfc_data_n(ref_type, udfc);
    return field_n ? field_n->m_type_n : FieldTypeN();
}

const wxString FieldModel::get_udfc_properties_n(RefTypeN ref_type, const wxString& udfc)
{
    const Data* field_n = get_udfc_data_n(ref_type, udfc);
    return field_n ? field_n->m_properties : "";
}

const std::map<wxString, int64> FieldModel::get_all_ucfd_id_m(RefTypeN ref_type)
{
    std::map<wxString, int64> ucfd_id_m;
    for (const auto& ucfd : UDFC_FIELDS()) {
        if (ucfd.empty())
            continue;
        ucfd_id_m[ucfd] = get_udfc_id_n(ref_type, ucfd);
    }
    return ucfd_id_m;
}

const wxArrayString FieldModel::get_data_udfc_a(const FieldData* field_n)
{
    wxArrayString udfc_a = UDFC_FIELDS();
    for (const auto& field_d : FieldModel::instance().find(
        FieldCol::REFTYPE(TrxModel::s_ref_type.name_n())
    )) {
        const wxString udfc = FieldModel::getUDFC(field_d.m_properties);
        if (!udfc.empty() && udfc_a.Index(udfc) != wxNOT_FOUND) {
            udfc_a.Remove(udfc);
        }
    }

    if (field_n) {
        const wxString udfc = FieldModel::getUDFC(field_n->m_properties);
        if (!udfc.empty())
            udfc_a.Add(udfc);
    }

    udfc_a.Sort();
    return udfc_a;
}
