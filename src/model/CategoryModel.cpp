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

bool CategoryModel::is_used(int64 cat_id)
{
    if (cat_id <= 0)
        return false;

    const auto& trx_a = TrxModel::instance().find(
        TrxCol::CATEGID(cat_id)
    );
    // FIXME: do not exclude deleted transactions
    for (const auto& trx_d : trx_a)
        if (!trx_d.is_deleted())
            return true;

    const auto& tp_a = TrxSplitModel::instance().find(
        TrxSplitCol::CATEGID(cat_id)
    );
    for (const auto& tp_d : tp_a) {
        const TrxData* trx_n = TrxModel::instance().get_id_data_n(tp_d.m_trx_id);
        if (!trx_n->is_deleted())
            return true;
    }

    const auto& sched_a = SchedModel::instance().find(
        SchedCol::CATEGID(cat_id)
    );
    if (!sched_a.empty())
        return true;

    const auto& sched_split_a = SchedSplitModel::instance().find(
        SchedSplitCol::CATEGID(cat_id)
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

bool CategoryModel::get_id_active(int64 cat_id)
{
    // root category (id -1) is always active
    if (cat_id <= 0)
        return true;
    const Data* cat_n = get_id_data_n(cat_id);
    return (cat_n && cat_n->m_active);
}

const wxString CategoryModel::get_data_fullname(
    const CategoryData* cat_n,
    wxString delimiter
) {
    if (!cat_n)
        return "";

    // note: CATEG_DELIMITER may change during program execution
    if (delimiter.empty()) {
        delimiter = InfoModel::instance().getString("CATEG_DELIMITER", ":");
    }

    wxString fullname = cat_n->m_name;
    while (cat_n->m_parent_id_n > 0) {
        cat_n = get_id_data_n(cat_n->m_parent_id_n);
        fullname = cat_n->m_name + delimiter + fullname;
    }

    return fullname;
}

const wxString CategoryModel::get_id_fullname(int64 cat_id, wxString delimiter)
{
    return get_data_fullname(get_id_data_n(cat_id), delimiter);
}

double CategoryModel::get_id_income(int64 cat_id)
{
    double sum = 0.0;
    auto trxId_tpA_m = TrxSplitModel::instance().find_all_mTrxId();
    for (const auto& trx_d : TrxModel::instance().find(
        TrxCol::CATEGID(cat_id),
        TrxModel::IS_VOID(false),
        TrxModel::IS_DELETED(false)
    )) {
        switch (trx_d.m_type.id())
        {
        case TrxType::e_withdrawal:
            sum -= trx_d.m_amount;
            break;
        case TrxType::e_deposit:
            sum += trx_d.m_amount;
        case TrxType::e_transfer:
        default:
            break;
        }

        for (const auto& tp_d : trxId_tpA_m[trx_d.m_id]) {
            switch (trx_d.m_type.id())
            {
            case TrxType::e_withdrawal:
                sum -= tp_d.m_amount;
                break;
            case TrxType::e_deposit:
                sum += tp_d.m_amount;
            case TrxType::e_transfer:
            default:
                break;
            }
        }
    }

    return sum;
}

const CategoryData* CategoryModel::get_key_data_n(const wxString& name, const int64 parentid)
{
    const Data* cat_n = search_cache_n(
        CategoryCol::CATEGNAME(name),
        CategoryCol::PARENTID(parentid)
    );
    if (cat_n)
        return cat_n;

    const DataA cat_a = find(
        CategoryCol::CATEGNAME(name),
        CategoryCol::PARENTID(parentid)
    );
    if (!cat_a.empty())
        cat_n = get_id_data_n(cat_a[0].m_id);
    return cat_n;
}

// FIXME: This method is ill-defined (there can be multiple matches)
const CategoryData* CategoryModel::get_name2_data_n(
    const wxString& name,
    const wxString& parent_name
) {
    for (const auto& cat_d : find(CategoryCol::CATEGNAME(name))) {
        if (cat_d.m_parent_id_n <= 0)
            continue;
        const Data* parent_n = get_id_data_n(cat_d.m_parent_id_n);
        if (parent_n->m_name.Lower() == parent_name.Lower()) {
            return get_id_data_n(cat_d.m_id);
        }
    }
    return nullptr;
}

CategoryModel::DataA CategoryModel::find_data_sub_a(const Data& cat_d)
{
    return find(CategoryCol::PARENTID(cat_d.m_id));
}

CategoryModel::DataA CategoryModel::find_data_subtree_a(const Data& cat_d)
{
    DataA tree_a;
    DataA sub_a = find(CategoryCol::PARENTID(cat_d.m_id));
    std::stable_sort(sub_a.begin(), sub_a.end(),
        CategoryData::SorterByCATEGNAME()
    );
    for (const auto& sub_d : sub_a) {
        tree_a.push_back(sub_d);
        DataA subtree_a = find_data_subtree_a(sub_d);
        for (const auto& subtree_d : subtree_a) {
            tree_a.push_back(subtree_d);
        }
    }
    return tree_a;
}

const std::map<wxString, int64> CategoryModel::find_all_id_mFullname(bool only_active)
{
    std::map<wxString, int64> fullname_id_m;
    for (const auto& cat_d : find_all(Col::COL_ID_CATEGID)) {
        if (only_active && !cat_d.m_active)
            continue;
        wxString fullname = get_id_fullname(cat_d.m_id);
        fullname_id_m[fullname] = cat_d.m_id;
    }
    return fullname_id_m;
}

const wxArrayString CategoryModel::find_pattern_name_a(const wxString& cat_pattern)
{
    wxArrayString name_a;
    for (auto& cat_d : find_all()) {
        if (cat_d.m_name.Lower().Matches(cat_pattern.Lower().Append("*")))
            name_a.push_back(cat_d.m_name);
    }
    return name_a;
}

void CategoryModel::getCategoryStats(
    std::map<int64, std::map<int, double>>& categoryStats,
    wxSharedPtr<wxArrayString> accountArray,
    mmDateRange* date_range,
    bool WXUNUSED(ignoreFuture), //TODO: deprecated
    bool group_by_month,
    std::map<int64, double>* budgetAmt,
    [[maybe_unused]] bool fin_months
) {
    mmDate startDate = mmDate(date_range->start_date());
    mmDate endDate = mmDate(date_range->end_date());

    // Initialization
    // Set std::map with zerros
    const auto& allcategories = find_all();
    double value = 0;
    int columns = group_by_month ? 12 : 1;

    std::vector<std::pair<mmDate, int>> date_month_a;
    for (int m = 0; m < columns; m++) {
        mmDate date = startDate;
        date.addDateSpan(wxDateSpan::Months(m));
        date_month_a.emplace_back(date, m);
    }
    std::reverse(date_month_a.begin(), date_month_a.end());

    for (const auto& category : allcategories) {
        for (int m = 0; m < columns; m++) {
            int month = group_by_month ? m : 0;
            categoryStats[category.m_id][month] = value;
        }
    }

    // Calculations
    auto trxId_tpA_m = TrxSplitModel::instance().find_all_mTrxId();
    for (const auto& trx_d : TrxModel::instance().find(
        TrxModel::DATE(OP_GE, startDate),
        TrxModel::DATE(OP_LE, endDate),
        TrxModel::IS_VOID(false)
    )) {
        if (trx_d.is_deleted())
            continue;

        if (accountArray) {
            const AccountData* account_n = AccountModel::instance().get_id_data_n(
                trx_d.m_account_id
            );
            if (accountArray->Index(account_n->m_name) == wxNOT_FOUND)
                continue;
        }

        const double convRate = CurrencyHistoryModel::instance().get_id_date_rate(
            AccountModel::instance().get_id_data_n(trx_d.m_account_id)->m_currency_id,
            trx_d.m_date()
        );

        mmDate trx_date = trx_d.m_date();
        int month = 0;
        if (group_by_month) {
            auto it = std::find_if(date_month_a.begin(), date_month_a.end(),
                [trx_date](std::pair<mmDate, int> date_month) {
                    return trx_date >= date_month.first;
                }
            );
            month = it->second;
        }

        int64 categID = trx_d.m_category_id_n;

        if (trxId_tpA_m[trx_d.m_id].empty()) {
            if (!trx_d.is_transfer()) {
                // Do not include asset or stock transfers in income expense calculations.
                if (TrxModel::is_foreignAsTransfer(trx_d))
                    continue;
                categoryStats[categID][month] += trx_d.account_flow(trx_d.m_account_id) * convRate;
            }
            else if (budgetAmt != 0) {
                double amt = trx_d.m_amount * convRate;
                if ((*budgetAmt)[categID] < 0)
                    categoryStats[categID][month] -= amt;
                else
                    categoryStats[categID][month] += amt;
            }
        }
        else {
            for (const auto& tp_d : trxId_tpA_m[trx_d.m_id]) {
                categoryStats[tp_d.m_category_id][month] +=
                    (trx_d.is_withdrawal() ? -tp_d.m_amount : tp_d.m_amount) *
                    convRate;
            }
        }
    }
}
