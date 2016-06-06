#include "httpClient.h"
#include "mongoose/mongoose.h"
#include <iostream>
using namespace std;


httpClient::httpClient()
{
}


httpClient::~httpClient()
{
}

json::Object httpClient::web_get(const string& url, const json::Object& query /* = nullptr */, int timeout /* = 3 */)
{
	m_data = nullptr;
	m_bEnd = false;
	struct mg_mgr mgr;
	mg_mgr_init(&mgr, this); // user_data to hold a pointer to the class instance.

	if (query.is_discarded())
	{
		return nullptr;
	}

	string strUrl = url + "?msg="; //  + CUrlEncode::Encode(query.dump());
	cout << "req: " << strUrl << endl;
	mg_connect_http(&mgr, httpClient::ev_handler, strUrl.c_str(), NULL, NULL);

	time_t ts_start = time(NULL);
	time_t ts_end = ts_start;
	while (!m_bEnd)
	{
		if ((ts_end - ts_start) >= timeout)
		{
			cout << "timeout" << endl;
			break;
		}
		ts_end = mg_mgr_poll(&mgr, 1000);
	}
	mg_mgr_free(&mgr);

	return m_data;
}

json::Object httpClient::web_post(const string& url, const json& post /* = nullptr */, int timeout /* = 3 */)
{
	m_data = nullptr;
	m_bEnd = false;
	struct mg_mgr mgr;
	mg_mgr_init(&mgr, this); // user_data to hold a pointer to the class instance.

	if (post.is_discarded())
	{
		return nullptr;
	}

	string data = "msg=" ; //  + CUrlEncode::Encode(post.dump());
	cout << "req: " << url << "  "  << data << endl;
	mg_connect_http(&mgr, httpClient::ev_handler, url.c_str(), NULL /* headers */, data.c_str());

	time_t ts_start = time(NULL);
	time_t ts_end = ts_start;
	while (!m_bEnd)
	{
		if ((ts_end - ts_start) >= timeout)
		{
			cout << "timeout" << endl;
			break;
		}
		ts_end = mg_mgr_poll(&mgr, 1000);
	}
	mg_mgr_free(&mgr);

	return m_data;
}

void httpClient::ev_handler(struct mg_connection *nc, int ev, void *ev_data)
{
	struct http_message *hm = (struct http_message *) ev_data;
	httpClient* http = (httpClient*)nc->mgr->user_data;
	switch (ev)
	{
	case NS_CONNECT:
		if (*(int *)ev_data != 0)
		{
			fprintf(stderr, "connect() failed: %s\n", strerror(*(int *)ev_data));
			http->set_end();
		}break;
	case NS_HTTP_REPLY:
		nc->flags |= NSF_CLOSE_IMMEDIATELY;
		http->set_data(string(hm->body.p, hm->body.p + hm->body.len));
		http->set_end();
		break;
	default:
		break;
	}
}

void httpClient::set_data(const string& response)
{
	try
	{
		// m_data = json::parse(response);
	}
	catch (std::logic_error)
	{
		cout << "parse error" << response << endl;
	}
}

void httpClient::set_end()
{
	m_bEnd = true;
}
