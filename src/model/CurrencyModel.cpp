/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2013 - 2022 Nikolay Akimov
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

#include <fmt/core.h>
#include <fmt/format.h>

#include "util/_util.h"

#include "AccountModel.h"
#include "CurrencyHistoryModel.h"
#include "CurrencyModel.h"
#include "PrefModel.h"
#include "StockModel.h"
#include "TrxModel.h"

#include "uicontrols/navigatortypes.h"

constexpr auto LIMIT = 1e-10;
static wxString s_locale;
static wxString s_use_locale;

CurrencyModel::CurrencyModel() :
    TableFactory<CurrencyTable, CurrencyData>()
{
}

CurrencyModel::~CurrencyModel()
{
}

// Initialize the global CurrencyModel table.
// Reset the CurrencyModel table or create the table if it does not exist.
CurrencyModel& CurrencyModel::instance(wxSQLite3Database* db)
{
    CurrencyModel& ins = Singleton<CurrencyModel>::instance();
    ins.reset_cache();
    ins.m_db = db;
    ins.ensure_table();
    ins.preload_cache();

    s_locale = wxEmptyString;
    s_use_locale = wxEmptyString;
    return ins;
}

// Return the static instance of CurrencyModel table
CurrencyModel& CurrencyModel::instance()
{
    return Singleton<CurrencyModel>::instance();
}

CurrencyCol::CURRENCY_TYPE CurrencyModel::CURRENCY_TYPE(OP op, CurrencyType currency_type)
{
    return CurrencyCol::CURRENCY_TYPE(op, currency_type.name());
}

int CurrencyModel::find_id_dep_c(int64 currency_id)
{
    int dep_c = 0;

    if (PrefModel::instance().getBaseCurrencyID() == currency_id)
        dep_c += 1;

    // FIXME: do not exclude closed accounts
    dep_c += AccountModel::instance().find(
        AccountCol::CURRENCYID(currency_id),
        AccountModel::STATUS(OP_NE, AccountStatus(AccountStatus::e_closed))
    ).size();

    // FIXME: search for currency_id in AssetModel

    return dep_c;
}

// Remove the Data record from memory and the database.
// Delete also all currency history
bool CurrencyModel::purge_id(int64 currency_id)
{
    // purge CurrencyHistoryData owned by currency_id
    db_savepoint();
    for (const auto& uh_d : CurrencyHistoryModel::instance().find(
        CurrencyHistoryCol::CURRENCYID(currency_id)
    ))
        CurrencyHistoryModel::instance().purge_id(uh_d.m_id);
    db_release_savepoint();

    return unsafe_remove_id(currency_id);
}

// Return the Data record of the base currency.
const CurrencyData* CurrencyModel::get_base_data_n()
{
    int64 currency_id = PrefModel::instance().getBaseCurrencyID();
    return get_id_data_n(currency_id);
}

bool CurrencyModel::get_base_symbol(wxString& symbol)
{
    const Data* currency_n = get_base_data_n();
    if (currency_n) {
        symbol = currency_n->m_symbol;
        return true;
    }
    return false;
}

// Return the currency Data record for the given symbol
const CurrencyData* CurrencyModel::get_symbol_data_n(const wxString& symbol)
{
    const CurrencyData* currency_n = search_cache_n(
        CurrencyCol::CURRENCY_SYMBOL(symbol)
    );
    if (currency_n)
        return currency_n;

    const DataA currency_a = CurrencyModel::instance().find(
        CurrencyCol::CURRENCY_SYMBOL(symbol)
    );
    if (currency_a.empty())
        currency_n = get_id_data_n(currency_a[0].m_id);

    return currency_n;
}

std::set<mmDate> CurrencyModel::find_id_date_m(int64 currency_id)
{
    std::set<mmDate> date_m;
    for (const auto& account_d : AccountModel::instance().find(
        CurrencyCol::CURRENCYID(currency_id)
    )) {
        if (AccountModel::type_id(account_d) == NavigatorTypes::TYPE_ID_INVESTMENT) {
            for (const auto& stock_d : StockModel::instance().find(
                StockCol::HELDAT(account_d.m_id)
            )) {
                date_m.insert(stock_d.m_purchase_date);
            }
        }
        else {
            for (const auto& trx_d : TrxModel::instance().find_or(
                TrxCol::ACCOUNTID(account_d.m_id),
                TrxCol::TOACCOUNTID(account_d.m_id)
            )) {
                date_m.insert(trx_d.m_date());
            }
        }
    }
    return date_m;
}

const wxArrayString CurrencyModel::find_all_name_a()
{
    wxArrayString name_a;
    for (const Data& currency_d : find_all(Col::COL_ID_CURRENCYNAME))
        name_a.Add(currency_d.m_name);
    return name_a;
}

const wxArrayString CurrencyModel::find_all_symbol_a()
{
    wxArrayString symbol_a;
    for (const Data& currency_d : find_all(Col::COL_ID_CURRENCY_SYMBOL))
        symbol_a.Add(currency_d.m_symbol);
    return symbol_a;
}

const std::map<wxString, int64> CurrencyModel::find_all_name_id_m()
{
    std::map<wxString, int64> name_id_m;
    for (const Data& currency_n : find_all(
        Col::COL_ID_CURRENCYNAME
    )) {
        name_id_m[currency_n.m_name] = currency_n.m_id;
    }
    return name_id_m;
}

// convert value to a string with required precision. Currency is used only for percision
const wxString CurrencyModel::toStringNoFormatting(
    double value, const Data* currency_n, int precision
) {
    if (!currency_n)
        currency_n = get_base_data_n();
    if (precision < 0)
        precision = currency_n->precision();
    wxString value_str = wxString::FromCDouble(value, precision);
    value_str.Replace(".", currency_n->m_decimal_point);

    return value_str;
}

// convert value to a currency formatted string with required precision
const wxString CurrencyModel::toString(
    double value, const Data* currency_n, int precision
) {
    if (!currency_n)
        currency_n = get_base_data_n();
    if (precision < 0)
        precision = currency_n->precision();

    static wxString d; //default Locale Support Y/N

    if (s_locale.empty()) {
        s_locale = InfoModel::instance().getString("LOCALE", " ");
        if (s_locale.empty()) {
            s_locale = " ";
        }
    }

    if (s_use_locale.empty()) {
        s_use_locale = s_locale == " " ? "N" : "Y";
        if (s_use_locale == "Y") {
            try {
                fmt::format(std::locale(s_locale.c_str()), "{:L}", 123);
            }
            catch (...) {
                s_locale = " ";
                s_use_locale = "N";
            }
        }
    }

// Default locale. Windows requires only "en_US" (see #5852) but others require "en_US.UTF-8" (see #6074)
    std::string default_locale = "en_US";
#ifndef __WXMSW__
    default_locale = "en_US.UTF-8";
#endif

    if (d.empty()) {
        try {
            fmt::format(std::locale(default_locale), "{:L}", 123);
            d = "Y";
        }
        catch (...) {
            d = "N";
        }
    }

    auto l = (s_use_locale == "Y" ? std::locale(s_locale.c_str()) : (d == "Y" ? std::locale(default_locale) : std::locale()));
    std::string s;
    value += LIMIT; //to ignore the negative sign on values of zero #564

    switch (precision)
    {
    case (0):
        s = (d == "Y") ? fmt::format(l, "{:.0Lf}", value) : fmt::format("{:.0f}", value);
        break;
    case (1):
        s = (d == "Y") ? fmt::format(l, "{:.1Lf}", value) : fmt::format("{:.1f}", value);
        break;
    case (2):
        s = (d == "Y") ? fmt::format(l, "{:.2Lf}", value) : fmt::format("{:.2f}", value);
        break;
    case (3):
        s = (d == "Y") ? fmt::format(l, "{:.3Lf}", value) : fmt::format("{:.3f}", value);
        break;
    case (4):
        s = (d == "Y") ? fmt::format(l, "{:.4Lf}", value) : fmt::format("{:.4f}", value);
        break;
    case (5):
        s = (d == "Y") ? fmt::format(l, "{:.5Lf}", value) : fmt::format("{:.5f}", value);
        break;
    case (6):
        s = (d == "Y") ? fmt::format(l, "{:.6Lf}", value) : fmt::format("{:.6f}", value);
        break;
    case (7):
        s = (d == "Y") ? fmt::format(l, "{:.7Lf}", value) : fmt::format("{:.7f}", value);
        break;
    case (8):
        s = (d == "Y") ? fmt::format(l, "{:.8Lf}", value) : fmt::format("{:.8f}", value);
        break;
    case (9):
        s = (d == "Y") ? fmt::format(l, "{:.9Lf}", value) : fmt::format("{:.9f}", value);
        break;
    default:
        s = (d == "Y") ? fmt::format(l, "{:.4Lf}", value) : fmt::format("{:.4f}", value);
    }

#ifdef __WXMSW__
    //FIXME: #4191
    for (auto& i : s) {
        if (i < 0) i = ' ';
    }
#endif

    if (s_use_locale == "N") {
        wxString out(s);
        out.Replace(".", "\x05");
        out.Replace(",", "\t");
        out.Replace("\x05", currency_n->m_decimal_point);
        out.Replace("\t", currency_n->m_group_separator);
        return out;
    }

    //wxLogDebug("toString : (%.12f) %s -> %s", value,  fmt::format("{:f}", value), s);
    return wxString(s);
}

// Add prefix and suffix characters to string value
const wxString CurrencyModel::toCurrency(double value, const Data* currency_n, int precision)
{
    wxString d2s = toString(value, currency_n, precision);
    if (currency_n) {
        d2s.Prepend(currency_n->m_prefix_symbol);
        d2s.Append(currency_n->m_suffix_symbol);
    }
    return d2s;
}

// Reset currency string like 1.234,56 to standard number format like 1234.56
const wxString CurrencyModel::fromString2CLocale(const wxString &s, const Data* currency_n)
{
    if (s.empty()) return s;
    wxString str = s;

    wxRegEx pattern(R"([^0-9.,+-/*()])");
    pattern.ReplaceAll(&str, wxEmptyString);

    auto locale = InfoModel::instance().getString("LOCALE", "");

    if (locale.empty()) {
        if (!currency_n->m_group_separator.empty())
            str.Replace(currency_n->m_group_separator, wxEmptyString);
        if (!currency_n->m_decimal_point.empty())
            str.Replace(currency_n->m_decimal_point, ".");
    }
    else {
        wxString decimal = toString(1.0);
        wxRegEx pattern2(R"([^.,])");
        pattern2.ReplaceAll(&decimal, wxEmptyString);

        wxString thousand = toString(1000.0, nullptr, 0);
        wxRegEx pattern3(R"([0-9])");
        pattern3.ReplaceAll(&thousand, wxEmptyString);

        if (!thousand.empty())
            str.Replace(thousand, wxEmptyString);

        if (!decimal.empty()) str.Replace(decimal, ".");
    }

    wxLogDebug("fromString2CLocale : %s -> %s", s, str);
    return str;
}

bool CurrencyModel::fromString(wxString s, double& val, const Data* currency_n)
{
    bool done = true;
    const auto value = fromString2CLocale(s, currency_n);
    if (!value.ToCDouble(&val))
        done = false;

    return done;
}

// Resets all BASECONVRATE to 1
void CurrencyModel::resetBaseConversionRates()
{
    db_savepoint();
    for (auto currency_d : find_all()) {
        currency_d.m_base_conv_rate = 1;
        save_data_n(currency_d);
    }
    db_release_savepoint();
}

int CurrencyModel::precision(int64 account_id)
{
    const AccountData* account_n = AccountModel::instance().get_id_data_n(account_id);
    if (account_n) {
        return AccountModel::instance().get_data_currency_p(*account_n)->precision();
    }
    else return 2;
}
