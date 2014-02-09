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

//----------------------------------------------------------------------------
#ifndef _WEB_SERVER_H_
#define _WEB_SERVER_H_

#include <wx/thread.h>

class mmGUIFrame;

class WebServerThread : public wxThread
{
public:
    WebServerThread(mmGUIFrame *handler);
    ~WebServerThread();

    static void ServerPage(wxString htmlpage);

protected:
    static wxString *m_htmlpage;
    static mmGUIFrame *m_pHandler;

    virtual ExitCode Entry();
    static int IndexHtml(struct mg_connection *conn);
    static bool SendFile(struct mg_connection *conn, wxString &filename);
};

//----------------------------------------------------------------------------
#endif // _WEB_SERVER_H_
//----------------------------------------------------------------------------
