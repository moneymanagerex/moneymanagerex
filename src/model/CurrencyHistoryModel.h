/*******************************************************
Copyright (C) 2015 Gabriele-V

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
#include "_ModelBase.h"
#include "table/CurrencyHistoryTable.h"

class CurrencyHistoryModel : public Model<CurrencyHistoryTable>
{
public:
    using Model<CurrencyHistoryTable>::cache_id;
    enum UPDTYPE { ONLINE = 1, MANUAL };

public:
    CurrencyHistoryModel();
    ~CurrencyHistoryModel();

public:
    /**
    Initialize the global CurrencyHistoryModel table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for StockModel table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static CurrencyHistoryModel& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for CurrencyHistoryModel table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static CurrencyHistoryModel& instance();

public:
    Data* cache_key(const int64& currencyID, const wxDate& date);
    static wxDate CURRDATE(const Data& hist);

    static CurrencyHistoryTable::CURRDATE CURRDATE(OP op, const wxDate& date);
    
    /** Adds or updates an element in currency history */
    int64 addUpdate(const int64 currencyID, const wxDate& date, double price, UPDTYPE type);

    /** Return the rate for a specific currency in a specific day*/
    static double getDayRate(int64 currencyID, const wxString& DateISO);
    static double getDayRate(int64 currencyID, const wxDate& Date = wxDate::Today());

    /** Return the last rate for a specific currency */
    static double getLastRate(const int64& currencyID);
    
    /** Clears the currency History table */
    static void ResetCurrencyHistory();
};

