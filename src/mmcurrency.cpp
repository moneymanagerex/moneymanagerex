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
    info_table_->SetIntSetting("BASECURRENCYID", currencyID);
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
                if (base_symbol == currency_symbol) currency->baseConv_ = 1;
                msg << wxString::Format(_("%s\t: %0.4f -> %0.4f\n"),
                    currency_symbol, currency->baseConv_, currency_data[currency_symbol].first);
                currency->baseConv_ = currency_data[currency_symbol].first;
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
    std::map<wxString, mmCurrency> currency_map;
    //                             symbol   name    prefix    suffix  unit_name cent  scale rate
    currency_map["USD"] = mmCurrency("USD", "United States dollar", L"$", L"", L"", L"", 100,1);
    currency_map["EUR"] = mmCurrency("EUR", "European euro", L"€", L"", L"", L"", 100,1);
    currency_map["GBP"] = mmCurrency("GBP", "UK Pound", L"£", "", "Pound", "Pence", 100, 1);
    currency_map["RUB"] = mmCurrency("RUB", "Russian Ruble", "", "р", "руб.", "коп.", 100, 1);
    currency_map["UAH"] = mmCurrency("UAH", "Ukrainian hryvnia", L"₴", L"", L"", L"", 100, 1);
    currency_map["AFN"] = mmCurrency("AFN", "Afghan afghani", L"", L"", L"", L"", 1 , 1);
    currency_map["ALL"] = mmCurrency("ALL", "Albanian lek", L"", L"", L"", L"", 1, 1);
    currency_map["DZD"] = mmCurrency("DZD", "Algerian dinar", L"", L"", L"", L"", 100, 1);
    currency_map["AOA"] = mmCurrency("AOA", "Angolan kwanza", L"", L"", L"", L"", 100, 1);
    currency_map["XCD"] = mmCurrency("XCD", "East Caribbean dollar", L"EC$", L"", L"", L"", 100, 1);
    currency_map["XCD"] = mmCurrency("XCD", "East Caribbean dollar", L"EC$", L"", L"", L"", 100, 1);
    currency_map["ARS"] = mmCurrency("ARS", "Argentine peso", L"", L"", L"", L"", 100, 1);
    currency_map["AMD"] = mmCurrency("AMD", "Armenian dram", L"", L"", L"", L"", 1, 1);
    currency_map["AWG"] = mmCurrency("AWG", "Aruban florin", L"ƒ", L"", L"", L"", 100, 1);
    currency_map["AUD"] = mmCurrency("AUD", "Australian dollar", L"$", L"", L"", L"", 100, 1);
    currency_map["AZN"] = mmCurrency("AZN", "Azerbaijani manat", L"", L"", L"", L"", 100, 1);
    currency_map["BSD"] = mmCurrency("BSD", "Bahamian dollar", L"B$", L"", L"", L"", 100, 1);
    currency_map["BHD"] = mmCurrency("BHD", "Bahraini dinar", L"", L"", L"", L"", 100, 1);
    currency_map["BDT"] = mmCurrency("BDT", "Bangladeshi taka", L"", L"", L"", L"", 100, 1);
    currency_map["BBD"] = mmCurrency("BBD", "Barbadian dollar", L"Bds$", L"", L"", L"", 100, 1);
    currency_map["BYR"] = mmCurrency("BYR", "Belarusian ruble", L"Br", L"", L"", L"",1,1);
    currency_map["BZD"] = mmCurrency("BZD", "Belize dollar", L"BZ$", L"", L"", L"", 100, 1);
    currency_map["XOF"] = mmCurrency("XOF", "West African CFA franc", L"CFA", L"", L"", L"", 100, 1);
    currency_map["BMD"] = mmCurrency("BMD", "Bermudian dollar", L"BD$", L"", L"", L"", 100, 1);
    currency_map["BTN"] = mmCurrency("BTN", "Bhutanese ngultrum", L"Nu.", L"", L"", L"", 100, 1);
    currency_map["BOB"] = mmCurrency("BOB", "Bolivian boliviano", L"Bs.", L"", L"", L"", 100, 1);
    currency_map["BAM"] = mmCurrency("BAM", "Bosnia and Herzegovina konvertibilna marka", L"KM", L"", L"", L"", 100, 1);
    currency_map["BWP"] = mmCurrency("BWP", "Botswana pula", L"P", L"", L"", L"", 100, 1);
    currency_map["BRL"] = mmCurrency("BRL", "Brazilian real", L"R$", L"", L"", L"", 100, 1);
    currency_map["BND"] = mmCurrency("BND", "Brunei dollar", L"B$", L"", L"", L"", 100, 1);
    currency_map["BGN"] = mmCurrency("BGN", "Bulgarian lev", L"", L"", L"", L"", 100, 1);
    currency_map["XOF"] = mmCurrency("XOF", "West African CFA franc", L"CFA", L"", L"", L"", 100, 1);
    currency_map["BIF"] = mmCurrency("BIF", "Burundi franc", L"FBu", L"", L"", L"", 1, 1);
    currency_map["KHR"] = mmCurrency("KHR", "Cambodian riel", L"", L"", L"", L"", 100, 1);
    currency_map["XAF"] = mmCurrency("XAF", "Central African CFA franc", L"CFA", L"", L"", L"", 1, 1);
    currency_map["CAD"] = mmCurrency("CAD", "Canadian dollar", L"$", L"", L"", L"", 100, 1);
    currency_map["CVE"] = mmCurrency("CVE", "Cape Verdean escudo", L"Esc", L"", L"", L"", 100, 1);
    currency_map["KYD"] = mmCurrency("KYD", "Cayman Islands dollar", L"KY$", L"", L"", L"", 100, 1);
    currency_map["XAF"] = mmCurrency("XAF", "Central African CFA franc", L"CFA", L"", L"", L"", 100, 1);
    currency_map["XAF"] = mmCurrency("XAF", "Central African CFA franc", L"CFA", L"", L"", L"", 100, 1);
    currency_map["CLP"] = mmCurrency("CLP", "Chilean peso", L"$", L"", L"", L"", 1, 1);
    currency_map["CNY"] = mmCurrency("CNY", "Chinese renminbi", L"¥", L"", L"", L"", 100, 1);
    currency_map["COP"] = mmCurrency("COP", "Colombian peso", L"Col$", L"", L"", L"", 100, 1);
    currency_map["KMF"] = mmCurrency("KMF", "Comorian franc", L"", L"", L"", L"", 1, 1);
    currency_map["XAF"] = mmCurrency("XAF", "Central African CFA franc", L"CFA", L"", L"", L"", 100, 1);
    currency_map["CDF"] = mmCurrency("CDF", "Congolese franc", L"F", L"", L"", L"", 100, 1);
    currency_map["CRC"] = mmCurrency("CRC", "Costa Rican colon", L"₡", L"", L"", L"", 1, 1);
    currency_map["XOF"] = mmCurrency("XOF", "West African CFA franc", L"CFA", L"", L"", L"", 1, 1);
    currency_map["HRK"] = mmCurrency("HRK", "Croatian kuna", L"kn", L"", L"", L"", 100, 1);
    currency_map["CUC"] = mmCurrency("CUC", "Cuban peso", L"$", L"", L"", L"", 100, 1);
    currency_map["CZK"] = mmCurrency("CZK", "Czech koruna", L"Kč", L"", L"", L"", 100, 1);
    currency_map["DKK"] = mmCurrency("DKK", "Danish krone", L"Kr", L"", L"", L"", 100, 1);
    currency_map["DJF"] = mmCurrency("DJF", "Djiboutian franc", L"Fdj", L"", L"", L"", 1, 1);
    currency_map["XCD"] = mmCurrency("XCD", "East Caribbean dollar", L"EC$", L"", L"", L"", 100, 1);
    currency_map["DOP"] = mmCurrency("DOP", "Dominican peso", L"RD$", L"", L"", L"", 100, 1);
    currency_map["EGP"] = mmCurrency("EGP", "Egyptian pound", L"£", L"", L"", L"", 100, 1);
    currency_map["GQE"] = mmCurrency("GQE", "Central African CFA franc", L"CFA", L"", L"", L"", 100, 1);
    currency_map["ERN"] = mmCurrency("ERN", "Eritrean nakfa", L"Nfa", L"", L"", L"", 100, 1);
    currency_map["EEK"] = mmCurrency("EEK", "Estonian kroon", L"KR", L"", L"", L"", 100, 1);
    currency_map["ETB"] = mmCurrency("ETB", "Ethiopian birr", L"Br", L"", L"", L"", 100, 1);
    currency_map["FKP"] = mmCurrency("FKP", "Falkland Islands pound", L"£", L"", L"", L"", 100, 1);
    currency_map["FJD"] = mmCurrency("FJD", "Fijian dollar", L"FJ$", L"", L"", L"", 100, 1);
    currency_map["XPF"] = mmCurrency("XPF", "CFP franc", L"F", L"", L"", L"", 100, 1);
    currency_map["XAF"] = mmCurrency("XAF", "Central African CFA franc", L"CFA", L"", L"", L"", 100, 1);
    currency_map["GMD"] = mmCurrency("GMD", "Gambian dalasi", L"D", L"", L"", L"", 100, 1);
    currency_map["GEL"] = mmCurrency("GEL", "Georgian lari", L"", L"", L"", L"", 100, 1);
    currency_map["GHS"] = mmCurrency("GHS", "Ghanaian cedi", L"", L"", L"", L"", 100, 1);
    currency_map["GIP"] = mmCurrency("GIP", "Gibraltar pound", L"£", L"", L"", L"", 100, 1);
    currency_map["XCD"] = mmCurrency("XCD", "East Caribbean dollar", L"EC$", L"", L"", L"", 100, 1);
    currency_map["GTQ"] = mmCurrency("GTQ", "Guatemalan quetzal", L"Q", L"", L"", L"", 100, 1);
    currency_map["XOF"] = mmCurrency("XOF", "West African CFA franc", L"CFA", L"", L"", L"", 100, 1);
    currency_map["GNF"] = mmCurrency("GNF", "Guinean franc", L"FG", L"", L"", L"", 1, 1);
    currency_map["GYD"] = mmCurrency("GYD", "Guyanese dollar", L"GY$", L"", L"", L"", 100, 1);
    currency_map["HTG"] = mmCurrency("HTG", "Haitian gourde", L"G", L"", L"", L"", 100, 1);
    currency_map["HNL"] = mmCurrency("HNL", "Honduran lempira", L"L", L"", L"", L"", 100, 1);
    currency_map["HKD"] = mmCurrency("HKD", "Hong Kong dollar", L"HK$", L"", L"", L"", 100, 1);
    currency_map["HUF"] = mmCurrency("HUF", "Hungarian forint", L"Ft", L"", L"", L"", 1, 1);
    currency_map["ISK"] = mmCurrency("ISK", "Icelandic króna", L"kr", L"", L"", L"", 1, 1);
    currency_map["INR"] = mmCurrency("INR", "Indian rupee", L"", L"", L"", L"", 100, 1);
    currency_map["IDR"] = mmCurrency("IDR", "Indonesian rupiah", L"Rp", L"", L"", L"", 1, 1);
    currency_map["XDR"] = mmCurrency("XDR", "Special Drawing Rights", L"SDR", L"", L"", L"", 100, 1);
    currency_map["IRR"] = mmCurrency("IRR", "Iranian rial", L"", L"", L"", L"", 1, 1);
    currency_map["IQD"] = mmCurrency("IQD", "Iraqi dinar", L"", L"", L"", L"", 1, 1);
    currency_map["ILS"] = mmCurrency("ILS", "Israeli new sheqel", L"", L"", L"", L"", 100, 1);
    currency_map["JMD"] = mmCurrency("JMD", "Jamaican dollar", L"J$", L"", L"", L"", 100, 1);
    currency_map["JPY"] = mmCurrency("JPY", "Japanese yen", L"¥", L"", L"", L"", 1, 1);
    currency_map["JOD"] = mmCurrency("JOD", "Jordanian dinar", L"", L"", L"", L"", 100, 1);
    currency_map["KZT"] = mmCurrency("KZT", "Kazakhstani tenge", L"T", L"", L"", L"", 100, 1);
    currency_map["KES"] = mmCurrency("KES", "Kenyan shilling", L"KSh", L"", L"", L"", 100, 1);
    currency_map["KPW"] = mmCurrency("KPW", "North Korean won", L"W", L"", L"", L"", 100, 1);
    currency_map["KRW"] = mmCurrency("KRW", "South Korean won", L"W", L"", L"", L"", 1, 1);
    currency_map["KWD"] = mmCurrency("KWD", "Kuwaiti dinar", L"", L"", L"", L"", 100, 1);
    currency_map["KGS"] = mmCurrency("KGS", "Kyrgyzstani som", L"", L"", L"", L"", 100, 1);
    currency_map["LAK"] = mmCurrency("LAK", "Lao kip", L"KN", L"", L"", L"", 100, 1);
    currency_map["LVL"] = mmCurrency("LVL", "Latvian lats", L"Ls", L"", L"", L"", 100, 1);
    currency_map["LBP"] = mmCurrency("LBP", "Lebanese lira", L"", L"", L"", L"", 1, 1);
    currency_map["LSL"] = mmCurrency("LSL", "Lesotho loti", L"M", L"", L"", L"", 100, 1);
    currency_map["LRD"] = mmCurrency("LRD", "Liberian dollar", L"L$", L"", L"", L"", 100, 1);
    currency_map["LYD"] = mmCurrency("LYD", "Libyan dinar", L"LD", L"", L"", L"", 100, 1);
    currency_map["LTL"] = mmCurrency("LTL", "Lithuanian litas", L"Lt", L"", L"", L"", 100, 1);
    currency_map["MOP"] = mmCurrency("MOP", "Macanese pataca", L"P", L"", L"", L"", 100, 1);
    currency_map["MKD"] = mmCurrency("MKD", "Macedonian denar", L"", L"", L"", L"", 100, 1);
    currency_map["MGA"] = mmCurrency("MGA", "Malagasy ariary", L"FMG", L"", L"", L"", 100, 1);
    currency_map["MWK"] = mmCurrency("MWK", "Malawian kwacha", L"MK", L"", L"", L"", 1, 1);
    currency_map["MYR"] = mmCurrency("MYR", "Malaysian ringgit", L"RM", L"", L"", L"", 100, 1);
    currency_map["MVR"] = mmCurrency("MVR", "Maldivian rufiyaa", L"Rf", L"", L"", L"", 100, 1);
    currency_map["XOF"] = mmCurrency("XOF", "West African CFA franc", L"CFA", L"", L"", L"", 100, 1);
    currency_map["MRO"] = mmCurrency("MRO", "Mauritanian ouguiya", L"UM", L"", L"", L"", 100, 1);
    currency_map["MUR"] = mmCurrency("MUR", "Mauritian rupee", L"Rs", L"", L"", L"", 1, 1);
    currency_map["MXN"] = mmCurrency("MXN", "Mexican peso", L"$", L"", L"", L"", 100, 1);
    currency_map["MDL"] = mmCurrency("MDL", "Moldovan leu", L"", L"", L"", L"", 100, 1);
    currency_map["MNT"] = mmCurrency("MNT", "Mongolian tugrik",L"₮", L"", L"", L"", 100, 1);
    currency_map["XCD"] = mmCurrency("XCD", "East Caribbean dollar", L"EC$", L"", L"", L"", 100, 1);
    currency_map["MAD"] = mmCurrency("MAD", "Moroccan dirham", L"", L"", L"", L"", 100, 1);
    currency_map["MZM"] = mmCurrency("MZM", "Mozambican metical", L"MTn", L"", L"", L"", 100, 1);
    currency_map["MMK"] = mmCurrency("MMK", "Myanma kyat", L"K", L"", L"", L"", 1, 1);
    currency_map["NAD"] = mmCurrency("NAD", "Namibian dollar", L"N$", L"", L"", L"", 100, 1);
    currency_map["NPR"] = mmCurrency("NPR", "Nepalese rupee", L"NRs", L"", L"", L"", 100, 1);
    currency_map["ANG"] = mmCurrency("ANG", "Netherlands Antillean gulden", L"NAƒ", L"", L"", L"", 100, 1);
    currency_map["XPF"] = mmCurrency("XPF", "CFP franc", L"F", L"", L"", L"", 100, 1);
    currency_map["NZD"] = mmCurrency("NZD", "New Zealand dollar", L"NZ$", L"", L"", L"", 100, 1);
    currency_map["NIO"] = mmCurrency("NIO", "Nicaraguan córdoba", L"C$", L"", L"", L"", 100, 1);
    currency_map["XOF"] = mmCurrency("XOF", "West African CFA franc", L"CFA", L"", L"", L"", 100, 1);
    currency_map["NGN"] = mmCurrency("NGN", "Nigerian naira", L"₦", L"", L"", L"", 100, 1);
    currency_map["NOK"] = mmCurrency("NOK", "Norwegian krone", L"kr", L"", L"", L"", 100, 1);
    currency_map["OMR"] = mmCurrency("OMR", "Omani rial", L"", L"", L"", L"", 100, 1);
    currency_map["PKR"] = mmCurrency("PKR", "Pakistani rupee", L"Rs.", L"", L"", L"", 1, 1);
    currency_map["PAB"] = mmCurrency("PAB", "Panamanian balboa", L"B./", L"", L"", L"", 100, 1);
    currency_map["PGK"] = mmCurrency("PGK", "Papua New Guinean kina", L"K", L"", L"", L"", 100, 1);
    currency_map["PYG"] = mmCurrency("PYG", "Paraguayan guarani", L"", L"", L"", L"", 1, 1);
    currency_map["PEN"] = mmCurrency("PEN", "Peruvian nuevo sol", L"S/.", L"", L"", L"", 100, 1);
    currency_map["PHP"] = mmCurrency("PHP", "Philippine peso", L"₱", L"", L"", L"", 100, 1);
    currency_map["PLN"] = mmCurrency("PLN", "Polish zloty", L"", L"", L"", L"", 100, 1);
    currency_map["QAR"] = mmCurrency("QAR", "Qatari riyal", L"QR", L"", L"", L"", 100, 1);
    currency_map["RON"] = mmCurrency("RON", "Romanian leu", L"L", L"", L"", L"", 100, 1);
    currency_map["RWF"] = mmCurrency("RWF", "Rwandan franc", L"RF", L"", L"", L"", 1, 1);
    currency_map["STD"] = mmCurrency("STD", "São Tomé and Príncipe dobra", L"Db", L"", L"", L"", 100, 1);
    currency_map["SAR"] = mmCurrency("SAR", "Saudi riyal", L"SR", L"", L"", L"", 100, 1);
    currency_map["XOF"] = mmCurrency("XOF", "West African CFA franc", L"CFA", L"", L"", L"", 100, 1);
    currency_map["RSD"] = mmCurrency("RSD", "Serbian dinar", L"din.", L"", L"", L"", 1, 1);
    currency_map["SCR"] = mmCurrency("SCR", "Seychellois rupee", L"SR", L"", L"", L"", 100, 1);
    currency_map["SLL"] = mmCurrency("SLL", "Sierra Leonean leone", L"Le", L"", L"", L"", 100, 1);
    currency_map["SGD"] = mmCurrency("SGD", "Singapore dollar", L"S$", L"", L"", L"", 100, 1);
    currency_map["SBD"] = mmCurrency("SBD", "Solomon Islands dollar", L"SI$", L"", L"", L"", 100, 1);
    currency_map["SOS"] = mmCurrency("SOS", "Somali shilling", L"Sh.", L"", L"", L"", 1, 1);
    currency_map["ZAR"] = mmCurrency("ZAR", "South African rand", L"R", L"", L"", L"", 100, 1);
    currency_map["LKR"] = mmCurrency("LKR", "Sri Lankan rupee", L"Rs", L"", L"", L"", 100, 1);
    currency_map["SHP"] = mmCurrency("SHP", "Saint Helena pound", L"£", L"", L"", L"", 100, 1);
    currency_map["XCD"] = mmCurrency("XCD", "East Caribbean dollar", L"EC$", L"", L"", L"", 100, 1);
    currency_map["SDG"] = mmCurrency("SDG", "Sudanese pound", L"", L"", L"", L"", 100, 1);
    currency_map["SRD"] = mmCurrency("SRD", "Surinamese dollar", L"$", L"", L"", L"", 100, 1);
    currency_map["SZL"] = mmCurrency("SZL", "Swazi lilangeni", L"E", L"", L"", L"", 100, 1);
    currency_map["SEK"] = mmCurrency("SEK", "Swedish krona", L"kr", L"", L"", L"", 100, 1);
    currency_map["CHF"] = mmCurrency("CHF", "Swiss franc", L"Fr.", L"", L"", L"", 100, 1);
    currency_map["SYP"] = mmCurrency("SYP", "Syrian pound", L"", L"", L"", L"", 1, 1);
    currency_map["TWD"] = mmCurrency("TWD", "New Taiwan dollar", L"NT$", L"", L"", L"", 100, 1);
    currency_map["TJS"] = mmCurrency("TJS", "Tajikistani somoni", L"", L"", L"", L"", 100, 1);
    currency_map["TZS"] = mmCurrency("TZS", "Tanzanian shilling", L"", L"", L"", L"", 1, 1);
    currency_map["THB"] = mmCurrency("THB", "Thai baht", L"฿", L"", L"", L"", 100, 1);
    currency_map["XOF"] = mmCurrency("XOF", "West African CFA franc", L"CFA", L"", L"", L"", 100, 1);
    currency_map["TTD"] = mmCurrency("TTD", "Trinidad and Tobago dollar", L"TT$", L"", L"", L"", 100, 1);
    currency_map["TND"] = mmCurrency("TND", "Tunisian dinar", L"DT", L"", L"", L"", 100, 1);
    currency_map["TRY"] = mmCurrency("TRY", "Turkish new lira", L"YTL", L"", L"", L"", 100, 1);
    currency_map["TMT"] = mmCurrency("TMT", "Turkmen manat", L"m", L"", L"", L"", 100, 1);
    currency_map["UGX"] = mmCurrency("UGX", "Ugandan shilling", L"USh", L"", L"", L"", 1, 1);
    currency_map["AED"] = mmCurrency("AED", "UAE dirham", L"", L"", L"", L"", 100, 1);
    currency_map["UYU"] = mmCurrency("UYU", "Uruguayan peso", L"$U", L"", L"", L"", 100, 1);
    currency_map["UZS"] = mmCurrency("UZS", "Uzbekistani som", L"", L"", L"", L"", 1, 1);
    currency_map["VUV"] = mmCurrency("VUV", "Vanuatu vatu", L"VT", L"", L"", L"", 100, 1);
    currency_map["VEB"] = mmCurrency("VEB", "Venezuelan bolivar", L"Bs", L"", L"", L"", 100, 1);
    currency_map["VND"] = mmCurrency("VND", "Vietnamese dong", L"₫", L"", L"", L"", 1, 1);
    currency_map["XPF"] = mmCurrency("XPF", "CFP franc", L"F", L"", L"", L"", 100, 1);
    currency_map["WST"] = mmCurrency("WST", "Samoan tala", L"WS$", L"", L"", L"", 100, 1);
    currency_map["YER"] = mmCurrency("YER", "Yemeni rial", L"", L"", L"", L"", 1, 1);
    currency_map["ZMK"] = mmCurrency("ZMK", "Zambian kwacha", L"ZK", L"", L"", L"", 1, 1);
    currency_map["ZWR"] = mmCurrency("ZWR", "Zimbabwean dollar", L"Z$", L"", L"", L"", 100, 1);

    return currency_map;
}
