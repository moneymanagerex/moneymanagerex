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
    : Model<DB_Table_CUSTOMFIELD>()
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
//    for (const auto & field : this->find(Model_CustomField::DB_Table_CUSTOMFIELD::REFTYPE(RefType)))
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

const wxString Model_CustomField::fieldtype_desc(const int FieldTypeEnum)
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

    return UNKNOWN;
}

Model_CustomField::FIELDTYPE Model_CustomField::type(const Data& r)
{
    return type(&r);
}

const wxArrayString Model_CustomField::all_type()
{
    static wxArrayString types;
    if (types.empty())
    {
        for (const auto& item : FIELDTYPE_CHOICES)
            types.Add(item.second);
    }
    return types;
}

const wxString Model_CustomField::getTooltip(const wxString& Properties)
{
    Document json_doc;
    if (!json_doc.Parse(Properties.c_str()).HasParseError())
    {
        if (json_doc.HasMember("Tooltip") && json_doc["Tooltip"].IsString()) {
            Value& s = json_doc["Tooltip"];
            return s.GetString();
        }
    }
    return "";
}

const wxString Model_CustomField::getRegEx(const wxString& Properties)
{
    Document json_doc;
    if (!json_doc.Parse(Properties.c_str()).HasParseError())
    {
        if (json_doc.HasMember("RegEx") && json_doc["RegEx"].IsString()) {
            Value& s = json_doc["RegEx"];
            return s.GetString();
        }
    }
    return "";
}

bool Model_CustomField::getAutocomplete(const wxString& Properties)
{
    Document json_doc;
    if (!json_doc.Parse(Properties.c_str()).HasParseError())
    {
        if (json_doc.HasMember("Autocomplete") && json_doc["Autocomplete"].IsBool()) {
            Value& b = json_doc["Autocomplete"];
            return b.GetBool();
        }
    }
    return false;
}

const wxString Model_CustomField::getDefault(const wxString& Properties)
{
    Document json_doc;
    if (!json_doc.Parse(Properties.c_str()).HasParseError())
    {
        if (json_doc.HasMember("Default") && json_doc["Default"].IsString()) {
            Value& s = json_doc["Default"];
            return s.GetString();
        }
    }
    return "";
}

const wxArrayString Model_CustomField::getChoices(const wxString& Properties)
{
    wxArrayString choices;
    Document json_doc;
    if (!json_doc.Parse(Properties.c_str()).HasParseError())
    {
        if (json_doc.HasMember("Choice") && json_doc["Choice"].IsArray())
        {
            Value& sa = json_doc["Choice"];
            for (const auto& entry : sa.GetArray())
            {
                choices.Add(entry.GetString());
            }
        }
    }

    return choices;
}

const wxString Model_CustomField::getUDFC(const wxString& Properties)
{
    Document json_doc;
    if (!json_doc.Parse(Properties.c_str()).HasParseError())
    {
        if (json_doc.HasMember("UDFC") && json_doc["UDFC"].IsString()) {
            Value& s = json_doc["UDFC"];
            return s.GetString();
        }
    }
    return "";
}

const std::map<wxString, int> Model_CustomField::getMatrix(Model_Attachment::REFTYPE reftype)
{
    std::map<wxString, int> m;
    const wxString& reftype_desc = Model_Attachment::reftype_desc(reftype);
    for (const auto& entry : UDFC_FIELDS())
    {
        if (entry.empty()) continue;
        m[entry] = getUDFCID(reftype_desc, entry);
    }
    return m;
}

int Model_CustomField::getUDFCID(const wxString& ref_type, const wxString& name)
{
    Document json_doc;
    const auto& a = Model_CustomField::instance().find(REFTYPE(ref_type));
    for (const auto& item : a)
    {
        if (!json_doc.Parse(item.PROPERTIES.c_str()).HasParseError())
        {
            if (json_doc.HasMember("UDFC") && json_doc["UDFC"].IsString())
            {
                Value& s = json_doc["UDFC"];
                const wxString& desc = s.GetString();
                if (desc == name) {
                    return item.FIELDID;
                }
            }
        }
    }

    return -1;
}

const wxString Model_CustomField::getUDFCName(const wxString& ref_type, const wxString& name)
{
    Document json_doc;
    const auto& a = Model_CustomField::instance().find(REFTYPE(ref_type));
    for (const auto& item : a)
    {
        if (!json_doc.Parse(item.PROPERTIES.c_str()).HasParseError())
        {
            if (json_doc.HasMember("UDFC") && json_doc["UDFC"].IsString())
            {
                Value& s = json_doc["UDFC"];
                const wxString& desc = s.GetString();
                if (desc == name) {
                    return item.DESCRIPTION;
                }
            }
        }
    }

    return name;
}

const wxArrayString Model_CustomField::UDFC_FIELDS()
{
    wxArrayString choices;
    choices.Add("");
    choices.Add("UDFC01");
    choices.Add("UDFC02");
    choices.Add("UDFC03");
    choices.Add("UDFC04");
    choices.Add("UDFC05");
    return choices;
};

const wxArrayString Model_CustomField::getUDFCList(DB_Table_CUSTOMFIELD::Data* r)
{
    wxString i;
    wxArrayString choices = UDFC_FIELDS();

    if (r)
    {
        Document json_doc;
        if (!json_doc.Parse(r->PROPERTIES.c_str()).HasParseError())
        {
            if (json_doc.HasMember("UDFC") && json_doc["UDFC"].IsString())
            {
                Value& s = json_doc["UDFC"];
                i = s.GetString();
            }
        }

        const auto& a = Model_CustomField::instance().find(Model_CustomField::DB_Table_CUSTOMFIELD::REFTYPE(r->REFTYPE));
        for (const auto& item : a)
        {
            if (!json_doc.Parse(item.PROPERTIES.c_str()).HasParseError())
            {
                if (json_doc.HasMember("UDFC") && json_doc["UDFC"].IsString())
                {
                    Value& s = json_doc["UDFC"];
                    if (choices.Index(s.GetString()) != wxNOT_FOUND && i != s.GetString()) {
                        choices.Remove(s.GetString());
                    }
                }
            }
        }
    }

    return choices;
}

int Model_CustomField::getDigitScale(const wxString& Properties)
{
    Document json_doc;
    if (!json_doc.Parse(Properties.c_str()).HasParseError())
    {
        if (json_doc.HasMember("DigitScale") && json_doc["DigitScale"].IsInt()) {
            Value& s = json_doc["DigitScale"];
            return s.GetInt();
        }
    }
    return 0;
}

const wxString Model_CustomField::formatProperties(const wxString& Tooltip, const wxString& RegEx
    , bool Autocomplete, const wxString& Default, const wxArrayString& Choices
    , const int DigitScale, const wxString& udfc_str)
{
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    
    if (!Tooltip.empty()) {
        json_writer.Key("Tooltip");
        json_writer.String(Tooltip.c_str());
    }

    if (!RegEx.empty()) {
        json_writer.Key("RegEx");
        json_writer.String(RegEx.c_str());
    }

    if (Autocomplete) {
        json_writer.Key("Autocomplete");
        json_writer.Bool(Autocomplete);
    }

    if (!Default.empty()) {
        json_writer.Key("Default");
        json_writer.String(Default.c_str());
    }

    if (!Choices.empty())
    {
        json_writer.Key("Choice");
        json_writer.StartArray();
        for (const auto &choice : Choices)
        {
            json_writer.String(choice.c_str());
        }
        json_writer.EndArray();
    }

    if (DigitScale) {
        json_writer.Key("DigitScale");
        json_writer.Int(DigitScale);
    }

    if (!udfc_str.empty()) {
        json_writer.Key("UDFC");
        json_writer.String(udfc_str.c_str());
    }

    json_writer.EndObject();

    return json_buffer.GetString();
}
