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

#pragma once
#include "entry_base.h"

/****************************************************************************
 TSettingsEntry Class

 This class holds a single record for the Initalization database
 ****************************************************************************/
class TSettingsEntry : public TEntryBase
{
public:
    /// Create a new record setting.
    TSettingsEntry(bool main_db, wxString name);

    /// Create an existing record setting.
    TSettingsEntry(bool main_db, wxSQLite3ResultSet& q1);

    /// Set the value of the record
    void SetValue(const wxString& value);
    void Save(wxSQLite3Database* db);
    wxString Name();
    wxString Value();

private:
    bool main_db_;
    wxString name_;
    wxString value_;
};

/****************************************************************************
 TSettingsList Class

 This class holds all the records for the Initalization database
 ****************************************************************************/
class TSettingsList : public TListBase
{
public:
    /// Constructor
    TSettingsList(wxSQLite3Database* db, bool main_db = false);
    ~TSettingsList();

    bool GetBoolSetting(const wxString& name, bool default_value);
    int GetIntSetting(const wxString& name, int default_value);
    wxString GetStringSetting(const wxString& name, const wxString& default_value);

    /// Save to existing value
    void SetBoolSetting(const wxString& name, bool value);
    void SetIntSetting(const wxString& name, int value);
    void SetStringSetting(const wxString& name, const wxString& value);

    bool Exists(const wxString& name);
    void Load();
    void Save();
private:
    bool main_db_;
    std::vector< std::shared_ptr<TSettingsEntry> > ini_records_;

    TSettingsEntry* GetRecord(const wxString& name);
};
