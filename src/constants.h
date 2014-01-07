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
#ifndef _MM_EX_CONSTANTS_H_
#define _MM_EX_CONSTANTS_H_

#include "defs.h"
//----------------------------------------------------------------------------

class wxString;

namespace mmex
{
extern const int MIN_DATAVERSION;
extern const wxString DATAVERSION;

extern const wxString DEFDATEFORMAT;
extern const wxString DEFDELIMTER;
extern const wxString DEFSTOCKURL;

/*
        Uses for titles, reports, etc.
        Do not use mmex::GetAppName() for such things.
*/
wxString getProgramName();
wxString getTitleProgramVersion();
wxString getProgramVersion();
wxString getProgramCopyright();
wxString getProgramWebSite();
wxString getProgramForum();
wxString getProgramDanateSite();
wxString getProgramTwitterSite();
wxString getProgramFacebookSite();
wxString getProgramDescription();
} // namespace mmex

const wxString VIEW_TRANS_ALL_STR            = wxTRANSLATE("View All Transactions");
const wxString VIEW_TRANS_RECONCILED_STR     = wxTRANSLATE("View Reconciled");
const wxString VIEW_TRANS_NOT_RECONCILED_STR = wxTRANSLATE("View Not-Reconciled");
const wxString VIEW_TRANS_UNRECONCILED_STR   = wxTRANSLATE("View UnReconciled");
const wxString VIEW_TRANS_VOID               = wxTRANSLATE("View Void");
const wxString VIEW_TRANS_FLAGGED            = wxTRANSLATE("View Flagged");
const wxString VIEW_TRANS_DUPLICATES         = wxTRANSLATE("View Duplicates");
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

const wxString LANGUAGE_PARAMETER = "LANGUAGE";
const wxString INIDB_USE_TRANSACTION_SOUND = "USETRANSSOUND";
const wxString INIDB_USE_ORG_DATE_COPYPASTE = "USEORIGDATEONCOPYPASTE";

const wxString INIDB_BUDGET_FINANCIAL_YEARS       = "BUDGET_FINANCIAL_YEARS";
const wxString INIDB_BUDGET_INCLUDE_TRANSFERS     = "BUDGET_INCLUDE_TRANSFERS";
const wxString INIDB_BUDGET_SETUP_WITHOUT_SUMMARY = "BUDGET_SETUP_WITHOUT_SUMMARY";
const wxString INIDB_BUDGET_SUMMARY_WITHOUT_CATEG = "BUDGET_SUMMARY_WITHOUT_CATEGORIES";
const wxString INIDB_IGNORE_FUTURE_TRANSACTIONS   = "IGNORE_FUTURE_TRANSACTIONS";

static const wxString MONTHS[12] =
{
    wxTRANSLATE("January"), wxTRANSLATE("February"), wxTRANSLATE("March")
    , wxTRANSLATE("April"), wxTRANSLATE("May "), wxTRANSLATE("June")
    , wxTRANSLATE("July"), wxTRANSLATE("August"), wxTRANSLATE("September")
    , wxTRANSLATE("October"), wxTRANSLATE("November"), wxTRANSLATE("December")
};

static const wxString MONTHS_SHORT[12] =
{
    wxTRANSLATE("Jan"), wxTRANSLATE("Feb"), wxTRANSLATE("Mar")
    , wxTRANSLATE("Apr"), wxTRANSLATE("May"), wxTRANSLATE("Jun")
    , wxTRANSLATE("Jul"), wxTRANSLATE("Aug"), wxTRANSLATE("Sep")
    , wxTRANSLATE("Oct"), wxTRANSLATE("Nov"), wxTRANSLATE("Dec")
};

static const wxString gDaysInWeek[7] =
{
    wxTRANSLATE("Sunday"), wxTRANSLATE("Monday"), wxTRANSLATE("Tuesday")
    , wxTRANSLATE("Wednesday"), wxTRANSLATE("Thursday"), wxTRANSLATE("Friday")
    , wxTRANSLATE("Saturday")
};


#endif // _MM_EX_CONSTANTS_H_
//----------------------------------------------------------------------------
