/*******************************************************
 Copyright (C) 2016 Guan Lisheng

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

#include "Model_Pragma.h"

// https://www.sqlite.org/pragma.html

Model_Pragma::Model_Pragma()
: Model<DB_Table>()
{
}

Model_Pragma::~Model_Pragma()
{
}

/**
* Initialize the global Model_Pragma.
*/
Model_Pragma& Model_Pragma::instance(wxSQLite3Database* db)
{
    Model_Pragma& ins = Singleton<Model_Pragma>::instance();
    ins.db_ = db;

    return ins;
}

/** Return the static instance of Model_Pragma */
Model_Pragma& Model_Pragma::instance()
{
    return Singleton<Model_Pragma>::instance();
}

