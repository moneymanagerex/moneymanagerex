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

#include "Model.h"
#include "Table.h"

class Model_Pragma : public Model<DB_Table>
{
public:
    Model_Pragma();
    ~Model_Pragma();

public:
    /**
    Initialize the global Model_Pragma table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for Model_Pragma table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Pragma& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for Model_Pragma table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Pragma& instance();
};
