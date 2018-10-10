/*******************************************************
Copyright (C) 2013 - 2018 Guan Lisheng (guanlisheng@gmail.com)
Copyright (C) 2018 Stefano Giorgio (stef145g)

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

#include "Model.h"
#include "Table_Usage_V1.h"

class Model_Usage : public Model<DB_Table_USAGE_V1>
{
public:
    Model_Usage();
    ~Model_Usage();

public:
    /**
    Initialize the global Model_Usage table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for Model_Usage table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Usage& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for Model_Usage table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Usage& instance();

private:
    wxDateTime m_start;
    wxArrayString m_json_usage, m_json_cache;

public:
    // Appends to usage array 
    void AppendToUsage(const wxString& json_string);

    // Appends to cache array 
    void AppendToCache(const wxString& json_string);

    //Return a json string
    wxString To_JSON_String() const;

public:
    void pageview(const wxString& documentPath, const wxString& documentTitle, int plt = 0 /*msec*/);
    void timing(const wxString& documentPath, const wxString& documentTitle, int plt = 0 /*msec*/);
    void pageview(const wxWindow* window, int plt = 0 /*msec*/);
};
