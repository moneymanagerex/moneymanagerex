/*******************************************************
 Copyright (C) 2016 Gabriele-V

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
#include "_ModelBase.h"
#include "table/FieldValueTable.h"

class FieldValueModel : public Model<FieldValueTable>
{
public:
    using Model<FieldValueTable>::cache_id;

public:
    FieldValueModel();
    ~FieldValueModel();

public:
    /**
    Initialize the global FieldValueModel table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for FieldValueModel table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static FieldValueModel& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for FieldValueModel table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static FieldValueModel& instance();

public:
    std::map<int64, Data_Set> get_all_id(const wxString& reftype);
    Data* cache_key(int64 FieldID, int64 RefID);
    wxArrayString allValue(const int64 FieldID);
    static bool DeleteAllData(const wxString& RefType, int64 RefID);
};

