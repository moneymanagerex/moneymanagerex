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
#include "PreferencesModel.h"
#include "StockModel.h"
#include "TransactionModel.h"

#include "uicontrols/navigatortypes.h"

constexpr auto LIMIT = 1e-10;
static wxString s_locale;
static wxString s_use_locale;

ChoicesName CurrencyModel::TYPE_CHOICES = ChoicesName({
    { TYPE_ID_FIAT,   _n("Fiat") },
    { TYPE_ID_CRYPTO, _n("Crypto") }
});

const wxString CurrencyModel::TYPE_NAME_FIAT   = type_name(TYPE_ID_FIAT);
const wxString CurrencyModel::TYPE_NAME_CRYPTO = type_name(TYPE_ID_CRYPTO);

CurrencyModel::CurrencyModel()
    : Model<CurrencyTable>()
{
}

CurrencyModel::~CurrencyModel()
{
}

/**
* Initialize the global CurrencyModel table.
* Reset the CurrencyModel table or create the table if it does not exist.
*/
CurrencyModel& CurrencyModel::instance(wxSQLite3Database* db)
{
    CurrencyModel& ins = Singleton<CurrencyModel>::instance();
    ins.m_db = db;
    ins.ensure_table();
    ins.destroy_cache();
    ins.preload();
    s_locale = wxEmptyString;
    s_use_locale = wxEmptyString;
    return ins;
}

/** Return the static instance of CurrencyModel table */
CurrencyModel& CurrencyModel::instance()
{
    return Singleton<CurrencyModel>::instance();
}

CurrencyTable::CURRENCY_TYPE CurrencyModel::CURRENCY_TYPE(OP op, TYPE_ID currencytype)
{
    return CurrencyTable::CURRENCY_TYPE(op, CurrencyModel::type_name(currencytype));
}

const wxArrayString CurrencyModel::all_currency_names()
{
    wxArrayString c;
    for (const auto&i : get_all(COL_CURRENCYNAME))
        c.Add(i.CURRENCYNAME);
    return c;
}


const std::map<wxString, int64> CurrencyModel::all_currency()
{
    std::map<wxString, int64> currencies;
    for (const auto& curr : this->get_all(COL_CURRENCYNAME))
    {
        currencies[curr.CURRENCYNAME] = curr.CURRENCYID;
    }
    return currencies;
}

const wxArrayString CurrencyModel::all_currency_symbols()
{
    wxArrayString c;
    for (const auto&i : get_all(COL_CURRENCY_SYMBOL))
        c.Add(i.CURRENCY_SYMBOL);
    return c;
}

// Getter
CurrencyModel::Data* CurrencyModel::GetBaseCurrency()
{
    int64 currency_id = PreferencesModel::instance().getBaseCurrencyID();
    CurrencyModel::Data* currency = CurrencyModel::instance().cache_id(currency_id);
    return currency;
}

bool CurrencyModel::GetBaseCurrencySymbol(wxString& base_currency_symbol)
{
    const auto base_currency = GetBaseCurrency();
    if (base_currency)
    {
        base_currency_symbol = base_currency->CURRENCY_SYMBOL;
        return true;
    }
    return false;
}

void CurrencyModel::ResetBaseConversionRates()
{
    CurrencyModel::instance().Savepoint();
    for (auto currency : CurrencyModel::instance().get_all())
    {
        currency.BASECONVRATE = 1;
        CurrencyModel::instance().save(&currency);
    }
    CurrencyModel::instance().ReleaseSavepoint();
}

CurrencyModel::Data* CurrencyModel::GetCurrencyRecord(const wxString& currency_symbol)
{
    CurrencyModel::Data* record = this->search_cache(CURRENCY_SYMBOL(currency_symbol));
    if (record) return record;

    CurrencyModel::Data_Set items = CurrencyModel::instance().find(CURRENCY_SYMBOL(currency_symbol));
    if (items.empty()) record = this->cache_id(items[0].id());

    return record;
}

std::map<wxDateTime, int> CurrencyModel::DateUsed(int64 CurrencyID)
{
    wxDateTime dt;
    std::map<wxDateTime, int> datesList;
    const auto &accounts = AccountModel::instance().find(CURRENCYID(CurrencyID));
    for (const auto &account : accounts)
    {
        if (AccountModel::type_id(account) == NavigatorTypes::TYPE_ID_INVESTMENT)
        {
            for (const auto& tran : StockModel::instance().find(StockModel::HELDAT(account.ACCOUNTID)))
            {
                dt.ParseDate(tran.PURCHASEDATE);
                datesList[dt] = 1;
            }
        }
        else
        {
            for (const auto& tran : TransactionModel::instance()
                .find_or(TransactionModel::ACCOUNTID(account.ACCOUNTID)
                , TransactionModel::TOACCOUNTID(account.ACCOUNTID)))
            {
                dt.ParseDate(tran.TRANSDATE);
                datesList[dt] = 1;
            }
        }
    }
    return datesList;
}
/**
* Remove the Data record from memory and the database.
* Delete also all currency history
*/
bool CurrencyModel::remove(int64 id)
{
    this->Savepoint();
    for (const auto& r : CurrencyHistoryModel::instance().find(CurrencyHistoryModel::CURRENCYID(id)))
        CurrencyHistoryModel::instance().remove(r.id());
    this->ReleaseSavepoint();
    return this->remove(id);
}

const wxString CurrencyModel::toCurrency(double value, const Data* currency, int precision)
{
    wxString d2s = toString(value, currency, precision);
    if (currency) {
        d2s.Prepend(currency->PFX_SYMBOL);
        d2s.Append(currency->SFX_SYMBOL);
    }
    return d2s;
}

const wxString CurrencyModel::toStringNoFormatting(double value, const Data* currency, int precision)
{
    const Data* curr = currency ? currency : GetBaseCurrency();
    precision = (precision >= 0) ? precision : log10(curr->SCALE.GetValue());
    wxString s = wxString::FromCDouble(value, precision);
    s.Replace(".", curr->DECIMAL_POINT);

    return s;
}

const wxString CurrencyModel::toString(double value, const Data* currency, int precision)
{
    static wxString d; //default Locale Support Y/N

    if (s_locale.empty()) {
        s_locale = InfoModel::instance().getString("LOCALE", " ");
        if (s_locale.empty()) {
            s_locale = " ";
        }
    }

    if (s_use_locale.empty()) {
        s_use_locale = s_locale == " " ? "N" : "Y";
        if (s_use_locale == "Y")
        {
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

    if (d.empty())
    {
        try {
            fmt::format(std::locale(default_locale), "{:L}", 123);
            d = "Y";
        }
        catch (...) {
            d = "N";
        }
    }

    if (precision < 0) {
        precision = log10(currency ? currency->SCALE.GetValue() : GetBaseCurrency()->SCALE.GetValue());
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

    if (s_use_locale == "N")
    {
        wxString out(s);
        out.Replace(".", "\x05");
        out.Replace(",", "\t");
        out.Replace("\x05", currency ? currency->DECIMAL_POINT : GetBaseCurrency()->DECIMAL_POINT);
        out.Replace("\t", currency ? currency->GROUP_SEPARATOR : GetBaseCurrency()->GROUP_SEPARATOR);
        return out;
    }

    //wxLogDebug("toString : (%.12f) %s -> %s", value,  fmt::format("{:f}", value), s);
    return wxString(s);
}

const wxString CurrencyModel::fromString2CLocale(const wxString &s, const Data* currency)
{
    if (s.empty()) return s;
    wxString str = s;

    wxRegEx pattern(R"([^0-9.,+-/*()])");
    pattern.ReplaceAll(&str, wxEmptyString);

    auto locale = InfoModel::instance().getString("LOCALE", "");

    if (locale.empty())
    {
        if (!currency->GROUP_SEPARATOR.empty())
            str.Replace(currency->GROUP_SEPARATOR, wxEmptyString);
        if (!currency->DECIMAL_POINT.empty())
            str.Replace(currency->DECIMAL_POINT, ".");
    }
    else
    {
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

bool CurrencyModel::fromString(wxString s, double& val, const Data* currency)
{
    bool done = true;
    const auto value = fromString2CLocale(s, currency);
    if (!value.ToCDouble(&val))
        done = false;

    return done;
}

int CurrencyModel::precision(const Data* r)
{
    return static_cast<int>(log10(static_cast<double>(r->SCALE.GetValue())));
}

int CurrencyModel::precision(const Data& r)
{
    return precision(&r);
}

int CurrencyModel::precision(int64 account_id)
{
    const AccountModel::Data* trans_account = AccountModel::instance().cache_id(account_id);
    if (account_id > 0)
    {
        return precision(AccountModel::currency(trans_account));
    }
    else return 2;
}
