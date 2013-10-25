/*******************************************************
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

#include "mmcurrency.h"
#include "util.h"
#include "mmCurrencyFormatter.h"
#include <map>
#include "model/Model_Infotable.h"

mmCurrency::mmCurrency(wxSQLite3ResultSet& q1)
{
    currencyID_       = q1.GetInt("CURRENCYID");
    currencyName_     = q1.GetString("CURRENCYNAME");
    pfxSymbol_        = q1.GetString("PFX_SYMBOL");
    sfxSymbol_        = q1.GetString("SFX_SYMBOL");
    dec_              = q1.GetString("DECIMAL_POINT");
    grp_              = q1.GetString("GROUP_SEPARATOR");
    unit_             = q1.GetString("UNIT_NAME");
    cent_             = q1.GetString("CENT_NAME");
    scaleDl_          = q1.GetInt("SCALE");
    baseConv_         = q1.GetDouble("BASECONVRATE", 1.0);
    if(q1.GetColumnCount() < 11) {
       /* no Currency symbol in the table yet */
       currencySymbol_ = wxEmptyString;
    } else {
       currencySymbol_ = q1.GetString("CURRENCY_SYMBOL");
    }
    decChar_ = 0;
    grpChar_ = 0;
    if (!dec_.IsEmpty())
    {
      decChar_ = dec_.GetChar(0);
    }

    if (!grp_.IsEmpty())
    {
      grpChar_ = grp_.GetChar(0);
    }
}

mmCurrency::mmCurrency()
    : currencyID_(-1)
    , currencyName_("US Dollar")
    , pfxSymbol_('$')
    , sfxSymbol_()
    , dec_('.')
    , grp_(',')
    , unit_("dollar")
    , cent_("cent")
    , scaleDl_(100)
    , baseConv_(1)
    , decChar_('\0')
    , grpChar_('\0')
    , currencySymbol_("USD")
{}

mmCurrency::mmCurrency(const wxString& currencySymbol
                       , const wxString& currencyName
                       , const wxString& pfxSymbol
                       , const wxString& sfxSymbol
                       , const wxString& unit
                       , const wxString& cent
                       , double scaleDl
                       , double baseConv
                       )
                       :
currencySymbol_(currencySymbol)
    , currencyName_(currencyName)
    , pfxSymbol_(pfxSymbol)
    , sfxSymbol_(sfxSymbol)
    , unit_(unit)
    , cent_(cent)
    , scaleDl_(scaleDl)
    , baseConv_(baseConv)
{
    currencyID_ = -1;
    dec_ = '.';
    grp_ = ',';
    decChar_ = '\0';
    grpChar_ ='\0';
}

void mmCurrency::loadCurrencySettings()
{
    CurrencyFormatter::instance().loadSettings(
        pfxSymbol_,
        sfxSymbol_,
        decChar_,
        grpChar_,
        unit_,
        cent_,
        scaleDl_
    );
}

std::vector<mmCurrency> mmCurrency::currency_map()
{
    std::vector<mmCurrency> cm;
    //                    symbol   name                   prefix    suffix  unit cent  scale rate
    cm.push_back(mmCurrency("USD", "United States dollar"  , L"$"   , L""     , L"", L"", 100,1));
    cm.push_back(mmCurrency("EUR", "European euro"         , L"€"   , L""     , L"", L"", 100,1));
    cm.push_back(mmCurrency("GBP", "UK Pound"              , L"£"   , ""      , "Pound", "Pence", 100, 1));
    cm.push_back(mmCurrency("RUB", "Russian Ruble"         , ""     , L"р"    , "руб.", "коп.", 100, 1));
    cm.push_back(mmCurrency("UAH", "Ukrainian hryvnia"     , L"₴"   , L""     , L"", L"", 100, 1));
    cm.push_back(mmCurrency("AFN", "Afghan afghani"        , L"؋"   , L""     , L"" , L"pul", 100 , 1));
    cm.push_back(mmCurrency("ALL", "Albanian lek"          , L""    , L"L"     , L"", L"", 1, 1));
    cm.push_back(mmCurrency("DZD", "Algerian dinar"        , L"دج"  , L""     , L"", L"", 100, 1));
    cm.push_back(mmCurrency("AOA", "Angolan kwanza"        , L""    , L"Kz"   , L"", L"Céntimo", 100, 1));
    cm.push_back(mmCurrency("XCD", "East Caribbean dollar" , L"EC$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("ARS", "Argentine peso"        , L"", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("AMD", "Armenian dram"         , L"", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("AWG", "Aruban florin"         , L"ƒ", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("AUD", "Australian dollar"     , L"$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("AZN", "Azerbaijani manat"     , L"", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("BSD", "Bahamian dollar"       , L"B$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("BHD", "Bahraini dinar"        , L"", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("BDT", "Bangladeshi taka"      , L"", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("BBD", "Barbadian dollar"      , L"Bds$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("BYR", "Belarusian ruble"      , L"Br", L"", L"", L"",1,1));
    cm.push_back(mmCurrency("BZD", "Belize dollar"         , L"BZ$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("XOF", "West African CFA franc", L"CFA", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("BMD", "Bermudian dollar"      , L"BD$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("BTN", "Bhutanese ngultrum"    , L"Nu.", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("BOB", "Bolivian boliviano"    , L"Bs.", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("BAM", "Bosnia and Herzegovina konvertibilna marka", L"KM", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("BWP", "Botswana pula"         , L"P", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("BRL", "Brazilian real"        , L"R$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("BND", "Brunei dollar"         , L"B$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("BGN", "Bulgarian lev"         , L"", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("BIF", "Burundi franc"         , L"FBu", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("KHR", "Cambodian riel"        , L"", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("XAF", "Central African CFA franc", L"CFA", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("CAD", "Canadian dollar"       , L"$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("CVE", "Cape Verdean escudo"   , L"Esc", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("KYD", "Cayman Islands dollar" , L"KY$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("CLP", "Chilean peso"          , L"$", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("CNY", "Chinese renminbi"      , L"¥", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("COP", "Colombian peso"        , L"Col$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("KMF", "Comorian franc"        , L"", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("CDF", "Congolese franc"       , L"F", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("CRC", "Costa Rican colon"     , L"₡", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("HRK", "Croatian kuna"         , L"kn", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("CUC", "Cuban peso"            , L"$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("CZK", "Czech koruna"          , L"Kč", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("DKK", "Danish krone"          , L"Kr", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("DJF", "Djiboutian franc"      , L"Fdj", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("DOP", "Dominican peso"        , L"RD$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("EGP", "Egyptian pound"        , L"£", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("ERN", "Eritrean nakfa"        , L"Nfa", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("EEK", "Estonian kroon"        , L"KR", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("ETB", "Ethiopian birr"        , L"Br", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("FKP", "Falkland Islands pound", L"£", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("FJD", "Fijian dollar"         , L"FJ$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("XPF", "CFP franc"             , L"F", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("GMD", "Gambian dalasi"        , L"D", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("GEL", "Georgian lari"         , L"", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("GHS", "Ghanaian cedi"         , L"", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("GIP", "Gibraltar pound"       , L"£", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("GTQ", "Guatemalan quetzal"    , L"Q", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("GNF", "Guinean franc"         , L"FG", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("GYD", "Guyanese dollar"       , L"GY$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("HTG", "Haitian gourde"        , L"G", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("HNL", "Honduran lempira"      , L"L", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("HKD", "Hong Kong dollar"      , L"HK$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("HUF", "Hungarian forint"      , L"Ft", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("ISK", L"Icelandic króna"       , L"kr", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("INR", "Indian rupee"          , L"", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("IDR", "Indonesian rupiah"     , L"Rp", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("XDR", "Special Drawing Rights", L"SDR", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("IRR", "Iranian rial"          , L"", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("IQD", "Iraqi dinar"           , L"", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("ILS", "Israeli new sheqel"    , L"", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("JMD", "Jamaican dollar"       , L"J$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("JPY", "Japanese yen"          , L"¥", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("JOD", "Jordanian dinar"       , L"", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("KZT", "Kazakhstani tenge"     , L"T", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("KES", "Kenyan shilling"       , L"KSh", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("KPW", "North Korean won"      , L"W", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("KRW", "South Korean won"      , L"W", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("KWD", "Kuwaiti dinar"         , L"", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("KGS", "Kyrgyzstani som"       , L"", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("LAK", "Lao kip"               , L"KN", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("LVL", "Latvian lats"          , L"Ls", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("LBP", "Lebanese lira"         , L"", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("LSL", "Lesotho loti"          , L"M", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("LRD", "Liberian dollar"       , L"L$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("LYD", "Libyan dinar"          , L"LD", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("LTL", "Lithuanian litas"      , L"Lt", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("MOP", "Macanese pataca"       , L"P", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("MKD", "Macedonian denar"      , L"", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("MGA", "Malagasy ariary"       , L"FMG", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("MWK", "Malawian kwacha"       , L"MK", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("MYR", "Malaysian ringgit"     , L"RM", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("MVR", "Maldivian rufiyaa"     , L"Rf", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("MRO", "Mauritanian ouguiya"   , L"UM", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("MUR", "Mauritian rupee"       , L"Rs", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("MXN", "Mexican peso"          , L"$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("MDL", "Moldovan leu"          , L"", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("MNT", "Mongolian tugrik"      ,L"₮", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("MAD", "Moroccan dirham"       , L"", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("MZM", "Mozambican metical"    , L"MTn", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("MMK", "Myanma kyat"           , L"K", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("NAD", "Namibian dollar"       , L"N$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("NPR", "Nepalese rupee"        , L"NRs", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("ANG", "Netherlands Antillean gulden", L"NAƒ", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("NZD", "New Zealand dollar"    , L"NZ$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("NIO", L"Nicaraguan córdoba"    , L"C$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("NGN", "Nigerian naira"        , L"₦", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("NOK", "Norwegian krone"       , L"kr", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("OMR", "Omani rial"            , L"", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("PKR", "Pakistani rupee"       , L"Rs.", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("PAB", "Panamanian balboa"     , L"B./", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("PGK", "Papua New Guinean kina", L"K", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("PYG", "Paraguayan guarani"    , L"", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("PEN", "Peruvian nuevo sol"    , L"S/.", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("PHP", "Philippine peso"       , L"₱", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("PLN", "Polish zloty"          , L"", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("QAR", "Qatari riyal"          , L"QR", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("RON", "Romanian leu"          , L"L", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("RWF", "Rwandan franc"         , L"RF", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("STD", L"São Tomé and Príncipe dobra", L"Db", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("SAR", "Saudi riyal"           , L"SR", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("RSD", "Serbian dinar"         , L"din.", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("SCR", "Seychellois rupee"     , L"SR", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("SLL", "Sierra Leonean leone"  , L"Le", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("SGD", "Singapore dollar"      , L"S$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("SBD", "Solomon Islands dollar", L"SI$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("SOS", "Somali shilling"       , L"Sh.", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("ZAR", "South African rand"    , L"R", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("LKR", "Sri Lankan rupee"      , L"Rs", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("SHP", "Saint Helena pound"    , L"£", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("SDG", "Sudanese pound"        , L"", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("SRD", "Surinamese dollar"     , L"$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("SZL", "Swazi lilangeni"       , L"E", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("SEK", "Swedish krona"         , L"kr", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("CHF", "Swiss franc"           , L"Fr.", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("SYP", "Syrian pound"          , L"", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("TWD", "New Taiwan dollar"     , L"NT$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("TJS", "Tajikistani somoni"    , L"", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("TZS", "Tanzanian shilling"    , L"", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("THB", "Thai baht"             , L"฿", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("TTD", "Trinidad and Tobago dollar", L"TT$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("TND", "Tunisian dinar"        , L"DT", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("TRY", "Turkish new lira"      , L"YTL", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("TMT", "Turkmen manat"         , L"m", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("UGX", "Ugandan shilling"      , L"USh", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("AED", "UAE dirham"            , L"", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("UYU", "Uruguayan peso"        , L"$U", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("UZS", "Uzbekistani som"       , L"", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("VUV", "Vanuatu vatu"          , L"VT", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("VEB", "Venezuelan bolivar"    , L"Bs", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("VND", "Vietnamese dong"       , L"₫", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("WST", "Samoan tala"           , L"WS$", L"", L"", L"", 100, 1));
    cm.push_back(mmCurrency("YER", "Yemeni rial"           , L"", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("ZMK", "Zambian kwacha"        , L"ZK", L"", L"", L"", 1, 1));
    cm.push_back(mmCurrency("ZWR", "Zimbabwean dollar"     , L"Z$", L"", L"", L"", 100, 1));

    return cm;
}
