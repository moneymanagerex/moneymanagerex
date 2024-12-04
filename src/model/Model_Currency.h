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

#ifndef MODEL_CURRENCY_H
#define MODEL_CURRENCY_H

#include "Model.h"
#include "db/DB_Table_Currencyformats_V1.h"
#include "Model_Infotable.h" // detect base currency setting BASECURRENCYID
#include <map>

class Model_Currency : public Model<DB_Table_CURRENCYFORMATS_V1>
{
public:
    using Model<DB_Table_CURRENCYFORMATS_V1>::remove;
    Model_Currency();
    ~Model_Currency();

public:
    /**
    Initialize the global Model_Currency table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for Model_Currency table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Currency& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for Model_Currency table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Currency& instance();

public:
    enum TYPE_ID
    {
        TYPE_ID_FIAT = 0,
        TYPE_ID_CRYPTO
    };
    static wxArrayString TYPE_STR;
    static const wxString TYPE_STR_FIAT;
    static const wxString TYPE_STR_CRYPTO;

private:
    static const std::vector<std::pair<TYPE_ID, wxString> > TYPE_CHOICES;
    static wxArrayString type_str_all();

public:
    static TYPE_ID type_id(const Data* r);
    static TYPE_ID type_id(const Data& r);
    static DB_Table_CURRENCYFORMATS_V1::CURRENCY_TYPE CURRENCY_TYPE(TYPE_ID currencytype, OP op = EQUAL);

    const wxArrayString all_currency_names();
    const std::map<wxString, int64>  all_currency();
    const wxArrayString all_currency_symbols();

    /** Return the Data record of the base currency.*/
    static Data* GetBaseCurrency();
    static bool GetBaseCurrencySymbol(wxString& base_currency_symbol);

    /** Resets all BASECONVRATE to 1 */
    static void ResetBaseConversionRates();

    /** Return the currency Data record for the given symbol */
    Model_Currency::Data* GetCurrencyRecord(const wxString& currency_symbol);

    /**
    * Remove the Data record from memory and the database.
    * Delete also all currency history
    */
    bool remove(int64 id);

    static std::map<wxDateTime,int> DateUsed(int64 CurrencyID);

    /** Add prefix and suffix characters to string value */
    static const wxString toCurrency(double value, const Data* currency = GetBaseCurrency(), int precision = -1);
 
    /** convert value to a string with required precision. Currency is used only for percision */
    static const wxString toStringNoFormatting(double value, const Data* currency = GetBaseCurrency(), int precision = -1);
    /** convert value to a currency formatted string with required precision */
    static const wxString toString(double value, const Data* currency = GetBaseCurrency(), int precision = -1);
    /** Reset currency string like 1.234,56 to standard number format like 1234.56 */
    static const wxString fromString2CLocale(const wxString &s, const Data* currency = Model_Currency::GetBaseCurrency());
    static bool fromString(wxString s, double& val, const Data* currency = GetBaseCurrency());
    static int precision(const Data* r);
    static int precision(const Data& r);
    static int precision(int64 account_id);
};
#endif // 
