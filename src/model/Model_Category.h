/*******************************************************
 Copyright (C) 2013 Guan Lisheng

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

#ifndef MODEL_CATEGORY_H
#define MODEL_CATEGORY_H

#include "Model.h"
#include "db/DB_Table_Category_V1.h"
#include "Model_Subcategory.h"
#include "Model_Checking.h"
#include "Model_Account.h"
#include "Model_Billsdeposits.h"
#include "reports/mmDateRange.h"

class Model_Category : public Model, public DB_Table_CATEGORY_V1
{
    using DB_Table_CATEGORY_V1::all;
    using DB_Table_CATEGORY_V1::get;
    using DB_Table_CATEGORY_V1::remove;
public:
    Model_Category(): Model(), DB_Table_CATEGORY_V1()
    {
    };
    ~Model_Category() {};

public:
    static Model_Category& instance()
    {
        return Singleton<Model_Category>::instance();
    }
    static Model_Category& instance(wxSQLite3Database* db)
    {
        Model_Category& ins = Singleton<Model_Category>::instance();
        ins.db_ = db;
        return ins;
    }
public:
    Data_Set all(COLUMN col = COLUMN(0), bool asc = true)
    {
        this->ensure(this->db_);
        return all(db_, col, asc);
    }
    template<typename... Args>
    Data_Set find(const Args&... args)
    {
        return find_by(this, db_, true, args...);
    }
    Data* get(int id)
    {
        return this->get(id, this->db_);
    }
    Data* get(const wxString& name)
    {
        Data* category = 0;
        Data_Set items = this->find(CATEGNAME(name));
        if (!items.empty()) category = this->get(items[0].CATEGID, this->db_);
        return category;
    }
    int save(Data* r)
    {
        r->save(this->db_);
        return r->id();
    }
    bool remove(int id)
    {
        return this->remove(id, db_);
    }
public:
    static Model_Subcategory::Data_Set sub_category(const Data* r)
    {
        return Model_Subcategory::instance().find(Model_Subcategory::CATEGID(r->CATEGID));
    }
    static Model_Subcategory::Data_Set sub_category(const Data& r)
    {
        return Model_Subcategory::instance().find(Model_Subcategory::CATEGID(r.CATEGID));
    }
    static wxString full_name(const Data* category, const Model_Subcategory::Data* sub_category = 0)
    {
        if (!category) return "";
        if (!sub_category)
            return category->CATEGNAME;
        else
            return category->CATEGNAME + ":" + sub_category->SUBCATEGNAME;
    }
    static wxString full_name(int category_id, int subcategory_id = -1)
    {
        Data* category = Model_Category::instance().get(category_id);
        Model_Subcategory::Data* sub_category = Model_Subcategory::instance().get(subcategory_id);
        return full_name(category, sub_category);
    }
    static bool is_used(int id, int sub_id = -1)
    {
        Model_Billsdeposits::Data_Set deposits = Model_Billsdeposits::instance().find(Model_Billsdeposits::CATEGID(id), Model_Billsdeposits::SUBCATEGID(sub_id));
        Model_Checking::Data_Set trans = Model_Checking::instance().find(Model_Checking::CATEGID(id), Model_Checking::SUBCATEGID(id));
        return !deposits.empty() || !trans.empty();
    }
    static bool has_income(int id, int sub_id = -1)
    {
        double sum = 0.0;
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

            for (const auto& split: Model_Checking::splittransaction(tran))
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
    static void getCategoryStats(
        std::map<int, std::map<int, std::map<int, double> > > &categoryStats
        , mmDateRange* date_range, bool ignoreFuture
        , bool group_by_month = true, bool with_date = true
        , std::map<int, std::map<int, double> > *budgetAmt = 0)
    {
        //Initialization
        //Get base currency rates for all accounts
        std::map<int, double> acc_conv_rates;
        for (const auto& account: Model_Account::instance().all())
        {
            Model_Currency::Data* currency = Model_Account::currency(account);
            acc_conv_rates[account.ACCOUNTID] = currency->BASECONVRATE;
        }
        //Set std::map with zerros
        const Model_Subcategory::Data_Set allSubcategories = Model_Subcategory::instance().all();
        double value = 0;
        int columns = group_by_month ? 12 : 1;
        wxDateTime start_date = wxDateTime(date_range->end_date()).SetDay(1);
        for (const auto& category: Model_Category::instance().all())
        {
            for (int m = 0; m < columns; m++)
            {
                wxDateTime d = wxDateTime(start_date).Subtract(wxDateSpan::Months(m));
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
        for (const auto& transaction: Model_Checking::instance().find(Model_Checking::STATUS(Model_Checking::VOID_, NOT_EQUAL)))
        {
            if (ignoreFuture)
            {
                if (Model_Checking::TRANSDATE(transaction).GetDateOnly().IsLaterThan(wxDateTime::Now().GetDateOnly()))
                    continue; //skip future dated transactions
            }

            if (with_date)
            {
                if (!Model_Checking::TRANSDATE(transaction).IsBetween(date_range->start_date(), date_range->end_date()))
                    continue; //skip
            }

            // We got this far, get the currency conversion rate for this account
            double convRate = acc_conv_rates[transaction.ACCOUNTID];

            wxDateTime d = Model_Checking::TRANSDATE(transaction);
            int idx = group_by_month ? (d.GetYear()*100 + (int)d.GetMonth()) : 0;
            int categID = transaction.CATEGID;

            if (categID > -1)
            {
                if (transaction.TRANSCODE != TRANS_TYPE_TRANSFER_STR)
                {
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
                Model_Splittransaction::Data_Set split = Model_Checking::splittransaction(transaction);
                for (const auto& entry: split)
                {
                    categoryStats[entry.CATEGID][entry.SUBCATEGID][idx] += entry.SPLITTRANSAMOUNT 
                        * convRate * (Model_Checking::balance(transaction) < 0 ? -1 : 1);
                }
            }
        }
    }
};

#endif //
