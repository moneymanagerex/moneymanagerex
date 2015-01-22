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

#ifndef MODEL_TRANSFERTRANS_H
#define MODEL_TRANSFERTRANS_H

#include "Model.h"
#include "db/DB_Table_Transfertrans_V1.h"
#include "Model_Currency.h" // detect base currency

class Model_TransferTrans : public Model<DB_Table_TRANSFERTRANS_V1>
{
public:
    enum TABLE_TYPE { ASSETS = 0, STOCKS };

public:
    static const std::vector<std::pair<TABLE_TYPE, wxString> > TABLE_TYPE_CHOICES;

public:
    Model_TransferTrans();
    ~Model_TransferTrans();

public:
    /**
    Initialize the global Model_TransferTrans table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for Model_TransferTrans table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_TransferTrans& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for Model_TransferTrans table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_TransferTrans& instance();
    
public:
    static wxArrayString all_table_type();

    static TABLE_TYPE type(const Data* r);
    static TABLE_TYPE type(const Data& r);

    /** Returns the currency Data record pointer*/
    static Model_Currency::Data* currency(const Data* /* r */);
};

#endif // 
