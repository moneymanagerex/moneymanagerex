/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)

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

#ifndef MODEL_SETTING_H
#define MODEL_SETTING_H

#include "Model.h"
#include "Table_Setting_V1.h"
#include "defs.h"

class Model_Setting : public Model<DB_Table_SETTING_V1>
{
public:
    Model_Setting();
    ~Model_Setting();

public:
    /**
    Initialize the global Model_Setting table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for Model_Setting table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Setting& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for Model_Setting table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Setting& instance();
    void Savepoint()
    {
        this->db_->Savepoint("MMEX_Setting");
    }
    void ReleaseSavepoint()
    {
        this->db_->ReleaseSavepoint("MMEX_Setting");
    }
    void Rollback()
    {
        this->db_->Rollback("MMEX_Setting");
    }

public:
    // Setter
    void Set(const wxString& key, int value);
    void Set(const wxString& key, bool value);
    void Set(const wxString& key, const wxColour& value);
    void Set(const wxString& key, const wxString& value);

public:
    // Getter
    bool GetBoolSetting(const wxString& key, bool default_value);
    int GetIntSetting(const wxString& key, int default_value);

    wxString GetStringSetting(const wxString& key, const wxString& default_value);
    wxString getLastDbPath();

    /* Returns true if key setting found */
    bool ContainsSetting(const wxString& key);

    static row_t to_row_t();

public:
    wxString ViewAccounts();
    void SetViewAccounts(const wxString& value);

    wxString ViewTransactions();
    void SetViewTransactions(const wxString& value);

    void ShrinkUsageTable();
};

#endif // 
