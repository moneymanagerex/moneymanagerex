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

const std::vector<std::pair<Model_Budget::PERIOD_ID, wxString> > Model_Budget::PERIOD_CHOICES =
{
    { Model_Budget::PERIOD_ID_NONE,       wxString(wxTRANSLATE("None")) },
    { Model_Budget::PERIOD_ID_WEEKLY,     wxString(wxTRANSLATE("Weekly")) },
    { Model_Budget::PERIOD_ID_BIWEEKLY,   wxString(wxTRANSLATE("Fortnightly")) },
    { Model_Budget::PERIOD_ID_MONTHLY,    wxString(wxTRANSLATE("Monthly")) },
    { Model_Budget::PERIOD_ID_BIMONTHLY,  wxString(wxTRANSLATE("Every 2 Months")) },
    { Model_Budget::PERIOD_ID_QUARTERLY,  wxString(wxTRANSLATE("Quarterly")) },
    { Model_Budget::PERIOD_ID_HALFYEARLY, wxString(wxTRANSLATE("Half-Yearly")) },
    { Model_Budget::PERIOD_ID_YEARLY,     wxString(wxTRANSLATE("Yearly")) },
    { Model_Budget::PERIOD_ID_DAILY,      wxString(wxTRANSLATE("Daily")) }
};

wxArrayString Model_Budget::PERIOD_STR = period_str_all();

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

wxArrayString Model_Budget::period_str_all()
{
    wxArrayString period;
    int i = 0;
    for (const auto& item : PERIOD_CHOICES)
    {
        wxASSERT_MSG(item.first == i++, "Wrong order in Model_Budget::PERIOD_CHOICES");
        period.Add(item.second);
    }
    return period;
}

wxArrayString Model_Budget::period_loc_all()
{
    wxArrayString period;
    for (const auto& item : PERIOD_CHOICES)
    {
        period.Add(wxGetTranslation(item.second));
    }
    return period;
}

Model_Budget::PERIOD_ID Model_Budget::period_id(const Data* r)
{
    for (const auto &entry : PERIOD_CHOICES)
    {
        if (r->PERIOD.CmpNoCase(entry.second) == 0) return entry.first;
    }
    return PERIOD_ID_NONE;
}

Model_Budget::PERIOD_ID Model_Budget::period_id(const Data& r)
{
    return period_id(&r);
}

DB_Table_BUDGETTABLE_V1::PERIOD Model_Budget::PERIOD(PERIOD_ID period, OP op)
{
    return DB_Table_BUDGETTABLE_V1::PERIOD(PERIOD_STR[period], op);
}

void Model_Budget::getBudgetEntry(int64 budgetYearID
    , std::map<int64, PERIOD_ID> &budgetPeriod
    , std::map<int64, double> &budgetAmt
    , std::map<int64, wxString> &budgetNotes)
{
    //Set std::map with zerros
    double value = 0;
    for (const auto& category : Model_Category::instance().all())
    {
        budgetPeriod[category.CATEGID] = PERIOD_ID_NONE;
        budgetAmt[category.CATEGID] = value;

    }

    for (const auto& budget : instance().find(BUDGETYEARID(budgetYearID)))
    {
        budgetPeriod[budget.CATEGID] = period_id(budget);
        budgetAmt[budget.CATEGID] = budget.AMOUNT;
        budgetNotes[budget.CATEGID] = budget.NOTES;
    }
}

void Model_Budget::getBudgetStats(
    std::map<int64, std::map<int, double> > &budgetStats
    , mmDateRange* date_range
    , bool groupByMonth)
{
    //Initialization
    //Set std::map with zeros
    double value = 0;
    const wxDateTime start_date(date_range->start_date());

    for (const auto& category : Model_Category::instance().all())
    {
        for (int month = 0; month < 12; month++) {
            budgetStats[category.CATEGID][month] = value;
        }
    }

    //Calculations
    std::map<int64, double> monthlyBudgetValue;
    std::map<int64, double> yearlyBudgetValue;
    std::map<int64, double> yearDeduction;
    std::map<std::pair<int, int64>, bool> isBudgeted;
    std::map<int64, int> budgetedMonths;
    const wxString year = wxString::Format("%i", start_date.GetYear());
    int64 budgetYearID = Model_Budgetyear::instance().Get(year);
    for (const auto& budget : instance().find(BUDGETYEARID(budgetYearID)))
    {
        // Determine the monhly budgeted amounts
        monthlyBudgetValue[budget.CATEGID] = getEstimate(true, period_id(budget), budget.AMOUNT);
        // Determine the yearly budgeted amounts
        yearlyBudgetValue[budget.CATEGID] = getEstimate(false, period_id(budget), budget.AMOUNT);
        // Store the yearly budget to use in reporting. Monthly budgets are stored in index 0-11, so use index 12 for year
        budgetStats[budget.CATEGID][12] = yearlyBudgetValue[budget.CATEGID];
    }
    bool budgetOverride = Option::instance().BudgetOverride();
    bool budgetDeductMonthly = Option::instance().BudgetDeductMonthly();
    for (int month = 0; month < 12; month++)
    {
        const wxString budgetYearMonth = wxString::Format("%s-%02d", year, month + 1);
        budgetYearID = Model_Budgetyear::instance().Get(budgetYearMonth);

        //fill with amount from monthly budgets first
        for (const auto& budget : instance().find(BUDGETYEARID(budgetYearID)))
        {
            std::pair<int, int64> month_categ = std::make_pair(month, budget.CATEGID);
            if (!isBudgeted[month_categ])
            {
                isBudgeted[month_categ] = true;
                budgetedMonths[budget.CATEGID]++;
            }
            budgetStats[budget.CATEGID][month] = getEstimate(true, period_id(budget), budget.AMOUNT);
            yearDeduction[budget.CATEGID] += budgetStats[budget.CATEGID][month];
        }
    }
    // Now go month by month and add the yearly budget
    for (int month = 0; month < 12; month++)
    {
        // If user selected to deduct monthly budgeted amounts 
        if (budgetDeductMonthly)
            for (const auto& categoryBudget : yearlyBudgetValue)
            {
                if (yearDeduction[categoryBudget.first] / categoryBudget.second >= 1) continue;
                //Deduct the monthly total from the yearly budget
                double adjusted_amount = categoryBudget.second - yearDeduction[categoryBudget.first];
                if (!budgetOverride)
                    // If user doesn't override the budget, add 1/12 of the adjusted amount to every period
                    budgetStats[categoryBudget.first][month] += adjusted_amount / 12;
                else if (!isBudgeted[std::make_pair(month, categoryBudget.first)])
                    // Otherwise if n months have a defined budget, add 1/(12-n) of the adjusted amount only to the (12-n) non-budgeted periods
                    budgetStats[categoryBudget.first][month] = adjusted_amount / (12 - budgetedMonths[categoryBudget.first]);
            }
        else
            // If the user is not deducting the monthly budget from the yearly budget
            for (const auto& categoryBudget : monthlyBudgetValue)
            {
                if (!budgetOverride)
                    // If user doesn't override their budget, add 1/12 of the yearly amount to every period
                    budgetStats[categoryBudget.first][month] += categoryBudget.second;
                else if (!isBudgeted[std::make_pair(month, categoryBudget.first)])
                    // Otherwise fill 1/12 of the yearly amount only in non-budgeted periods
                    budgetStats[categoryBudget.first][month] = categoryBudget.second;
            }
    }
    if (!groupByMonth)
    {
        std::map<int64, std::map<int,double> > yearlyBudgetStats;
        for (const auto& category : Model_Category::instance().all()) {
            yearlyBudgetStats[category.CATEGID][0] = 0.0;
        }

        for (const auto& cat : budgetStats)
            for(int month = 0; month < 12; month++)
                yearlyBudgetStats[cat.first][0] += budgetStats[cat.first][month];

        budgetStats = yearlyBudgetStats;
    }
}

void Model_Budget::copyBudgetYear(int64 newYearID, int64 baseYearID)
{
    std::map<int64, double> yearDeduction;
    int budgetedMonths = 0;
    bool optionDeductMonthly = Option::instance().BudgetDeductMonthly();
    const wxString baseBudgetYearName = Model_Budgetyear::instance().get(baseYearID)->BUDGETYEARNAME;
    const wxString newBudgetYearName = Model_Budgetyear::instance().get(newYearID)->BUDGETYEARNAME;

    // Only deduct monthly amounts if a monthly budget is being created based on a yearly budget
    optionDeductMonthly &= (baseBudgetYearName.length() == 4 && newBudgetYearName.length() > 4);

    if (optionDeductMonthly) {
        for (int month = 0; month < 12; month++)
        {
            const wxString budgetYearMonth = wxString::Format("%s-%02d", newBudgetYearName.SubString(0,3), month + 1);
            int64 budgetYearID = Model_Budgetyear::instance().Get(budgetYearMonth);
            Model_Budget::Data_Set monthlyBudgetData = instance().find(BUDGETYEARID(budgetYearID));
            if (!monthlyBudgetData.empty()) budgetedMonths++;
            //calculate deduction
            for (const auto& budget : monthlyBudgetData)
            {
                yearDeduction[budget.CATEGID] += getEstimate(true, period_id(budget), budget.AMOUNT);
            }
        }
    }

    for (const Data& data : instance().find(BUDGETYEARID(baseYearID)))
    {
        Data* budgetEntry = instance().clone(&data);
        budgetEntry->BUDGETYEARID = newYearID;
        double yearAmount = getEstimate(false, period_id(data), data.AMOUNT);
        if (optionDeductMonthly && budgetedMonths > 0)
        {
            budgetEntry->PERIOD = PERIOD_CHOICES[PERIOD_ID_MONTHLY].second;
            if (yearDeduction[budgetEntry->CATEGID] / yearAmount < 1)
                budgetEntry->AMOUNT = (yearAmount - yearDeduction[budgetEntry->CATEGID]) / (12 - budgetedMonths);
            else budgetEntry->AMOUNT = 0;
        }
        instance().save(budgetEntry);
    }
}

double Model_Budget::getEstimate(bool is_monthly, const PERIOD_ID period, const double amount)
{
    int p[PERIOD_ID_MAX] = { 0, 52, 26, 12, 6, 4, 2, 1, 365 };
    double estimated = amount * p[period];
    if (is_monthly) estimated = estimated / 12;
    return estimated;
}
