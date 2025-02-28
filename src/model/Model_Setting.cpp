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

// Initialize the global Model_Setting table.
// Reset the Model_Setting table or create the table if it does not exist.
Model_Setting& Model_Setting::instance(wxSQLite3Database* db)
{
    Model_Setting& ins = Singleton<Model_Setting>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);
    ins.preload();
    return ins;
}

// Return the static instance of Model_Setting table.
Model_Setting& Model_Setting::instance()
{
    return Singleton<Model_Setting>::instance();
}

// Returns true if key setting found
bool Model_Setting::contains(const wxString& key)
{
    return !find(SETTINGNAME(key)).empty();
}

// Raw
void Model_Setting::setRaw(const wxString& key, const wxString& newValue)
{
    // search in cache
    Data* setting = get_one(SETTINGNAME(key));
    if (!setting) {
        // not found in cache; search in db
        Data_Set items = find(SETTINGNAME(key));
        if (!items.empty())
            setting = get(items[0].SETTINGID, db_);
        if (!setting) {
            // not found; create
            setting = create();
            setting->SETTINGNAME = key;
        }
    }
    setting->SETTINGVALUE = newValue;
    setting->save(db_);
}
const wxString Model_Setting::getRaw(const wxString& key, const wxString& defaultValue)
{
    // search in cache
    Data* setting = get_one(SETTINGNAME(key));
    if (setting)
        return setting->SETTINGVALUE;
    // search in db
    Data_Set items = find(SETTINGNAME(key));
    if (!items.empty())
        return items[0].SETTINGVALUE;
    // not found
    return defaultValue;
}

// String
void Model_Setting::setString(const wxString& key, const wxString& newValue)
{
    setRaw(key, newValue);
}
const wxString Model_Setting::getString(const wxString& key, const wxString& defaultValue)
{
    return getRaw(key, defaultValue);
}

// Bool
void Model_Setting::setBool(const wxString& key, bool newValue)
{
    setRaw(key, wxString::Format("%s", newValue ? "TRUE" : "FALSE"));
}
bool Model_Setting::getBool(const wxString& key, bool defaultValue)
{
    wxString rawValue = getRaw(key, "");
    if (rawValue == "TRUE") return true;
    if (rawValue == "FALSE") return false;
    return defaultValue; 
}

// Int
void Model_Setting::setInt(const wxString& key, int newValue)
{
    setRaw(key, wxString::Format("%d", newValue));
}
int Model_Setting::getInt(const wxString& key, int defaultValue)
{
    wxString rawValue = getRaw(key, "");
    if (!rawValue.IsEmpty() && rawValue.IsNumber())
        return wxAtoi(rawValue);
    return defaultValue;
}

// Colour
void Model_Setting::setColour(const wxString& key, const wxColour& newValue)
{
    setRaw(key, wxString::Format("%d,%d,%d",
        newValue.Red(), newValue.Green(), newValue.Blue()
    ));
}
const wxColour Model_Setting::getColour(const wxString& key, const wxColour& defaultValue)
{
    const wxString rawValue = getRaw(key, "");
    if (!rawValue.IsEmpty()) {
        wxRegEx pattern("([0-9]{1,3}),([0-9]{1,3}),([0-9]{1,3})");
        if (pattern.Matches(rawValue)) {
            const wxString r = pattern.GetMatch(rawValue, 1);
            const wxString g = pattern.GetMatch(rawValue, 2);
            const wxString b = pattern.GetMatch(rawValue, 3);
            return wxColour(wxAtoi(r), wxAtoi(g), wxAtoi(b));
        }
        else {
            return wxColour(rawValue);
        }
    }
    return defaultValue;
}

//-------------------------------------------------------------------
// Jdoc
void Model_Setting::setJdoc(const wxString& key, Document& newValue)
{
    wxString j_str = JSON_PrettyFormated(newValue);
    setRaw(key, j_str);
}
void Model_Setting::setJdoc(const wxString& key, StringBuffer& newValue)
{
    wxString j_str = wxString::FromUTF8(newValue.GetString());
    setRaw(key, j_str);
}
Document Model_Setting::getJdoc(const wxString& key, const wxString& defaultValue)
{
    Document j_doc;
    wxString j_str = getRaw(key, defaultValue);
    j_doc.Parse(j_str.utf8_str());
    return j_doc;
}

// ArrayString
void Model_Setting::setArrayString(const wxString& key, const wxArrayString& a)
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartArray();
    for (const auto& value : a) {
        json_writer.String(value.utf8_str());
    }
    json_writer.EndArray();
    const wxString& json_string = wxString::FromUTF8(json_buffer.GetString());
    setRaw(key, json_string);
    wxLogDebug("Model_Setting::setArrayString(%s): %s", key, json_string);
}

const wxArrayString Model_Setting::getArrayString(const wxString& key)
{
    wxString rawValue = getRaw(key, "");
    Document j_doc;
    if (rawValue.IsEmpty() ||
        j_doc.Parse(rawValue.utf8_str()).HasParseError() ||
        !j_doc.IsArray()
    )
        return wxArrayString();

    wxArrayString a;
    wxLogDebug("{{{ Model_Setting::getArrayString(%s)", key);
    for (rapidjson::SizeType i = 0; i < j_doc.Size(); i++) {
        wxASSERT(j_doc[i].IsString());
        const auto value = wxString::FromUTF8(j_doc[i].GetString());
        wxLogDebug("%s", value);
        a.Add(value);
    }
    wxLogDebug("}}}");
    return a;
}

void Model_Setting::prependArrayItem(const wxString& key, const wxString& value, int limit)
{
    if (value.IsEmpty())
        return;
    Data* setting = get_one(SETTINGNAME(key));
    if (!setting) { // not cached
        Data_Set items = find(SETTINGNAME(key));
        if (!items.empty())
        setting = get(items[0].SETTINGID, db_);
        if (!setting) {
            setting = create();
            setting->SETTINGNAME = key;
        }
    }
    wxArrayString a;
    a.Add(value);

    Document j_doc;
    if (!j_doc.Parse(setting->SETTINGVALUE.utf8_str()).HasParseError()
        && j_doc.IsArray()
    ) {
        int i = 1;
        for (auto& v : j_doc.GetArray()) {
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
    for (const auto& entry : a) {
        json_writer.String(entry.utf8_str());
    }
    json_writer.EndArray();

    setting->SETTINGVALUE = wxString::FromUTF8(json_buffer.GetString());
    setting->save(db_);
}

//-------------------------------------------------------------------
// VIEWACCOUNTS
void Model_Setting::setViewAccounts(const wxString& newValue)
{
    setString("VIEWACCOUNTS", newValue);
}
wxString Model_Setting::getViewAccounts()
{
    return getString("VIEWACCOUNTS", VIEW_ACCOUNTS_ALL_STR);
}

// THEME
void Model_Setting::setTheme(const wxString& newValue)
{
    setString("THEME", newValue);
}
wxString Model_Setting::getTheme()
{
    return getString("THEME", "default");
}

// LASTFILENAME
wxString Model_Setting::getLastDbPath()
{
    wxString path = getString("LASTFILENAME", "");
    if (!mmex::isPortableMode())
        return path;
    wxString vol = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetVolume();
    if (!vol.IsEmpty()) {
        wxFileName fname(path);
        fname.SetVolume(vol); // database should be on portable device
        if (fname.FileExists()) {
            path = fname.GetFullPath();
        }
    }
    return path;
}

//-------------------------------------------------------------------
// Trim usage settings in case if values greater than 500k
void Model_Setting::shrinkUsageTable()
{
    const wxULongLong max_size = 524287;
    const wxULongLong file_size = wxFileName(mmex::getPathUser(mmex::SETTINGS)).GetSize();
    if (file_size < max_size)
        return;

    const wxString save_point = "SETTINGS_TRIM_USAGE";
    wxDate date(wxDate::Now());
    date.Subtract(wxDateSpan::Months(2));
    db_->Savepoint(save_point);
    try {
        wxString sql = wxString::Format("delete from USAGE_V1 where USAGEDATE < \"%s\";", date.FormatISODate());
        db_->ExecuteUpdate(sql);
    }
    catch (const wxSQLite3Exception& /*e*/) {
        db_->Rollback(save_point);
    }
    db_->ReleaseSavepoint(save_point);
    db_->Vacuum();
}

row_t Model_Setting::to_row_t()
{
    row_t row;
    for (const auto &r: instance().all())
        row(r.SETTINGNAME.ToStdWstring()) = r.SETTINGVALUE;
    return row;
}
