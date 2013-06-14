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

#ifndef _MM_EX_PATHS_H_
#define _MM_EX_PATHS_H_
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

enum EDocFile { F_README, F_CONTRIB, F_LICENSE, F_VERSION, HTML_INDEX, HTML_CUSTOM_SQL, HTML_INVESTMENT, HTML_BUDGET, DOC_FILES_MAX };
wxString getPathDoc(EDocFile f);

enum EResFile { PROGRAM_ICON, TRANS_SOUND, CURRENCY_DB_SEED, RES_FILES_MAX };
wxString getPathResource(EResFile f);

// use instead of getPathResource(PROGRAM_ICON)
const wxIcon& getProgramIcon();

enum ESharedFile { LANG_DIR, SHARED_FILES_MAX };
wxString getPathShared(ESharedFile f);

enum EUserFile { SETTINGS, CUSTOM_REPORTS, DIRECTORY, USER_FILES_MAX };
wxString getPathUser(EUserFile f);

bool isPortableMode();

} // namespace mmex

//----------------------------------------------------------------------------
#endif // _MM_EX_PATHS_H_
//----------------------------------------------------------------------------
