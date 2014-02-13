/*******************************************************
 Copyright (C) 2013 Guan Lisheng (guanlisheng@gmail.com)

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
#include "Model_Category.h"
#include "Model_Checking.h"
#include "Model_Billsdeposits.h"
#include "Model_Account.h"
#include "reports/mmDateRange.h"

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
    bool init_categories = !ins.exists(db);
    ins.ensure(db);
    if (init_categories)
        ins.initialize();
    ins.destroy_cache();

    return ins;
}

/** Return the static instance of Model_Category table */
Model_Category& Model_Category::instance()
{
    return Singleton<Model_Category>::instance();
}

void Model_Category::initialize()
{
    std::vector < std::pair<wxString /*category*/, std::vector<wxString> > > all_categoris;

    //Bills
    {
        std::vector<wxString> sub_categoris;
        sub_categoris.push_back(wxTRANSLATE("Telephone"));
        sub_categoris.push_back(wxTRANSLATE("Electricity"));
        sub_categoris.push_back(wxTRANSLATE("Gas"));
        sub_categoris.push_back(wxTRANSLATE("Internet"));
        sub_categoris.push_back(wxTRANSLATE("Rent"));
        sub_categoris.push_back(wxTRANSLATE("Cable TV"));
        sub_categoris.push_back(wxTRANSLATE("Water"));

        all_categoris.push_back(std::make_pair(wxTRANSLATE("Bills"), sub_categoris));
    }

    // Food
    {
        std::vector<wxString> sub_categoris;
        sub_categoris.push_back(wxTRANSLATE("Groceries"));
        sub_categoris.push_back(wxTRANSLATE("Dining out"));

        all_categoris.push_back(std::make_pair(wxTRANSLATE("Food"), sub_categoris));
    }

    // Leisure
    {
        std::vector<wxString> sub_categoris;
        sub_categoris.push_back(wxTRANSLATE("Movies"));
        sub_categoris.push_back(wxTRANSLATE("Video Rental"));
        sub_categoris.push_back(wxTRANSLATE("Magazines"));

        all_categoris.push_back(std::make_pair(wxTRANSLATE("Leisure"), sub_categoris));
    }

    // Automobile
    {
        std::vector<wxString> sub_categoris;
        sub_categoris.push_back(wxTRANSLATE("Maintenance"));
        sub_categoris.push_back(wxTRANSLATE("Gas"));
        sub_categoris.push_back(wxTRANSLATE("Parking"));
        sub_categoris.push_back(wxTRANSLATE("Registration"));

        all_categoris.push_back(std::make_pair(wxTRANSLATE("Automobile"), sub_categoris));
    }

    // Education
    {
        std::vector<wxString> sub_categoris;
        sub_categoris.push_back(wxTRANSLATE("Books"));
        sub_categoris.push_back(wxTRANSLATE("Tuition"));
        sub_categoris.push_back(wxTRANSLATE("Others"));

        all_categoris.push_back(std::make_pair(wxTRANSLATE("Education"), sub_categoris));
    }

    // Homeneeds 
    {
        std::vector<wxString> sub_categoris;
        sub_categoris.push_back(wxTRANSLATE("Clothing"));
        sub_categoris.push_back(wxTRANSLATE("Furnishing"));
        sub_categoris.push_back(wxTRANSLATE("Others"));

        all_categoris.push_back(std::make_pair(wxTRANSLATE("Homeneeds"), sub_categoris));
    }

    // healthcare
    {
        std::vector<wxString> sub_categoris;
        sub_categoris.push_back(wxTRANSLATE("Health"));
        sub_categoris.push_back(wxTRANSLATE("Dental"));
        sub_categoris.push_back(wxTRANSLATE("Eyecare"));
        sub_categoris.push_back(wxTRANSLATE("Physician"));
        sub_categoris.push_back(wxTRANSLATE("Prescriptions"));

        all_categoris.push_back(std::make_pair(wxTRANSLATE("Healthcare"), sub_categoris));
    }

    // Insurance 
    {
        std::vector<wxString> sub_categoris;
        sub_categoris.push_back(wxTRANSLATE("Auto"));
        sub_categoris.push_back(wxTRANSLATE("Life"));
        sub_categoris.push_back(wxTRANSLATE("Home"));
        sub_categoris.push_back(wxTRANSLATE("Health"));

        all_categoris.push_back(std::make_pair(wxTRANSLATE("Insurance"), sub_categoris));
    }

    // Vacation 
    {
        std::vector<wxString> sub_categoris;
        sub_categoris.push_back(wxTRANSLATE("Travel"));
        sub_categoris.push_back(wxTRANSLATE("Lodging"));
        sub_categoris.push_back(wxTRANSLATE("Sightseeing"));

        all_categoris.push_back(std::make_pair(wxTRANSLATE("Vacation"), sub_categoris));
    }

    // Taxes 
    {
        std::vector<wxString> sub_categoris;
        sub_categoris.push_back(wxTRANSLATE("Income Tax"));
        sub_categoris.push_back(wxTRANSLATE("House Tax"));
        sub_categoris.push_back(wxTRANSLATE("Water Tax"));
        sub_categoris.push_back(wxTRANSLATE("Others"));

        all_categoris.push_back(std::make_pair(wxTRANSLATE("Taxes"), sub_categoris));
    }

    // Miscellaneous
    {
        std::vector<wxString> sub_categoris;

        all_categoris.push_back(std::make_pair(wxTRANSLATE("Miscellaneous"), sub_categoris));
    }

    // Gifts
    {
        std::vector<wxString> sub_categoris;

        all_categoris.push_back(std::make_pair(wxTRANSLATE("Gifts"), sub_categoris));
    }

    // Income
    {
        std::vector<wxString> sub_categoris;
        sub_categoris.push_back(wxTRANSLATE("Salary"));
        sub_categoris.push_back(wxTRANSLATE("Reimbursement/Refunds"));
        sub_categoris.push_back(wxTRANSLATE("Investment Income"));

        all_categoris.push_back(std::make_pair(wxTRANSLATE("Income"), sub_categoris));
    }

    // Other Income
    {
        std::vector<wxString> sub_categoris;
        all_categoris.push_back(std::make_pair(wxTRANSLATE("Other Income"), sub_categoris));
    }

    // Other Expenses
    {
        std::vector<wxString> sub_categoris;
        all_categoris.push_back(std::make_pair(wxTRANSLATE("Other Expenses"), sub_categoris));
    }

    // Transfer
    {
        std::vector<wxString> sub_categoris;
        all_categoris.push_back(std::make_pair(wxTRANSLATE("Transfer"), sub_categoris));
    }

    this->Begin();
    for (const auto& record : all_categoris)
    {
        Model_Category::Data* category = this->create();
        category->CATEGNAME = wxGetTranslation(record.first);
        category->save(this->db_);
        for (const auto& sub_record : record.second)
        {
            Model_Subcategory::Data* sub_category = Model_Subcategory::instance().create();
            sub_category->CATEGID = category->CATEGID;
            sub_category->SUBCATEGNAME = wxGetTranslation(sub_record);
            Model_Subcategory::instance().save(sub_category);
        }
    }
    this->Commit();
}

Model_Category::Data* Model_Category::get(const wxString& name)
{
    Data* category = 0;
    Data_Set items = this->find(CATEGNAME(name));
    if (!items.empty()) category = this->get(items[0].CATEGID, this->db_);
    return category;
}

Model_Subcategory::Data_Set Model_Category::sub_category(const Data* r)
{
    return Model_Subcategory::instance().find(Model_Subcategory::CATEGID(r->CATEGID));
}

Model_Subcategory::Data_Set Model_Category::sub_category(const Data& r)
{
    return Model_Subcategory::instance().find(Model_Subcategory::CATEGID(r.CATEGID));
}

wxString Model_Category::full_name(const Data* category, const Model_Subcategory::Data* sub_category)
{
    if (!category) return "";
    if (!sub_category)
        return category->CATEGNAME;
    else
        return category->CATEGNAME + ":" + sub_category->SUBCATEGNAME;
}

wxString Model_Category::full_name(int category_id, int subcategory_id)
{
    Data* category = Model_Category::instance().get(category_id);
    Model_Subcategory::Data* sub_category = Model_Subcategory::instance().get(subcategory_id);
    return full_name(category, sub_category);
}

bool Model_Category::is_used(int id, int sub_id)
{
    const auto &deposits = Model_Billsdeposits::instance().find(Model_Billsdeposits::CATEGID(id), Model_Billsdeposits::SUBCATEGID(sub_id));
    const auto &trans = Model_Checking::instance().find(Model_Checking::CATEGID(id), Model_Checking::SUBCATEGID(sub_id));
    return !deposits.empty() || !trans.empty();
}

bool Model_Category::has_income(int id, int sub_id)
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

void Model_Category::getCategoryStats(
        std::map<int, std::map<int, std::map<int, double> > > &categoryStats
        , mmDateRange* date_range, bool ignoreFuture
        , bool group_by_month, bool with_date
        , std::map<int, std::map<int, double> > *budgetAmt)
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
    const auto &allSubcategories = Model_Subcategory::instance().all();
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
            if (Model_Checking::TRANSDATE(transaction).IsLaterThan(wxDateTime::Today()))
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
            if (Model_Checking::type(transaction) != Model_Checking::TRANSFER)
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
            const auto &split = Model_Checking::splittransaction(transaction);
            for (const auto& entry: split)
            {
                categoryStats[entry.CATEGID][entry.SUBCATEGID][idx] += entry.SPLITTRANSAMOUNT 
                    * convRate * (Model_Checking::balance(transaction) < 0 ? -1 : 1);
            }
        }
    }
}

