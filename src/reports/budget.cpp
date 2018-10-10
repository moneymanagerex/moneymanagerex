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
#include "budget.h"

#include "mmex.h"
#include "mmframe.h"
#include "reports/htmlbuilder.h"

mmReportBudget::mmReportBudget(): mmPrintableBase("mmReportBudget")
{}

mmReportBudget::~mmReportBudget()
{}

void mmReportBudget::SetDateToEndOfMonth(int month, wxDateTime& date)
{
    date.SetDay(28);
    date.SetMonth((wxDateTime::Month)month);
    date.SetToLastMonthDay();
}

void mmReportBudget::SetDateToEndOfYear(const int day, const int month, wxDateTime& date, bool isEndDate)
{
    date.SetDay(day);
    date.SetMonth((wxDateTime::Month)month);
    if (isEndDate)
    {
        date.Subtract(wxDateSpan::Day());
        date.Add(wxDateSpan::Year());
    }
}

void mmReportBudget::SetBudgetMonth(wxString budgetYearStr, wxDateTime& startDate, wxDateTime& endDate)
{
    wxStringTokenizer tz(budgetYearStr,"-");
    wxString yearStr = tz.GetNextToken();
    wxString monthStr = tz.GetNextToken();
    int month = wxAtoi(monthStr) - 1;
    startDate.SetMonth((wxDateTime::Month)month);
    SetDateToEndOfMonth(month,endDate);
}

void mmReportBudget::GetFinancialYearValues(int& day, int& month)
{
    day = wxAtoi(Option::instance().FinancialYearStartDay());
    month = wxAtoi(Option::instance().FinancialYearStartMonth()) - 1;
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

wxString mmReportBudget::AdjustYearValues(int day, int month, long year, const wxString& yearStr)
{
    wxString ret = yearStr;
    if ((ret.length() < 5))
    {
        if (Option::instance().BudgetFinancialYears())
        {
            GetFinancialYearValues(day, month);
            ret = wxString::Format(_("Financial Year: %s - %li"), yearStr, (year + 1));
        }
        else
        {
            ret = wxString::Format(_("Year: %s"), yearStr);
        }
    }
    else
    {
        ret = wxString::Format(_("Month: %s"), yearStr);
    }

    return ret;
}

void mmReportBudget::AdjustYearValues(int day, int month, wxDateTime& date)
{
    if (Option::instance().BudgetFinancialYears())
    {
        GetFinancialYearValues(day, month);
        SetDateToEndOfYear(day, month, date, false);
    }
}

void mmReportBudget::AdjustDateForEndFinancialYear(wxDateTime& date)
{
    if (Option::instance().BudgetFinancialYears())
    {
        int day, month;
        GetFinancialYearValues(day, month);
        SetDateToEndOfYear(day, month, date);
    }
}

wxString mmReportBudget::getHTMLText()
{
    return wxEmptyString;
}

