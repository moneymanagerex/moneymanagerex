/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)


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

#include "Model_Infotable.h"
#include "constants.h"
#include "util.h"

Model_Infotable::Model_Infotable()
: Model<DB_Table_INFOTABLE_V1>()
{
}

Model_Infotable::~Model_Infotable()
{
}

/**
* Initialize the global Model_Infotable.
* Reset the Model_Infotable or create the table if it does not exist.
*/
Model_Infotable& Model_Infotable::instance(wxSQLite3Database* db)
{
    Model_Infotable& ins = Singleton<Model_Infotable>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);
    ins.preload();
    if (!ins.KeyExists("MMEXVERSION"))
    {
        ins.Set("MMEXVERSION", mmex::version::string);
        ins.Set("DATAVERSION", mmex::DATAVERSION);
        ins.Set("CREATEDATE", wxDateTime::Now());
        ins.Set("DATEFORMAT", mmex::DEFDATEFORMAT);
    }

    return ins;
}

/** Return the static instance of Model_Infotable */
Model_Infotable& Model_Infotable::instance()
{
    return Singleton<Model_Infotable>::instance();
}

// Setter
void Model_Infotable::Set(const wxString& key, int64 value)
{
    this->Set(key, wxString::Format("%lld", value));
}

void Model_Infotable::Set(const wxString& key, int value)
{
    this->Set(key, wxString::Format("%d", value));
}

void Model_Infotable::Set(const wxString& key, bool value)
{
    this->Set(key, wxString::Format("%s", value ? "TRUE" : "FALSE"));
}

void Model_Infotable::Set(const wxString& key, const wxDateTime& date)
{
    this->Set(key, date.FormatISODate());
}

void Model_Infotable::Set(const wxString& key, const wxSize& size)
{
    this->Set(key, wxString::Format("%i,%i", size.GetWidth(), size.GetHeight()));
}

void Model_Infotable::Set(const wxString& key, const wxString& value)
{
    Data* info = this->get_one(INFONAME(key));
    if (!info) // not cached
    {
        Data_Set items = this->find(INFONAME(key));
        if (!items.empty()) info = this->get(items[0].INFOID);
    }
    if (info)
    {
        info->INFOVALUE= value;
        info->save(this->db_);
    }
    else
    {
        info = this->create();
        info->INFONAME = key;
        info->INFOVALUE = value;
        info->save(this->db_);
    }
}

void Model_Infotable::Set(const wxString& key, const wxColour& value)
{
    this->Set(key, wxString::Format("%d,%d,%d", value.Red(), value.Green(), value.Blue()));
}

void Model_Infotable::Prepend(const wxString& key, const wxString& value, int limit)
{
    Data* setting = this->get_one(INFONAME(key));
    if (!setting) // not cached
    {
        Data_Set items = this->find(INFONAME(key));
        if (!items.empty()) setting = this->get(items[0].INFOID);
    }

    if (!setting)
    {
        setting = this->create();
        setting->INFONAME = key;
    }
    int i = 1;
    wxArrayString a;
    if (!value.empty() && limit != 0)
        a.Add(value);

    Document j_doc;
    if (j_doc.Parse(setting->INFOVALUE.utf8_str()).HasParseError()) {
        j_doc.Parse("[]");
    }

    if (j_doc.IsArray())
    {
        for (auto& v : j_doc.GetArray())
        {
            if (i >= limit && limit != -1) break;
            if (v.IsString()) {
                const auto item = wxString::FromUTF8(v.GetString());
                if (a.Index(item) == wxNOT_FOUND) {
                    a.Add(item);
                    i++;
                }
            }
        }
    }

    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartArray();
    for (const auto& entry : a)
    {
        json_writer.String(entry.utf8_str());
    }
    json_writer.EndArray();

    setting->INFOVALUE = wxString::FromUTF8(json_buffer.GetString());
    setting->save(this->db_);
}

void Model_Infotable::Erase(const wxString& key, int row)
{
    Document j_doc;
    if (j_doc.Parse(GetStringInfo(key, "[]").utf8_str()).HasParseError()) {
        j_doc.Parse("[]");
    }

    if (j_doc.IsArray())
    {
        j_doc.Erase(j_doc.Begin() + row);

        StringBuffer json_buffer;
        PrettyWriter<StringBuffer> json_writer(json_buffer);
        j_doc.Accept(json_writer);
        const wxString json_string = wxString::FromUTF8(json_buffer.GetString());
        Set(key, json_string);
        wxLogDebug(json_string);
    }
}

void Model_Infotable::Update(const wxString& key, int row, const wxString& value)
{
    Document j_doc, j_doc_new;
    if (j_doc.Parse(GetStringInfo(key, "[]").utf8_str()).HasParseError()) {
        j_doc.Parse("[]");
    }

    if (j_doc.IsArray())
    {
        StringBuffer json_buffer;
        PrettyWriter<StringBuffer> json_writer(json_buffer);
        json_writer.StartArray();
        for (SizeType i = 0; i < j_doc.Size(); i++)
        {
            if (row == static_cast<int>(i))
                json_writer.String(value.utf8_str());
            else
                json_writer.String(j_doc[i].GetString());
        }
        json_writer.EndArray();

        const wxString& json_string = wxString::FromUTF8(json_buffer.GetString());
        Set(key, json_string);
        wxLogDebug(json_string);
    }
}

// Getter
bool Model_Infotable::GetBoolInfo(const wxString& key, bool default_value)
{
    const wxString value = this->GetStringInfo(key, "");
    if (value == "1" || value.CmpNoCase("TRUE") == 0)
        return true;
    else if (value == "0" || value.CmpNoCase("FALSE") == 0)
        return false;
    else
        return default_value;
}

int Model_Infotable::GetIntInfo(const wxString& key, int default_value)
{
    const wxString value = this->GetStringInfo(key, "");
    if (!value.IsEmpty() && value.IsNumber())
        return wxAtoi(value);

    return default_value;
}

int64 Model_Infotable::GetInt64Info(const wxString& key, int64 default_value)
{
    const wxString value = this->GetStringInfo(key, "");
    if (!value.IsEmpty() && value.IsNumber())
        return int64(wxAtol(value));

    return default_value;
}

wxString Model_Infotable::GetStringInfo(const wxString& key, const wxString& default_value)
{
    Data* info = this->get_one(INFONAME(key));
    if (info)
        return info->INFOVALUE;
    else // not cached
    {
        Data_Set items = this->find(INFONAME(key));
        if (!items.empty())
            return items[0].INFOVALUE;
    }

    return default_value;
}
const wxSize Model_Infotable::GetSizeSetting(const wxString& key)
{
    const wxString value = this->GetStringInfo(key, "");
    if (!value.IsEmpty())
    {
        wxRegEx pattern("^([0-9]+),([0-9]+)$");
        if (pattern.Matches(value))
        {
            const auto& x = pattern.GetMatch(value, 1);
            const auto& y = pattern.GetMatch(value, 2);
            return wxSize(wxAtoi(x), wxAtoi(y));
        }
    }
    return wxDefaultSize;
}

const wxColour Model_Infotable::GetColourSetting(const wxString& key, const wxColour& default_value)
{
    const wxString value = this->GetStringInfo(key, "");
    if (!value.IsEmpty())
    {
        wxRegEx pattern("([0-9]{1,3}),([0-9]{1,3}),([0-9]{1,3})");
        if (pattern.Matches(value))
        {
            const wxString red = pattern.GetMatch(value, 1);
            const wxString green = pattern.GetMatch(value, 2);
            const wxString blue = pattern.GetMatch(value, 3);

            return wxColour(wxAtoi(red), wxAtoi(green), wxAtoi(blue));
        }
        else
        {
            return wxColour(value);
        }
    }

    return default_value;
}

const wxArrayString Model_Infotable::GetArrayStringSetting(const wxString& key, bool sort)
{
    wxString data;
    Data* setting = this->get_one(INFONAME(key));
    if (!setting) // not cached
    {
        Data_Set items = this->find(INFONAME(key));
        if (items.empty()) {
            return wxArrayString();
        }
        else {
            data = items[0].INFOVALUE;
        }
    }
    else
    {
        data = setting->INFOVALUE;
    }

    wxArrayString a;
    Document j_doc;
    if (j_doc.Parse(data.utf8_str()).HasParseError()) {
        j_doc.Parse("[]");
    }

    if (j_doc.IsArray())
    {
        for (rapidjson::SizeType i = 0; i < j_doc.Size(); i++)
        {
            wxASSERT(j_doc[i].IsString());
            const auto item = wxString::FromUTF8(j_doc[i].GetString());
            wxLogDebug("%s", item);
            a.Add(item);
        }
    }

    // Crude sort of JSON (case sensitive), could be improved by actually sorting by a field
    // but this should be sufficient if you want to sort by first element
    if (sort)
        a.Sort();
    return a;
}

/* Returns true if key setting found */
bool Model_Infotable::KeyExists(const wxString& key)
{
    return !this->find(INFONAME(key)).empty();
}

bool Model_Infotable::checkDBVersion()
{
    if (!this->KeyExists("DATAVERSION")) return false;

    return this->GetIntInfo("DATAVERSION", 0) >= mmex::MIN_DATAVERSION;
}

loop_t Model_Infotable::to_loop_t()
{
    loop_t loop;
    for (const auto &r: instance().all())
        loop += r.to_row_t();
    return loop;
}

//-------------------------------------------------------------------
bool Model_Infotable::OpenCustomDialog(const wxString& RefType)
{
    return GetBoolInfo("CUSTOMDIALOG_OPEN:" + RefType, false);
}

void Model_Infotable::SetOpenCustomDialog(const wxString& RefType, bool Status)
{
    Set("CUSTOMDIALOG_OPEN:" + RefType, Status);
}

wxSize Model_Infotable::CustomDialogSize(const wxString& RefType)
{
    wxString strSize = GetStringInfo("CUSTOMDIALOG_SIZE:" + RefType, "0;0");
    return wxSize(wxAtoi(strSize.BeforeFirst(';')), wxAtoi(strSize.AfterFirst(';')));
}

void Model_Infotable::SetCustomDialogSize(const wxString& RefType, const wxSize& Size)
{
    wxString strSize;
    strSize << Size.GetWidth() << ";" << Size.GetHeight();
    Set("CUSTOMDIALOG_SIZE:" + RefType, strSize);
}

int Model_Infotable::FindLabelInJSON(const wxString& entry, const wxString& labelID)
{
    // Important: Get the unsorted array
    wxArrayString settings = Model_Infotable::instance().GetArrayStringSetting(entry);
    int sel = 0;
    for (const auto& data : settings)
    {
        Document j_doc;
        if (j_doc.Parse(data.utf8_str()).HasParseError())
            j_doc.Parse("{}");
        Value& j_label = GetValueByPointerWithDefault(j_doc, "/LABEL", "");
        const wxString& s_label = j_label.IsString() ? wxString::FromUTF8(j_label.GetString()) : "";
        if (s_label == labelID)
            return sel;
        ++sel;
    }
    return wxNOT_FOUND;
}
