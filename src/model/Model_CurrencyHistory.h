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

#ifndef MODEL_CURRENCYHISTORY_H
#define MODEL_CURRENCYHISTORY_H

#include "Model.h"
#include "Table_Currencyhistory.h"

class Model_CurrencyHistory : public Model<DB_Table_CURRENCYHISTORY>
{
public:
    using Model<DB_Table_CURRENCYHISTORY>::get;
    enum UPDTYPE { ONLINE = 1, MANUAL };

public:
    Model_CurrencyHistory();
    ~Model_CurrencyHistory();

public:
    /**
    Initialize the global Model_CurrencyHistory table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for Model_Stock table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_CurrencyHistory& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for Model_CurrencyHistory table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_CurrencyHistory& instance();

public:
    Data* get(const int& currencyID, const wxDate& date);
    static wxDate CURRDATE(const Data& hist);

    static DB_Table_CURRENCYHISTORY::CURRDATE CURRDATE(const wxDate& date, OP op = EQUAL);
    
    /** Adds or updates an element in currency history */
    int addUpdate(const int& currencyID, const wxDate& date, double price, UPDTYPE type);

    /** Return the rate for a specific currency in a specific day*/
    static double getDayRate(const int& currencyID, const wxString& DateISO);
    static double getDayRate(const int& currencyID, const wxDate& Date = wxDate::Today());

    /** Return the last rate for a specific currency */
    static double getLastRate(const int& currencyID);
    /** Return the last currency rate not after the date */
    static double getLastRate(const int& currencyID, const wxString& dateISO);
    
    /** Clears the currency History table */
    static void ResetCurrencyHistory();
};

#endif // 
