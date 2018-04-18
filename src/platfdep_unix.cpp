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

namespace 
{

void SetInstallPrefix()
{
    wxStandardPathsBase &p = wxStandardPaths::Get();

    wxFileName fname(p.GetExecutablePath());
    fname.SetFullName(wxGetEmptyString());
    
    const wxArrayString &dirs = fname.GetDirs();

    if (dirs.Last().Lower() == "bin") // something like a /usr/bin or /usr/local/bin
        fname.RemoveLastDir();
    
    if (wxStandardPaths *pp = dynamic_cast<wxStandardPaths*>(&p))
    pp->SetInstallPrefix(fname.GetFullPath());
}

} // namespace 

//----------------------------------------------------------------------------

/*
    $(prefix)/share/mmex.
    Default install prefix is /usr (often /usr/local).
*/
const wxFileName mmex::GetSharedDir()
{
    static wxFileName fname;

    if (!fname.IsOk()) 
    {
        SetInstallPrefix();
        fname = wxFileName::DirName(wxStandardPaths::Get().GetDataDir());
    }

    return fname;
}
//----------------------------------------------------------------------------

/*
    $(prefix)/share/doc/mmex
*/
const wxFileName mmex::GetDocDir()
{
    static wxFileName fname;

    if (!fname.IsOk()) 
    {
        fname = GetSharedDir();

        const wxArrayString &dirs = fname.GetDirs();
        if (dirs.Last().Lower() == GetAppName())
            fname.RemoveLastDir(); // mmex folder

        fname.AppendDir("doc");
        fname.AppendDir(GetAppName());
    }

    return fname;
}
//----------------------------------------------------------------------------

/*
    $(prefix)/share/mmex/res
*/
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
//----------------------------------------------------------------------------

const wxString mmex::GetAppName()
{
    return "mmex";
}
//----------------------------------------------------------------------------
