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

#include "base/constants.h"
#include "base/paths.h"

#include "SettingModel.h"
#include "PreferencesModel.h"

#include "panel/JournalPanel.h"

SettingModel::SettingModel()
: Model<SettingTable>()
{
}

SettingModel::~SettingModel()
{
}

// Initialize the global SettingModel table.
// Reset the SettingModel table or create the table if it does not exist.
SettingModel& SettingModel::instance(wxSQLite3Database* db)
{
    SettingModel& ins = Singleton<SettingModel>::instance();
    ins.m_db = db;
    ins.destroy_cache();
    ins.ensure_table();
    ins.preload();
    return ins;
}

// Return the static instance of SettingModel table.
SettingModel& SettingModel::instance()
{
    return Singleton<SettingModel>::instance();
}

// Returns true if key setting found
bool SettingModel::contains(const wxString& key)
{
    return !find(SETTINGNAME(key)).empty();
}

// Raw
void SettingModel::setRaw(const wxString& key, const wxString& newValue)
{
    // search in cache
    Data* setting = search_cache(SETTINGNAME(key));
    if (!setting) {
        // not found in cache; search in db
        Data_Set items = find(SETTINGNAME(key));
        if (!items.empty())
            setting = cache_id(items[0].SETTINGID);
        if (!setting) {
            // not found; create
            setting = create();
            setting->SETTINGNAME = key;
        }
    }
    setting->SETTINGVALUE = newValue;
    save(setting);
}
const wxString SettingModel::getRaw(const wxString& key, const wxString& defaultValue)
{
    // search in cache
    Data* setting = search_cache(SETTINGNAME(key));
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
void SettingModel::setString(const wxString& key, const wxString& newValue)
{
    setRaw(key, newValue);
}
const wxString SettingModel::getString(const wxString& key, const wxString& defaultValue)
{
    return getRaw(key, defaultValue);
}

// Bool
void SettingModel::setBool(const wxString& key, bool newValue)
{
    setRaw(key, wxString::Format("%s", newValue ? "TRUE" : "FALSE"));
}
bool SettingModel::getBool(const wxString& key, bool defaultValue)
{
    wxString rawValue = getRaw(key, "");
    if (rawValue == "TRUE") return true;
    if (rawValue == "FALSE") return false;
    return defaultValue; 
}

// Int
void SettingModel::setInt(const wxString& key, int newValue)
{
    setRaw(key, wxString::Format("%d", newValue));
}
int SettingModel::getInt(const wxString& key, int defaultValue)
{
    wxString rawValue = getRaw(key, "");
    if (!rawValue.IsEmpty() && rawValue.IsNumber())
        return wxAtoi(rawValue);
    return defaultValue;
}

// Colour
void SettingModel::setColour(const wxString& key, const wxColour& newValue)
{
    setRaw(key, wxString::Format("%d,%d,%d",
        newValue.Red(), newValue.Green(), newValue.Blue()
    ));
}
const wxColour SettingModel::getColour(const wxString& key, const wxColour& defaultValue)
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
void SettingModel::setJdoc(const wxString& key, Document& newValue)
{
    wxString j_str = JSON_PrettyFormated(newValue);
    setRaw(key, j_str);
}
void SettingModel::setJdoc(const wxString& key, StringBuffer& newValue)
{
    wxString j_str = wxString::FromUTF8(newValue.GetString());
    setRaw(key, j_str);
}
Document SettingModel::getJdoc(const wxString& key, const wxString& defaultValue)
{
    Document j_doc;
    wxString j_str = getRaw(key, defaultValue);
    j_doc.Parse(j_str.utf8_str());
    return j_doc;
}

// ArrayString
void SettingModel::setArrayString(const wxString& key, const wxArrayString& a)
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
    wxLogDebug("SettingModel::setArrayString(%s): %s", key, json_string);
}

const wxArrayString SettingModel::getArrayString(const wxString& key)
{
    wxString rawValue = getRaw(key, "");
    Document j_doc;
    if (rawValue.IsEmpty() ||
        j_doc.Parse(rawValue.utf8_str()).HasParseError() ||
        !j_doc.IsArray()
    )
        return wxArrayString();

    wxArrayString a;
    wxLogDebug("{{{ SettingModel::getArrayString(%s)", key);
    for (rapidjson::SizeType i = 0; i < j_doc.Size(); i++) {
        wxASSERT(j_doc[i].IsString());
        const auto value = wxString::FromUTF8(j_doc[i].GetString());
        wxLogDebug("%s", value);
        a.Add(value);
    }
    wxLogDebug("}}}");
    return a;
}

void SettingModel::prependArrayItem(const wxString& key, const wxString& value, int limit)
{
    if (value.IsEmpty())
        return;
    Data* setting = search_cache(SETTINGNAME(key));
    if (!setting) { // not cached
        Data_Set items = find(SETTINGNAME(key));
        if (!items.empty())
        setting = cache_id(items[0].SETTINGID);
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
    save(setting);
}

//-------------------------------------------------------------------
// VIEWACCOUNTS
void SettingModel::setViewAccounts(const wxString& newValue)
{
    setString("VIEWACCOUNTS", newValue);
}
wxString SettingModel::getViewAccounts()
{
    return getString("VIEWACCOUNTS", VIEW_ACCOUNTS_ALL_STR);
}

// THEME
void SettingModel::setTheme(const wxString& newValue)
{
    setString("THEME", newValue);
}
wxString SettingModel::getTheme()
{
    return getString("THEME", "default");
}

// LASTFILENAME
wxString SettingModel::getLastDbPath()
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
void SettingModel::shrinkUsageTable()
{
    const wxULongLong max_size = 524287;
    const wxULongLong file_size = wxFileName(mmex::getPathUser(mmex::SETTINGS)).GetSize();
    if (file_size < max_size)
        return;

    const wxString save_point = "SETTINGS_TRIM_USAGE";
    wxDate date(wxDate::Now());
    date.Subtract(wxDateSpan::Months(2));
    m_db->Savepoint(save_point);
    try {
        wxString sql = wxString::Format("delete from USAGE_V1 where USAGEDATE < \"%s\";", date.FormatISODate());
        m_db->ExecuteUpdate(sql);
    }
    catch (const wxSQLite3Exception& /*e*/) {
        m_db->Rollback(save_point);
    }
    m_db->ReleaseSavepoint(save_point);
    m_db->Vacuum();
}

row_t SettingModel::to_row_t()
{
    row_t row;
    for (const auto &r: instance().get_all())
        row(r.SETTINGNAME.ToStdWstring()) = r.SETTINGVALUE;
    return row;
}
