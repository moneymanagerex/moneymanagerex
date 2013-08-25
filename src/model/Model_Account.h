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

class Model_Account : public Model, public DB_Table_ACCOUNTLIST_V1
{
    using DB_Table_ACCOUNTLIST_V1::all;
public:
    enum STATE { OPEN = 0, CLOSED };
    enum TYPE  { CHECKING = 0, TERM, INVESTMENT };
public:
    Model_Account(): Model(), DB_Table_ACCOUNTLIST_V1() 
    {
        this->states_.Add(wxTRANSLATE("Open"));
        this->states_.Add(wxTRANSLATE("Closed"));

        this->types_.Add(wxTRANSLATE("Checking"));
        this->types_.Add(wxTRANSLATE("Term"));
        this->types_.Add(wxTRANSLATE("Investment"));
    };
    ~Model_Account() {};

public:
    wxArrayString states_, types_;

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
};

#endif // 
