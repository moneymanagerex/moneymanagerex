/*******************************************************
 Copyright (C) 2014 James Higley

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

#include "defs.h"
#include "webserver.h"
#include "platfdep.h"
#include "mongoose/mongoose.h"
#include "singleton.h"

WebServerThread::WebServerThread(): wxThread()
{
}

WebServerThread::~WebServerThread()
{
}

wxThread::ExitCode WebServerThread::Entry()
{
    // Create and configure the server
    //struct mg_server *server = mg_create_server(NULL, WebServerThread::IndexHtml);
    struct mg_server *server = mg_create_server(NULL, NULL);
    mg_set_option(server, "listening_port", "8080"); // TODO: port number (8080) should be a user configuration value
    mg_set_option(server, "document_root", mmex::GetResourceDir().GetPath());
    chdir(mg_get_option(server, "document_root"));

    // Serve requests
    while (1)
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
    m_thread = new WebServerThread();
    wxLogDebug("Mongoose Service started");
    m_thread->Run();
    return 0;
}

int Mongoose_Service::stop()
{
    if (m_thread->Delete() == wxTHREAD_NO_ERROR)
    {
        wxLogDebug("Mongoose Service ended.");
    }
    else
    {
        wxLogError("Can't delete the thread!");
    }
    return 0;
}
