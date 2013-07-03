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
#include "dbwrapper.h"
#include "mmCurrencyFormatter.h"
#include <map>

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

//-----------------------------------------------------------------------------//
mmCurrencyList::mmCurrencyList(std::shared_ptr<wxSQLite3Database> db)
    : db_(db)
    , info_table_()
{}

void mmCurrencyList::SetInfoTable(MMEX_IniSettings* info_table)
{
    info_table_ = info_table;
}

void mmCurrencyList::LoadBaseCurrencySettings() const
{
    wxASSERT(info_table_);
    int currencyID = info_table_->GetIntSetting("BASECURRENCYID", -1);

    if (currencyID != -1)
    {
        SetCurrencySetting(getCurrencySharedPtr(currencyID));
    }
    else
    {
        CurrencyFormatter::instance().loadDefaultSettings();
    }
}

void mmCurrencyList::LoadCurrencySetting(const wxString& currencySymbol)
{
    SetCurrencySetting(getCurrencySharedPtr(currencySymbol, true));
}

void mmCurrencyList::SetCurrencySetting(mmCurrency* pCurrency) const
{
    if (pCurrency)
    {
        CurrencyFormatter::instance().loadSettings(
            pCurrency->pfxSymbol_, pCurrency->sfxSymbol_, pCurrency->decChar_,
            pCurrency->grpChar_, pCurrency->unit_, pCurrency->cent_, pCurrency->scaleDl_);
    }
    else
    {
        CurrencyFormatter::instance().loadDefaultSettings();
    }
}

int mmCurrencyList::GetBaseCurrencySettings() const
{
    wxASSERT(info_table_);
    int iBaseCurrencyID = info_table_->GetIntSetting("BASECURRENCYID", -1);
    wxASSERT(iBaseCurrencyID > 0);
    return iBaseCurrencyID;

}

void mmCurrencyList::SetBaseCurrencySettings(int currencyID)
{
    wxASSERT(info_table_);
    info_table_->SetSetting("BASECURRENCYID", currencyID);
}

int mmCurrencyList::AddCurrency(mmCurrency* pCurrency)
{
    const mmCurrency &r = *pCurrency;
    std::vector<wxString> data;
    data.push_back(r.currencyName_);
    data.push_back(r.pfxSymbol_);
    data.push_back(r.sfxSymbol_);
    data.push_back(r.dec_);
    data.push_back(r.grp_);
    data.push_back(r.unit_);
    data.push_back(r.cent_);
    data.push_back(wxString()<<r.scaleDl_);
    data.push_back(wxString()<<r.baseConv_);
    data.push_back(r.currencySymbol_);

    long currencyID;
    wxString sql = wxString::FromUTF8(INSERT_INTO_CURRENCYFORMATS_V1);
    int iError = mmDBWrapper::mmSQLiteExecuteUpdate(db_.get(), data, sql, currencyID);
    if ( iError == 0 && currencyID > 0)
    {
        pCurrency->currencyID_ = currencyID;
        currencies_.push_back(pCurrency);
    }
    return currencyID;
}

void mmCurrencyList::UpdateCurrency(mmCurrency* pCurrency)
{
    wxASSERT(pCurrency->currencyID_ > 0);

    wxSQLite3Statement st = db_->PrepareStatement(UPDATE_CURRENCYFORMATS_V1);
    const mmCurrency &r = *pCurrency;

    int i = 0;
    st.Bind(++i, r.pfxSymbol_);
    st.Bind(++i, r.sfxSymbol_);
    st.Bind(++i, r.dec_);
    st.Bind(++i, r.grp_);
    st.Bind(++i, r.unit_);
    st.Bind(++i, r.cent_);
    st.Bind(++i, r.scaleDl_);
    st.Bind(++i, r.baseConv_);
    st.Bind(++i, r.currencySymbol_);
    st.Bind(++i, r.currencyName_);
    st.Bind(++i, r.currencyID_);

    wxASSERT(st.GetParamCount() == i);

    st.ExecuteUpdate();
    st.Finalize();
}

bool mmCurrencyList::DeleteCurrency(int currencyID)
{
    wxASSERT(currencyID > 0);

    bool result = false;
    if (mmDBWrapper::deleteCurrency(db_.get(), currencyID))
    {
        std::vector <mmCurrency* >::iterator Iter;
        for ( Iter = currencies_.begin( ) ; Iter != currencies_.end( ) ; Iter++ )
        {
            if ((*Iter)->currencyID_ == currencyID)
            {
                currencies_.erase(Iter);
                result = true;
                break;
            }
        }
    }
    return result;
}

int mmCurrencyList::getCurrencyID(const wxString& currencyName, bool symbol) const
{
    int currencyID = -1;
    mmCurrency* pCurrency = getCurrencySharedPtr(currencyName, symbol);
    currencyID = pCurrency->currencyID_;
    return currencyID;
}

wxString mmCurrencyList::getCurrencyName(int currencyID, bool symbol) const
{
    for (const auto &currency: currencies_)
    {
        if (currency->currencyID_ == currencyID)
        {
            return symbol ? currency->currencySymbol_ : currency->currencyName_;
        }
    }

    return wxEmptyString;
}

mmCurrency* mmCurrencyList::getCurrencySharedPtr(int currencyID) const
{
    for (const auto &currency: currencies_)
    {
        if (currency->currencyID_ == currencyID)
            return currency;
    }

    wxASSERT(false);
    return NULL;
}

mmCurrency* mmCurrencyList::getCurrencySharedPtr(const wxString& currencyName, bool symbol) const
{
    for (const auto &currency: currencies_)
    {
        if (symbol)
        {
            if (currency->currencySymbol_ == currencyName)
            return currency;
        }
        else
        {
            if (currency->currencyName_ == currencyName)
            return currency;
        }
    }

    return NULL;
}

void mmCurrencyList::LoadCurrencies()
{
    wxSQLite3ResultSet q1 = db_->ExecuteQuery(SELECT_ALL_FROM_CURRENCYFORMATS_V1);

    while (q1.NextRow())
    {
        mmCurrency* pCurrency(new mmCurrency(q1));
        currencies_.push_back(pCurrency);
    }

    q1.Finalize();
}

bool mmCurrencyList::OnlineUpdateCurRate(wxString& sError)
{
    LoadBaseCurrencySettings();
    int currencyID = GetBaseCurrencySettings();

    if(currencyID == -1)
    {
        sError = _("Could not find base currency symbol!");
        return false;
    }
    wxString base_symbol = getCurrencySharedPtr(currencyID)->currencySymbol_;

    wxString site;
    for (const auto &currency : currencies_)
    {
        const wxString symbol = currency->currencySymbol_.Upper();
        if (!symbol.IsEmpty()) site << symbol << base_symbol << "=X+";
    }
    if (site.Right(1).Contains("+")) site.RemoveLast(1);
    site = wxString::Format("http://download.finance.yahoo.com/d/quotes.csv?s=%s&f=sl1n&e=.csv", site);

    wxString sOutput;
    int err_code = site_content(site, sOutput);
    if (err_code != wxURL_NOERR)
    {
        sError = sOutput;
        return false;
    }

    wxString CurrencySymbol, sName;
    double dRate = 1;

    std::map<wxString, std::pair<double, wxString> > currency_data;

    // Break it up into lines
    wxStringTokenizer tkz(sOutput, "\r\n");

    while (tkz.HasMoreTokens())
    {
        wxString csvline = tkz.GetNextToken();

        wxRegEx pattern("\"(...)...=X\",([^,][0-9.]+),\"([^\"]*)\"");
        if (pattern.Matches(csvline))
        {
            CurrencySymbol = pattern.GetMatch(csvline, 1);
            pattern.GetMatch(csvline, 2).ToDouble(&dRate);
            sName = pattern.GetMatch(csvline, 3);
            currency_data[CurrencySymbol] = std::make_pair(dRate, sName);
        }
    }

    wxString msg = _("Currency rate updated");
    msg << "\n\n";

    db_->Begin();

    for (const auto &currency : currencies_)
    {
        const wxString currency_symbol = currency->currencySymbol_.Upper();
        if (!currency_symbol.IsEmpty())
        {
            if (currency_data.find(currency_symbol) != currency_data.end())
            {
                currency->baseConv_ = currency_data[currency_symbol].first;
                if (base_symbol == currency_symbol) currency->baseConv_ = 1;
                msg << wxString::Format(_("%s\t: %0.4f -> %0.4f\n"),
                    currency_symbol, currency->baseConv_, currency->baseConv_);
                UpdateCurrency(currency);
            }
            else
            {
                 msg << wxString::Format(_("%s\t: %s\n"),currency_symbol ,_("Invalid Value "));
            }
        }
    }

    db_->Commit();
    sError = msg;
    return true;
}

std::map<wxString, mmCurrency> mmCurrency::currency_map()
{
    std::map<wxString, mmCurrency> cm;
    //                    symbol   name                   prefix    suffix  unit cent  scale rate
    cm["USD"] = mmCurrency("USD", "United States dollar"  , L"$"   , L""     , L"", L"", 100,1);
    cm["EUR"] = mmCurrency("EUR", "European euro"         , L"€"   , L""     , L"", L"", 100,1);
    cm["GBP"] = mmCurrency("GBP", "UK Pound"              , L"£"   , ""      , "Pound", "Pence", 100, 1);
    cm["RUB"] = mmCurrency("RUB", "Russian Ruble"         , ""     , L"р"    , "руб.", "коп.", 100, 1);
    cm["UAH"] = mmCurrency("UAH", "Ukrainian hryvnia"     , L"₴"   , L""     , L"", L"", 100, 1);
    cm["AFN"] = mmCurrency("AFN", "Afghan afghani"        , L"؋"   , L""     , L"" , L"pul", 100 , 1);
    cm["ALL"] = mmCurrency("ALL", "Albanian lek"          , L""    , L"L"     , L"", L"", 1, 1);
    cm["DZD"] = mmCurrency("DZD", "Algerian dinar"        , L"دج"  , L""     , L"", L"", 100, 1);
    cm["AOA"] = mmCurrency("AOA", "Angolan kwanza"        , L""    , L"Kz"   , L"", L"Céntimo", 100, 1);
    cm["XCD"] = mmCurrency("XCD", "East Caribbean dollar" , L"EC$", L"", L"", L"", 100, 1);
    cm["ARS"] = mmCurrency("ARS", "Argentine peso"        , L"", L"", L"", L"", 100, 1);
    cm["AMD"] = mmCurrency("AMD", "Armenian dram"         , L"", L"", L"", L"", 1, 1);
    cm["AWG"] = mmCurrency("AWG", "Aruban florin"         , L"ƒ", L"", L"", L"", 100, 1);
    cm["AUD"] = mmCurrency("AUD", "Australian dollar"     , L"$", L"", L"", L"", 100, 1);
    cm["AZN"] = mmCurrency("AZN", "Azerbaijani manat"     , L"", L"", L"", L"", 100, 1);
    cm["BSD"] = mmCurrency("BSD", "Bahamian dollar"       , L"B$", L"", L"", L"", 100, 1);
    cm["BHD"] = mmCurrency("BHD", "Bahraini dinar"        , L"", L"", L"", L"", 100, 1);
    cm["BDT"] = mmCurrency("BDT", "Bangladeshi taka"      , L"", L"", L"", L"", 100, 1);
    cm["BBD"] = mmCurrency("BBD", "Barbadian dollar"      , L"Bds$", L"", L"", L"", 100, 1);
    cm["BYR"] = mmCurrency("BYR", "Belarusian ruble"      , L"Br", L"", L"", L"",1,1);
    cm["BZD"] = mmCurrency("BZD", "Belize dollar"         , L"BZ$", L"", L"", L"", 100, 1);
    cm["XOF"] = mmCurrency("XOF", "West African CFA franc", L"CFA", L"", L"", L"", 100, 1);
    cm["BMD"] = mmCurrency("BMD", "Bermudian dollar"      , L"BD$", L"", L"", L"", 100, 1);
    cm["BTN"] = mmCurrency("BTN", "Bhutanese ngultrum"    , L"Nu.", L"", L"", L"", 100, 1);
    cm["BOB"] = mmCurrency("BOB", "Bolivian boliviano"    , L"Bs.", L"", L"", L"", 100, 1);
    cm["BAM"] = mmCurrency("BAM", "Bosnia and Herzegovina konvertibilna marka", L"KM", L"", L"", L"", 100, 1);
    cm["BWP"] = mmCurrency("BWP", "Botswana pula"         , L"P", L"", L"", L"", 100, 1);
    cm["BRL"] = mmCurrency("BRL", "Brazilian real"        , L"R$", L"", L"", L"", 100, 1);
    cm["BND"] = mmCurrency("BND", "Brunei dollar"         , L"B$", L"", L"", L"", 100, 1);
    cm["BGN"] = mmCurrency("BGN", "Bulgarian lev"         , L"", L"", L"", L"", 100, 1);
    cm["XOF"] = mmCurrency("XOF", "West African CFA franc", L"CFA", L"", L"", L"", 100, 1);
    cm["BIF"] = mmCurrency("BIF", "Burundi franc"         , L"FBu", L"", L"", L"", 1, 1);
    cm["KHR"] = mmCurrency("KHR", "Cambodian riel"        , L"", L"", L"", L"", 100, 1);
    cm["XAF"] = mmCurrency("XAF", "Central African CFA franc", L"CFA", L"", L"", L"", 1, 1);
    cm["CAD"] = mmCurrency("CAD", "Canadian dollar"       , L"$", L"", L"", L"", 100, 1);
    cm["CVE"] = mmCurrency("CVE", "Cape Verdean escudo"   , L"Esc", L"", L"", L"", 100, 1);
    cm["KYD"] = mmCurrency("KYD", "Cayman Islands dollar" , L"KY$", L"", L"", L"", 100, 1);
    cm["XAF"] = mmCurrency("XAF", "Central African CFA franc", L"CFA", L"", L"", L"", 100, 1);
    cm["CLP"] = mmCurrency("CLP", "Chilean peso"          , L"$", L"", L"", L"", 1, 1);
    cm["CNY"] = mmCurrency("CNY", "Chinese renminbi"      , L"¥", L"", L"", L"", 100, 1);
    cm["COP"] = mmCurrency("COP", "Colombian peso"        , L"Col$", L"", L"", L"", 100, 1);
    cm["KMF"] = mmCurrency("KMF", "Comorian franc"        , L"", L"", L"", L"", 1, 1);
    cm["CDF"] = mmCurrency("CDF", "Congolese franc"       , L"F", L"", L"", L"", 100, 1);
    cm["CRC"] = mmCurrency("CRC", "Costa Rican colon"     , L"₡", L"", L"", L"", 1, 1);
    cm["XOF"] = mmCurrency("XOF", "West African CFA franc", L"CFA", L"", L"", L"", 1, 1);
    cm["HRK"] = mmCurrency("HRK", "Croatian kuna"         , L"kn", L"", L"", L"", 100, 1);
    cm["CUC"] = mmCurrency("CUC", "Cuban peso"            , L"$", L"", L"", L"", 100, 1);
    cm["CZK"] = mmCurrency("CZK", "Czech koruna"          , L"Kč", L"", L"", L"", 100, 1);
    cm["DKK"] = mmCurrency("DKK", "Danish krone"          , L"Kr", L"", L"", L"", 100, 1);
    cm["DJF"] = mmCurrency("DJF", "Djiboutian franc"      , L"Fdj", L"", L"", L"", 1, 1);
    cm["XCD"] = mmCurrency("XCD", "East Caribbean dollar" , L"EC$", L"", L"", L"", 100, 1);
    cm["DOP"] = mmCurrency("DOP", "Dominican peso"        , L"RD$", L"", L"", L"", 100, 1);
    cm["EGP"] = mmCurrency("EGP", "Egyptian pound"        , L"£", L"", L"", L"", 100, 1);
    cm["GQE"] = mmCurrency("GQE", "Central African CFA franc", L"CFA", L"", L"", L"", 100, 1);
    cm["ERN"] = mmCurrency("ERN", "Eritrean nakfa"        , L"Nfa", L"", L"", L"", 100, 1);
    cm["EEK"] = mmCurrency("EEK", "Estonian kroon"        , L"KR", L"", L"", L"", 100, 1);
    cm["ETB"] = mmCurrency("ETB", "Ethiopian birr"        , L"Br", L"", L"", L"", 100, 1);
    cm["FKP"] = mmCurrency("FKP", "Falkland Islands pound", L"£", L"", L"", L"", 100, 1);
    cm["FJD"] = mmCurrency("FJD", "Fijian dollar"         , L"FJ$", L"", L"", L"", 100, 1);
    cm["XPF"] = mmCurrency("XPF", "CFP franc"             , L"F", L"", L"", L"", 100, 1);
    cm["XAF"] = mmCurrency("XAF", "Central African CFA franc", L"CFA", L"", L"", L"", 100, 1);
    cm["GMD"] = mmCurrency("GMD", "Gambian dalasi"        , L"D", L"", L"", L"", 100, 1);
    cm["GEL"] = mmCurrency("GEL", "Georgian lari"         , L"", L"", L"", L"", 100, 1);
    cm["GHS"] = mmCurrency("GHS", "Ghanaian cedi"         , L"", L"", L"", L"", 100, 1);
    cm["GIP"] = mmCurrency("GIP", "Gibraltar pound"       , L"£", L"", L"", L"", 100, 1);
    cm["XCD"] = mmCurrency("XCD", "East Caribbean dollar" , L"EC$", L"", L"", L"", 100, 1);
    cm["GTQ"] = mmCurrency("GTQ", "Guatemalan quetzal"    , L"Q", L"", L"", L"", 100, 1);
    cm["XOF"] = mmCurrency("XOF", "West African CFA franc", L"CFA", L"", L"", L"", 100, 1);
    cm["GNF"] = mmCurrency("GNF", "Guinean franc"         , L"FG", L"", L"", L"", 1, 1);
    cm["GYD"] = mmCurrency("GYD", "Guyanese dollar"       , L"GY$", L"", L"", L"", 100, 1);
    cm["HTG"] = mmCurrency("HTG", "Haitian gourde"        , L"G", L"", L"", L"", 100, 1);
    cm["HNL"] = mmCurrency("HNL", "Honduran lempira"      , L"L", L"", L"", L"", 100, 1);
    cm["HKD"] = mmCurrency("HKD", "Hong Kong dollar"      , L"HK$", L"", L"", L"", 100, 1);
    cm["HUF"] = mmCurrency("HUF", "Hungarian forint"      , L"Ft", L"", L"", L"", 1, 1);
    cm["ISK"] = mmCurrency("ISK", L"Icelandic króna"       , L"kr", L"", L"", L"", 1, 1);
    cm["INR"] = mmCurrency("INR", "Indian rupee"          , L"", L"", L"", L"", 100, 1);
    cm["IDR"] = mmCurrency("IDR", "Indonesian rupiah"     , L"Rp", L"", L"", L"", 1, 1);
    cm["XDR"] = mmCurrency("XDR", "Special Drawing Rights", L"SDR", L"", L"", L"", 100, 1);
    cm["IRR"] = mmCurrency("IRR", "Iranian rial"          , L"", L"", L"", L"", 1, 1);
    cm["IQD"] = mmCurrency("IQD", "Iraqi dinar"           , L"", L"", L"", L"", 1, 1);
    cm["ILS"] = mmCurrency("ILS", "Israeli new sheqel"    , L"", L"", L"", L"", 100, 1);
    cm["JMD"] = mmCurrency("JMD", "Jamaican dollar"       , L"J$", L"", L"", L"", 100, 1);
    cm["JPY"] = mmCurrency("JPY", "Japanese yen"          , L"¥", L"", L"", L"", 1, 1);
    cm["JOD"] = mmCurrency("JOD", "Jordanian dinar"       , L"", L"", L"", L"", 100, 1);
    cm["KZT"] = mmCurrency("KZT", "Kazakhstani tenge"     , L"T", L"", L"", L"", 100, 1);
    cm["KES"] = mmCurrency("KES", "Kenyan shilling"       , L"KSh", L"", L"", L"", 100, 1);
    cm["KPW"] = mmCurrency("KPW", "North Korean won"      , L"W", L"", L"", L"", 100, 1);
    cm["KRW"] = mmCurrency("KRW", "South Korean won"      , L"W", L"", L"", L"", 1, 1);
    cm["KWD"] = mmCurrency("KWD", "Kuwaiti dinar"         , L"", L"", L"", L"", 100, 1);
    cm["KGS"] = mmCurrency("KGS", "Kyrgyzstani som"       , L"", L"", L"", L"", 100, 1);
    cm["LAK"] = mmCurrency("LAK", "Lao kip"               , L"KN", L"", L"", L"", 100, 1);
    cm["LVL"] = mmCurrency("LVL", "Latvian lats"          , L"Ls", L"", L"", L"", 100, 1);
    cm["LBP"] = mmCurrency("LBP", "Lebanese lira"         , L"", L"", L"", L"", 1, 1);
    cm["LSL"] = mmCurrency("LSL", "Lesotho loti"          , L"M", L"", L"", L"", 100, 1);
    cm["LRD"] = mmCurrency("LRD", "Liberian dollar"       , L"L$", L"", L"", L"", 100, 1);
    cm["LYD"] = mmCurrency("LYD", "Libyan dinar"          , L"LD", L"", L"", L"", 100, 1);
    cm["LTL"] = mmCurrency("LTL", "Lithuanian litas"      , L"Lt", L"", L"", L"", 100, 1);
    cm["MOP"] = mmCurrency("MOP", "Macanese pataca"       , L"P", L"", L"", L"", 100, 1);
    cm["MKD"] = mmCurrency("MKD", "Macedonian denar"      , L"", L"", L"", L"", 100, 1);
    cm["MGA"] = mmCurrency("MGA", "Malagasy ariary"       , L"FMG", L"", L"", L"", 100, 1);
    cm["MWK"] = mmCurrency("MWK", "Malawian kwacha"       , L"MK", L"", L"", L"", 1, 1);
    cm["MYR"] = mmCurrency("MYR", "Malaysian ringgit"     , L"RM", L"", L"", L"", 100, 1);
    cm["MVR"] = mmCurrency("MVR", "Maldivian rufiyaa"     , L"Rf", L"", L"", L"", 100, 1);
    cm["XOF"] = mmCurrency("XOF", "West African CFA franc", L"CFA", L"", L"", L"", 100, 1);
    cm["MRO"] = mmCurrency("MRO", "Mauritanian ouguiya"   , L"UM", L"", L"", L"", 100, 1);
    cm["MUR"] = mmCurrency("MUR", "Mauritian rupee"       , L"Rs", L"", L"", L"", 1, 1);
    cm["MXN"] = mmCurrency("MXN", "Mexican peso"          , L"$", L"", L"", L"", 100, 1);
    cm["MDL"] = mmCurrency("MDL", "Moldovan leu"          , L"", L"", L"", L"", 100, 1);
    cm["MNT"] = mmCurrency("MNT", "Mongolian tugrik"      ,L"₮", L"", L"", L"", 100, 1);
    cm["XCD"] = mmCurrency("XCD", "East Caribbean dollar" , L"EC$", L"", L"", L"", 100, 1);
    cm["MAD"] = mmCurrency("MAD", "Moroccan dirham"       , L"", L"", L"", L"", 100, 1);
    cm["MZM"] = mmCurrency("MZM", "Mozambican metical"    , L"MTn", L"", L"", L"", 100, 1);
    cm["MMK"] = mmCurrency("MMK", "Myanma kyat"           , L"K", L"", L"", L"", 1, 1);
    cm["NAD"] = mmCurrency("NAD", "Namibian dollar"       , L"N$", L"", L"", L"", 100, 1);
    cm["NPR"] = mmCurrency("NPR", "Nepalese rupee"        , L"NRs", L"", L"", L"", 100, 1);
    cm["ANG"] = mmCurrency("ANG", "Netherlands Antillean gulden", L"NAƒ", L"", L"", L"", 100, 1);
    cm["XPF"] = mmCurrency("XPF", "CFP franc"             , L"F", L"", L"", L"", 100, 1);
    cm["NZD"] = mmCurrency("NZD", "New Zealand dollar"    , L"NZ$", L"", L"", L"", 100, 1);
    cm["NIO"] = mmCurrency("NIO", L"Nicaraguan córdoba"    , L"C$", L"", L"", L"", 100, 1);
    cm["XOF"] = mmCurrency("XOF", "West African CFA franc", L"CFA", L"", L"", L"", 100, 1);
    cm["NGN"] = mmCurrency("NGN", "Nigerian naira"        , L"₦", L"", L"", L"", 100, 1);
    cm["NOK"] = mmCurrency("NOK", "Norwegian krone"       , L"kr", L"", L"", L"", 100, 1);
    cm["OMR"] = mmCurrency("OMR", "Omani rial"            , L"", L"", L"", L"", 100, 1);
    cm["PKR"] = mmCurrency("PKR", "Pakistani rupee"       , L"Rs.", L"", L"", L"", 1, 1);
    cm["PAB"] = mmCurrency("PAB", "Panamanian balboa"     , L"B./", L"", L"", L"", 100, 1);
    cm["PGK"] = mmCurrency("PGK", "Papua New Guinean kina", L"K", L"", L"", L"", 100, 1);
    cm["PYG"] = mmCurrency("PYG", "Paraguayan guarani"    , L"", L"", L"", L"", 1, 1);
    cm["PEN"] = mmCurrency("PEN", "Peruvian nuevo sol"    , L"S/.", L"", L"", L"", 100, 1);
    cm["PHP"] = mmCurrency("PHP", "Philippine peso"       , L"₱", L"", L"", L"", 100, 1);
    cm["PLN"] = mmCurrency("PLN", "Polish zloty"          , L"", L"", L"", L"", 100, 1);
    cm["QAR"] = mmCurrency("QAR", "Qatari riyal"          , L"QR", L"", L"", L"", 100, 1);
    cm["RON"] = mmCurrency("RON", "Romanian leu"          , L"L", L"", L"", L"", 100, 1);
    cm["RWF"] = mmCurrency("RWF", "Rwandan franc"         , L"RF", L"", L"", L"", 1, 1);
    cm["STD"] = mmCurrency("STD", L"São Tomé and Príncipe dobra", L"Db", L"", L"", L"", 100, 1);
    cm["SAR"] = mmCurrency("SAR", "Saudi riyal"           , L"SR", L"", L"", L"", 100, 1);
    cm["XOF"] = mmCurrency("XOF", "West African CFA franc", L"CFA", L"", L"", L"", 100, 1);
    cm["RSD"] = mmCurrency("RSD", "Serbian dinar"         , L"din.", L"", L"", L"", 1, 1);
    cm["SCR"] = mmCurrency("SCR", "Seychellois rupee"     , L"SR", L"", L"", L"", 100, 1);
    cm["SLL"] = mmCurrency("SLL", "Sierra Leonean leone"  , L"Le", L"", L"", L"", 100, 1);
    cm["SGD"] = mmCurrency("SGD", "Singapore dollar"      , L"S$", L"", L"", L"", 100, 1);
    cm["SBD"] = mmCurrency("SBD", "Solomon Islands dollar", L"SI$", L"", L"", L"", 100, 1);
    cm["SOS"] = mmCurrency("SOS", "Somali shilling"       , L"Sh.", L"", L"", L"", 1, 1);
    cm["ZAR"] = mmCurrency("ZAR", "South African rand"    , L"R", L"", L"", L"", 100, 1);
    cm["LKR"] = mmCurrency("LKR", "Sri Lankan rupee"      , L"Rs", L"", L"", L"", 100, 1);
    cm["SHP"] = mmCurrency("SHP", "Saint Helena pound"    , L"£", L"", L"", L"", 100, 1);
    cm["XCD"] = mmCurrency("XCD", "East Caribbean dollar" , L"EC$", L"", L"", L"", 100, 1);
    cm["SDG"] = mmCurrency("SDG", "Sudanese pound"        , L"", L"", L"", L"", 100, 1);
    cm["SRD"] = mmCurrency("SRD", "Surinamese dollar"     , L"$", L"", L"", L"", 100, 1);
    cm["SZL"] = mmCurrency("SZL", "Swazi lilangeni"       , L"E", L"", L"", L"", 100, 1);
    cm["SEK"] = mmCurrency("SEK", "Swedish krona"         , L"kr", L"", L"", L"", 100, 1);
    cm["CHF"] = mmCurrency("CHF", "Swiss franc"           , L"Fr.", L"", L"", L"", 100, 1);
    cm["SYP"] = mmCurrency("SYP", "Syrian pound"          , L"", L"", L"", L"", 1, 1);
    cm["TWD"] = mmCurrency("TWD", "New Taiwan dollar"     , L"NT$", L"", L"", L"", 100, 1);
    cm["TJS"] = mmCurrency("TJS", "Tajikistani somoni"    , L"", L"", L"", L"", 100, 1);
    cm["TZS"] = mmCurrency("TZS", "Tanzanian shilling"    , L"", L"", L"", L"", 1, 1);
    cm["THB"] = mmCurrency("THB", "Thai baht"             , L"฿", L"", L"", L"", 100, 1);
    cm["XOF"] = mmCurrency("XOF", "West African CFA franc", L"CFA", L"", L"", L"", 100, 1);
    cm["TTD"] = mmCurrency("TTD", "Trinidad and Tobago dollar", L"TT$", L"", L"", L"", 100, 1);
    cm["TND"] = mmCurrency("TND", "Tunisian dinar"        , L"DT", L"", L"", L"", 100, 1);
    cm["TRY"] = mmCurrency("TRY", "Turkish new lira"      , L"YTL", L"", L"", L"", 100, 1);
    cm["TMT"] = mmCurrency("TMT", "Turkmen manat"         , L"m", L"", L"", L"", 100, 1);
    cm["UGX"] = mmCurrency("UGX", "Ugandan shilling"      , L"USh", L"", L"", L"", 1, 1);
    cm["AED"] = mmCurrency("AED", "UAE dirham"            , L"", L"", L"", L"", 100, 1);
    cm["UYU"] = mmCurrency("UYU", "Uruguayan peso"        , L"$U", L"", L"", L"", 100, 1);
    cm["UZS"] = mmCurrency("UZS", "Uzbekistani som"       , L"", L"", L"", L"", 1, 1);
    cm["VUV"] = mmCurrency("VUV", "Vanuatu vatu"          , L"VT", L"", L"", L"", 100, 1);
    cm["VEB"] = mmCurrency("VEB", "Venezuelan bolivar"    , L"Bs", L"", L"", L"", 100, 1);
    cm["VND"] = mmCurrency("VND", "Vietnamese dong"       , L"₫", L"", L"", L"", 1, 1);
    cm["XPF"] = mmCurrency("XPF", "CFP franc"             , L"F", L"", L"", L"", 100, 1);
    cm["WST"] = mmCurrency("WST", "Samoan tala"           , L"WS$", L"", L"", L"", 100, 1);
    cm["YER"] = mmCurrency("YER", "Yemeni rial"           , L"", L"", L"", L"", 1, 1);
    cm["ZMK"] = mmCurrency("ZMK", "Zambian kwacha"        , L"ZK", L"", L"", L"", 1, 1);
    cm["ZWR"] = mmCurrency("ZWR", "Zimbabwean dollar"     , L"Z$", L"", L"", L"", 100, 1);

    return cm;
}
