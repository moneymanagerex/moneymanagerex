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
#include "mmDateTime.h"

mmDateTime::mmDateTime(wxDateTime dateTime) :
    m_dateTime(dateTime)
{
    if (!m_dateTime.IsValid()) {
        wxLogDebug("ERROR: mmDateTime::mmDateTime(): dateTime is invalid");
        m_dateTime = wxDateTime::Now();
    }
    m_dateTime.SetMillisecond(0);
}

mmDateTime::mmDateTime(const wxString& isoDateTime) :
    mmDateTime(parseDateTime(isoDateTime))
{
}

const wxTimeSpan mmDateTime::htol = wxTimeSpan::Milliseconds(500);

mmDateTimeN::mmDateTimeN(mmDateTime dateTime) :
    m_dateTimeN(dateTime.m_dateTime)
{
}

mmDateTimeN::mmDateTimeN(wxDateTime dateTimeN) :
    m_dateTimeN(dateTimeN)
{
    if (m_dateTimeN.IsValid()) {
        m_dateTimeN.SetMillisecond(0);
    }
}

mmDateTimeN::mmDateTimeN(const wxString& isoDateTimeN) :
    mmDateTimeN(parseDateTime(isoDateTimeN))
{
}

