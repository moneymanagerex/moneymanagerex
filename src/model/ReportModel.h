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

#pragma once

#include "base/defs.h"
#include "util/mmSingleton.h"
#include "table/_TableFactory.h"
#include "data/ReportData.h"

class ReportRecord : public std::map<std::wstring, std::wstring>
{
public:
    ReportRecord() {}
    ~ReportRecord() {}

    // Access functions for LuaGlue
    // The required conversion between char and wchar_t is done through wxString.
    std::string get(const char* index)
    { 
        return std::string(wxString((*this)[wxString(index).ToStdWstring()]).ToUTF8());
    }
    void set(const char* index, const char * val)
    {
        (*this)[wxString(index).ToStdWstring()] = wxString::FromUTF8(val).ToStdWstring();
    }
};

struct ReportParam
{
    wxString label;
    wxString type;
    wxString def_value;
    int      ID;
    wxString name;

    static auto get_param_a() -> const std::vector<ReportParam>;
    static auto get_label_name_a() -> const std::vector<std::pair<wxString, wxString>>;
    static bool prepare_sql(wxString& query, std::map<wxString, wxString>& label_value_m);
};

class ReportModel : public TableFactory<ReportTable, ReportData>
{
public:
    ReportModel(); 
    ~ReportModel();

public:
    static ReportModel& instance(wxSQLite3Database* db);
    static ReportModel& instance();

public:
    auto get_name_data_n(const wxString& name) -> const Data*;
    auto find_all_group_name_a() -> const wxArrayString;
    int  generate_html(const Data& r, wxString& out);

    // not used
    bool sql_result_as_json(const wxString& query, PrettyWriter<StringBuffer>& json_writer);
};

