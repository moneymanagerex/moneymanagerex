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

#include "base/defs.h"
#include "util/mmChoice.h"

#include "table/CurrencyTable.h"
#include "data/CurrencyData.h"

#include "_ModelBase.h"
#include "InfoModel.h" // detect base currency setting BASECURRENCYID

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
    bool purge_id(int64 currency_id) override;

    auto get_symbol_data_n(const wxString& symbol) -> const Data*;

    auto find_all_name_a() -> const wxArrayString;
    auto find_all_symbol_a() -> const wxArrayString;
    auto find_all_name_id_m() -> const std::map<wxString, int64>;

    // TODO: move to AccountModel
    int  precision(int64 account_id);

    static bool is_used(int64 currency_id);
    static auto GetBaseCurrency() -> const Data*;
    static bool GetBaseCurrencySymbol(wxString& symbol);

    static auto DateUsed(int64 CurrencyID) -> std::map<wxDateTime, int>;

    static const wxString toStringNoFormatting(
        double value, const Data* currency_n = GetBaseCurrency(), int precision = -1
    );
    static const wxString toString(
        double value, const Data* currency_n = GetBaseCurrency(), int precision = -1
    );
    static const wxString toCurrency(
        double value, const Data* currency_n = GetBaseCurrency(), int precision = -1
    );
    static const wxString fromString2CLocale(
        const wxString &s, const Data* currency_n = CurrencyModel::GetBaseCurrency()
    );
    static bool fromString(
        wxString s, double& val, const Data* currency_n = GetBaseCurrency()
    );

    static void ResetBaseConversionRates();
};
