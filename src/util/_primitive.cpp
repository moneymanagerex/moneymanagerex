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

#include <unordered_map>
#include "_primitive.h"

// -- String

int CaseInsensitiveCmp(const wxString &s1, const wxString &s2)
{
    return s1.CmpNoCase(s2);
}

int CaseInsensitiveLocaleCmp(const wxString &s1, const wxString &s2)
{
    return std::wcscoll(s1.Lower().wc_str(),s2.Lower().wc_str());
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

// -- URI

bool isValidURI(const wxString& validate)
{
    wxString uri = validate.Lower().Trim();
    wxRegEx pattern(R"(^(?:http(s)?:\/\/)?[\w.-]+(?:\.[\w\.-]+)+[\w\-\._~:/?#[\]@!\$&'\(\)\*\+,;=.]+$)");
    return pattern.Matches(uri);
}

// -- DateTime

const wxString MONTHS[12] =
{
    _n("January"), _n("February"), _n("March"),
    _n("April"),   _n("May"),      _n("June"),
    _n("July"),    _n("August"),   _n("September"),
    _n("October"), _n("November"), _n("December")
};

const wxString MONTHS_SHORT[12] =
{
    _n("Jan"), _n("Feb"), _n("Mar"),
    _n("Apr"), _n("May"), _n("Jun"),
    _n("Jul"), _n("Aug"), _n("Sep"),
    _n("Oct"), _n("Nov"), _n("Dec")
};

const wxString g_days_of_week[7] =
{
    _n("Sunday"), _n("Monday"), _n("Tuesday"), _n("Wednesday"),
    _n("Thursday"), _n("Friday"), _n("Saturday")
};

const wxString g_short_days_of_week[7] =
{
    _n("Sun"), _n("Mon"), _n("Tue"), _n("Wed"), _n("Thu"), _n("Fri"), _n("Sat")
};

bool mmParseISODate(const wxString& in, wxDateTime& out)
{
    // Avoid wxDateTime::ParseDateTime / ParseDate on ISO 8601 strings: they probe
    // localized weekday/month names via wxUILocaleImplCF::GetWeekDayName/GetMonthName,
    // which over-releases non-ASCII CFStrings in wxWidgets 3.3.x on macOS.
    if (in.IsEmpty() || !(
        out.ParseISOCombined(in) ||
        out.ParseISOCombined(in, ' ') ||
        out.ParseISODate(in)
    )) {
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

wxDateTime DEPRECATED_parseDateTime(const wxString& str_date)
{
    static std::unordered_map<wxString, wxDateTime> cache;

    if (str_date.empty())
        return wxInvalidDateTime;
    if (auto it = cache.find(str_date); it != cache.end())
        return it->second;

    // reset cache if it is too big
    if (cache.size() > 50000) {
        cache.clear();
    }

    // str_date is in ISO 8601 format "YYYY-MM-DD"
    wxDateTime date;
    date.ParseISOCombined(str_date) || date.ParseISODate(str_date);
    cache.insert(std::make_pair(str_date, date));
    return date;
}
