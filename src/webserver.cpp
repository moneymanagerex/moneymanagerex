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

#ifdef __WXMSW__
#define WIN32_LEAN_AND_MEAN
#endif

#include "webserver.h"
#include "defs.h"
#include "platfdep.h"
#include "paths.h"
#include "mongoose/mongoose.h"
#include "singleton.h"
#include "model/Model_Setting.h"


static struct mg_serve_http_opts s_http_server_opts;

static void ev_handler(struct mg_connection *nc, int ev, void *p) 
{
    if (ev == MG_EV_HTTP_REQUEST)
    {
        mg_serve_http(nc, (struct http_message *) p, s_http_server_opts);
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
    struct mg_connection *nc;

    mg_mgr_init(&mgr, NULL);
    nc = mg_bind(&mgr, strPort.c_str(), ev_handler);
    
    mg_set_protocol_http_websocket(nc);
    std::string document_root(wxFileName(mmex::getReportIndex()).GetPath().c_str());
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

    return (wxThread::ExitCode)0;
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
    if (Model_Setting::instance().GetBoolSetting("ENABLEWEBSERVER", true))
    {
        m_thread = new WebServerThread();
        wxLogDebug("Mongoose Service started");
        m_thread->Run();
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
