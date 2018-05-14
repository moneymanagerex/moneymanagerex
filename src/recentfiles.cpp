/*************************************************************************
 Copyright (C) 2011..2014 Stefano Giorgio
 Copyright (C) 2014 Guan Lisheng (guanlisheng@gmail.com)

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
#include "Model_Setting.h"

mmFileHistory::mmFileHistory(size_t maxFiles, wxWindowID idBase)
    : wxFileHistory(maxFiles, idBase)
{
}

mmFileHistory::~mmFileHistory()
{
    this->Save();
}

void mmFileHistory::Clear()
{
    int count = GetCount();
    for (int i = 0; i < count; i++)
    {
        RemoveFileFromHistory(0);
    }
}

void mmFileHistory::Load()
{
    int fileCount = 1;
    wxString buf;
    buf.Printf("RECENT_DB_%d", fileCount);

    wxString historyFile;
    while (fileCount <= GetMaxFiles())
    {
        historyFile = Model_Setting::instance().GetStringSetting(buf, wxEmptyString);
        if (!historyFile.empty())
        {
            AddFileToHistory(historyFile);
        }
        buf.Printf("RECENT_DB_%d", GetMaxFiles() - fileCount);
        fileCount++;
    }
}

void mmFileHistory::Save()
{
    Model_Setting::instance().Savepoint();
    for (int i = 1; i < GetMaxFiles(); i++)
    {
        wxString buf;
        buf.Printf("RECENT_DB_%d", i);
        if (i < (int)GetCount())
            Model_Setting::instance().Set(buf, GetHistoryFile(i));
        else
            Model_Setting::instance().Set(buf, wxString(""));
    }
    Model_Setting::instance().ReleaseSavepoint();
}
