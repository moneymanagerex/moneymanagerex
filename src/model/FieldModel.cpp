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

mmChoiceNameA FieldModel::TYPE_CHOICES = mmChoiceNameA({
    { TYPE_ID_STRING,       _n("String") },
    { TYPE_ID_INTEGER,      _n("Integer") },
    { TYPE_ID_DECIMAL,      _n("Decimal") },
    { TYPE_ID_BOOLEAN,      _n("Boolean") },
    { TYPE_ID_DATE,         _n("Date") },
    { TYPE_ID_TIME,         _n("Time") },
    { TYPE_ID_SINGLECHOICE, _n("SingleChoice") },
    { TYPE_ID_MULTICHOICE,  _n("MultiChoice") }
}, -1, true);

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

///** Return a dataset with fields linked to a specific object */
//const FieldModel::DataA FieldModel::GetFields(ModelBase::REFTYPE_ID RefType)
//{
//    DataA fields;
//    wxString reftype_str = ModelBase::reftype_name(RefType);
//    for (const auto & field : this->find(FieldCol::REFTYPE(RefType)))
//    {
//        fields.push_back(field);
//    }
//    return field;
//}

/** Delete a field and all his data */
bool FieldModel::Delete(const int64& field_id)
{
    db_savepoint();
    for (const auto& fv_d : FieldValueModel::instance().find(
        FieldValueCol::FIELDID(field_id)
    )) {
        FieldValueModel::instance().purge_id(fv_d.id());
    }
    db_release_savepoint();
    return unsafe_remove_id(field_id);
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

const wxString FieldModel::getRegEx(const wxString& properties)
{
    Document json_doc;
    if (!json_doc.Parse(properties.utf8_str()).HasParseError()) {
        if (json_doc.HasMember("RegEx") && json_doc["RegEx"].IsString()) {
            Value& s = json_doc["RegEx"];
            return wxString::FromUTF8Unchecked(s.GetString());
        }
    }
    return "";
}

bool FieldModel::getAutocomplete(const wxString& properties)
{
    Document json_doc;
    if (!json_doc.Parse(properties.utf8_str()).HasParseError()) {
        if (json_doc.HasMember("Autocomplete") && json_doc["Autocomplete"].IsBool()) {
            Value& b = json_doc["Autocomplete"];
            return b.GetBool();
        }
    }
    return false;
}

const wxString FieldModel::getDefault(const wxString& properties)
{
    Document json_doc;
    if (!json_doc.Parse(properties.utf8_str()).HasParseError()) {
        if (json_doc.HasMember("Default") && json_doc["Default"].IsString()) {
            Value& s = json_doc["Default"];
            return wxString::FromUTF8Unchecked(s.GetString());
        }
    }
    return "";
}

const wxArrayString FieldModel::getChoices(const wxString& properties)
{
    wxArrayString choices;
    Document json_doc;
    if (!json_doc.Parse(properties.utf8_str()).HasParseError()) {
        if (json_doc.HasMember("Choice") && json_doc["Choice"].IsArray()) {
            Value& sa = json_doc["Choice"];
            for (const auto& entry : sa.GetArray()) {
                choices.Add(wxString::FromUTF8Unchecked(entry.GetString()));
            }
        }
    }

    return choices;
}

const wxString FieldModel::getUDFC(const wxString& properties)
{
    Document json_doc;
    if (!json_doc.Parse(properties.utf8_str()).HasParseError()) {
        if (json_doc.HasMember("UDFC") && json_doc["UDFC"].IsString()) {
            Value& s = json_doc["UDFC"];
            return s.GetString();
        }
    }
    return "";
}

const std::map<wxString, int64> FieldModel::getMatrix(const wxString& reftype)
{
    std::map<wxString, int64> m;
    for (const auto& entry : UDFC_FIELDS()) {
        if (entry.empty()) continue;
        m[entry] = getUDFCID(reftype, entry);
    }
    return m;
}

int64 FieldModel::getUDFCID(const wxString& ref_type, const wxString& name)
{
    Document json_doc;
    for (const auto& field_d : FieldModel::instance().find(
        FieldCol::REFTYPE(ref_type)
    )) {
        if (!json_doc.Parse(field_d.m_properties.utf8_str()).HasParseError()) {
            if (json_doc.HasMember("UDFC") && json_doc["UDFC"].IsString()) {
                Value& s = json_doc["UDFC"];
                const wxString& desc = s.GetString();
                if (desc == name) {
                    return field_d.m_id;
                }
            }
        }
    }
    return -1;
}

const wxString FieldModel::getUDFCName(const wxString& ref_type, const wxString& name)
{
    Document json_doc;
    for (const auto& field_d : FieldModel::instance().find(
        FieldCol::REFTYPE(ref_type)
    )) {
        if (json_doc.Parse(field_d.m_properties.utf8_str()).HasParseError())
            continue;
        if (json_doc.HasMember("UDFC") && json_doc["UDFC"].IsString()) {
            Value& s = json_doc["UDFC"];
            const wxString& desc = s.GetString();
            if (desc == name) {
                return field_d.m_description;
            }
        }
    }
    return wxEmptyString;
}

FieldModel::TYPE_ID FieldModel::getUDFCType(const wxString& ref_type, const wxString& name)
{
    Document json_doc;
    for (const auto& field_d : FieldModel::instance().find(
        FieldCol::REFTYPE(ref_type)
    )) {
        if (json_doc.Parse(field_d.m_properties.utf8_str()).HasParseError())
            continue;
        if (json_doc.HasMember("UDFC") && json_doc["UDFC"].IsString()) {
            Value& s = json_doc["UDFC"];
            const wxString& desc = s.GetString();
            if (desc == name) {
                return static_cast<TYPE_ID>(field_d.m_type_n.id_n());
            }
        }
    }
    return FieldModel::TYPE_ID_UNKNOWN;
}

const wxString FieldModel::getUDFCProperties(const wxString& ref_type, const wxString& name)
{
    Document json_doc;
    for (const auto& field_d : FieldModel::instance().find(
        FieldCol::REFTYPE(ref_type)
    )) {
        if (json_doc.Parse(field_d.m_properties.utf8_str()).HasParseError())
            continue;
        if (json_doc.HasMember("UDFC") && json_doc["UDFC"].IsString()) {
            Value& s = json_doc["UDFC"];
            const wxString& desc = s.GetString();
            if (desc == name) {
                return field_d.m_properties;
            }
        }
    }
    return wxEmptyString;
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

const wxArrayString FieldModel::getUDFCList(const FieldData* field_n)
{
    const wxString& ref_type = TrxModel::refTypeName;

    wxArrayString choices = UDFC_FIELDS();

    for (const auto& field_d : FieldModel::instance().find(
        FieldCol::REFTYPE(ref_type)
    )) {
        Document json_doc;
        if (json_doc.Parse(field_d.m_properties.utf8_str()).HasParseError())
            continue;
        if (json_doc.HasMember("UDFC") && json_doc["UDFC"].IsString()) {
            Value& s = json_doc["UDFC"];
            if (choices.Index(s.GetString()) != wxNOT_FOUND) {
                choices.Remove(s.GetString());
            }
        }
    }

    if (field_n) {
        Document json_doc;
        if (!json_doc.Parse(field_n->m_properties.utf8_str()).HasParseError() &&
            json_doc.HasMember("UDFC") && json_doc["UDFC"].IsString()
        ) {
            Value& s = json_doc["UDFC"];
            std::string str = s.GetString();
            choices.Add(s.GetString());
        }
    }

    choices.Sort();
    return choices;
}

int FieldModel::getDigitScale(const wxString& properties)
{
    Document json_doc;
    if (json_doc.Parse(properties.utf8_str()).HasParseError())
        return 0;

    if (json_doc.HasMember("DigitScale") && json_doc["DigitScale"].IsInt()) {
        Value& s = json_doc["DigitScale"];
        return s.GetInt();
    }
    return 0;
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

