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

#ifndef MODEL_SPLITTRANSACTIONS_H
#define MODEL_SPLITTRANSACTIONS_H

#include "Model.h"
#include "db/DB_Table_Splittransactions_V1.h"

class Model_Splittransaction : public Model<DB_Table_SPLITTRANSACTIONS_V1>
{
public:
    Model_Splittransaction();
    ~Model_Splittransaction();

public:
    /**
    * Initialize the global Model_Splittransaction table.
    * Reset the Model_Splittransaction table or create the table if it does not exist.
    */
    static Model_Splittransaction& instance(wxSQLite3Database* db);

    /** Return the static instance of Model_Splittransaction table */
    static Model_Splittransaction& instance();

public:
    static double get_total(const Data_Set& rows);
};

#endif // 
