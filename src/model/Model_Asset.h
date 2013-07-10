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

#ifndef MODEL_ASSET_H
#define MODEL_ASSET_H

#include "Model.h"
#include "db/DB_Table_Assets_V1.h"
#include "Model_Currency.h" // detect base currency

class Model_Asset : public Model, public DB_Table_ASSETS_V1
{
    using DB_Table_ASSETS_V1::all;
public:
    enum RATE { RATE_NONE = 0, RATE_APPRECIATE, RATE_DEPRECIATE };
    enum TYPE { TYPE_PROPERTY = 0, TYPE_AUTO, TYPE_HOUSE, TYPE_ART, TYPE_JEWELLERY, TYPE_CASH, TYPE_OTHER };

public:
    Model_Asset(): Model(), DB_Table_ASSETS_V1() 
    {
        //  keep the sequence with RATE
        this->rates_.Add(wxTRANSLATE("None"));
        this->rates_.Add(wxTRANSLATE("Appreciates"));
        this->rates_.Add(wxTRANSLATE("Depreciates"));

        // keep the sequence with TYPE
        this->types_.Add(wxTRANSLATE("Property"));
        this->types_.Add(wxTRANSLATE("Automobile"));
        this->types_.Add(wxTRANSLATE("Household Object"));
        this->types_.Add(wxTRANSLATE("Art"));
        this->types_.Add(wxTRANSLATE("Jewellery"));
        this->types_.Add(wxTRANSLATE("Cash"));
        this->types_.Add(wxTRANSLATE("Other"));
    };
    ~Model_Asset() {};

public:
    wxArrayString rates_, types_;

public:
    static Model_Asset& instance()
    {
        return Singleton<Model_Asset>::instance();
    }
    static Model_Asset& instance(wxSQLite3Database* db)
    {
        Model_Asset& ins = Singleton<Model_Asset>::instance();
        ins.db_ = db;
        return ins;
    }
    static wxString version()
    {
        return "$Rev$";
    }
public:
    Data_Set all(COLUMN col = COLUMN(0), bool asc = true)
    {
        return all(db_, col, asc);
    }
    double balance()
    {
        double balance = 0.0;
        for (const auto& asset: this->all())
        {
            balance += asset.VALUE;
        }
        return balance;
    }
};

#endif // 
