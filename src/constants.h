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

const wxString NAVTREECTRL_REPORTS = "Reports";
const wxString NAVTREECTRL_HELP = "Help";
const wxString NAVTREECTRL_CUSTOM_REPORTS = "Custom_Reports";
const wxString NAVTREECTRL_INVESTMENT = "Stocks";
const wxString NAVTREECTRL_BUDGET = "Budgeting";

const wxString  VIEW_ACCOUNTS_ALL_STR       = "ALL";
const wxString  VIEW_ACCOUNTS_OPEN_STR      = "Open";
const wxString  VIEW_ACCOUNTS_FAVORITES_STR = "Favorites";

const wxString INIDB_USE_TRANSACTION_SOUND  = "USETRANSSOUND";
const wxString INIDB_USE_ORG_DATE_COPYPASTE = "USEORIGDATEONCOPYPASTE";

const wxString INIDB_BUDGET_FINANCIAL_YEARS       = "BUDGET_FINANCIAL_YEARS";
const wxString INIDB_BUDGET_INCLUDE_TRANSFERS     = "BUDGET_INCLUDE_TRANSFERS";
const wxString INIDB_BUDGET_SETUP_WITHOUT_SUMMARY = "BUDGET_SETUP_WITHOUT_SUMMARY";
const wxString INIDB_BUDGET_SUMMARY_WITHOUT_CATEG = "BUDGET_SUMMARY_WITHOUT_CATEGORIES";
const wxString INIDB_IGNORE_FUTURE_TRANSACTIONS   = "IGNORE_FUTURE_TRANSACTIONS";

const wxString TIPS_BANKS  = "checkingpanel";
const wxString TIPS_BILLS  = "billsdeposits";
const wxString TIPS_STOCK  = "investment";

const wxString BILLSDEPOSITS_REPEATS[] =
{
    wxTRANSLATE("None"),
    wxTRANSLATE("Weekly"),
    wxTRANSLATE("Bi-Weekly"),
    wxTRANSLATE("Monthly"),
    wxTRANSLATE("Bi-Monthly"),
    wxTRANSLATE("Quarterly"),
    wxTRANSLATE("Half-Yearly"),
    wxTRANSLATE("Yearly"),
    wxTRANSLATE("Four Months"),
    wxTRANSLATE("Four Weeks"),
    wxTRANSLATE("Daily"),
    wxTRANSLATE("In %s Days"),
    wxTRANSLATE("In %s Months"),
    wxTRANSLATE("Every %s Days"),
    wxTRANSLATE("Every %s Months"),
    wxTRANSLATE("Monthly (last day)"),
    wxTRANSLATE("Monthly (last business day)")

};

static const wxString DATE_PRESETTINGS[] =
{
    VIEW_TRANS_ALL_STR,
    VIEW_TRANS_TODAY_STR,
    VIEW_TRANS_CURRENT_MONTH_STR,
    VIEW_TRANS_LAST_30_DAYS_STR,
    VIEW_TRANS_LAST_90_DAYS_STR,
    VIEW_TRANS_LAST_MONTH_STR ,
    VIEW_TRANS_LAST_3MONTHS_STR,
    VIEW_TRANS_LAST_12MONTHS_STR,
    VIEW_TRANS_CURRENT_YEAR_STR
};

static const wxString TRANSACTION_STATUSES[] =
{
    wxTRANSLATE("None"),
    wxTRANSLATE("Reconciled"),
    wxTRANSLATE("Void"),
    wxTRANSLATE("Follow up"),
    wxTRANSLATE("Duplicate"),
    wxTRANSLATE("Un-Reconciled"),
    wxTRANSLATE("All Except Reconciled")
};

static const wxString TIPS[] =
{
    wxTRANSLATE("Recommendation: Always backup your .mmb database file regularly."),
    wxTRANSLATE("Recommendation: If upgrading to a new version of MMEX, make sure you backup your .mmb database file before doing so."),
    wxTRANSLATE("Recommendation: Use copy (Ctrl+С) and paste (Ctrl+V) for frequently used transactions."),
    wxTRANSLATE("Tip: Remember to make backups of your .mmb."),
    wxTRANSLATE("Tip: The .mmb file is not encrypted. That means anyone else having the proper know how can actually open the file and read the contents. So make sure that if you are storing any sensitive financial information it is properly guarded."),
    wxTRANSLATE("Tip: To mark a transaction as reconciled, just select the transaction and hit the 'r' or 'R' key. To mark a transaction as unreconciled, just select the transaction and hit the 'u' or 'U' key."),
    wxTRANSLATE("Tip: To mark a transaction as requiring followup, just select the transaction and hit the 'f' or 'F' key."),
    wxTRANSLATE("Tip: MMEX supports printing of all reports that can be viewed. The print options are available under the menu, File->Print."),
    wxTRANSLATE("Tip: You can modify some runtime behavior of MMEX by changing the options in the Options Dialog. "),
    wxTRANSLATE("Tip: To print a statement with transactions from any arbitary set of criteria, use the transaction filter to select the transactions you want and then do a print from the menu."),
    wxTRANSLATE("Tip: Set exchange rate for currencies in case if you have accounts with different currencies."),

    wxTRANSLATE("Organize Categories Dialog Tip: Pressing the h key will cycle through all categories starting with the letter h"),
    wxTRANSLATE("Organize Categories Dialog Tip: Pressing 2 key combination will cycle through all categories starting with that key combination. Example: Pressing ho will select Homeneeds, Home, House Tax, etc..."),

    wxTRANSLATE("Tip to get out of debt: Pay yourself 10% first. Put this into an account that is hard to touch. Make sure it is a chore to get the money out (you have to drive to the bank), so you will only tap it consciously and for major expenses."),
    wxTRANSLATE("Tip to get out of debt: Establish an emergency fund."),
    wxTRANSLATE("Tip to get out of debt: Stop acquiring new debt."),
    wxTRANSLATE("Tip to get out of debt: Create a realistic budget for your expenses."),
    wxTRANSLATE("Tip to get out of debt: Spend less than you earn."),
    wxTRANSLATE("Tip to get out of debt: Pay more than the minimum."),
    wxTRANSLATE("Before going to a shop and buy something: take the time making a list of what you really need. In the shop buy what is in your list.")
};

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

//*----------------------------------------------------------------------------*//

const wxString LANGUAGE_PARAMETER = "LANGUAGE";

// TODO remove these const variables
const wxString TRANS_TYPE_WITHDRAWAL_STR = "Withdrawal";
const wxString TRANS_TYPE_DEPOSIT_STR    = "Deposit";
const wxString TRANS_TYPE_TRANSFER_STR   = "Transfer";

//----------------------------------------------------------------------------
#endif // _MM_EX_CONSTANTS_H_
//----------------------------------------------------------------------------
