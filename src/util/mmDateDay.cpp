/*******************************************************
 Copyright (C) 2026 George Ef (george.a.ef@gmail.com)

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

#include "base/defs.h"
#include <wx/log.h>
#include "mmDateDay.h"

mmDateDay::mmDateDay(wxDateTime dateTime_new) :
    dateTime{dateTime_new}
{
    if (!dateTime.IsValid()) {
        wxLogDebug("ERROR: mmDateDay::mmDateDay(): dateTime is invalid");
        dateTime = wxDateTime(12, 0, 0, 0);
    }
    // set time to noon (12:00)
    dateTime.SetHour(12).SetMinute(0).SetSecond(0).SetMillisecond(0);
}

const wxTimeSpan mmDateDay::htol = wxTimeSpan::Hours(12);

mmDateDayN::mmDateDayN(mmDateDay dateDay_new) :
    dateTimeN(dateDay_new.dateTime)
{
}
mmDateDayN::mmDateDayN(wxDateTime dateTimeN_new) :
    dateTimeN{dateTimeN_new}
{
    if (dateTimeN.IsValid()) {
        // set time to noon (12:00)
        dateTimeN.SetHour(12).SetMinute(0).SetSecond(0).SetMillisecond(0);
    }
}
mmDateDayN::mmDateDayN(const wxString& isoDateN_new)
{
    *this = mmDateDayN(parseDateTime(isoDateN_new));
}

