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
#include "paths.h"
#include "platfdep.h"
#include "guiid.h"
//----------------------------------------------------------------------------
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/icon.h>
//----------------------------------------------------------------------------

namespace
{

inline const wxString getSettingsFileName()
{
    return "mmexini.db3";
}
//----------------------------------------------------------------------------

inline const wxString getCustomReportIndexFilename()
{
    return "CustomReportIndex.txt";
}
//----------------------------------------------------------------------------

inline const wxString getDirectory()
{
    return "";
}
//----------------------------------------------------------------------------

wxFileName getSettingsPathPortable()
{
    wxFileName f = mmex::GetSharedDir();
    f.SetFullName(getSettingsFileName());

    return f;
}

} // namespace

//----------------------------------------------------------------------------

/*
        This routine is platform-independent.

        MMEX is a portable application which means ability to to run
        without installation, for example, from USB flash drive.

        If mmex finds mmexini.db3 in its folder, it assumes portable
        mode and GetUserDir() in such case points to that folder.

        FIXME: security issue - temp files will be created on host filesystem.
*/
wxFileName mmex::GetUserDir(bool create)
{
    static wxFileName fname;

    if (!fname.IsOk())
    {
        fname = getSettingsPathPortable();

        bool portable_file_ok = fname.IsFileWritable() && fname.IsFileReadable();

        if (!portable_file_ok)
        {
            fname.AssignDir(wxStandardPaths::Get().GetUserDataDir());

            if (create && !fname.DirExists())
            {
                portable_file_ok = fname.Mkdir(0700, wxPATH_MKDIR_FULL); // 0700 - octal, "111 000 000"
                wxASSERT(portable_file_ok);
            }
        }

        fname.SetFullName(wxGetEmptyString());
    }

    return fname;
}
//----------------------------------------------------------------------------

wxFileName mmex::GetLogDir(bool create)
{
    static wxFileName fname;

    if (!fname.IsOk())
    {
        fname = GetUserDir(create);
        //FIXME: file not found ERROR
        //fname.AppendDir("logs");
    }

    return fname;
}
//----------------------------------------------------------------------------

bool mmex::isPortableMode()
{
    wxFileName f = getSettingsPathPortable();
    return f.GetFullPath() == getPathUser(SETTINGS);
}
//----------------------------------------------------------------------------

wxString mmex::getPathDoc(const EDocFile& f)
{
    static const wxString files[DOC_FILES_MAX] = {
      "README.TXT",
      "contrib.txt",
      "license.txt",
      "version.txt",
      "help/index.html",
      "help/custom_reports.html",
      "help/investment.html",
      "help/budget.html",
    };

    wxASSERT(f >= 0 && f < DOC_FILES_MAX);

    wxString path = GetDocDir().GetPath();
    path += wxFileName::GetPathSeparator();
    path += files[f];

    return path;
}
//----------------------------------------------------------------------------

wxString mmex::getPathResource(EResFile f)
{
    static const wxString files[RES_FILES_MAX] = {
      "mmex.ico",
      "kaching.wav",
      "currency_seed.csv"
    };

    wxASSERT(f >= 0 && f < RES_FILES_MAX);

    wxFileName fname = GetResourceDir();
    fname.SetFullName(files[f]);

    return fname.GetFullPath();
}
//----------------------------------------------------------------------------

wxString mmex::getPathShared(ESharedFile f)
{
    static const wxString files[SHARED_FILES_MAX] = {
      "po"
    };

    wxASSERT(f >= 0 && f < SHARED_FILES_MAX);

    wxFileName fname = GetSharedDir();
    fname.SetFullName(files[f]);

    return fname.GetFullPath();
}
//----------------------------------------------------------------------------

wxString mmex::getPathUser(EUserFile f)
{
    static const wxChar* files[USER_FILES_MAX] = {
      getSettingsFileName(),
      getCustomReportIndexFilename(),
      getDirectory()
    };

    wxASSERT(f >= 0 && f < USER_FILES_MAX);

    wxFileName fname = GetUserDir(true);
    fname.SetFullName(files[f]);

    return fname.GetFullPath();
}
//----------------------------------------------------------------------------

const wxIcon& mmex::getProgramIcon()
{
    static wxIcon icon(mmexico_xpm);
    return icon;
}
//----------------------------------------------------------------------------
