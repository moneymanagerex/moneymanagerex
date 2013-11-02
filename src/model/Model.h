/*******************************************************
 Copyright (C) 2013 Guan Lisheng

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

#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <map>
#include <algorithm>
#include <wx/datetime.h>
#include "singleton.h"
#include <wx/sharedptr.h>
#include <wx/log.h>
#include <wx/string.h>

class wxString;
class wxSQLite3Statement;
class wxSQLite3Database;
class wxSQLite3ResultSet;

typedef wxDateTime wxDate;

class Model
{
public:
    Model():db_(0) {};
    virtual ~Model() {};

public:
    void Begin()
    {
        this->db_->Begin();
    }
    void Commit()
    {
        this->db_->Commit();
    }
    void Rollback()
    {
        this->db_->Rollback();
    }
protected:
    static wxDate to_date(const wxString& str_date)
    {
        wxDate date = wxDateTime::Today();
        date.ParseISODate(str_date); // the date in ISO 8601 format "YYYY-MM-DD".
        return date;
    }
protected:
    wxSQLite3Database* db_;
};
#endif // 
