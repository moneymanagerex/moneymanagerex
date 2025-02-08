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
    // Initialize the global Model_Infotable table on initial call.
    // Resets the global table on subsequent calls.
    // Return the static instance address for Model_Infotable table.
    // Note: Assigning the address to a local variable can destroy the instance.
    static Model_Infotable& instance(wxSQLite3Database* db);

    // Return the static instance address for Model_Infotable table.
    // Note: Assigning the address to a local variable can destroy the instance.
    static Model_Infotable& instance();

public:
    bool contains(const wxString& key);

    void setRaw(const wxString& key, const wxString& newValue);
    wxString getRaw(const wxString& key, const wxString& defaultValue);

    void setString(const wxString& key, const wxString& newValue);
    wxString getString(const wxString& key, const wxString& defaultValue);

    void setBool(const wxString& key, bool newValue);
    bool getBool(const wxString& key, bool defaultValue);

    void setInt(const wxString& key, int newValue);
    int getInt(const wxString& key, int defaultValue);

    void setInt64(const wxString& key, int64 newValue);
    int64 getInt64(const wxString& key, int64 defaultValue);

    void setSize(const wxString& key, const wxSize& newValue);
    const wxSize getSize(const wxString& key);

    void setColour(const wxString& key, const wxColour& newValue);
    const wxColour getColour(const wxString& key, const wxColour& defaultValue = wxColour(255, 255, 255));

    void setDate(const wxString& key, const wxDateTime& newValue);

    void setArrayString(const wxString& key, const wxArrayString& a);
    const wxArrayString getArrayString(const wxString& key, bool sort = false);

    int findArrayItem(const wxString& key, const wxString& label);
    void updateArrayItem(const wxString& key, int i, const wxString& newValue);
    void prependArrayItem(const wxString& key, const wxString& value, int limit);
    void eraseArrayItem(const wxString& key, int i);

public:
    void setOpenCustomDialog(const wxString& refType, bool newValue);
    bool getOpenCustomDialog(const wxString& refType);

    void setCustomDialogSize(const wxString& refType, const wxSize& newValue);
    wxSize getCustomDialogSize(const wxString& refType);

public:
    bool checkDBVersion();
    static loop_t to_loop_t();
};

#endif
