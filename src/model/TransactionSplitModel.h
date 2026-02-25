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

#pragma once

#include "base/defs.h"
#include "table/TransactionSplitTable.h"
#include "_ModelBase.h"
#include "CurrencyModel.h"

struct Split
{
    int64 CATEGID;
    double SPLITTRANSAMOUNT;
    wxArrayInt64 TAGS;
    wxString NOTES;
};

class TransactionSplitModel : public Model<TransactionSplitTable>
{
public:
    TransactionSplitModel();
    ~TransactionSplitModel();

public:
    /**
    Initialize the global TransactionSplitModel table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for TransactionSplitModel table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static TransactionSplitModel& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for TransactionSplitModel table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static TransactionSplitModel& instance();

    using Model<TransactionSplitTable>::remove;

public:
    static double get_total(const Data_Set& rows);
    static double get_total(const std::vector<Split>& local_splits);
    static const wxString get_tooltip(const std::vector<Split>& local_splits, const CurrencyModel::Data* currency);
    std::map<int64, TransactionSplitModel::Data_Set> get_all_id();
    int update(Data_Set& rows, int64 transactionID);
    int update(const std::vector<Split>& rows, int64 transactionID);
    bool remove(const int64 id);

public:
    static const wxString refTypeName;
};

