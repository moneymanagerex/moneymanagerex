/*******************************************************
 Copyright (C) 2013 James Higley

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

#ifndef MODEL_BUDGETSPLITTRANSACTIONS_H
#define MODEL_BUDGETSPLITTRANSACTIONS_H

#include "Model.h"
#include "db/DB_Table_Budgetsplittransactions_V1.h"

class Model_Budgetsplittransaction : public Model<DB_Table_BUDGETSPLITTRANSACTIONS_V1>
{
public:
    Model_Budgetsplittransaction();
    ~Model_Budgetsplittransaction();

public:
    /**
    * Initialize the global Model_Budgetsplittransaction table.
    * Reset the Model_Budgetsplittransaction table or create the table if it does not exist.
    */
    static Model_Budgetsplittransaction& instance(wxSQLite3Database* db);

    /** Return the static instance of Model_Budgetsplittransaction table */
    static Model_Budgetsplittransaction& instance();

public:
    double get_total(const Data_Set& rows);
};

#endif // 
