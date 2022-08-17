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
#include "build.h"
#include <wx/string.h>
#include <wx/filefn.h>
#include <curl/curl.h>
#include "rapidjson/rapidjson.h"
#include "db/DB_Upgrade.h"

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

const wxString mmex::version::string = mmex::version::getProgramVersion();

bool mmex::version::isStable()
{
    const wxString ver = wxString() << MMEX_VERSION_WITH_UNSTABLE;
    wxRegEx pattern("(Beta|Alpha|RC)");
    bool is_stable = !pattern.Matches(ver);
    return is_stable;
}

const wxString mmex::version::getProgramVersion()
{
    const wxString ver = wxString() << MMEX_VERSION_WITH_UNSTABLE;
    return ver;
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
int mmex::MIN_DATAVERSION = 2;
const wxString mmex::DATAVERSION = "3";
const wxString mmex::DEFDATEFORMAT =  "%Y-%m-%d"; //ISO 8601
const wxString mmex::DEFDELIMTER = ",";

const wxString mmex::getProgramName()
{
    return wxString("Money Manager Ex");
}
const wxString mmex::getTitleProgramVersion()
{
    const wxString architecture =
#if defined(_WIN64) || defined(__x86_64__)
        "64-bit";
#else
        "";
#endif

    return wxString::Format("%s %s", mmex::version::string, architecture);
}

int mmex::version::getDbLatestVersion()
{
    return dbLatestVersion;
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

const wxString mmex::weblink::GA             = "http://www.google-analytics.com/collect?";
const wxString mmex::weblink::WebSite        = mmex::weblink::addReferralToURL("https://moneymanagerex.org", "Website");
const wxString mmex::weblink::Releases       = "https://api.github.com/repos/moneymanagerex/moneymanagerex/releases";
const wxString mmex::weblink::Download       = mmex::weblink::addReferralToURL("https://moneymanagerex.org/docs/downloads/latest", "Download");
const wxString mmex::weblink::News           = mmex::weblink::addReferralToURL("https://moneymanagerex.org/blog/", "News");
const wxString mmex::weblink::NewsRSS        = "https://moneymanagerex.org/blog/index.xml";
const wxString mmex::weblink::Forum          = mmex::weblink::addReferralToURL("http://forum.moneymanagerex.org", "Forum");
const wxString mmex::weblink::Wiki           = "https://github.com/moneymanagerex/moneymanagerex/wiki";
const wxString mmex::weblink::BugReport      = "https://github.com/moneymanagerex/moneymanagerex/issues";
const wxString mmex::weblink::Donate         = "https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=moneymanagerex%40gmail%2ecom&lc=US&item_name=MoneyManagerEx&no_note=0&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donateCC_LG%2egif%3aNonHostedGuest";
const wxString mmex::weblink::Twitter        = "https://twitter.com/MoneyManagerEx";
const wxString mmex::weblink::Facebook       = "https://www.facebook.com/pages/Money-Manager-Ex/242286559144586";
const wxString mmex::weblink::SquareCashGuan = "https://cash.me/$guanlisheng/1";
const wxString mmex::weblink::GitHub         = "https://github.com/moneymanagerex/moneymanagerex";
const wxString mmex::weblink::GitHubLogin    = "https://github.com/login";
const wxString mmex::weblink::YouTube        = "https://www.youtube.com/user/moneymanagerex";
const wxString mmex::weblink::Slack          = "https://moneymanagerex.slack.com/"; //"http://slack.moneymanagerex.org/";
const wxString mmex::weblink::GooglePlay     = "https://android.moneymanagerex.org/";
const wxString mmex::weblink::WebApp         = "https://github.com/moneymanagerex/web-money-manager-ex";
const wxString mmex::weblink::Chiark         = "https://www.chiark.greenend.org.uk/~sgtatham/bugs.html";
const wxString mmex::weblink::Crowdin        = "https://crowdin.com/project/moneymanagerex/";

// Will display the stock page when using Looks up the current value
const wxString mmex::weblink::DefStockUrl = "http://finance.yahoo.com/echarts?s=%s";

// Yahoo API
const wxString mmex::weblink::YahooQuotes = "https://query1.finance.yahoo.com/v7/finance/quote?symbols=%s&fields=regularMarketPrice,currency,shortName";
/*"ValidRanges":["1d","5d","1mo","3mo","6mo","1y","2y","5y","10y","ytd","max"]
   Valid intervals: [1m, 2m, 5m, 15m, 30m, 60m, 90m, 1h, 1d, 5d, 1wk, 1mo, 3mo]*/
const wxString mmex::weblink::YahooQuotesHistory = "https://query1.finance.yahoo.com/v8/finance/chart/%s?%s&fields=currency";

// coincap asset search by symbol and id 
const wxString mmex::weblink::CoinCapSearch = "http://api.coincap.io/v2/assets?search=%s";

// coincap asset price history, all values in USD
// Valid intervals: [m1, m5, m15, m30, h1, h2, h6, h12, d1]
const wxString mmex::weblink::CoinCapHistory = "http://api.coincap.io/v2/assets/%s/history?interval=%s&start=%lld&end=%lld";

/* End namespace weblink */

const wxString LANGUAGE_PARAMETER = "LANGUAGE";
const wxString INIDB_USE_CURRENCY_HISTORY = "USECURRENCYHISTORY";
const wxString INIDB_USE_TRANSACTION_SOUND = "USETRANSSOUND";
const wxString INIDB_USE_ORG_DATE_COPYPASTE = "USEORIGDATEONCOPYPASTE";
const wxString INIDB_USE_ORG_DATE_DUPLICATE = "USEORIGDATEONDUPLICATE";
const wxString INIDB_SEND_USAGE_STATS = "SENDUSAGESTATS";
const wxString INIDB_CHECK_NEWS = "CHECKNEWS";

const wxString VIEW_TRANS_FILTER_DIALOG_STR  = wxTRANSLATE("Advanced Filter");
const wxString VIEW_TRANS_ALL_STR            = wxTRANSLATE("View All Transactions");
const wxString VIEW_TRANS_TODAY_STR          = wxTRANSLATE("View Today");
const wxString VIEW_TRANS_CURRENT_MONTH_STR  = wxTRANSLATE("View Current Month");
const wxString VIEW_TRANS_LAST_30_DAYS_STR   = wxTRANSLATE("View Last 30 days");
const wxString VIEW_TRANS_LAST_90_DAYS_STR   = wxTRANSLATE("View Last 90 days");
const wxString VIEW_TRANS_LAST_MONTH_STR     = wxTRANSLATE("View Last Month");
const wxString VIEW_TRANS_LAST_3MONTHS_STR   = wxTRANSLATE("View Last 3 Months");
const wxString VIEW_TRANS_LAST_12MONTHS_STR  = wxTRANSLATE("View Last 12 Months");
const wxString VIEW_TRANS_CURRENT_YEAR_STR   = wxTRANSLATE("View Current Year");
const wxString VIEW_TRANS_CRRNT_FIN_YEAR_STR = wxTRANSLATE("View Current Financial Year");
const wxString VIEW_TRANS_LAST_YEAR_STR      = wxTRANSLATE("View Last Year");
const wxString VIEW_TRANS_LAST_FIN_YEAR_STR  = wxTRANSLATE("View Last Financial Year");
const wxString VIEW_TRANS_SINCE_STATEMENT_STR = wxTRANSLATE("View Since Statement Date");

const wxString VIEW_ACCOUNTS_ALL_STR = wxTRANSLATE("ALL");
const wxString VIEW_ACCOUNTS_OPEN_STR = wxTRANSLATE("Open");
const wxString VIEW_ACCOUNTS_CLOSED_STR = wxTRANSLATE("Closed");
const wxString VIEW_ACCOUNTS_FAVORITES_STR = wxTRANSLATE("Favorites");

const wxString INIDB_HIDE_SHARE_ACCOUNTS          = "HIDE_SHARE_ACCOUNTS";
const wxString INIDB_BUDGET_FINANCIAL_YEARS       = "BUDGET_FINANCIAL_YEARS";
const wxString INIDB_BUDGET_INCLUDE_TRANSFERS     = "BUDGET_INCLUDE_TRANSFERS";
const wxString INIDB_BUDGET_SETUP_WITHOUT_SUMMARY = "BUDGET_SETUP_WITHOUT_SUMMARY";
const wxString INIDB_BUDGET_SUMMARY_WITHOUT_CATEG = "BUDGET_SUMMARY_WITHOUT_CATEGORIES";
const wxString INIDB_IGNORE_FUTURE_TRANSACTIONS   = "IGNORE_FUTURE_TRANSACTIONS";
const wxString INIDB_SHOW_TOOLTIPS                = "IGNORE_SHOW_TOOLTIPS";
const wxString INIDB_SHOW_MONEYTIPS               = "IGNORE_SHOW_MONEYTIPS";

const wxString ATTACHMENTS_FOLDER_DOCUMENTS = "%DOCUMENTS%";
const wxString ATTACHMENTS_FOLDER_DATABASE = "%DATABASE%";
const wxString ATTACHMENTS_FOLDER_USERPROFILE = "%USERPROFILE%";
const wxString ATTACHMENTS_FOLDER_APPDATA = "%APPDATA%";

const wxString INIDB_NEWS_LAST_READ_DATE = "NEWS_LAST_READ_DATE";


const wxString g_fiat_curr()
{
    return "ADP|AED|AFA|AFN|ALK|ALL|AMD|ANG|AOA|AOK|AON|AOR|ARA|ARP|ARS|ARY|ATS|AUD|AWG|AYM|AZM|AZN|BAD|"
           "BAM|BBD|BDT|BEC|BEF|BEL|BGJ|BGK|BGL|BGN|BHD|BIF|BMD|BND|BOB|BOP|BRB|BRC|BRE|BRL|BRN|BRR|BSD|"
           "BTN|BUK|BWP|BYB|BYN|BYR|BZD|CAD|CDF|CHC|CHF|CLP|CNY|COP|CRC|CSD|CSJ|CSK|CUC|CUP|CVE|CYP|CZK|"
           "DDM|DEM|DJF|DKK|DOP|DZD|ECS|ECV|EEK|EGP|ERN|ESA|ESB|ESP|ETB|EUR|FIM|FJD|FKP|FRF|GBP|GEK|GEL|"
           "GHC|GHP|GHS|GIP|GMD|GNE|GNF|GNS|GQE|GRD|GTQ|GWE|GWP|GYD|HKD|HNL|HRD|HRK|HTG|HUF|IDR|IEP|ILP|"
           "ILR|ILS|INR|IQD|IRR|ISJ|ISK|ITL|JMD|JOD|JPY|KES|KGS|KHR|KMF|KPW|KRW|KWD|KYD|KZT|LAJ|LAK|LBP|"
           "LKR|LRD|LSL|LSM|LTL|LTT|LUC|LUF|LUL|LVL|LVR|LYD|MAD|MDL|MGA|MGF|MKD|MLF|MMK|MNT|MOP|MRO|MRU|"
           "MTL|MTP|MUR|MVQ|MVR|MWK|MXN|MXP|MYR|MZE|MZM|MZN|NAD|NGN|NIC|NIO|NLG|NOK|NPR|NZD|OMR|PAB|PEH|"
           "PEI|PEN|PES|PGK|PHP|PKR|PLN|PLZ|PTE|PYG|QAR|RHD|ROK|ROL|RON|RSD|RUB|RUR|RWF|SAR|SBD|SCR|SDD|"
           "SDG|SDP|SEK|SGD|SHP|SIT|SKK|SLL|SOS|SRD|SRG|SSP|STD|STN|SUR|SVC|SYP|SZL|THB|TJR|TJS|TMM|TMT|"
           "TND|TOP|TPE|TRL|TRY|TTD|TWD|TZS|UAH|UAK|UGS|UGW|UGX|USD|USS|UYN|UYP|UYU|UZS|VEB|VEF|VNC|VND|"
           "VUV|WST|XAF|XCD|XDR|XEU|XFO|XOF|XPF|YDD|YER|YUD|YUM|YUN|ZAL|ZAR|ZMK|ZMW|ZRN|ZRZ|ZWC|ZWD|ZWL|ZWN|ZWR";
}

const wxArrayString g_locales()
{
    wxArrayString list;
    list.Add("");
    list.Add("af_ZA");
    list.Add("am_ET");
    list.Add("ar_AE");
    list.Add("ar_BH");
    list.Add("ar_DZ");
    list.Add("ar_EG");
    list.Add("ar_IQ");
    list.Add("ar_JO");
    list.Add("ar_KW");
    list.Add("ar_LB");
    list.Add("ar_LY");
    list.Add("ar_MA");
    list.Add("ar_OM");
    list.Add("ar_QA");
    list.Add("ar_SA");
    list.Add("ar_SY");
    list.Add("ar_TN");
    list.Add("ar_YE");
    list.Add("as_IN");
    list.Add("ba_RU");
    list.Add("be_BY");
    list.Add("bg_BG");
    list.Add("bn_BD");
    list.Add("bn_IN");
    list.Add("bo_CN");
    list.Add("br_FR");
    list.Add("ca_ES");
    list.Add("co_FR");
    list.Add("cs_CZ");
    list.Add("cy_GB");
    list.Add("da_DK");
    list.Add("de_AT");
    list.Add("de_CH");
    list.Add("de_DE");
    list.Add("de_LI");
    list.Add("de_LU");
    list.Add("dv_MV");
    list.Add("el_GR");
    list.Add("en_AU");
    list.Add("en_BZ");
    list.Add("en_CA");
    list.Add("en_GB");
    list.Add("en_IE");
    list.Add("en_IN");
    list.Add("en_JM");
    list.Add("en_MY");
    list.Add("en_NZ");
    list.Add("en_PH");
    list.Add("en_SG");
    list.Add("en_TT");
    list.Add("en_US");
    list.Add("en_ZA");
    list.Add("en_ZW");
    list.Add("es_AR");
    list.Add("es_BO");
    list.Add("es_CL");
    list.Add("es_CO");
    list.Add("es_CR");
    list.Add("es_DO");
    list.Add("es_EC");
    list.Add("es_ES");
    list.Add("es_GT");
    list.Add("es_HN");
    list.Add("es_MX");
    list.Add("es_NI");
    list.Add("es_PA");
    list.Add("es_PE");
    list.Add("es_PR");
    list.Add("es_PY");
    list.Add("es_SV");
    list.Add("es_US");
    list.Add("es_UY");
    list.Add("es_VE");
    list.Add("et_EE");
    list.Add("eu_ES");
    list.Add("fa_IR");
    list.Add("fi_FI");
    list.Add("fo_FO");
    list.Add("fr_BE");
    list.Add("fr_CA");
    list.Add("fr_CH");
    list.Add("fr_FR");
    list.Add("fr_LU");
    list.Add("fr_MC");
    list.Add("fy_NL");
    list.Add("ga_IE");
    list.Add("gd_GB");
    list.Add("gl_ES");
    list.Add("gu_IN");
    list.Add("he_IL");
    list.Add("hi_IN");
    list.Add("hr_BA");
    list.Add("hr_HR");
    list.Add("hu_HU");
    list.Add("hy_AM");
    list.Add("id_ID");
    list.Add("ig_NG");
    list.Add("ii_CN");
    list.Add("is_IS");
    list.Add("it_CH");
    list.Add("it_IT");
    list.Add("ja_JP");
    list.Add("ka_GE");
    list.Add("kk_KZ");
    list.Add("kl_GL");
    list.Add("km_KH");
    list.Add("kn_IN");
    list.Add("ko_KR");
    list.Add("ky_KG");
    list.Add("lb_LU");
    list.Add("lo_LA");
    list.Add("lt_LT");
    list.Add("lv_LV");
    list.Add("mi_NZ");
    list.Add("mk_MK");
    list.Add("ml_IN");
    list.Add("mn_MN");
    list.Add("mr_IN");
    list.Add("ms_BN");
    list.Add("ms_MY");
    list.Add("mt_MT");
    list.Add("nb_NO");
    list.Add("ne_NP");
    list.Add("nl_BE");
    list.Add("nl_NL");
    list.Add("nn_NO");
    list.Add("oc_FR");
    list.Add("or_IN");
    list.Add("pa_IN");
    list.Add("pl_PL");
    list.Add("ps_AF");
    list.Add("pt_BR");
    list.Add("pt_PT");
    list.Add("rm_CH");
    list.Add("ro_RO");
    list.Add("ru_RU");
    list.Add("rw_RW");
    list.Add("sa_IN");
    list.Add("se_FI");
    list.Add("se_NO");
    list.Add("se_SE");
    list.Add("si_LK");
    list.Add("sk_SK");
    list.Add("sl_SI");
    list.Add("sq_AL");
    list.Add("sv_FI");
    list.Add("sv_SE");
    list.Add("sw_KE");
    list.Add("ta_IN");
    list.Add("te_IN");
    list.Add("th_TH");
    list.Add("tk_TM");
    list.Add("tn_ZA");
    list.Add("tr_TR");
    list.Add("tt_RU");
    list.Add("ug_CN");
    list.Add("uk_UA");
    list.Add("ur_PK");
    list.Add("vi_VN");
    list.Add("wo_SN");
    list.Add("xh_ZA");
    list.Add("yo_NG");
    list.Add("zh_CN");
    list.Add("zh_HK");
    list.Add("zh_MO");
    list.Add("zh_SG");
    list.Add("zh_TW");
    list.Add("zu_ZA");
    return list;

}
