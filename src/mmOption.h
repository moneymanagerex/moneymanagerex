/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

#ifndef _MM_EX_OPTION_H_
#define _MM_EX_OPTION_H_

#include "defs.h"

/*
   mmOptions caches the options for MMEX
   so that we don't hit the DB that often
   for data.
*/
class mmOptions
{
public:
    mmOptions();
    static mmOptions& instance();
    void LoadInfotableOptions();

    wxString dateFormat_;
    wxString language_;
    wxString userNameString_;

    wxString financialYearStartDayString_;
    wxString financialYearStartMonthString_;
    bool databaseUpdated_;
};

class mmIniOptions
{
public:
    mmIniOptions();
    static mmIniOptions& instance();
    void loadOptions();
    int account_image_id(int account_id);

    bool enableAssets_;
    bool enableBudget_;
    bool enableRepeatingTransactions_;
    bool enableCheckForUpdates_;
    bool enableReportIssues_;
    bool enableBeNotifiedForNewReleases_;
    bool enableVisitWebsite_;
    int html_font_size_;
//    wxString aboutCompanyName_;

    bool expandBankHome_;
    bool expandTermHome_;
    bool expandStocksHome_;
    bool expandBankTree_;
    bool expandTermTree_;

    bool budgetFinancialYears_;
    bool budgetIncludeTransfers_;
    bool budgetSetupWithoutSummaries_;
    bool budgetSummaryWithoutCategories_;
    bool ignoreFutureTransactions_;

    int transPayeeSelectionNone_;
    int transCategorySelectionNone_;
    int transStatusReconciled_;
    int transDateDefault_;
};

class CommitCallbackHook : public wxSQLite3Hook
{
public:
    virtual bool CommitCallback();
};

#endif // _MM_EX_OPTION_H_
//----------------------------------------------------------------------------
