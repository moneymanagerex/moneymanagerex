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
 
/****************************************************************************
 Revision of last commit: $Revision$
 Author of last commit:   $Author$
 ****************************************************************************/
#include "mmex_settings.h"

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
 MMEX_IniRecord Class methods
 ****************************************************************************/
MMEX_IniRecord::MMEX_IniRecord(std::shared_ptr<wxSQLite3Database> ini_db
    , bool main_db
    , const wxString& name)
: iniDb_(ini_db)
, main_db_(main_db)
, settingId_(-1)
, settingName_(name)
{}

MMEX_IniRecord::MMEX_IniRecord(std::shared_ptr<wxSQLite3Database> ini_db
    , bool main_db
    , wxSQLite3ResultSet& q1)
: iniDb_(ini_db)
, main_db_(main_db)
{
    if (main_db)
    {
        settingId_    = q1.GetInt("INFOID");
        settingName_  = q1.GetString("INFONAME");
        settingValue_ = q1.GetString("INFOVALUE");
    }
    else
    {
        settingId_    = q1.GetInt("SETTINGID");
        settingName_  = q1.GetString("SETTINGNAME");
        settingValue_ = q1.GetString("SETTINGVALUE");
    }
}

wxString MMEX_IniRecord::Name()
{
    return settingName_;
}

wxString MMEX_IniRecord::Value()
{
    return settingValue_;
}

void MMEX_IniRecord::SetValue(const wxString& value)
{
    settingValue_ = value;
}

void MMEX_IniRecord::Save()
{
    try
    {
        wxSQLite3Statement st;
        if (main_db_) st = iniDb_->PrepareStatement(UPDATE_INFO_RECORD);
        else          st = iniDb_->PrepareStatement(UPDATE_INI_RECORD);
        st.Bind(1, settingValue_);
        st.Bind(2, settingName_);

        int rows_affected = st.ExecuteUpdate();
        st.Finalize();

        if (!rows_affected)
        {
            if (main_db_) st = iniDb_->PrepareStatement(INSERT_INFO_RECORD);
            else          st = iniDb_->PrepareStatement(INSERT_INI_RECORD);
            st.Bind(1, settingName_);
            st.Bind(2, settingValue_);

            rows_affected = st.ExecuteUpdate();
            st.Finalize();
        }
        wxASSERT(rows_affected == 1);
    }
    catch (const wxSQLite3Exception& e)
    {
        wxLogDebug("MMEX_IniRecord::Save() - Exception: %s", e.GetMessage());
        wxLogError("MMEX_IniRecord::Save() - Insert/Update. " + wxString::Format(_("Error: %s"), e.GetMessage()));
    }
}
/****************************************************************************/

/****************************************************************************
 MMEX_IniSettings Class methods
 ****************************************************************************/
MMEX_IniSettings::MMEX_IniSettings(std::shared_ptr<wxSQLite3Database> ini_db, bool main_db)
: ini_db_(ini_db)
, main_db_(main_db)
{
    try
    {
        wxString table_name = INI_TABLE_NAME;
        if (main_db_)
        {
            table_name = INFO_TABLE_NAME;
        }
        if (!ini_db->TableExists(table_name))
        {
            if (main_db_) ini_db->ExecuteUpdate(CREATE_INFO_TABLE);
            else          ini_db->ExecuteUpdate(CREATE_INI_TABLE);
        }
        Load();
    }
    catch (const wxSQLite3Exception& e)
    {
        wxLogDebug("MMEX_IniSettings Constructor: Exception: %s", e.GetMessage());
        wxLogError("MMEX_IniSettings Constructor: create/Load table. " + wxString::Format(_("Error: %s"), e.GetMessage()));
    }
}

MMEX_IniSettings::~MMEX_IniSettings()
{
    Save();
}

void MMEX_IniSettings::Load()
{
    wxSQLite3ResultSet q1;
    if (main_db_) q1 = ini_db_->ExecuteQuery(SELECT_INFO_RECORD);
    else          q1 = ini_db_->ExecuteQuery(SELECT_INI_RECORD);
    while (q1.NextRow())
    {
        std::shared_ptr<MMEX_IniRecord> pRecord(new MMEX_IniRecord(ini_db_, main_db_, q1));
        ini_records_.push_back(pRecord);
    }
    q1.Finalize();
}

void MMEX_IniSettings::Save()
{
    size_t list_size = ini_records_.size();
    size_t element = 0;

    ini_db_->Begin();
    while (element < list_size)
    {
        ini_records_[element]->Save();
        ++ element;
    }
    ini_db_->Commit();
}

MMEX_IniRecord* MMEX_IniSettings::GetRecord(const wxString& name)
{
    MMEX_IniRecord* pRecord = 0;
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

bool MMEX_IniSettings::GetBoolSetting(const wxString& name, bool default_value)
{
    MMEX_IniRecord* pRecord = GetRecord(name);
    if (pRecord)
    {
        if (pRecord->Value() == "TRUE") return true;
        else return false;
    }
    return default_value;
}

int MMEX_IniSettings::GetIntSetting(const wxString& name, int default_value)
{
    MMEX_IniRecord* pRecord = GetRecord(name);
    if (pRecord) return wxAtoi(pRecord->Value());
    return default_value;
}

wxString MMEX_IniSettings::GetStringSetting(const wxString& name, const wxString& default_value)
{
    MMEX_IniRecord* pRecord = GetRecord(name);
    if (pRecord) return pRecord->Value();
    return default_value;
}

void MMEX_IniSettings::SetBoolSetting(const wxString& name, bool value)
{
    MMEX_IniRecord* pExistingRecord = GetRecord(name);
    if (!pExistingRecord)
    {
        std::shared_ptr<MMEX_IniRecord> pNewRecord(new MMEX_IniRecord(ini_db_, main_db_, name));
        ini_records_.push_back(pNewRecord);
        pExistingRecord = pNewRecord.get();
    }
    if (value) pExistingRecord->SetValue("TRUE");
    else       pExistingRecord->SetValue("FALSE"); 
}

void MMEX_IniSettings::SetIntSetting(const wxString& name, int value)
{
    MMEX_IniRecord* pExistingRecord = GetRecord(name);
    if (!pExistingRecord)
    {
        std::shared_ptr<MMEX_IniRecord> pNewRecord(new MMEX_IniRecord(ini_db_, main_db_, name));
        ini_records_.push_back(pNewRecord);
        pExistingRecord = pNewRecord.get();
    }
    pExistingRecord->SetValue(wxString() << value);
}

void MMEX_IniSettings::SetStringSetting(const wxString& name, const wxString& value)
{
    MMEX_IniRecord* pExistingRecord = GetRecord(name);
    if (!pExistingRecord)
    {
        std::shared_ptr<MMEX_IniRecord> pNewRecord(new MMEX_IniRecord(ini_db_, main_db_, name));
        ini_records_.push_back(pNewRecord);
        pExistingRecord = pNewRecord.get();
    }
    pExistingRecord->SetValue(value);
}

bool MMEX_IniSettings::Exists(const wxString& name)
{
    MMEX_IniRecord* pRecord = GetRecord(name);
    if(pRecord) return true;
    return false;
}

/****************************************************************************/
