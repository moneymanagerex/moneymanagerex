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
#include <wx/fs_mem.h>
extern "C"
{
#include "mongoose/mongoose.h"
}

class mmGUIFrame * WebServerThread::m_pHandler = NULL;
class wxString * WebServerThread::m_htmlpage = NULL;

WebServerThread::WebServerThread(mmGUIFrame *handler) : wxThread(wxTHREAD_DETACHED)
{
    m_pHandler = handler;
}

WebServerThread::~WebServerThread()
{
    wxCriticalSectionLocker enter(m_pHandler->m_pFileSystemCS);
    if (m_htmlpage)
        delete m_htmlpage;

    wxCriticalSectionLocker enter2(m_pHandler->m_pThreadCS);
    m_pHandler->m_pThread = NULL;
}

int WebServerThread::index_html(struct mg_connection *conn) {
    int nReturn = 0;

    wxCriticalSectionLocker enter(m_pHandler->m_pFileSystemCS);
    if (m_htmlpage != NULL)
    {
        if (strcmp(conn->uri, "/") == 0)
        {
            nReturn = 1;
            mg_send_data(conn, m_htmlpage->GetData(), m_htmlpage->Length());
        }
        else
        {
            mg_printf_data(conn, "requested URI is [%s]", conn->uri);
        }
    }
    else
        mg_printf_data(conn, "Unable to find the requested URI is [%s]", conn->uri);

    return nReturn;
}

wxThread::ExitCode WebServerThread::Entry()
{
    // Create and configure the server
    struct mg_server *server = mg_create_server(NULL);
    mg_set_option(server, "listening_port", "8080"); // TODO: port number (8080) should be a user configuration value
    mg_set_option(server, "enable_directory_listing", "no");
    mg_add_uri_handler(server, "/", WebServerThread::index_html);

    // Serve requests
    while (1)
    {
        mg_poll_server(server, 1000);
        {
            wxCriticalSectionLocker enter(m_pHandler->m_pExitCS);
            if (m_pHandler->m_bExitServer) break;
        }
    }

    // Free memory for items which mongoose is not doing
    const char **all_opts = mg_get_valid_option_names();
    for (int i = 0; all_opts[i * 2] != NULL; i++)
    {
        const char *value = mg_get_option(server, all_opts[i * 2]);
        if (strlen(value) > 0)
        {
            free(const_cast<char *>(value));
        }
    };
    // Memory for uri handler is also not released

    // Cleanup, and free server instance
    mg_destroy_server(&server);

    return (wxThread::ExitCode)0;
}

void WebServerThread::ServerPage(wxString &htmlpage)
{
    // Test code to copy html to web server. Report can be viewed in browser window using "http://localhost:8080".
    wxString oldstr = "memory:";
    wxString newstr = "localhost:8080/";
    /*size_t nbr =*/ htmlpage.Replace(oldstr, newstr);
//    wxString pagename = "report.html";
//    wxMemoryFSHandler::AddFile(pagename, htmlpage);

    wxCriticalSectionLocker enter1(m_pHandler->m_pThreadCS);
    wxCriticalSectionLocker enter2(m_pHandler->m_pFileSystemCS);
    if (m_htmlpage)
        delete m_htmlpage;
    m_htmlpage = new wxString(htmlpage);
}
