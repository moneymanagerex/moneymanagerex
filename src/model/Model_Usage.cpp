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

void Model_Usage::append(json::Object& o)
{
    this->a.Insert(o);
}

std::string Model_Usage::to_string() const
{
    json::Object o;
    o["start"] = json::String(m_start.FormatISOCombined().ToStdString());
    o["end"] = json::String(wxDateTime::Now().FormatISOCombined().ToStdString());
    o["usage"] = a;
    std::stringstream ss;
    json::Writer::Write(o, ss);
    return ss.str();
}
