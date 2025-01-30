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

#include "util.h"
#include "option.h"
#include "daterange2.h"

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
std::unordered_map<char, DateRange2::PERIOD_ID> DateRange2::PERIOD_LABEL_ID = {};

DateRange2::DateRange2(wxDateTime s, wxDateTime t) :
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
    )
{
    setT(t);
    setS(s);
}

void DateRange2::setRange(
    int so1_new, PERIOD_ID sp1_new,
    int eo1_new, PERIOD_ID ep1_new,
    int so2_new, PERIOD_ID sp2_new,
    int eo2_new, PERIOD_ID ep2_new,
    int f_new
) {
    sp1 = (sp1_new == PERIOD_ID_none) ? PERIOD_ID_A : sp1_new;
    ep1 = (ep1_new == PERIOD_ID_none) ? PERIOD_ID_A : ep1_new;
    so1 = (sp1 == PERIOD_ID_A) ? 0 : so1_new;
    eo1 = (ep1 == PERIOD_ID_A) ? 0 : eo1_new;
    if (sp2_new == PERIOD_ID_none || ep2_new == PERIOD_ID_none) {
        sp2 = PERIOD_ID_none; so2 = 0;
        ep2 = PERIOD_ID_none; eo2 = 0;
    }
    else {
        sp2 = sp2_new; so2 = (sp2 == PERIOD_ID_A) ? 0 : so2_new;
        ep2 = ep2_new; eo2 = (ep2 == PERIOD_ID_A) ? 0 : eo2_new;
    }
    f = (f_new == 1) ? 1 : 0;
}

bool DateRange2::setRange(const wxString range)
{
    StringIt range_i = range.begin();
    return scanLabel(range_i, range.end());
}

wxDateTime DateRange2::addOffset(wxDateTime date, int offset, PERIOD_ID period)
{
    if (offset == 0)
        return date;
    if (period == PERIOD_ID_Y)
        return date + wxDateSpan::Years(offset);
    else if (period == PERIOD_ID_Q)
        return date + wxDateSpan::Months(3*offset);
    else if (period == PERIOD_ID_M)
        return date + wxDateSpan::Months(offset);
    else if (period == PERIOD_ID_W)
        return date + wxDateSpan::Weeks(offset);
    else if (period == PERIOD_ID_T || period == PERIOD_ID_S)
        return date + wxDateSpan::Days(offset);
    else
        return wxInvalidDateTime;
}

wxDateTime DateRange2::period_start(wxDateTime date, PERIOD_ID period) const
{
    if (date == wxInvalidDateTime || period < PERIOD_ID_Y || period > PERIOD_ID_S)
        return wxInvalidDateTime;
    wxDateTime start_date = date;
    if (period == PERIOD_ID_Y || period == PERIOD_ID_Q || period == PERIOD_ID_M) {
        if (start_date.GetDay() < firstDay[f])
            start_date -= wxDateSpan::Months(1);
        start_date.SetDay(firstDay[f]);
        if (period == PERIOD_ID_Y) {
            if (start_date.GetMonth() < firstMonth[f])
                start_date -= wxDateSpan::Years(1);
            start_date.SetMonth(firstMonth[f]);
        }
        else if (period == PERIOD_ID_Q) {
            int m = (start_date.GetMonth() - firstMonth[f] + 12) % 3;
            if (m > 0)
                start_date -= wxDateSpan::Months(m);
        }
    }
    else if (period == PERIOD_ID_W) {
        int d = (start_date.GetWeekDay() - firstWeekday + 7) % 7;
        if (d > 0)
            start_date -= wxDateSpan::Days(d);
    }
    return start_date;
}

wxDateTime DateRange2::period_end(wxDateTime date, PERIOD_ID period) const
{
    if (date == wxInvalidDateTime || period < PERIOD_ID_Y || period > PERIOD_ID_S)
        return wxInvalidDateTime;
    wxDateTime end_date = date;
    if (period == PERIOD_ID_Y || period == PERIOD_ID_Q || period == PERIOD_ID_M) {
        if (end_date.GetDay() >= firstDay[f])
            end_date += wxDateSpan::Months(1);
        end_date.SetDay(firstDay[f]);
        if (period == PERIOD_ID_Y) {
            if (end_date.GetMonth() > firstMonth[f])
                end_date += wxDateSpan::Years(1);
            end_date.SetMonth(firstMonth[f]);
        }
        else if (period == PERIOD_ID_Q) {
            int m = (firstMonth[f] - end_date.GetMonth() + 12) % 3;
            if (m > 0)
                end_date += wxDateSpan::Months(m);
        }
        end_date -= wxDateSpan::Days(1);
    }
    else if (period == PERIOD_ID_W) {
        int d = (firstWeekday - end_date.GetWeekDay() + 6) % 7;
        if (d > 0)
            end_date += wxDateSpan::Days(d);
    }
    return end_date;
}

wxDateTime DateRange2::checking_start() const
{
    if (sp1 == PERIOD_ID_A || sp2 == PERIOD_ID_A)
        return wxInvalidDateTime;
    wxDateTime start_date1 = (sp1 == PERIOD_ID_S) ? s : t;
    start_date1 = addOffset(start_date1, so1, sp1);
    start_date1 = period_start(start_date1, sp1);
    if (start_date1 == wxInvalidDateTime || sp2 == PERIOD_ID_none || ep2 == PERIOD_ID_none)
        return start_date1;
    wxDateTime start_date2 = (sp2 == PERIOD_ID_S) ? s : t;
    start_date2 = addOffset(start_date2, so2, sp2);
    start_date2 = period_start(start_date2, sp1 > sp2 ? sp1 : sp2);
    if (start_date2 == wxInvalidDateTime)
        return wxInvalidDateTime;
    return start_date1 <= start_date2 ? start_date1 : start_date2;
}

wxDateTime DateRange2::checking_end() const
{
    if (ep1 == PERIOD_ID_A || ep2 == PERIOD_ID_A)
        return wxInvalidDateTime;
    wxDateTime end_date1 = (ep1 == PERIOD_ID_S) ? s : t;
    end_date1 = addOffset(end_date1, eo1, ep1);
    end_date1 = period_end(end_date1, ep1);
    if (end_date1 == wxInvalidDateTime || sp2 == PERIOD_ID_none || ep2 == PERIOD_ID_none)
        return end_date1;
    wxDateTime end_date2 = (ep2 == PERIOD_ID_S) ? s : t;
    end_date2 = addOffset(end_date2, eo2, ep2);
    end_date2 = period_end(end_date2, ep1 > ep2 ? ep1 : ep2);
    if (end_date2 == wxInvalidDateTime)
        return wxInvalidDateTime;
    return end_date1 >= end_date2 ? end_date1 : end_date2;
}

wxDateTime DateRange2::reporting_start() const
{
    // TODO
    return wxInvalidDateTime;
}

wxDateTime DateRange2::reporting_end() const
{
    // TODO
    return wxInvalidDateTime;
}

void DateRange2::scanWhiteSpace(StringIt &str_i, StringIt str_end)
{
    const wxString ws = " \t\r\n"; 
    while (str_i != str_end && ws.Find(*str_i) != wxNOT_FOUND)
        str_i++;
}

char DateRange2::scanToken(StringIt &str_i, StringIt str_end, int &token_o, PERIOD_ID &token_p)
{
    scanWhiteSpace(str_i, str_end);
    if (str_i == str_end)
        return ';';
    char c;
    if (!(*str_i).IsAscii() || !(*str_i).GetAsChar(&c))
        return '_';

    char c1 = 0;
    if (c == '+' || c == '-') {
        c1 = c;
        str_i++;
        if (str_i == str_end || !(*str_i).IsAscii() || !(*str_i).GetAsChar(&c))
            return '_';
    }
    if (isdigit(c)) {
        StringIt str_start = str_i;
        do { str_i++; } while (
            str_i != str_end && (*str_i).IsAscii() && (*str_i).GetAsChar(&c) && isdigit(c)
        );
        wxString str_o = "";
        str_o.append(str_start, str_i);
        token_o = wxAtoi(str_o);
        if (c1 == '-') token_o = -token_o;
        return 'o';
    }
    if (c1)
        return '_';

    if (c == 'F') {
        str_i++;
        return 'f';
    }

    auto label_it = PERIOD_LABEL_ID.find(c);
    if (label_it != PERIOD_LABEL_ID.end()) {
        str_i++;
        token_p = label_it->second;
        return 'p';
    }

    if (c == '.') {
        c1 = c;
        str_i++;
        if (str_i == str_end || !(*str_i).IsAscii() || !(*str_i).GetAsChar(&c))
            return '_';
    }
    if (c == '.') {
        str_i++;
        return '.';
    }
    if (c1)
        return '_';

    if (c == ',' || c == ';') {
        str_i++;
        return c;
    }

    return '_';
}

bool DateRange2::scanLabel(StringIt &str_i, StringIt str_end)
{
    // range = subrange ("," subrange)? ("F")? (";" name)?
    // subrange = so? sp ".." eo? ep
    // subrange = so ".." eo p
    // subrange = o? p

    int so_new[2] = { 0, 0 }; PERIOD_ID sp_new[2] = { PERIOD_ID_none, PERIOD_ID_none };
    int eo_new[2] = { 0, 0 }; PERIOD_ID ep_new[2] = { PERIOD_ID_none, PERIOD_ID_none };
    int f_new = 0;
    int i = 0;     // index into {s,e}{o,p}[] (0: first subrange, 1: second subrange)
    int state = 0; // scan state: 0 (so) 1 (sp) 2 (..) 3 (eo) 4 (ep) 5 (f) 6 (;) 7
    char token = 0;                     // token (one of [ofp.,;_])
    int token_o = 0;                    // offset (applicable if token == 'o')
    PERIOD_ID token_p = PERIOD_ID_none; // period (applicable if token == 'p')

    while (1) {
        token = scanToken(str_i, str_end, token_o, token_p);
        //wxLogDebug("DEBUG: state=%d, token=%c", state, token ? token : '0');
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
        wxLogDebug("ERROR: DateRange2::scanLabel(): state=%d", state);
        return false;
    }

    setRange(
        so_new[0], sp_new[0], eo_new[0], ep_new[0],
        so_new[1], sp_new[1], eo_new[1], ep_new[1],
        f_new
    );

    return true;
}

const wxString DateRange2::scanName(StringIt &str_i, StringIt str_end)
{
    wxString str = "";
    scanWhiteSpace(str_i, str_end);
    str.append(str_i, str_end);
    return str;
}

const wxString DateRange2::offset_range_str(int so, int eo, bool show_zero)
{
    if (so == eo) {
        return offset_str(so, show_zero);
    }
    else {
        StringBuilder s;
        s.append(offset_str(so, true));
        s.append("..");
        s.append(offset_str(eo, true));
        return s.buffer;
    }
}

const wxString DateRange2::getLabel() const
{
    StringBuilder sb;

    // first range
    wxString sp1_label = PERIOD_INFO[sp1].label;
    if (sp1 == ep1) {
        sb.append(offset_range_str(so1, eo1)); sb.sep(); sb.append(sp1_label);
    }
    else {
        wxString ep1_label = PERIOD_INFO[ep1].label;
        sb.append(offset_str(so1)); sb.sep(); sb.append(sp1_label);
        sb.sep(); sb.append(".."); sb.sep();
        sb.append(offset_str(eo1)); sb.sep(); sb.append(ep1_label);
    }

    // second range
    if (sp2 != PERIOD_ID_none && ep2 != PERIOD_ID_none) {
        sb.append(","); sb.sep();
        wxString sp2_label = PERIOD_INFO[sp2].label;
        if (sp2 == ep2) {
            sb.append(offset_range_str(so2, eo2)); sb.sep(); sb.append(sp2_label);
        }
        else {
            wxString ep2_label = PERIOD_INFO[ep2].label;
            sb.append(offset_str(so2)); sb.sep(); sb.append(sp2_label);
            sb.sep(); sb.append(".."); sb.sep();
            sb.append(offset_str(eo2)); sb.sep(); sb.append(ep2_label);
        }
    }

    // financial
    if (f == 1) {
        sb.sep(); sb.append("F");
    }

    return sb.buffer;
}

const wxString DateRange2::getName() const
{
    wxString str = getLabel();
    // TODO
    return str;
}

const wxString DateRange2::getDescription() const
{
    static StringBuilder sb;
    sb.reset();
    // TODO
    return sb.buffer;
}

#ifndef NDEBUG
DateRange2::DateRange2(
    int firstDay_0, int firstDay_1,
    wxDateTime::Month firstMonth_0, wxDateTime::Month firstMonth_1,
    wxDateTime::WeekDay firstWeekday_new,
    wxDateTime t_new, wxDateTime s_new
) :
    firstDay{firstDay_0, firstDay_1},
    firstMonth{firstMonth_0, firstMonth_1},
    firstWeekday(firstWeekday_new)
{
    setT(t_new);
    setS(s_new);
}
#endif

bool DateRange2::init()
{
    bool ok = true;

    // check order in PERIOD_INFO
    for (int i = 0; i < sizeof(PERIOD_INFO)/sizeof(PERIOD_INFO[0]); i++) {
        wxASSERT_MSG(PERIOD_INFO[i].id == i, "Wrong order in DateRange2::PERIOD_INFO");
    }

    // initialize PERIOD_LABEL_ID
    PERIOD_LABEL_ID = {};
    for (int i = 0; i < sizeof(PERIOD_INFO)/sizeof(PERIOD_INFO[0]); ++i) {
        char c = PERIOD_INFO[i].label[0];
        PERIOD_LABEL_ID[c] = PERIOD_INFO[i].id;
    }

    return ok;
}

#ifndef NDEBUG
bool DateRange2::debug()
{
    bool ok = true;
    wxLogDebug("{{{ DateRange2::debug()");

    wxDateTime t, s;
    t.ParseISOCombined("2025-01-30T00:00:01"); // Thu
    s.ParseISOCombined("2024-08-30T00:00:01"); // Fri
    DateRange2 dr = DateRange2(
        1, 6,
        wxDateTime::Month::Jan, wxDateTime::Month::Apr,
        wxDateTime::WeekDay::Mon,
        t, s
    );
    //wxLogDebug("INFO: t=[%s]", dateTimeISO(dr.getT()));
    //wxLogDebug("INFO: s=[%s]", dateTimeISO(dr.getS()));

    struct { wxString range; wxString sc; wxString ec; } test1[] = {
        { "A",            "",           "" },
        { "Y",            "2025-01-01", "2025-12-31" },
        { "Q",            "2025-01-01", "2025-03-31" },
        { "M",            "2025-01-01", "2025-01-31" },
        { "W",            "2025-01-27", "2025-02-02" },
        { "T",            "2025-01-30", "2025-01-30" },
        { "S",            "2024-08-30", "2024-08-30" },
        { "Y F",          "2024-04-06", "2025-04-05" },
        { "Q F",          "2025-01-06", "2025-04-05" },
        { "M F",          "2025-01-06", "2025-02-05" },
        { "W F",          "2025-01-27", "2025-02-02" },
        { "T F",          "2025-01-30", "2025-01-30" },
        { "S F",          "2024-08-30", "2024-08-30" },
        { "1 M",          "2025-02-01", "2025-02-28" },
        { "-1..0 Y",      "2024-01-01", "2025-12-31" },
        { "-1..+1 Q",     "2024-10-01", "2025-06-30" },
        { "A .. W",       "",           "2025-02-02" },
        { "-1 Y .. M",    "2024-01-01", "2025-01-31" },
        { "-1 Y, M",      "2024-01-01", "2025-01-31" },
        { "-1 M, Q .. Y", "2024-12-01", "2025-01-31" },
        { "W, -1..+1 Q",  "2024-10-28", "2025-05-04" },
    };
    for (int i = 0; i < sizeof(test1)/sizeof(test1[0]); ++i) {
        wxString range = test1[i].range;
        //wxLogDebug("DEBUG in test1[%d] [%s]", i, range);
        if (!dr.setRange(test1[i].range)) {
            ok = false;
            wxLogDebug("ERROR in test1[%d]: Cannot scan [%s]", i, range);
            continue;
        }
        wxString label = dr.getLabel();
        if (label != range)
            wxLogDebug("INFO in test1[%d]: [%s] -> [%s]", i, range, label);
        wxString sc = dateISO(dr.checking_start());
        if (sc != test1[i].sc) {
            ok = false;
            wxLogDebug("ERROR in test1[%d] [%s]: sc=[%s], expected [%s]", i, range, sc, test1[i].sc);
        }
        wxString ec = dateISO(dr.checking_end());
        if (ec != test1[i].ec) {
            ok = false;
            wxLogDebug("ERROR in test1[%d] [%s]: ec=[%s], expected [%s]", i, range, ec, test1[i].ec);
        }
    }

    wxLogDebug("}}}");
    return ok;
}
#endif
