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

//----------------------------------------------------------------------------
#include "platfdep.h"
#include <wx/stdpaths.h>
#include <wx/filename.h>
//----------------------------------------------------------------------------

/*
    The root directory of the installation of MMEX.
*/
static const wxFileName GetInstallDir()
{
    const wxStandardPathsBase &p = wxStandardPaths::Get();
    wxFileName fname(p.GetExecutablePath());
    
    const wxArrayString &dirs = fname.GetDirs();

    if (dirs.Last().Upper() == "BIN") // bin\mmex.exe
        fname.RemoveLastDir();
    
    return fname;
}

//----------------------------------------------------------------------------

const wxString mmex::GetAppName()
{
    return "MoneyManagerEx";
}
//----------------------------------------------------------------------------

const wxFileName mmex::GetSharedDir()
{
    static wxFileName fname(GetInstallDir());
    return fname;
}
//----------------------------------------------------------------------------

const wxFileName mmex::GetDocDir()
{
    return GetSharedDir();
}
//----------------------------------------------------------------------------

const wxFileName mmex::GetResourceDir()
{
    static wxFileName fname;

    if (!fname.IsOk()) 
    {
        fname = GetSharedDir();
        fname.AppendDir("res");
    }

    return fname;
}
