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

 The definition: #define MMEX_BUILD_TYPE_RELEASE_CANDIDATE
 is used to control the release type. This will add a suffix: -RCx

 Release Build:
 Remove the definition: MMEX_BUILD_TYPE_RELEASE_CANDIDATE
 The release candidate number is ignored.

 Release Candidate Build:
 Include the definition: MMEX_BUILD_TYPE_RELEASE_CANDIDATE
 Increment MMEX_VERSION_DEVELOPMENT_COUNT for the release candidate version.

 For Internet Format for update checking read in util.cpp
 *************************************************************************/
const wxString MMEX_VERSION = "1.1.0";

//#define MMEX_BUILD_TYPE_RELEASE_CANDIDATE
const int MMEX_VERSION_DEVELOPMENT_COUNT = 3;

const wxSizerFlags g_flags = wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL).Border(wxALL, 5);
const wxSizerFlags g_flagsExpand = wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND).Border(wxALL, 5).Proportion(1);

//---------------------------------------------------------------------------
const int mmex::MIN_DATAVERSION = 2;
const wxString mmex::DATAVERSION = "2";
const wxString mmex::DEFDATEFORMAT =  "%Y-%m-%d"; //ISO 8601
const wxString mmex::DEFDELIMTER = ",";

const wxString mmex::getProgramName()
{
    return wxString("MoneyManagerEx");
}
const wxString mmex::getTitleProgramVersion()
{
    return _("Version: ") + mmex::getProgramVersion();
}
const wxString mmex::getProgramVersion()
{
    wxString release_candidate;
    #ifdef MMEX_BUILD_TYPE_RELEASE_CANDIDATE
        release_candidate = "-RC";
        release_candidate << MMEX_VERSION_DEVELOPMENT_COUNT;
    #endif

    return wxString::Format("%s%s", MMEX_VERSION, release_candidate);
}
const wxString mmex::getProgramCopyright()
{
    return "(c) 2005-2014 Madhan Kanagavel";
}
const wxString mmex::getProgramDescription()
{
    wxString description;
    description << _("MMEX is using the following support products") << ":\n"
        << "======================================\n"
        << wxVERSION_STRING << "\n"
        << "SQLite3 " << wxSQLite3Database::GetVersion() << "\n"
        << wxSQLITE3_VERSION_STRING << "\n"
        << "Mongoose " << MONGOOSE_VERSION << "\n"
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
    wxString url = BaseURL;
    if (BaseURL.find("/",true) > 5)
        url += "/";
    
    url += "?";
    url += "utm_campaign=Application_Desktop";

    url += "&";
    url += "utm_source=" + CampSource;

    url += "&";
    url += "utm_medium=MMEX_v" + mmex::getProgramVersion();

    return url;
}

const wxString mmex::weblink::WebSite = mmex::weblink::addReferralToURL("http://www.moneymanagerex.org", "Website");
const wxString mmex::weblink::Update = "http://www.moneymanagerex.org/version.html";
const wxString mmex::weblink::UsageStats = "http://usagestats.moneymanagerex.org/API/main_stats_v1.php";
const wxString mmex::weblink::Download = mmex::weblink::addReferralToURL("http://www.moneymanagerex.org/download", "Download");
const wxString mmex::weblink::News = mmex::weblink::addReferralToURL("http://www.moneymanagerex.org/news", "News");
const wxString mmex::weblink::NewsRSS = "http://www.moneymanagerex.org/news?format=feed";
const wxString mmex::weblink::Forum = mmex::weblink::addReferralToURL("http://forum.moneymanagerex.org", "Forum");
const wxString mmex::weblink::Wiki = "http://wiki.moneymanagerex.org";
const wxString mmex::weblink::BugReport = "http://bugreport.moneymanagerex.org";
const wxString mmex::weblink::Donate = "https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=moneymanagerex%40gmail%2ecom&lc=US&item_name=MoneyManagerEx&no_note=0&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHostedGuest";
const wxString mmex::weblink::Twitter = "https://twitter.com/MoneyManagerEx";
const wxString mmex::weblink::Facebook = "http://www.facebook.com/pages/Money-Manager-Ex/242286559144586";
const wxString mmex::weblink::YahooQuotes = "http://download.finance.yahoo.com/d/quotes.csv?s=%s&f=sl1c4n&e=.csv";

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

const wxString VIEW_TRANS_ALL_STR            = wxTRANSLATE("View All Transactions");
const wxString VIEW_TRANS_TODAY_STR          = wxTRANSLATE("View Today");
const wxString VIEW_TRANS_CURRENT_MONTH_STR  = wxTRANSLATE("View Current Month");
const wxString VIEW_TRANS_LAST_30_DAYS_STR   = wxTRANSLATE("View Last 30 days");
const wxString VIEW_TRANS_LAST_90_DAYS_STR   = wxTRANSLATE("View Last 90 days");
const wxString VIEW_TRANS_LAST_MONTH_STR     = wxTRANSLATE("View Last Month");
const wxString VIEW_TRANS_LAST_3MONTHS_STR   = wxTRANSLATE("View Last 3 Months");
const wxString VIEW_TRANS_LAST_12MONTHS_STR  = wxTRANSLATE("View Last 12 Months");
const wxString VIEW_TRANS_CURRENT_YEAR_STR   = wxTRANSLATE("View Current Year");

const wxString  VIEW_ACCOUNTS_ALL_STR       = "ALL";
const wxString  VIEW_ACCOUNTS_OPEN_STR      = "Open";
const wxString  VIEW_ACCOUNTS_FAVORITES_STR = "Favorites";

const wxString INIDB_BUDGET_FINANCIAL_YEARS       = "BUDGET_FINANCIAL_YEARS";
const wxString INIDB_BUDGET_INCLUDE_TRANSFERS     = "BUDGET_INCLUDE_TRANSFERS";
const wxString INIDB_BUDGET_SETUP_WITHOUT_SUMMARY = "BUDGET_SETUP_WITHOUT_SUMMARY";
const wxString INIDB_BUDGET_SUMMARY_WITHOUT_CATEG = "BUDGET_SUMMARY_WITHOUT_CATEGORIES";
const wxString INIDB_IGNORE_FUTURE_TRANSACTIONS   = "IGNORE_FUTURE_TRANSACTIONS";

const wxString ATTACHMENTS_FOLDER_DOCUMENTS = "%DOCUMENTS%";
const wxString ATTACHMENTS_FOLDER_DATABASE = "%DATABASE%";
const wxString ATTACHMENTS_FOLDER_USERPROFILE = "%USERPROFILE%";
const wxString ATTACHMENTS_FOLDER_APPDATA = "%APPDATA%";
