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

#ifndef MODEL_ACCOUNT_H
#define MODEL_ACCOUNT_H

#include "Model.h"
#include "db/DB_Table_Accountlist_V1.h"
#include "Model_Currency.h" // detect base currency
#include "Model_Checking.h"
#include "Model_Stock.h"

class Model_Account : public Model, public DB_Table_ACCOUNTLIST_V1
{
    using DB_Table_ACCOUNTLIST_V1::all;
    using DB_Table_ACCOUNTLIST_V1::get;
public:
    enum STATUS { OPEN = 0, CLOSED };
    enum TYPE  { CHECKING = 0, TERM, INVESTMENT };
public:
    Model_Account(): Model(), DB_Table_ACCOUNTLIST_V1() 
    {
        this->statuss_.Add(wxTRANSLATE("Open"));
        this->statuss_.Add(wxTRANSLATE("Closed"));

        this->types_.Add(wxTRANSLATE("Checking"));
        this->types_.Add(wxTRANSLATE("Term"));
        this->types_.Add(wxTRANSLATE("Investment"));
    };
    ~Model_Account() {};

public:
    wxArrayString statuss_, types_;

public:
    static Model_Account& instance()
    {
        return Singleton<Model_Account>::instance();
    }
    static Model_Account& instance(wxSQLite3Database* db)
    {
        Model_Account& ins = Singleton<Model_Account>::instance();
        ins.db_ = db;
        return ins;
    }
public:
    Data_Set all(COLUMN col = COLUMN(0), bool asc = true)
    {
        this->ensure(this->db_);
        return all(db_, col, asc);
    }
    Data* get(int id)
    {
        return this->get(id, this->db_);
    }

    static Model_Currency::Data* currency(const Data* r)
    {
        return Model_Currency::instance().get(r->CURRENCYID);
    }
public:
    static double balance(const Data* r)
    {
        double sum = r->INITIALBAL;
        for (const auto& tran: Model_Checking::instance().find(Model_Checking::COL_ACCOUNTID, r->ACCOUNTID, Model_Checking::COL_TOACCOUNTID, r->ACCOUNTID, false))
        {
           sum += Model_Checking::balance(tran, r->ACCOUNTID); 
        }
        return sum;
    }
    static double balance(const Data& r) { return balance(&r); }
    static std::pair<double, double> investment_balance(const Data* r)
    {
        std::pair<double /*origianl input value*/, double /**/> sum;
        for (const auto& stock: Model_Stock::instance().find(Model_Stock::COL_HELDAT, r->ACCOUNTID))
        {
            sum.first += stock.VALUE;
            sum.second += Model_Stock::value(stock);
        }
        return sum;
    }
    static std::pair<double, double> investment_balance(const Data& r) { return investment_balance(&r); }
public:
    static STATUS status(const Data* account)
    {
        if (account->STATUS.CmpNoCase("Open") == 0)
            return OPEN;
        return CLOSED;
    }
    static STATUS status(const Data& account)
    {
        return status(&account);
    }
    static TYPE type(const Data* account)
    {
        if (account->ACCOUNTTYPE.CmpNoCase("Checking") == 0)
            return CHECKING;
        else if (account->ACCOUNTTYPE.CmpNoCase("Term") == 0)
            return TERM;
        else
            return INVESTMENT;
    }
    static TYPE type(const Data& account)
    {
        return type(&account);
    }
};

#endif // 
