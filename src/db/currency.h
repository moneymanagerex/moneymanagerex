/*******************************************************
 Copyright (C) 2013 Stefano Giorgio

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
#include "entry_base.h"

/************************************************************************************
 Class TCurrencyEntry
 ***********************************************************************************/
class TCurrencyEntry : public TEntryBase
{
private:
    friend class TCurrencyList;    // Allows the class, access to private members.
    
    int Add(wxSQLite3Database* db);
    void Delete(wxSQLite3Database* db);
    void SetDatabaseValues(wxSQLite3Statement& st, int& db_index);

public:

    std::map<wxString, wxString> CURRENCIES_MAP();
    //TODO:  Complete the enum list
    enum CURRENCY_SYMBOL {
        SYMBOL_AFN, NAME_AFN,   //"AFN",  "Afghanistan Afghani",
        SYMBOL_ALL, NAME_ALL,   //"ALL",  "Albania Lek",
        SYMBOL_DZD, NAME_DZD,   //"DZD",  "Algeria Dinar",
        SYMBOL_AOA, NAME_AOA,   //"AOA",  "Angola Kwanza",
        SYMBOL_ARS, NAME_ARS,   //"ARS",  "Argentina Peso",
        SYMBOL_AMD, NAME_AMD,   //"AMD",  "Armenia Dram",
        SYMBOL_AWG, NAME_AWG,   //"AWG",  "Aruba Guilder",
        SYMBOL_AUD, NAME_AUD,   //"AUD",  "Australia Dollar",
        SYMBOL_AZN, NAME_AZN,   //"AZN",  "Azerbaijan New Manat",
        SYMBOL_BSD, NAME_BSD,   //"BSD",  "Bahamas Dollar",
        SYMBOL_BHD, NAME_BHD,   //"BHD",  "Bahrain Dinar",
        SYMBOL_BDT, NAME_BDT,   //"BDT",  "Bangladesh Taka",
        SYMBOL_BBD, NAME_BBD,   //"BBD",  "Barbados Dollar",
        SYMBOL_BYR, NAME_BYR,   //"BYR",  "Belarus Ruble",
        SYMBOL_BZD, NAME_BZD,   //"BZD",  "Belize Dollar",
        SYMBOL_BMD, NAME_BMD,   //"BMD",  "Bermuda Dollar",
        SYMBOL_BTN, NAME_BTN,   //"BTN",  "Bhutan Ngultrum",
        SYMBOL_BOB, NAME_BOB,   //"BOB",  "Bolivia Boliviano",
        SYMBOL_BAM, NAME_BAM,   //"BAM",  "Bosnia and Herzegovina Convertible Marka",
        SYMBOL_BWP, NAME_BWP,   //"BWP",  "Botswana Pula",
        SYMBOL_BRL, NAME_BRL,   //"BRL",  "Brazil Real",
        SYMBOL_BND, NAME_BND,   //"BND",  "Brunei Darussalam Dollar",
        SYMBOL_BGN, NAME_BGN,   //"BGN",  "Bulgaria Lev",
        SYMBOL_BIF, NAME_BIF,   //"BIF",  "Burundi Franc",
        SYMBOL_KHR, NAME_KHR,   //"KHR",  "Cambodia Riel",
        SYMBOL_CAD, NAME_CAD,   //"CAD",  "Canada Dollar",
        SYMBOL_CVE, NAME_CVE,   //"CVE",  "Cape Verde Escudo",
        SYMBOL_KYD, NAME_KYD,   //"KYD",  "Cayman Islands Dollar",
        SYMBOL_CLP, NAME_CLP,   //"CLP",  "Chile Peso",
    //"CNY",  "China Yuan Renminbi",
    //"COP",  "Colombia Peso",
    //"XOF",  "Communauté Financière Africaine (BCEAO) Franc",
    //"KMF",  "Comoros Franc",
    //"XPF",  "Comptoirs Français du Pacifique (CFP) Franc",
    //"CDF",  "Congo/Kinshasa Franc",
    //"CRC",  "Costa Rica Colon",
    //"HRK",  "Croatia Kuna",
    //"CUC",  "Cuba Convertible Peso",
    //"CUP",  "Cuba Peso",
    //"CZK",  "Czech Republic Koruna",
    //"DKK",  "Denmark Krone",
    //"DJF",  "Djibouti Franc",
    //"DOP",  "Dominican Republic Peso",
    //"XCD",  "East Caribbean Dollar",
    //"EGP",  "Egypt Pound",
    //"SVC",  "El Salvador Colon",
    //"ERN",  "Eritrea Nakfa",
    //"ETB",  "Ethiopia Birr",
    SYMBOL_EUR = 96, NAME_EUR,  //"EUR",  "Euro Member Countries",
    //"FKP",  "Falkland Islands (Malvinas) Pound",
    //"FJD",  "Fiji Dollar",
    //"GMD",  "Gambia Dalasi",
    //"GEL",  "Georgia Lari",
    //"GHS",  "Ghana Cedi",
    //"GIP",  "Gibraltar Pound",
    //"GTQ",  "Guatemala Quetzal",
    //"GGP",  "Guernsey Pound",
    //"GNF",  "Guinea Franc",
    //"GYD",  "Guyana Dollar",
    //"HTG",  "Haiti Gourde",
    //"HNL",  "Honduras Lempira",
    //"HKD",  "Hong Kong Dollar",
    //"HUF",  "Hungary Forint",
    //"ISK",  "Iceland Krona",
    //"INR",  "India Rupee",
    //"IDR",  "Indonesia Rupiah",
    //"XDR",  "International Monetary Fund (IMF) Special Drawing Rights",
    //"IRR",  "Iran Rial",
    //"IQD",  "Iraq Dinar",
    //"IMP",  "Isle of Man Pound",
    //"ILS",  "Israel Shekel",
    //"JMD",  "Jamaica Dollar",
    //"JPY",  "Japan Yen",
    //"JEP",  "Jersey Pound",
    //"JOD",  "Jordan Dinar",
    //"KZT",  "Kazakhstan Tenge",
    //"KES",  "Kenya Shilling",
    //"KPW",  "Korea (North) Won",
    //"KRW",  "Korea (South) Won",
    //"KWD",  "Kuwait Dinar",
    //"KGS",  "Kyrgyzstan Som",
    //"LAK",  "Laos Kip",
    //"LVL",  "Latvia Lat",
    //"LBP",  "Lebanon Pound",
    //"LSL",  "Lesotho Loti",
    //"LRD",  "Liberia Dollar",
    //"LYD",  "Libya Dinar",
    //"LTL",  "Lithuania Litas",
    //"MOP",  "Macau Pataca",
    //"MKD",  "Macedonia Denar",
    //"MGA",  "Madagascar Ariary",
    //"MWK",  "Malawi Kwacha",
    //"MYR",  "Malaysia Ringgit",
    //"MVR",  "Maldives (Maldive Islands) Rufiyaa",
    //"MRO",  "Mauritania Ouguiya",
    //"MUR",  "Mauritius Rupee",
    //"MXN",  "Mexico Peso",
    //"MDL",  "Moldova Leu",
    //"MNT",  "Mongolia Tughrik",
    //"MAD",  "Morocco Dirham",
    //"MZN",  "Mozambique Metical",
    //"MMK",  "Myanmar (Burma) Kyat",
    //"NAD",  "Namibia Dollar",
    //"NPR",  "Nepal Rupee",
    //"ANG",  "Netherlands Antilles Guilder",
    //"NZD",  "New Zealand Dollar",
    //"NIO",  "Nicaragua Cordoba",
    //"NGN",  "Nigeria Naira",
    //"NOK",  "Norway Krone",
    //"OMR",  "Oman Rial",
    //"PKR",  "Pakistan Rupee",
    //"PAB",  "Panama Balboa",
    //"PGK",  "Papua New Guinea Kina",
    //"PYG",  "Paraguay Guarani",
    //"PEN",  "Peru Nuevo Sol",
    //"PHP",  "Philippines Peso",
    //"PLN",  "Poland Zloty",
    //"QAR",  "Qatar Riyal",
    //"RON",  "Romania New Leu",
    //"RUB",  "Russia Ruble",
    //"RWF",  "Rwanda Franc",
    //"SHP",  "Saint Helena Pound",
    //"WST",  "Samoa Tala",
    //"STD",  "São Tomé and Príncipe Dobra",
    //"SAR",  "Saudi Arabia Riyal",
    //"RSD",  "Serbia Dinar",
    //"SCR",  "Seychelles Rupee",
    //"SLL",  "Sierra Leone Leone",
    //"SGD",  "Singapore Dollar",
    //"SBD",  "Solomon Islands Dollar",
    //"SOS",  "Somalia Shilling",
    //"ZAR",  "South Africa Rand",
    //"LKR",  "Sri Lanka Rupee",
    //"SDG",  "Sudan Pound",
    //"SRD",  "Suriname Dollar",
    //"SZL",  "Swaziland Lilangeni",
    //"SEK",  "Sweden Krona",
    //"CHF",  "Switzerland Franc",
    //"SYP",  "Syria Pound",
    //"TWD",  "Taiwan New Dollar",
    //"TJS",  "Tajikistan Somoni",
    //"TZS",  "Tanzania Shilling",
    //"THB",  "Thailand Baht",
    //"TOP",  "Tonga Pa'anga",
    //"TTD",  "Trinidad and Tobago Dollar",
    //"TND",  "Tunisia Dinar",
    //"TRY",  "Turkey Lira",
    //"TMT",  "Turkmenistan Manat",
    //"TVD",  "Tuvalu Dollar",
    //"UGX",  "Uganda Shilling",
    //"UAH",  "Ukraine Hryvna",
        SYMBOL_AED = 302, NAME_AED, //"AED",  "United Arab Emirates Dirham",
        SYMBOL_GBP, NAME_GBP,       //"GBP",  "United Kingdom Pound",
        SYMBOL_USD, NAME_USD,       //"USD",  "United States Dollar",
    //"UYU",  "Uruguay Peso",
    //"UZS",  "Uzbekistan Som",
    //"VUV",  "Vanuatu Vatu",
    //"VEF",  "Venezuela Bolivar",
    //"VND",  "Viet Nam Dong",
    //"YER",  "Yemen Rial",
    //"ZMK",  "Zambia Kwacha",
    //"ZWD",  "Zimbabwe Dollar"
    };

    wxString name_;
    wxString pfxSymbol_;
    wxString sfxSymbol_;
    wxString dec_;
    wxString grp_;
    wxString unit_;
    wxString cent_;
    int scaleDl_;
    double baseConv_;
    wxChar decChar_;
    wxChar grpChar_;
    wxString currencySymbol_;

    // Constructor used when loading Currency from the database
    TCurrencyEntry(wxSQLite3ResultSet& q1);
    
    // Constructor for creating a new Currency entry
    TCurrencyEntry();
    // Update the entry in the database
    void Update(wxSQLite3Database* db);

    void SetCurrencySettings();
};

/************************************************************************************
 Class TCurrencyList
 ***********************************************************************************/
class TCurrencyList : public TListBase
{
private:
    int basecurrency_id_;

    void LoadEntries(bool load_entries = true);
    // delete all the objects in the list and clear the list.
    void DestroyEntryList();

public:
    std::vector<TCurrencyEntry*> entrylist_;

    TCurrencyList(wxSQLite3Database* db);
    ~TCurrencyList();

    // Allows specialised loads by providing the required SQL statement
    void LoadEntriesUsing(const wxString& sql_statement);

    int AddEntry(TCurrencyEntry* pCurrencyEntry);
    void SetBaseCurrency(int currency_id);

    // Note: No checking is done for usage in other tables.
    void DeleteEntry(int currency_id);
    // Note: No checking is done for usage in other tables.
    void DeleteEntry(const wxString& name, bool is_symbol = false);

    TCurrencyEntry* GetEntryPtr(const wxString& name, bool is_symbol = false);
    TCurrencyEntry* GetEntryPtr(int currency_id);
    TCurrencyEntry* GetIndexedEntryPtr(int index);

    int GetCurrencyId(const wxString& name, bool is_symbol = false);
    wxString GetCurrencyName(int currency_id);
    bool CurrencyExists(const wxString& name, bool is_symbol = false);
};
