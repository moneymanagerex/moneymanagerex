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

#include "Model_Budget.h"
#include <wx/intl.h>
#include "Model_Category.h"

Model_Budget::Model_Budget()
: Model<DB_Table_BUDGETTABLE>()
{
}

Model_Budget::~Model_Budget()
{
}

/**
* Initialize the global Model_Budget table.
* Reset the Model_Budget table or create the table if it does not exist.
*/
Model_Budget& Model_Budget::instance(wxSQLite3Database* db)
{
    Model_Budget& ins = Singleton<Model_Budget>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);
    
    return ins;
}

/** Return the static instance of Model_Budget table */
Model_Budget& Model_Budget::instance()
{
    return Singleton<Model_Budget>::instance();
}

const std::vector<std::pair<Model_Budget::PERIOD_ENUM, wxString> > Model_Budget::PERIOD_ENUM_CHOICES =
{
    {Model_Budget::NONE, wxString(wxTRANSLATE("None"))}
    , {Model_Budget::WEEKLY, wxString(wxTRANSLATE("Weekly"))}
    , {Model_Budget::BIWEEKLY, wxString(wxTRANSLATE("Bi-Weekly"))}
    , {Model_Budget::MONTHLY, wxString(wxTRANSLATE("Monthly"))}
    , {Model_Budget::BIMONTHLY, wxString(wxTRANSLATE("Bi-Monthly"))}
    , {Model_Budget::QUARTERLY, wxString(wxTRANSLATE("Quarterly"))}
    , {Model_Budget::HALFYEARLY, wxString(wxTRANSLATE("Half-Yearly"))}
    , {Model_Budget::YEARLY, wxString(wxTRANSLATE("Yearly"))}
    , {Model_Budget::DAILY, wxString(wxTRANSLATE("Daily"))}
};

wxArrayString Model_Budget::all_period()
{
    static wxArrayString period;
    if (period.empty())
    {
        for (const auto& item : PERIOD_ENUM_CHOICES)
            period.Add(wxGetTranslation(item.second));
    }
    return period;
}

Model_Budget::PERIOD_ENUM Model_Budget::period(const Data* r)
{
    for (const auto &entry : PERIOD_ENUM_CHOICES)
    {
        if (r->PERIOD.CmpNoCase(entry.second) == 0) return entry.first;
    }
    return NONE;
}

Model_Budget::PERIOD_ENUM Model_Budget::period(const Data& r)
{
    return period(&r);
}

DB_Table_BUDGETTABLE::PERIOD Model_Budget::PERIOD(PERIOD_ENUM period, OP op)
{
    return DB_Table_BUDGETTABLE::PERIOD(all_period()[period], op);
}

void Model_Budget::getBudgetEntry(int budgetYearID
    , std::map<int, std::map<int, PERIOD_ENUM> > &budgetPeriod
    , std::map<int, std::map<int, double> > &budgetAmt)
{
    //Set std::map with zerros
    double value = 0;
    for (const auto& category : Model_Category::all_categories())
    {
        budgetPeriod[category.second.first][category.second.second] = NONE;
        budgetAmt[category.second.first][category.second.second] = value;
    }

    for (const auto& budget : instance().find(BUDGETYEARID(budgetYearID)))
    {
        budgetPeriod[budget.CATEGID][budget.SUBCATEGID] = period(budget);
        budgetAmt[budget.CATEGID][budget.SUBCATEGID] = budget.AMOUNT;
    }
}

void Model_Budget::copyBudgetYear(int newYearID, int baseYearID)
{
    for (const Data& data : instance().find(BUDGETYEARID(baseYearID)))
    {
        Data* budgetEntry = instance().clone(&data);
        budgetEntry->BUDGETYEARID = newYearID;
        instance().save(budgetEntry);
    }
}

double Model_Budget::getMonthlyEstimate(const PERIOD_ENUM period, const double amount)
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

double Model_Budget::getYearlyEstimate(const PERIOD_ENUM period, const double amount)
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
