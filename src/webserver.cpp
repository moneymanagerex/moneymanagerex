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

#include "webserver.h"
#include "mmframe.h"
#include "platfdep.h"
#include <wx/fs_mem.h>
#include "mongoose/mongoose.h"
#include "singleton.h"

class mmGUIFrame * WebServerThread::m_pHandler = NULL;

WebServerThread::WebServerThread(mmGUIFrame *handler) : wxThread(wxTHREAD_DETACHED)
{
    m_pHandler = handler;
}

WebServerThread::~WebServerThread()
{
    wxCriticalSectionLocker enter2(m_pHandler->m_pThreadCS);
    m_pHandler->m_pThread = NULL;
}

bool WebServerThread::SendFile(struct mg_connection *conn, const wxString &filename)
{
    bool bFound = false;

    wxFileSystem fs;
    wxFSFile *pFile = fs.OpenFile(filename);
    if (pFile)
    {
        bFound = true;
        wxInputStream *is = pFile->GetStream();
        while (is->CanRead())
        {
            char szBuffer[256];
            is->Read(&szBuffer, sizeof(szBuffer));
            int nRead = is->LastRead();
            mg_send_data(conn, szBuffer, nRead);
            if (nRead < sizeof(szBuffer))
                break;
        }
        delete pFile;
    }
    return bFound;
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
        {
            wxCriticalSectionLocker enter(m_pHandler->m_pExitCS);
            if (m_pHandler->m_bExitServer) break;
        }
    }

    // Cleanup, and free server instance
    mg_destroy_server(&server);

    return (wxThread::ExitCode)0;
}

Mongoose_Service::Mongoose_Service()
{}

Mongoose_Service::~Mongoose_Service()
{}

Mongoose_Service&
Mongoose_Service::instance()
{
    return Singleton<Mongoose_Service>::instance();
}

int Mongoose_Service::open()
{
    // TODO
    return 0;
}

int Mongoose_Service::svc()
{
    // TODO 
    return 0;
}

int Mongoose_Service::stop()
{
    // TODO cleanup
    return 0;
}
