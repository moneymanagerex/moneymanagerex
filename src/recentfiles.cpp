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

#include "recentfiles.h"
#include "model/Model_Setting.h"

RecentDatabaseFiles::RecentDatabaseFiles(wxMenu *menuRecentFiles)
: menuRecentFiles_(menuRecentFiles)
, recentListSize_(6)
, dbIndexName_("RECENT_DB_%i")
{
    recentFileList_.Add(Model_Setting::instance().getLastDbPath());
    for (int index = 1; index < recentListSize_; ++ index)
    {
        recentFileList_.Add(wxEmptyString);
    }
    loadRecentList();
    setMenuFileItems();
}

RecentDatabaseFiles::~RecentDatabaseFiles()
{
    saveRecentList();
}

void RecentDatabaseFiles::loadRecentList()
{
    for (int index = 1; index < recentListSize_; ++ index)
    {
        const wxString dbIndex = wxString::Format(dbIndexName_, index);
        recentFileList_[index] = Model_Setting::instance().GetStringSetting(dbIndex, "");
    }
}

void RecentDatabaseFiles::saveRecentList()
{
    for (int index = 1; index < recentListSize_; ++ index)
    {
        const wxString dbIndex = wxString::Format(dbIndexName_, index);
        Model_Setting::instance().Set(dbIndex, recentFileList_[index]);
    }
}

void RecentDatabaseFiles::setMenuFileItems()
{
    // exit if no menu pointer provided. Generic Usage. 
    if (!menuRecentFiles_)
        return;

    for (int i = 0; i < recentListSize_; i++)
    {
        if (menuRecentFiles_->FindItem(i + wxID_FILE))
            menuRecentFiles_->Delete(i + wxID_FILE);
    }

    for (int i = 0; i < recentListSize_; i++)
    {
        if (!recentFileList_[i].IsEmpty())
            menuRecentFiles_->Append(i + wxID_FILE, recentFileList_[i]);
    }

    if (recentFileList_[0].IsEmpty())
    {
        menuRecentFiles_->Append(wxID_FILE, _("Empty"));
        menuRecentFiles_->Enable(wxID_FILE, false);
    }
}

void RecentDatabaseFiles::updateRecentList(const wxString& currentFileName)
{
    // if the file given is actually the root file, then ignore the update.
    if (recentFileList_[0] == currentFileName)
        return;

    bool sameFileDetected = false;
    int emptyPosition = 0;
    /*****************************************************************
     If we are using an existing current file, remove the same 
     filename from the current list by making it empty.
    ******************************************************************/
    for (int index = 1; index < recentListSize_; ++ index)
    {
        if (recentFileList_[index] == currentFileName )
        {
            recentFileList_[index].Empty();
            sameFileDetected = true;
            emptyPosition = index;
        }
    }

    /*****************************************************************
     If the same file was found in the list and was made empty,
     move the same file empty position to the top of the list.
    ******************************************************************/
    while (sameFileDetected && (! recentFileList_[0].IsEmpty()))
    {
        for (int index = 1; index < emptyPosition + 1; ++ index)
        {
            if (recentFileList_[index].IsEmpty())
            {
                recentFileList_[index] = recentFileList_[index - 1];
                recentFileList_[index - 1].Empty();
            }
        }
    }

    /*****************************************************************
     For new files, we make room at the top for the new file.
    ******************************************************************/
    if (!sameFileDetected)
    {
        // move all entries down one
        for (int index = recentListSize_ - 2; index > -1; --index)
        {
            if (!recentFileList_[index].IsEmpty())
            {
                recentFileList_[index + 1] = recentFileList_[index];
            }
        }
    }

    /*****************************************************************
     Add the new file name to the top of the list, and
     set the menu items to reflect the new list.
    ******************************************************************/
    recentFileList_[0] = currentFileName;
    setMenuFileItems();
}

void RecentDatabaseFiles::clearRecentList()
{
    for (int index = 0; index < recentListSize_; ++ index)
    {
        recentFileList_[index].Empty();
    }
    setMenuFileItems();
}

wxString RecentDatabaseFiles::getRecentFile(int fileNum)
{
    return recentFileList_[fileNum];
}

void RecentDatabaseFiles::removeRecentFile(int fileNum)
{
    // move entries up one
    for (int index = fileNum; index < recentListSize_ - 1; ++index)
    {
        recentFileList_[index] = recentFileList_[index + 1];
    }
    recentFileList_[recentListSize_ - 1].Empty();
    setMenuFileItems();
}

bool RecentDatabaseFiles::validLastListedFile(wxString& lastListedFileName)
{
    bool validFileName = false;
    for (int index = recentListSize_ - 1; index > -1; --index)
    {
        if ( !recentFileList_[index].IsEmpty())
        {
            lastListedFileName = recentFileList_[index];
            validFileName = true;
            break;
        }
    }
    return validFileName;
}
