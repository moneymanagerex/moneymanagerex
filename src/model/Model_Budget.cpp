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

#include "Model_Budget.h"
#include "Model_Budgetyear.h"
#include <wx/intl.h>
#include "model/Model_Category.h"
#include "db/DB_Table_Budgettable_V1.h"
#include "option.h"

Model_Budget::Model_Budget()
: Model<DB_Table_BUDGETTABLE_V1>()
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
    , {Model_Budget::BIWEEKLY, wxString(wxTRANSLATE("Fortnightly"))}
    , {Model_Budget::MONTHLY, wxString(wxTRANSLATE("Monthly"))}
    , {Model_Budget::BIMONTHLY, wxString(wxTRANSLATE("Every 2 Months"))}
    , {Model_Budget::QUARTERLY, wxString(wxTRANSLATE("Quarterly"))}
    , {Model_Budget::HALFYEARLY, wxString(wxTRANSLATE("Half-Yearly"))}
    , {Model_Budget::YEARLY, wxString(wxTRANSLATE("Yearly"))}
    , {Model_Budget::DAILY, wxString(wxTRANSLATE("Daily"))}
};

wxArrayString Model_Budget::all_period()
{
    wxArrayString period;
    for (const auto& item : PERIOD_ENUM_CHOICES) period.Add(wxGetTranslation(item.second));
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

DB_Table_BUDGETTABLE_V1::PERIOD Model_Budget::PERIOD(PERIOD_ENUM period, OP op)
{
    return DB_Table_BUDGETTABLE_V1::PERIOD(all_period()[period], op);
}

void Model_Budget::getBudgetEntry(int budgetYearID
    , std::map<int, std::map<int, PERIOD_ENUM> > &budgetPeriod
    , std::map<int, std::map<int, double> > &budgetAmt
    , std::map<int, std::map<int, wxString> > &budgetNotes)
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
        budgetNotes[budget.CATEGID][budget.SUBCATEGID] = budget.NOTES;
    }
}

void Model_Budget::getBudgetStats(
      std::map<int, std::map<int, std::map<int, double> > > &budgetStats
    , mmDateRange* date_range
    , bool groupByMonth)
{
    //Initialization
    //Set std::map with zeros
    double value = 0;
    const wxDateTime start_date(date_range->start_date());

    for (const auto& category : Model_Category::all_categories())
        for (int month = 0; month < 12; month++)
            budgetStats[category.second.first][category.second.second][month] = value;

    //Calculations
    std::map<int, std::map<int, double> > yearBudgetValue;
    const wxString year = wxString::Format("%i", start_date.GetYear());
    int budgetYearID = Model_Budgetyear::instance().Get(year);
    for (const auto& budget : instance().find(BUDGETYEARID(budgetYearID)))
        yearBudgetValue[budget.CATEGID][budget.SUBCATEGID] = getEstimate(true, period(budget), budget.AMOUNT);

    for (int month = 0; month < 12; month++)
    {
        for (const auto& cat : yearBudgetValue)
            for (const auto& id : cat.second)
                budgetStats[cat.first][id.first][month] += id.second;

        const wxString budgetYearMonth = wxString::Format("%s-%02d", year, month + 1);
        budgetYearID = Model_Budgetyear::instance().Get(budgetYearMonth);
        for (const auto& budget : instance().find(BUDGETYEARID(budgetYearID)))
            if (Option::instance().BudgetOverride())
                budgetStats[budget.CATEGID][budget.SUBCATEGID][month] = getEstimate(true, period(budget), budget.AMOUNT);
            else
                budgetStats[budget.CATEGID][budget.SUBCATEGID][month] += getEstimate(true, period(budget), budget.AMOUNT);
    }
    if (!groupByMonth)
    {
        std::map<int, std::map<int, std::map<int, double> > > yearlyBudgetStats;
        for (const auto& category : Model_Category::all_categories())
            yearlyBudgetStats[category.second.first][category.second.second][0] = 0.0;

        for (const auto& cat : budgetStats)
            for (const auto& subcat : cat.second)
                for (const auto& month : subcat.second)
                    yearlyBudgetStats[cat.first][subcat.first][0] += month.second;

        budgetStats = yearlyBudgetStats;
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

double Model_Budget::getEstimate(bool is_monthly, const PERIOD_ENUM period, const double amount)
{
    int p[MAX] = { 0, 52, 26, 12, 6, 4, 2, 1, 365 };
    double estimated = amount * p[period];
    if (is_monthly) estimated = estimated / 12;
    return estimated;
}
