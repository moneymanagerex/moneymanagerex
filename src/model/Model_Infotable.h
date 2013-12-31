/*******************************************************
 Copyright (C) 2013 Guan Lisheng (guanlisheng@gmail.com)

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
    * Initialize the global Model_Infotable.
    * Reset the Model_Infotable or create the table if it does not exist.
    */
    static Model_Infotable& instance(wxSQLite3Database* db);

    /** Return the static instance of Model_Infotable */
    static Model_Infotable& instance();

public:
    // Setter
    void Set(const wxString& key, int value);
    void Set(const wxString& key, const wxDateTime& date);
    void Set(const wxString& key, const wxString& value);

public:
    // Getter
    bool GetBoolInfo(const wxString& key, bool default_value);
    int GetIntInfo(const wxString& key, int default_value);
    wxString GetStringInfo(const wxString& key, const wxString& default_value);

    int GetBaseCurrencyId();
    void SetBaseCurrencyID(int currency_id);

    /* Returns true if key setting found */
    bool KeyExists(const wxString& key);
    bool checkDBVersion();

    static loop_t to_loop_t();
};

#endif // 
