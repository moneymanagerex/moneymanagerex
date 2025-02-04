/*******************************************************
Copyright (C) 2013 - 2018 Guan Lisheng (guanlisheng@gmail.com)
Copyright (C) 2018 Stefano Giorgio (stef145g)

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

#include "Model_Usage.h"
#include "Model_Setting.h"
#include "util.h"
#include "constants.h"
#include "paths.h"
#include <wx/platinfo.h>
#include <wx/thread.h>
#include <wx/intl.h>
#include "option.h"

Model_Usage::Model_Usage()
    : Model<DB_Table_USAGE_V1>()
{
}

Model_Usage::~Model_Usage()
{
}

/**
* Initialize the global Model_Usage table.
* Reset the Model_Usage table or create the table if it does not exist.
*/
Model_Usage& Model_Usage::instance(wxSQLite3Database* db)
{
    Model_Usage& ins = Singleton<Model_Usage>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);
    ins.m_start = wxDateTime::UNow();

    return ins;
}

/** Return the static instance of Model_Usage table */
Model_Usage& Model_Usage::instance()
{
    return Singleton<Model_Usage>::instance();
}

void Model_Usage::AppendToUsage(const wxString& json_string)
{
    wxLogDebug("===== Model_Usage::AppendToUsage =================");
    wxLogDebug("%s", json_string);
    wxLogDebug("\n");
    m_json_usage.Add(json_string);
}

void Model_Usage::AppendToCache(const wxString& json_string)
{
    wxLogDebug("===== Model_Usage::AppendToCache =================");
    wxLogDebug("%s", json_string);
    wxLogDebug("\n");
    m_json_cache.Add(json_string);
}

wxString Model_Usage::To_JSON_String() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("start");
    json_writer.String(m_start.FormatISOCombined(' ').utf8_str());

    json_writer.Key("end");
    json_writer.String(wxDateTime::Now().FormatISOCombined(' ').utf8_str());

    json_writer.Key("usage");
    {
        json_writer.StartArray();
        for (size_t i = 0; i < m_json_usage.GetCount(); i++)
        {
            const char* item = m_json_usage.Item(i).utf8_str();
            json_writer.RawValue(item, strlen(item), kObjectType);
        }
        json_writer.EndArray();
    }
    json_writer.Key("cache");
    {
        json_writer.StartArray();
        for (size_t i = 0; i < m_json_cache.GetCount(); i++)
        {
            const char* item = m_json_cache.Item(i).utf8_str();
            json_writer.RawValue(item, strlen(item), kObjectType);
        }
        json_writer.EndArray();
    }
    json_writer.EndObject();
    return wxString::FromUTF8(json_buffer.GetString());
}

std::pair<wxString /*UUID*/, wxString /*UID*/> uuid()
{
    wxString UUID = Model_Setting::instance().getString("UUID", wxEmptyString);
    wxString UID = Model_Setting::instance().getString("UID", wxEmptyString);

    if (!UUID.IsEmpty() && !UID.IsEmpty())
        return std::make_pair(UUID, UID);

    if (UUID.IsEmpty() && UID.IsEmpty())
    {
        wxDateTime now = wxDateTime::UNow();
        UUID = UID = wxString::Format("%s_%s", wxPlatformInfo::Get().GetPortIdShortName(), now.Format("%Y%m%d%H%M%S%l"));
        Model_Setting::instance().setString("UUID", UUID);
        Model_Setting::instance().setString("UID", UID);
    }
    else if (UUID.IsEmpty())
    {
        UUID = UID;
        Model_Setting::instance().setString("UUID", UUID);
    }
    else if (UID.IsEmpty())
    {
        UID = UUID;
        Model_Setting::instance().setString("UID", UID);
    }

    return std::make_pair(UUID, UID);
}

class SendStatsThread : public wxThread
{
public:
    explicit SendStatsThread(const wxString& url) : wxThread(), m_url(url) {
    }
    explicit SendStatsThread(const wxString& url, const wxString& payload) : wxThread()
        , m_url(url)
        , m_payload(payload)
        {
    }
    ~SendStatsThread() {
    }

protected:
    wxString m_url;
    wxString m_payload;
    virtual ExitCode Entry();
};

void Model_Usage::pageview(const wxWindow* window, long plt /* = 0 msec*/)
{
    if (!window) return;
    if (window->GetName().IsEmpty()) return;

    const wxWindow *current = window;

    wxString documentTitle = window->GetLabel();
    if (documentTitle.IsEmpty()) documentTitle = window->GetName();

    wxString documentPath;
    while (current)
    {
        if (current->GetName().IsEmpty())
        {
            current = current->GetParent();
            continue;
        }
        documentPath = "/" + current->GetName() + documentPath;
        current = current->GetParent();
    }

    pageview(wxURI(documentPath).BuildURI(), documentTitle, plt);
}

void Model_Usage::pageview(const wxWindow* window, const mmPrintableBase* rb, long plt /* = 0 msec*/)
{
    if (!window) return;
    if (window->GetName().IsEmpty()) return;

    const wxWindow *current = window;

    wxString documentTitle = rb->getReportTitle(false);

    wxString documentPath;
    while (current)
    {
        if (current->GetName().IsEmpty())
        {
            current = current->GetParent();
            continue;
        }
        documentPath = "/" + current->GetName() + documentPath;
        current = current->GetParent();
    }

    pageview(wxURI(documentPath).BuildURI(), documentTitle, plt);
}

void Model_Usage::pageview(const wxString& documentPath, const wxString& documentTitle, long plt /* = 0 msec*/)
{
    if (!Option::instance().doSendUsageStats())
    {
        return;
    }

    wxString url = mmex::weblink::AMP;

    Document document;
    document.SetObject();

    Value events(kArrayType);

    Value event(kObjectType);
    Value event_type("page_view", document.GetAllocator());
    event.AddMember("event_type", event_type, document.GetAllocator());

    Value device_id(uuid().first.utf8_str(), document.GetAllocator());
    event.AddMember("device_id", device_id, document.GetAllocator());

    Value user_id(uuid().second.utf8_str(), document.GetAllocator());
    event.AddMember("user_id", user_id, document.GetAllocator());

    Value platform(wxPlatformInfo::Get().GetPortIdShortName().utf8_str(), document.GetAllocator());
    event.AddMember("platform", platform, document.GetAllocator());

    Value os_name(wxGetOsDescription().utf8_str(), document.GetAllocator());
    event.AddMember("os_name", os_name, document.GetAllocator());

    Value language(Option::instance().getLanguageCode().utf8_str(), document.GetAllocator());
    event.AddMember("language", language, document.GetAllocator());

    Value version_name(mmex::version::string.utf8_str(), document.GetAllocator());
    event.AddMember("version_name", version_name, document.GetAllocator());

    Value session_id(wxString::Format("%lld", this->m_start.GetTicks()).utf8_str(), document.GetAllocator());
    event.AddMember("session_id", session_id, document.GetAllocator());

    Value event_properties(kObjectType);
    Value page_title(documentTitle.utf8_str(), document.GetAllocator());
    event_properties.AddMember("page_title", page_title, document.GetAllocator());

    Value page_path(documentPath.utf8_str(), document.GetAllocator());
    event_properties.AddMember("page_path", page_path, document.GetAllocator());

    event_properties.AddMember("plt", Value(static_cast<int>(plt)), document.GetAllocator());

    event.AddMember("event_properties", event_properties, document.GetAllocator());

    events.PushBack(event, document.GetAllocator());
    document.AddMember("events", events, document.GetAllocator());

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    document.Accept(writer);

    // Spawn thread to send statistics
    SendStatsThread* thread = new SendStatsThread(url, wxString::FromUTF8(buffer.GetString()));
    thread->Run();
}

extern WXDLLIMPEXP_DATA_WEBVIEW(const char) wxWebViewBackendDefault[];
wxThread::ExitCode SendStatsThread::Entry()
{
    wxLogDebug("Sending stats (thread %lu, priority %u, %s, %i cores): %s, payload %s",
        GetId(), GetPriority(), wxGetOsDescription(), GetCPUCount(), m_url, m_payload);
    wxString result = wxEmptyString;
    if (this->m_payload.IsEmpty())
        http_get_data(m_url, result, wxString::Format("%s/%s (%s; %s) %s", mmex::getProgramName(), mmex::version::string, wxPlatformInfo::Get().GetOperatingSystemFamilyName(), wxGetOsDescription(), wxWebViewBackendDefault));
    else
        http_post_data(m_url, m_payload, "Content-Type: application/json", result);
    wxLogDebug("Response: %s", result);
    return nullptr;
}
