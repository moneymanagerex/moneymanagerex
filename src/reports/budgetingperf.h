/*******************************************************
 Copyright (C) 2006-2012
 Copyright (C) 2017 James Higley

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

#ifndef MM_EX_REPORTBUDGETING_PERFORMANCE_H_
#define MM_EX_REPORTBUDGETING_PERFORMANCE_H_

#include "budget.h"
#include <map>

class mmReportBudgetingPerformance : public mmReportBudget
{
public:
    mmReportBudgetingPerformance();
    virtual ~mmReportBudgetingPerformance();

    virtual int report_parameters();
    virtual wxString getHTMLText();

private:
    void DisplayRow(mmHTMLBuilder &hb, double estimated, double actual
        , const wxString& catName, const std::map<int, double>& stats
        , bool bTotalRow = false);
};

#endif // MM_EX_REPORTBUDGETING_PERFORMANCE_H_
