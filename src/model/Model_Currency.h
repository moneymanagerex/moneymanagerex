/*******************************************************
 Copyright (C) 2013 Guan Lisheng (guanlisheng@gmail.com)

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
#include <wx/numformatter.h>
#include <tuple>

class Model_Currency : public Model<DB_Table_CURRENCYFORMATS_V1>
{
public:
    Model_Currency(): Model<DB_Table_CURRENCYFORMATS_V1>() {};
    ~Model_Currency() {};

public:
    /** Return the static instance of Model_Currency table */
    static Model_Currency& instance()
    {
        return Singleton<Model_Currency>::instance();
    }

    /**
    * Initialize the global Model_Currency table.
    * Reset the Model_Currency table or create the table if it does not exist.
    */
    static Model_Currency& instance(wxSQLite3Database* db)
    {
        Model_Currency& ins = Singleton<Model_Currency>::instance();
        ins.db_ = db;
        ins.destroy_cache();
        bool init_currencies = !ins.exists(db);
        ins.ensure(db);
        if (init_currencies)
            ins.initialize();
        return ins;
    }

public:
    wxArrayString all_currency_names()
    {
        wxArrayString c;
        for (const auto&i : all(COL_CURRENCYNAME)) c.Add(i.CURRENCYNAME);
        return c;
    }
    wxArrayString all_currency_symbols()
    {
        wxSortedArrayString c;
        for (const auto&i : all(COL_CURRENCY_SYMBOL)) c.Add(i.CURRENCY_SYMBOL);
        return c;
    }
public:
    void initialize()
    {
        this->Begin();
        for (const auto& i : all_currencies_template())
        {
            Data *currency = this->create();

            currency->CURRENCY_SYMBOL = std::get<0>(i);
            currency->CURRENCYNAME = std::get<1>(i);
            currency->PFX_SYMBOL = std::get<2>(i);
            currency->SFX_SYMBOL = std::get<3>(i);
            currency->UNIT_NAME = std::get<4>(i);
            currency->CENT_NAME = std::get<5>(i);
            currency->SCALE = std::get<6>(i);
            currency->BASECONVRATE = std::get<7>(i);
            currency->DECIMAL_POINT = ".";
            currency->GROUP_SEPARATOR = ",";

            currency->save(this->db_);
        }
        this->Commit();
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
        if (currency)
        {
            d2s.Prepend(currency->PFX_SYMBOL);
            d2s.Append(currency->SFX_SYMBOL);
        }
        return d2s;
    }
    static wxString os_group_separator()
    {
        wxString sys_thousand_separator = "";
        wxChar sep = ' ';
        if (wxNumberFormatter::GetThousandsSeparatorIfUsed(&sep))
            sys_thousand_separator = wxString::Format("%c", sep);
        return sys_thousand_separator;
    }
    static wxString toString(double value, const Data* currency = GetBaseCurrency())
    {
        int precision = 2;
        int style = wxNumberFormatter::Style_WithThousandsSep;
        wxString s = wxNumberFormatter::ToString(value, precision, style);
        if (currency)
        {
            precision = Model_Currency::precision(currency);
            if (!currency->GROUP_SEPARATOR.empty()) style = wxNumberFormatter::Style_WithThousandsSep;
            s.Replace(wxNumberFormatter::GetDecimalSeparator(), "/");
            s.Replace(os_group_separator(), "|");
            s.Replace("|", currency->GROUP_SEPARATOR);
            s.Replace("/", currency->DECIMAL_POINT);
        }
        return s;
    }
    static wxString fromString(wxString s, const Data* currency)
    {
        // Remove prefix and suffix characters from value
        if (currency)
        {
            if (!currency->PFX_SYMBOL.IsEmpty())
            {
                wxString removed;
                if (s.StartsWith(currency->PFX_SYMBOL, &removed))
                    s = removed;
            }
            if (!currency->SFX_SYMBOL.IsEmpty())
            {
                wxString removed;
                if (s.EndsWith(currency->SFX_SYMBOL, &removed))
                    s = removed;
            }

            wxString sys_thousand_separator;
            wxChar sep = ' ';
            if (wxNumberFormatter::GetThousandsSeparatorIfUsed(&sep))
                sys_thousand_separator = wxString::Format("%c", sep);
            if (!currency->DECIMAL_POINT.empty()) s.Replace(currency->DECIMAL_POINT, "/");
            if (!currency->GROUP_SEPARATOR.empty()) s.Replace(currency->GROUP_SEPARATOR, "|");
            s.Replace("|", sys_thousand_separator);
            s.Replace("/", wxNumberFormatter::GetDecimalSeparator());

        }
        return s;
    }
    static bool fromString(wxString s, double& val, const Data* currency = GetBaseCurrency())
    {
        bool done = true;
        if (!wxNumberFormatter::FromString(fromString(s, currency), &val))
            done = false;
        return done;
    }
    static int precision(const Data* r)
    {
        return static_cast<int>(log10(static_cast<double>(r->SCALE)));
    }
    static int precision(const Data& r) { return precision(&r); }
public:
    static std::vector<std::tuple<wxString, wxString, wxString, wxString, wxString, wxString, int, int> > all_currencies_template();
};
#endif // 
