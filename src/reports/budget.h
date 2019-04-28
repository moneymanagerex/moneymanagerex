/*************************************************************************
 Copyright (C) 2012 Stefano Giorgio
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
 *************************************************************************/
#ifndef MM_EX_REPORTBUDGETING_H_
#define MM_EX_REPORTBUDGETING_H_

#include "reportbase.h"

class mmHTMLBuilder;

/*************************************************************************
 Class: mmReportBudget : This class is a base class for Budget Reports.
        This class should not be used to create objects.
 *************************************************************************/
class mmReportBudget : public mmPrintableBase
{
public:
    mmReportBudget();
    virtual ~mmReportBudget();

    /// Returns correct values for day and month, adjusted to financial year if required.
    void AdjustYearValues(int day, wxDateTime::Month month, wxDateTime& year) const;

    /// Returns correct values for day and month, adjusted to financial year if required.
    /// Also returns a heading string for Month or Year reports.
    const wxString AdjustYearValues(int day, wxDateTime::Month month, int year, const wxString& yearStr) const;

    /// Sets date to end of financial year if required by user.
    void AdjustDateForEndFinancialYear(wxDateTime& date) const;

    /// Return day and month values to user defined financial year.
    void GetFinancialYearValues(int& day, wxDateTime::Month& month) const;

    /// Advance the given date to the end of the current month.
    void SetDateToEndOfMonth(const wxDateTime::Month month, wxDateTime& date) const;

    /// Advance the given date by one year.
    void SetDateToEndOfYear(int day, const wxDateTime::Month month, wxDateTime& date, bool isEndDate = true) const;

    /// sets the start and end dates for a budget month
    void SetBudgetMonth(wxString budgetYearStr, wxDateTime& startDate, wxDateTime& endDate) const;

    virtual wxString getHTMLText();
};

#endif // MM_EX_REPORTBUDGETING_H_
