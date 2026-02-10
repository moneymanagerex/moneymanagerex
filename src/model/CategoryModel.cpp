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

#include <tuple>

#include "AccountModel.h"
#include "CategoryModel.h"
#include "CurrencyHistoryModel.h"
#include "PreferencesModel.h"
#include "ScheduledModel.h"
#include "TransactionModel.h"

#include "reports/mmDateRange.h"

CategoryModel::CategoryModel(): Model<DB_Table_CATEGORY_V1>()
{
}

CategoryModel::~CategoryModel()
{
}

/**
* Initialize the global CategoryModel table.
* Reset the CategoryModel table or create the table if it does not exist.
*/
CategoryModel& CategoryModel::instance(wxSQLite3Database* db)
{
    CategoryModel& ins = Singleton<CategoryModel>::instance();
    ins.db_ = db;
    ins.ensure(db);
    ins.destroy_cache();
    ins.preload();

    return ins;
}

/** Return the static instance of CategoryModel table */
CategoryModel& CategoryModel::instance()
{
    return Singleton<CategoryModel>::instance();
}

const wxArrayString CategoryModel::FilterCategory(const wxString& category_pattern)
{
    wxArrayString categories;
    for (auto &category : CategoryModel::instance().all())
    {
        if (category.CATEGNAME.Lower().Matches(category_pattern.Lower().Append("*")))
            categories.push_back(category.CATEGNAME);
    }
    return categories;
}

CategoryModel::Data* CategoryModel::get(const wxString& name, const wxString& parentname)
{
    Data* category = nullptr;
    Data_Set items = this->find(CATEGNAME(name));
    for (const auto& item : items)
    {
        if (item.PARENTID != -1)
        {
            if (instance().get(item.PARENTID)->CATEGNAME.Lower() == parentname.Lower())
            {
                category = this->get(item.CATEGID, this->db_);
                break;
            }
        }
    }

    return category;
}

CategoryModel::Data* CategoryModel::get(const wxString& name, const int64 parentid)
{
    Data* category = this->get_one(CATEGNAME(name), PARENTID(parentid));
    if (category) return category;

    Data_Set items = this->find(CATEGNAME(name), PARENTID(parentid));
    if (!items.empty()) category = this->get(items[0].CATEGID, this->db_);
    return category;
}

const std::map<wxString, int64> CategoryModel::all_categories(bool excludeHidden)
{
    std::map<wxString, int64> full_categs;
    for (const auto& c : instance().all(COL_CATEGID))
    {
        if (excludeHidden && (c.ACTIVE == 0))
            continue;

        full_categs[full_name(c.CATEGID)] = c.CATEGID;
    }
    return full_categs;
}

CategoryModel::Data_Set CategoryModel::sub_category(const Data* r)
{
    return instance().find(PARENTID(r->CATEGID));
}

CategoryModel::Data_Set CategoryModel::sub_category(const Data& r)
{
    return instance().find(PARENTID(r.CATEGID));
}

CategoryModel::Data_Set CategoryModel::sub_tree(const Data* r)
{
    Data_Set subtree;
    Data_Set subcategories = instance().find(PARENTID(r->CATEGID));
    std::stable_sort(subcategories.begin(), subcategories.end(), SorterByCATEGNAME());
    for (const auto& subcategory : subcategories) {
        subtree.push_back(subcategory);
        Data_Set subtreecats = sub_tree(subcategory);
        for (const auto& cat : subtreecats) {
            subtree.push_back(cat);
        }
    }
    return subtree;
}

CategoryModel::Data_Set CategoryModel::sub_tree(const Data& r)
{
    return sub_tree(&r);
}

const wxString CategoryModel::full_name(const Data* category)
{
    static wxString delimiter;
    if (delimiter.empty()) {
        delimiter = InfotableModel::instance().getString("CATEG_DELIMITER", ":");
    }
    if (!category) return "";
    if (category->PARENTID == -1)
        return category->CATEGNAME;
    else {
        wxString name = category->CATEGNAME;
        Data* parentCategory = instance().get(category->PARENTID);
        while (parentCategory) {
            name = name.Prepend(delimiter).Prepend(parentCategory->CATEGNAME);
            parentCategory = instance().get(parentCategory->PARENTID);
        }
        return name;
    }
}

const wxString CategoryModel::full_name(int64 category_id)
{
    Data* category = instance().get(category_id);
    return full_name(category);
}

const wxString CategoryModel::full_name(int64 category_id, wxString delimiter)
{
    Data* category = instance().get(category_id);
    if (!category) return "";
    if (category->PARENTID == -1)
        return category->CATEGNAME;
    else {
        wxString name = category->CATEGNAME;
        Data* parentCategory = instance().get(category->PARENTID);
        while (parentCategory) {
            name = name.Prepend(delimiter).Prepend(parentCategory->CATEGNAME);
            parentCategory = instance().get(parentCategory->PARENTID);
        }
        return name;
    }
}

// -- Check if Category should be made available for use.
//    Hiding a category hides all sub-categories

bool CategoryModel::is_hidden(int64 catID)
{
    const auto category = CategoryModel::instance().get(catID);
    if (category && category->ACTIVE == 0)
        return true;

    return false;
}

bool CategoryModel::is_used(int64 id)
{
    if (id < 0) return false;
    const auto& trans = TransactionModel::instance().find(TransactionModel::CATEGID(id));
    if (!trans.empty())
    {
        for (const auto& txn : trans)
            if (txn.DELETEDTIME.IsEmpty())
                return true;
    }
    const auto& split = TransactionSplitModel::instance().find(TransactionModel::CATEGID(id));
    if (!split.empty())
    {
        for (const auto& txn : split)
            if (TransactionModel::instance().get(txn.TRANSID)->DELETEDTIME.IsEmpty())
                return true;
    }
    const auto& deposits = ScheduledModel::instance().find(ScheduledModel::CATEGID(id));
    if (!deposits.empty()) return true;
    const auto& deposit_split = ScheduledSplitModel::instance().find(ScheduledModel::CATEGID(id));
    if (!deposit_split.empty()) return true;
    Data_Set children = instance().find(PARENTID(id));
    if (!children.empty()){
        bool used = false;
        for(const auto& child : children){
            used = used || is_used(child.CATEGID);
        }
        return used;
    }
    return false;
}
bool CategoryModel::has_income(int64 id)
{
    double sum = 0.0;
    auto splits = TransactionSplitModel::instance().get_all();
    for (const auto& tran: TransactionModel::instance().find(TransactionModel::CATEGID(id)))
    {
        if (!tran.DELETEDTIME.IsEmpty()) continue;

        switch (TransactionModel::type_id(tran))
        {
        case TransactionModel::TYPE_ID_WITHDRAWAL:
            sum -= tran.TRANSAMOUNT;
            break;
        case TransactionModel::TYPE_ID_DEPOSIT:
            sum += tran.TRANSAMOUNT;
        case TransactionModel::TYPE_ID_TRANSFER:
        default:
            break;
        }

        for (const auto& split: splits[tran.id()])
        {
            switch (TransactionModel::type_id(tran))
            {
            case TransactionModel::TYPE_ID_WITHDRAWAL:
                sum -= split.SPLITTRANSAMOUNT;
                break;
            case TransactionModel::TYPE_ID_DEPOSIT:
                sum += split.SPLITTRANSAMOUNT;
            case TransactionModel::TYPE_ID_TRANSFER:
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
    const auto& allcategories = instance().all();
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
            categoryStats[category.CATEGID][month] = value;
        }
    }
    //Calculations
    auto splits = TransactionSplitModel::instance().get_all();
    for (const auto& transaction : TransactionModel::instance().find(
        TransactionModel::STATUS(TransactionModel::STATUS_ID_VOID, NOT_EQUAL)
        , TransactionModel::TRANSDATE(date_range->start_date(), GREATER_OR_EQUAL)
        , TransactionModel::TRANSDATE(date_range->end_date().FormatISOCombined(), LESS_OR_EQUAL)))
    {
        if (!transaction.DELETEDTIME.IsEmpty()) continue;

        if (accountArray)
        {
            const auto account = AccountModel::instance().get(transaction.ACCOUNTID);
            if (wxNOT_FOUND == accountArray->Index(account->ACCOUNTNAME)) {
                continue;
            }
        }

        const double convRate = CurrencyHistoryModel::getDayRate(
            AccountModel::instance().get(transaction.ACCOUNTID)->CURRENCYID, transaction.TRANSDATE);
        wxDateTime d = TransactionModel::getTransDateTime(transaction);

        int month = 0;
        if (group_by_month)
        {
            auto it = std::find_if(monthMap.begin(), monthMap.end()
                , [d](std::pair<wxDateTime, int> date){return d >= date.first;});
            month = it->second;
        }

        int64 categID = transaction.CATEGID;

        if (splits[transaction.id()].empty())
        {
            if (TransactionModel::type_id(transaction) != TransactionModel::TYPE_ID_TRANSFER)
            {
                // Do not include asset or stock transfers in income expense calculations.
                if (TransactionModel::foreignTransactionAsTransfer(transaction))
                    continue;
                categoryStats[categID][month] += TransactionModel::account_flow(transaction, transaction.ACCOUNTID) * convRate;
            }
            else if (budgetAmt != 0)
            {
                double amt = transaction.TRANSAMOUNT * convRate;
                if ((*budgetAmt)[categID] < 0)
                    categoryStats[categID][month] -= amt;
                else
                    categoryStats[categID][month] += amt;
            }
        }
        else
        {
            for (const auto& entry : splits[transaction.id()])
            {
                categoryStats[entry.CATEGID][month] += entry.SPLITTRANSAMOUNT
                    * convRate * ((TransactionModel::type_id(transaction) == TransactionModel::TYPE_ID_WITHDRAWAL) ? -1 : 1);
            }
        }
    }
}
