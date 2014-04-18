/*************************************************************************
 Copyright (C) 2011,2012 Stefano Giorgio
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
#include "model/Model_Setting.h"

mmFileHistory::mmFileHistory(size_t maxFiles, wxWindowID idBase): wxFileHistory(maxFiles, idBase)
{
}

mmFileHistory::~mmFileHistory()
{
    this->Save();
}

void mmFileHistory::Clear()
{
    m_fileHistory.Clear();
}

void mmFileHistory::Load()
{
    m_fileHistory.Clear();

    wxString buf;
    buf.Printf("RECENT_DB_%d", 1);

    wxString historyFile;
    while (m_fileHistory.GetCount() < m_fileMaxFiles)
    {
        historyFile = Model_Setting::instance().GetStringSetting(buf, wxEmptyString);
        if (historyFile.empty()) break;

        m_fileHistory.Add(historyFile);

        buf.Printf("RECENT_DB_%d", (int)m_fileHistory.GetCount()+1);
        historyFile = wxEmptyString;
    }

    AddFilesToMenu();
}

void mmFileHistory::Save()
{
    for (size_t i = 0; i < m_fileMaxFiles; i++)
    {
        wxString buf;
        buf.Printf("RECENT_DB_%d", (int)i+1);
        if (i < m_fileHistory.GetCount())
            Model_Setting::instance().Set(buf, m_fileHistory[i]);
        else
            Model_Setting::instance().Set(buf, wxString(""));
    }
}
