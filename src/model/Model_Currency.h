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

#ifndef MODEL_CURRENCY_H
#define MODEL_CURRENCY_H

#include "Model.h"
#include "db/DB_Table_Currencyformats_V1.h"
#include "Model_Infotable.h" // detect base currency setting BASECURRENCYID
#include "mmcurrency.h"
#include <wx/numformatter.h>

class Model_Currency : public Model, public DB_Table_CURRENCYFORMATS_V1
{
    using DB_Table_CURRENCYFORMATS_V1::all;
    using DB_Table_CURRENCYFORMATS_V1::get;
    using DB_Table_CURRENCYFORMATS_V1::remove;
public:
    Model_Currency(): Model(), DB_Table_CURRENCYFORMATS_V1() {};
    ~Model_Currency() {};

public:
    static Model_Currency& instance()
    {
        return Singleton<Model_Currency>::instance();
    }
    static Model_Currency& instance(wxSQLite3Database* db)
    {
        Model_Currency& ins = Singleton<Model_Currency>::instance();
        ins.db_ = db;
        ins.all();
        return ins;
    }

public:
    Data_Set all(COLUMN col = COLUMN(0), bool asc = true)
    {
        if (!this->exists(this->db_))
        {
            this->ensure(this->db_);
            for (const auto& i : mmCurrency::currency_map())
            {
                Data *currency = this->create();
                currency->CURRENCYNAME = i.currencyName_;
                currency->PFX_SYMBOL   = i.pfxSymbol_;
                currency->SFX_SYMBOL   = i.sfxSymbol_;
                currency->DECIMAL_POINT = i.dec_;
                currency->GROUP_SEPARATOR = i.grp_;
                currency->UNIT_NAME = i.unit_;
                currency->CENT_NAME = i.cent_;
                currency->SCALE = i.scaleDl_;
                currency->BASECONVRATE = i.baseConv_;
                currency->CURRENCY_SYMBOL = i.currencySymbol_;

                currency->save(this->db_);
            }
        }
        return all(db_, col, asc);
    }
    Data* get(int id)
    {
        return get(id, this->db_);
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
public:
    // Getter
    static Data* GetBaseCurrency()
    {
        int currency_id = Model_Infotable::instance().GetBaseCurrencyId();
        return Model_Currency::instance().get(currency_id);
    }
public:
    static wxString toCurrency(double value, const Data* currency = GetBaseCurrency())
    {
        wxString d2s = toString(value, currency);
        d2s.Prepend(currency->PFX_SYMBOL);
        d2s.Append(currency->SFX_SYMBOL);
        return d2s;
    }
    static wxString toString(double value, const Data* currency = GetBaseCurrency())
    {
        wxString d2s = "";
        if (currency) 
            d2s = wxNumberFormatter::ToString(value, Model_Currency::precision(currency)); // Style_WithThousandsSep
        else
            d2s = wxNumberFormatter::ToString(value, 1);
        return d2s;
    }
    static int precision(const Data* r)
    {
        return static_cast<int>(log10(static_cast<double>(r->SCALE)));
    }
    static int precision(const Data& r) { return precision(&r); }
};

#endif // 
