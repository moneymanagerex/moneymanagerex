/*******************************************************
Copyright (C) 2006-2012     Lisheng Guan (guanlisheng@gmail.com)
Copyright (C) 2014 - 2022   Nikolay Akimov
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

#include <wx/intl.h>

#include "constants.h"
#include "DateRange.h"
#include "model/PreferencesModel.h"

DateRange::DateRange() :
    today_(wxDateTime::Today()),
    today_end_(wxDateTime(23, 59, 59, 999)),
    future_(DATE_MAX),
    futureIgnored_(false)
{
    start_date_ = today_;
    end_date_ = today_end_;
    startDay_ = PreferencesModel::instance().getReportingFirstDay();
    title_ = _n("Date Range");
}

DateRange::~DateRange()
{
}

void DateRange::destroy()
{
    delete this;
}

const wxString DateRange::local_title() const
{
    return wxGetTranslation(title_);
}

void DateRange::findBeginOfMonth()
{
    if (this->today_.GetDay() < startDay_)
        this->start_date_.Subtract(wxDateSpan::Months(1));
    start_date_.SetDay(startDay_);
}

void DateRange::findEndOfMonth()
{
    if (this->today_.GetDay() < startDay_)
        this->end_date_.Subtract(wxDateSpan::Months(1));
    end_date_ = getDayEnd(end_date_.Add(wxDateSpan::Months(1)).SetDay(1).Subtract(wxDateSpan::Day()).Add(wxDateSpan::Days(startDay_ - 1)));
}

mmCurrentMonth::mmCurrentMonth()
: DateRange()
{
    this->findEndOfMonth();
    this->findBeginOfMonth();
    if (PreferencesModel::instance().getIgnoreFutureTransactions() && (end_date_ > today_end_)) 
    {
        end_date_ = today_end_;
        futureIgnored_ = true;
    }
    this->title_ = _n("Current Month");
}

mmToday::mmToday()
: DateRange()
{
    this->title_ = _n("Today");
}

mmCurrentMonthToDate::mmCurrentMonthToDate()
: DateRange()
{
    findBeginOfMonth();
    this->end_date_ = today_end_;
    this->title_ = _n("Current Month to Date");
}

mmLastMonth::mmLastMonth()
: DateRange()
{
    this->end_date_.Subtract(wxDateSpan::Months(1));
    this->findEndOfMonth();
    this->start_date_.Subtract(wxDateSpan::Months(1));
    this->findBeginOfMonth();
    this->title_ = _n("Previous Month");
}

mmLast30Days::mmLast30Days()
: DateRange()
{
    this->start_date_
        .Subtract(wxDateSpan::Months(1))
        .Add(wxDateSpan::Days(1));
    // no change to end_date_
    this->title_ = _n("1 Month Ago to Date");
}

mmLast90Days::mmLast90Days()
: DateRange()
{
    this->start_date_
        .Subtract(wxDateSpan::Months(3))
        .Add(wxDateSpan::Days(1));
    // no change to end_date_
    this->title_ = _n("3 Months Ago to Date");
}

mmLast3Months::mmLast3Months()
: DateRange()
{
    this->findEndOfMonth();
    this->start_date_.Subtract(wxDateSpan::Months(2));
    this->findBeginOfMonth();
    if (PreferencesModel::instance().getIgnoreFutureTransactions() && (end_date_ > today_end_)) 
    {
        end_date_ = today_end_;
        futureIgnored_ = true;
    }
    this->title_ = _n("Latest 3 Months");
}

mmLast12Months::mmLast12Months()
: DateRange()
{
    this->findEndOfMonth();
    this->start_date_.Subtract(wxDateSpan::Months(11));
    this->findBeginOfMonth();
    if (PreferencesModel::instance().getIgnoreFutureTransactions() && (end_date_ > today_end_)) 
    {
        end_date_ = today_end_;
        futureIgnored_ = true;
    }
    this->title_ = _n("Latest 12 Months");
}

mmCurrentYear::mmCurrentYear()
: DateRange()
{
    this->findBeginOfMonth();
    this->start_date_.SetMonth(wxDateTime::Jan);
    this->end_date_ = this->start_date_;
    this->end_date_.Add(wxDateSpan::Months(11));
    this->findEndOfMonth();
    if (PreferencesModel::instance().getIgnoreFutureTransactions() && (end_date_ > today_end_)) 
    {
        end_date_ = today_end_;
        futureIgnored_ = true;
    }
    this->title_ = _n("Current Year");
}

mmCurrentYearToDate::mmCurrentYearToDate()
: DateRange()
{
    this->findBeginOfMonth();
    this->start_date_.SetMonth(wxDateTime::Jan);
    this->end_date_ = today_end_;
    this->title_ = _n("Current Year to Date");
}

mmLastYear::mmLastYear()
: DateRange()
{
    this->findBeginOfMonth();
    this->start_date_.SetMonth(wxDateTime::Jan);
    this->start_date_.Subtract(wxDateSpan::Years(1));
    this->end_date_ = this->start_date_;
    this->end_date_.Add(wxDateSpan::Months(11));
    this->findEndOfMonth();
    this->title_ = _n("Previous Year");
}

mmLastYearBefore::mmLastYearBefore()
: DateRange()
{
    this->findBeginOfMonth();
    this->start_date_.SetMonth(wxDateTime::Jan);
    this->start_date_.Subtract(wxDateSpan::Years(2));
    this->end_date_ = this->start_date_;
    this->end_date_.Add(wxDateSpan::Months(11));
    this->findEndOfMonth();
    this->title_ = _n("Year Before Last");
}

mmCurrentFinancialYear::mmCurrentFinancialYear()
: DateRange()
{
    int day = PreferencesModel::instance().getFinancialFirstDay();
    wxDateTime::Month month = PreferencesModel::instance().getFinancialFirstMonth();

    wxDateTime::Month this_month = this->start_date_.GetMonth();
    auto finDate = this->start_date_;
    finDate.SetDay(1).SetMonth(month);
    auto last_month_day = finDate.GetLastMonthDay().GetDay();
    wxASSERT(day <= last_month_day);
    finDate.SetDay(day <= last_month_day ? day : last_month_day);

    if (finDate.IsLaterThan(this->start_date_))
        this->start_date_.Subtract(wxDateSpan::Year());
    this->start_date_.Add(wxDateSpan::Months(month - this_month));

    this->start_date_.Subtract(wxDateSpan::Days(this->start_date_.GetDay() - day));
    
    this->end_date_ = this->start_date_;
    end_date_ = getDayEnd(end_date_.Add(wxDateSpan::Year()).Subtract(wxDateSpan::Day()));

    if (PreferencesModel::instance().getIgnoreFutureTransactions() && (end_date_ > today_end_)) 
    {
        end_date_ = today_end_;
        futureIgnored_ = true;
    }

    this->title_ = _n("Current Financial Year");
}

mmCurrentFinancialYearToDate::mmCurrentFinancialYearToDate()
: DateRange()
{
    mmCurrentFinancialYear current_financial_year;
    this->start_date_ = current_financial_year.start_date();
    // no change to end_date_

    this->title_ = _n("Current Financial Year to Date");
}

mmLastFinancialYear::mmLastFinancialYear()
: DateRange()
{
    mmCurrentFinancialYear current_financial_year;
    this->start_date_ = current_financial_year.start_date().Subtract(wxDateSpan::Year());
    this->end_date_ = this->start_date_;
    end_date_ = getDayEnd(end_date_.Add(wxDateSpan::Year()).Subtract(wxDateSpan::Day()));
    this->title_ = _n("Previous Financial Year");
}

mmAllTime::mmAllTime()
: DateRange()
{
    this->start_date_.SetDay(1).SetMonth(wxDateTime::Jan).SetYear(1900);
    this->end_date_ = future_;
    if (PreferencesModel::instance().getIgnoreFutureTransactions() && (end_date_ > today_end_)) 
    {
        end_date_ = today_end_;
        futureIgnored_ = true;
    }
    this->title_ = _n("Over Time");
}

bool mmAllTime::is_with_date() const
{
    return false;
}

mmSpecifiedRange::mmSpecifiedRange(const wxDateTime& start, const wxDateTime& end)
: DateRange()
{
    this->title_ = _n("Custom");
    this->start_date_ = start;
    this->end_date_ = end;
    if (PreferencesModel::instance().getIgnoreFutureTransactions() && (end_date_ > today_end_)) 
    {
        end_date_ = today_end_;
        futureIgnored_ = true;
    }
}

mmLast365Days::mmLast365Days() : DateRange()
{
    this->start_date_.Subtract(wxDateSpan::Months(12)).Add(wxDateSpan::Days(1));
    // no change to end_date_
    this->title_ = _n("1 Year Ago to Date");
}

mmLastNDays::mmLastNDays(int days)
    : DateRange()
{
    this->start_date_
        .Subtract(wxDateSpan::Days(days))
        .Add(wxDateSpan::Days(1));
    // no change to end_date_
    this->title_ = wxString::Format(_t("Last %i Days"), days);
}

void mmLastNDays::SetRange(int days)
{
    // recalculate start_date_
    this->start_date_ = today_;
    this->start_date_
        .Subtract(wxDateSpan::Days(days))
        .Add(wxDateSpan::Days(1));
    // no change to end_date_
    // update title
    this->title_ = wxString::Format(_t("Last %i Days"), days);
}

mmSinseToday::mmSinseToday() : DateRange()
{
    this->end_date_ = future_;
    this->title_ = _n("Since Today");
}

mmSinse30days::mmSinse30days() : DateRange()
{
    this->start_date_
        .Subtract(wxDateSpan::Months(1))
        .Add(wxDateSpan::Days(1));
    this->end_date_ = future_;
    this->title_ = _n("Since 1 Month Ago");
}

mmSinse90days::mmSinse90days() : DateRange()
{
    this->start_date_
        .Subtract(wxDateSpan::Months(3))
        .Add(wxDateSpan::Days(1));
    this->end_date_ = future_;
    this->title_ = _n("Since 3 Months Ago");
}

mmSinseCurrentYear::mmSinseCurrentYear() : DateRange()
{
    this->findBeginOfMonth();
    this->start_date_.SetMonth(wxDateTime::Jan);
    this->end_date_ = future_;
    this->title_ = _n("Since Start of This Year");
}

mmSinseCurrentFinancialYear::mmSinseCurrentFinancialYear() : mmCurrentFinancialYear()
{
    futureIgnored_ = false;
    this->end_date_ = future_;
    this->title_ = _n("Since Start of Financial Year");
}
