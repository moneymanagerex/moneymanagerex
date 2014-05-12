/*************************************************************************
 Copyright (C) 2012 Stefano Giorgio

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
 *************************************************************************/

/*************************************************************************
 Renamed after extensive modifications to original file reportbudgetsetup.cpp
**************************************************************************/
#ifndef MM_EX_REPORTBUDGET_CATEGORYSUMMARY_H_
#define MM_EX_REPORTBUDGET_CATEGORYSUMMARY_H_

#include "budget.h"

class mmReportBudgetCategorySummary : public mmReportBudget
{
public:
    mmReportBudgetCategorySummary(int budgetYearID);

    virtual wxString getHTMLText();
    virtual wxString version();

private:
    int budgetYearID_;

    wxString actualAmountColour(double amount, double actual, double estimated, bool total = false);
};

#endif // MM_EX_REPORTBUDGET_CATEGORYSUMMARY_H_
