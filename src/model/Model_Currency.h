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

#ifndef MODEL_CURRENCY_H
#define MODEL_CURRENCY_H

#include "Model.h"
#include "db/DB_Table_Currencyformats_V1.h"
#include "Model_Infotable.h" // detect base currency setting BASECURRENCYID

class Model_Currency : public Model, public DB_Table_CURRENCYFORMATS_V1
{
    using DB_Table_CURRENCYFORMATS_V1::all;
public:
    Model_Currency(): Model(), DB_Table_CURRENCYFORMATS_V1() {};
    ~Model_Currency() {};

public:
    static Model_Currency& instance()
    {
        return Singleton<Model_Currency>::instance();
    }

public:
    Data_Set all(COLUMN col = COLUMN(0), bool asc = true)
    {
        this->ensure(this->db_);
        return all(db_, col, asc);
    }
public:
    // Getter
    Data* GetBaseCurrency()
    {
        int currency_id = Model_Infotable::instance().GetBaseCurrencyId();
        return this->get(currency_id, this->db_);
    }

};

#endif // 
