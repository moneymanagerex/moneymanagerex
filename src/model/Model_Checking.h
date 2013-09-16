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

#ifndef MODEL_CHECKING_H
#define MODEL_CHECKING_H

#include "Model.h"
#include "db/DB_Table_Checkingaccount_V1.h"

class Model_Checking : public Model, public DB_Table_CHECKINGACCOUNT_V1
{
    using DB_Table_CHECKINGACCOUNT_V1::all;
    using DB_Table_CHECKINGACCOUNT_V1::get;
public:
    Model_Checking(): Model(), DB_Table_CHECKINGACCOUNT_V1() 
    {
    };
    ~Model_Checking() {};

public:
    static Model_Checking& instance()
    {
        return Singleton<Model_Checking>::instance();
    }
    static Model_Checking& instance(wxSQLite3Database* db)
    {
        Model_Checking& ins = Singleton<Model_Checking>::instance();
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
    int save(Data* asset)
    {
        asset->save(this->db_);
        return asset->id();
    }
};

#endif // 
