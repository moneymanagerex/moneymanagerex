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
    json::Object jsonProperties;
    std::wstringstream jsonPropertiesStream;

    jsonPropertiesStream << Properties.ToStdWstring();
    json::Reader::Read(jsonProperties, jsonPropertiesStream);
    return wxString(json::String(jsonProperties[L"Tooltip"]));
}

wxString Model_CustomField::getRegEx(const wxString& Properties)
{
    json::Object jsonProperties;
    std::wstringstream jsonPropertiesStream;

    jsonPropertiesStream << Properties.ToStdWstring();
    json::Reader::Read(jsonProperties, jsonPropertiesStream);
    return wxString(json::String(jsonProperties[L"RegEx"]));
}

bool Model_CustomField::getAutocomplete(const wxString& Properties)
{
    json::Object jsonProperties;
    std::wstringstream jsonPropertiesStream;

    jsonPropertiesStream << Properties.ToStdWstring();
    json::Reader::Read(jsonProperties, jsonPropertiesStream);
    return json::Boolean(jsonProperties[L"Autocomplete"]);
}

wxString Model_CustomField::getDefault(const wxString& Properties)
{
    json::Object jsonProperties;
    std::wstringstream jsonPropertiesStream;

    jsonPropertiesStream << Properties.ToStdWstring();
    json::Reader::Read(jsonProperties, jsonPropertiesStream);
    return wxString(json::String(jsonProperties[L"Default"]));
}

wxArrayString Model_CustomField::getChoices(const wxString& Properties)
{
    json::Object jsonProperties;
    std::wstringstream jsonPropertiesStream;
    wxArrayString Choices;

    jsonPropertiesStream << Properties.ToStdWstring();
    json::Reader::Read(jsonProperties, jsonPropertiesStream);

    const json::Array& jsonChoices = jsonProperties[L"Choices"];
    for (json::Array::const_iterator it = jsonChoices.Begin(); it != jsonChoices.End(); ++it)
    {
        const json::Object& obj = *it;
        Choices.Add(wxString(json::String(obj[L"Choice"])));
    }

    return Choices;
}

wxString Model_CustomField::formatProperties(const wxString& Tooltip, const wxString& RegEx, bool Autocomplete, const wxString& Default, const wxArrayString& Choices)
{
    json::Object jsonProperties;
    std::wstringstream jsonPropertiesStream;
    json::Array jsonChoices;
    wxString outputMessage;

    jsonProperties[L"Tooltip"] = json::String(Tooltip.ToStdWstring());
    jsonProperties[L"RegEx"] = json::String(RegEx.ToStdWstring());
    jsonProperties[L"Autocomplete"] = json::Boolean(Autocomplete);
    jsonProperties[L"Default"] = json::String(Default.ToStdWstring());

    for (const auto &choice : Choices)
    {
        json::Object o;
        o[L"Choice"] = json::String(choice.ToStdWstring());
        jsonChoices.Insert(o);
    }
    jsonProperties[L"Choices"] = json::Array(jsonChoices);

    json::Writer::Write(jsonProperties, jsonPropertiesStream);
    return jsonPropertiesStream.str();
}
