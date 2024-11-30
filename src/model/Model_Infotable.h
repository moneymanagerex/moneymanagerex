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

#ifndef MODEL_INFOTABLE_H
#define MODEL_INFOTABLE_H

#include "Model.h"
#include "db/DB_Table_Infotable_V1.h"
#include "defs.h"

class Model_Infotable : public Model<DB_Table_INFOTABLE_V1>
{
public:
    Model_Infotable();
    ~Model_Infotable();

public:
    /**
    Initialize the global Model_Infotable table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for Model_Infotable table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Infotable& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for Model_Infotable table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Infotable& instance();

public:
    // Setter
    void Set(const wxString& key, int value);
    void Set(const wxString& key, bool value);
    void Set(const wxString& key, const wxDateTime& date);
    void Set(const wxString& key, const wxString& value);
    void Set(const wxString& key, const wxColour& value);
    void Set(const wxString& key, const wxSize& size);
    void Prepend(const wxString& key, const wxString& value, int limit);
    void Erase(const wxString& key, int row);
    void Update(const wxString& key, int row, const wxString& value);

public:
    // Getter
    bool GetBoolInfo(const wxString& key, bool default_value);
    int GetIntInfo(const wxString& key, int default_value);
    int64 GetInt64Info(const wxString& key, int64 default_value);
    wxString GetStringInfo(const wxString& key, const wxString& default_value);
    const wxColour GetColourSetting(const wxString& key, const wxColour& default_value = wxColour(255, 255, 255));
    const wxArrayString GetArrayStringSetting(const wxString& key, bool sort = false);
    const wxSize GetSizeSetting(const wxString& key);

    /* Returns true if key setting found */
    bool KeyExists(const wxString& key);
    /* Check database at minimum revision*/
    bool checkDBVersion();

    static loop_t to_loop_t();

public:
    bool OpenCustomDialog(const wxString& RefType);
    void SetOpenCustomDialog(const wxString& RefType, bool Status);
    wxSize CustomDialogSize(const wxString& RefType);
    void SetCustomDialogSize(const wxString& RefType, const wxSize& Size);
    //Use to search through a set of JSON data for a particular label
    int FindLabelInJSON(const wxString& entry, const wxString& labelID);
};

#endif // 
