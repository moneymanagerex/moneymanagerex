/*******************************************************
 Copyright (C) 2013 Guan Lisheng

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

#ifndef MODEL_ASSET_H
#define MODEL_ASSET_H

#include "Model.h"
#include "db/DB_Table_Assets_V1.h"
#include "Model_Currency.h" // detect base currency

class Model_Asset : public Model, public DB_Table_ASSETS_V1
{
    using DB_Table_ASSETS_V1::all;
    using DB_Table_ASSETS_V1::get;
    using DB_Table_ASSETS_V1::remove;
public:
    enum RATE { RATE_NONE = 0, RATE_APPRECIATE, RATE_DEPRECIATE };
    enum TYPE { TYPE_PROPERTY = 0, TYPE_AUTO, TYPE_HOUSE, TYPE_ART, TYPE_JEWELLERY, TYPE_CASH, TYPE_OTHER };

public:
    Model_Asset(): Model(), DB_Table_ASSETS_V1() 
    {
    };
    ~Model_Asset() {};

public:
    static wxArrayString all_rate()
    {
        wxArrayString rates;
        //  keep the sequence with RATE
        rates.Add(wxTRANSLATE("None"));
        rates.Add(wxTRANSLATE("Appreciates"));
        rates.Add(wxTRANSLATE("Depreciates"));

        return rates;
    }
    static wxArrayString all_type()
    {
        wxArrayString types;
        // keep the sequence with TYPE
        types.Add(wxTRANSLATE("Property"));
        types.Add(wxTRANSLATE("Automobile"));
        types.Add(wxTRANSLATE("Household Object"));
        types.Add(wxTRANSLATE("Art"));
        types.Add(wxTRANSLATE("Jewellery"));
        types.Add(wxTRANSLATE("Cash"));
        types.Add(wxTRANSLATE("Other"));

        return types;
    }

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
    template<typename... Args>
    Data_Set find(const Args&... args)
    {
        return find_by(this, db_, true, args...);
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
    bool remove(int id)
    {
        return this->remove(id, db_);
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
        else if (r->ASSETTYPE.CmpNoCase("Household Object") == 0)
            return TYPE_HOUSE;
        else if (r->ASSETTYPE.CmpNoCase("Art") == 0)
            return TYPE_ART;
        else if (r->ASSETTYPE.CmpNoCase("Jewellery") == 0)
            return TYPE_JEWELLERY;
        else if (r->ASSETTYPE.CmpNoCase("Cash") == 0)
            return TYPE_CASH;
        else if (r->ASSETTYPE.CmpNoCase("Other") == 0)
            return TYPE_OTHER;

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
    static Model_Currency::Data* currency(const Data* /* r */)
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
