/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
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

#include "CategoryModel.h"

#include <tuple>
#include "base/_defs.h"
#include "util/mmDateRange.h"

#include "PrefModel.h"
#include "CurrencyHistoryModel.h"
#include "AccountModel.h"
#include "PayeeModel.h"
#include "TrxModel.h"
#include "SchedModel.h"
#include "BudgetModel.h"

// -- constructor

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

// -- override

bool CategoryModel::find_id_isUsed(int64 cat_id, bool ignore_deleted)
{
    if (cat_id <= 0)
        return false;

    bool is_used = false;

    // TODO: move this check out of find_id_isUsed()
    // Do not check recursively if sub-categories are used.
    // If cat_id has sub-categories, let the user delete them first manually.
    is_used = is_used || CategoryModel::instance().find_count(
        CategoryCol::WHERE_PARENTID(OP_EQ, cat_id)
    ) > 0;

    is_used = is_used || PayeeModel::instance().find_count(
        PayeeCol::WHERE_CATEGID(OP_EQ, cat_id)
    ) > 0;

    is_used = is_used || TrxModel::instance().find_count(
        TrxCol::WHERE_CATEGID(OP_EQ, cat_id),
        TrxModel::WHERE_IGNORE_DELETED(ignore_deleted)
    ) > 0;

    if (!ignore_deleted) {
        // Fast path: find cat_id with a single query.
        is_used = is_used || TrxSplitModel::instance().find_count(
            TrxSplitCol::WHERE_CATEGID(OP_EQ, cat_id)
        ) > 0;
    }
    else if (!is_used) {
        // Slow path: fetch each transaction split, until a match is found.
        for (const TrxSplitData& tp_d : TrxSplitModel::instance().find_data_a(
            TrxSplitCol::WHERE_CATEGID(OP_EQ, cat_id)
        )) {
            is_used = is_used || TrxModel::instance().find_id_count(tp_d.m_trx_id, true) > 0;
            if (is_used)
                break;
        }
    }

    is_used = is_used || SchedModel::instance().find_count(
        SchedCol::WHERE_CATEGID(OP_EQ, cat_id)
    ) > 0;

    is_used = is_used || SchedSplitModel::instance().find_count(
        SchedSplitCol::WHERE_CATEGID(OP_EQ, cat_id)
    ) > 0;

    is_used = is_used || BudgetModel::instance().find_count(
        BudgetCol::WHERE_CATEGID(OP_EQ, cat_id)
    ) > 0;

    return is_used;
}

// -- methods

bool CategoryModel::get_id_active(int64 cat_id)
{
    // root category (id -1) is always active
    if (cat_id <= 0)
        return true;

    const Data* cat_n = get_idN_data_n(cat_id);
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
        cat_n = get_idN_data_n(cat_n->m_parent_id_n);
        // #8276: temporary fix for corrupted CATEGORY_V1 (parent does not exist)
        if (!cat_n) 
        {
            fullname = _t("Error") + delimiter + fullname;
            break;
        } else
            fullname = cat_n->m_name + delimiter + fullname;
    }

    return fullname;
}

const wxString CategoryModel::get_id_fullname(int64 cat_id, wxString delimiter)
{
    return get_data_fullname(get_idN_data_n(cat_id), delimiter);
}

double CategoryModel::get_id_income(int64 cat_id)
{
    double sum = 0.0;
    auto trxId_tpA_m = TrxSplitModel::instance().find_all_mTrxId();
    for (const auto& trx_d : TrxModel::instance().find_data_a(
        TrxCol::WHERE_CATEGID(OP_EQ, cat_id),
        TrxModel::WHERE_IS_VOID(false),
        TrxModel::WHERE_IS_DELETED(false)
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

    for (int64 cat_id : find_id_a(
        CategoryCol::WHERE_CATEGNAME(OP_EQ, name),
        CategoryCol::WHERE_PARENTID(OP_EQ, parentid)
    )) {
        cat_n = get_idN_data_n(cat_id);
    }

    return cat_n;
}

// FIXME: This method is ill-defined (there can be multiple matches)
const CategoryData* CategoryModel::get_name2_data_n(
    const wxString& name,
    const wxString& parent_name
) {
    for (const auto& cat_d : find_data_a(
        CategoryCol::WHERE_CATEGNAME(OP_EQ, name)
    )) {
        if (cat_d.m_parent_id_n <= 0)
            continue;
        const Data* parent_n = get_idN_data_n(cat_d.m_parent_id_n);
        if (parent_n->m_name.Lower() == parent_name.Lower()) {
            return get_idN_data_n(cat_d.m_id);
        }
    }
    return nullptr;
}

CategoryModel::DataA CategoryModel::find_data_sub_a(const Data& cat_d)
{
    return find_data_a(
        CategoryCol::WHERE_PARENTID(OP_EQ, cat_d.m_id)
    );
}

CategoryModel::DataA CategoryModel::find_data_subtree_a(const Data& cat_d)
{
    DataA tree_a;
    for (const auto& sub_d : find_data_a(
        CategoryCol::WHERE_PARENTID(OP_EQ, cat_d.m_id),
        TableClause::ORDERBY(Col::NAME_CATEGNAME)
    )) {
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
    for (const auto& cat_d : find_data_a(
        TableClause::ORDERBY(Col::NAME_CATEGID)
    )) {
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
    for (auto& cat_d : find_data_a()) {
        if (cat_d.m_name.Lower().Matches(cat_pattern.Lower().Append("*")))
            name_a.push_back(cat_d.m_name);
    }
    return name_a;
}

void CategoryModel::getCategoryStats(
    std::map<int64, std::map<int, double>>& amount_mMonth_mCatId,
    wxSharedPtr<wxArrayString> account_name_a_n,
    mmDateRange* date_range,
    bool WXUNUSED(ignoreFuture), //TODO: deprecated
    bool group_by_month,
    std::map<int64, double>* amount_mCatId_n,
    [[maybe_unused]] bool fin_months
) {
    mmDate startDate = mmDate(date_range->start_date());
    mmDate endDate = mmDate(date_range->end_date());

    // Initialization
    // Set std::map with zerros
    double value = 0;
    int columns = group_by_month ? 12 : 1;

    std::vector<std::pair<mmDate, int>> date_month_a;
    for (int m = 0; m < columns; m++) {
        mmDate date = startDate.plusDateSpan(wxDateSpan::Months(m));
        date_month_a.emplace_back(date, m);
    }
    std::reverse(date_month_a.begin(), date_month_a.end());

    for (int64 cat_id : find_id_a()) {
        for (int m = 0; m < columns; m++) {
            int month = group_by_month ? m : 0;
            amount_mMonth_mCatId[cat_id][month] = value;
        }
    }

    // Calculations
    auto trxId_tpA_m = TrxSplitModel::instance().find_all_mTrxId();
    for (const auto& trx_d : TrxModel::instance().find_data_a(
        TrxModel::WHERE_DATE(OP_GE, startDate),
        TrxModel::WHERE_DATE(OP_LE, endDate),
        TrxModel::WHERE_IS_VALID(true)
    )) {
        if (account_name_a_n) {
            const AccountData* account_n = AccountModel::instance().get_idN_data_n(
                trx_d.m_account_id
            );
            if (account_name_a_n->Index(account_n->m_name) == wxNOT_FOUND)
                continue;
        }

        const double convRate = CurrencyHistoryModel::instance().get_id_date_rate(
            AccountModel::instance().get_idN_data_n(trx_d.m_account_id)->m_currency_id,
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

        int64 cat_id_n = trx_d.m_category_id_n;

        if (trxId_tpA_m[trx_d.m_id].empty()) {
            if (!trx_d.is_transfer()) {
                // Do not include asset or stock transfers in income expense calculations.
                if (TrxModel::is_foreignAsTransfer(trx_d))
                    continue;
                amount_mMonth_mCatId[cat_id_n][month] += trx_d.account_flow(trx_d.m_account_id) * convRate;
            }
            else if (amount_mCatId_n) {
                double amount = trx_d.m_amount * convRate;
                if ((*amount_mCatId_n)[cat_id_n] < 0)
                    amount_mMonth_mCatId[cat_id_n][month] -= amount;
                else
                    amount_mMonth_mCatId[cat_id_n][month] += amount;
            }
        }
        else {
            for (const auto& tp_d : trxId_tpA_m[trx_d.m_id]) {
                amount_mMonth_mCatId[tp_d.m_category_id][month] +=
                    (trx_d.is_withdrawal() ? -tp_d.m_amount : tp_d.m_amount) *
                    convRate;
            }
        }
    }
}
