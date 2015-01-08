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
    void Savepoint()
    {
        this->db_->Savepoint("MMEX");
    }
    void ReleaseSavepoint()
    {
        this->db_->ReleaseSavepoint("MMEX");
    }
    void Rollback()
    {
        this->db_->Rollback("MMEX");
    }
protected:
    static wxDate to_date(const wxString& str_date)
    {
        static std::map<wxString, wxDate> cache;
        const auto it = cache.find(str_date);
        if (it != cache.end()) return it->second;

        wxDate date = wxDateTime::Today();
        date.ParseISODate(str_date); // the date in ISO 8601 format "YYYY-MM-DD".
        cache.insert(std::make_pair(str_date, date));
        return date;
    }
public:
    virtual json::Object cache_to_json() const = 0;
    virtual void show_statistics() const = 0;
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
    /** Return a list of Data record addresses (Data_Set) derived directly from the database. */
    const typename DB_TABLE::Data_Set all(COLUMN col = COLUMN(0), bool asc = true)
    {
        this->ensure(this->db_);
        return all(db_, col, asc);
    }

    template<typename... Args>
    /**
    Command: find(const Args&... args)
    Args: One or more Specialised Parameters creating SQL statement conditions used after the WHERE statement.
    Specialised Parameters: Table_Column_Name(content)[, Table_Column_Name(content)[, ...]]
    Example:
    Model_Asset::ASSETID(2), Model_Asset::ASSETTYPE(Model_Asset::TYPE_JEWELLERY)
    produces SQL statement condition: ASSETID = 2 AND ASSETTYPE = "Jewellery"
    * Returns a Data_Set containing the addresses of the items found.
    * The Data_Set is empty when nothing found.
    */
    const typename DB_TABLE::Data_Set find(const Args&... args)
    {
        return find_by(this, db_, true, args...);
    }

    template<typename... Args>
    /**
    Command: find_or(const Args&... args)
    Args: One or more Specialised Parameters creating SQL statement conditions used after the WHERE statement.
    Specialised Parameters: Table_Column_Name(content)[, Table_Column_Name(content)[, ...]]
    Example:
    Model_Asset::ASSETID(2), Model_Asset::ASSETTYPE(Model_Asset::TYPE_JEWELLERY)
    produces SQL statement condition: ASSETID = 2 OR ASSETTYPE = "Jewellery"
    * Returns a Data_Set containing the addresses of the items found.
    * The Data_Set is empty when nothing found.
    */
    const typename DB_TABLE::Data_Set find_or(const Args&... args)
    {
        return find_by(this, db_, false, args...);
    }

    /**
    * Return the Data record pointer for the given ID
    * from either memory cache or the database.
    */
    typename DB_TABLE::Data* get(int id)
    {
        return this->get(id, this->db_);
    }

    /** Save the Data record memory instance to the database. */
    int save(typename DB_TABLE::Data* r)
    {
        r->save(this->db_);
        return r->id();
    }

    /**
    * Save all Data record memory instances contained
    * in the record list (Data_Set) to the database.
    */
    template<class DATA>
    int save(std::vector<DATA>& rows)
    {
        this->Savepoint();
        for (auto& r : rows) 
        {
            if (r.id() < 0) 
                wxLogDebug("Incorrect function call to save %s", r.to_json().c_str());
            this->save(&r);
        }
        this->ReleaseSavepoint();

        return rows.size();
    }

    template<class DATA>
    int save(std::vector<DATA*>& rows)
    {
        this->Savepoint();
        for (auto& r : rows) this->save(r);
        this->ReleaseSavepoint();

        return rows.size();
    }

    /** Remove the Data record instance from memory and the database. */
    bool remove(int id)
    {
        return this->remove(id, db_);
    }

public:
    void preload(int max_num = 1000)
    {
        int i = 0;
        for (const auto & item : all())
        {
            get(item.id());
            if (++i >= max_num) break;
        }       
    }

    json::Object cache_to_json() const
    {
        json::Object o;
        o[L"table"] = json::String(this->name().ToStdWstring());
        o[L"cached"] = json::Number(this->cache_.size());
        o[L"index_by_id"] = json::Number(this->index_by_id_.size());
        o[L"hit"] = json::Number(this->hit_);
        o[L"miss"] = json::Number(this->miss_);
        o[L"skip"] = json::Number(this->skip_);

        return o;
    }
    /** Show table statistics*/
	void show_statistics() const
	{
		size_t cache_size = this->cache_.size();
        size_t index_by_id_size = this->index_by_id_.size();
#ifdef _WIN64
        wxLogDebug("%s : (cache %llu, index_by_id %llu, hit %llu, miss %llu, skip %llu)", this->name(), cache_size, index_by_id_size, this->hit_, this->miss_, this->skip_);
#else
        wxLogDebug("%s : (cache %lu, index_by_id %lu, hit %lu, miss %lu, skip %lu)", this->name(), cache_size, index_by_id_size, this->hit_, this->miss_, this->skip_);
#endif
	}
};

#endif // 
