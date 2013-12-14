/*******************************************************
Copyright (C) 2006-2012     Lisheng Guan (guanlisheng@gmail.com)

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
#include <wx/intl.h>

mmDateRange::mmDateRange()
: today_(wxDateTime::Today())
{
    start_date_ = today_;
    end_date_ = today_;
    title_ = "Date Range";
}

const wxDateTime mmDateRange::start_date() const
{
    return this->start_date_;
}

const wxDateTime mmDateRange::end_date() const
{
    return this->end_date_;
}

const wxDateTime mmDateRange::today() const
{
    return this->today_;
}

const bool mmDateRange::is_with_date() const
{
    return true;
}

const wxString mmDateRange::title() const
{
    return title_;
}

mmCurrentMonth::mmCurrentMonth()
: mmDateRange()
{
    this->start_date_ = wxDateTime(today_).SetDay(1);
    this->end_date_ = today_.GetLastMonthDay();
    this->title_ = _("Current Month");
}

mmToday::mmToday()
: mmDateRange()
{
    this->start_date_ = today_;
    this->end_date_ = today_;
    this->title_ = _("Today");
}

mmCurrentMonthToDate::mmCurrentMonthToDate()
: mmDateRange()
{
    this->start_date_ = wxDateTime(today_).SetDay(1);
    // no change to end_date_
    this->title_ = _("Current Month to Date");
}

mmLastMonth::mmLastMonth()
: mmDateRange()
{
    this->start_date_.Subtract(wxDateSpan::Months(1)).SetDay(1);
    this->end_date_ = wxDateTime(this->start_date_).GetLastMonthDay();
    this->title_ = _("Last Month");
}

mmLast30Days::mmLast30Days()
: mmDateRange()
{
    this->start_date_ = wxDateTime(start_date_)
        .Subtract(wxDateSpan::Months(1))
        .Add(wxDateSpan::Days(1));
    // no change to end_date_
    this->title_ = _("Last 30 Days");
}

mmLast90Days::mmLast90Days()
: mmDateRange()
{
    this->start_date_ = wxDateTime(start_date_)
        .Subtract(wxDateSpan::Months(3))
        .Add(wxDateSpan::Days(1));
    // no change to end_date_
    this->title_ = _("Last 90 Days");
}

mmLast3Months::mmLast3Months()
: mmDateRange()
{
    this->end_date_ = wxDateTime(this->start_date_).GetLastMonthDay();
    this->start_date_ = wxDateTime(end_date_).SetDay(1)
        .Add(wxDateSpan::Months(1))
        .Subtract(wxDateSpan::Months(3));
    this->title_ = _("Last 3 Months");
}

mmLast12Months::mmLast12Months()
: mmDateRange()
{
    this->end_date_ = wxDateTime(this->start_date_).GetLastMonthDay();
    this->start_date_ = wxDateTime(end_date_).SetDay(1)
        .Add(wxDateSpan::Months(1))
        .Subtract(wxDateSpan::Years(1));
    this->title_ = _("Last 12 Months");
}

mmCurrentYear::mmCurrentYear()
: mmDateRange()
{
    this->start_date_.SetDay(1).SetMonth(wxDateTime::Jan);
    this->end_date_ = wxDateTime(start_date_).SetMonth(wxDateTime::Dec).SetDay(31);
    this->title_ = _("Current Year");
}

mmCurrentYearToDate::mmCurrentYearToDate()
: mmDateRange()
{
    this->start_date_.SetDay(1).SetMonth(wxDateTime::Jan);
    // no change to end_date_
    this->title_ = _("Current Year to Date");
}

mmLastYear::mmLastYear()
: mmDateRange()
{
    this->start_date_.Subtract(wxDateSpan::Years(1)).SetDay(1).SetMonth(wxDateTime::Jan);
    this->end_date_ = wxDateTime(start_date_).SetMonth(wxDateTime::Dec).SetDay(31);
    this->title_ = _("Last Year");
}

mmCurrentFinancialYear::mmCurrentFinancialYear(const int day, const int month)
: mmDateRange()
{
    this->start_date_.SetDay(1).SetMonth(wxDateTime::Jan);
    this->end_date_ = wxDateTime(start_date_).SetMonth(wxDateTime::Dec).SetDay(31);
    this->start_date_.Add(wxDateSpan::Days(day-1)).Add(wxDateSpan::Months(month-1));
    this->end_date_.Add(wxDateSpan::Days(day-1)).Add(wxDateSpan::Months(month-1));

    if (today_ < start_date_)
    {
        start_date_.Subtract(wxDateSpan::Years(1));
        end_date_.Subtract(wxDateSpan::Years(1));
    }
    this->title_ = _("Current Financial Year");
}

mmLastFinancialYear::mmLastFinancialYear(const int day, const int month)
: mmDateRange()
{
    this->start_date_.SetDay(1).SetMonth(wxDateTime::Jan)
        .Add(wxDateSpan::Days(day-1))
        .Add(wxDateSpan::Months(month-1))
        .Subtract(wxDateSpan::Years(1));;
    this->end_date_ = wxDateTime(start_date_).SetMonth(wxDateTime::Dec)
        .SetDay(31).Add(wxDateSpan::Days(day-1))
        .Add(wxDateSpan::Months(month-1));

    if (today_ < start_date_)
    {
        this->start_date_.Subtract(wxDateSpan::Years(1));
        this->end_date_.Subtract(wxDateSpan::Years(1));
    }
    this->title_ = _("Last Financial Year");
}

mmAllTime::mmAllTime()
: mmDateRange()
{
    this->title_ = _("Over Time");
    this->start_date_.SetDay(1).SetMonth(wxDateTime::Jan).SetYear(1);
    this->end_date_.SetYear(9999);
}

const bool mmAllTime::is_with_date() const
{
    return false;
}

mmSpecifiedRange::mmSpecifiedRange(wxDateTime& start, wxDateTime& end)
: mmDateRange()
{
    this->title_ = "";
    this->start_date_ = start;
    this->end_date_ = end;
}
