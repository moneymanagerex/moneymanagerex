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
#include "option.h"

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
DateRange2::PERIOD_LABEL_ID_t DateRange2::PERIOD_LABEL_ID = make_period_label_id();

DateRange2::PERIOD_LABEL_ID_t DateRange2::make_period_label_id()
{
    PERIOD_LABEL_ID_t period_label_id;
    for (int i = 0; i < sizeof(PERIOD_INFO)/sizeof(PERIOD_INFO[0]); ++i) {
        char c = PERIOD_INFO[i].label[0];
        period_label_id[c] = PERIOD_INFO[i].id;
    }
    return period_label_id;
}

DateRange2::Spec::Spec(
    int so1_new, PERIOD_ID sp1_new,
    int eo1_new, PERIOD_ID ep1_new,
    int so2_new, PERIOD_ID sp2_new,
    int eo2_new, PERIOD_ID ep2_new,
    int f_new, wxString name_new
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
    name = name_new;
}

void DateRange2::Spec::scanWhiteSpace(StringIt &str_i, StringIt str_end)
{
    const wxString ws = " \t\r\n"; 
    while (str_i != str_end && ws.Find(*str_i) != wxNOT_FOUND)
        str_i++;
}

// the returned token is one of: o (offset), f (F), p (period), . (..), (,), ; (end), _ (error)
// token_o is set if token==o; token_p is set if token==p
char DateRange2::Spec::scanToken(StringIt &str_i, StringIt str_end, int &token_o, PERIOD_ID &token_p)
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

// return true if parse is successful
bool DateRange2::Spec::parseLabel(StringIt &str_i, StringIt str_end)
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
        char token;                         // one of [ofp.,;_]
        int token_o = 0;                    // offset (applicable if token == 'o')
        PERIOD_ID token_p = PERIOD_ID_none; // period (applicable if token == 'p')
        token = scanToken(str_i, str_end, token_o, token_p);
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
        wxLogDebug("ERROR: DateRange2::parseLabel(): state=%d", state);
        return false;
    }

    so1 = so_new[0]; sp1 = sp_new[0]; eo1 = eo_new[0]; ep1 = ep_new[0];
    so2 = so_new[1]; sp2 = sp_new[1]; eo2 = eo_new[1]; ep2 = ep_new[1];
    f = f_new;

    return true;
}

void DateRange2::Spec::parseName(StringIt &str_i, StringIt str_end)
{
    name = "";
    scanWhiteSpace(str_i, str_end);
    name.append(str_i, str_end);
}

bool DateRange2::Spec::parseSpec(const wxString &str, const wxString &name_new)
{
    Spec spec_new = Spec();
    StringIt str_i = str.begin();
    if (!spec_new.parseLabel(str_i, str.end()))
        return false;
    if (!name_new.empty())
        spec_new.name = name_new;
    else
        spec_new.parseName(str_i, str.end());
    *this = spec_new;
    return true;
}

const wxString DateRange2::Spec::offset_range_str(int so, int eo, bool show_zero)
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

const wxString DateRange2::Spec::getLabel() const
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

const wxString DateRange2::Spec::checking_name() const
{
    wxString str = getLabel();
    // TODO
    return str;
}

const wxString DateRange2::Spec::checking_description() const
{
    static StringBuilder sb;
    sb.reset();
    // TODO
    return sb.buffer;
}

DateRange2::DateRange2(wxDateTime date_s, wxDateTime date_t) :
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
    spec(Spec())
{
    setDateT(date_t);
    setDateS(date_s);
}

// return true if parse is successful
bool DateRange2::parseSpec(const wxString &str, const wxString &name)
{
    Spec spec_new = Spec();
    if (!spec_new.parseSpec(str, name))
        return false;
    spec = spec_new;
    return true;
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
        if (start_date.GetDay() < firstDay[spec.f])
            start_date -= wxDateSpan::Months(1);
        start_date.SetDay(firstDay[spec.f]);
        if (period == PERIOD_ID_Y) {
            if (start_date.GetMonth() < firstMonth[spec.f])
                start_date -= wxDateSpan::Years(1);
            start_date.SetMonth(firstMonth[spec.f]);
        }
        else if (period == PERIOD_ID_Q) {
            int m = (start_date.GetMonth() - firstMonth[spec.f] + 12) % 3;
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
        if (end_date.GetDay() >= firstDay[spec.f])
            end_date += wxDateSpan::Months(1);
        end_date.SetDay(firstDay[spec.f]);
        if (period == PERIOD_ID_Y) {
            if (end_date.GetMonth() > firstMonth[spec.f])
                end_date += wxDateSpan::Years(1);
            end_date.SetMonth(firstMonth[spec.f]);
        }
        else if (period == PERIOD_ID_Q) {
            int m = (firstMonth[spec.f] - end_date.GetMonth() + 12) % 3;
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
    if (spec.sp1 == PERIOD_ID_A || spec.sp2 == PERIOD_ID_A)
        return wxInvalidDateTime;
    wxDateTime start_date1 = (spec.sp1 == PERIOD_ID_S) ? date_s : date_t;
    start_date1 = addOffset(start_date1, spec.so1, spec.sp1);
    start_date1 = period_start(start_date1, spec.sp1);
    if (start_date1 == wxInvalidDateTime || spec.sp2 == PERIOD_ID_none || spec.ep2 == PERIOD_ID_none)
        return start_date1;
    wxDateTime start_date2 = (spec.sp2 == PERIOD_ID_S) ? date_s : date_t;
    start_date2 = addOffset(start_date2, spec.so2, spec.sp2);
    start_date2 = period_start(start_date2, spec.sp1 > spec.sp2 ? spec.sp1 : spec.sp2);
    if (start_date2 == wxInvalidDateTime)
        return wxInvalidDateTime;
    return start_date1 <= start_date2 ? start_date1 : start_date2;
}

wxDateTime DateRange2::checking_end() const
{
    if (spec.ep1 == PERIOD_ID_A || spec.ep2 == PERIOD_ID_A)
        return wxInvalidDateTime;
    wxDateTime end_date1 = (spec.ep1 == PERIOD_ID_S) ? date_s : date_t;
    end_date1 = addOffset(end_date1, spec.eo1, spec.ep1);
    end_date1 = period_end(end_date1, spec.ep1);
    if (end_date1 == wxInvalidDateTime || spec.sp2 == PERIOD_ID_none || spec.ep2 == PERIOD_ID_none)
        return end_date1;
    wxDateTime end_date2 = (spec.ep2 == PERIOD_ID_S) ? date_s : date_t;
    end_date2 = addOffset(end_date2, spec.eo2, spec.ep2);
    end_date2 = period_end(end_date2, spec.ep1 > spec.ep2 ? spec.ep1 : spec.ep2);
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

#ifndef NDEBUG
DateRange2::DateRange2(
    int firstDay_new_0, int firstDay_new_1,
    wxDateTime::Month firstMonth_new_0, wxDateTime::Month firstMonth_new_1,
    wxDateTime::WeekDay firstWeekday_new,
    wxDateTime date_t_new, wxDateTime date_s_new
) :
    firstDay{firstDay_new_0, firstDay_new_1},
    firstMonth{firstMonth_new_0, firstMonth_new_1},
    firstWeekday(firstWeekday_new),
    spec(Spec())
{
    setDateT(date_t_new);
    setDateS(date_s_new);
}

bool DateRange2::debug()
{
    bool ok = true;
    wxLogDebug("{{{ DateRange2::debug()");

    // check order in PERIOD_INFO
    for (int i = 0; i < sizeof(PERIOD_INFO)/sizeof(PERIOD_INFO[0]); i++) {
        wxASSERT_MSG(PERIOD_INFO[i].id == i, "Wrong order in DateRange2::PERIOD_INFO");
    }

    wxDateTime date_t, date_s;
    date_t.ParseISOCombined("2025-01-30T00:00:01"); // Thu
    date_s.ParseISOCombined("2024-08-30T00:00:01"); // Fri
    DateRange2 dr = DateRange2(
        1, 6,
        wxDateTime::Month::Jan, wxDateTime::Month::Apr,
        wxDateTime::WeekDay::Mon,
        date_t, date_s
    );
    //wxLogDebug("INFO: date_t=[%s]", dateTimeISO(dr.getT()));
    //wxLogDebug("INFO: date_s=[%s]", dateTimeISO(dr.getS()));

    struct { wxString spec; wxString sc; wxString ec; } checking[] = {
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
    for (int i = 0; i < sizeof(checking)/sizeof(checking[0]); ++i) {
        wxString spec = checking[i].spec;
        //wxLogDebug("checking[%d] [%s]", i, spec);
        if (!dr.parseSpec(spec)) {
            ok = false;
            wxLogDebug("ERROR in checking[%d]: Cannot parse [%s]", i, spec);
            continue;
        }
        wxString label = dr.getSpec().getLabel();
        if (label != spec)
            wxLogDebug("checking[%d] [%s]: label=[%s]", i, spec, label);
        wxString sc = dateISO(dr.checking_start());
        if (sc != checking[i].sc) {
            ok = false;
            wxLogDebug("ERROR in checking[%d] [%s]: sc=[%s], expected [%s]",
                i, spec, sc, checking[i].sc
            );
        }
        wxString ec = dateISO(dr.checking_end());
        if (ec != checking[i].ec) {
            ok = false;
            wxLogDebug("ERROR in checking[%d] [%s]: ec=[%s], expected [%s]",
                i, spec, ec, checking[i].ec
            );
        }
    }

    wxLogDebug("}}}");
    return ok;
}
#endif
