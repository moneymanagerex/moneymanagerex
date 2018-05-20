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

#ifndef MODEL_SUBCATEGORY_H
#define MODEL_SUBCATEGORY_H

#include "Model.h"
#include "Table_Subcategory.h"

class Model_Subcategory : public Model<DB_Table_SUBCATEGORY>
{
public:
    using Model<DB_Table_SUBCATEGORY>::get;

public:
    Model_Subcategory();
    ~Model_Subcategory();

public:
    /**
    Initialize the global Model_Subcategory table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for Model_Subcategory table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Subcategory& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for Model_Subcategory table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Subcategory& instance();

public:
    /** Return the Data record for the given subcategory name and category ID */
    Data* get(const wxString& name, int category_id);

public:
    static bool is_used(int id);
};

#endif // 
