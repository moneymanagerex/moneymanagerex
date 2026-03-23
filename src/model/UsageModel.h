/*******************************************************
Copyright (C) 2013 - 2018 Guan Lisheng (guanlisheng@gmail.com)
Copyright (C) 2018 Stefano Giorgio (stef145g)

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

#include "base/defs.h"
#include "util/mmSingleton.h"
#include "table/_TableFactory.h"
#include "data/UsageData.h"

class ReportBase;

class UsageModel : public TableFactory<UsageTable, UsageData>
{
private:
    wxDateTime m_start;
    wxArrayString m_json_usage, m_json_cache;

public:
    UsageModel();
    ~UsageModel();

public:
    static UsageModel& instance(wxSQLite3Database* db);
    static UsageModel& instance();

public:
    void append_usage(const wxString& json_string);
    void append_cache(const wxString& json_string);
    auto to_json() const -> const wxString;

    void pageview(const wxString& documentPath, const wxString& documentTitle, long plt = 0 /*msec*/);
    void pageview(const wxWindow* window, long plt = 0 /*msec*/);
    void pageview(const wxWindow* window, const ReportBase* rb, long plt = 0 /*msec*/);
};
