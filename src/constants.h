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

static const wxSizerFlags g_flags = wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL).Border(wxALL, 5);
static const wxSizerFlags g_flagsExpand = wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxEXPAND).Border(wxALL, 5).Proportion(1);

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

#endif // _MM_EX_CONSTANTS_H_
//----------------------------------------------------------------------------
