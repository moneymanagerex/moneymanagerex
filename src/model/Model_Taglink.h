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

#ifndef MODEL_TAGLINK_H
#define MODEL_TAGLINK_H

#include "Model.h"
#include "db/DB_Table_Taglink_V1.h"

class Model_Taglink : public Model<DB_Table_TAGLINK_V1>
{
public:
    Model_Taglink();
    ~Model_Taglink();

    using Model<DB_Table_TAGLINK_V1>::get;

public:
    /**
    Initialize the global Model_Taglink table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for Model_Taglink table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Taglink& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for Model_Taglink table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Taglink& instance();

    /**
    * Return the Data record pointer for the given payee name
    * Returns 0 when payee not found.
    */
    Data* get(const wxString& refType, int64 refId, int64 tagId);

    /**
    * Return a map of all tags
    */
    std::map<int64, Model_Taglink::Data_Set> get_all(const wxString& refType);

    /**
    * Return a map of all tags for the specified transaction
    * Mostly useful to return a sorted list of tagnames associated with a transaction
    */
    std::map<wxString, int64> get(const wxString& refType, int64 refId);

    /* Delete all tags for a REFTYPE + REFID */
    void DeleteAllTags(const wxString& refType, int64 refID);

    int update(const Data_Set& rows, const wxString& refType, int64 refId);
};

#endif // 
