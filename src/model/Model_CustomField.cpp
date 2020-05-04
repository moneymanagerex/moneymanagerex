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

#include "Model_CustomField.h"
#include "Model_CustomFieldData.h"
#include <wx/string.h>

const std::vector<std::pair<Model_CustomField::FIELDTYPE, wxString> > Model_CustomField::FIELDTYPE_CHOICES =
{
    {Model_CustomField::STRING, wxString(wxTRANSLATE("String"))},
    {Model_CustomField::INTEGER, wxString(wxTRANSLATE("Integer"))},
    {Model_CustomField::DECIMAL, wxString(wxTRANSLATE("Decimal"))},
    {Model_CustomField::BOOLEAN, wxString(wxTRANSLATE("Boolean"))},
    {Model_CustomField::DATE, wxString(wxTRANSLATE("Date"))},
    {Model_CustomField::TIME, wxString(wxTRANSLATE("Time"))},
    {Model_CustomField::SINGLECHOICE, wxString(wxTRANSLATE("SingleChoice"))},
    {Model_CustomField::MULTICHOICE, wxString(wxTRANSLATE("MultiChoice"))}
};

Model_CustomField::Model_CustomField()
: Model<DB_Table_CUSTOMFIELD_V1>()
{
}

Model_CustomField::~Model_CustomField()
{
}

/**
* Initialize the global Model_CustomField table.
* Reset the Model_CustomField table or create the table if it does not exist.
*/
Model_CustomField& Model_CustomField::instance(wxSQLite3Database* db)
{
    Model_CustomField& ins = Singleton<Model_CustomField>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);

    return ins;
}

/** Return the static instance of Model_CustomField table */
Model_CustomField& Model_CustomField::instance()
{
    return Singleton<Model_CustomField>::instance();
}

///** Return a dataset with fields linked to a specific object */
//const Model_CustomField::Data_Set Model_CustomField::GetFields(Model_Attachment::REFTYPE RefType)
//{
//    Data_Set fields;
//    wxString reftype_desc = Model_Attachment::reftype_desc(RefType);
//    for (const auto & field : this->find(Model_CustomField::DB_Table_CUSTOMFIELD_V1::REFTYPE(RefType)))
//    {
//        fields.push_back(field);
//    }
//    return field;
//}

/** Delete a field and all his data */
bool Model_CustomField::Delete(const int& FieldID)
{
    this->Savepoint();
    for (const auto& r : Model_CustomFieldData::instance().find(Model_CustomFieldData::FIELDID(FieldID)))
        Model_CustomFieldData::instance().remove(r.id());
    this->ReleaseSavepoint();
    return this->remove(FieldID, db_);
}

wxString Model_CustomField::fieldtype_desc(const int FieldTypeEnum)
{
    const auto& item = FIELDTYPE_CHOICES[FieldTypeEnum];
    const wxString reftype_desc = item.second;
    return reftype_desc;
}

Model_CustomField::FIELDTYPE Model_CustomField::type(const Data* r)
{
    for (const auto& item : FIELDTYPE_CHOICES)
    {
        if (item.second.CmpNoCase(r->TYPE) == 0)
            return item.first;
    }

    return FIELDTYPE(-1);
}

Model_CustomField::FIELDTYPE Model_CustomField::type(const Data& r)
{
    return type(&r);
}

wxArrayString Model_CustomField::all_type()
{
    wxArrayString types;
    for (const auto& item : FIELDTYPE_CHOICES) types.Add(item.second);
    return types;
}

wxString Model_CustomField::getTooltip(const wxString& Properties)
{
    Document json_doc;
    if (!json_doc.Parse(Properties.utf8_str()).HasParseError())
    {
        if (json_doc.HasMember("Tooltip") && json_doc["Tooltip"].IsString()) {
            Value& s = json_doc["Tooltip"];
            return wxString::FromUTF8(s.GetString());
        }
    }
    return "";
}

wxString Model_CustomField::getRegEx(const wxString& Properties)
{
    Document json_doc;
    if (!json_doc.Parse(Properties.utf8_str()).HasParseError())
    {
        if (json_doc.HasMember("RegEx") && json_doc["RegEx"].IsString()) {
            Value& s = json_doc["RegEx"];
            return wxString::FromUTF8(s.GetString());
        }
    }
    return "";
}

bool Model_CustomField::getAutocomplete(const wxString& Properties)
{
    Document json_doc;
    if (!json_doc.Parse(Properties.utf8_str()).HasParseError())
    {
        if (json_doc.HasMember("Autocomplete") && json_doc["Autocomplete"].IsBool()) {
            Value& b = json_doc["Autocomplete"];
            return b.GetBool();
        }
    }
    return false;
}

wxString Model_CustomField::getDefault(const wxString& Properties)
{
    Document json_doc;
    if (!json_doc.Parse(Properties.utf8_str()).HasParseError())
    {
        if (json_doc.HasMember("Default") && json_doc["Default"].IsString()) {
            Value& s = json_doc["Default"];
            return wxString::FromUTF8(s.GetString());
        }
    }
    return "";
}

wxArrayString Model_CustomField::getChoices(const wxString& Properties)
{
    wxArrayString choices;
    Document json_doc;
    if (!json_doc.Parse(Properties.utf8_str()).HasParseError())
    {
        if (json_doc.HasMember("Choice") && json_doc["Choice"].IsArray())
        {
            Value& sa = json_doc["Choice"];
            for (const auto& entry : sa.GetArray())
            {
                choices.Add(wxString::FromUTF8(entry.GetString()));
            }
        }
    }

    return choices;
}

wxString Model_CustomField::formatProperties(const wxString& Tooltip, const wxString& RegEx
    , bool Autocomplete, const wxString& Default, const wxArrayString& Choices)
{
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();

    if (!Tooltip.empty()) {
        json_writer.Key("Tooltip");
        json_writer.String(Tooltip.utf8_str());
    }

    if (!RegEx.empty()) {
        json_writer.Key("RegEx");
        json_writer.String(RegEx.utf8_str());
    }

    if (Autocomplete) {
        json_writer.Key("Autocomplete");
        json_writer.Bool(Autocomplete);
    }

    if (!Default.empty()) {
        json_writer.Key("Default");
        json_writer.String(Default.utf8_str());
    }

    if (!Choices.empty())
    {
        json_writer.Key("Choice");
        json_writer.StartArray();
        for (const auto &choice : Choices)
        {
            json_writer.String(choice.utf8_str());
        }
        json_writer.EndArray();
    }

    /* TODO
    if (DigitScale) {
        json_writer.Key("DigitScale");
        json_writer.Int(DigitScale);
    }

    if (!udfc_str.empty()) {
        json_writer.Key("UDFC");
        json_writer.String(udfc_str.utf8_str());
    }
    */

    json_writer.EndObject();

    return wxString::FromUTF8(json_buffer.GetString());
}

int Model_CustomField::getDigitScale(const wxString& Properties)
{
    Document json_doc;
    if (!json_doc.Parse(Properties.utf8_str()).HasParseError())
    {
        if (json_doc.HasMember("DigitScale") && json_doc["DigitScale"].IsInt()) {
            Value& s = json_doc["DigitScale"];
            return s.GetInt();
        }
    }
    return 0;
}
