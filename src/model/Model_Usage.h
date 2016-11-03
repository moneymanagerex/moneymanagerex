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

#ifndef MODEL_USAGE_H
#define MODEL_USAGE_H

#include "Model.h"
#include "db/DB_Table_Usage_V1.h"
#include <wx/thread.h>

struct mg_connection;

class SendStatsThread : public wxThread
{
public:
	SendStatsThread(const std::string& url);
	~SendStatsThread();
	static void ev_handler(struct mg_connection *nc, int ev, void *ev_data);

	bool m_end;

protected:
	std::string m_url;

	virtual ExitCode Entry();
};

class Model_Usage : public Model<DB_Table_USAGE_V1>
{
public:
    Model_Usage();
    ~Model_Usage();

public:
    /**
    Initialize the global Model_Usage table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for Model_Usage table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Usage& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for Model_Usage table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Usage& instance();

private:
    wxDateTime m_start;
    json::Array a, m_cache;

public:
    void append(const json::Object& o);
    void append_cache_usage(const json::Object& o);
    std::wstring to_string() const;

public:
    void pageview(const std::string& documentPath, const std::string& documentTitle);
    void pageview(const wxString& documentPath, const wxString& documentTitle);
    void pageview(const wxWindow* window);
};

#endif // 
