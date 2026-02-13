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

#include "mmFileHistory.h"
#include "model/SettingModel.h"

mmFileHistory::mmFileHistory(size_t maxFiles, wxWindowID idBase)
    : wxFileHistory(maxFiles, idBase)
{
}

mmFileHistory::~mmFileHistory()
{
    this->SaveHistory();
}

void mmFileHistory::ClearHistory()
{
    const int count = GetCount();
    for (int i = 0; i < count; i++)
    {
        RemoveFileFromHistory(0);
    }
}

void mmFileHistory::LoadHistory()
{
    wxString buf, historyFile;

    for (int i = GetMaxFiles(); i > 0 ; i--) {
        //if (i == 0) {
        //    historyFile = SettingModel::instance().getString("LASTFILENAME", "");
        //}
        //else {
            buf.Printf("RECENT_DB_%d", i);
            historyFile = SettingModel::instance().getString(buf, wxEmptyString);
        //}
        if (!historyFile.empty()) {
            AddFileToHistory(historyFile);
        }
    }
}

void mmFileHistory::SaveHistory()
{
    wxLogDebug("{{{ mmFileHistory::SaveHistory()");
    SettingModel::instance().Savepoint();
    wxString buf, historyFile;
    for (int i = 0; i < GetMaxFiles(); i++) {
        buf = wxString::Format("RECENT_DB_%d", i + 1);
        if (i < static_cast<int>(GetCount())) {
            historyFile = (i == 0) ?
                SettingModel::instance().getString("LASTFILENAME", "") :
                GetHistoryFile(i);
            wxLogDebug("%s %s", buf, historyFile);
            SettingModel::instance().setString(buf, historyFile);
        }
        else {
            SettingModel::instance().setString(buf, wxString(""));
        }
    }
    SettingModel::instance().ReleaseSavepoint();
    wxLogDebug("}}}");
}
