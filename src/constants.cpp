/*******************************************************
Copyright (C) 2009 VaDiM

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

#include "constants.h"
#include <wx/string.h>
#include <wx/filefn.h>
#include "lua.hpp"
#include "mongoose/mongoose.h"

/*************************************************************************
 MMEX_VERSION
 Update the version definition for the program as follows:
 Version Format = MAJOR.MINOR.PATCH, increment the:
 1. MAJOR version when you make incompatible API changes,
 2. MINOR version when you add functionality in a backwards-compatible manner, and
 3. PATCH version when you make backwards-compatible bug fixes.
 Ref: http://semver.org

 Alpha, Beta, RC  = -1 (Stable) won't add any suffix
 Alpha, Beta, RC  = 0 (Unstable) will add suffix to version without number
 Alpha, Beta, RC  > 0 (Unstable) will add suffix to version with number

 For Internet Format for update checking read in util.cpp
 *************************************************************************/
const int mmex::version::Major = 1;
const int mmex::version::Minor = 3;
const int mmex::version::Patch = 1;
const int mmex::version::Alpha = -1;
const int mmex::version::Beta  = -1;
const int mmex::version::RC    = -1;
const wxString mmex::version::string = mmex::version::generateProgramVersion(mmex::version::Major, mmex::version::Minor, mmex::version::Patch
    ,mmex::version::Alpha, mmex::version::Beta, mmex::version::RC);

const wxString mmex::version::generateProgramVersion(int vMajor, int vMinor, int vPatch, int vAlpha, int vBeta, int vRC)
{
    wxString suffix = "";
    if (vAlpha >= 0 || vBeta >= 0 || vRC >= 0)
    {
        if (vAlpha >= 0)
            suffix = vAlpha < 1 ? "-Alpha" : wxString::Format("-Alpha.%i", vAlpha);
        if (Beta >= 0)
            suffix = vBeta < 1 ? "-Beta" : wxString::Format("-Beta.%i", vBeta);
        if (vRC >= 0)
            suffix = vRC < 1 ? "-RC" : wxString::Format("-RC.%i", vRC);
    }
    return wxString::Format("%i.%i.%i%s", vMajor, vMinor, vPatch, suffix);
}

/* End version namespace*/

const wxSizerFlags g_flagsH = wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL).Border(wxALL, 5);
const wxSizerFlags g_flagsV = wxSizerFlags().Align(wxALIGN_LEFT).Border(wxALL, 5);
const wxSizerFlags g_flagsBorder1H = wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL).Border(wxALL, 1);
const wxSizerFlags g_flagsBorder1V = wxSizerFlags().Align(wxALIGN_LEFT).Border(wxALL, 1);
const wxSizerFlags g_flagsCenter = wxSizerFlags().Align(wxALIGN_CENTER).Border(wxALL, 5);
const wxSizerFlags g_flagsExpand = wxSizerFlags().Align(wxALIGN_LEFT | wxEXPAND).Border(wxALL, 5).Proportion(1);
const wxSizerFlags g_flagsExpandBorder1 = wxSizerFlags().Align(wxALIGN_LEFT | wxEXPAND).Border(wxALL, 1).Proportion(1);

const wxString g_CancelLabel =
#if defined(__APPLE__)
    wxTRANSLATE("Cancel");
#else
    wxTRANSLATE("&Cancel ");
#endif
const wxString g_CloseLabel =
#if defined(__APPLE__)
    wxTRANSLATE("Close");
#else
    wxTRANSLATE("&Close ");
#endif
//---------------------------------------------------------------------------
const int mmex::MIN_DATAVERSION = 2;
const wxString mmex::DATAVERSION = "3";
const wxString mmex::DEFDATEFORMAT =  "%Y-%m-%d"; //ISO 8601
const wxString mmex::DEFDELIMTER = ",";

const wxString mmex::getProgramName()
{
    return wxString("Money Manager Ex");
}
const wxString mmex::getTitleProgramVersion()
{
    return wxString::Format(_("Version: %s"), mmex::version::string);
}

const wxString mmex::getProgramCopyright()
{
    return wxString::Format("(c) 2005-%d Madhan Kanagavel", wxDateTime::Now().GetCurrentYear());
}
const wxString mmex::getProgramDescription()
{
    wxString description;
    description << _("MMEX is using the following support products") << ":\n"
        << "======================================\n"
        << wxVERSION_STRING << "\n"
        << "SQLite3 " << wxSQLite3Database::GetVersion() << "\n"
        << wxSQLITE3_VERSION_STRING << "\n"
        << "Mongoose " << MG_VERSION << "\n"
        << LUA_VERSION << "\n";
#if defined(_MSC_VER)
    description << "Microsoft Visual Studio " << _MSC_VER;
#elif defined(__clang__)
    description << "Clang/LLVM " << __VERSION__;
#elif (defined(__GNUC__) || defined(__GNUG__)) && !(defined(__clang__) || defined(__INTEL_COMPILER))
    description << "GNU GCC/G++ " << __VERSION__;
#endif

    return description;
}

/* Namespace weblink */
const wxString mmex::weblink::addReferralToURL(const wxString& BaseURL, const wxString& CampSource)
{
    /*
    With Google analytics it's possible to send some data in URL
    to divide direct access from access through desktop app links
    https://support.google.com/analytics/answer/1033867?hl=en
    */
    
    const wxString url = wxString::Format("%s?utm_campaign=Application_Desktop&utm_source=%s&utm_medium=MMEX_v%s"
        , BaseURL, CampSource, mmex::version::string);

    return url;
}

const wxString mmex::weblink::WebSite = mmex::weblink::addReferralToURL("http://www.moneymanagerex.org", "Website");
const wxString mmex::weblink::Update = wxString::Format("http://www.moneymanagerex.org/version.php?Version=%s", mmex::version::string);
const wxString mmex::weblink::UpdateLinks = wxString::Format("http://www.moneymanagerex.org/version.php?Version=%s&Links=true", mmex::version::string);
const wxString mmex::weblink::Changelog = wxString::Format("http://www.moneymanagerex.org/version.php?Version=%s&ChangeLog=", mmex::version::string);
const wxString mmex::weblink::Download = mmex::weblink::addReferralToURL("http://www.moneymanagerex.org/download", "Download");
const wxString mmex::weblink::News = mmex::weblink::addReferralToURL("http://www.moneymanagerex.org/news", "News");
const wxString mmex::weblink::NewsRSS = "http://www.moneymanagerex.org/news?format=feed";
const wxString mmex::weblink::Forum = mmex::weblink::addReferralToURL("http://forum.moneymanagerex.org", "Forum");
const wxString mmex::weblink::Wiki = "http://wiki.moneymanagerex.org";
const wxString mmex::weblink::BugReport = "http://bugreport.moneymanagerex.org";
const wxString mmex::weblink::Donate = "https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=moneymanagerex%40gmail%2ecom&lc=US&item_name=MoneyManagerEx&no_note=0&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHostedGuest";
const wxString mmex::weblink::Twitter = "https://twitter.com/MoneyManagerEx";
const wxString mmex::weblink::Facebook = "http://www.facebook.com/pages/Money-Manager-Ex/242286559144586";
// https://greenido.wordpress.com/2009/12/22/yahoo-finance-hidden-api/
const wxString mmex::weblink::YahooQuotes = "http://download.finance.yahoo.com/d/quotes.csv?s=%s&f=sl1c4n&e=.csv";
const wxString mmex::weblink::YahooQuotesHistory = "http://ichart.finance.yahoo.com/table.csv?s=";
const wxString mmex::weblink::BceCurrencyHistory = "http://www.ecb.europa.eu/stats/eurofxref/eurofxref-hist.xml";
//const wxString mmex::weblink::BceCurrencyHistory = "http://192.168.6.1/eurofxref-hist.xml"; // used for debug
const wxString mmex::weblink::GooglePlay = "https://play.google.com/store/apps/details?id=com.money.manager.ex";
const wxString mmex::weblink::WebApp = "https://github.com/moneymanagerex/web-money-manager-ex";

// Will display the stock page when using Looks up the current value
const wxString mmex::weblink::DefStockUrl = "http://finance.yahoo.com/echarts?s=%s";
// Looks up the current value
// const wxChar *const mmex::DEFSTOCKURL = "http://finance.yahoo.com/lookup?s=%s";

// Using google: To specify the exchange, use exch:code
// Using yahoo: To specify the exchange, use code.exch
// const wxChar *const mmex::DEFSTOCKURL = "http://www.google.com/finance?q=%s";

//US Dollar (USD) in Euro (EUR) Chart
//http://www.google.com/finance?q=CURRENCY%3AUSD

/* End namespace weblink */

const wxString LANGUAGE_PARAMETER = "LANGUAGE";
const wxString INIDB_USE_TRANSACTION_SOUND = "USETRANSSOUND";
const wxString INIDB_USE_ORG_DATE_COPYPASTE = "USEORIGDATEONCOPYPASTE";
const wxString INIDB_SEND_USAGE_STATS = "SENDUSAGESTATS";

const wxString VIEW_TRANS_ALL_STR            = wxTRANSLATE("View All Transactions");
const wxString VIEW_TRANS_TODAY_STR          = wxTRANSLATE("View Today");
const wxString VIEW_TRANS_CURRENT_MONTH_STR  = wxTRANSLATE("View Current Month");
const wxString VIEW_TRANS_LAST_30_DAYS_STR   = wxTRANSLATE("View Last 30 days");
const wxString VIEW_TRANS_LAST_90_DAYS_STR   = wxTRANSLATE("View Last 90 days");
const wxString VIEW_TRANS_LAST_MONTH_STR     = wxTRANSLATE("View Last Month");
const wxString VIEW_TRANS_LAST_3MONTHS_STR   = wxTRANSLATE("View Last 3 Months");
const wxString VIEW_TRANS_LAST_12MONTHS_STR  = wxTRANSLATE("View Last 12 Months");
const wxString VIEW_TRANS_CURRENT_YEAR_STR   = wxTRANSLATE("View Current Year");
const wxString VIEW_TRANS_CURRENT_FIN_YEAR_STR = wxTRANSLATE("View Current Financial Year");
const wxString VIEW_TRANS_LAST_YEAR_STR      = wxTRANSLATE("View Last Year");
const wxString VIEW_TRANS_LAST_FIN_YEAR_STR  = wxTRANSLATE("View Last Financial Year");

const wxString VIEW_ACCOUNTS_ALL_STR = wxTRANSLATE("ALL");
const wxString VIEW_ACCOUNTS_OPEN_STR = wxTRANSLATE("Open");
const wxString VIEW_ACCOUNTS_CLOSED_STR = wxTRANSLATE("Closed");
const wxString VIEW_ACCOUNTS_FAVORITES_STR = wxTRANSLATE("Favorites");

const wxString INIDB_BUDGET_FINANCIAL_YEARS       = "BUDGET_FINANCIAL_YEARS";
const wxString INIDB_BUDGET_INCLUDE_TRANSFERS     = "BUDGET_INCLUDE_TRANSFERS";
const wxString INIDB_BUDGET_SETUP_WITHOUT_SUMMARY = "BUDGET_SETUP_WITHOUT_SUMMARY";
const wxString INIDB_BUDGET_SUMMARY_WITHOUT_CATEG = "BUDGET_SUMMARY_WITHOUT_CATEGORIES";
const wxString INIDB_IGNORE_FUTURE_TRANSACTIONS   = "IGNORE_FUTURE_TRANSACTIONS";

const wxString ATTACHMENTS_FOLDER_DOCUMENTS = "%DOCUMENTS%";
const wxString ATTACHMENTS_FOLDER_DATABASE = "%DATABASE%";
const wxString ATTACHMENTS_FOLDER_USERPROFILE = "%USERPROFILE%";
const wxString ATTACHMENTS_FOLDER_APPDATA = "%APPDATA%";

const wxString INIDB_NEWS_LAST_READ_DATE = "NEWS_LAST_READ_DATE";
