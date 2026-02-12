/*******************************************************
 Copyright (C) 2016 Guan Lisheng (guanlisheng@gmail.com)

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

#include "defs.h"
#include "db/DB_Table_Tag_V1.h"
#include "_Model.h"

class TagModel : public Model<DB_Table_TAG_V1>
{
public:
    TagModel();
    ~TagModel();

    using Model<DB_Table_TAG_V1>::get;

public:
    /**
    Initialize the global TagModel table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for TagModel table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static TagModel& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for TagModel table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static TagModel& instance();

    /**
    * Return the Data record pointer for the given tag name
    * Returns 0 when tag not found.
    */
    Data* get(const wxString& name);

    /* Returns 0 if not used, 1 if used, and -1 if used only in deleted transactions */
    int is_used(int64 id);
};

