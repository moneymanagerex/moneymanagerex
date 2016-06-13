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

#ifdef __WXMSW__
#define WIN32_LEAN_AND_MEAN
#endif

#include "Model_Usage.h"
#include "Model_Setting.h"
#include "util.h"
#include "constants.h"
#include "paths.h"
#include <wx/platinfo.h>
#include <wx/intl.h>
#include "mongoose/mongoose.h"
#include "option.h"

Model_Usage::Model_Usage()
: Model<DB_Table_USAGE_V1>()
, m_end(false)
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

void Model_Usage::ev_handler(struct mg_connection *nc, int ev, void *ev_data)
{
    struct http_message *hm = (struct http_message *) ev_data;
    Model_Usage* usage = (Model_Usage*)nc->mgr->user_data;
    int connect_status;

    switch (ev)
    {   
        case MG_EV_CONNECT:
            connect_status = * (int *) ev_data;
            if (connect_status != 0)
            {
                usage->m_end = true; 
            }
            break;
        case MG_EV_HTTP_REPLY:
            printf("Got reply:\n%.*s\n", (int) hm->body.len, hm->body.p);
            nc->flags |= MG_F_SEND_AND_CLOSE;
            usage->m_end = true;
            break;
        default:
            break;
    }
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

    std::cout<<url<<std::endl;

    struct mg_mgr mgr;
    struct mg_connection *nc;

    mg_mgr_init(&mgr, this);

    std::string user_agent = "User-Agent: " + std::string(wxGetOsDescription().c_str()) + "\r\n";
    nc = mg_connect_http(&mgr, Model_Usage::ev_handler, url.c_str(), user_agent.c_str(), NULL); // GET

    mg_set_protocol_http_websocket(nc);

    time_t ts_start = time(NULL);
    time_t ts_end = ts_start;
    this->m_end = false;

    while(!this->m_end)
    {
        if ((ts_end - ts_start) >= 1) // 1 sec
        {
            std::cout << "timeout" << std::endl;
            break;
        }
        ts_end = mg_mgr_poll(&mgr, 1000);
    }
    mg_mgr_free(&mgr);
}
