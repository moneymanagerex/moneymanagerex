/*******************************************************
 Copyright (C) 2013,2014 James Higley
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

#ifndef MODEL_BUDGET_H
#define MODEL_BUDGET_H

#include "Model.h"
#include "db/DB_Table_Budgettable_V1.h"
#include "reports/mmDateRange.h"
#include <float.h>

class Model_Budget : public Model<DB_Table_BUDGETTABLE_V1>
{
public:
    Model_Budget();
    ~Model_Budget();

public:
    /**
    Initialize the global Model_Budget table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for Model_Budget table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Budget& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for Model_Budget table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Budget& instance();

public:
    enum PERIOD_ID
    {
        PERIOD_ID_NONE = 0,
        PERIOD_ID_WEEKLY,
        PERIOD_ID_BIWEEKLY,
        PERIOD_ID_MONTHLY,
        PERIOD_ID_BIMONTHLY,
        PERIOD_ID_QUARTERLY,
        PERIOD_ID_HALFYEARLY,
        PERIOD_ID_YEARLY,
        PERIOD_ID_DAILY,
        PERIOD_ID_MAX
    };
    static wxArrayString PERIOD_STR;

private:
    static const std::vector<std::pair<PERIOD_ID, wxString> > PERIOD_CHOICES;
    static wxArrayString period_str_all();

public:
    static wxArrayString period_loc_all();
    static PERIOD_ID period_id(const Data* r);
    static PERIOD_ID period_id(const Data& r);
    static DB_Table_BUDGETTABLE_V1::PERIOD PERIOD(PERIOD_ID period, OP op = EQUAL);

    static void getBudgetEntry(int budgetYearID,
        std::map<int, PERIOD_ID> &budgetPeriod,
        std::map<int, double> &budgetAmt,
        std::map<int, wxString> &budgetNotes);
    static void getBudgetStats(
        std::map<int, std::map<int, double> > &budgetStats
        , mmDateRange* date_range
        , bool groupByMonth);
    static void copyBudgetYear(int newYearID, int baseYearID);
    static double getEstimate(bool is_monthly, const PERIOD_ID period, const double amount);
};

#endif // 
