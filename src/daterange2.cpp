/*******************************************************
 Copyright (C) 2025 George Ef (george.a.ef@gmail.com)

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

#include "daterange2.h"
#include "constants.h"
#include "option.h"

DateDay::DateDay() :
    dateTime{wxDateTime()}
{
}

DateDay::DateDay(wxDateTime dateTime_new) :
    dateTime{dateTime_new}
{
    if (dateTime.IsValid()) {
        // set time to noon (12:00)
        dateTime.SetHour(12).SetMinute(0).SetSecond(0).SetMillisecond(0);
    }
}

const DateRange2::PERIOD_INFO_t DateRange2::PERIOD_INFO[] =
{
    { PERIOD_ID_A, "A" },
    { PERIOD_ID_Y, "Y" },
    { PERIOD_ID_Q, "Q" },
    { PERIOD_ID_M, "M" },
    { PERIOD_ID_W, "W" },
    { PERIOD_ID_T, "T" },
    { PERIOD_ID_S, "S" },
};
const DateRange2::PERIOD_LABEL_ID_t DateRange2::PERIOD_LABEL_ID = make_period_label_id();

DateRange2::PERIOD_LABEL_ID_t DateRange2::make_period_label_id()
{
    PERIOD_LABEL_ID_t period_label_id;
    for (int i = 0; i < static_cast<int>(sizeof(PERIOD_INFO)/sizeof(PERIOD_INFO[0])); ++i) {
        char c = PERIOD_INFO[i].label[0];
        period_label_id[c] = PERIOD_INFO[i].id;
    }
    return period_label_id;
}

wxDateSpan DateRange2::span(int offset, DateRange2::PERIOD_ID period)
{
    if (period == PERIOD_ID_Y)
        return wxDateSpan::Years(offset);
    else if (period == PERIOD_ID_Q)
        return wxDateSpan::Months(3*offset);
    else if (period == PERIOD_ID_M)
        return wxDateSpan::Months(offset);
    else if (period == PERIOD_ID_W)
        return wxDateSpan::Weeks(offset);
    else if (period == PERIOD_ID_T || period == PERIOD_ID_S)
        return wxDateSpan::Days(offset);
    else
        return wxDateSpan();
}

DateRange2::Range::Range(
    int so1_new, PERIOD_ID sp1_new,
    int eo1_new, PERIOD_ID ep1_new,
    int so2_new, PERIOD_ID sp2_new,
    int eo2_new, PERIOD_ID ep2_new,
    int f_new, wxString name_new
) {
    // sp1/ep1 cannot be PERIOD_ID_none
    sp1 = (sp1_new == PERIOD_ID_none) ? PERIOD_ID_A : sp1_new;
    ep1 = (ep1_new == PERIOD_ID_none) ? PERIOD_ID_A : ep1_new;
    // so1/eo1 is not applicable if sp1/ep1 resp., is PERIOD_ID_A
    so1 = (sp1 == PERIOD_ID_A) ? 0 : so1_new;
    eo1 = (ep1 == PERIOD_ID_A) ? 0 : eo1_new;
    // either both sp2 and ep2 are PERIOD_ID_none, or none of them
    if (sp2_new == PERIOD_ID_none || ep2_new == PERIOD_ID_none) {
        // so2/eo2 are not applicable if sp2/ep2 are PERIOD_ID_none
        sp2 = PERIOD_ID_none; so2 = 0;
        ep2 = PERIOD_ID_none; eo2 = 0;
    }
    else {
        // so2/eo2 is not applicable if sp2/ep2 resp., is PERIOD_ID_A
        sp2 = sp2_new; so2 = (sp2 == PERIOD_ID_A) ? 0 : so2_new;
        ep2 = ep2_new; eo2 = (ep2 == PERIOD_ID_A) ? 0 : eo2_new;
    }
    // f is either 0 (calendar) or 1 (financial)
    f = (f_new == 1) ? 1 : 0;
    name = name_new;
}

void DateRange2::Range::scanWhiteSpace(StringIt &buffer_i, StringIt buffer_end)
{
    const wxString ws = " \t\r\n"; 
    while (buffer_i != buffer_end && ws.Find(*buffer_i) != wxNOT_FOUND)
        ++buffer_i;
}

// the returned token is one of:
//   o : offset
//   p : period
//   . : .. (range)
//   , : , (subrange separator)
//   f : F (financial)
//   ; : ; or end
//   _ : error
// token_o is set if token==o; token_p is set if token==p
char DateRange2::Range::scanToken(
    StringIt &buffer_i, StringIt buffer_end,
    int &token_o, PERIOD_ID &token_p
) {
    scanWhiteSpace(buffer_i, buffer_end);
    if (buffer_i == buffer_end)
        return ';';
    char c;
    if (!(*buffer_i).IsAscii() || !(*buffer_i).GetAsChar(&c))
        return '_';

    if (c == ',' || c == ';') {
        ++buffer_i;
        return c;
    }

    if (c == 'F') {
        ++buffer_i;
        return 'f';
    }

    if (c == '.') {
        ++buffer_i;
        if (buffer_i == buffer_end || !(*buffer_i).IsAscii() || !(*buffer_i).GetAsChar(&c))
            return '_';
        if (c == '.') {
            ++buffer_i;
            return '.';
        }
        else {
            return '_';
        }
    }

    char c1 = 0;
    if (c == '+' || c == '-') {
        c1 = c;
        ++buffer_i;
        if (buffer_i == buffer_end || !(*buffer_i).IsAscii() || !(*buffer_i).GetAsChar(&c))
            return '_';
    }
    if (isdigit(c)) {
        StringIt str_start = buffer_i;
        do { ++buffer_i; } while (
            buffer_i != buffer_end && (*buffer_i).IsAscii() && (*buffer_i).GetAsChar(&c)
            && isdigit(c)
        );
        wxString str_o = "";
        str_o.append(str_start, buffer_i);
        token_o = wxAtoi(str_o);
        if (c1 == '-') token_o = -token_o;
        return 'o';
    }
    if (c1)
        return '_';

    auto label_it = PERIOD_LABEL_ID.find(c);
    if (label_it != PERIOD_LABEL_ID.end()) {
        ++buffer_i;
        token_p = label_it->second;
        return 'p';
    }

    return '_';
}

// return true if parse is successful
bool DateRange2::Range::parseLabel(StringIt &buffer_i, StringIt buffer_end)
{
    // range = subrange ("," subrange)? ("F")? (";" name)?
    // subrange = so? sp ".." eo? ep
    // subrange = so ".." eo p
    // subrange = o? p

    int so_new[2] = { 0, 0 }; PERIOD_ID sp_new[2] = { PERIOD_ID_none, PERIOD_ID_none };
    int eo_new[2] = { 0, 0 }; PERIOD_ID ep_new[2] = { PERIOD_ID_none, PERIOD_ID_none };
    int f_new = 0;
    int i = 0;     // index into {s,e}{o,p}[] (0: first subrange, 1: second subrange)
    int state = 0; // parse state: 0 (so) 1 (sp) 2 (..) 3 (eo) 4 (ep) 5 (f) 6 (;) 7

    while (1) {
        char token;                         // one of [op.,@f;_]
        int token_o = 0;                    // offset (if token == 'o')
        PERIOD_ID token_p = PERIOD_ID_none; // period (if token == 'p')
        token = scanToken(buffer_i, buffer_end, token_o, token_p);
        //wxLogDebug("DEBUG: state=%d, token=%c", state, token);
        if (state == 0 && token == 'o') {
            so_new[i] = token_o;
            state = 1;
            continue;
        }
        if ((state == 0 || state == 1) && token == 'p') {
            sp_new[i] = token_p;
            state = 2;
            continue;
        }
        if ((state == 1 || state == 2) && token == '.') {
            state = 3;
            continue;
        }
        if (state == 3 && token == 'o') {
            eo_new[i] = token_o;
            state = 4;
            continue;
        }
        if ((state == 3 || state == 4) && token == 'p') {
            if (sp_new[i] == PERIOD_ID_none)
                sp_new[i] = token_p;
            ep_new[i] = token_p;
            state = 5;
            continue;
        }
        if (state == 2 && ((i == 0 && token == ',') || token == 'f' || token == ';')) {
            eo_new[i] = so_new[i];
            ep_new[i] = sp_new[i];
            state = 5;
        }
        if (i == 0 && state == 5 && token == ',') {
            i = 1;
            state = 0;
            continue;
        }
        if (state == 5 && token == 'f') {
            f_new = 1;
            state = 6;
            continue;
        }
        if ((state == 5 || state == 6) && token == ';') {
            state = 7;
            break;
        }
        break;
    }

    if (state != 7) {
        wxLogDebug("ERROR: DateRange2::Range::parseLabel(): state=%d", state);
        return false;
    }

    so1 = so_new[0]; sp1 = sp_new[0]; eo1 = eo_new[0]; ep1 = ep_new[0];
    so2 = so_new[1]; sp2 = sp_new[1]; eo2 = eo_new[1]; ep2 = ep_new[1];
    f = f_new;

    return true;
}

void DateRange2::Range::parseName(StringIt &buffer_i, StringIt buffer_end)
{
    name = "";
    scanWhiteSpace(buffer_i, buffer_end);
    name.append(buffer_i, buffer_end);
}

bool DateRange2::Range::parseLabelName(const wxString &buffer, const wxString &name_new)
{
    Range range_new = Range();
    StringIt buffer_i = buffer.begin();
    if (!range_new.parseLabel(buffer_i, buffer.end()))
        return false;
    if (!name_new.empty())
        range_new.name = name_new;
    else
        range_new.parseName(buffer_i, buffer.end());
    *this = range_new;
    return true;
}

const wxString DateRange2::Range::offsetRangeStr(int so, int eo, bool show_zero)
{
    if (so == eo) {
        return offsetStr(so, show_zero);
    }
    else {
        StringBuilder s;
        s.append(offsetStr(so, true));
        s.append("..");
        s.append(offsetStr(eo, true));
        return s.buffer;
    }
}

const wxString DateRange2::Range::getLabel() const
{
    StringBuilder sb;

    // first range
    wxString sp1_label = PERIOD_INFO[sp1].label;
    if (sp1 == ep1) {
        sb.append(offsetRangeStr(so1, eo1)); sb.sep(); sb.append(sp1_label);
    }
    else {
        wxString ep1_label = PERIOD_INFO[ep1].label;
        sb.append(offsetStr(so1)); sb.sep(); sb.append(sp1_label);
        sb.sep(); sb.append(".."); sb.sep();
        sb.append(offsetStr(eo1)); sb.sep(); sb.append(ep1_label);
    }

    // second range
    if (sp2 != PERIOD_ID_none && ep2 != PERIOD_ID_none) {
        sb.append(","); sb.sep();
        wxString sp2_label = PERIOD_INFO[sp2].label;
        if (sp2 == ep2) {
            sb.append(offsetRangeStr(so2, eo2)); sb.sep(); sb.append(sp2_label);
        }
        else {
            wxString ep2_label = PERIOD_INFO[ep2].label;
            sb.append(offsetStr(so2)); sb.sep(); sb.append(sp2_label);
            sb.sep(); sb.append(".."); sb.sep();
            sb.append(offsetStr(eo2)); sb.sep(); sb.append(ep2_label);
        }
    }

    // financial
    if (f == 1) {
        sb.sep(); sb.append("F");
    }

    return sb.buffer;
}

const wxString DateRange2::Range::getLabelName() const
{
    wxString s = getLabel();
    if (!name.empty()) {
        s.append("; ");
        s.append(name);
    }
    return s;
}

const wxString DateRange2::Range::checkingName() const
{
    wxString str = getLabel();
    // TODO
    return str;
}

const wxString DateRange2::Range::checkingDescription() const
{
    static StringBuilder sb;
    sb.reset();
    sb.append(getLabel());
    // TODO
    return sb.buffer;
}

DateRange2::Reporting::Reporting(
    int m_new, PERIOD_ID p_new
) {
    // p cannot be PERIOD_ID_S (statement date)
    p = (p_new == PERIOD_ID_S) ? PERIOD_ID_T : p_new;

    // m is not applicable if p is PERIOD_ID_none;
    // if it is applicable, it cannot be 0;
    // it is normalized to 1 if p is PERIOD_ID_A
    m = (p == PERIOD_ID_none) ? 0
      : (p == PERIOD_ID_A) ? 1
      : (m_new == 0) ? 1
      : m_new;
}

// return true if parse is successful
bool DateRange2::Reporting::parseLabel(StringIt &buffer_i, StringIt buffer_end)
{
    // reporting = m? p

    int m_new = 0; PERIOD_ID p_new = PERIOD_ID_none;
    int state = 0; // parse state: 0 (m) 1 (p) 2 (;) 3

    while (1) {
        char token;                         // one of [op;_]
        int token_o = 0;                    // offset (if token == 'o')
        PERIOD_ID token_p = PERIOD_ID_none; // period (if token == 'p')
        token = DateRange2::Range::scanToken(buffer_i, buffer_end, token_o, token_p);
        //wxLogDebug("DEBUG: state=%d, token=%c", state, token);
        if (state == 0 && token == 'o') {
            m_new = token_o;
            state = 1;
            continue;
        }
        if ((state == 0 || state == 1) && token == 'p') {
            if (m_new == 0 || token_p == PERIOD_ID_A)
                m_new = 1;
            p_new = (token_p == PERIOD_ID_S) ? PERIOD_ID_T : token_p;
            state = 2;
            continue;
        }
        if ((state == 0 || state == 2) && token == ';') {
            state = 3;
            break;
        }
        break;
    }

    if (state != 3) {
        wxLogDebug("ERROR: DateRange2::Reporting::parseLabel(): state=%d", state);
        return false;
    }

    m = m_new;
    p = p_new;

    return true;
}

const wxString DateRange2::Reporting::getLabel() const
{
    if (p == PERIOD_ID_none || m == 0)
        return "";
    StringBuilder sb;
    wxString p_label = PERIOD_INFO[p].label;
    sb.append(multiplierStr(m)); sb.sep(); sb.append(p_label);
    return sb.buffer;
}

DateRange2::DateRange2(
    DateDay date_s_new, DateDay date_t_new,
    DateDay default_start_new, DateDay default_end_new
) :
    firstDay{
        Option::instance().getReportingFirstDay(),
        Option::instance().getFinancialFirstDay()
    },
    firstMonth{
        wxDateTime::Jan,
        Option::instance().getFinancialFirstMonth()
    },
    firstWeekday(
        Option::instance().getReportingFirstWeekday()
    ),
    range(Range()),
    reporting(Reporting())
{
    setDateS(date_s_new);
    setDateT(date_t_new);
    setDefaultStart(default_start_new);
    setDefaultEnd(default_end_new);
}

#ifndef NDEBUG
DateRange2::DateRange2(
    int firstDay_new_0, int firstDay_new_1,
    wxDateTime::Month firstMonth_new_0, wxDateTime::Month firstMonth_new_1,
    wxDateTime::WeekDay firstWeekday_new,
    DateDay date_s_new, DateDay date_t_new,
    DateDay default_start_new, DateDay default_end_new
) :
    firstDay{firstDay_new_0, firstDay_new_1},
    firstMonth{firstMonth_new_0, firstMonth_new_1},
    firstWeekday(firstWeekday_new),
    range(Range()),
    reporting(Reporting())
{
    setDateS(date_s_new);
    setDateT(date_t_new);
    setDefaultStart(default_start_new);
    setDefaultEnd(default_end_new);
}
#endif

// return true if parse is successful
bool DateRange2::parseRange(const wxString &buffer, const wxString &name)
{
    Range range_new = Range();
    if (!range_new.parseLabelName(buffer, name))
        return false;
    range = range_new;
    return true;
}

// return true if parse is successful
bool DateRange2::parseReporting(const wxString &buffer)
{
    Reporting reporting_new = Reporting();
    StringIt buffer_i = buffer.begin();
    if (!reporting_new.parseLabel(buffer_i, buffer.end()))
        return false;
    reporting = reporting_new;
    return true;
}

DateDay DateRange2::periodStart(DateDay date, PERIOD_ID period) const
{
    if (!date.isDefined() || period < PERIOD_ID_Y || period > PERIOD_ID_S)
        return DateDay();
    wxDateTime start_dateTime = date.getDateTime();
    if (period == PERIOD_ID_Y || period == PERIOD_ID_Q || period == PERIOD_ID_M) {
        if (start_dateTime.GetDay() < firstDay[range.f])
            start_dateTime -= wxDateSpan::Months(1);
        start_dateTime.SetDay(firstDay[range.f]);
        if (period == PERIOD_ID_Y) {
            if (start_dateTime.GetMonth() < firstMonth[range.f])
                start_dateTime -= wxDateSpan::Years(1);
            start_dateTime.SetMonth(firstMonth[range.f]);
        }
        else if (period == PERIOD_ID_Q) {
            int m = (start_dateTime.GetMonth() - firstMonth[range.f] + 12) % 3;
            if (m > 0)
                start_dateTime -= wxDateSpan::Months(m);
        }
    }
    else if (period == PERIOD_ID_W) {
        int d = (start_dateTime.GetWeekDay() - firstWeekday + 7) % 7;
        if (d > 0)
            start_dateTime -= wxDateSpan::Days(d);
    }
    return DateDay(start_dateTime);
}

DateDay DateRange2::periodEnd(DateDay date, PERIOD_ID period) const
{
    if (!date.isDefined() || period < PERIOD_ID_Y || period > PERIOD_ID_S)
        return DateDay();
    wxDateTime end_dateTime = date.getDateTime();
    if (period == PERIOD_ID_Y || period == PERIOD_ID_Q || period == PERIOD_ID_M) {
        if (end_dateTime.GetDay() >= firstDay[range.f])
            end_dateTime += wxDateSpan::Months(1);
        end_dateTime.SetDay(firstDay[range.f]);
        if (period == PERIOD_ID_Y) {
            if (end_dateTime.GetMonth() > firstMonth[range.f])
                end_dateTime += wxDateSpan::Years(1);
            end_dateTime.SetMonth(firstMonth[range.f]);
        }
        else if (period == PERIOD_ID_Q) {
            int m = (firstMonth[range.f] - end_dateTime.GetMonth() + 12) % 3;
            if (m > 0)
                end_dateTime += wxDateSpan::Months(m);
        }
        end_dateTime -= wxDateSpan::Days(1);
    }
    else if (period == PERIOD_ID_W) {
        int d = (firstWeekday - end_dateTime.GetWeekDay() + 6) % 7;
        if (d > 0)
            end_dateTime += wxDateSpan::Days(d);
    }
    return DateDay(end_dateTime);
}

DateDay DateRange2::rangeStart() const
{
    if (range.sp1 == PERIOD_ID_A || range.sp2 == PERIOD_ID_A)
        return default_start;
    DateDay start_date1 = (range.sp1 == PERIOD_ID_S) ? date_s : date_t;
    if (range.so1 != 0)
        start_date1.addSpan(span(range.so1, range.sp1));
    start_date1 = periodStart(start_date1, range.sp1);
    if (!start_date1.isDefined() || range.sp2 == PERIOD_ID_none || range.ep2 == PERIOD_ID_none)
        return start_date1;
    DateDay start_date2 = (range.sp2 == PERIOD_ID_S) ? date_s : date_t;
    if (range.so2 != 0)
        start_date2.addSpan(span(range.so2, range.sp2));
    start_date2 = periodStart(start_date2, range.sp1 > range.sp2 ? range.sp1 : range.sp2);
    if (!start_date2.isDefined())
        return DateDay();
    return start_date1 <= start_date2 ? start_date1 : start_date2;
}

DateDay DateRange2::rangeEnd() const
{
    if (range.ep1 == PERIOD_ID_A || range.ep2 == PERIOD_ID_A)
        return default_end;
    DateDay end_date1 = (range.ep1 == PERIOD_ID_S) ? date_s : date_t;
    if (range.eo1 != 0)
        end_date1.addSpan(span(range.eo1, range.ep1));
    end_date1 = periodEnd(end_date1, range.ep1);
    if (!end_date1.isDefined() || range.sp2 == PERIOD_ID_none || range.ep2 == PERIOD_ID_none)
        return end_date1;
    DateDay end_date2 = (range.ep2 == PERIOD_ID_S) ? date_s : date_t;
    if (range.eo2 != 0)
        end_date2.addSpan(span(range.eo2, range.ep2));
    end_date2 = periodEnd(end_date2, range.ep1 > range.ep2 ? range.ep1 : range.ep2);
    if (!end_date2.isDefined())
        return DateDay();
    return end_date2 <= end_date1 ? end_date1 : end_date2;
}

DateDay DateRange2::reportingNext() const
{
    DateDay range_start = rangeStart();
    DateDay range_end = rangeEnd();

    if (!range_start.isDefined() || !range_end.isDefined())
        return range_end;
    if (reporting.p == PERIOD_ID_none || reporting.p == PERIOD_ID_A || reporting.m == 0)
        return range_end;

    if (reporting.m > 0) {
        // return the end of the multi-period aligned at range_start
        // (i.e., its first period contains range_start)
        DateDay next = periodEnd(range_start, reporting.p);
        if (reporting.m > 1) {
            next.addSpan(span(reporting.m - 1, reporting.p));
            next = periodEnd(next, reporting.p);
        }
        return next <= range_end ? next : range_end;
    }
    else { // if (reporting.m < 0)
        // return the end of the multi-period aligned at range_end
        // (i.e., its last period contains range_end)
        DateDay next = periodEnd(range_end, reporting.p);
        DateDay next1 = next;
        next1.addSpan(span(reporting.m, reporting.p));
        while (range_start <= next1) {
            next = next1;
            next1.addSpan(span(reporting.m, reporting.p));
            next1 = periodEnd(next1, reporting.p);
        }
        return next <= range_end ? next : range_end;
    }
}

const wxString DateRange2::checkingTooltip() const
{
    static StringBuilder sb;
    sb.reset();

    DateDay date1 = rangeStart();
    DateDay date2 = rangeEnd();
    if (date1.isDefined())
        sb.append(date1.dateISO());
    sb.sep(); sb.append(".."); sb.sep();
    if (date2.isDefined())
        sb.append(date2.dateISO());
    sb.flush();

    sb.append("\n");
    sb.append(range.checkingDescription());
    return sb.buffer;
}

const wxString DateRange2::reportingTooltip() const
{
    // TODO
    return "";
}

DateRange2::ReportingIterator::ReportingIterator(const DateRange2* a_new) :
    a(a_new),
    count(0),
    next(a_new->reportingNext()),
    last(a_new->rangeEnd())
{
}

void DateRange2::ReportingIterator::increment()
{
    // The iterator reaches the end when `next` is equal to `last` (this includes
    // the special case of open end, in which `last` is undefined).
    // `count` is initialized to 0 and set to -1 at the end.
    // Notice that the iterator returns at least one DateDay before it reaches the end.

    if (count == -1)
        return;
    if (next == last) {
        count = -1;
        return;
    }
    if (!next.isDefined() || !last.isDefined()) {
        // This should not happen.
        // If one of `next` and `last` is undefined, the other should also be undefined.
        wxLogDebug("ERROR in DateRange2::ReportingIterator: only one of `next` and `last` is defined");
        count = -1;
        return;
    }

    int rm = a->reporting.m;
    if (rm < 0) rm = -rm;
    PERIOD_ID rp = a->reporting.p;
    DateDay next1 = next;
    next1.addSpan(DateRange2::span(rm, rp));
    next1 = a->periodEnd(next1, rp);
    if (last < next1)
        next1 = last;

    if (next1 <= next) {
        // This should not happen.
        // `next` must strictly increase in each step.
        wxLogDebug("ERROR in DateRange2::ReportingIterator: cannot increment `next`");
        count = -1;
        return;
    }

    ++count;
    next = next1;
}

#ifndef NDEBUG
bool DateRange2::debug()
{
    bool ok = true;
    wxLogDebug("{{{ DateRange2::debug()");

    // check order in PERIOD_INFO
    for (int i = 0; i < static_cast<int>(sizeof(PERIOD_INFO)/sizeof(PERIOD_INFO[0])); i++) {
        wxASSERT_MSG(PERIOD_INFO[i].id == i, "Wrong order in PERIOD_INFO");
    }

    // create a DateRange2 object; default_start is undefined
    wxDateTime dateTime_s, dateTime_t, date_default_end;
    dateTime_s.ParseISOCombined("2024-08-30T00:00:01"); // Fri
    dateTime_t.ParseISOCombined("2025-01-30T00:00:01"); // Thu
    date_default_end.ParseISODate("2025-01-31"); // Fri
    DateRange2 dr = DateRange2(
        1, 6,
        wxDateTime::Month::Jan, wxDateTime::Month::Apr,
        wxDateTime::WeekDay::Mon,
        DateDay(dateTime_s), DateDay(dateTime_t),
        DateDay(), DateDay(date_default_end)
    );
    wxLogDebug("INFO: date_s.dateTime=[%s]", dateTimeISO(dr.getDateS().getDateTime()));
    wxLogDebug("INFO: date_t.dateTime=[%s]", dateTimeISO(dr.getDateT().getDateTime()));
    wxLogDebug("INFO: default_start=[%s]", dr.getDefaultStart().dateISO());
    wxLogDebug("INFO: default_end=[%s]", dr.getDefaultEnd().dateISO());

    struct {
        wxString range_label; wxString reporting_label;
        wxString range_start; wxString range_end; int reporting_count;
    } test[] = {
        { "A",            "",     "",           "2025-01-31",  1 }, // -,   Fri
        { "Y",            "M",    "2025-01-01", "2025-12-31", 12 }, // Wed, Wed
        { "Q",            "",     "2025-01-01", "2025-03-31",  1 }, // Wed, Mon
        { "M",            "W",    "2025-01-01", "2025-01-31",  5 }, // Wed, Fri
        { "W",            "",     "2025-01-27", "2025-02-02",  1 }, // Mon, Mon
        { "T",            "",     "2025-01-30", "2025-01-30",  1 }, // Thu, Thu
        { "S",            "",     "2024-08-30", "2024-08-30",  1 }, // Fri, Fri
        { "Y F",          "2 W",  "2024-04-06", "2025-04-05", 27 }, // Sat, Sat
        { "Q F",          "",     "2025-01-06", "2025-04-05",  1 }, // Mon, Sat
        { "M F",          "",     "2025-01-06", "2025-02-05",  1 }, // Mon, Wed
        { "W F",          "",     "2025-01-27", "2025-02-02",  1 }, // Mon, Sun
        { "T F",          "",     "2025-01-30", "2025-01-30",  1 }, // Thu, Thu
        { "S F",          "",     "2024-08-30", "2024-08-30",  1 }, // Fri, Fri
        { "1 M",          "",     "2025-02-01", "2025-02-28",  1 }, // Sat, Fri
        { "-1..0 Y",      "Q",    "2024-01-01", "2025-12-31",  8 }, // Mon, Wed
        { "-1..+1 Q",     "M",    "2024-10-01", "2025-06-30",  9 }, // Tue, Mon
        { "A .. W",       "W",    "",           "2025-02-02",  1 }, // -,   Mon
        { "-1 Y .. M",    "Q",    "2024-01-01", "2025-01-31",  5 }, // Mon, Fri
        { "-1 Y, M",      "W",    "2024-01-01", "2025-01-31", 57 }, // Mon, Fri
        { "-1 M, Q .. Y", "-2 M", "2024-12-01", "2025-01-31",  1 }, // Sun, Fri
        { "W, -1..+1 Q",  "-2 M", "2024-10-28", "2025-05-04",  4 }, // Mon, Sun
    };
    for (int i = 0; i < static_cast<int>(sizeof(test)/sizeof(test[0])); ++i) {
        StringBuilder sb;
        if (!test[i].range_label.empty()) {
            sb.append(test[i].range_label);
        }
        if (!test[i].reporting_label.empty()) {
            sb.sep(); sb.append("@"); sb.sep(); sb.append(test[i].reporting_label);
        }
        wxString label = sb.buffer;
        //wxLogDebug("test[%d] [%s]", i, label);

        // parse range and reporeting labels
        if (!dr.parseRange(test[i].range_label)) {
            ok = false;
            wxLogDebug("ERROR in test[%d] [%s]: Cannot parse range label", i, label);
            continue;
        }
        if (!dr.parseReporting(test[i].reporting_label)) {
            ok = false;
            wxLogDebug("ERROR in test[%d] [%s]: Cannot parse reporting label", i, label);
            continue;
        }
        wxString range_label = dr.rangeLabel();
        if (range_label != test[i].range_label)
            wxLogDebug("test[%d] [%s]: range_label=[%s]", i, label, range_label);
        wxString reporting_label = dr.reportingLabel();
        if (reporting_label != test[i].reporting_label)
            wxLogDebug("test[%d] [%s]: reporting_label=[%s]", i, label, reporting_label);

        // check range start
        wxString range_start = dr.rangeStart().dateISO();
        if (range_start != test[i].range_start) {
            ok = false;
            wxLogDebug("ERROR in test[%d] [%s]: range_start=[%s], expected [%s]",
                i, label, range_start, test[i].range_start
            );
        }

        // check range end
        wxString range_end = dr.rangeEnd().dateISO();
        if (range_end != test[i].range_end) {
            ok = false;
            wxLogDebug("ERROR in test[%d] [%s]: range_end=[%s], expected [%s]",
                i, label, range_end, test[i].range_end
            );
        }

        // check reporting iterator
        //wxLogDebug("test[%d] [%s]: reporting iterator", i, label);
        int reporting_count = 0;
        for (auto it = dr.cbegin(); it != dr.cend(); ++it) {
            ++reporting_count;
            //wxLogDebug("  count=%d, next=[%s]", reporting_count, (*it).dateISO());
        }
        if (reporting_count != test[i].reporting_count) {
            ok = false;
            wxLogDebug("ERROR in test[%d] [%s]: reporting_count=%d, expected %d",
                i, label, reporting_count, test[i].reporting_count
            );
        }
    }

    wxLogDebug("}}}");
    return ok;
}
#endif
