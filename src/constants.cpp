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
#include <wx/utils.h>
#include <wx/wxsqlite3.h>
#include "lua.h"
#ifdef MMEX_WEBSERVER
#include "mongoose/mongoose.h"
#endif
#include "rapidjson/rapidjson.h"
#include "DB_Upgrade.h" /* for dbLatestVersion */
#include <curl/curl.h>

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
const wxString mmex::DEFDATEFORMAT = "%Y-%m-%d"; //ISO 8601
const wxString mmex::DEFDELIMTER = ",";

const wxString mmex::getProgramName()
{
    return wxString("Money Manager Ex");
}
const wxString mmex::getTitleProgramVersion()
{
    return _("Version: ") + mmex::version::string;
}

const wxString mmex::getProgramCopyright()
{
#define COMPILE_YEAR ( (__DATE__[ 7] - '0') * 1000 + \
                       (__DATE__[ 8] - '0') *  100 + \
                       (__DATE__[ 9] - '0') *   10 + \
                       (__DATE__[10] - '0') )
    return wxString::Format("(c) 2005-%d Madhan Kanagavel", COMPILE_YEAR);
}
const wxString mmex::getProgramDescription()
{
    const wxString bull = L" \u2022 ";
    wxString description;
    wxString curl = curl_version();
        curl.Replace(" ","\n" + bull);
        curl.Replace("/", " ");

    description << mmex::getTitleProgramVersion() << "\n"
        << _("Database version: ") << dbLatestVersion
#if WXSQLITE3_HAVE_CODEC
        << " (" << wxSQLite3Cipher::GetCipherName(wxSQLite3Cipher::GetGlobalCipherDefault()) << ")"
#endif
        << "\n"
#ifdef GIT_COMMIT_HASH
        << _("Git commit: ") << GIT_COMMIT_HASH
        << " (" << GIT_COMMIT_DATE << ")\n"
#endif
#ifdef GIT_BRANCH
        << _("Git branch: ") << GIT_BRANCH << "\n"
#endif

        << "\n" << _("MMEX is using the following support products:") << "\n"
        << bull + wxVERSION_STRING
        << wxString::Format(" (%s %d.%d)\n",
            wxPlatformInfo::Get().GetPortIdName(),
            wxPlatformInfo::Get().GetToolkitMajorVersion(),
            wxPlatformInfo::Get().GetToolkitMinorVersion())
        << bull + wxSQLITE3_VERSION_STRING
        << " (SQLite " << wxSQLite3Database::GetVersion() << ")\n"
#ifdef MMEX_WEBSERVER
        << bull + "Mongoose " << MG_VERSION << "\n"
#endif
        << bull + "RapidJSON " << RAPIDJSON_VERSION_STRING << "\n"
        << bull + LUA_RELEASE << "\n"
        << bull + curl << "\n\n"

        << _("Build on") << " " << __DATE__ << " " << __TIME__  << " " << _("with:") << "\n"
        << bull + CMAKE_VERSION << "\n"
        << bull + MAKE_VERSION << "\n"
        << bull + GETTEXT_VERSION << "\n"
#if defined(_MSC_VER)
#ifdef VS_VERSION
        << bull + "Microsoft Visual Studio " + VS_VERSION << "\n"
#endif
        << bull + "Microsoft Visual C++ " + CXX_VERSION << "\n"
#elif defined(__clang__)
        << bull + "Clang " + __VERSION__ << "\n"
#elif (defined(__GNUC__) || defined(__GNUG__)) && !(defined(__clang__) || defined(__INTEL_COMPILER))
        << bull + "GCC " + __VERSION__ << "\n"
#endif
#ifdef CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION
        << bull + CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION << "\n"
#endif
#ifdef LINUX_DISTRO_STRING
        << bull + LINUX_DISTRO_STRING << "\n"
#endif

        << "\n" << _("Running on:") << "\n"
#ifdef __LINUX__
        << bull + wxGetLinuxDistributionInfo().Description
        << " \"" << wxGetLinuxDistributionInfo().CodeName << "\"\n"
#endif
        << bull + wxGetOsDescription() << "\n"
        << bull + wxPlatformInfo::Get().GetDesktopEnvironment()
        << " " << wxLocale::GetLanguageName(wxLocale::GetSystemLanguage())
        << " (" << wxLocale::GetSystemEncodingName() << ")\n"
        << wxString::Format(bull + "%ix%i %ibit %ix%ippi\n",
            wxGetDisplaySize().GetX(),
            wxGetDisplaySize().GetY(),
            wxDisplayDepth(),
            wxGetDisplayPPI().GetX(),
            wxGetDisplayPPI().GetY())
    ;
    description.RemoveLast();

    return description;
}

const wxString mmex::getCaption(const wxString& caption)
{
    return mmex::getProgramName() + (caption.IsEmpty() ? wxGetEmptyString() : " - " + caption);
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

const wxString mmex::weblink::GA = "https://www.google-analytics.com/collect?";
const wxString mmex::weblink::WebSite = mmex::weblink::addReferralToURL("https://www.moneymanagerex.org", "Website");
const wxString mmex::weblink::Releases = "https://api.github.com/repos/moneymanagerex/moneymanagerex/releases";
const wxString mmex::weblink::News = mmex::weblink::addReferralToURL("https://www.moneymanagerex.org/news", "News");
const wxString mmex::weblink::NewsRSS = "http://www.moneymanagerex.org/news?format=feed";
const wxString mmex::weblink::Forum = mmex::weblink::addReferralToURL("https://forum.moneymanagerex.org", "Forum");
const wxString mmex::weblink::Wiki = "http://wiki.moneymanagerex.org";
const wxString mmex::weblink::GitHub = "https://github.com/moneymanagerex/moneymanagerex";
const wxString mmex::weblink::YouTube = "https://www.youtube.com/user/moneymanagerex";
const wxString mmex::weblink::Slack = "http://slack.moneymanagerex.org/";
const wxString mmex::weblink::BugReport = "https://github.com/moneymanagerex/moneymanagerex/issues";
const wxString mmex::weblink::Donate = "https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=moneymanagerex%40gmail%2ecom&lc=US&item_name=MoneyManagerEx&no_note=0&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHostedGuest";
const wxString mmex::weblink::SquareCashGuan = "https://cash.me/$guanlisheng/1";
const wxString mmex::weblink::Twitter = "https://twitter.com/MoneyManagerEx";
const wxString mmex::weblink::Facebook = "https://www.facebook.com/MoneyManagerEx/";

// Yahoo API
const wxString mmex::weblink::YahooQuotes = "https://query1.finance.yahoo.com/v7/finance/quote?symbols=%s&fields=regularMarketPrice,currency,shortName";
/*"ValidRanges":["1d","5d","1mo","3mo","6mo","1y","2y","5y","10y","ytd","max"]
   Valid intervals: [1m, 2m, 5m, 15m, 30m, 60m, 90m, 1h, 1d, 5d, 1wk, 1mo, 3mo]*/
const wxString mmex::weblink::YahooQuotesHistory = "https://query1.finance.yahoo.com/v8/finance/chart/%s?range=%s&interval=%s&fields=currency";

//CoinCap API
const wxString mmex::weblink::CoinCap = "https://coincap.io/front";
const wxString mmex::weblink::CoinCapHistory = "https://coincap.io/history/365day/%s";

//
const wxString mmex::weblink::GooglePlay = "https://play.google.com/store/apps/details?id=com.money.manager.ex";
const wxString mmex::weblink::WebApp = "https://github.com/moneymanagerex/web-money-manager-ex";

// Will display the stock page when using Looks up the current value
const wxString mmex::weblink::DefStockUrl = "https://finance.yahoo.com/echarts?s=%s";
// Looks up the current value
// const wxChar *const mmex::DEFSTOCKURL = "https://finance.yahoo.com/lookup?s=%s";

// Using google: To specify the exchange, use exch:code
// Using yahoo: To specify the exchange, use code.exch
// const wxChar *const mmex::DEFSTOCKURL = "https://www.google.com/finance?q=%s";

//US Dollar (USD) in Euro (EUR) Chart
//https://www.google.com/finance?q=CURRENCY%3AUSD

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
const wxString VIEW_TRANS_SINCE_STATEMENT_STR = wxTRANSLATE("View Since Statement Date");

const wxString VIEW_ACCOUNTS_ALL_STR = wxTRANSLATE("All");
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
