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

#ifndef MODEL_CURRENCY_H
#define MODEL_CURRENCY_H

#include "Model.h"
#include "Table_Currencyformats.h"
#include <map>

class Model_Currency : public Model<DB_Table_CURRENCYFORMATS>
{
public:
    using Model<DB_Table_CURRENCYFORMATS>::remove;

    enum CURRTYPE { FIAT = 0, CRYPTO};

    static const std::vector<std::pair<CURRTYPE, wxString> > CURRTYPE_CHOICES;

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
    wxArrayString all_currency_names();
    wxArrayString all_currency_symbols();
    wxArrayString all_currency_types();

    /** Return the Data record of the base currency.*/
    static Data* GetBaseCurrency();
    static bool GetBaseCurrencySymbol(wxString& base_currency_symbol);

    /** Return the currency Data record for the given symbol */
    Model_Currency::Data* GetCurrencyRecord(const wxString& currency_symbol);

    /**
    * Remove the Data record from memory and the database.
    * Delete also all currency history
    */
    bool remove(int id);

    static std::map<wxDateTime,int> DateUsed(int CurrencyID);

    /** Return the description of the choice type */
    static wxString currtype_desc(const int CurrTypeEnum);

    /** Add prefix and suffix characters to string value */
    static wxString toCurrency(double value, const Data* currency = GetBaseCurrency(), int precision = -1);

    /** convert value to a string with required precision. Currency is used only for precision */
    static wxString toStringNoFormatting(double value, const Data* currency = GetBaseCurrency(), int precision = -1);
    /** convert value to a currency formatted string with required precision */
    static wxString toString(double value, const Data* currency = GetBaseCurrency(), int precision = -1);
    /** Reset currency string like 1.234,56 to standard C locale number format like 1234.56 */
    static const wxString fromString2Default(const wxString &s, const Data* currency = Model_Currency::GetBaseCurrency());
    static bool fromString(wxString s, double& val, const Data* currency = GetBaseCurrency());
    static int precision(const Data* r);
    static int precision(const Data& r);
    static int precision(int account_id);
    static bool BoolOf(int value);
};
#endif // 
