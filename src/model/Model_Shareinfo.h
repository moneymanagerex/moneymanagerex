/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2016 Stefano Giorgio

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
#include "db/DB_Table_Shareinfo_V1.h"

class Model_Shareinfo : public Model<DB_Table_SHAREINFO_V1>
{
public:
    Model_Shareinfo();
    ~Model_Shareinfo();

public:
    /**
    Initialize the global Model_Shareinfo table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for Model_Shareinfo table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Shareinfo& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for Model_Shareinfo table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Shareinfo& instance();

public:
    /** Return the list of share records for the given checking_id */
    static Model_Shareinfo::Data_Set ShareList(const int checking_id);

    /* Return the share record for the checking account */
    static Model_Shareinfo::Data ShareEntry(const int checking_id);

    /**
    * Set the share record for the checking account.
    * Create one if the record does not exist
    */
    static void ShareEntry(const int checking_id
        , const double share_number
        , const double share_price
        , const double share_commission
        , const wxString share_lot);

    /* Remove the share entry associated with the checking account entry */
    static void RemoveShareEntry(const int checking_id);
};
