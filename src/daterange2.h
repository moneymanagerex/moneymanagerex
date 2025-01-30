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

#ifndef MM_EX_DATE_RANGE2_H_
#define MM_EX_DATE_RANGE2_H_

#include <wx/datetime.h>

// DateRange2 implements only date calculations, without time.
// We use wxDateTime because wxWidgets does not have a type for date without time.
// The time is set to noon (12:00), in order to avoid accidental rounding errors.

class DateRange2
{
public:
    enum PERIOD_ID
    {
        PERIOD_ID_A = 0, // all
        PERIOD_ID_Y,     // year = 4 quarters = 12 months
        PERIOD_ID_Q,     // quarter = 3 months
        PERIOD_ID_M,     // month
        PERIOD_ID_W,     // week = 7 days
        PERIOD_ID_T,     // today
        PERIOD_ID_S,     // statement date
        PERIOD_ID_size,
        PERIOD_ID_none = PERIOD_ID_size
    };
private:
    typedef struct { PERIOD_ID id; wxString label; } PERIOD_INFO_t;
    static const PERIOD_INFO_t PERIOD_INFO[];
    typedef std::unordered_map<char, PERIOD_ID> PERIOD_LABEL_ID_t;
    static PERIOD_LABEL_ID_t PERIOD_LABEL_ID;
    static PERIOD_LABEL_ID_t make_period_label_id();

public:
    DateRange2(wxDateTime s = wxInvalidDateTime, wxDateTime t = wxInvalidDateTime);

protected:
    const int firstDay[2];                  // first day in PERIOD_ID_[YQM] (1..28)
    const wxDateTime::Month firstMonth[2];  // first month in PERIOD_ID_[YQ] (0..11)
    const wxDateTime::WeekDay firstWeekday; // first weekday in PERIOD_ID_W (0=Sun, 1=Mon)
    wxDateTime t;                           // the date of PERIOD_ID_T
    wxDateTime s;                           // the date of PERIOD_ID_S
    int so1 = 0;                            // start offset of first subrange
    int eo1 = 0;                            // end   offset of first subrange
    PERIOD_ID sp1 = PERIOD_ID_A;            // start period of first subrange
    PERIOD_ID ep1 = PERIOD_ID_A;            // end   period of first subrange
    int so2 = 0;                            // start offset of second subrange
    int eo2 = 0;                            // end   offset of second subrange
    PERIOD_ID sp2 = PERIOD_ID_none;         // start period of second subrange
    PERIOD_ID ep2 = PERIOD_ID_none;         // end   period of second subrange
    int f = 0;                              // index in first*[] (0=calendar, 1=financial)

public:
    void setT(wxDateTime date = wxInvalidDateTime);
    void setS(wxDateTime date = wxInvalidDateTime);
    wxDateTime getT() const;
    wxDateTime getS() const;
    void setRange(
        int so1_new = 0, PERIOD_ID sp1_new = PERIOD_ID_A,
        int eo1_new = 0, PERIOD_ID ep1_new = PERIOD_ID_A,
        int so2_new = 0, PERIOD_ID sp2_new = PERIOD_ID_none,
        int eo2_new = 0, PERIOD_ID ep2_new = PERIOD_ID_none,
        int f_new = 0
    );
    bool setRange(const wxString range);
    wxDateTime period_start(wxDateTime date, PERIOD_ID period) const;
    wxDateTime period_end(wxDateTime date, PERIOD_ID period) const;
    wxDateTime checking_start() const;
    wxDateTime checking_end() const;
    wxDateTime reporting_start() const;
    wxDateTime reporting_end() const;
    bool scanLabel(StringIt &str_i, StringIt str_end);
    static const wxString scanName(StringIt &str_i, StringIt str_end);
    const wxString getLabel() const;
    const wxString checking_Name() const;
    const wxString checking_Description() const;

private:
    static wxDateTime addOffset(wxDateTime date, int offset, PERIOD_ID period);
    static void scanWhiteSpace(StringIt &str_i, StringIt str_end);
    static char scanToken(StringIt &str_i, StringIt str_end, int &token_o, PERIOD_ID &token_p);
    static const wxString offset_str(int offset, bool show_zero = false);
    static const wxString offset_range_str(int so, int eo, bool show_zero = false);

#ifndef NDEBUG
private:
    DateRange2(
    int firstDay_0, int firstDay_1,
        wxDateTime::Month firstMonth_0, wxDateTime::Month firstMonth_1,
        wxDateTime::WeekDay firstWeekday_new,
        wxDateTime s_new = wxInvalidDateTime, wxDateTime t_new = wxInvalidDateTime
    );
public:
    static bool debug();
#endif
};

inline void DateRange2::setT(wxDateTime date)
{
    // wxInvalidDateTime means today
    if (date == wxInvalidDateTime) {
        // get the date of today, with time set to noon (12:00)
        date = wxDateTime(12, 0, 0, 0);
    }
    else {
        // set time to noon (12:00)
        date.SetHour(12).SetMinute(0).SetSecond(0).SetMillisecond(0);
    }
    t = date;
}

inline void DateRange2::setS(wxDateTime date)
{
    // wxInvalidDateTime means not applicable
    if (date != wxInvalidDateTime) {
        // set time to noon (12:00)
        date.SetHour(12).SetMinute(0).SetSecond(0).SetMillisecond(0);
    }
    s = date;
}

inline wxDateTime DateRange2::getT() const
{
    return t;
}

inline wxDateTime DateRange2::getS() const
{
    return s;
}

inline const wxString DateRange2::offset_str(int offset, bool show_zero)
{
    return (offset != 0) ? wxString::Format("%+d", offset) : show_zero ? "0" : "";
}

#endif
