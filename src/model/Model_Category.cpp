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

#include "Model_Category.h"
#include "Model_Checking.h"
#include "Model_Billsdeposits.h"
#include "Model_Account.h"
#include "Model_CurrencyHistory.h"
#include "reports/mmDateRange.h"
#include "option.h"
#include <tuple>

Model_Category::Model_Category(): Model<DB_Table_CATEGORY_V1>()
{
}

Model_Category::~Model_Category()
{
}

/**
* Initialize the global Model_Category table.
* Reset the Model_Category table or create the table if it does not exist.
*/
Model_Category& Model_Category::instance(wxSQLite3Database* db)
{
    Model_Category& ins = Singleton<Model_Category>::instance();
    ins.db_ = db;
    ins.ensure(db);
    ins.destroy_cache();
    ins.preload();

    return ins;
}

/** Return the static instance of Model_Category table */
Model_Category& Model_Category::instance()
{
    return Singleton<Model_Category>::instance();
}

const wxArrayString Model_Category::FilterCategory(const wxString& category_pattern)
{
    wxArrayString categories;
    for (auto &category : Model_Category::instance().all())
    {
        if (category.CATEGNAME.Lower().Matches(category_pattern.Lower().Append("*")))
            categories.push_back(category.CATEGNAME);
    }
    return categories;
}

Model_Category::Data* Model_Category::get(const wxString& name, const wxString& parentname)
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

Model_Category::Data* Model_Category::get(const wxString& name, const int64 parentid)
{
    Data* category = this->get_one(CATEGNAME(name), PARENTID(parentid));
    if (category) return category;

    Data_Set items = this->find(CATEGNAME(name), PARENTID(parentid));
    if (!items.empty()) category = this->get(items[0].CATEGID, this->db_);
    return category;
}

const std::map<wxString, int64> Model_Category::all_categories(bool excludeHidden)
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

Model_Category::Data_Set Model_Category::sub_category(const Data* r)
{
    return instance().find(PARENTID(r->CATEGID));
}

Model_Category::Data_Set Model_Category::sub_category(const Data& r)
{
    return instance().find(PARENTID(r.CATEGID));
}

Model_Category::Data_Set Model_Category::sub_tree(const Data* r)
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

Model_Category::Data_Set Model_Category::sub_tree(const Data& r)
{
    return sub_tree(&r);
}

const wxString Model_Category::full_name(const Data* category)
{
    static wxString delimiter;
    if (delimiter.empty()) {
        delimiter = Model_Infotable::instance().GetStringInfo("CATEG_DELIMITER", ":");
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

const wxString Model_Category::full_name(int64 category_id)
{
    Data* category = instance().get(category_id);
    return full_name(category);
}

const wxString Model_Category::full_name(int64 category_id, wxString delimiter)
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

bool Model_Category::is_hidden(int64 catID)
{
    const auto category = Model_Category::instance().get(catID);
    if (category && category->ACTIVE == 0)
        return true;

    return false;
}

bool Model_Category::is_used(int64 id)
{
    if (id < 0) return false;
    const auto& trans = Model_Checking::instance().find(Model_Checking::CATEGID(id));
    if (!trans.empty())
    {
        for (const auto& txn : trans)
            if (txn.DELETEDTIME.IsEmpty())
                return true;
    }
    const auto& split = Model_Splittransaction::instance().find(Model_Checking::CATEGID(id));
    if (!split.empty())
    {
        for (const auto& txn : split)
            if (Model_Checking::instance().get(txn.TRANSID)->DELETEDTIME.IsEmpty())
                return true;
    }
    const auto& deposits = Model_Billsdeposits::instance().find(Model_Billsdeposits::CATEGID(id));
    if (!deposits.empty()) return true;
    const auto& deposit_split = Model_Budgetsplittransaction::instance().find(Model_Billsdeposits::CATEGID(id));
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
bool Model_Category::has_income(int64 id)
{
    double sum = 0.0;
    auto splits = Model_Splittransaction::instance().get_all();
    for (const auto& tran: Model_Checking::instance().find(Model_Checking::CATEGID(id)))
    {
        if (!tran.DELETEDTIME.IsEmpty()) continue;

        switch (Model_Checking::type_id(tran))
        {
        case Model_Checking::TYPE_ID_WITHDRAWAL:
            sum -= tran.TRANSAMOUNT;
            break;
        case Model_Checking::TYPE_ID_DEPOSIT:
            sum += tran.TRANSAMOUNT;
        case Model_Checking::TYPE_ID_TRANSFER:
        default:
            break;
        }

        for (const auto& split: splits[tran.id()])
        {
            switch (Model_Checking::type_id(tran))
            {
            case Model_Checking::TYPE_ID_WITHDRAWAL:
                sum -= split.SPLITTRANSAMOUNT;
                break;
            case Model_Checking::TYPE_ID_DEPOSIT:
                sum += split.SPLITTRANSAMOUNT;
            case Model_Checking::TYPE_ID_TRANSFER:
            default:
                break;
            }
        }
    }

    return sum > 0;
}

void Model_Category::getCategoryStats(
    std::map<int64, std::map<int, double>>& categoryStats
    , wxSharedPtr<wxArrayString> accountArray
    , mmDateRange* date_range
    , bool WXUNUSED(ignoreFuture) //TODO: deprecated
    , bool group_by_month
    , std::map<int64, double> *budgetAmt
    , bool fin_months)
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
        monthMap.push_back(std::make_pair(d,m));
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
    auto splits = Model_Splittransaction::instance().get_all();
    for (const auto& transaction : Model_Checking::instance().find(
        Model_Checking::STATUS(Model_Checking::STATUS_ID_VOID, NOT_EQUAL)
        , Model_Checking::TRANSDATE(date_range->start_date(), GREATER_OR_EQUAL)
        , Model_Checking::TRANSDATE(date_range->end_date().FormatISOCombined(), LESS_OR_EQUAL)))
    {
        if (!transaction.DELETEDTIME.IsEmpty()) continue;

        if (accountArray)
        {
            const auto account = Model_Account::instance().get(transaction.ACCOUNTID);
            if (wxNOT_FOUND == accountArray->Index(account->ACCOUNTNAME)) {
                continue;
            }
        }

        const double convRate = Model_CurrencyHistory::getDayRate(
            Model_Account::instance().get(transaction.ACCOUNTID)->CURRENCYID, transaction.TRANSDATE);
        wxDateTime d = Model_Checking::TRANSDATE(transaction);

        int month = 0;
        if (group_by_month)
        {
            auto it = std::find_if(monthMap.begin(), monthMap.end()
                , [d](std::pair<wxDateTime, int> date){return d >= date.first;});
            month = it->second;
        }

        int64 categID = transaction.CATEGID;

        if (categID > -1)
        {
            if (Model_Checking::type_id(transaction) != Model_Checking::TYPE_ID_TRANSFER)
            {
                // Do not include asset or stock transfers in income expense calculations.
                if (Model_Checking::foreignTransactionAsTransfer(transaction))
                    continue;
                categoryStats[categID][month] += Model_Checking::account_flow(transaction, transaction.ACCOUNTID) * convRate;
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
                    * convRate * ((Model_Checking::type_id(transaction) == Model_Checking::TYPE_ID_WITHDRAWAL) ? -1 : 1);
            }
        }
    }
}
