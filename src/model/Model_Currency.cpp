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

const std::vector<std::pair<Model_Currency::CURRTYPE, wxString> > Model_Currency::CURRTYPE_CHOICES =
{
    { Model_Currency::FIAT, wxString(wxTRANSLATE("Fiat")) },
    { Model_Currency::CRYPTO, wxString(wxTRANSLATE("Crypto")) }
};


Model_Currency::Model_Currency()
: Model<DB_Table_CURRENCYFORMATS>()
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

wxArrayString Model_Currency::all_currency_types()
{
    static wxArrayString types;
    if (types.empty())
    {
        for (const auto& item : CURRTYPE_CHOICES)
            types.Add(item.second);
    }
    return types;
}

// Getter
Model_Currency::Data* Model_Currency::GetBaseCurrency()
{
    int currency_id = Option::instance().BaseCurrency();
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

Model_Currency::Data* Model_Currency::GetCurrencyRecord(const wxString& currency_symbol)
{
    Model_Currency::Data* record = this->get_one(CURRENCY_SYMBOL(currency_symbol));
    if (record) return record;

    Model_Currency::Data_Set items = Model_Currency::instance().find(CURRENCY_SYMBOL(currency_symbol));
    if (!items.empty())
        record = this->get(items[0].id(), this->db_);

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

/** Return the description of the choice type */
wxString Model_Currency::currtype_desc(const int CurrTypeEnum)
{
    const auto& item = CURRTYPE_CHOICES[CurrTypeEnum];
    wxString reftype_desc = item.second;
    return reftype_desc;
}

wxString Model_Currency::toCurrency(double value, const Data* currency, int precision)
{
    precision = precision >= 0 ? precision : (currency ? log10(currency->SCALE) : 2);
    wxString d2s = toString(value, currency, precision);
    if (currency)
    {
        d2s.Prepend(currency->PFX_SYMBOL);
        d2s.Append(currency->SFX_SYMBOL);
    }
    return d2s;
}

wxString Model_Currency::toStringNoFormatting(double value, const Data* currency, int precision)
{
    precision = (precision >= 0 ? precision : (currency ? log10(currency->SCALE) : 2));
    wxString s = wxString::FromCDouble(value, precision);

    // remove minus from -0 or -0.0000 that comes from printf("%.Nf")
    wxRegEx re ("^-(?=0(\\.0+)?$)", wxRE_ADVANCED);
    re.Replace(&s, wxEmptyString);
    return s;
}
wxString Model_Currency::toString(double value, const Data* currency, int precision)
{
    wxString sep = currency ? currency->DECIMAL_POINT : ".",
        s = Model_Currency::toStringNoFormatting(value, currency, precision);

    if (sep!=".") // protect decimal point
    {
        sep="\t";
        s.Replace(".", sep);
    }

    // add proper group separator
    wxRegEx re ("(\\d)(?=(\\d{3})+[" + sep + "$])", wxRE_ADVANCED);
    re.Replace(&s, "\\1" + (currency ? currency->GROUP_SEPARATOR : ","));

    if (sep!=".") s.Replace(sep, currency->DECIMAL_POINT);
    return s;
}

const wxString Model_Currency::fromString2Default(const wxString &s, const Data* currency)
{
    wxString str = s;
    const auto bc = Model_Currency::GetBaseCurrency();
    const Data* c = currency ? currency : bc;

    if(c)
    {
        if (!c->GROUP_SEPARATOR.empty())
            str.Replace(c->GROUP_SEPARATOR, wxEmptyString);
        if (!c->DECIMAL_POINT.empty())
            str.Replace(c->DECIMAL_POINT, wxS("."));

        wxRegEx pattern(R"([^0-9.+-/*()])");
        pattern.ReplaceAll(&str, wxEmptyString);
    }
    return str;
}

bool Model_Currency::fromString(wxString s, double& val, const Data* currency)
{
    return fromString2Default(s, currency).ToCDouble(&val);
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

bool Model_Currency::BoolOf(int value)
{
    return value > 0 ? true : false;
}