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
#include "model/Model_Asset.h"
#include "model/Model_Stock.h"
#include "model/Model_StockHistory.h"
#include "model/Model_Infotable.h"
#include "model/Model_Setting.h"
#include "model/Model_Budgetyear.h"
#include "model/Model_Account.h"
#include "model/Model_Payee.h"
#include "model/Model_Checking.h"
#include "model/Model_Category.h"
#include "model/Model_Subcategory.h"
#include "model/Model_Billsdeposits.h"
#include "model/Model_Splittransaction.h"
#include "model/Model_Budget.h"
#include "model/Model_Report.h"
#include "model/Model_Attachment.h"
#include "model/Model_Usage.h"

std::string event_to_name(enum mg_event ev)
{
    switch (ev)
    {
    case MG_POLL:       return "MG_POLL";
    case MG_CONNECT:    return "MG_CONNECT";
    case MG_AUTH:       return "MG_AUTH";
    case MG_REQUEST:    return "MG_REQUEST";
    case MG_REPLY:      return "MG_REPLY";
    case MG_CLOSE:      return "MG_CLOSE";
    case MG_LUA:        return "MG_LUA";
    case MG_HTTP_ERROR: return "MG_HTTP_ERROR";
    default:            return "UNKNOWN";
    }
}

static int ev_handler(struct mg_connection *conn, enum mg_event ev) 
{
    wxLogDebug("%s, RUI: %s, TYPE: %s", conn->request_method, conn->uri, event_to_name(ev));
    if (ev == MG_AUTH) return MG_TRUE;

    return MG_FALSE;
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
    struct mg_server *server = mg_create_server(nullptr, ev_handler);
    mg_set_option(server, "listening_port", strPort.mb_str());
    mg_set_option(server, "document_root", wxFileName(mmex::getReportIndex()).GetPath().mb_str());
    wxSetWorkingDirectory(mg_get_option(server, "document_root"));

    // Serve requests 
    while (IsAlive())
    {
        mg_poll_server(server, 1000);
    }

    // Cleanup, and free server instance
    mg_destroy_server(&server);

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
