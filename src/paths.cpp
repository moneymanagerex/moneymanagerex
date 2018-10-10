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
#include "constants.h"
#include "util.h"
#include "../resources/mmexico.xpm"
//----------------------------------------------------------------------------
#include "Model_Setting.h"
#include "Model_Infotable.h"
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
const wxFileName mmex::GetUserDir(bool create)
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

const wxFileName mmex::GetLogDir(bool create)
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
      "help/grm.html",
      "help/stocks_and_shares.html",
      "help/budget.html",
    };

    wxASSERT(f >= 0 && f < DOC_FILES_MAX);

    wxString path = files[f];
    path.Replace("/",wxFileName::GetPathSeparator());
    return path.Prepend(GetDocDir().GetPathWithSep());
}
//----------------------------------------------------------------------------

wxString mmex::getPathResource(EResFile f)
{
    static const wxString files[RES_FILES_MAX] = {
      "mmex.ico",
      "kaching.wav",
      "home_page.htt"
    };

    wxASSERT(f >= 0 && f < RES_FILES_MAX);

    wxFileName fname = GetResourceDir();
    fname.SetFullName(files[f]);

    return fname.GetFullPath();
}
//----------------------------------------------------------------------------

const wxString mmex::getPathShared(ESharedFile f)
{
    static const wxString files[SHARED_FILES_MAX] = {
      "locale"
    };

    wxASSERT(f >= 0 && f < SHARED_FILES_MAX);

    wxString path = files[f];
    path.Replace("/",wxFILE_SEP_PATH);
    return path.Prepend(GetSharedDir().GetPathWithSep());
}
//----------------------------------------------------------------------------

const wxString mmex::getPathUser(EUserFile f)
{
    static const wxString files[USER_FILES_MAX] = {
      getSettingsFileName(),
      getDirectory()
    };

    wxASSERT(f >= 0 && f < USER_FILES_MAX);

    wxFileName fname = GetUserDir(true);
    fname.SetFullName(files[f]);

    return fname.GetFullPath();
}
//----------------------------------------------------------------------------

/*
This function transforms mnemonic pathes to real one
For example %USERPROFILE%\MyBudget will be transformed to C:\Users\James\MyBudget
*/
const wxString mmex::getPathAttachment(const wxString &attachmentsFolder)
{
    if (attachmentsFolder == wxEmptyString)
        return wxEmptyString;

    wxString AttachmentsFolder = attachmentsFolder;
    const wxString sep = wxFileName::GetPathSeparator();
    const wxString LastDBPath = Model_Setting::instance().getLastDbPath();
    const wxString& LastDBFolder = wxFileName::FileName(LastDBPath).GetPath() + sep;
    const wxString& UserFolder = mmex::GetUserDir(false).GetPath() + sep;

    if (attachmentsFolder.StartsWith(ATTACHMENTS_FOLDER_USERPROFILE, &AttachmentsFolder))
        AttachmentsFolder.Prepend(wxGetHomeDir() + sep);
    else if (attachmentsFolder.StartsWith(ATTACHMENTS_FOLDER_DOCUMENTS, &AttachmentsFolder))
        AttachmentsFolder.Prepend(wxStandardPaths::Get().GetDocumentsDir() + sep);
    else if (attachmentsFolder.StartsWith(ATTACHMENTS_FOLDER_DATABASE, &AttachmentsFolder))
        AttachmentsFolder.Prepend(LastDBFolder);
    else if (attachmentsFolder.StartsWith(ATTACHMENTS_FOLDER_APPDATA, &AttachmentsFolder))
        AttachmentsFolder.Prepend(UserFolder);

    if (AttachmentsFolder.Last() != sep)
        AttachmentsFolder.Append(sep);
    if (Model_Infotable::instance().GetBoolInfo("ATTACHMENTSSUBFOLDER", true))
        AttachmentsFolder += wxString::Format("MMEX_%s_Attachments%s", wxFileName::FileName(LastDBPath).GetName(), sep);

    return AttachmentsFolder;
}

const wxString mmex::getTempFolder()
{
    const wxString path = mmex::isPortableMode() ? mmex::GetUserDir(false).GetPath() : wxStandardPaths::Get().GetTempDir();
    const wxString folder = mmex::isPortableMode() ? "tmp"
        : wxString::Format("%s_%s_tmp", mmex::GetAppName(), ::wxGetUserId());
    return wxString::Format("%s%s%s%s", path, wxString(wxFILE_SEP_PATH), folder, wxString(wxFILE_SEP_PATH));
}

const wxString mmex::getReportFullFileName(const wxString& name)
{
// FIXME: remove name param or use it below inplace of "index"
    return wxString::Format("%s%s%shtml", mmex::getTempFolder()
        , "index"
        , wxString(wxFILE_SEP_EXT));
}

//----------------------------------------------------------------------------

const wxIcon& mmex::getProgramIcon()
{
    static wxIcon icon(mmexico_xpm);
    return icon;
}
//----------------------------------------------------------------------------
