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
#include "budget.h"

#include "../mmex.h"
#include "../budgetingpanel.h"
#include "../htmlbuilder.h"

mmReportBudget::mmReportBudget(mmGUIFrame* mainFrame, mmCoreDB* core)
: mmPrintableBase(core)
, mainFrame_(mainFrame)
{}

/**************************************************************************************
 This will set the budget estimate for a month in a year.
 ***************************************************************************************/
void mmReportBudget::setBudgetMonthlyEstimate(mmBudgetEntryHolder& budEntry)
{
    int ndays = 365;

    if (budEntry.period_ == "Monthly") {
        budEntry.estimated_ = budEntry.amt_;

    } else if (budEntry.period_ == "Yearly") {
        budEntry.estimated_ = budEntry.amt_ / 12;

    } else if (budEntry.period_ == "Weekly") {
        budEntry.estimated_ = ((budEntry.amt_ / 7) * ndays) / 12;

    } else if (budEntry.period_ == "Bi-Weekly") {
        budEntry.estimated_ = ((budEntry.amt_ / 14) * ndays) / 12;

    } else if (budEntry.period_ == "Bi-Monthly") {
        budEntry.estimated_ = budEntry.amt_ / 2;

    } else if (budEntry.period_ == "Quarterly") {
        budEntry.estimated_ = budEntry.amt_ / 3;

    } else if (budEntry.period_ == "Half-Yearly") {
        budEntry.estimated_ = (budEntry.amt_ / 6);

    } else if (budEntry.period_ == "Daily") {
        budEntry.estimated_ = (budEntry.amt_ * ndays) / 12;

    } else {
        wxASSERT(true);
    }
}

void mmReportBudget::setBudgetYearlyEstimate(mmBudgetEntryHolder& budEntry)
{
    if (budEntry.period_ == "Monthly") {
        budEntry.estimated_ = budEntry.amt_ * 12;

    } else if (budEntry.period_ == "Yearly") {
        budEntry.estimated_ = budEntry.amt_;

    } else if (budEntry.period_ == "Weekly") {
        budEntry.estimated_ = budEntry.amt_ * 52;

    } else if (budEntry.period_ == "Bi-Weekly") {
        budEntry.estimated_ = budEntry.amt_ * 26;

    } else if (budEntry.period_ == "Bi-Monthly") {
        budEntry.estimated_ = budEntry.amt_ * 6;

    } else if (budEntry.period_ == "Quarterly") {
        budEntry.estimated_ = budEntry.amt_ * 4;

    } else if (budEntry.period_ == "Half-Yearly") {
        budEntry.estimated_ = budEntry.amt_ * 2;

    } else if (budEntry.period_ == "Daily") {
        budEntry.estimated_ = budEntry.amt_ * 365;

    } else {
        wxASSERT(true);
    }
}

void mmReportBudget::setBudgetEstimate(mmBudgetEntryHolder& budEntry, bool monthBudget)
{
    if (monthBudget)
        setBudgetMonthlyEstimate(budEntry);
    else
        setBudgetYearlyEstimate(budEntry);
}

void mmReportBudget::initBudgetEntryFields(mmBudgetEntryHolder& budEntry, int id)
{
    budEntry.id_ = id;
    budEntry.subcategID_ = -1;
    budEntry.subCatStr_  = "";
    budEntry.amt_ = 0.0;
    budEntry.period_ = "None";
    budEntry.estimated_ = 0.0;
    budEntry.estimatedStr_ = "0.0";
    budEntry.actual_ = 0.0;
    budEntry.actualStr_ = "0.0";
}

void mmReportBudget::SetDateToEndOfMonth(int month, wxDateTime& date)
{
    date.SetDay(28);
    date.SetMonth((wxDateTime::Month)month);
    date.SetToLastMonthDay();
}

void mmReportBudget::SetDateToEndOfYear(int day, int month, wxDateTime& date, bool isEndDate)
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
    day = wxAtoi(mmOptions::instance().financialYearStartDayString_);
    month = wxAtoi(mmOptions::instance().financialYearStartMonthString_) - 1;
    if ( (day > 28) && (month == wxDateTime::Feb) )
        day = 28;
    else if ( ((day > 30) && (month == wxDateTime::Sep)) ||
              ((day > 30) && (month == wxDateTime::Apr)) ||
              ((day > 30) && (month == wxDateTime::Jun)) ||
              ((day > 30) && (month == wxDateTime::Nov)) )
    {
        day = 30;
    }
}

wxString mmReportBudget::AdjustYearValues(int& day, int& month, long year, wxString yearStr)
{
    if ((yearStr.length() < 5)) {
        if ( mainFrame_->budgetFinancialYears() ) {
            GetFinancialYearValues(day, month);
            yearStr = wxString() << _("Financial Year: ") << yearStr << " - " << (year + 1);
        } else
            yearStr = wxString() << _("Year: ") << yearStr;
    } else {
        yearStr = wxString() << _("Month: ") << yearStr;
    }

    return yearStr;
}

void mmReportBudget::AdjustYearValues(int& day, int& month, wxDateTime& year)
{
    if ( mainFrame_->budgetFinancialYears() ) {
        GetFinancialYearValues(day, month);
        SetDateToEndOfYear(day, month, year, false);
    }
}

void mmReportBudget::AdjustDateForEndFinancialYear(wxDateTime& date)
{
    if ( mainFrame_->budgetFinancialYears() )
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
