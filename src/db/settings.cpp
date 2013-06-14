/****************************************************************************
 Copyright (C) 2012 Stefano Giorgio      

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
 ****************************************************************************/
 
#include "settings.h"

const wxString INI_TABLE_NAME = "SETTING_V1";
const char CREATE_INI_TABLE[] =
    "create table SETTING_V1 ("
    "SETTINGID integer not null primary key, "
    "SETTINGNAME TEXT NOT NULL UNIQUE, "
    "SETTINGVALUE TEXT)";

const char UPDATE_INI_RECORD[] = "update SETTING_V1 set SETTINGVALUE = ? where SETTINGNAME = ?";
const char INSERT_INI_RECORD[] = "insert into SETTING_V1 (SETTINGNAME, SETTINGVALUE) values (?, ?)";
const char SELECT_INI_RECORD[] = "select * from SETTING_V1";

//---------------------------------------------------------------------------
const wxString INFO_TABLE_NAME = "INFOTABLE_V1";
const char CREATE_INFO_TABLE[] =
    "create table INFOTABLE_V1 ("
    "INFOID integer not null primary key, "
    "INFONAME TEXT NOT NULL UNIQUE, "
    "INFOVALUE TEXT)";

const char UPDATE_INFO_RECORD[] = "update INFOTABLE_V1 set INFOVALUE = ? where INFONAME = ?";
const char INSERT_INFO_RECORD[] = "insert into INFOTABLE_V1 (INFONAME, INFOVALUE) values (?, ?)";
const char SELECT_INFO_RECORD[] = "select * from INFOTABLE_V1";

/****************************************************************************
 TSettingsEntry Class methods
 ****************************************************************************/
TSettingsEntry::TSettingsEntry(bool main_db, wxString name)
: TEntryBase()
, main_db_(main_db)
, name_(name)
{}

TSettingsEntry::TSettingsEntry(bool main_db, wxSQLite3ResultSet& q1)
: TEntryBase()
, main_db_(main_db)
{
    if (main_db)
    {
        id_    = q1.GetInt("INFOID");
        name_  = q1.GetString("INFONAME");
        value_ = q1.GetString("INFOVALUE");
    }
    else
    {
        id_           = q1.GetInt("SETTINGID");
        name_  = q1.GetString("SETTINGNAME");
        value_ = q1.GetString("SETTINGVALUE");
    }
}

wxString TSettingsEntry::Name()
{
    return name_;
}

wxString TSettingsEntry::Value()
{
    return value_;
}

void TSettingsEntry::SetValue(const wxString& value)
{
    value_ = value;
}

void TSettingsEntry::Save(wxSQLite3Database* db)
{
    try
    {
        wxSQLite3Statement st;
        if (main_db_) st = db->PrepareStatement(UPDATE_INFO_RECORD);
        else          st = db->PrepareStatement(UPDATE_INI_RECORD);
        st.Bind(1, value_);
        st.Bind(2, name_);

        int rows_affected = st.ExecuteUpdate();
        st.Finalize();

        if (!rows_affected)
        {
            if (main_db_) st = db->PrepareStatement(INSERT_INFO_RECORD);
            else          st = db->PrepareStatement(INSERT_INI_RECORD);
            st.Bind(1, name_);
            st.Bind(2, value_);

            rows_affected = st.ExecuteUpdate();
            st.Finalize();
        }
        wxASSERT(rows_affected == 1);
    }
    catch (const wxSQLite3Exception& e)
    {
        wxLogError("TSettingsEntry:Save: %s", e.GetMessage().c_str());
    }
}
/****************************************************************************/

/****************************************************************************
 TSettingsList Class methods
 ****************************************************************************/
TSettingsList::TSettingsList(wxSQLite3Database* db, bool main_db)
: TListBase(db)
, main_db_(main_db)
{
    try
    {
        wxString table_name = INI_TABLE_NAME;
        if (main_db_)
        {
            table_name = INFO_TABLE_NAME;
        }
        if (!db->TableExists(table_name))
        {
            if (main_db_) db->ExecuteUpdate(CREATE_INFO_TABLE);
            else          db->ExecuteUpdate(CREATE_INI_TABLE);
        }
        Load();
    }
    catch (const wxSQLite3Exception& e)
    {
        wxLogError("TSettingsList Constructor: %s", e.GetMessage().c_str());
    }
}

TSettingsList::~TSettingsList()
{
    Save();
}

void TSettingsList::Load()
{
    wxSQLite3ResultSet q1;
    if (main_db_) q1 = ListDatabase()->ExecuteQuery(SELECT_INFO_RECORD);
    else          q1 = ListDatabase()->ExecuteQuery(SELECT_INI_RECORD);
    while (q1.NextRow())
    {
        std::shared_ptr<TSettingsEntry> pRecord(new TSettingsEntry(main_db_, q1));
        ini_records_.push_back(pRecord);
    }
    q1.Finalize();
}

void TSettingsList::Save()
{
    size_t list_size = ini_records_.size();
    size_t element = 0;

    ListDatabase()->Begin();
    while (element < list_size)
    {
        ini_records_[element]->Save(ListDatabase());
        ++ element;
    }
    ListDatabase()->Commit();
}

TSettingsEntry* TSettingsList::GetRecord(const wxString& name)
{
    TSettingsEntry* pRecord = 0;
    size_t list_size = ini_records_.size();
    size_t element = 0;

    while (element < list_size)
    {
        if (ini_records_[element]->Name() == name)
        {
            pRecord = ini_records_[element].get();
            break;
        }
        ++ element;
    }
    return pRecord;
}

bool TSettingsList::GetBoolSetting(const wxString& name, bool default_value)
{
    TSettingsEntry* pRecord = GetRecord(name);
    if (pRecord)
    {
        if (pRecord->Value() == "TRUE") return true;
        else return false;
    }
    return default_value;
}

int TSettingsList::GetIntSetting(const wxString& name, int default_value)
{
    TSettingsEntry* pRecord = GetRecord(name);
    if (pRecord) return wxAtoi(pRecord->Value());
    return default_value;
}

wxString TSettingsList::GetStringSetting(const wxString& name, const wxString& default_value)
{
    TSettingsEntry* pRecord = GetRecord(name);
    if (pRecord) return pRecord->Value();
    return default_value;
}

void TSettingsList::SetBoolSetting(const wxString& name, bool value)
{
    TSettingsEntry* pExistingRecord = GetRecord(name);
    if (!pExistingRecord)
    {
        std::shared_ptr<TSettingsEntry> pNewRecord(new TSettingsEntry(main_db_, name));
        ini_records_.push_back(pNewRecord);
        pExistingRecord = pNewRecord.get();
    }
    if (value) pExistingRecord->SetValue("TRUE");
    else       pExistingRecord->SetValue("FALSE"); 
}

void TSettingsList::SetIntSetting(const wxString& name, int value)
{
    TSettingsEntry* pExistingRecord = GetRecord(name);
    if (!pExistingRecord)
    {
        std::shared_ptr<TSettingsEntry> pNewRecord(new TSettingsEntry(main_db_, name));
        ini_records_.push_back(pNewRecord);
        pExistingRecord = pNewRecord.get();
    }
    pExistingRecord->SetValue(wxString() << value);
}

void TSettingsList::SetStringSetting(const wxString& name, const wxString& value)
{
    TSettingsEntry* pExistingRecord = GetRecord(name);
    if (!pExistingRecord)
    {
        std::shared_ptr<TSettingsEntry> pNewRecord(new TSettingsEntry(main_db_, name));
        ini_records_.push_back(pNewRecord);
        pExistingRecord = pNewRecord.get();
    }
    pExistingRecord->SetValue(value);
}

bool TSettingsList::Exists(const wxString& name)
{
    TSettingsEntry* pRecord = GetRecord(name);
    if(pRecord) return true;
    return false;
}

/****************************************************************************/
