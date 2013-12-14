/*******************************************************
 Copyright (C) 2013 James Higley

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
#include "model/Model_Category.h"
#include "model/Model_Subcategory.h"

class Model_Budget : public Model_Mix<DB_Table_BUDGETTABLE_V1>
{
public:
    Model_Budget(): Model_Mix<DB_Table_BUDGETTABLE_V1>() {};
    ~Model_Budget() 
    {
    };

public:
    enum PERIOD_ENUM { NONE = 0, WEEKLY, BIWEEKLY, MONTHLY, BIMONTHLY, QUARTERLY, HALFYEARLY, YEARLY, DAILY };
    static wxArrayString all_period()
    {
        wxArrayString period;
        // keep the sequence with PERIOD_ENUM
        period.Add(wxTRANSLATE("None"));
        period.Add(wxTRANSLATE("Weekly"));
        period.Add(wxTRANSLATE("Bi-Weekly"));
        period.Add(wxTRANSLATE("Monthly"));
        period.Add(wxTRANSLATE("Bi-Monthly"));
        period.Add(wxTRANSLATE("Quarterly"));
        period.Add(wxTRANSLATE("Half-Yearly"));
        period.Add(wxTRANSLATE("Yearly"));
        period.Add(wxTRANSLATE("Daily"));
        return period;
    }
    static PERIOD_ENUM period(const Data* r)
    {
        if (r->PERIOD.CmpNoCase(all_period()[NONE]) == 0)
            return NONE;
        else if (r->PERIOD.CmpNoCase(all_period()[WEEKLY]) == 0)
            return WEEKLY;
        else if (r->PERIOD.CmpNoCase(all_period()[BIWEEKLY]) == 0)
            return BIWEEKLY;
        else if (r->PERIOD.CmpNoCase(all_period()[MONTHLY]) == 0)
            return MONTHLY;
        else if (r->PERIOD.CmpNoCase(all_period()[BIMONTHLY]) == 0)
            return BIMONTHLY;
        else if (r->PERIOD.CmpNoCase(all_period()[QUARTERLY]) == 0)
            return QUARTERLY;
        else if (r->PERIOD.CmpNoCase(all_period()[HALFYEARLY]) == 0)
            return HALFYEARLY;
        else if (r->PERIOD.CmpNoCase(all_period()[YEARLY]) == 0)
            return YEARLY;
        else if (r->PERIOD.CmpNoCase(all_period()[DAILY]) == 0)
            return DAILY;
        else
            return NONE;
    }
    static PERIOD_ENUM period(const Data& r) { return period(&r); }
    static DB_Table_BUDGETTABLE_V1::PERIOD PERIOD(PERIOD_ENUM period, OP op = EQUAL) { return DB_Table_BUDGETTABLE_V1::PERIOD(all_period()[period], op); }

public:
    /** Return the static instance of Model_Budget table */
    static Model_Budget& instance()
    {
        return Singleton<Model_Budget>::instance();
    }

    /**
    * Initialize the global Model_Budget table.
    * Reset the Model_Budget table or create the table if it does not exist.
    */
    static Model_Budget& instance(wxSQLite3Database* db)
    {
        Model_Budget& ins = Singleton<Model_Budget>::instance();
        ins.db_ = db;
        ins.destroy_cache();
        ins.ensure(db);

        return ins;
    }

    static void getBudgetEntry(int budgetYearID, std::map<int, std::map<int, PERIOD_ENUM> > &budgetPeriod,
        std::map<int, std::map<int, double> > &budgetAmt)
    {
        const Model_Category::Data_Set allCategories = Model_Category::instance().all();
        const Model_Subcategory::Data_Set allSubcategories = Model_Subcategory::instance().all();
        //Set std::map with zerros
        double value = 0;
        for (const auto& category : allCategories)
        {
            budgetPeriod[category.CATEGID][-1] = NONE;
            budgetAmt[category.CATEGID][-1] = value;
            for (const auto & sub_category : allSubcategories)
            {
                if (sub_category.CATEGID == category.CATEGID)
                {
                    budgetPeriod[category.CATEGID][sub_category.SUBCATEGID] = NONE;
                    budgetAmt[category.CATEGID][sub_category.SUBCATEGID] = value;
                }
            }
        }
        for (const auto& budget : instance().find(BUDGETYEARID(budgetYearID)))
        {
            budgetPeriod[budget.CATEGID][budget.SUBCATEGID] = period(budget);
            budgetAmt[budget.CATEGID][budget.SUBCATEGID] = budget.AMOUNT;
        }
    }
    static void copyBudgetYear(int newYearID, int baseYearID)
    {
        for (const Data& data : instance().find(BUDGETYEARID(baseYearID)))
        {
            Data* budgetEntry = instance().clone(&data);
            budgetEntry->BUDGETYEARID = newYearID;
            instance().save(budgetEntry);
        }
    }
    static double getMonthlyEstimate(const PERIOD_ENUM period, const double amount)
    {
        double estimated = 0;
        int ndays = 365;
        if (period == MONTHLY) {
            estimated = amount;

        }
        else if (period == YEARLY) {
            estimated = amount / 12;

        }
        else if (period == WEEKLY) {
            estimated = ((amount / 7) * ndays) / 12;

        }
        else if (period == BIWEEKLY) {
            estimated = ((amount / 14) * ndays) / 12;

        }
        else if (period == BIMONTHLY) {
            estimated = amount / 2;

        }
        else if (period == QUARTERLY) {
            estimated = amount / 3;

        }
        else if (period == HALFYEARLY) {
            estimated = (amount / 6);

        }
        else if (period == DAILY) {
            estimated = (amount * ndays) / 12;

        }
        return estimated;
    }
    static double getYearlyEstimate(const PERIOD_ENUM period, const double amount)
    {
        double estimated = 0;
        if (period == MONTHLY) {
            estimated = amount * 12;

        }
        else if (period == YEARLY) {
            estimated = amount;

        }
        else if (period == WEEKLY) {
            estimated = amount * 52;

        }
        else if (period == BIWEEKLY) {
            estimated = amount * 26;

        }
        else if (period == BIMONTHLY) {
            estimated = amount * 6;

        }
        else if (period == QUARTERLY) {
            estimated = amount * 4;

        }
        else if (period == HALFYEARLY) {
            estimated = amount * 2;

        }
        else if (period== DAILY) {
            estimated = amount * 365;

        }
        return estimated;
    }
};

#endif // 
