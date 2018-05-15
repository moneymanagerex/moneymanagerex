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

#ifndef MODEL_INFOTABLE_H
#define MODEL_INFOTABLE_H

#include "Model.h"
#include "Table_Infotable.h"
#include "defs.h"

class Model_Infotable : public Model<DB_Table_INFOTABLE>
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
    void Set(const wxString& key, const wxDateTime& date);
    void Set(const wxString& key, const wxString& value);
    void Set(const wxString& key, const wxColour& value);

public:
    // Getter
    bool GetBoolInfo(const wxString& key, bool default_value);
    int GetIntInfo(const wxString& key, int default_value);
    wxString GetStringInfo(const wxString& key, const wxString& default_value);
    const wxColour GetColourSetting(const wxString& key, const wxColour& default_value = wxColour(255, 255, 255));

    /* Returns true if key setting found */
    bool KeyExists(const wxString& key);
    /* Check database at minimum revision*/
    bool checkDBVersion();

    static loop_t to_loop_t();

public:
    bool OpenCustomDialog(const wxString& RefType);
    void SetOpenCustomDialog(const wxString& RefType, const bool Status);
    wxSize CustomDialogSize(const wxString& RefType);
    void SetCustomDialogSize(const wxString& RefType, const wxSize& Size);
};

#endif // 
