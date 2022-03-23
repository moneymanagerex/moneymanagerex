/*******************************************************
Copyright (C) 2006-2012     Lisheng Guan (guanlisheng@gmail.com)
Copyright (C) 2014 - 2021   Nikolay Akimov
Copyright (C) 2021          Mark Whalley (mark@ipx.co.uk)

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

#include "mmDateRange.h"
#include "option.h"
#include <wx/intl.h>

#define DATE_MAX 253402214400   /* Dec 31, 9999 */

mmDateRange::mmDateRange()
    : today_(wxDateTime::Today())
    , future_(wxDateTime(DATE_MAX))
    , futureIgnored_(false)
{
    start_date_ = today_;
    end_date_ = today_;
    startDay_ = Option::instance().getReportingFirstDay();
    title_ = wxTRANSLATE("Date Range");
}

mmDateRange::~mmDateRange()
{
}

void mmDateRange::destroy()
{
    delete this;
}

const wxString mmDateRange::local_title() const
{
    return wxGetTranslation(title_);
}

void mmDateRange::findBeginOfMonth()
{
    if (this->today_.GetDay() < startDay_)
        this->start_date_.Subtract(wxDateSpan::Months(1));
    start_date_.SetDay(startDay_);
}

void mmDateRange::findEndOfMonth()
{
    if (this->today_.GetDay() < startDay_)
        this->end_date_.Subtract(wxDateSpan::Months(1));
    end_date_.Add(wxDateSpan::Months(1)).SetDay(1).Subtract(wxDateSpan::Day()).Add(wxDateSpan::Days(startDay_ - 1));
}

mmCurrentMonth::mmCurrentMonth()
: mmDateRange()
{
    this->findEndOfMonth();
    this->findBeginOfMonth();
    if (Option::instance().getIgnoreFutureTransactions() && (end_date_ > today_)) 
    {
        end_date_ = today_;
        futureIgnored_ = true;
    }
    this->title_ = wxTRANSLATE("Current Month");
}

mmToday::mmToday()
: mmDateRange()
{
    this->title_ = wxTRANSLATE("Today");
}

mmCurrentMonthToDate::mmCurrentMonthToDate()
: mmDateRange()
{
    findBeginOfMonth();
    this->end_date_ = today_;
    this->title_ = wxTRANSLATE("Current Month to Date");
}

mmLastMonth::mmLastMonth()
: mmDateRange()
{
    this->end_date_.Subtract(wxDateSpan::Months(1));
    this->findEndOfMonth();
    this->start_date_.Subtract(wxDateSpan::Months(1));
    this->findBeginOfMonth();
    this->title_ = wxTRANSLATE("Last Month");
}

mmLast30Days::mmLast30Days()
: mmDateRange()
{
    this->start_date_
        .Subtract(wxDateSpan::Months(1))
        .Add(wxDateSpan::Days(1));
    // no change to end_date_
    this->title_ = wxTRANSLATE("Last 30 Days");
}

mmLast90Days::mmLast90Days()
: mmDateRange()
{
    this->start_date_
        .Subtract(wxDateSpan::Months(3))
        .Add(wxDateSpan::Days(1));
    // no change to end_date_
    this->title_ = wxTRANSLATE("Last 90 Days");
}

mmLast3Months::mmLast3Months()
: mmDateRange()
{
    this->findEndOfMonth();
    this->start_date_.Subtract(wxDateSpan::Months(2));
    this->findBeginOfMonth();
    if (Option::instance().getIgnoreFutureTransactions() && (end_date_ > today_)) 
    {
        end_date_ = today_;
        futureIgnored_ = true;
    }
    this->title_ = wxTRANSLATE("Last 3 Months");
}

mmLast12Months::mmLast12Months()
: mmDateRange()
{
    this->findEndOfMonth();
    this->start_date_.Subtract(wxDateSpan::Months(11));
    this->findBeginOfMonth();
    if (Option::instance().getIgnoreFutureTransactions() && (end_date_ > today_)) 
    {
        end_date_ = today_;
        futureIgnored_ = true;
    }
    this->title_ = wxTRANSLATE("Last 12 Months");
}

mmCurrentYear::mmCurrentYear()
: mmDateRange()
{
    this->findBeginOfMonth();
    this->start_date_.SetMonth(wxDateTime::Jan);
    this->end_date_ = this->start_date_;
    this->end_date_.Add(wxDateSpan::Months(11));
    this->findEndOfMonth();
    if (Option::instance().getIgnoreFutureTransactions() && (end_date_ > today_)) 
    {
        end_date_ = today_;
        futureIgnored_ = true;
    }
    this->title_ = wxTRANSLATE("Current Year");
}

mmCurrentYearToDate::mmCurrentYearToDate()
: mmDateRange()
{
    this->findBeginOfMonth();
    this->start_date_.SetMonth(wxDateTime::Jan);
    this->end_date_ = today_;
    this->title_ = wxTRANSLATE("Current Year to Date");
}

mmLastYear::mmLastYear()
: mmDateRange()
{
    this->findBeginOfMonth();
    this->start_date_.SetMonth(wxDateTime::Jan);
    this->start_date_.Subtract(wxDateSpan::Years(1));
    this->end_date_ = this->start_date_;
    this->end_date_.Add(wxDateSpan::Months(11));
    this->findEndOfMonth();
    this->title_ = wxTRANSLATE("Last Year");
}

mmCurrentFinancialYear::mmCurrentFinancialYear()
: mmDateRange()
{
    int day = wxAtoi(Option::instance().FinancialYearStartDay());
    int month = wxAtoi(Option::instance().FinancialYearStartMonth());

    int this_month = this->start_date_.GetMonth() + 1;
    auto finDate = this->start_date_;
    finDate.SetDay(1).SetMonth(wxDateTime::Month(month - 1));
    auto last_month_day = finDate.GetLastMonthDay().GetDay();
    wxASSERT(day <= last_month_day);
    finDate.SetDay(day <= last_month_day ? day : last_month_day);

    if (finDate.IsLaterThan(this->start_date_))
        this->start_date_.Subtract(wxDateSpan::Year()).Add(wxDateSpan::Months(month - this_month));
    else
        this->start_date_.Subtract(wxDateSpan::Months(this_month - month));

    this->start_date_.Subtract(wxDateSpan::Days(this->start_date_.GetDay() - 1)).Add(wxDateSpan::Days(day - 1));
    
    this->end_date_ = this->start_date_;
    this->end_date_.Add(wxDateSpan::Year()).Subtract(wxDateSpan::Day());
    if (Option::instance().getIgnoreFutureTransactions() && (end_date_ > today_)) 
    {
        end_date_ = today_;
        futureIgnored_ = true;
    }
    this->title_ = wxTRANSLATE("Current Financial Year");
}

mmCurrentFinancialYearToDate::mmCurrentFinancialYearToDate()
: mmDateRange()
{
    mmCurrentFinancialYear current_financial_year;
    this->start_date_ = current_financial_year.start_date();
    // no change to end_date_

    this->title_ = wxTRANSLATE("Current Financial Year to Date");
}

mmLastFinancialYear::mmLastFinancialYear()
: mmDateRange()
{
    mmCurrentFinancialYear current_financial_year;
    this->start_date_ = current_financial_year.start_date().Subtract(wxDateSpan::Year());
    this->end_date_ = this->start_date_;
    this->end_date_.Add(wxDateSpan::Year()).Subtract(wxDateSpan::Day());

    this->title_ = wxTRANSLATE("Last Financial Year");
}

mmAllTime::mmAllTime()
: mmDateRange()
{
    this->start_date_.SetDay(1).SetMonth(wxDateTime::Jan).SetYear(1900);
    this->end_date_ = future_;
    if (Option::instance().getIgnoreFutureTransactions() && (end_date_ > today_)) 
    {
        end_date_ = today_;
        futureIgnored_ = true;
    }
    this->title_ = wxTRANSLATE("Over Time");
}

bool mmAllTime::is_with_date() const
{
    return false;
}

mmSpecifiedRange::mmSpecifiedRange(const wxDateTime& start, const wxDateTime& end)
: mmDateRange()
{
    this->title_ = wxTRANSLATE("Custom");
    this->start_date_ = start;
    this->end_date_ = end;
    if (Option::instance().getIgnoreFutureTransactions() && (end_date_ > today_)) 
    {
        end_date_ = today_;
        futureIgnored_ = true;
    }
}

mmLast365Days::mmLast365Days() : mmDateRange()
{
    this->start_date_.Subtract(wxDateSpan::Months(12)).Add(wxDateSpan::Days(1));
    // no change to end_date_
    this->title_ = wxTRANSLATE("Last 365 Days");
}
