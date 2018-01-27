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

void Model_Usage::AppendToUsage(const wxString& json_string)
{
    this->m_json_usage.Add(json_string);
    wxLogDebug("===== Model_Usage::AppendToUsage =================");
    wxLogDebug("%s", json_string);
}

void Model_Usage::AppendToCache(const wxString& json_string)
{
    this->m_json_cache.Add(json_string);
    wxLogDebug("===== Model_Usage::AppendToCache =================");
    wxLogDebug("%s", json_string);
}

wxString Model_Usage::To_JSON_String() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("start");
    json_writer.String(m_start.FormatISOCombined(' '));

    json_writer.Key("end");
    json_writer.String(wxDateTime::Now().FormatISOCombined(' '));

    json_writer.Key("usage");
    {
        json_writer.StartArray();
        for (size_t i = 0; i < m_json_usage.GetCount(); i++)
        {
            wxString item = m_json_usage.Item(i);
            json_writer.String(item);
        }
        json_writer.EndArray();
    }
    json_writer.Key("cache");
    {
        json_writer.StartArray();
        for (size_t i = 0; i < m_json_cache.GetCount(); i++)
        {
            wxString item = m_json_cache.Item(i);
            json_writer.String(item);
        }
        json_writer.EndArray();
    }
    json_writer.EndObject();
    return json_buffer.GetString();
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

void SendStatsThread::ev_handler(struct mg_connection *nc, int ev, void *ev_data)
{
    struct http_message *hm = (struct http_message *) ev_data;
	SendStatsThread* usage = (SendStatsThread*)nc->mgr->user_data;
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

void Model_Usage::pageview(const wxWindow* window, int plt /* = 0 msec*/)
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

    if (plt)
        timing(wxURI(documentPath).BuildURI(), wxURI(documentTitle).BuildURI(), plt);
    return pageview(wxURI(documentPath).BuildURI(), wxURI(documentTitle).BuildURI(), plt);
}

void Model_Usage::timing(const wxString& documentPath, const wxString& documentTitle, int plt /* = 0 msec*/)
{
    if (!Option::instance().SendUsageStatistics())
    {
        return;
    }

    static std::string GA_URL_ENDPOINT = "http://www.google-analytics.com/collect?";

    std::string url = GA_URL_ENDPOINT;

    std::vector<std::pair<wxString, wxString>> parameters = {
        { "v", "1" },
        { "t", "timing" },
        { "tid", "UA-51521761-6" },
        { "cid", uuid() },
        { "dp", documentPath },
        { "dt", documentTitle },
        //        {"geoid", },
        { "ul", Option::instance().LanguageISO6391() },
        { "sr", wxString::Format("%ix%i", wxGetDisplaySize().GetX(), wxGetDisplaySize().GetY()) },
        { "vp", "" },
        { "sd", wxString::Format("%i-bits", wxDisplayDepth()) },
        // application
        { "an", "MoneyManagerEx" },
        { "av", mmex::version::string }, // application version
                                         // custom dimensions
        { "cd1", wxPlatformInfo::Get().GetPortIdShortName() },
        { "plt", wxString::Format("%d", plt)}
    };

    for (const auto& kv : parameters)
    {
        if (kv.second.empty()) continue;
        url += wxString::Format("%s=%s&", kv.first, kv.second).ToStdString();
    }

    url.back() = ' '; // override the last &

	// Spawn thread to send statistics
	SendStatsThread* thread = new SendStatsThread(url);
	if (thread)
		thread->Run();
}

void Model_Usage::pageview(const wxString& documentPath, const wxString& documentTitle, int plt /* = 0 msec*/)
{
    if (!Option::instance().SendUsageStatistics())
    {
        return;
    }

    static std::string GA_URL_ENDPOINT = "http://www.google-analytics.com/collect?";

    std::string url = GA_URL_ENDPOINT;

    std::vector<std::pair<wxString, wxString>> parameters = {
        { "v", "1" },
        { "t", "pageview" },
        { "tid", "UA-51521761-6" },
        { "cid", uuid() },
        { "dp", documentPath },
        { "dt", documentTitle },
        //        {"geoid", },
        { "ul", Option::instance().LanguageISO6391() },
        { "sr", wxString::Format("%ix%i", wxGetDisplaySize().GetX(), wxGetDisplaySize().GetY()) },
        { "vp", "" },
        { "sd", wxString::Format("%i-bits", wxDisplayDepth()) },
        // application
        { "an", "MoneyManagerEx" },
        { "av", mmex::version::string }, // application version
                                         // custom dimensions
        { "cd1", wxPlatformInfo::Get().GetPortIdShortName() },
        { "plt", wxString::Format("%d", plt)}
    };

    for (const auto& kv : parameters)
    {
        if (kv.second.empty()) continue;
        url += wxString::Format("%s=%s&", kv.first, kv.second).ToStdString();
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
	std::cout << m_url << std::endl;

	struct mg_mgr mgr;
	struct mg_connection *nc;

	mg_mgr_init(&mgr, this);

	std::string user_agent = "User-Agent: " + wxGetOsDescription().ToStdString() + "\r\n";
	std::cout << user_agent << std::endl;
	nc = mg_connect_http(&mgr, SendStatsThread::ev_handler, m_url.c_str(), user_agent.c_str(), NULL); // GET

    if (nc != nullptr)
    {
	    mg_set_protocol_http_websocket(nc);

	    time_t ts_start = time(NULL);
	    time_t ts_end = ts_start;
	    this->m_end = false;

	    while (!this->m_end)
	    {
		    if ((ts_end - ts_start) >= 1) // 1 sec
		    {
			    std::cout << "timeout" << std::endl;
			    break;
		    }
		    ts_end = mg_mgr_poll(&mgr, 1000);
	    }
    }
	mg_mgr_free(&mgr);

	return nullptr;
}
