/*******************************************************
 Copyright (C) 2014 James Higley
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

#include "webserver.h"
#include "defs.h"
#include "platfdep.h"
#include "paths.h"
#include "mongoose/mongoose.h"
#include "singleton.h"
#include "model/Model_Setting.h"
#include "model/Model_Report.h"
#include "model/Model_Setting.h"
#include "model/Model_Infotable.h"

#include "cajun/json/elements.h"
#include "cajun/json/reader.h"
#include "cajun/json/writer.h"

static struct mg_serve_http_opts s_http_server_opts;

static void handle_sql(struct mg_connection* nc, struct http_message* hm)
{
    char query[0xffff];
    mg_get_http_var(&hm->query_string, "query", query, sizeof(query));
    std::cout<<query<<std::endl;

    json::Object result;
    result[L"query"] = json::String(wxString(query).ToStdWstring());
    bool ret = Model_Report::instance().get_objects_from_sql(wxString(query), result); 

    for (const auto & r : Model_Setting::instance().all())
    {
        result[r.SETTINGNAME.ToStdWstring()] = json::String(r.SETTINGVALUE.ToStdWstring());
    }

    for (const auto & r : Model_Infotable::instance().all())
    {
        result[r.INFONAME.ToStdWstring()] = json::String(r.INFOVALUE.ToStdWstring());
    }
     
    std::wstringstream ss;
    json::Writer::Write(result, ss);
    std::wstring str = ss.str();
    std::cout<<str<<std::endl;

    mg_printf(nc, "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json; charset=utf-8\r\n"
                "Content-Length: %lu\r\n\r\n%ls", str.length(), str.c_str());
}

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) 
{
    struct http_message *hm = (struct http_message *) ev_data;

    switch (ev)
    {
        case MG_EV_HTTP_REQUEST:
            if (mg_vcmp(&hm->uri, "/api/v1/sql") == 0)
            {
                handle_sql(nc, hm);
            }
            else
            {
                mg_serve_http(nc, hm, s_http_server_opts); /* Serve static content */
            }
            break;
        default:
            break;
    }
}

WebServerThread::WebServerThread(): wxThread()
{
}

WebServerThread::~WebServerThread()
{
}

wxThread::ExitCode WebServerThread::Entry()
{
    // Get user setting
    int webserverPort = Model_Setting::instance().GetIntSetting("WEBSERVERPORT", 8080);
    const wxString& strPort = wxString::Format("%d", webserverPort);

    // Create and configure the server
    struct mg_mgr mgr;

    mg_mgr_init(&mgr, NULL);
    struct mg_connection* nc = mg_bind(&mgr, strPort.c_str(), ev_handler);
    if (nc == nullptr)
    {
        wxLogDebug(wxString::Format("mg_bind(%s) failed", strPort));
        mg_mgr_free(&mgr);
        return (wxThread::ExitCode)-1;
    }
    
    mg_set_protocol_http_websocket(nc);
    std::string document_root(wxFileName(mmex::getReportFullName("index")).GetPath().c_str());
    s_http_server_opts.document_root = document_root.c_str();
    s_http_server_opts.enable_directory_listing = "yes";

    wxSetWorkingDirectory(wxString(s_http_server_opts.document_root));

    // Serve requests 
    while (IsAlive())
    {
        mg_mgr_poll(&mgr, 1000);
    }

    // Cleanup, and free server instance
    mg_mgr_free(&mgr);

    return nullptr;
}

Mongoose_Service::Mongoose_Service(): m_thread(0)
{
}

Mongoose_Service::~Mongoose_Service()
{}

Mongoose_Service&
Mongoose_Service::instance()
{
    return Singleton<Mongoose_Service>::instance();
}

int Mongoose_Service::open()
{
    this->svc();
    return 0;
}

int Mongoose_Service::svc()
{
    if (Model_Setting::instance().GetBoolSetting("ENABLEWEBSERVER", false))
    {
        m_thread = new WebServerThread();
        if (m_thread->Run() == wxTHREAD_NO_ERROR)
        {
            wxLogDebug("Mongoose Service started");
        }
        else
        {
            wxLogDebug("Can't create the web server thread!");
            delete m_thread;
            m_thread = 0;
        }
    }
    return 0;
}

int Mongoose_Service::stop()
{
    if (m_thread != 0)
    {
        if (m_thread->Delete() == wxTHREAD_NO_ERROR)
        {
            wxLogDebug("Mongoose Service ended.");
        }
        else
        {
            wxLogError("Can't delete the thread!");
        }
    }
    return 0;
}
