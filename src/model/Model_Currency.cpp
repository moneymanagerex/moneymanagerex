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

#include "Model_Currency.h"
#include "Model_CurrencyHistory.h"
#include "Model_Account.h"
#include "Model_Checking.h"
#include "Model_Stock.h"
#include "option.h"
#include "util.h"

#include <fmt/core.h>
#include <fmt/format.h>

constexpr auto LIMIT = 1e-10;
static wxString s_locale;
static wxString s_use_locale;

const std::vector<std::pair<Model_Currency::TYPE_ID, wxString> > Model_Currency::TYPE_CHOICES =
{
    { Model_Currency::TYPE_ID_FIAT,   wxString(wxTRANSLATE("Fiat")) },
    { Model_Currency::TYPE_ID_CRYPTO, wxString(wxTRANSLATE("Crypto")) }
};

wxArrayString Model_Currency::TYPE_STR = type_str_all();
const wxString Model_Currency::TYPE_STR_FIAT   = TYPE_STR[TYPE_ID_FIAT];
const wxString Model_Currency::TYPE_STR_CRYPTO = TYPE_STR[TYPE_ID_CRYPTO];

Model_Currency::Model_Currency()
    : Model<DB_Table_CURRENCYFORMATS_V1>()
{
}

Model_Currency::~Model_Currency()
{
}

/**
* Initialize the global Model_Currency table.
* Reset the Model_Currency table or create the table if it does not exist.
*/
Model_Currency& Model_Currency::instance(wxSQLite3Database* db)
{
    Model_Currency& ins = Singleton<Model_Currency>::instance();
    ins.db_ = db;
    ins.ensure(db);
    ins.destroy_cache();
    ins.preload();
    s_locale = wxEmptyString;
    s_use_locale = wxEmptyString;
    return ins;
}

/** Return the static instance of Model_Currency table */
Model_Currency& Model_Currency::instance()
{
    return Singleton<Model_Currency>::instance();
}

wxArrayString Model_Currency::type_str_all()
{
    wxArrayString types;
    int i = 0;
    for (const auto& item : TYPE_CHOICES)
    {
        wxASSERT_MSG(item.first == i++, "Wrong order in Model_Currency::TYPE_CHOICES");
        types.Add(item.second);
    }
    return types;
}

Model_Currency::TYPE_ID Model_Currency::type_id(const Data* r)
{
    for (const auto &entry : TYPE_CHOICES)
    {
        if (r->CURRENCY_TYPE.CmpNoCase(entry.second) == 0) return entry.first;
    }
    return TYPE_ID_FIAT;
}

Model_Currency::TYPE_ID Model_Currency::type_id(const Data& r)
{
    return type_id(&r);
}

DB_Table_CURRENCYFORMATS_V1::CURRENCY_TYPE Model_Currency::CURRENCY_TYPE(TYPE_ID currencytype, OP op)
{
    return DB_Table_CURRENCYFORMATS_V1::CURRENCY_TYPE(Model_Currency::TYPE_STR[currencytype], op);
}

const wxArrayString Model_Currency::all_currency_names()
{
    wxArrayString c;
    for (const auto&i : all(COL_CURRENCYNAME))
        c.Add(i.CURRENCYNAME);
    return c;
}


const std::map<wxString, int> Model_Currency::all_currency()
{
    std::map<wxString, int> currencies;
    for (const auto& curr : this->all(COL_CURRENCYNAME))
    {
        currencies[curr.CURRENCYNAME] = curr.CURRENCYID;
    }
    return currencies;
}

const wxArrayString Model_Currency::all_currency_symbols()
{
    wxArrayString c;
    for (const auto&i : all(COL_CURRENCY_SYMBOL))
        c.Add(i.CURRENCY_SYMBOL);
    return c;
}

// Getter
Model_Currency::Data* Model_Currency::GetBaseCurrency()
{
    int currency_id = Option::instance().getBaseCurrencyID();
    Model_Currency::Data* currency = Model_Currency::instance().get(currency_id);
    return currency;
}

bool Model_Currency::GetBaseCurrencySymbol(wxString& base_currency_symbol)
{
    const auto base_currency = GetBaseCurrency();
    if (base_currency)
    {
        base_currency_symbol = base_currency->CURRENCY_SYMBOL;
        return true;
    }
    return false;
}

void Model_Currency::ResetBaseConversionRates()
{
    Model_Currency::instance().Savepoint();
    for (auto currency : Model_Currency::instance().all())
    {
        currency.BASECONVRATE = 1;
        Model_Currency::instance().save(&currency);
    }
    Model_Currency::instance().ReleaseSavepoint();
}

Model_Currency::Data* Model_Currency::GetCurrencyRecord(const wxString& currency_symbol)
{
    Model_Currency::Data* record = this->get_one(CURRENCY_SYMBOL(currency_symbol));
    if (record) return record;

    Model_Currency::Data_Set items = Model_Currency::instance().find(CURRENCY_SYMBOL(currency_symbol));
    if (items.empty()) record = this->get(items[0].id(), this->db_);

    return record;
}

std::map<wxDateTime, int> Model_Currency::DateUsed(int CurrencyID)
{
    wxDateTime dt;
    std::map<wxDateTime, int> datesList;
    const auto &accounts = Model_Account::instance().find(CURRENCYID(CurrencyID));
    for (const auto &account : accounts)
    {
        if (Model_Account::type_id(account) == Model_Account::TYPE_ID_INVESTMENT)
        {
            for (const auto trans : Model_Stock::instance().find(Model_Stock::HELDAT(account.ACCOUNTID)))
            {
                dt.ParseDate(trans.PURCHASEDATE);
                datesList[dt] = 1;
            }
        }
        else
        {
            for (const auto& trans : Model_Checking::instance()
                .find_or(Model_Checking::ACCOUNTID(account.ACCOUNTID)
                , Model_Checking::TOACCOUNTID(account.ACCOUNTID)))
            {
                dt.ParseDate(trans.TRANSDATE);
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
bool Model_Currency::remove(int id)
{
    this->Savepoint();
    for (const auto& r : Model_CurrencyHistory::instance().find(Model_CurrencyHistory::CURRENCYID(id)))
        Model_CurrencyHistory::instance().remove(r.id());
    this->ReleaseSavepoint();
    return this->remove(id, db_);
}

const wxString Model_Currency::toCurrency(double value, const Data* currency, int precision)
{
    wxString d2s = toString(value, currency, precision);
    if (currency) {
        d2s.Prepend(currency->PFX_SYMBOL);
        d2s.Append(currency->SFX_SYMBOL);
    }
    return d2s;
}

const wxString Model_Currency::toStringNoFormatting(double value, const Data* currency, int precision)
{
    const Data* curr = currency ? currency : GetBaseCurrency();
    precision = (precision >= 0) ? precision : log10(curr->SCALE);
    wxString s = wxString::FromCDouble(value, precision);
    s.Replace(".", curr->DECIMAL_POINT);

    return s;
}

const wxString Model_Currency::toString(double value, const Data* currency, int precision)
{
    static wxString d; //default Locale Support Y/N

    if (s_locale.empty()) {
        s_locale = Model_Infotable::instance().GetStringInfo("LOCALE", " ");
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
        precision = log10(currency ? currency->SCALE : GetBaseCurrency()->SCALE);
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

const wxString Model_Currency::fromString2CLocale(const wxString &s, const Data* currency)
{
    if (s.empty()) return s;
    wxString str = s;

    wxRegEx pattern(R"([^0-9.,+-/*()])");
    pattern.ReplaceAll(&str, wxEmptyString);

    auto locale = Model_Infotable::instance().GetStringInfo("LOCALE", "");

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

bool Model_Currency::fromString(wxString s, double& val, const Data* currency)
{
    bool done = true;
    const auto value = fromString2CLocale(s, currency);
    if (!value.ToCDouble(&val))
        done = false;

    return done;
}

int Model_Currency::precision(const Data* r)
{
    return static_cast<int>(log10(static_cast<double>(r->SCALE)));
}

int Model_Currency::precision(const Data& r)
{
    return precision(&r);
}

int Model_Currency::precision(int account_id)
{
    const Model_Account::Data* trans_account = Model_Account::instance().get(account_id);
    if (account_id > 0)
    {
        return precision(Model_Account::currency(trans_account));
    }
    else return 2;
}

