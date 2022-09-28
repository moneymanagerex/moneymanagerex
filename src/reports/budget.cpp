/*************************************************************************
 Copyright (C) 2012 Stefano Giorgio
 Copyright (C) 2017 James Higley
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

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
#include "budget.h"

#include "mmex.h"
#include "mmframe.h"
#include "reports/htmlbuilder.h"

mmReportBudget::mmReportBudget(): mmPrintableBase("mmReportBudget")
{
    setReportParameters(Reports::UNUSED);
}

mmReportBudget::~mmReportBudget()
{}

void mmReportBudget::SetDateToEndOfMonth(const wxDateTime::Month month, wxDateTime& date) const
{
    date.SetDay(28);
    date.SetMonth(month);
    date.SetToLastMonthDay();
}

void mmReportBudget::SetDateToEndOfYear(const int day, const wxDateTime::Month month, wxDateTime& date, bool isEndDate) const
{
    date.SetDay(day);
    date.SetMonth(month);
    if (isEndDate)
    {
        date.Subtract(wxDateSpan::Day());
        date.Add(wxDateSpan::Year());
    }
}

void mmReportBudget::SetBudgetMonth(wxString budgetYear, wxDateTime& startDate, wxDateTime& endDate) const
{
    wxRegEx pattern_month(R"(^([0-9]{4})-([0-9]{2})$)");
    if (pattern_month.Matches(budgetYear))
    {
        wxString monthStr = pattern_month.GetMatch(budgetYear, 2);
        int month = wxAtoi(monthStr) - 1;
        startDate.Add(wxDateSpan::Months(month));
        endDate = startDate;
        endDate.Add(wxDateSpan::Months(1));
    }
}

void mmReportBudget::GetFinancialYearValues(int& day, wxDateTime::Month& month) const
{
    day = wxAtoi(Option::instance().FinancialYearStartDay());
    month = static_cast<wxDateTime::Month>(wxAtoi(Option::instance().FinancialYearStartMonth()) - 1);
    if ((day > 28) && (month == wxDateTime::Feb))
    {
        day = 28;
    }
    else if ( ((day > 30) && (month == wxDateTime::Sep)) ||
              ((day > 30) && (month == wxDateTime::Apr)) ||
              ((day > 30) && (month == wxDateTime::Jun)) ||
              ((day > 30) && (month == wxDateTime::Nov)) )
    {
        day = 30;
    }
}

const wxString mmReportBudget::AdjustYearValues(int day, wxDateTime::Month month, int year, const wxString& yearStr) const
{
    wxString ret = yearStr;
    if ((ret.length() < 5))
    {
        if (Option::instance().BudgetFinancialYears())
        {
            GetFinancialYearValues(day, month);
            ret = wxString::Format(_("Financial Year: %s - %i"), yearStr, (year + 1));
        }
        else
        {
            ret = wxString::Format(_("Year: %s"), yearStr);
        }
    }
    else
    {
        const wxString month_str = wxGetTranslation(mmGetMonthName(month));
        ret = wxString::Format(_("Year: %i Month: %s"), year, month_str);
    }

    return ret;
}

void mmReportBudget::AdjustYearValues(int day, wxDateTime::Month month, wxDateTime& date) const
{
    if (Option::instance().BudgetFinancialYears())
    {
        GetFinancialYearValues(day, month);
        SetDateToEndOfYear(day, month, date, false);
    }
}

void mmReportBudget::AdjustDateForEndFinancialYear(wxDateTime& date) const
{
    if (Option::instance().BudgetFinancialYears())
    {
        int day;
        wxDateTime::Month month;
        GetFinancialYearValues(day, month);
        SetDateToEndOfYear(day, month, date);
    }
}

wxString mmReportBudget::getHTMLText()
{
    return wxEmptyString;
}

