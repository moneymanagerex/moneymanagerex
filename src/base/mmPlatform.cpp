/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2013-2022 Nikolay Akimov
 Copyright (C) 2021-2024 Mark Whalley (mark@ipx.co.uk)

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

#include "mmPlatform.h"

#include <wx/string.h>
#include <wx/platinfo.h>
#include <clocale>

const wxString mmPlatform::platformType()
{
    return wxPlatformInfo::Get().GetOperatingSystemFamilyName().substr(0, 3).MakeLower();
}

void mmPlatform::initNumericLocale()
{
#ifdef __WXOSX__
    const wxPlatformInfo& platform = wxPlatformInfo::Get();
    // CoreUI can crash rendering NSAlert symbols with a comma-decimal C
    // locale on macOS 27 (#8536, wxWidgets #26977). The upstream report
    // confirms Apple's fix in 27.2 beta 2; leave other OS versions alone.
    if (platform.GetOSMajorVersion() == 27 && platform.GetOSMinorVersion() < 2) {
        // Keep translations, dates and monetary conventions unchanged.
        // Numeric UI formatting must use the UI/currency locale explicitly.
        std::setlocale(LC_NUMERIC, "C");
    }
#endif
}
