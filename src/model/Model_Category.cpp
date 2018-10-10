/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)

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
#include <tuple>

Model_Category::Model_Category(): Model<DB_Table_CATEGORY>()
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

Model_Category::Data* Model_Category::get(const wxString& name)
{
    Data* category = this->get_one(CATEGNAME(name));
    if (category) return category;

    Data_Set items = this->find(CATEGNAME(name));
    if (!items.empty()) category = this->get(items[0].CATEGID, this->db_);
    return category;
}

const std::map<wxString, std::pair<int, int> > Model_Category::all_categories()
{
    std::map<wxString, std::pair<int, int> > full_categs;
    for (const auto& c : instance().all(COL_CATEGNAME))
    {
        full_categs[c.CATEGNAME] = std::make_pair(c.CATEGID, -1);
        for (const auto& s : Model_Subcategory::instance().find(Model_Subcategory::CATEGID(c.CATEGID)))
        {
            const wxString nameStr = instance().full_name(c.CATEGID, s.SUBCATEGID);
            full_categs[nameStr] = std::make_pair(c.CATEGID, s.SUBCATEGID);
        }
    }
    return full_categs;
}

Model_Subcategory::Data_Set Model_Category::sub_category(const Data* r)
{
    return Model_Subcategory::instance().find(Model_Subcategory::CATEGID(r->CATEGID));
}

Model_Subcategory::Data_Set Model_Category::sub_category(const Data& r)
{
    return Model_Subcategory::instance().find(Model_Subcategory::CATEGID(r.CATEGID));
}

const wxString Model_Category::full_name(const Data* category, const Model_Subcategory::Data* sub_category)
{
    if (!category) return "";
    if (!sub_category)
        return category->CATEGNAME;
    else
        return category->CATEGNAME + ":" + sub_category->SUBCATEGNAME;
}

const wxString Model_Category::full_name(const int category_id, const int subcategory_id)
{
    Data* category = Model_Category::instance().get(category_id);
    Model_Subcategory::Data* sub_category = Model_Subcategory::instance().get(subcategory_id);
    return full_name(category, sub_category);
}

bool Model_Category::is_used(int id, int sub_id)
{
    const auto &trans = Model_Checking::instance().find(Model_Checking::CATEGID(id), Model_Checking::SUBCATEGID(sub_id));
    if (!trans.empty()) return true;
    const auto &split = Model_Splittransaction::instance().find(Model_Checking::CATEGID(id), Model_Checking::SUBCATEGID(sub_id));
    if (!split.empty()) return true;
    const auto &deposits = Model_Billsdeposits::instance().find(Model_Billsdeposits::CATEGID(id), Model_Billsdeposits::SUBCATEGID(sub_id));
    if (!deposits.empty()) return true;
    const auto &deposit_split = Model_Budgetsplittransaction::instance().find(Model_Billsdeposits::CATEGID(id), Model_Billsdeposits::SUBCATEGID(sub_id));
    if (!deposit_split.empty()) return true;

    return false;
}

bool Model_Category::has_income(int id, int sub_id)
{
    double sum = 0.0;
    auto splits = Model_Splittransaction::instance().get_all();
    for (const auto& tran: Model_Checking::instance().find(Model_Checking::CATEGID(id), Model_Checking::SUBCATEGID(sub_id)))
    {
        switch (Model_Checking::type(tran))
        {
        case Model_Checking::WITHDRAWAL:
            sum -= tran.TRANSAMOUNT;
            break;
        case Model_Checking::DEPOSIT:
            sum += tran.TRANSAMOUNT;
        case Model_Checking::TRANSFER:
        default:
            break;
        }

        for (const auto& split: splits[tran.id()])
        {
            switch (Model_Checking::type(tran))
            {
            case Model_Checking::WITHDRAWAL:
                sum -= split.SPLITTRANSAMOUNT;
                break;
            case Model_Checking::DEPOSIT:
                sum += split.SPLITTRANSAMOUNT;
            case Model_Checking::TRANSFER:
            default:
                break;
            }
        }
    }

    return sum > 0;
}

void Model_Category::getCategoryStats(
        std::map<int, std::map<int, std::map<int, double> > > &categoryStats
        , mmDateRange* date_range, bool ignoreFuture //TODO: deprecated
        , bool group_by_month
        , std::map<int, std::map<int, double> > *budgetAmt)
{
    //Initialization
    //Set std::map with zerros
    const auto &allSubcategories = Model_Subcategory::instance().all();
    double value = 0;
    int columns = group_by_month ? 12 : 1;
    const wxDateTime start_date = wxDateTime(date_range->end_date()).SetDay(1);
    for (const auto& category: Model_Category::instance().all())
    {
        for (int m = 0; m < columns; m++)
        {
            const wxDateTime d = start_date.Subtract(wxDateSpan::Months(m));
            int idx = group_by_month ? (d.GetYear()*100 + (int)d.GetMonth()) : 0;
            categoryStats[category.CATEGID][-1][idx] = value;
            for (const auto & sub_category : allSubcategories)
            {
                if (sub_category.CATEGID == category.CATEGID)
                    categoryStats[category.CATEGID][sub_category.SUBCATEGID][idx] = value;
            }
        }
    }
    //Calculations
    auto splits = Model_Splittransaction::instance().get_all();
    for (const auto& transaction: Model_Checking::instance().find(
        Model_Checking::STATUS(Model_Checking::VOID_, NOT_EQUAL)
        , Model_Checking::TRANSDATE(date_range->start_date(), GREATER_OR_EQUAL)
        , Model_Checking::TRANSDATE(date_range->end_date(), LESS_OR_EQUAL)))
    {
        const double convRate = Model_CurrencyHistory::getDayRate(Model_Account::instance().get(transaction.ACCOUNTID)->CURRENCYID, transaction.TRANSDATE);
        const wxDateTime &d = Model_Checking::TRANSDATE(transaction);
        int idx = group_by_month ? (d.GetYear()*100 + (int)d.GetMonth()) : 0;
        int categID = transaction.CATEGID;

        if (categID > -1)
        {
            if (Model_Checking::type(transaction) != Model_Checking::TRANSFER)
            {
                // Do not include asset or stock transfers in income expense calculations.
                if (Model_Checking::foreignTransactionAsTransfer(transaction))
                    continue;
                categoryStats[categID][transaction.SUBCATEGID][idx] += Model_Checking::balance(transaction) * convRate;
            }
            else if (budgetAmt != 0)
            {
                double amt = transaction.TRANSAMOUNT * convRate;
                if ((*budgetAmt)[categID][transaction.SUBCATEGID] < 0)
                    categoryStats[categID][transaction.SUBCATEGID][idx] -= amt;
                else
                    categoryStats[categID][transaction.SUBCATEGID][idx] += amt;
            }
        }
        else
        {
            for (const auto& entry: splits[transaction.id()])
            {
                categoryStats[entry.CATEGID][entry.SUBCATEGID][idx] += entry.SPLITTRANSAMOUNT 
                    * convRate * (Model_Checking::balance(transaction) < 0 ? -1 : 1);
            }
        }
    }
}

