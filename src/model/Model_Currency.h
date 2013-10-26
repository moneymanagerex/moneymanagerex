/*******************************************************
 Copyright (C) 2013 Guan Lisheng

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
#include <wx/numformatter.h>
#include <tuple>

class Model_Currency : public Model, public DB_Table_CURRENCYFORMATS_V1
{
    using DB_Table_CURRENCYFORMATS_V1::all;
    using DB_Table_CURRENCYFORMATS_V1::get;
    using DB_Table_CURRENCYFORMATS_V1::remove;
public:
    Model_Currency(): Model(), DB_Table_CURRENCYFORMATS_V1() {};
    ~Model_Currency() {};

public:
    static Model_Currency& instance()
    {
        return Singleton<Model_Currency>::instance();
    }
    static Model_Currency& instance(wxSQLite3Database* db)
    {
        Model_Currency& ins = Singleton<Model_Currency>::instance();
        ins.db_ = db;
        ins.all();
        return ins;
    }

public:
    Data_Set all(COLUMN col = COLUMN(0), bool asc = true)
    {
        if (!this->exists(this->db_))
        {
            this->ensure(this->db_);
            std::vector<std::tuple<wxString, wxString, wxString, wxString, wxString, wxString, int, int> > all_currency
            {
                std::make_tuple("USD", "United States dollar"  , L"$"   , L""     , L"", L"", 100,1),
                std::make_tuple("EUR", "European euro"         , L"€"   , L""     , L"", L"", 100,1),
                std::make_tuple("GBP", "UK Pound"              , L"£"   , ""      , "Pound", "Pence", 100, 1),
                std::make_tuple("RUB", "Russian Ruble"         , ""     , L"р"    , "руб.", "коп.", 100, 1),
                std::make_tuple("UAH", "Ukrainian hryvnia"     , L"₴"   , L""     , L"", L"", 100, 1),
                std::make_tuple("AFN", "Afghan afghani"        , L"؋"   , L""     , L"" , L"pul", 100 , 1),
                std::make_tuple("ALL", "Albanian lek"          , L""    , L"L"     , L"", L"", 1, 1),
                std::make_tuple("DZD", "Algerian dinar"        , L"دج"  , L""     , L"", L"", 100, 1),
                std::make_tuple("AOA", "Angolan kwanza"        , L""    , L"Kz"   , L"", L"Céntimo", 100, 1),
                std::make_tuple("XCD", "East Caribbean dollar" , L"EC$", L"", L"", L"", 100, 1),
                std::make_tuple("ARS", "Argentine peso"        , L"", L"", L"", L"", 100, 1),
                std::make_tuple("AMD", "Armenian dram"         , L"", L"", L"", L"", 1, 1),
                std::make_tuple("AWG", "Aruban florin"         , L"ƒ", L"", L"", L"", 100, 1),
                std::make_tuple("AUD", "Australian dollar"     , L"$", L"", L"", L"", 100, 1),
                std::make_tuple("AZN", "Azerbaijani manat"     , L"", L"", L"", L"", 100, 1),
                std::make_tuple("BSD", "Bahamian dollar"       , L"B$", L"", L"", L"", 100, 1),
                std::make_tuple("BHD", "Bahraini dinar"        , L"", L"", L"", L"", 100, 1),
                std::make_tuple("BDT", "Bangladeshi taka"      , L"", L"", L"", L"", 100, 1),
                std::make_tuple("BBD", "Barbadian dollar"      , L"Bds$", L"", L"", L"", 100, 1),
                std::make_tuple("BYR", "Belarusian ruble"      , L"Br", L"", L"", L"",1,1),
                std::make_tuple("BZD", "Belize dollar"         , L"BZ$", L"", L"", L"", 100, 1),
                std::make_tuple("XOF", "West African CFA franc", L"CFA", L"", L"", L"", 100, 1),
                std::make_tuple("BMD", "Bermudian dollar"      , L"BD$", L"", L"", L"", 100, 1),
                std::make_tuple("BTN", "Bhutanese ngultrum"    , L"Nu.", L"", L"", L"", 100, 1),
                std::make_tuple("BOB", "Bolivian boliviano"    , L"Bs.", L"", L"", L"", 100, 1),
                std::make_tuple("BAM", "Bosnia and Herzegovina konvertibilna marka", L"KM", L"", L"", L"", 100, 1),
                std::make_tuple("BWP", "Botswana pula"         , L"P", L"", L"", L"", 100, 1),
                std::make_tuple("BRL", "Brazilian real"        , L"R$", L"", L"", L"", 100, 1),
                std::make_tuple("BND", "Brunei dollar"         , L"B$", L"", L"", L"", 100, 1),
                std::make_tuple("BGN", "Bulgarian lev"         , L"", L"", L"", L"", 100, 1),
                std::make_tuple("BIF", "Burundi franc"         , L"FBu", L"", L"", L"", 1, 1),
                std::make_tuple("KHR", "Cambodian riel"        , L"", L"", L"", L"", 100, 1),
                std::make_tuple("XAF", "Central African CFA franc", L"CFA", L"", L"", L"", 1, 1),
                std::make_tuple("CAD", "Canadian dollar"       , L"$", L"", L"", L"", 100, 1),
                std::make_tuple("CVE", "Cape Verdean escudo"   , L"Esc", L"", L"", L"", 100, 1),
                std::make_tuple("KYD", "Cayman Islands dollar" , L"KY$", L"", L"", L"", 100, 1),
                std::make_tuple("CLP", "Chilean peso"          , L"$", L"", L"", L"", 1, 1),
                std::make_tuple("CNY", "Chinese renminbi"      , L"¥", L"", L"", L"", 100, 1),
                std::make_tuple("COP", "Colombian peso"        , L"Col$", L"", L"", L"", 100, 1),
                std::make_tuple("KMF", "Comorian franc"        , L"", L"", L"", L"", 1, 1),
                std::make_tuple("CDF", "Congolese franc"       , L"F", L"", L"", L"", 100, 1),
                std::make_tuple("CRC", "Costa Rican colon"     , L"₡", L"", L"", L"", 1, 1),
                std::make_tuple("HRK", "Croatian kuna"         , L"kn", L"", L"", L"", 100, 1),
                std::make_tuple("CUC", "Cuban peso"            , L"$", L"", L"", L"", 100, 1),
                std::make_tuple("CZK", "Czech koruna"          , L"Kč", L"", L"", L"", 100, 1),
                std::make_tuple("DKK", "Danish krone"          , L"Kr", L"", L"", L"", 100, 1),
                std::make_tuple("DJF", "Djiboutian franc"      , L"Fdj", L"", L"", L"", 1, 1),
                std::make_tuple("DOP", "Dominican peso"        , L"RD$", L"", L"", L"", 100, 1),
                std::make_tuple("EGP", "Egyptian pound"        , L"£", L"", L"", L"", 100, 1),
                std::make_tuple("ERN", "Eritrean nakfa"        , L"Nfa", L"", L"", L"", 100, 1),
                std::make_tuple("EEK", "Estonian kroon"        , L"KR", L"", L"", L"", 100, 1),
                std::make_tuple("ETB", "Ethiopian birr"        , L"Br", L"", L"", L"", 100, 1),
                std::make_tuple("FKP", "Falkland Islands pound", L"£", L"", L"", L"", 100, 1),
                std::make_tuple("FJD", "Fijian dollar"         , L"FJ$", L"", L"", L"", 100, 1),
                std::make_tuple("XPF", "CFP franc"             , L"F", L"", L"", L"", 100, 1),
                std::make_tuple("GMD", "Gambian dalasi"        , L"D", L"", L"", L"", 100, 1),
                std::make_tuple("GEL", "Georgian lari"         , L"", L"", L"", L"", 100, 1),
                std::make_tuple("GHS", "Ghanaian cedi"         , L"", L"", L"", L"", 100, 1),
                std::make_tuple("GIP", "Gibraltar pound"       , L"£", L"", L"", L"", 100, 1),
                std::make_tuple("GTQ", "Guatemalan quetzal"    , L"Q", L"", L"", L"", 100, 1),
                std::make_tuple("GNF", "Guinean franc"         , L"FG", L"", L"", L"", 1, 1),
                std::make_tuple("GYD", "Guyanese dollar"       , L"GY$", L"", L"", L"", 100, 1),
                std::make_tuple("HTG", "Haitian gourde"        , L"G", L"", L"", L"", 100, 1),
                std::make_tuple("HNL", "Honduran lempira"      , L"L", L"", L"", L"", 100, 1),
                std::make_tuple("HKD", "Hong Kong dollar"      , L"HK$", L"", L"", L"", 100, 1),
                std::make_tuple("HUF", "Hungarian forint"      , L"Ft", L"", L"", L"", 1, 1),
                std::make_tuple("ISK", L"Icelandic króna"       , L"kr", L"", L"", L"", 1, 1),
                std::make_tuple("INR", "Indian rupee"          , L"", L"", L"", L"", 100, 1),
                std::make_tuple("IDR", "Indonesian rupiah"     , L"Rp", L"", L"", L"", 1, 1),
                std::make_tuple("XDR", "Special Drawing Rights", L"SDR", L"", L"", L"", 100, 1),
                std::make_tuple("IRR", "Iranian rial"          , L"", L"", L"", L"", 1, 1),
                std::make_tuple("IQD", "Iraqi dinar"           , L"", L"", L"", L"", 1, 1),
                std::make_tuple("ILS", "Israeli new sheqel"    , L"", L"", L"", L"", 100, 1),
                std::make_tuple("JMD", "Jamaican dollar"       , L"J$", L"", L"", L"", 100, 1),
                std::make_tuple("JPY", "Japanese yen"          , L"¥", L"", L"", L"", 1, 1),
                std::make_tuple("JOD", "Jordanian dinar"       , L"", L"", L"", L"", 100, 1),
                std::make_tuple("KZT", "Kazakhstani tenge"     , L"T", L"", L"", L"", 100, 1),
                std::make_tuple("KES", "Kenyan shilling"       , L"KSh", L"", L"", L"", 100, 1),
                std::make_tuple("KPW", "North Korean won"      , L"W", L"", L"", L"", 100, 1),
                std::make_tuple("KRW", "South Korean won"      , L"W", L"", L"", L"", 1, 1),
                std::make_tuple("KWD", "Kuwaiti dinar"         , L"", L"", L"", L"", 100, 1),
                std::make_tuple("KGS", "Kyrgyzstani som"       , L"", L"", L"", L"", 100, 1),
                std::make_tuple("LAK", "Lao kip"               , L"KN", L"", L"", L"", 100, 1),
                std::make_tuple("LVL", "Latvian lats"          , L"Ls", L"", L"", L"", 100, 1),
                std::make_tuple("LBP", "Lebanese lira"         , L"", L"", L"", L"", 1, 1),
                std::make_tuple("LSL", "Lesotho loti"          , L"M", L"", L"", L"", 100, 1),
                std::make_tuple("LRD", "Liberian dollar"       , L"L$", L"", L"", L"", 100, 1),
                std::make_tuple("LYD", "Libyan dinar"          , L"LD", L"", L"", L"", 100, 1),
                std::make_tuple("LTL", "Lithuanian litas"      , L"Lt", L"", L"", L"", 100, 1),
                std::make_tuple("MOP", "Macanese pataca"       , L"P", L"", L"", L"", 100, 1),
                std::make_tuple("MKD", "Macedonian denar"      , L"", L"", L"", L"", 100, 1),
                std::make_tuple("MGA", "Malagasy ariary"       , L"FMG", L"", L"", L"", 100, 1),
                std::make_tuple("MWK", "Malawian kwacha"       , L"MK", L"", L"", L"", 1, 1),
                std::make_tuple("MYR", "Malaysian ringgit"     , L"RM", L"", L"", L"", 100, 1),
                std::make_tuple("MVR", "Maldivian rufiyaa"     , L"Rf", L"", L"", L"", 100, 1),
                std::make_tuple("MRO", "Mauritanian ouguiya"   , L"UM", L"", L"", L"", 100, 1),
                std::make_tuple("MUR", "Mauritian rupee"       , L"Rs", L"", L"", L"", 1, 1),
                std::make_tuple("MXN", "Mexican peso"          , L"$", L"", L"", L"", 100, 1),
                std::make_tuple("MDL", "Moldovan leu"          , L"", L"", L"", L"", 100, 1),
                std::make_tuple("MNT", "Mongolian tugrik"      ,L"₮", L"", L"", L"", 100, 1),
                std::make_tuple("MAD", "Moroccan dirham"       , L"", L"", L"", L"", 100, 1),
                std::make_tuple("MZM", "Mozambican metical"    , L"MTn", L"", L"", L"", 100, 1),
                std::make_tuple("MMK", "Myanma kyat"           , L"K", L"", L"", L"", 1, 1),
                std::make_tuple("NAD", "Namibian dollar"       , L"N$", L"", L"", L"", 100, 1),
                std::make_tuple("NPR", "Nepalese rupee"        , L"NRs", L"", L"", L"", 100, 1),
                std::make_tuple("ANG", "Netherlands Antillean gulden", L"NAƒ", L"", L"", L"", 100, 1),
                std::make_tuple("NZD", "New Zealand dollar"    , L"NZ$", L"", L"", L"", 100, 1),
                std::make_tuple("NIO", L"Nicaraguan córdoba"    , L"C$", L"", L"", L"", 100, 1),
                std::make_tuple("NGN", "Nigerian naira"        , L"₦", L"", L"", L"", 100, 1),
                std::make_tuple("NOK", "Norwegian krone"       , L"kr", L"", L"", L"", 100, 1),
                std::make_tuple("OMR", "Omani rial"            , L"", L"", L"", L"", 100, 1),
                std::make_tuple("PKR", "Pakistani rupee"       , L"Rs.", L"", L"", L"", 1, 1),
                std::make_tuple("PAB", "Panamanian balboa"     , L"B./", L"", L"", L"", 100, 1),
                std::make_tuple("PGK", "Papua New Guinean kina", L"K", L"", L"", L"", 100, 1),
                std::make_tuple("PYG", "Paraguayan guarani"    , L"", L"", L"", L"", 1, 1),
                std::make_tuple("PEN", "Peruvian nuevo sol"    , L"S/.", L"", L"", L"", 100, 1),
                std::make_tuple("PHP", "Philippine peso"       , L"₱", L"", L"", L"", 100, 1),
                std::make_tuple("PLN", "Polish zloty"          , L"", L"", L"", L"", 100, 1),
                std::make_tuple("QAR", "Qatari riyal"          , L"QR", L"", L"", L"", 100, 1),
                std::make_tuple("RON", "Romanian leu"          , L"L", L"", L"", L"", 100, 1),
                std::make_tuple("RWF", "Rwandan franc"         , L"RF", L"", L"", L"", 1, 1),
                std::make_tuple("STD", L"São Tomé and Príncipe dobra", L"Db", L"", L"", L"", 100, 1),
                std::make_tuple("SAR", "Saudi riyal"           , L"SR", L"", L"", L"", 100, 1),
                std::make_tuple("RSD", "Serbian dinar"         , L"din.", L"", L"", L"", 1, 1),
                std::make_tuple("SCR", "Seychellois rupee"     , L"SR", L"", L"", L"", 100, 1),
                std::make_tuple("SLL", "Sierra Leonean leone"  , L"Le", L"", L"", L"", 100, 1),
                std::make_tuple("SGD", "Singapore dollar"      , L"S$", L"", L"", L"", 100, 1),
                std::make_tuple("SBD", "Solomon Islands dollar", L"SI$", L"", L"", L"", 100, 1),
                std::make_tuple("SOS", "Somali shilling"       , L"Sh.", L"", L"", L"", 1, 1),
                std::make_tuple("ZAR", "South African rand"    , L"R", L"", L"", L"", 100, 1),
                std::make_tuple("LKR", "Sri Lankan rupee"      , L"Rs", L"", L"", L"", 100, 1),
                std::make_tuple("SHP", "Saint Helena pound"    , L"£", L"", L"", L"", 100, 1),
                std::make_tuple("SDG", "Sudanese pound"        , L"", L"", L"", L"", 100, 1),
                std::make_tuple("SRD", "Surinamese dollar"     , L"$", L"", L"", L"", 100, 1),
                std::make_tuple("SZL", "Swazi lilangeni"       , L"E", L"", L"", L"", 100, 1),
                std::make_tuple("SEK", "Swedish krona"         , L"kr", L"", L"", L"", 100, 1),
                std::make_tuple("CHF", "Swiss franc"           , L"Fr.", L"", L"", L"", 100, 1),
                std::make_tuple("SYP", "Syrian pound"          , L"", L"", L"", L"", 1, 1),
                std::make_tuple("TWD", "New Taiwan dollar"     , L"NT$", L"", L"", L"", 100, 1),
                std::make_tuple("TJS", "Tajikistani somoni"    , L"", L"", L"", L"", 100, 1),
                std::make_tuple("TZS", "Tanzanian shilling"    , L"", L"", L"", L"", 1, 1),
                std::make_tuple("THB", "Thai baht"             , L"฿", L"", L"", L"", 100, 1),
                std::make_tuple("TTD", "Trinidad and Tobago dollar", L"TT$", L"", L"", L"", 100, 1),
                std::make_tuple("TND", "Tunisian dinar"        , L"DT", L"", L"", L"", 100, 1),
                std::make_tuple("TRY", "Turkish new lira"      , L"YTL", L"", L"", L"", 100, 1),
                std::make_tuple("TMT", "Turkmen manat"         , L"m", L"", L"", L"", 100, 1),
                std::make_tuple("UGX", "Ugandan shilling"      , L"USh", L"", L"", L"", 1, 1),
                std::make_tuple("AED", "UAE dirham"            , L"", L"", L"", L"", 100, 1),
                std::make_tuple("UYU", "Uruguayan peso"        , L"$U", L"", L"", L"", 100, 1),
                std::make_tuple("UZS", "Uzbekistani som"       , L"", L"", L"", L"", 1, 1),
                std::make_tuple("VUV", "Vanuatu vatu"          , L"VT", L"", L"", L"", 100, 1),
                std::make_tuple("VEB", "Venezuelan bolivar"    , L"Bs", L"", L"", L"", 100, 1),
                std::make_tuple("VND", "Vietnamese dong"       , L"₫", L"", L"", L"", 1, 1),
                std::make_tuple("WST", "Samoan tala"           , L"WS$", L"", L"", L"", 100, 1),
                std::make_tuple("YER", "Yemeni rial"           , L"", L"", L"", L"", 1, 1),
                std::make_tuple("ZMK", "Zambian kwacha"        , L"ZK", L"", L"", L"", 1, 1),
                std::make_tuple("ZWR", "Zimbabwean dollar"     , L"Z$", L"", L"", L"", 100, 1)
            };

            this->Begin();
            for (const auto& i : all_currency)
            {
                Data *currency = this->create();
                
                currency->CURRENCY_SYMBOL   = std::get<0>(i);
                currency->CURRENCYNAME      = std::get<1>(i);
                currency->PFX_SYMBOL        = std::get<2>(i);
                currency->SFX_SYMBOL        = std::get<3>(i);
                currency->UNIT_NAME         = std::get<4>(i);
                currency->CENT_NAME         = std::get<5>(i);
                currency->SCALE             = std::get<6>(i);
                currency->BASECONVRATE      = std::get<7>(i);
                currency->DECIMAL_POINT     = ".";
                currency->GROUP_SEPARATOR   = ",";

                currency->save(this->db_);
            }
            this->Commit();
        }
        return all(db_, col, asc);
    }
    Data* get(int id)
    {
        return get(id, this->db_);
    }
    int save(Data* r)
    {
        r->save(this->db_);
        return r->id();
    }
    bool remove(int id)
    {
        return this->remove(id, db_);
    }
public:
    // Getter
    static Data* GetBaseCurrency()
    {
        int currency_id = Model_Infotable::instance().GetBaseCurrencyId();
        return Model_Currency::instance().get(currency_id);
    }
public:
    static wxString toCurrency(double value, const Data* currency = GetBaseCurrency())
    {
        wxString d2s = toString(value, currency);
        d2s.Prepend(currency->PFX_SYMBOL);
        d2s.Append(currency->SFX_SYMBOL);
        return d2s;
    }
    static wxString toString(double value, const Data* currency = GetBaseCurrency())
    {
        wxString d2s = "";
        if (currency) 
            d2s = wxNumberFormatter::ToString(value, Model_Currency::precision(currency)); // Style_WithThousandsSep
        else
            d2s = wxNumberFormatter::ToString(value, 1);
        return d2s;
    }
    static int precision(const Data* r)
    {
        return static_cast<int>(log10(static_cast<double>(r->SCALE)));
    }
    static int precision(const Data& r) { return precision(&r); }
};

#endif // 
