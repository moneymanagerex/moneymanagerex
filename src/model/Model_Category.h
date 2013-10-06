// -*- C++ -*-
//=============================================================================
/**
 *      Copyright (c) 2013 Guan Lisheng (guanlisheng@gmail.com)
 *
 *      @file
 *
 *      @author [sqlite2cpp.py]
 *
 *      @brief
 *
 *      Revision History:
 *          AUTO GENERATED at 2013-07-03 22:55:49.491495.
 *          DO NOT EDIT!
 */
//=============================================================================

#ifndef MODEL_CATEGORY_H
#define MODEL_CATEGORY_H

#include "Model.h"
#include "db/DB_Table_Category_V1.h"
#include "Model_Subcategory.h"
#include "Model_Checking.h"
#include "Model_Account.h"
#include "Model_Billsdeposits.h"

class Model_Category : public Model, public DB_Table_CATEGORY_V1
{
    using DB_Table_CATEGORY_V1::all;
    using DB_Table_CATEGORY_V1::find;
    using DB_Table_CATEGORY_V1::get;
public:
    Model_Category(): Model(), DB_Table_CATEGORY_V1() 
    {
    };
    ~Model_Category() {};

public:
    wxArrayString types_;

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
    template<class V>
    Data_Set find(COLUMN col, const V& v)
    {
        return find(db_, col, v);
    }
    template<class V1, class V2>
    Data_Set find(COLUMN col1, const V1& v1, COLUMN col2, const V2& v2)
    {
        return find(db_, col1, v1, col2, v2);
    }
    Data* get(int id)
    {
        return this->get(id, this->db_);
    }
    Data* get(const wxString& name)
    {
        Data* category = 0;
        Data_Set items = this->find(this->db_, COL_CATEGNAME, name);
        if (!items.empty()) category = this->get(items[0].CATEGID, this->db_);
        return category;
    }
    int save(Data* r)
    {
        r->save(this->db_);
        return r->id();
    }
public:
    static Model_Subcategory::Data_Set sub_category(const Data* r)
    {
        return Model_Subcategory::instance().find(Model_Subcategory::COL_CATEGID, r->CATEGID);
    }
    static Model_Subcategory::Data_Set sub_category(const Data& r)
    {
        return Model_Subcategory::instance().find(Model_Subcategory::COL_CATEGID, r.CATEGID);
    }
    static wxString full_name(const Data* category, const Model_Subcategory::Data* sub_category = 0)
    {
        if (!category) return "";
        if (!sub_category)
            return category->CATEGNAME;
        else
            return category->CATEGNAME + ":" + sub_category->SUBCATEGNAME;
    }

    static void top_category(std::vector<std::pair<wxString /*category name*/
        , double/*amount*/> >& /* category */, const mmDateRange* date_range, int /* top_n = 7 */)
    {
        for (const auto& tran: Model_Checking::instance().all())
        {
            if (tran.STATUS == "V" || tran.STATUS == "X") continue;
            if (tran.TRANSCODE == TRANS_TYPE_TRANSFER_STR) continue; // skip
            if (Model_Checking::TRANSDATE(tran) < date_range->start_date()) continue;
            if (Model_Checking::TRANSDATE(tran) > date_range->end_date()) continue;
            // TODO: see getTopCategoryStats
            const Model_Account::Data* account = Model_Account::instance().get(tran.ACCOUNTID);
            if (!account) continue;
            const Model_Currency::Data* currency = Model_Account::currency(account);
            if (!currency) continue;
            //const Model_Category::Data* category = Model_Category::instance();
            //if (!category) continue;
            
            if (tran.CATEGID > -1)
            {
                //const wxString categ_name = core_->categoryList_.GetFullCategoryString(CATEGID, SUBCATEGID);
                //stat[categ_name] += trx->value(-1) * currency->BASECONVRATE;
            }
            
        }
    }
    static bool is_used(int id, int sub_id = -1)
    {
        Model_Billsdeposits::Data_Set deposits = Model_Billsdeposits::instance().find(Model_Billsdeposits::COL_CATEGID, id, Model_Billsdeposits::COL_SUBCATEGID, sub_id);
        Model_Checking::Data_Set trans = Model_Checking::instance().find(Model_Checking::COL_CATEGID, id, Model_Checking::COL_SUBCATEGID, sub_id);
        return !deposits.empty() || !trans.empty();
    }
    static bool has_income(int id, int sub_id = -1)
    {
        double sum = 0.0;
        for (const auto& tran: Model_Checking::instance().find(Model_Checking::COL_CATEGID, id, Model_Checking::COL_SUBCATEGID, sub_id))
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
};

#endif // 
