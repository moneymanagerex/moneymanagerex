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

#pragma once

#include "base/defs.h"

#include "table/SettingTable.h"
#include "data/SettingData.h"

#include "_ModelBase.h"

class SettingModel : public TableFactory<SettingTable, SettingData>
{
public:
    SettingModel();
    ~SettingModel();

public:
    static SettingModel& instance(wxSQLite3Database* db);
    static SettingModel& instance();

public:
    void setting_savepoint()
    {
        this->m_db->Savepoint("MMEX_Setting");
    }
    void setting_release_savepoint()
    {
        this->m_db->ReleaseSavepoint("MMEX_Setting");
    }
    void setting_rollback()
    {
        this->m_db->Rollback("MMEX_Setting");
    }

public:
    bool contains(const wxString& key);

    void setRaw(const wxString& key, const wxString& newValue);
    auto getRaw(const wxString& key, const wxString& defaultValue) -> const wxString;

    void setString(const wxString& key, const wxString& newValue);
    auto getString(const wxString& key, const wxString& defaultValue) -> const wxString;

    void setBool(const wxString& key, bool newValue);
    bool getBool(const wxString& key, bool defaultValue);

    void setInt(const wxString& key, int newValue);
    int  getInt(const wxString& key, int defaultValue);

    void setColour(const wxString& key, const wxColour& newValue);
    auto getColour(const wxString& key, const wxColour& defaultValue) -> const wxColour;

    void setJdoc(const wxString& key, Document& newValue);
    void setJdoc(const wxString& key, StringBuffer& newValue);
    auto getJdoc(const wxString& key, const wxString& defaultValue) -> Document;

    void setArrayString(const wxString& key, const wxArrayString& a);
    auto getArrayString(const wxString& key) -> const wxArrayString;

    void prependArrayItem(const wxString& key, const wxString& value, int limit);

    void setViewAccounts(const wxString& newValue);
    auto getViewAccounts() -> const wxString;
    
    void setTheme(const wxString& newValue);
    auto getTheme() -> const wxString;

    auto getLastDbPath() -> const wxString;

    void shrinkUsageTable();
    auto to_html_row() -> row_t;
};

