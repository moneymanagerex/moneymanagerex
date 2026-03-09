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

#include "base/defs.h"
#include <tuple>
#include "util/mmDateRange.h"

#include "AccountModel.h"
#include "CategoryModel.h"
#include "CurrencyHistoryModel.h"
#include "PrefModel.h"
#include "SchedModel.h"
#include "TrxModel.h"

CategoryModel::CategoryModel() :
    TableFactory<CategoryTable, CategoryData>()
{
}

CategoryModel::~CategoryModel()
{
}

// Initialize the global CategoryModel table.
// Reset the CategoryModel table or create the table if it does not exist.
CategoryModel& CategoryModel::instance(wxSQLite3Database* db)
{
    CategoryModel& ins = Singleton<CategoryModel>::instance();
    ins.reset_cache();
    ins.m_db = db;
    ins.ensure_table();
    ins.preload_cache();

    return ins;
}

// Return the static instance of CategoryModel table
CategoryModel& CategoryModel::instance()
{
    return Singleton<CategoryModel>::instance();
}

const CategoryData* CategoryModel::get_key_data_n(const wxString& name, const int64 parentid)
{
    const Data* cat_n = search_cache_n(
        CategoryCol::CATEGNAME(name),
        CategoryCol::PARENTID(parentid)
    );
    if (cat_n)
        return cat_n;

    DataA cat_a = find(
        CategoryCol::CATEGNAME(name),
        CategoryCol::PARENTID(parentid)
    );
    if (!cat_a.empty())
        cat_n = get_id_data_n(cat_a[0].m_id);
    return cat_n;
}

CategoryModel::DataA CategoryModel::find_data_sub_a(const Data& cat_d)
{
    return find(CategoryCol::PARENTID(cat_d.m_id));
}

const wxArrayString CategoryModel::FilterCategory(const wxString& cat_pattern)
{
    wxArrayString categories;
    for (auto& cat_d : CategoryModel::instance().find_all()) {
        if (cat_d.m_name.Lower().Matches(cat_pattern.Lower().Append("*")))
            categories.push_back(cat_d.m_name);
    }
    return categories;
}

// FIXME: This method is ill-defined (there can be multiple matches)
const CategoryData* CategoryModel::get_name(const wxString& name, const wxString& parent_name)
{
    const Data* cat_n = nullptr;
    DataA cat_a = find(CategoryCol::CATEGNAME(name));
    for (const auto& cat_d : cat_a) {
        if (cat_d.m_parent_id_n != -1) {
            const Data* parent_n = get_id_data_n(cat_d.m_parent_id_n);
            if (parent_n->m_name.Lower() == parent_name.Lower()) {
                cat_n = get_id_data_n(cat_d.m_id);
                break;
            }
        }
    }

    return cat_n;
}

const std::map<wxString, int64> CategoryModel::all_categories(bool excludeHidden)
{
    std::map<wxString, int64> full_categs;
    for (const auto& cat_d : instance().find_all(Col::COL_ID_CATEGID)) {
        if (excludeHidden && !cat_d.m_active)
            continue;

        full_categs[full_name(cat_d.m_id)] = cat_d.m_id;
    }
    return full_categs;
}

CategoryModel::DataA CategoryModel::find_data_subtree_a(const Data& cat_d)
{
    DataA tree;
    DataA sub_a = find(CategoryCol::PARENTID(cat_d.m_id));
    std::stable_sort(sub_a.begin(), sub_a.end(), CategoryData::SorterByCATEGNAME());
    for (const auto& sub_d : sub_a) {
        tree.push_back(sub_d);
        DataA subtree_a = find_data_subtree_a(sub_d);
        for (const auto& subtree_d : subtree_a) {
            tree.push_back(subtree_d);
        }
    }
    return tree;
}

const wxString CategoryModel::full_name(const Data* cat_n)
{
    static wxString delimiter;
    if (delimiter.empty()) {
        delimiter = InfoModel::instance().getString("CATEG_DELIMITER", ":");
    }

    if (!cat_n)
        return "";

    if (cat_n->m_parent_id_n == -1)
        return cat_n->m_name;
    else {
        wxString full_name = cat_n->m_name;
        const Data* parent_n = get_id_data_n(cat_n->m_parent_id_n);
        while (parent_n) {
            full_name = full_name.Prepend(delimiter).Prepend(parent_n->m_name);
            parent_n = get_id_data_n(parent_n->m_parent_id_n);
        }
        return full_name;
    }
}

const wxString CategoryModel::full_name(int64 cat_id)
{
    const Data* cat_n = get_id_data_n(cat_id);
    return full_name(cat_n);
}

const wxString CategoryModel::full_name(int64 cat_id, wxString delimiter)
{
    const Data* cat_n = instance().get_id_data_n(cat_id);
    if (!cat_n)
        return "";
    if (cat_n->m_parent_id_n == -1)
        return cat_n->m_name;
    else {
        wxString name = cat_n->m_name;
        const Data* parent_n = instance().get_id_data_n(cat_n->m_parent_id_n);
        while (parent_n) {
            name = name.Prepend(delimiter).Prepend(parent_n->m_name);
            parent_n = instance().get_id_data_n(parent_n->m_parent_id_n);
        }
        return name;
    }
}

// Check if Category should be made available for use.
// Hiding a category hides all sub-categories
bool CategoryModel::is_hidden(int64 cat_id)
{
    const auto cat_n = CategoryModel::instance().get_id_data_n(cat_id);
    return (cat_n && !cat_n->m_active);
}

bool CategoryModel::is_used(int64 cat_id)
{
    if (cat_id <= 0)
        return false;

    const auto& trx_a = TrxModel::instance().find(
        TrxCol::CATEGID(cat_id)
    );
    // FIXME: do not exclude deleted transactions
    for (const auto& trx_d : trx_a)
        if (trx_d.DELETEDTIME.IsEmpty())
            return true;

    const auto& split_a = TrxSplitModel::instance().find(
        TrxCol::CATEGID(cat_id)
    );
    for (const auto& split_d : split_a)
        if (TrxModel::instance().get_id_data_n(split_d.m_trx_id)->DELETEDTIME.IsEmpty())
            return true;

    const auto& sched_a = SchedModel::instance().find(
        SchedCol::CATEGID(cat_id)
    );
    if (!sched_a.empty())
        return true;

    const auto& sched_split_a = SchedSplitModel::instance().find(
        SchedCol::CATEGID(cat_id)
    );
    if (!sched_split_a.empty())
        return true;

    DataA child_a = find(CategoryCol::PARENTID(cat_id));
    if (!child_a.empty()){
        bool used = false;
        for(const auto& child_d : child_a){
            used = used || is_used(child_d.m_id);
        }
        return used;
    }

    // FIXME: check if cat_id is used in PayeeData
    // FIXME: check if cat_id is used in BudgetData

    return false;
}

bool CategoryModel::has_income(int64 cat_id)
{
    double sum = 0.0;
    auto splits = TrxSplitModel::instance().get_all_id();
    // FIXME: ignore Void transactions
    for (const auto& trx_d : TrxModel::instance().find(
        TrxCol::CATEGID(cat_id),
        TrxCol::DELETEDTIME(wxEmptyString)
    )) {
        switch (TrxModel::type_id(trx_d))
        {
        case TrxModel::TYPE_ID_WITHDRAWAL:
            sum -= trx_d.m_amount;
            break;
        case TrxModel::TYPE_ID_DEPOSIT:
            sum += trx_d.m_amount;
        case TrxModel::TYPE_ID_TRANSFER:
        default:
            break;
        }

        for (const auto& tp_d : splits[trx_d.m_id]) {
            switch (TrxModel::type_id(trx_d))
            {
            case TrxModel::TYPE_ID_WITHDRAWAL:
                sum -= tp_d.m_amount;
                break;
            case TrxModel::TYPE_ID_DEPOSIT:
                sum += tp_d.m_amount;
            case TrxModel::TYPE_ID_TRANSFER:
            default:
                break;
            }
        }
    }

    return sum > 0;
}

void CategoryModel::getCategoryStats(
    std::map<int64, std::map<int, double>>& categoryStats
    , wxSharedPtr<wxArrayString> accountArray
    , mmDateRange* date_range
    , bool WXUNUSED(ignoreFuture) //TODO: deprecated
    , bool group_by_month
    , std::map<int64, double> *budgetAmt
    , [[maybe_unused]] bool fin_months)
{
    //Initialization
    //Set std::map with zerros
    const auto& allcategories = instance().find_all();
    double value = 0;
    int columns = group_by_month ? 12 : 1;
    const wxDateTime start_date(date_range->start_date());

    std::vector<std::pair<wxDateTime, int>> monthMap;
    for (int m = 0; m < columns; m++)
    {
        const wxDateTime d = start_date.Add(wxDateSpan::Months(m));
        monthMap.emplace_back(d, m);
    }
    std::reverse(monthMap.begin(), monthMap.end());

    for (const auto& category : allcategories)
    {
        for (int m = 0; m < columns; m++)
        {
            int month = group_by_month ? m : 0;
            categoryStats[category.m_id][month] = value;
        }
    }
    //Calculations
    auto id_tp_m = TrxSplitModel::instance().get_all_id();
    for (const auto& transaction : TrxModel::instance().find(
        TrxModel::STATUS(OP_NE, TrxModel::STATUS_ID_VOID),
        TrxModel::TRANSDATE(OP_GE, date_range->start_date()),
        TrxCol::TRANSDATE(OP_LE, date_range->end_date().FormatISOCombined())
    )) {
        if (!transaction.DELETEDTIME.IsEmpty()) continue;

        if (accountArray) {
            const auto account = AccountModel::instance().get_id_data_n(transaction.m_account_id);
            if (wxNOT_FOUND == accountArray->Index(account->m_name)) {
                continue;
            }
        }

        const double convRate = CurrencyHistoryModel::getDayRate(
            AccountModel::instance().get_id_data_n(transaction.m_account_id)->m_currency_id,
            transaction.TRANSDATE
        );
        wxDateTime d = TrxModel::getTransDateTime(transaction);

        int month = 0;
        if (group_by_month) {
            auto it = std::find_if(monthMap.begin(), monthMap.end()
                , [d](std::pair<wxDateTime, int> date){return d >= date.first;});
            month = it->second;
        }

        int64 categID = transaction.m_category_id_n;

        if (id_tp_m[transaction.id()].empty()) {
            if (TrxModel::type_id(transaction) != TrxModel::TYPE_ID_TRANSFER) {
                // Do not include asset or stock transfers in income expense calculations.
                if (TrxModel::is_foreignAsTransfer(transaction))
                    continue;
                categoryStats[categID][month] += TrxModel::account_flow(transaction, transaction.m_account_id) * convRate;
            }
            else if (budgetAmt != 0) {
                double amt = transaction.m_amount * convRate;
                if ((*budgetAmt)[categID] < 0)
                    categoryStats[categID][month] -= amt;
                else
                    categoryStats[categID][month] += amt;
            }
        }
        else {
            for (const auto& tp_d : id_tp_m[transaction.id()]) {
                categoryStats[tp_d.m_category_id][month] +=
                    tp_d.m_amount * convRate *
                    ((TrxModel::type_id(transaction) == TrxModel::TYPE_ID_WITHDRAWAL) ? -1 : 1);
            }
        }
    }
}
