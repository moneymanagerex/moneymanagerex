﻿// -*- C++ -*-
/** @file
 * @brief     CRUD implementation for TRANSLINK SQLite table
 * @warning   Auto generated with sqlite2cpp.py script. DO NOT EDIT!
 * @copyright © 2013-2018 Guan Lisheng
 * @copyright © 2017-2018 Stefano Giorgio
 * @author    Guan Lisheng (guanlisheng@gmail.com)
 * @author    Stefano Giorgio (stef145g)
 * @author    Tomasz Słodkowicz
 * @date      2018-10-07 02:45:31.001407
 */
#pragma once

#include "Table.h"

struct DB_Table_TRANSLINK : public DB_Table
{
    struct Data;
    typedef DB_Table_TRANSLINK Self;

    /** A container to hold list of Data records for the table*/
    struct Data_Set : public std::vector<Self::Data>
    {
        /** Return the data records as a json array string */
        wxString to_json() const
        {
            StringBuffer json_buffer;
            PrettyWriter<StringBuffer> json_writer(json_buffer);

            json_writer.StartArray();
            for (const auto & item: *this)
            {
                json_writer.StartObject();
                item.as_json(json_writer);
                json_writer.EndObject();
            }
            json_writer.EndArray();

            return json_buffer.GetString();
        }
    };

    /** A container to hold a list of Data record pointers for the table in memory*/
    typedef std::vector<Self::Data*> Cache;
    typedef std::map<int, Self::Data*> Index_By_Id;
    Cache cache_;
    Index_By_Id index_by_id_;
    Data* fake_; // in case the entity not found

    /** Destructor: clears any data records stored in memory */
    ~DB_Table_TRANSLINK()
    {
        delete this->fake_;
        destroy_cache();
    }

    /** Removes all records stored in memory (cache) for the table*/
    void destroy_cache()
    {
        std::for_each(cache_.begin(), cache_.end(), std::mem_fun(&Data::destroy));
        cache_.clear();
        index_by_id_.clear(); // no memory release since it just stores pointer and the according objects are in cache
    }

    /** Creates the database table if the table does not exist*/
    bool ensure(wxSQLite3Database* db)
    {
        if (!exists(db))
        {
            try
            {
                db->ExecuteUpdate("CREATE TABLE TRANSLINK (TRANSLINKID  integer NOT NULL primary key, CHECKINGACCOUNTID integer NOT NULL, LINKTYPE TEXT NOT NULL /* Asset, Stock */, LINKRECORDID integer NOT NULL)");
                this->ensure_data(db);
            }
            catch(const wxSQLite3Exception &e)
            {
                wxLogError("TRANSLINK: Exception %s", e.GetMessage().c_str());
                return false;
            }
        }

        this->ensure_index(db);

        return true;
    }

    bool ensure_index(wxSQLite3Database* db)
    {
        try
        {
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_CHECKINGACCOUNT ON TRANSLINK (CHECKINGACCOUNTID)");
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_LINKRECORD ON TRANSLINK (LINKTYPE, LINKRECORDID)");
        }
        catch(const wxSQLite3Exception &e)
        {
            wxLogError("TRANSLINK: Exception %s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }

    void ensure_data(wxSQLite3Database* db)
    {
        db->Begin();
        db->Commit();
    }

    struct TRANSLINKID : public DB_Column<int>
    {
        static wxString name() { return "TRANSLINKID"; }
        explicit TRANSLINKID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };

    struct CHECKINGACCOUNTID : public DB_Column<int>
    {
        static wxString name() { return "CHECKINGACCOUNTID"; }
        explicit CHECKINGACCOUNTID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };

    struct LINKTYPE : public DB_Column<wxString>
    {
        static wxString name() { return "LINKTYPE"; }
        explicit LINKTYPE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };

    struct LINKRECORDID : public DB_Column<int>
    {
        static wxString name() { return "LINKRECORDID"; }
        explicit LINKRECORDID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };

    typedef TRANSLINKID PRIMARY;
    enum COLUMN
    {
        COL_TRANSLINKID = 0
        , COL_CHECKINGACCOUNTID = 1
        , COL_LINKTYPE = 2
        , COL_LINKRECORDID = 3
        , COL_UNKNOWN = -1
    };

    /** Returns the column name as a string*/
    static wxString column_to_name(COLUMN col)
    {
        switch(col)
        {
            case COL_TRANSLINKID: return "TRANSLINKID";
            case COL_CHECKINGACCOUNTID: return "CHECKINGACCOUNTID";
            case COL_LINKTYPE: return "LINKTYPE";
            case COL_LINKRECORDID: return "LINKRECORDID";
            default: break;
        }

        return "UNKNOWN";
    }

    /** Returns the column number from the given column name*/
    static COLUMN name_to_column(const wxString& name)
    {
        if ("TRANSLINKID" == name) return COL_TRANSLINKID;
        else if ("CHECKINGACCOUNTID" == name) return COL_CHECKINGACCOUNTID;
        else if ("LINKTYPE" == name) return COL_LINKTYPE;
        else if ("LINKRECORDID" == name) return COL_LINKRECORDID;

        return COL_UNKNOWN;
    }

    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_TRANSLINK;
        /** This is a instance pointer to itself in memory. */
        Self* table_;

        int TRANSLINKID; // primary key
        int CHECKINGACCOUNTID;
        wxString LINKTYPE;
        int LINKRECORDID;

        int id() const
        {
            return TRANSLINKID;
        }

        void id(int id)
        {
            TRANSLINKID = id;
        }

        bool operator < (const Data& r) const
        {
            return this->id() < r.id();
        }

        bool operator < (const Data* r) const
        {
            return this->id() < r->id();
        }

        explicit Data(Self* table = 0)
        {
            table_ = table;

            TRANSLINKID = -1;
            CHECKINGACCOUNTID = -1;
            LINKRECORDID = -1;
        }

        explicit Data(wxSQLite3ResultSet& q, Self* table = 0)
        {
            table_ = table;

            TRANSLINKID = q.GetInt(0);
            CHECKINGACCOUNTID = q.GetInt(1);
            LINKTYPE = q.GetString(2);
            LINKRECORDID = q.GetInt(3);
        }

        Data& operator=(const Data& other)
        {
            if (this == &other) return *this;

            TRANSLINKID = other.TRANSLINKID;
            CHECKINGACCOUNTID = other.CHECKINGACCOUNTID;
            LINKTYPE = other.LINKTYPE;
            LINKRECORDID = other.LINKRECORDID;
            return *this;
        }


        bool match(const Self::TRANSLINKID &in) const
        {
            return this->TRANSLINKID == in.v_;
        }

        bool match(const Self::CHECKINGACCOUNTID &in) const
        {
            return this->CHECKINGACCOUNTID == in.v_;
        }

        bool match(const Self::LINKTYPE &in) const
        {
            return this->LINKTYPE.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::LINKRECORDID &in) const
        {
            return this->LINKRECORDID == in.v_;
        }

        /** Return the data record as a json string */
        wxString to_json() const
        {
            StringBuffer json_buffer;
            PrettyWriter<StringBuffer> json_writer(json_buffer);

            json_writer.StartObject();
            this->as_json(json_writer);
            json_writer.EndObject();

            return json_buffer.GetString();
        }

        /** Add the field data as json key:value pairs */
        void as_json(PrettyWriter<StringBuffer>& json_writer) const
        {
            json_writer.Key("TRANSLINKID");
            json_writer.Int(this->TRANSLINKID);
            json_writer.Key("CHECKINGACCOUNTID");
            json_writer.Int(this->CHECKINGACCOUNTID);
            json_writer.Key("LINKTYPE");
            json_writer.String(this->LINKTYPE.c_str());
            json_writer.Key("LINKRECORDID");
            json_writer.Int(this->LINKRECORDID);
        }

        row_t to_row_t() const
        {
            row_t row;
            row(L"TRANSLINKID") = TRANSLINKID;
            row(L"CHECKINGACCOUNTID") = CHECKINGACCOUNTID;
            row(L"LINKTYPE") = LINKTYPE;
            row(L"LINKRECORDID") = LINKRECORDID;
            return row;
        }

        void to_template(html_template& t) const
        {
            t(L"TRANSLINKID") = TRANSLINKID;
            t(L"CHECKINGACCOUNTID") = CHECKINGACCOUNTID;
            t(L"LINKTYPE") = LINKTYPE;
            t(L"LINKRECORDID") = LINKRECORDID;
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db)
        {
            if (db && db->IsReadOnly()) return false;
            if (!table_ || !db)
            {
                wxLogError("can not save TRANSLINK");
                return false;
            }

            return table_->save(this, db);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!table_ || !db)
            {
                wxLogError("can not remove TRANSLINK");
                return false;
            }

            return table_->remove(this, db);
        }

        void destroy()
        {
            delete this;
        }
    };

    enum
    {
        NUM_COLUMNS = 4
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    /** Name of the table */
    wxString name() const { return "TRANSLINK"; }

    DB_Table_TRANSLINK() : fake_(new Data())
    {
        query_ = "SELECT TRANSLINKID, CHECKINGACCOUNTID, LINKTYPE, LINKRECORDID FROM TRANSLINK ";
    }

    /** Create a new Data record and add to memory table (cache) */
    Self::Data* create()
    {
        Self::Data* entity = new Self::Data(this);
        cache_.push_back(entity);
        return entity;
    }

    /** Create a copy of the Data record and add to memory table (cache) */
    Self::Data* clone(const Data* e)
    {
        Self::Data* entity = create();
        *entity = *e;
        entity->id(-1);
        return entity;
    }

    /**
    * Saves the Data record to the database table.
    * Either create a new record or update the existing record.
    * Remove old record from the memory table (cache)
    */
    bool save(Self::Data* entity, wxSQLite3Database* db)
    {
        wxString sql = wxEmptyString;
        if (entity->id() <= 0) //  new & insert
        {
            sql = "INSERT INTO TRANSLINK(CHECKINGACCOUNTID, LINKTYPE, LINKRECORDID) VALUES(?, ?, ?)";
        }
        else
        {
            sql = "UPDATE TRANSLINK SET CHECKINGACCOUNTID = ?, LINKTYPE = ?, LINKRECORDID = ? WHERE TRANSLINKID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->CHECKINGACCOUNTID);
            stmt.Bind(2, entity->LINKTYPE);
            stmt.Bind(3, entity->LINKRECORDID);
            if (entity->id() > 0)
                stmt.Bind(4, entity->TRANSLINKID);

            stmt.ExecuteUpdate();
            stmt.Finalize();

            if (entity->id() > 0) // existent
            {
                for(Cache::iterator it = cache_.begin(); it != cache_.end(); ++ it)
                {
                    Self::Data* e = *it;
                    if (e->id() == entity->id())
                        *e = *entity;  // in-place update
                }
            }
        }
        catch(const wxSQLite3Exception &e)
        {
            wxLogError("TRANSLINK: Exception %s, %s", e.GetMessage().c_str(), entity->to_json());
            return false;
        }

        if (entity->id() <= 0)
        {
            entity->id((db->GetLastRowId()).ToLong());
            index_by_id_.insert(std::make_pair(entity->id(), entity));
        }
        return true;
    }

    /** Remove the Data record from the database and the memory table (cache) */
    bool remove(int id, wxSQLite3Database* db)
    {
        if (id <= 0) return false;
        try
        {
            wxString sql = "DELETE FROM TRANSLINK WHERE TRANSLINKID = ?";
            wxSQLite3Statement stmt = db->PrepareStatement(sql);
            stmt.Bind(1, id);
            stmt.ExecuteUpdate();
            stmt.Finalize();

            Cache c;
            for(Cache::iterator it = cache_.begin(); it != cache_.end(); ++ it)
            {
                Self::Data* entity = *it;
                if (entity->id() == id)
                {
                    index_by_id_.erase(entity->id());
                    delete entity;
                }
                else
                {
                    c.push_back(entity);
                }
            }
            cache_.clear();
            cache_.swap(c);
        }
        catch(const wxSQLite3Exception &e)
        {
            wxLogError("TRANSLINK: Exception %s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }

    /** Remove the Data record from the database and the memory table (cache) */
    bool remove(Self::Data* entity, wxSQLite3Database* db)
    {
        if (remove(entity->id(), db))
        {
            entity->id(-1);
            return true;
        }

        return false;
    }

    template<typename... Args>
    Self::Data* get_one(const Args& ... args)
    {
        for (Index_By_Id::iterator it = index_by_id_.begin(); it != index_by_id_.end(); ++ it)
        {
            Self::Data* item = it->second;
            if (item->id() > 0 && match(item, args...))
            {
                ++ hit_;
                return item;
            }
        }

        ++ miss_;

        return 0;
    }

    /**
    * Search the memory table (Cache) for the data record.
    * If not found in memory, search the database and update the cache.
    */
    Self::Data* get(int id, wxSQLite3Database* db)
    {
        if (id <= 0)
        {
            ++ skip_;
            return 0;
        }

        Index_By_Id::iterator it = index_by_id_.find(id);
        if (it != index_by_id_.end())
        {
            ++ hit_;
            return it->second;
        }

        ++ miss_;
        Self::Data* entity = 0;
        wxString where = wxString::Format(" WHERE %s = ?", PRIMARY::name().c_str());
        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(this->query() + where);
            stmt.Bind(1, id);

            wxSQLite3ResultSet q = stmt.ExecuteQuery();
            if(q.NextRow())
            {
                entity = new Self::Data(q, this);
                cache_.push_back(entity);
                index_by_id_.insert(std::make_pair(id, entity));
            }
            stmt.Finalize();
        }
        catch(const wxSQLite3Exception &e)
        {
            wxLogError("%s: Exception %s", this->name().c_str(), e.GetMessage().c_str());
        }

        if (!entity)
        {
            entity = this->fake_;
            // wxLogError("%s: %d not found", this->name().c_str(), id);
        }

        return entity;
    }

    /**
    * Return a list of Data records (Data_Set) derived directly from the database.
    * The Data_Set is sorted based on the column number.
    */
    const Data_Set all(wxSQLite3Database* db, COLUMN col = COLUMN(0), bool asc = true)
    {
        Data_Set result;
        try
        {
            wxSQLite3ResultSet q = db->ExecuteQuery(col == COLUMN(0) ? this->query() : this->query() + " ORDER BY " + column_to_name(col) + " COLLATE NOCASE " + (asc ? " ASC " : " DESC "));

            while(q.NextRow())
            {
                Self::Data entity(q, this);
                result.push_back(std::move(entity));
            }

            q.Finalize();
        }
        catch(const wxSQLite3Exception &e)
        {
            wxLogError("%s: Exception %s", this->name().c_str(), e.GetMessage().c_str());
        }

        return result;
    }
};

