/*******************************************************
 Copyright (C) 2013,2014 James Higley
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2026 George Ef (george.a.ef@gmail.com)

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

#include "base/_defs.h"
#include <wx/intl.h>
#include <wx/regex.h>
#include "table/BudgetTable.h"

#include "BudgetModel.h"
#include "BudgetPeriodModel.h"
#include "CategoryModel.h"
#include "PrefModel.h"

// -- static

TableClauseV<wxString> BudgetModel::WHERE_FREQUENCY(OP op, BudgetFreq freq)
{
    return BudgetCol::WHERE_PERIOD(op, freq.key());
}

// -- constructor

// Initialize the global BudgetModel table.
// Reset the BudgetModel table or create the table if it does not exist.
BudgetModel& BudgetModel::instance(wxSQLite3Database* db)
{
    BudgetModel& ins = Singleton<BudgetModel>::instance();
    ins.reset_cache();
    ins.m_db = db;
    ins.ensure_table();

    return ins;
}

// Return the static instance of BudgetModel table
BudgetModel& BudgetModel::instance()
{
    return Singleton<BudgetModel>::instance();
}

// -- methods

std::vector<int64> BudgetModel::find_child_period_id_a(int64 bp_id)
{
    std::vector<int64> out;

    BudgetPeriodModel& bpm = BudgetPeriodModel::instance();
    const wxString name = bpm.get_id_name_n(bp_id);

    // Only a plain year has months beneath it; "2026-03" has nothing below.
    wxRegEx pattern_year(R"(^([0-9]{4})$)");
    if (!pattern_year.Matches(name))
        return out;

    const wxString prefix = name + "-";
    for (const auto& bp_d : bpm.find_data_a()) {
        if (bp_d.m_id != bp_id && bp_d.m_name.StartsWith(prefix))
            out.push_back(bp_d.m_id);
    }
    return out;
}

void BudgetModel::getBudgetEntry(
    int64 bp_id,
    std::map<int64, BudgetFreq>& freq_mCatId,
    std::map<int64, double>& amount_mCatId,
    std::map<int64, wxString>& notes_mCatId,
    bool include_child_periods
) {
    // initaialize category maps; set amount to zero
    for (int64 cat_id : CategoryModel::instance().find_id_a()) {
        freq_mCatId[cat_id]   = BudgetFreq(BudgetFreq::e_none);
        amount_mCatId[cat_id] = 0.0;
    }

    // A category may now hold more than one entry in the same period, because a
    // period can be split into segments -- groceries budgeted in each half of
    // the month. Assigning per category would keep only the last row and
    // silently lose the rest, so the rows are accumulated.
    //
    // Rows are combined on a yearly basis so that entries with different
    // frequencies still add up correctly; the result is expressed back in the
    // shared frequency when they agree, and yearly when they do not.
    std::map<int64, double> yearly_mCatId;
    std::map<int64, int> freqId_mCatId;   // -1 once a category mixes frequencies
    std::map<int64, double> raw_mCatId;   // used when frequency is None

    std::vector<int64> period_id_a{ bp_id };
    if (include_child_periods) {
        const std::vector<int64> children = find_child_period_id_a(bp_id);
        period_id_a.insert(period_id_a.end(), children.begin(), children.end());
    }

    for (const int64 period_id : period_id_a) {
        for (const auto& budget_d : find_data_a(
            BudgetCol::WHERE_BUDGETYEARID(OP_EQ, period_id)
        )) {
            const int64 cat_id = budget_d.m_category_id;

            yearly_mCatId[cat_id] += budget_d.amount_per_year();
            raw_mCatId[cat_id]    += budget_d.m_amount;

            const auto it = freqId_mCatId.find(cat_id);
            if (it == freqId_mCatId.end())
                freqId_mCatId[cat_id] = budget_d.m_freq.id();
            else if (it->second != budget_d.m_freq.id())
                it->second = -1;

            // Keep the first note that says something, rather than letting a
            // later blank row erase it.
            if (notes_mCatId[cat_id].IsEmpty())
                notes_mCatId[cat_id] = budget_d.m_notes;
        }
    }

    for (const auto& entry : freqId_mCatId) {
        const int64 cat_id = entry.first;
        const int freq_id = entry.second;

        if (freq_id >= 0) {
            const BudgetFreq freq(freq_id);
            freq_mCatId[cat_id] = freq;
            // A frequency of None carries no yearly meaning, so those rows are
            // simply added as they stand.
            amount_mCatId[cat_id] = (freq.times_per_year() > 0)
                ? yearly_mCatId[cat_id] / freq.times_per_year()
                : raw_mCatId[cat_id];
        }
        else {
            // Mixed frequencies have no common shorthand; a yearly total is the
            // only honest way to state the combination.
            freq_mCatId[cat_id]   = BudgetFreq(BudgetFreq::e_yearly);
            amount_mCatId[cat_id] = yearly_mCatId[cat_id];
        }
    }
}

void BudgetModel::getBudgetStats(
    std::map<int64, std::map<int, double>>& budgetStats,
    mmDateRange* date_range,
    bool groupByMonth
) {
    // Set std::map with zeros
    for (int64 cat_id : CategoryModel::instance().find_id_a()) {
        for (int month = 0; month < 12; month++) {
            budgetStats[cat_id][month] = 0.0;
        }
    }

    // Calculations
    const wxDateTime start_date(date_range->start_date());
    std::map<int64, double> monthlyBudgetValue;
    std::map<int64, double> yearlyBudgetValue;
    std::map<int64, double> yearDeduction;
    std::map<std::pair<int, int64>, bool> isBudgeted;
    std::map<int64, int> budgetedMonths;
    const wxString year = wxString::Format("%i", start_date.GetYear());
    int64 bp_id_n = BudgetPeriodModel::instance().get_name_id_n(year);
    for (const Data& budget_d : find_data_a(
        BudgetCol::WHERE_BUDGETYEARID(OP_EQ, bp_id_n)
    )) {
        int64 cat_id = budget_d.m_category_id;
        // Determine the monhly budgeted amounts
        monthlyBudgetValue[cat_id] = budget_d.amount_per_month();
        // Determine the yearly budgeted amounts
        yearlyBudgetValue[cat_id] = budget_d.amount_per_year();
        // Store the yearly budget to use in reporting.
        // Monthly budgets are stored in index 0-11, so use index 12 for year
        budgetStats[cat_id][12] = yearlyBudgetValue[cat_id];
    }
    bool budgetOverride = PrefModel::instance().getBudgetOverride();
    bool budgetDeductMonthly = PrefModel::instance().getBudgetDeductMonthly();
    for (int month = 0; month < 12; month++) {
        const wxString month_name = wxString::Format("%s-%02d", year, month + 1);
        bp_id_n = BudgetPeriodModel::instance().get_name_id_n(month_name);

        //fill with amount from monthly budgets first
        for (const Data& budget_d : find_data_a(
            BudgetCol::WHERE_BUDGETYEARID(OP_EQ, bp_id_n)
        )) {
            int64 cat_id = budget_d.m_category_id;
            std::pair<int, int64> month_categ = std::make_pair(month, cat_id);
            if (!isBudgeted[month_categ]) {
                isBudgeted[month_categ] = true;
                budgetedMonths[cat_id]++;
            }
            budgetStats[cat_id][month] = budget_d.amount_per_month();
            yearDeduction[cat_id] += budgetStats[cat_id][month];
        }
    }
    // Now go month by month and add the yearly budget
    for (int month = 0; month < 12; month++) {
        // If user selected to deduct monthly budgeted amounts 
        if (budgetDeductMonthly)
            for (const auto& categoryBudget : yearlyBudgetValue) {
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
            for (const auto& categoryBudget : monthlyBudgetValue) {
                if (!budgetOverride)
                    // If user doesn't override their budget, add 1/12 of the yearly amount to every period
                    budgetStats[categoryBudget.first][month] += categoryBudget.second;
                else if (!isBudgeted[std::make_pair(month, categoryBudget.first)])
                    // Otherwise fill 1/12 of the yearly amount only in non-budgeted periods
                    budgetStats[categoryBudget.first][month] = categoryBudget.second;
            }
    }
    if (!groupByMonth) {
        std::map<int64, std::map<int,double> > yearlyBudgetStats;
        for (const auto& cat_d : CategoryModel::instance().find_data_a()) {
            yearlyBudgetStats[cat_d.m_id][0] = 0.0;
        }

        for (const auto& cat : budgetStats)
            for(int month = 0; month < 12; month++)
                yearlyBudgetStats[cat.first][0] += budgetStats[cat.first][month];

        budgetStats = yearlyBudgetStats;
    }
}

void BudgetModel::copyBudgetYear(int64 dst_bp_id, int64 src_bp_id)
{
    std::map<int64, double> yearDeduction;
    int budgetedMonths = 0;
    bool optionDeductMonthly = PrefModel::instance().getBudgetDeductMonthly();
    const wxString src_bp_name = BudgetPeriodModel::instance().get_id_name_n(src_bp_id);
    const wxString dst_bp_name = BudgetPeriodModel::instance().get_id_name_n(dst_bp_id);

    // Only deduct monthly amounts if a monthly budget is being created based on a yearly budget
    optionDeductMonthly &= (src_bp_name.length() == 4 && dst_bp_name.length() > 4);

    if (optionDeductMonthly) {
        for (int month = 0; month < 12; ++month) {
            const wxString dst_month_name = wxString::Format("%s-%02d",
                dst_bp_name.SubString(0,3),
                month + 1
            );
            int64 dst_month_id_n = BudgetPeriodModel::instance().get_name_id_n(dst_month_name);
            if (dst_month_id_n <= 0)
                continue;
            BudgetModel::DataA dst_budget_a = find_data_a(
                BudgetCol::WHERE_BUDGETYEARID(OP_EQ, dst_month_id_n)
            );
            if (!dst_budget_a.empty())
                budgetedMonths += 1;
            //calculate deduction
            for (const auto& dst_budget_d : dst_budget_a) {
                yearDeduction[dst_budget_d.m_category_id] += dst_budget_d.amount_per_month();
            }
        }
    }

    for (const Data& src_budget_d : find_data_a(
        BudgetCol::WHERE_BUDGETYEARID(OP_EQ, src_bp_id)
    )) {
        Data new_budget_d = Data();
        new_budget_d.clone_from(src_budget_d);
        new_budget_d.m_period_id = dst_bp_id;
        if (optionDeductMonthly && budgetedMonths > 0) {
            double yearAmount = src_budget_d.amount_per_year();
            new_budget_d.m_freq = BudgetFreq(BudgetFreq::e_monthly);
            // CHECK: budgetedMonths can be 12
            new_budget_d.m_amount    = (yearDeduction[new_budget_d.m_category_id] < yearAmount)
                ? (yearAmount - yearDeduction[new_budget_d.m_category_id]) / (12 - budgetedMonths)
                : 0;
        }
        add_data_n(new_budget_d);
    }
}
