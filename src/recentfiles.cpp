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
#include "guiid.h"
#include "model/Model_Setting.h"

RecentDatabaseFiles::RecentDatabaseFiles(wxMenu *menuRecentFiles)
: menuRecentFiles_(menuRecentFiles)
, recentListSize_(6)
, dbIndexName_("RECENT_DB_")
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
        wxString dbIndex = wxString() << dbIndexName_ << index;
        recentFileList_[index] = Model_Setting::instance().GetStringSetting(dbIndex, "");
    }
}

void RecentDatabaseFiles::saveRecentList()
{
    for (int index = 1; index < recentListSize_; ++ index)
    {
        wxString dbIndex = wxString() << dbIndexName_ << index;
        Model_Setting::instance().Set(dbIndex, recentFileList_[index] );
    }
}

void RecentDatabaseFiles::setMenuFileItems()
{
    // exit if no menu pointer provided. Generic Usage. 
    if (!menuRecentFiles_)
        return;

    if (menuRecentFiles_->FindItem(MENU_RECENT_FILES_0))
        menuRecentFiles_->Delete(MENU_RECENT_FILES_0);

    if (menuRecentFiles_->FindItem(wxID_FILE1))
        menuRecentFiles_->Delete(wxID_FILE1);

    if (menuRecentFiles_->FindItem(wxID_FILE2))
        menuRecentFiles_->Delete(wxID_FILE2);

    if (menuRecentFiles_->FindItem(wxID_FILE3))
        menuRecentFiles_->Delete(wxID_FILE3);

    if (menuRecentFiles_->FindItem(wxID_FILE4))
        menuRecentFiles_->Delete(wxID_FILE4);

    if (menuRecentFiles_->FindItem(wxID_FILE5))
        menuRecentFiles_->Delete(wxID_FILE5);

    if (recentFileList_[0].IsEmpty())
    {
        menuRecentFiles_->Append(MENU_RECENT_FILES_0, _("Empty"));
        menuRecentFiles_->Enable(MENU_RECENT_FILES_0, false);
    }
    else
    {
        menuRecentFiles_->Append(MENU_RECENT_FILES_0, recentFileList_[0]);
    }
    
    if (! recentFileList_[1].IsEmpty()) 
        menuRecentFiles_->Append(wxID_FILE1, recentFileList_[1]);

    if (! recentFileList_[2].IsEmpty()) 
        menuRecentFiles_->Append(wxID_FILE2, recentFileList_[2]);

    if (! recentFileList_[3].IsEmpty()) 
        menuRecentFiles_->Append(wxID_FILE3, recentFileList_[3]);

    if (! recentFileList_[4].IsEmpty()) 
        menuRecentFiles_->Append(wxID_FILE4, recentFileList_[4]);

    if (! recentFileList_[5].IsEmpty()) 
        menuRecentFiles_->Append(wxID_FILE5, recentFileList_[5]);
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
