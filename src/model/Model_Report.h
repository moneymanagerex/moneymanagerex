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

#ifndef MODEL_REPORT_H
#define MODEL_REPORT_H

#include "Model.h"
#include "db/DB_Table_Report_V1.h"

class Model_Report : public Model<DB_Table_REPORT_V1>
{
public:
    using Model<DB_Table_REPORT_V1>::get;

public:
    Model_Report(); 
    ~Model_Report();

public:
    /**
    Initialize the global Model_Report table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for Model_Report table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Report& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for Model_Report table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Report& instance();

public:
    bool get_objects_from_sql(const wxString& query, PrettyWriter<StringBuffer>& json_writer);
    wxArrayString allGroupNames();
    int get_html(const Data* r, wxString& out);
    //wxString get_html(const Data& r);

public:
    Data* get(const wxString& name);
    static bool PrepareSQL(wxString& sql, std::map <wxString, wxString>& rep_params);
    static const std::vector<std::pair<wxString, wxString>> getParamNames();

private:
    struct Values
    {
        wxString label;
        wxString type;
        wxString def_value;
        int ID;
        wxString name;
    };
    static const std::vector<Values> SqlPlaceHolders();
};

#endif // 
