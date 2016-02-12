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
#include "paths.h"
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

void Model_Usage::append(const json::Object& o)
{
    this->a.Insert(o);
}

void Model_Usage::append_cache_usage(const json::Object& o)
{
    this->m_cache.Insert(o);
}

std::wstring Model_Usage::to_string() const
{
    json::Object o;
    o[L"start"] = json::String(m_start.FormatISOCombined(' ').ToStdWstring());
    o[L"end"] = json::String(wxDateTime::Now().FormatISOCombined(' ').ToStdWstring());
    o[L"usage"] = a;
    o[L"cache"] = m_cache;

    std::wstringstream ss;
    json::Writer::Write(o, ss);
    return ss.str();
}

wxString uuid()
{
    wxString UUID = Model_Setting::instance().GetStringSetting("UUID", wxEmptyString);
    if (UUID == wxEmptyString || UUID.length() < wxString("mac_20140428075834").length())
    {
        wxDateTime now = wxDateTime::Now();
        UUID = wxString::Format("%s_%s", wxPlatformInfo::Get().GetPortIdShortName(), now.Format("%Y%m%d%H%M%S"));
        Model_Setting::instance().Set("UUID", UUID);
    }
    return UUID;
}

bool Model_Usage::send()
{
    int last_sent = Model_Setting::instance().GetIntSetting("LAST_SENT", 0);
    for (const auto & i : Model_Usage::instance().find(USAGEID(last_sent, GREATER)))
    {
        if (send(i))
            Model_Setting::instance().Set("LAST_SENT", i.id());
        else
            break;
    }

    return true;
}

bool Model_Usage::send(const Data* r)
{
    wxString url = mmex::weblink::UsageStats;
    url += "?";

    //UUID
    url += wxString::Format("User_ID=%s", uuid());

    //Version
    url += "&";
    url += wxString::Format("Version=%s", mmex::version::string);
    if (mmex::isPortableMode())
        url += " Portable";

    //Platform
    url += "&";
    url += wxString::Format("Platform=%s", wxPlatformInfo::Get().GetPortIdShortName());

    //Operating System
    url += "&";
    url += wxString::Format("OperatingSystem=%s", wxGetOsDescription());

    //Language
    wxString Language = Model_Setting::instance().GetStringSetting(LANGUAGE_PARAMETER, "english");
    if (Language.IsEmpty())
        Language = "english";
    url += "&";
    url += wxString::Format("Language=%s", Language);

    //Country
    std::locale userLocale("");
    wxString Country = userLocale.name();
    /* Above function works on Windows only:
       for other platforms is send an empty string and country is obtained from IP Address by webservice */
    if (wxPlatformInfo::Get().GetPortIdShortName() == "msw")
        Country = Country.SubString(Country.Find("_") + 1, Country.Find(".") - 1);
    else
        Country = wxEmptyString;

    url += "&";
    url += wxString::Format("Country=%s", Country);

    //Resolution
    wxSize Resolution = wxGetDisplaySize();
    url += "&";
    url += wxString::Format("Resolution=%ix%i", Resolution.GetX(), Resolution.GetY());

    //Start & End time
    std::wstringstream ss;
    ss << r->JSONCONTENT.ToStdWstring();
    json::Object o;
    json::Reader::Read(o, ss);

    url += "&";
    url += wxString::Format("Start_Time=%s", wxString(json::String(o[L"start"])));
    url += "&";
    url += wxString::Format("End_Time=%s", wxString(json::String(o[L"end"])));

    wxLogDebug("%s", url);
    wxString dummy;
    int sendResult = site_content(url,  dummy);
    wxLogDebug("%s", dummy);
    
    if (sendResult == wxURL_NOERR)
        return true;
    else
        return false;
}

bool Model_Usage::send(const Data& r)
{
    return send(&r);
}
