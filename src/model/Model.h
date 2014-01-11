/*******************************************************
 Copyright (C) 2013 Guan Lisheng (guanlisheng@gmail.com)

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

#ifdef _MSC_VER
#pragma warning (disable:4100)
#endif

#include <vector>
#include <map>
#include <algorithm>
#include <wx/datetime.h>
#include "singleton.h"
#include <wx/sharedptr.h>
#include <wx/log.h>
#include <wx/string.h>
#include "db/DB_Table.h"

class wxString;
class wxSQLite3Statement;
class wxSQLite3Database;
class wxSQLite3ResultSet;

typedef wxDateTime wxDate;

class ModelBase
{
public:
    ModelBase():db_(0) {};
    virtual ~ModelBase() {};

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

template<class DB_TABLE>
class Model: public ModelBase, public DB_TABLE
{
public:
    using DB_TABLE::all;
    using DB_TABLE::get;
    using DB_TABLE::save;
    using DB_TABLE::remove;
    typedef typename DB_TABLE::COLUMN COLUMN;
    /** Return a list of Data records (Data_Set) derived directly from the database. */
    typename DB_TABLE::Data_Set all(COLUMN col = COLUMN(0), bool asc = true)
    {
        this->ensure(this->db_);
        return all(db_, col, asc);
    }

    template<typename... Args>
    /** Args: Specialised parameters used in SQL statements after the WHERE statement */
    typename DB_TABLE::Data_Set find(const Args&... args)
    {
        return find_by(this, db_, true, args...);
    }
    template<typename... Args>
    typename DB_TABLE::Data_Set find_or(const Args&... args)
    {
        return find_by(this, db_, false, args...);
    }

    /** Return the Data record instance for the given ID*/
    typename DB_TABLE::Data* get(int id)
    {
        return this->get(id, this->db_);
    }

    /** Save the Data record instance in memory to the database. */
    int save(typename DB_TABLE::Data* r)
    {
        r->save(this->db_);
        return r->id();
    }
    /**
    * Save the all the Data record instances in memory to the database
    * for the record list (Data_Set).
    */
    template<class DATA_SET>
    int save(DATA_SET& rows)
    {
        this->Begin();
        for (auto& r : rows) this->save(&r);
        this->Commit();

        return rows.size();
    }

    /** Remove the Data record instance from memory and the database. */
    bool remove(int id)
    {
        return this->remove(id, db_);
    }
};

#endif // 
