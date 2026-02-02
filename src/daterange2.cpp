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

const DatePeriod::MapIdLabel DatePeriod::mapIdLabel[] =
{
    { _A, "A" },
    { _Y, "Y" },
    { _Q, "Q" },
    { _M, "M" },
    { _W, "W" },
    { _T, "T" },
    { _S, "S" },
};
const DatePeriod::MapLabelId DatePeriod::mapLabelId = makeLabelId();

DatePeriod::MapLabelId DatePeriod::makeLabelId()
{
    MapLabelId mapLabelId;
    for (int i = 0; i < static_cast<int>(sizeof(mapIdLabel)/sizeof(mapIdLabel[0])); ++i) {
        char c = mapIdLabel[i].label[0];
        mapLabelId[c] = mapIdLabel[i].id;
    }
    return mapLabelId;
}

DatePeriod::DatePeriod(Id id_new) :
    id(id_new)
{
}

DatePeriod::DatePeriod(char label)
{
    auto it = mapLabelId.find(label);
    id = (it == mapLabelId.end()) ? _A : it->second;
}

wxDateSpan DatePeriod::span(int offset, DatePeriod period)
{
    if (period == _Y)
        return wxDateSpan::Years(offset);
    else if (period == _Q)
        return wxDateSpan::Months(3*offset);
    else if (period == _M)
        return wxDateSpan::Months(offset);
    else if (period == _W)
        return wxDateSpan::Weeks(offset);
    else if (period == _T || period == _S)
        return wxDateSpan::Days(offset);
    else
        return wxDateSpan();
}

DateRange2::Range::Range(
    int so1_new, DatePeriod  sp1_new,
    int eo1_new, DatePeriod  ep1_new,
    int so2_new, DatePeriodN sp2_new,
    int eo2_new, DatePeriodN ep2_new,
    int f_new, wxString name_new
) :
    sp1(sp1_new),
    ep1(ep1_new)
{
    // so1/eo1 is not applicable if sp1/ep1 resp., is DatePeriod::_A
    so1 = (sp1 == DatePeriod::_A) ? 0 : so1_new;
    eo1 = (ep1 == DatePeriod::_A) ? 0 : eo1_new;
    // either both sp2 and ep2 are null, or none of them
    if (!sp2_new.has_value() || !ep2_new.has_value()) {
        // so2/eo2 are not applicable if sp2/ep2 are null
        sp2 = DatePeriodN(); so2 = 0;
        ep2 = DatePeriodN(); eo2 = 0;
    }
    else {
        // so2/eo2 is not applicable if sp2/ep2 resp., is DatePeriod::A
        sp2 = sp2_new; so2 = (sp2 == DatePeriod::_A) ? 0 : so2_new;
        ep2 = ep2_new; eo2 = (ep2 == DatePeriod::_A) ? 0 : eo2_new;
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
    int &token_o, DatePeriod &token_p
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

    if (auto it = DatePeriod::mapLabelId.find(c); it != DatePeriod::mapLabelId.end()) {
        ++buffer_i;
        token_p = it->second;
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

    int so_new[2] = { 0, 0 }; DatePeriodN sp_new[2] = { DatePeriodN(), DatePeriodN() };
    int eo_new[2] = { 0, 0 }; DatePeriodN ep_new[2] = { DatePeriodN(), DatePeriodN() };
    int f_new = 0;
    int i = 0;     // index into {s,e}{o,p}[] (0: first subrange, 1: second subrange)
    int state = 0; // parse state: 0 (so) 1 (sp) 2 (..) 3 (eo) 4 (ep) 5 (f) 6 (;) 7

    while (1) {
        char token;                           // one of [op.,@f;_]
        int token_o = 0;                      // offset (if token == 'o')
        DatePeriod token_p = DatePeriod::_A;  // period (if token == 'p')
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
            if (!sp_new[i].has_value())
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

    if (state != 7 || !sp_new[0].has_value() || !ep_new[0].has_value()) {
        wxLogDebug("ERROR: DateRange2::Range::parseLabel(): state=%d", state);
        return false;
    }

    so1 = so_new[0]; sp1 = sp_new[0].value(); so2 = so_new[1]; sp2 = sp_new[1];
    eo1 = eo_new[0]; ep1 = ep_new[0].value(); eo2 = eo_new[1]; ep2 = ep_new[1];
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
    if (sp1 == ep1) {
        sb.append(offsetRangeStr(so1, eo1)); sb.sep(); sb.append(sp1.label());
    }
    else {
        sb.append(offsetStr(so1)); sb.sep(); sb.append(sp1.label());
        sb.sep(); sb.append(".."); sb.sep();
        sb.append(offsetStr(eo1)); sb.sep(); sb.append(ep1.label());
    }

    // second range
    if (sp2.has_value() && ep2.has_value()) {
        sb.append(","); sb.sep();
        if (sp2 == ep2) {
            sb.append(offsetRangeStr(so2, eo2)); sb.sep(); sb.append(sp2.value().label());
        }
        else {
            sb.append(offsetStr(so2)); sb.sep(); sb.append(sp2.value().label());
            sb.sep(); sb.append(".."); sb.sep();
            sb.append(offsetStr(eo2)); sb.sep(); sb.append(ep2.value().label());
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
    int m_new, DatePeriod p_new
) :
    // p cannot be DatePeriod::_S (account statement date)
    // m cannot be 0; it is normalized to 1 if p is DatePeriod::_A
    p((p_new == DatePeriod::_S) ? DatePeriod::_T : p_new),
    m((m_new == 0 || p == DatePeriod::_A) ? 1 : m_new)
{
}

// return true if parse is successful
bool DateRange2::Reporting::parseLabel(StringIt &buffer_i, StringIt buffer_end)
{
    // reporting = m? p

    int m_new = 1; DatePeriod p_new = DatePeriod::_A;
    int state = 0; // parse state: 0 (m) 1 (p) 2 (;) 3

    while (1) {
        char token;                           // one of [op;_]
        int token_o = 0;                      // offset (if token == 'o')
        DatePeriod token_p = DatePeriod::_A;  // period (if token == 'p')
        token = DateRange2::Range::scanToken(buffer_i, buffer_end, token_o, token_p);
        //wxLogDebug("DEBUG: state=%d, token=%c", state, token);
        if (state == 0 && token == 'o') {
            // m cannot be 0
            m_new = (token_o == 0) ? 1 : token_o;
            state = 1;
            continue;
        }
        if ((state == 0 || state == 1) && token == 'p') {
            // m is normalized to 1 if p is DatePeriod::_A
            if (token_p == DatePeriod::_A)
                m_new = 1;
            // p cannot be DatePeriod::_S
            p_new = (token_p == DatePeriod::_S) ? DatePeriod::_T : token_p;
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
    StringBuilder sb;
    sb.append(multiplierStr(m)); sb.sep(); sb.append(p.label());
    return sb.buffer;
}

DateRange2::DateRange2(
    DateDayN sDateN_new,
    DateDay  tDate_new,
    DateDayN defStartDateN_new,
    DateDayN defEndDateN_new
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
    sDateN(sDateN_new),
    tDate(tDate_new),
    defStartDateN(defStartDateN_new),
    defEndDateN(defEndDateN_new),
    range(Range()),
    reporting(Reporting())
{
}

#ifndef NDEBUG
DateRange2::DateRange2(
    int firstDay_new_0, int firstDay_new_1,
    wxDateTime::Month firstMonth_new_0, wxDateTime::Month firstMonth_new_1,
    wxDateTime::WeekDay firstWeekday_new,
    DateDayN sDateN_new,
    DateDay  tDate_new,
    DateDayN defStartDateN_new,
    DateDayN defEndDateN_new
) :
    firstDay{firstDay_new_0, firstDay_new_1},
    firstMonth{firstMonth_new_0, firstMonth_new_1},
    firstWeekday(firstWeekday_new),
    sDateN(sDateN_new),
    tDate(tDate_new),
    defStartDateN(defStartDateN_new),
    defEndDateN(defEndDateN_new),
    range(Range()),
    reporting(Reporting())
{
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

DateDayN DateRange2::periodStart(DateDay date, DatePeriod period) const
{
    if (period == DatePeriod::_A)
        return DateDayN();
    wxDateTime s = date.getDateTime();
    if (period == DatePeriod::_Y || period == DatePeriod::_Q || period == DatePeriod::_M) {
        if (s.GetDay() < firstDay[range.f])
            s -= wxDateSpan::Months(1);
        s.SetDay(firstDay[range.f]);
        if (period == DatePeriod::_Y) {
            if (s.GetMonth() < firstMonth[range.f])
                s -= wxDateSpan::Years(1);
            s.SetMonth(firstMonth[range.f]);
        }
        else if (period == DatePeriod::_Q) {
            int m = (s.GetMonth() - firstMonth[range.f] + 12) % 3;
            if (m > 0)
                s -= wxDateSpan::Months(m);
        }
    }
    else if (period == DatePeriod::_W) {
        int d = (s.GetWeekDay() - firstWeekday + 7) % 7;
        if (d > 0)
            s -= wxDateSpan::Days(d);
    }
    return DateDay(s);
}

DateDayN DateRange2::periodEnd(DateDay date, DatePeriod period) const
{
    if (period == DatePeriod::_A)
        return DateDayN();
    wxDateTime e = date.getDateTime();
    if (period == DatePeriod::_Y || period == DatePeriod::_Q || period == DatePeriod::_M) {
        if (e.GetDay() >= firstDay[range.f])
            e += wxDateSpan::Months(1);
        e.SetDay(firstDay[range.f]);
        if (period == DatePeriod::_Y) {
            if (e.GetMonth() > firstMonth[range.f])
                e += wxDateSpan::Years(1);
            e.SetMonth(firstMonth[range.f]);
        }
        else if (period == DatePeriod::_Q) {
            int m = (firstMonth[range.f] - e.GetMonth() + 12) % 3;
            if (m > 0)
                e += wxDateSpan::Months(m);
        }
        e -= wxDateSpan::Days(1);
    }
    else if (period == DatePeriod::_W) {
        int d = (firstWeekday - e.GetWeekDay() + 6) % 7;
        if (d > 0)
            e += wxDateSpan::Days(d);
    }
    return DateDay(e);
}

DateDayN DateRange2::rangeStart() const
{
    if (range.sp1 == DatePeriod::_A || range.sp2 == DatePeriod::_A)
        return defStartDateN;
    DateDayN s1N = (range.sp1 == DatePeriod::_S) ? sDateN : tDate;
    if (!s1N.has_value())
        return DateDayN();
    DateDay s1 = s1N.value();
    if (range.so1 != 0)
        s1.addSpan(DatePeriod::span(range.so1, range.sp1));
    s1 = periodStart(s1, range.sp1).value();
    if (!range.sp2.has_value())
        return s1;
    DateDayN s2N = (range.sp2 == DatePeriod::_S) ? sDateN : tDate;
    if (!s2N.has_value())
        return DateDayN();
    DateDay s2 = s2N.value();
    if (range.so2 != 0)
        s2.addSpan(DatePeriod::span(range.so2, range.sp2.value()));
    DatePeriod p = range.sp1.toInt() > range.sp2.value().toInt() ? range.sp1
        : range.sp2.value();
    s2 = periodStart(s2, p).value();
    return s1 <= s2 ? s1 : s2;
}

DateDayN DateRange2::rangeEnd() const
{
    if (range.ep1 == DatePeriod::_A || range.ep2 == DatePeriod::_A)
        return defEndDateN;
    DateDayN e1N = (range.ep1 == DatePeriod::_S) ? sDateN : tDate;
    if (!e1N.has_value())
        return DateDayN();
    DateDay e1 = e1N.value();
    if (range.eo1 != 0)
        e1.addSpan(DatePeriod::span(range.eo1, range.ep1));
    e1 = periodEnd(e1, range.ep1).value();
    if (!range.ep2.has_value())
        return e1;
    DateDayN e2N = (range.ep2 == DatePeriod::_S) ? sDateN : tDate;
    if (!e2N.has_value())
        return DateDayN();
    DateDay e2 = e2N.value();
    if (range.eo2 != 0)
        e2.addSpan(DatePeriod::span(range.eo2, range.ep2.value()));
    DatePeriod p = range.ep1.toInt() > range.ep2.value().toInt() ? range.ep1
        : range.ep2.value();
    e2 = periodEnd(e2, p).value();
    return e2 <= e1 ? e1 : e2;
}

DateDayN DateRange2::reportingNext() const
{
    DateDayN sN = rangeStart();
    DateDayN eN = rangeEnd();
    if (!sN.has_value() || !eN.has_value())
        return eN;

    DateDay s = sN.value();
    DateDay e = eN.value();
    if (s > e)
        return DateDayN();

    if (reporting.p == DatePeriod::_A)
        return e;

    if (reporting.m > 0) {
        // return the end of the multi-period aligned at s
        // (i.e., its first period contains s)
        DateDay next = periodEnd(s, reporting.p).value();
        if (reporting.m > 1) {
            next.addSpan(DatePeriod::span(reporting.m - 1, reporting.p));
            next = periodEnd(next, reporting.p).value();
        }
        return next <= e ? next : e;
    }
    else { // if (reporting.m < 0)
        // return the end of the multi-period aligned at e
        // (i.e., its last period contains e)
        DateDay next = periodEnd(e, reporting.p).value();
        DateDay next1 = next;
        next1.addSpan(DatePeriod::span(reporting.m, reporting.p));
        while (s <= next1) {
            next = next1;
            next1.addSpan(DatePeriod::span(reporting.m, reporting.p));
            next1 = periodEnd(next1, reporting.p).value();
        }
        return next <= e ? next : e;
    }
}

const wxString DateRange2::checkingTooltip() const
{
    static StringBuilder sb;
    sb.reset();

    DateDayN s = rangeStart();
    DateDayN e = rangeEnd();
    if (s.has_value())
        sb.append(s.value().isoDate());
    sb.sep(); sb.append(".."); sb.sep();
    if (e.has_value())
        sb.append(e.value().isoDate());
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
    nextDateN(a_new->reportingNext()),
    lastDateN(a_new->rangeEnd())
{
}

void DateRange2::ReportingIterator::increment()
{
    // The iterator reaches the end when (nextDateN == lastDateN)
    // (this includes the special case of open end, in which lastDateN is null).
    // count is initialized to 0 and set to -1 at the end.
    // Notice that the iterator returns at least one DateDayN before it reaches the end.

    if (count == -1)
        return;
    if (!nextDateN.has_value() || nextDateN == lastDateN) {
        count = -1;
        return;
    }
    if (!nextDateN.has_value() || !lastDateN.has_value()) {
        // this should not happen
        wxLogDebug("ERROR in DateRange2::ReportingIterator: only one of nextDateN, lastDateN is null");
        count = -1;
        return;
    }

    int rm = a->reporting.m;
    if (rm < 0) rm = -rm;
    DatePeriod rp = a->reporting.p;
    // assertion: rp is not DatePeriod::_A
    DateDay next1 = nextDateN.value();
    next1.addSpan(DatePeriod::span(rm, rp));
    next1 = a->periodEnd(next1, rp).value();
    if (lastDateN.value() < next1)
        next1 = lastDateN.value();

    if (next1 <= nextDateN.value()) {
        // this should not happen
        // nextDateN must strictly increase in each step
        wxLogDebug("ERROR in DateRange2::ReportingIterator: cannot increment");
        count = -1;
        return;
    }

    ++count;
    nextDateN = next1;
}

#ifndef NDEBUG
bool DateRange2::debug()
{
    bool ok = true;
    wxLogDebug("{{{ DateRange2::debug()");

    // check order in DatePeriod::mapIdLabel
    int n = static_cast<int>(sizeof(DatePeriod::mapIdLabel)/sizeof(DatePeriod::mapIdLabel[0]));
    for (int i = 0; i < n; i++) {
        wxASSERT_MSG(
            static_cast<int>(DatePeriod::mapIdLabel[i].id) == i,
            "Wrong order in DatePeriod::mapIdLabel"
        );
    }

    // create a DateRange2 object; defStartDateN is null
    wxDateTime sDateTime, tDateTime, defEndDateTime;
    sDateTime.ParseISOCombined("2024-08-30T00:00:01"); // Fri
    tDateTime.ParseISOCombined("2025-01-30T00:00:01"); // Thu
    defEndDateTime.ParseISODate("2025-01-31"); // Fri
    DateRange2 dr = DateRange2(
        1, 6,
        wxDateTime::Month::Jan, wxDateTime::Month::Apr,
        wxDateTime::WeekDay::Mon,
        DateDay(sDateTime), DateDay(tDateTime),
        DateDayN(), DateDay(defEndDateTime)
    );
    wxLogDebug("INFO: sDateN.dateTime=[%s]", dateTimeISO(dr.getSDateN().getDateTimeN()));
    wxLogDebug("INFO: tDate.dateTime=[%s]", dateTimeISO(dr.getTDate().getDateTime()));
    wxLogDebug("INFO: defStartDateN=[%s]", dr.getDefStartDateN().isoDateN());
    wxLogDebug("INFO: defEndDateN=[%s]", dr.getDefEndDateN().isoDateN());

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
        wxString range_start = dr.rangeStart().isoDateN();
        if (range_start != test[i].range_start) {
            ok = false;
            wxLogDebug("ERROR in test[%d] [%s]: range_start=[%s], expected [%s]",
                i, label, range_start, test[i].range_start
            );
        }

        // check range end
        wxString range_end = dr.rangeEnd().isoDateN();
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
            //wxLogDebug("  count=%d, next=[%s]", reporting_count, (*it).isoDateN());
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
