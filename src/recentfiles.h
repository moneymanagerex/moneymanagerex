/*************************************************************************
 Copyright (C) 2011,2012 Stefano Giorgio

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
 *************************************************************************/
#ifndef _MM_EX_RECENTFILES_H_
#define _MM_EX_RECENTFILES_H_
#include <wx/arrstr.h>
#include "defs.h"

/******************************************************************************
 Class: RecentDatabaseFiles

 Note:  For generic use, the number saved is one less that the specified size.
        The 0 element is not saved by this list.
*******************************************************************************/
class RecentDatabaseFiles
{
public:
    // This constructor is for recent files list
    RecentDatabaseFiles(wxMenu *menuRecentFiles);
    ~RecentDatabaseFiles();

    void loadRecentList();
    void saveRecentList();
    void setMenuFileItems();
    void updateRecentList(const wxString& currentFileName);
    void clearRecentList();
    wxString getRecentFile(int fileNum);

    // returns true if lastListedFileName is valid;
    bool validLastListedFile(wxString& lastListedFileName);

private:
    wxMenu *menuRecentFiles_;
    wxArrayString recentFileList_;

    int recentListSize_;
    wxString dbIndexName_;
};

#endif

