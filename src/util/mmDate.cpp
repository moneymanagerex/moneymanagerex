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
#include "mmDate.h"

mmDate::mmDate(wxDateTime dateTime) :
    m_dateTime(dateTime)
{
    if (!m_dateTime.IsValid()) {
        wxLogDebug("ERROR: mmDate::mmDate(): dateTime is invalid");
        m_dateTime = wxDateTime(12, 0, 0, 0);
    }
    // set time to noon (12:00)
    m_dateTime.SetHour(12).SetMinute(0).SetSecond(0).SetMillisecond(0);
}

mmDate::mmDate(mmDateTime dateTime) :
    mmDate(dateTime.getDateTime())
{
}

mmDate::mmDate(const wxString& isoDateTime) :
    mmDate(parseDateTime(isoDateTime))
{
}

const wxTimeSpan mmDate::htol = wxTimeSpan::Hours(12);

mmDateN::mmDateN(mmDate dateDay) :
    m_dateTimeN(dateDay.m_dateTime)
{
}

mmDateN::mmDateN(wxDateTime dateTimeN) :
    m_dateTimeN(dateTimeN)
{
    if (m_dateTimeN.IsValid()) {
        // set time to noon (12:00)
        m_dateTimeN.SetHour(12).SetMinute(0).SetSecond(0).SetMillisecond(0);
    }
}

mmDateN::mmDateN(mmDateTimeN dateTimeN) :
    mmDateN(dateTimeN.getDateTimeN())
{
}

mmDateN::mmDateN(const wxString& isoDateTimeN) :
    mmDateN(parseDateTime(isoDateTimeN))
{
}

