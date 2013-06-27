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

std::map<wxString, mmCurrency> mmCurrency::currency_map_() 
{
    std::map<wxString, mmCurrency> currency_map;
    currency_map["GBP"] = mmCurrency("GBP", "UK Pound", L"£", "", "Pound", "Pence", 100, 1);
    currency_map["RUB"] = mmCurrency("RUB", "Russian Ruble", "", "р", "руб.", "коп.", 100, 1);
    currency_map["INR"] = mmCurrency("INR", "Indian Rupee", L"₹", "", "rupee", "", 100, 1);

    /*
    Russian Ruble         ;; р.;,;;р.;к.;100;1;RUB
    Indian Rupee          ;;₹;,;;rupee;;100;1;INR
    New Taiwan Dollar     ;NT$;;,;.;dollar;;100;1;TWD
    Ukraine Hryvna        ;;₴;;,;Грн.;к.;100;1;UAH
    Swiss Franc           ;;Fr.;.;,;franc;centimes;100;1;CHF
    East Caribbean Dollars;EC$;;.;,;dollar;cents;100;1;XCD
    */
    return currency_map;
}
