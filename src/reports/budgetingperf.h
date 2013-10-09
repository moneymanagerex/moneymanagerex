/*******************************************************
 Copyright (C) 2006-2012

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

#ifndef _MM_EX_REPORTBUDGETING_PERFORMANCE_H_
#define _MM_EX_REPORTBUDGETING_PERFORMANCE_H_

#include "mmcoredb.h"
#include "budget.h"

class mmReportBudgetingPerformance : public mmReportBudget
{
public:
    mmReportBudgetingPerformance(mmCoreDB* core, int budgetYearID);

    wxString getHTMLText();

private:
    int budgetYearID_;
    const mmCoreDB* core_;

    void DisplayEstimateMonths(mmHTMLBuilder& hb, mmBudgetEntryHolder& budgetEntry, int startMonth);
    void DisplayActualMonths(mmHTMLBuilder& hb, mmBudgetEntryHolder& budgetEntry, int startMonth, long startYear);
};

#endif // _MM_EX_REPORTBUDGETING_PERFORMANCE_H_
