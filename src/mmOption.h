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

#ifndef MM_EX_OPTION_H_
#define MM_EX_OPTION_H_

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

    const int account_image_id(int account_id, bool def = false);
    int html_font_size_;
    int ico_size_;

    bool budgetFinancialYears_;
    bool budgetIncludeTransfers_;
    bool budgetSetupWithoutSummaries_;
    bool budgetReportWithSummaries_;
    bool ignoreFutureTransactions_;

    int transPayeeSelectionNone_;
    int transPayeeMandatory_;
    int transCategorySelectionNone_;
    int transStatusReconciled_;
    int transDateDefault_;
};

#endif // MM_EX_OPTION_H_
//----------------------------------------------------------------------------
