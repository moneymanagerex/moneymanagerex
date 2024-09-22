/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2014 - 2022 Nikolay Akimov
 Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)

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

#include "Model_Setting.h"
#include "constants.h"
#include "option.h"
#include "paths.h"
#include "mmcheckingpanel.h"

Model_Setting::Model_Setting()
: Model<DB_Table_SETTING_V1>()
{
}

Model_Setting::~Model_Setting()
{
}

/**
* Initialize the global Model_Setting table.
* Reset the Model_Setting table or create the table if it does not exist.
*/
Model_Setting& Model_Setting::instance(wxSQLite3Database* db)
{
    Model_Setting& ins = Singleton<Model_Setting>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);
    ins.preload();

    return ins;
}

/** Return the static instance of Model_Setting table. */
Model_Setting& Model_Setting::instance()
{
    return Singleton<Model_Setting>::instance();
}

// Setter
void Model_Setting::Set(const wxString& key, int value)
{
    this->Set(key, wxString::Format("%d", value));
}

void Model_Setting::Set(const wxString& key, bool value)
{
    this->Set(key, wxString::Format("%s", value ? "TRUE" : "FALSE"));
}

void Model_Setting::Set(const wxString& key, const wxColour& value)
{
    this->Set(key, wxString::Format("%d,%d,%d", value.Red(), value.Green(), value.Blue()));
}

void Model_Setting::Set(const wxString& key, const wxString& value)
{
    Data* setting = this->get_one(SETTINGNAME(key));
    if (!setting) // not cached
    {
        Data_Set items = this->find(SETTINGNAME(key));
        if (!items.empty()) setting = this->get(items[0].SETTINGID, this->db_);
    }
    if (setting)
    {
        setting->SETTINGVALUE = value;
        setting->save(this->db_);
    }
    else
    {
        setting = this->create();
        setting->SETTINGNAME = key;
        setting->SETTINGVALUE = value;
        setting->save(this->db_);
    }
}

void Model_Setting::Prepend(const wxString& key, const wxString& value, int limit)
{
    if (value.IsEmpty())
        return;
    Data* setting = this->get_one(SETTINGNAME(key));
    if (!setting) // not cached
    {
        Data_Set items = this->find(SETTINGNAME(key));
        if (!items.empty()) setting = this->get(items[0].SETTINGID, this->db_);
    }

    if (!setting)
    {
        setting = this->create();
        setting->SETTINGNAME = key;
    }

    int i = 1;
    wxArrayString a;
    a.Add(value);

    Document j_doc;
    if (j_doc.Parse(setting->SETTINGVALUE.utf8_str()).HasParseError()) {
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

    setting->SETTINGVALUE = wxString::FromUTF8(json_buffer.GetString());
    setting->save(this->db_);
}

// Getter
bool Model_Setting::GetBoolSetting(const wxString& key, bool default_value)
{
    wxString value = this->GetStringSetting(key, "");
    if (value == "TRUE") return true;
    if (value == "FALSE") return false;

    return default_value; 
}

int Model_Setting::GetIntSetting(const wxString& key, int default_value)
{
    wxString value = this->GetStringSetting(key, "");
    if (!value.IsEmpty() && value.IsNumber()) return wxAtoi(value);

    return default_value;
}

const wxString Model_Setting::GetStringSetting(const wxString& key, const wxString& default_value)
{
    Data* setting = this->get_one(SETTINGNAME(key));
    if (!setting) // not cached
    {
        Data_Set items = this->find(SETTINGNAME(key));
        if (!items.empty()) return items[0].SETTINGVALUE;
    }
    else
    {
        return setting->SETTINGVALUE;
    }
    return default_value;
}

const wxArrayString Model_Setting::GetArrayStringSetting(const wxString& key)
{
    wxString data;
    Data* setting = this->get_one(SETTINGNAME(key));
    if (!setting) // not cached
    {
        Data_Set items = this->find(SETTINGNAME(key));
        if (items.empty()) {
            return wxArrayString();
        }
        else {
            data = items[0].SETTINGVALUE;
        }
    }
    else
    {
        data = setting->SETTINGVALUE;
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

    return a;
}

wxString Model_Setting::getLastDbPath()
{
    wxString path = this->GetStringSetting("LASTFILENAME", "");

    if (!mmex::isPortableMode()) return path;

    wxString vol = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetVolume();

    if (!vol.IsEmpty())
    {

        wxFileName fname(path);
        fname.SetVolume(vol); // database should be on portable device

        if (fname.FileExists()) {
            path = fname.GetFullPath();
        }
    }

    return path;
}

/* Returns true if key setting found */
bool Model_Setting::ContainsSetting(const wxString& key)
{
    return !this->find(SETTINGNAME(key)).empty();
}

row_t Model_Setting::to_row_t()
{
    row_t row;
    for (const auto &r: instance().all())
        row(r.SETTINGNAME.ToStdWstring()) = r.SETTINGVALUE;
    return row;
}

//-------------------------------------------------------------------
wxString Model_Setting::GetViewAccounts()
{
    return GetStringSetting("VIEWACCOUNTS", VIEW_ACCOUNTS_ALL_STR);
}

void Model_Setting::SetViewAccounts(const wxString& value)
{
    Set("VIEWACCOUNTS", value);
}

//-------------------------------------------------------------------
wxString Model_Setting::Theme()
{
    return GetStringSetting("THEME", "default");
}

void Model_Setting::SetTheme(const wxString& value)
{
    Set("THEME", value);
}

//-------------------------------------------------------------------
wxString Model_Setting::ViewTransactions()
{
    return GetStringSetting("VIEWTRANSACTIONS", mmCheckingPanel::FILTER_STR_ALL);
}

void Model_Setting::SetViewTransactions(const wxString& value)
{
    Set("VIEWTRANSACTIONS", value);
}

// Trim usage settings in case if values greater than 500k
void Model_Setting::ShrinkUsageTable()
{
    const wxULongLong max_size = 524287;
    const wxULongLong file_size = wxFileName(mmex::getPathUser(mmex::SETTINGS)).GetSize();
    if (file_size < max_size)
    {
        return;
    }

    const wxString save_point = "SETTINGS_TRIM_USAGE";
    wxDate date(wxDate::Now());
    date.Subtract(wxDateSpan::Months(2));
    db_->Savepoint(save_point);
    try
    {
        wxString sql = wxString::Format("delete from USAGE_V1 where USAGEDATE < \"%s\";", date.FormatISODate());
        db_->ExecuteUpdate(sql);
    }
    catch (const wxSQLite3Exception& /*e*/)
    {
        db_->Rollback(save_point);
    }
    db_->ReleaseSavepoint(save_point);
    db_->Vacuum();
}
