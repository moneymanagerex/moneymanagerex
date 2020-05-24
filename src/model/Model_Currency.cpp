/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)

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
#include <wx/numformatter.h>

const double ROUNDING_ERROR_f32 = 0.000001;

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
    return ins;
}

/** Return the static instance of Model_Currency table */
Model_Currency& Model_Currency::instance()
{
    return Singleton<Model_Currency>::instance();
}

wxArrayString Model_Currency::all_currency_names()
{
    wxArrayString c;
    for (const auto&i : all(COL_CURRENCYNAME))
        c.Add(i.CURRENCYNAME);
    return c;
}

wxArrayString Model_Currency::all_currency_symbols()
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
    std::map<wxDateTime, int> DatesList;
    const auto &accounts = Model_Account::instance().find(CURRENCYID(CurrencyID));
    for (const auto &account : accounts)
    {
        if (Model_Account::type(account) == Model_Account::TYPE::INVESTMENT)
        {
            for (const auto trans : Model_Stock::instance().find(Model_Stock::HELDAT(account.ACCOUNTID)))
            {
                dt.ParseDate(trans.PURCHASEDATE);
                DatesList[dt] = 1;
            }
        }
        else
        {
            for (const auto trans : Model_Checking::instance().find(Model_Checking::ACCOUNTID(account.ACCOUNTID)))
            {
                dt.ParseDate(trans.TRANSDATE);
                DatesList[dt] = 1;
            }
        }
    }
    return DatesList;
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

wxString Model_Currency::toCurrency(double value, const Data* currency, int precision)
{
    precision = precision >= 0 ? precision : (currency ? log10(currency->SCALE) : 2);
    wxString d2s = toString(value, currency, precision);
    if (currency) {
        d2s.Prepend(currency->PFX_SYMBOL);
        d2s.Append(currency->SFX_SYMBOL);
    }
    return d2s;
}

wxString Model_Currency::os_group_separator()
{
    wxString sys_thousand_separator = " ";
    wxChar sep = ' ';
    if (wxNumberFormatter::GetThousandsSeparatorIfUsed(&sep)) {
        sys_thousand_separator = wxString::Format("%c", sep);
    }
    return sys_thousand_separator;
}

wxString Model_Currency::toStringNoFormatting(double value, const Data* currency, int precision)
{
    precision = (precision >= 0 ? precision : (currency ? log10(currency->SCALE) : 2));
    int style = wxNumberFormatter::Style_None;
    wxString s = wxNumberFormatter::ToString(value, precision, style);

    if (value >= -ROUNDING_ERROR_f32 && s.Mid(0, 1) == "-") {
        s = s.Mid(1);
    }

    return s;
}
wxString Model_Currency::toString(double value, const Data* currency, int precision)
{
    precision = (precision >= 0 ? precision : (currency ? log10(currency->SCALE) : 2));
    int style = wxNumberFormatter::Style_WithThousandsSep;
    wxString s = wxNumberFormatter::ToString(value, precision, style);

    if (currency)
    {
        s.Replace(os_group_separator(), "\t");
        s.Replace(wxNumberFormatter::GetDecimalSeparator(), "\x05");
        s.Replace("\t", currency->GROUP_SEPARATOR);
        s.Replace("\x05", currency->DECIMAL_POINT);
    }

    if (value >= -ROUNDING_ERROR_f32 && s.Mid(0, 1) == "-") {
        s = s.Mid(1);
    }

    return s;
}

const wxString Model_Currency::fromString2Lua(const wxString &s, const Data* currency)
{
    auto str = fromString2Default(s, currency);
    str.Replace(wxNumberFormatter::GetDecimalSeparator(), ".");
    return str;
}

const wxString Model_Currency::fromString2Default(const wxString &s, const Data* currency)
{
    if (s.empty()) return s;
    wxString str = s;
    const auto bc = GetBaseCurrency();
    const Data* c = currency ? currency : bc;

    if (c)
    {
        if (!c->GROUP_SEPARATOR.empty())
            str.Replace(c->GROUP_SEPARATOR, wxEmptyString);
        if (!c->DECIMAL_POINT.empty())
            str.Replace(c->DECIMAL_POINT, wxNumberFormatter::GetDecimalSeparator());

        wxRegEx pattern(R"([^0-9.,+-/*()])");
        pattern.ReplaceAll(&str, wxEmptyString);
    }
    wxLogDebug("%s -> %s", s, str);
    return str;
}

bool Model_Currency::fromString(wxString s, double& val, const Data* currency)
{
    bool done = true;
    const auto value = fromString2Default(s, currency);
    if (!wxNumberFormatter::FromString(value, &val))
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

