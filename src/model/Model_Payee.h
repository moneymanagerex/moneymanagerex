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

#ifndef MODEL_PAYEE_H
#define MODEL_PAYEE_H

#include "Model.h"
#include "db/DB_Table_Payee_V1.h"

class Model_Payee : public Model, public DB_Table_PAYEE_V1
{
    using DB_Table_PAYEE_V1::all;
    using DB_Table_PAYEE_V1::get;
public:
    Model_Payee(): Model(), DB_Table_PAYEE_V1() 
    {
    };
    ~Model_Payee() {};

public:
    static Model_Payee& instance()
    {
        return Singleton<Model_Payee>::instance();
    }
    static Model_Payee& instance(wxSQLite3Database* db)
    {
        Model_Payee& ins = Singleton<Model_Payee>::instance();
        ins.db_ = db;
        return ins;
    }
public:
    Data_Set all(COLUMN col = COLUMN(0), bool asc = true)
    {
        this->ensure(this->db_);
        return all(db_, col, asc);
    }
    Data_Set FilterPayees(const wxString& payee_pattern)
    {
        Data_Set payees;
        for (auto &payee: this->all())
        {
            if (payee.PAYEENAME.Lower().Matches(payee_pattern.Lower().Append("*")))
                payees.push_back(payee);
        }
        return payees;
    }
    Data* get(int id)
    {
        return this->get(id, this->db_);
    }
    Data* get(const wxString& name)
    {
        Data* payee = 0;
        Data_Set items = this->find(this->db_, COL_PAYEENAME, name);
        if (!items.empty()) payee = this->get(items[0].PAYEEID, this->db_);
        return payee;
    }
    int save(Data* asset)
    {
        asset->save(this->db_);
        return asset->id();
    }
};

#endif // 
