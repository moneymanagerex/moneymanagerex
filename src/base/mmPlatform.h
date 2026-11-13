/*******************************************************
Copyright (C) 2009 VaDiM

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

#pragma once

// Platform-dependent API.

class wxFileName;
class wxString;

struct mmPlatform
{
    static auto platformType() -> const wxString;

    // wxStandardPaths uses wxApp::GetAppName(), so you should
    // call wxApp::SetAppName(mmPlatform::appName()) in wxApp::OnInit().
    // Use mmPlatform::getProgramName() for other purposes.
    static auto appName() -> const wxString;

    static auto shareDir() -> const wxFileName;
    static auto docDir() -> const wxFileName;
    static auto resourceDir() -> const wxFileName;

    static bool isDarkMode();

    // Call after wxLocale initialization, before initializing the GUI.
    static void initNumericLocale();
};
