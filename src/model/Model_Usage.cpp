/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)

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

#include "Model_Usage.h"
#include "Model_Setting.h"
#include "util.h"
#include "constants.h"
#include "paths.h"
#include <wx/platinfo.h>
#include <wx/intl.h>
#include "option.h"

Model_Usage::Model_Usage()
: Model<DB_Table_USAGE_V1>()
{
}

Model_Usage::~Model_Usage()
{
}

/**
* Initialize the global Model_Usage table.
* Reset the Model_Usage table or create the table if it does not exist.
*/
Model_Usage& Model_Usage::instance(wxSQLite3Database* db)
{
    Model_Usage& ins = Singleton<Model_Usage>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);
    ins.m_start = wxDateTime::Now();

    return ins;
}

/** Return the static instance of Model_Usage table */
Model_Usage& Model_Usage::instance()
{
    return Singleton<Model_Usage>::instance();
}

void Model_Usage::append(const json::Object& o)
{
    this->a.Insert(o);
}

void Model_Usage::append_cache_usage(const json::Object& o)
{
    this->m_cache.Insert(o);
}

std::wstring Model_Usage::to_string() const
{
    json::Object o;
    o[L"start"] = json::String(m_start.FormatISOCombined(' ').ToStdWstring());
    o[L"end"] = json::String(wxDateTime::Now().FormatISOCombined(' ').ToStdWstring());
    o[L"usage"] = a;
    o[L"cache"] = m_cache;

    std::wstringstream ss;
    json::Writer::Write(o, ss);
    return ss.str();
}

wxString uuid()
{
    wxString UUID = Model_Setting::instance().GetStringSetting("UUID", wxEmptyString);
    if (UUID == wxEmptyString || UUID.length() < wxString("mac_20140428075834123").length())
    {
        wxDateTime now = wxDateTime::UNow();
        UUID = wxString::Format("%s_%s", wxPlatformInfo::Get().GetPortIdShortName(), now.Format("%Y%m%d%H%M%S%l"));
        Model_Setting::instance().Set("UUID", UUID);
    }
    return UUID;
}

void Model_Usage::pageview(const wxWindow* window)
{
    if (!window) return;
    if (window->GetName().IsEmpty()) return;

    const wxWindow *current = window;

    wxString documentTitle = window->GetLabel();
    if (documentTitle.IsEmpty()) documentTitle = window->GetName();

    wxString documentPath;
    while (current)
    {
        if (current->GetName().IsEmpty())
        {
            current = current->GetParent();
            continue;
        }
        documentPath = "/" + current->GetName() + documentPath; 
        current = current->GetParent();
    }

    return pageview(wxURI(documentPath).BuildURI(), wxURI(documentTitle).BuildURI());
}

void Model_Usage::pageview(const wxString& documentPath, const wxString& documentTitle)
{
    return pageview(std::string(documentPath.c_str()), std::string(documentTitle.c_str()));
}

void Model_Usage::pageview(const std::string& documentPath, const std::string& documentTitle)
{
    if (!Option::instance().SendUsageStatistics())
    {
        return;
    }

    static std::string GA_URL_ENDPOINT = "http://www.google-analytics.com/collect?";

    std::string url = GA_URL_ENDPOINT;

    std::map<std::string, std::string> parameters = {
        {"v", "1"},
        {"t", "pageview"},
        {"tid", "UA-51521761-6"},
        {"cid", std::string(uuid().c_str())},
        {"dp", documentPath},
        {"dt", documentTitle},
//        {"geoid", },
        {"ul", std::string(Option::instance().Language())},
        {"sr", std::string(wxString::Format("%ix%i", wxGetDisplaySize().GetX(), wxGetDisplaySize().GetY()).c_str())},
        {"vp", ""},
        {"sd", std::string(wxString::Format("%i-bits", wxDisplayDepth()))},
        // application
        {"an", "MoneyManagerEx"},
        {"av", std::string(mmex::version::string.c_str())}, // application version
        // custom dimensions
        {"cd1", std::string(wxPlatformInfo::Get().GetPortIdShortName().c_str())},
    };

    for (const auto & kv : parameters)
    {
        if (kv.second.empty()) continue;
        url += kv.first + "=" + kv.second + "&";
    }

    url.back() = ' '; // override the last &

	// Spawn thread to send statistics
	SendStatsThread* thread = new SendStatsThread(url);
	if (thread)
		thread->Run();
}

SendStatsThread::SendStatsThread(const std::string& url) : wxThread()
, m_url(url)
, m_end(false)
{
}

SendStatsThread::~SendStatsThread()
{
}

wxThread::ExitCode SendStatsThread::Entry()
{
    wxLogDebug("Sending stats (thread %lu, priority %u, %s, %i cores): %s",
        GetId(), GetPriority(), wxGetOsDescription(), GetCPUCount(), m_url);
    wxString result = wxEmptyString;
    //http_get_data(m_url, result, "User-Agent: " + wxGetOsDescription() + "\r\n");
    wxLogDebug("Response: %s", result);
    return nullptr;
}
