/*******************************************************
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
 ********************************************************/
#pragma once

#include "model/PrefModel.h"
#include <map>
#include <mutex>
#include <vector>

struct DatabaseRowChange
{
    wxString table;
    wxLongLong rowid;
    wxSQLite3Hook::wxUpdateType type;
};

// Generic SQLite change journal input. It performs no SQL or network work from
// inside SQLite callbacks; consumers drain the collected row keys later.
class DatabaseChangeCollector
{
public:
    void record(wxSQLite3Hook::wxUpdateType type, const wxString& table, wxLongLong rowid)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_changes[{table, rowid.GetValue()}] = {table, rowid, type};
    }

    std::vector<DatabaseRowChange> drain()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<DatabaseRowChange> result;
        result.reserve(m_changes.size());
        for (auto& [key, change] : m_changes) result.push_back(change);
        m_changes.clear();
        return result;
    }

private:
    std::mutex m_mutex;
    std::map<std::pair<wxString, wxLongLong_t>, DatabaseRowChange> m_changes;
};

class CommitCallbackHook : public wxSQLite3Hook
{
public:
    virtual bool CommitCallback()
    {
        PrefModel::instance().setDatabaseUpdated(true);
        return false;
    }
};

class UpdateCallbackHook : public wxSQLite3Hook
{
public:
    explicit UpdateCallbackHook(DatabaseChangeCollector* collector = nullptr) : m_collector(collector) {}

    virtual void UpdateCallback(wxSQLite3Hook::wxUpdateType type, [[maybe_unused]] const wxString& database, const wxString& table, wxLongLong rowid)
    {
        if (m_collector) m_collector->record(type, table, rowid);
    }

private:
    DatabaseChangeCollector* m_collector;
};
