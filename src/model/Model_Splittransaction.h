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

#ifndef MODEL_SPLITTRANSACTIONS_H
#define MODEL_SPLITTRANSACTIONS_H

#include "Model.h"
#include "Model_Currency.h"
#include "db/DB_Table_Splittransactions_V1.h"

struct Split
{
    int64 CATEGID;
    double SPLITTRANSAMOUNT;
    wxArrayInt64 TAGS;
    wxString NOTES;
};

class Model_Splittransaction : public Model<DB_Table_SPLITTRANSACTIONS_V1>
{
public:
    Model_Splittransaction();
    ~Model_Splittransaction();

public:
    /**
    Initialize the global Model_Splittransaction table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for Model_Splittransaction table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Splittransaction& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for Model_Splittransaction table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Splittransaction& instance();

    using Model<DB_Table_SPLITTRANSACTIONS_V1>::remove;

public:
    static double get_total(const Data_Set& rows);
    static double get_total(const std::vector<Split>& local_splits);
    static const wxString get_tooltip(const std::vector<Split>& local_splits, const Model_Currency::Data* currency);
    std::map<int64, Model_Splittransaction::Data_Set> get_all();
    int update(Data_Set& rows, int64 transactionID);
    int update(const std::vector<Split>& rows, int64 transactionID);
    bool remove(int64 id);

public:
    static const wxString refTypeName;
};

#endif // 
