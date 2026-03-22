/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2022  Mark Whalley (mark@ipx.co.uk)

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

#pragma once

#include <map>
#include <set>

#include "base/defs.h"
#include "util/mmSingleton.h"
#include "table/_TableFactory.h"
#include "data/CurrencyData.h"

#include "InfoModel.h"

class CurrencyModel : public TableFactory<CurrencyTable, CurrencyData>
{
public:
    CurrencyModel();
    ~CurrencyModel();

public:
    static CurrencyModel& instance(wxSQLite3Database* db);
    static CurrencyModel& instance();

    static CurrencyCol::CURRENCY_TYPE CURRENCY_TYPE(OP op, CurrencyType currency_type);

public:
    // TODO: add to virtual methods in TableFactory
    int  find_id_dep_c(int64 currency_id);

    // override TableFactory
    virtual bool purge_id(int64 currency_id) override;

    auto get_base_data_n() -> const Data*;
    bool get_base_symbol(wxString& symbol);
    auto get_symbol_data_n(const wxString& symbol) -> const Data*;
    auto find_id_date_m(int64 currency_id) -> std::set<mmDate>;
    auto find_all_name_a() -> const wxArrayString;
    auto find_all_symbol_a() -> const wxArrayString;
    auto find_all_name_id_m() -> const std::map<wxString, int64>;

    const wxString toStringNoFormatting(
        double value, const Data* currency_n = nullptr, int precision = -1
    );
    const wxString toString(
        double value, const Data* currency_n = nullptr, int precision = -1
    );
    const wxString toCurrency(
        double value, const Data* currency_n = CurrencyModel::instance().get_base_data_n(), int precision = -1
    );
    const wxString fromString2CLocale(
        const wxString &s, const Data* currency_n = CurrencyModel::instance().get_base_data_n()
    );
    bool fromString(
        wxString s, double& val, const Data* currency_n = CurrencyModel::instance().get_base_data_n()
    );

    void resetBaseConversionRates();

    // TODO: move to AccountModel
    int  precision(int64 account_id);
};
