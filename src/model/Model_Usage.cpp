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

#include "Model_Usage.h"
#include "Model_Setting.h"
#include "util.h"
#include "constants.h"
#include <wx/platinfo.h>
#include <wx/intl.h>

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
    ins.m_start = wxDateTime::Now();

    return ins;
}

/** Return the static instance of Model_Usage table */
Model_Usage& Model_Usage::instance()
{
    return Singleton<Model_Usage>::instance();
}

wxString Model_Usage::version()
{
    return "$Rev: 6165 $";
}

void Model_Usage::append(const json::Object& o)
{
    this->a.Insert(o);
}

void Model_Usage::append_cache_usage(const json::Object& o)
{
    this->m_cache.Insert(o);
}
wxString uuid();
std::string Model_Usage::to_string() const
{
    json::Object o;
    o["start"] = json::String(m_start.FormatISOCombined().ToStdString());
    o["end"] = json::String(wxDateTime::Now().FormatISOCombined().ToStdString());
    o["usage"] = a;
    o["cache"] = m_cache;

    std::stringstream ss;
    json::Writer::Write(o, ss);
    return ss.str();
}

wxString uuid()
{
    Model_Setting::Data* uuid = Model_Setting::instance().get("UUID");
    if (uuid)
        return uuid->SETTINGVALUE;
    
    // TODO better logic
    wxString UUID = wxString::Format("%s_%ld", wxPlatformInfo::Get().GetPortIdShortName(), wxGetUTCTimeMillis().ToLong());
    Model_Setting::instance().Set("UUID", UUID);
    return UUID;
}

bool Model_Usage::send()
{
    int last_sent = Model_Setting::instance().GetIntSetting("LAST_SENT", 0);
    for (const auto & i : Model_Usage::instance().find(USAGEID(last_sent, GREATER)))
    {
        if (send(i)) Model_Setting::instance().Set("LAST_SENT", i.id());
    }

    return true;
}

bool Model_Usage::send(const Data* r)
{
    wxString url = "http://hosting.villanet.it/MMEX_Stats/main_stats_v1.php?";
    url += wxString::Format("User_ID=%s", uuid()); // uuid
    url += "&";
    url += wxString::Format("Version=%s", mmex::getProgramVersion());
    url += "&";
    url += wxString::Format("Platform=%s", wxPlatformInfo::Get().GetPortIdName());
    url += "&";
    url += wxString::Format("Language=%s", Model_Setting::instance().GetStringSetting(LANGUAGE_PARAMETER, "english"));
    url += "&";
    url += wxString::Format("Nation=%s", "USA"); //FIXME

    std::stringstream ss;
    ss << r->JSONCONTENT.ToStdString();
    json::Object o;
    json::Reader::Read(o, ss);

    url += "&";
    url += wxString::Format("Start_Time=%s", wxString(json::String(o["start"])));
    url += "&";
    url += wxString::Format("End_Time=%s", wxString(json::String(o["end"])));

    wxLogDebug(url);
    wxString dummy;
    site_content(url,  dummy);
    wxLogDebug(dummy);
    
    return true;
}

bool Model_Usage::send(const Data& r)
{
    return send(&r);
}
