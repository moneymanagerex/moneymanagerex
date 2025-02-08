/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2013-2022 Nikolay Akimov
 Copyright (C) 2021-2024 Mark Whalley (mark@ipx.co.uk)
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

#include "primitive.h"

//----------------------------------------------------------------------------

int pow10(const int y)
{
    switch (y) {
    case 0: return 1;
    case 1: return 10;
    case 2: return 100;
    case 3: return 1000;
    case 4: return 10000;
    case 5: return 100000;
    case 6: return 1000000;
    case 7: return 10000000;
    case 8: return 100000000;
    case 9: return 1000000000;
    default: return 10;
    }
}

//----------------------------------------------------------------------------

int CaseInsensitiveCmp(const wxString &s1, const wxString &s2)
{
    return s1.CmpNoCase(s2);
}

int CaseInsensitiveLocaleCmp(const wxString &s1, const wxString &s2)
{
    return std::wcscoll(s1.Lower().wc_str(),s2.Lower().wc_str());
}

void StringBuilder::append(const wxString x) {
    if (x.empty())
        return;
    buffer.Append(x);
    flag = true;
}

void StringBuilder::sep(const wxString s) {
    if (!flag)
        return;
    buffer.Append(s);
    flag = false;
}

const wxString mmTrimAmount(
    const wxString& value, const wxString& decimal, const wxString& decimal_new
) {
    const wxString valid_chars = "-0123456789" + decimal;
    wxString str;
    for (const auto& c : value) {
        if (valid_chars.Contains(c))
            str += c;
    }
    if (!decimal_new.empty())
        str.Replace(decimal, decimal_new);
    return str;
}

bool isValidURI(const wxString& validate)
{
    wxString uri = validate.Lower().Trim();
    wxRegEx pattern(R"(^(?:http(s)?:\/\/)?[\w.-]+(?:\.[\w\.-]+)+[\w\-\._~:/?#[\]@!\$&'\(\)\*\+,;=.]+$)");
    return pattern.Matches(uri);
}

//----------------------------------------------------------------------------

const wxString MONTHS[12] =
{
    wxTRANSLATE("January"), wxTRANSLATE("February"), wxTRANSLATE("March"),
    wxTRANSLATE("April"),   wxTRANSLATE("May"),      wxTRANSLATE("June"),
    wxTRANSLATE("July"),    wxTRANSLATE("August"),   wxTRANSLATE("September"),
    wxTRANSLATE("October"), wxTRANSLATE("November"), wxTRANSLATE("December")
};

const wxString MONTHS_SHORT[12] =
{
    wxTRANSLATE("Jan"), wxTRANSLATE("Feb"), wxTRANSLATE("Mar"),
    wxTRANSLATE("Apr"), wxTRANSLATE("May"), wxTRANSLATE("Jun"),
    wxTRANSLATE("Jul"), wxTRANSLATE("Aug"), wxTRANSLATE("Sep"),
    wxTRANSLATE("Oct"), wxTRANSLATE("Nov"), wxTRANSLATE("Dec")
};

const wxString g_days_of_week[7] =
{
    wxTRANSLATE("Sunday"), wxTRANSLATE("Monday"), wxTRANSLATE("Tuesday"),
    wxTRANSLATE("Wednesday"), wxTRANSLATE("Thursday"), wxTRANSLATE("Friday"),
    wxTRANSLATE("Saturday")
};

const wxString g_short_days_of_week[7] =
{
    wxTRANSLATE("Sun"), wxTRANSLATE("Mon"), wxTRANSLATE("Tue"),
    wxTRANSLATE("Wed"), wxTRANSLATE("Thu"), wxTRANSLATE("Fri"),
    wxTRANSLATE("Sat")
};

bool mmParseISODate(const wxString& in, wxDateTime& out)
{
    if (in.IsEmpty() || !(out.ParseDateTime(in) || out.ParseDate(in))) {
        out = wxDateTime::Today();
        return false;
    }
    int year = out.GetYear();
    if (year < 50)
        out.Add(wxDateSpan::Years(2000));
    else if (year < 100)
        out.Add(wxDateSpan::Years(1900));
    return true;
}

//----------------------------------------------------------------------------

const wxColor* bestFontColour(const wxColour& background)
{
    // http://stackoverflow.com/a/3943023/112731
    int r = static_cast<int>(background.Red());
    int g = static_cast<int>(background.Green());
    int b = static_cast<int>(background.Blue());
    int k = (r * 299 + g * 587 + b * 114);
    wxLogDebug(
        "best FontColour: [%s] -> r=%d, g=%d, b=%d | k: %d",
        background.GetAsString(wxC2S_HTML_SYNTAX), r, g, b, k
    );
    return (k > 149000) ? wxBLACK : wxWHITE;
}

wxColour getUDColour(const int c)
{
    switch (c) {
    case 1: return  mmColors::userDefColor1;
    case 2: return  mmColors::userDefColor2;
    case 3: return  mmColors::userDefColor3;
    case 4: return  mmColors::userDefColor4;
    case 5: return  mmColors::userDefColor5;
    case 6: return  mmColors::userDefColor6;
    case 7: return  mmColors::userDefColor7;
    }
    return wxNullColour;
}

