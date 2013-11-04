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
        rates.Add("None");
        rates.Add("Appreciates");
        rates.Add("Depreciates");

        return rates;
    }
    static wxArrayString all_type()
    {
        wxArrayString types;
        // keep the sequence with TYPE
        types.Add("Property");
        types.Add("Automobile");
        types.Add("Household Object");
        types.Add("Art");
        types.Add("Jewellery");
        types.Add("Cash");
        types.Add("Other");

        return types;
    }

public:
    /** Return the address of the global database table*/
    static Model_Asset& instance()
    {
        return Singleton<Model_Asset>::instance();
    }

    /**
     * Initialize the global database table.
     * Create the table if it does not exist.
    */
    static Model_Asset& instance(wxSQLite3Database* db)
    {
        Model_Asset& ins = Singleton<Model_Asset>::instance();
        ins.db_ = db;
        ins.destroy_cache();
        ins.all();
        return ins;
    }
    static wxString version()
    {
        return "$Rev$";
    }
public:
    /** Return a list of all the records in the database table*/
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
    static DB_Table_ASSETS_V1::ASSETTYPE ASSETTYPE(TYPE type, OP op=EQUAL) { return DB_Table_ASSETS_V1::ASSETTYPE(all_type()[type], op); }
    static DB_Table_ASSETS_V1::STARTDATE STARTDATE(const wxDate& date, OP op=EQUAL) { return DB_Table_ASSETS_V1::STARTDATE(date.FormatISODate(), op); }
public:
    static wxDate STARTDATE(const Data* r) { return Model::to_date(r->STARTDATE); }
    static wxDate STARTDATE(const Data& r) { return Model::to_date(r.STARTDATE); }

    static TYPE type(const Data* r) 
    {
        if (r->ASSETTYPE.CmpNoCase(all_type()[TYPE_PROPERTY]) == 0)
            return TYPE_PROPERTY;
        else if (r->ASSETTYPE.CmpNoCase(all_type()[TYPE_AUTO]) == 0)
            return TYPE_AUTO;
        else if (r->ASSETTYPE.CmpNoCase(all_type()[TYPE_HOUSE]) == 0)
            return TYPE_HOUSE;
        else if (r->ASSETTYPE.CmpNoCase(all_type()[TYPE_ART]) == 0)
            return TYPE_ART;
        else if (r->ASSETTYPE.CmpNoCase(all_type()[TYPE_JEWELLERY]) == 0)
            return TYPE_JEWELLERY;
        else if (r->ASSETTYPE.CmpNoCase(all_type()[TYPE_CASH]) == 0)
            return TYPE_CASH;
        else if (r->ASSETTYPE.CmpNoCase(all_type()[TYPE_OTHER]) == 0)
            return TYPE_OTHER;

        return TYPE(-1);
    }
    static TYPE type(const Data& r)
    {
        return type(&r);
    }

    static RATE rate(const Data* r)
    {
        if (r->VALUECHANGE.CmpNoCase(all_rate()[RATE_NONE]) == 0)
            return RATE_NONE;
        else if (r->VALUECHANGE.CmpNoCase(all_rate()[RATE_APPRECIATE]) == 0)
            return RATE_APPRECIATE;
        else if (r->VALUECHANGE.CmpNoCase(all_rate()[RATE_DEPRECIATE]) == 0)
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
