/*******************************************************
 Copyright (C) 2013,2014 James Higley

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
#include "Table_Budgettable.h"

class Model_Budget : public Model<DB_Table_BUDGETTABLE>
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
    enum PERIOD_ENUM { NONE = 0, WEEKLY, BIWEEKLY, MONTHLY, BIMONTHLY, QUARTERLY, HALFYEARLY, YEARLY, DAILY };
    static const std::vector<std::pair<PERIOD_ENUM, wxString> > PERIOD_ENUM_CHOICES;

    static wxArrayString all_period();
    static PERIOD_ENUM period(const Data* r);
    static PERIOD_ENUM period(const Data& r);
    static DB_Table_BUDGETTABLE::PERIOD PERIOD(PERIOD_ENUM period, OP op = EQUAL);

    static void getBudgetEntry(int budgetYearID, std::map<int,
        std::map<int, PERIOD_ENUM> > &budgetPeriod,
        std::map<int, std::map<int, double> > &budgetAmt);
    static void copyBudgetYear(int newYearID, int baseYearID);
    static double getMonthlyEstimate(const PERIOD_ENUM period, const double amount);
    static double getYearlyEstimate(const PERIOD_ENUM period, const double amount);
};

#endif // 
