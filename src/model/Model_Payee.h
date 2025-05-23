/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

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

#ifndef MODEL_PAYEE_H
#define MODEL_PAYEE_H

#include "Model.h"
#include "db/DB_Table_Payee_V1.h"

class Model_Payee : public Model<DB_Table_PAYEE_V1>
{
public:
    using Model<DB_Table_PAYEE_V1>::remove;
    using Model<DB_Table_PAYEE_V1>::get;

public:
    Model_Payee();
    ~Model_Payee();

public:
    /**
    Initialize the global Model_Payee table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for Model_Payee table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Payee& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for Model_Payee table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Payee& instance();

public:
    const Data_Set FilterPayees(const wxString& payee_pattern);

    /**
    * Return the Data record pointer for the given payee name
    * Returns 0 when payee not found.
    */
    Data* get(const wxString& name);
    static wxString get_payee_name(int64 payee_id);

    bool remove(int64 id);

    const std::map<wxString, int64> all_payees(bool excludeHidden = false);
    const wxArrayString all_payee_names();
    const std::map<wxString, int64> used_payee();

    static bool is_hidden(int64 id);
    static bool is_hidden(const Data* record);
    static bool is_hidden(const Data& record);

    static bool is_used(int64 id);
    static bool is_used(const Data* record);
    static bool is_used(const Data& record);

public:
    static const wxString refTypeName;
};

#endif // 
