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
    using DB_Table_ASSETS_V1::get;
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
        this->ensure(this->db_);
        return all(db_, col, asc);
    }
    Data* get(int id)
    {
        return this->get(id, this->db_);
    }
    int save(Data* r)
    {
        r->save(this->db_);
        return r->id();
    }
    double balance()
    {
        double balance = 0.0;
        for (const auto& r: this->all())
        {
            balance += value(r);
        }
        return balance;
    }

public:
    static wxDate STARTDATE(const Data* r) { return Model::to_date(r->STARTDATE); }
    static wxDate STARTDATE(const Data& r) { return Model::to_date(r.STARTDATE); }

    static TYPE type(const Data* r) 
    {
        if (r->ASSETTYPE.CmpNoCase("Property") == 0)
            return TYPE_PROPERTY;
        else if (r->ASSETTYPE.CmpNoCase("Automobile") == 0)
            return TYPE_AUTO;

        return TYPE(-1);
    }
    static TYPE type(const Data& r)
    {
        return type(&r);
    }

    static RATE rate(const Data* r)
    {
        if (r->VALUECHANGE.CmpNoCase("None") == 0)
            return RATE_NONE;
        else if (r->VALUECHANGE.CmpNoCase("Appreciates") == 0)
            return RATE_APPRECIATE;
        else if (r->VALUECHANGE.CmpNoCase("Depreciates") == 0)
            return RATE_DEPRECIATE;

        return RATE(-1);
    }
    static RATE rate(const Data& r)
    {
        return rate(&r);
    }
    static Model_Currency::Data* currency(const Data* r)
    {
        return Model_Currency::instance().GetBaseCurrency();
    }
public:
    static double value(const Data* r)
    {
        double sum = r->VALUE;
        wxDate start_date = STARTDATE(r);
        int diff_days = abs(start_date.Subtract(wxDateTime::Now()).GetDays());
        switch (rate(r))
        {
        case RATE_NONE:
            break;
        case RATE_APPRECIATE:
            sum += ((r->VALUE * (r->VALUECHANGERATE/100))/365.25) * diff_days;
            break;
        case RATE_DEPRECIATE:
            sum -= ((r->VALUE * (r->VALUECHANGERATE/100))/365.25) * diff_days;
            break;
        default:
            break;
        }

        return sum;
    }
    static double value(const Data& r) { return value(&r); }
};

#endif // 
