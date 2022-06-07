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

#ifndef MM_EX_PATHS_H_
#define MM_EX_PATHS_H_
//----------------------------------------------------------------------------
/*
        mmex's files and dirs locations.
        All functions return absolute paths.
*/
//----------------------------------------------------------------------------
class wxString;
class wxIcon;
//----------------------------------------------------------------------------

namespace mmex
{

enum EDocFile { F_README = 0, F_CONTRIB, F_LICENSE, HTML_INDEX, HTML_WEBAPP, HTML_CUSTOM_SQL, HTML_INVESTMENT, HTML_BUDGET, HTML_REPORTS, DOC_FILES_MAX };
wxString getPathDoc(EDocFile f, bool url = true);

enum EResFile { TRANS_SOUND = 0, HOME_PAGE_TEMPLATE, THEMESDIR, REPORTS, RES_FILES_MAX };
const wxString getPathResource(EResFile f);

// use instead of getPathResource(PROGRAM_ICON)
const wxIcon& getProgramIcon();

enum ESharedFile { LANG_DIR = 0, SHARED_FILES_MAX };
const wxString getPathShared(ESharedFile f);

enum EUserFile { SETTINGS = 0, DIRECTORY, USERTHEMEDIR, USER_FILES_MAX };
const wxString getPathUser(EUserFile f);

const wxString getPathAttachment(const wxString &AttachmentsFolder);
bool isPortableMode();
const wxString getTempFolder();

} // namespace mmex

//----------------------------------------------------------------------------
#endif // MM_EX_PATHS_H_
//----------------------------------------------------------------------------
