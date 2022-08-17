/*******************************************************
Copyright (C) 2009 VaDiM
Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)

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

#include "defs.h"
#include "versions.h"
//----------------------------------------------------------------------------

class wxString;

extern const wxSizerFlags g_flagsH;
extern const wxSizerFlags g_flagsV;
extern const wxSizerFlags g_flagsBorder1H;
extern const wxSizerFlags g_flagsBorder1V;
extern const wxSizerFlags g_flagsCenter;
extern const wxSizerFlags g_flagsExpand;
extern const wxSizerFlags g_flagsExpandBorder1;

extern const wxString g_CancelLabel;
extern const wxString g_CloseLabel;

namespace mmex
{
extern int MIN_DATAVERSION;
extern const wxString DATAVERSION;

extern const wxString DEFDATEFORMAT;
extern const wxString DEFDELIMTER;

/*
        Uses for titles, reports, etc.
        Do not use mmex::GetAppName() for such things.
*/
const wxString getProgramName();
const wxString getTitleProgramVersion();
const wxString getCaption(const wxString& caption);

namespace version
{
    extern const wxString string;
    extern bool isStable();
    extern const wxString getProgramVersion();
    int getDbLatestVersion();
}

namespace weblink
{
    const wxString addReferralToURL(const wxString& BaseURL, const wxString& CampSource);
    extern const wxString GA;
    extern const wxString WebSite;
    extern const wxString Releases;
    extern const wxString Download;
    extern const wxString News;
    extern const wxString NewsRSS;
    extern const wxString Forum;
    extern const wxString Wiki;
    extern const wxString BugReport;
    extern const wxString Donate;
    extern const wxString Twitter;
    extern const wxString Facebook;
    extern const wxString GitHub;
    extern const wxString GitHubLogin;
    extern const wxString YouTube;
    extern const wxString Slack;
    extern const wxString SquareCashGuan;
    extern const wxString DefStockUrl;
    extern const wxString GooglePlay;
    extern const wxString WebApp;
    extern const wxString Chiark;
    extern const wxString Crowdin;
    extern const wxString YahooQuotes;
    extern const wxString YahooQuotesHistory;
    extern const wxString CoinCapSearch;
    extern const wxString CoinCapHistory;
} // namespace weblink
} // namespace mmex

extern const wxString VIEW_TRANS_FILTER_DIALOG_STR;
extern const wxString VIEW_TRANS_ALL_STR;
extern const wxString VIEW_TRANS_TODAY_STR;
extern const wxString VIEW_TRANS_CURRENT_MONTH_STR;
extern const wxString VIEW_TRANS_LAST_30_DAYS_STR;
extern const wxString VIEW_TRANS_LAST_90_DAYS_STR;
extern const wxString VIEW_TRANS_LAST_MONTH_STR;
extern const wxString VIEW_TRANS_LAST_3MONTHS_STR;
extern const wxString VIEW_TRANS_LAST_12MONTHS_STR;
extern const wxString VIEW_TRANS_CURRENT_YEAR_STR;
extern const wxString VIEW_TRANS_CRRNT_FIN_YEAR_STR;
extern const wxString VIEW_TRANS_LAST_YEAR_STR;
extern const wxString VIEW_TRANS_LAST_FIN_YEAR_STR;
extern const wxString VIEW_TRANS_SINCE_STATEMENT_STR;

extern const wxString VIEW_ACCOUNTS_ALL_STR;
extern const wxString VIEW_ACCOUNTS_OPEN_STR;
extern const wxString VIEW_ACCOUNTS_CLOSED_STR;
extern const wxString VIEW_ACCOUNTS_FAVORITES_STR;

extern const wxString LANGUAGE_PARAMETER;
extern const wxString INIDB_USE_CURRENCY_HISTORY;
extern const wxString INIDB_USE_TRANSACTION_SOUND;
extern const wxString INIDB_USE_ORG_DATE_COPYPASTE;
extern const wxString INIDB_USE_ORG_DATE_DUPLICATE;
extern const wxString INIDB_SEND_USAGE_STATS;
extern const wxString INIDB_CHECK_NEWS;

extern const wxString INIDB_HIDE_SHARE_ACCOUNTS;
extern const wxString INIDB_BUDGET_FINANCIAL_YEARS;
extern const wxString INIDB_BUDGET_INCLUDE_TRANSFERS;
extern const wxString INIDB_BUDGET_SUMMARY_WITHOUT_CATEG;
extern const wxString INIDB_IGNORE_FUTURE_TRANSACTIONS;
extern const wxString INIDB_SHOW_TOOLTIPS;
extern const wxString INIDB_SHOW_MONEYTIPS;

extern const wxString ATTACHMENTS_FOLDER_DOCUMENTS;
extern const wxString ATTACHMENTS_FOLDER_DATABASE;
extern const wxString ATTACHMENTS_FOLDER_USERPROFILE;
extern const wxString ATTACHMENTS_FOLDER_APPDATA;

extern const wxString INIDB_NEWS_LAST_READ_DATE;

extern const wxArrayString g_locales();
extern const wxString g_fiat_curr();

enum id
{
    mmID_HOMEPAGE = wxID_HIGHEST + 1,
    mmID_BILLS,
    mmID_STOCKS,
    mmID_ASSETS,
    mmID_BUDGET,
    mmID_REPORTS,
    mmID_CHECKING,
    mmID_CATEGORY,
    mmID_PAYEE,
    mmID_SPLIT,
    mmID_ALLTRANSACTIONS,
    mmID_BROWSER,
    mmID_REMOVE,
    mmID_COLOR,
    MENU_REPORT_BUG,
    MENU_BILLSDEPOSITS,
    MENU_STOCKS,
    MENU_GOTOACCOUNT,
    MENU_ASSETS,
    MENU_VIEW_BUDGET_FINANCIAL_YEARS,
    MENU_VIEW_BUDGET_CATEGORY_SUMMARY,
    MENU_VIEW_BUDGET_TRANSFER_TOTAL,
    MENU_VIEW_IGNORE_FUTURE_TRANSACTIONS,
    MENU_VIEW_SHOW_TOOLTIPS,
    MENU_VIEW_SHOW_MONEYTIPS,
    MENU_VIEW_TOGGLE_FULLSCREEN,
    mmID_MAX,

};

