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

#ifndef MODEL_STOCK_H
#define MODEL_STOCK_H

#include "Model.h"
#include "db/DB_Table_Stock_V1.h"
#include "Model_Currency.h" // detect base currency

class Model_Stock : public Model, public DB_Table_STOCK_V1
{
    using DB_Table_STOCK_V1::all;
public:
    Model_Stock(): Model(), DB_Table_STOCK_V1() {};
    ~Model_Stock() {};

public:
    static Model_Stock& instance()
    {
        return Singleton<Model_Stock>::instance();
    }
    static Model_Stock& instance(wxSQLite3Database* db)
    {
        Model_Stock& ins = Singleton<Model_Stock>::instance();
        ins.db_ = db;
        return ins;
    }
public:
    Data_Set all(COLUMN col = COLUMN(0), bool asc = true)
    {
        return all(db_, col, asc);
    }
};

#endif // 
