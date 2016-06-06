#pragma once

#include <string>
using std::string;

#include "cajun/json/elements.h"
#include "cajun/json/reader.h"
#include "cajun/json/writer.h"

class httpClient
{
public:
	httpClient();
	~httpClient();
	void set_data(const string& response);
	void set_end();
public:
	json::Object web_get(const string& url, const json::Object& query = nullptr, int timeout = 3);
	json::Object web_post(const string& url, const json::Object& post = nullptr, int timeout = 3);
private:
	static void ev_handler(struct mg_connection *nc, int ev, void *ev_data);
	json::Object m_data;
	bool m_bEnd;
};
