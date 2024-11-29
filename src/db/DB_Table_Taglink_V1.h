﻿// -*- C++ -*-
//=============================================================================
/**
 *      Copyright: (c) 2013 - 2024 Guan Lisheng (guanlisheng@gmail.com)
 *      Copyright: (c) 2017 - 2018 Stefano Giorgio (stef145g)
 *      Copyright: (c) 2022 Mark Whalley (mark@ipx.co.uk)
 *
 *      @file
 *
 *      @author [sqlite2cpp.py]
 *
 *      @brief
 *
 *      Revision History:
 *          AUTO GENERATED at 2024-11-29 17:46:15.270729.
 *          DO NOT EDIT!
 */
//=============================================================================
#pragma once

#include "DB_Table.h"

struct DB_Table_TAGLINK_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_TAGLINK_V1 Self;

    /** A container to hold list of Data records for the table*/
    struct Data_Set : public std::vector<Self::Data>
    {
        /**Return the data records as a json array string */
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
    typedef std::map<int64, Self::Data*> Index_By_Id;
    Cache cache_;
    Index_By_Id index_by_id_;
    Data* fake_; // in case the entity not found

    /** Destructor: clears any data records stored in memory */
    ~DB_Table_TAGLINK_V1() 
    {
        delete this->fake_;
        destroy_cache();
    }
     
    /** Removes all records stored in memory (cache) for the table*/ 
    void destroy_cache()
    {
        std::for_each(cache_.begin(), cache_.end(), std::mem_fn(&Data::destroy));
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
                db->ExecuteUpdate("CREATE TABLE TAGLINK_V1(TAGLINKID INTEGER PRIMARY KEY, REFTYPE TEXT NOT NULL, REFID INTEGER NOT NULL, TAGID INTEGER NOT NULL, FOREIGN KEY (TAGID) REFERENCES TAG_V1 (TAGID), UNIQUE(REFTYPE, REFID, TAGID))");
                this->ensure_data(db);
            }
            catch(const wxSQLite3Exception &e) 
            { 
                wxLogError("TAGLINK_V1: Exception %s", e.GetMessage().utf8_str());
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
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_TAGLINK ON TAGLINK_V1 (REFTYPE, REFID, TAGID)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("TAGLINK_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }

        return true;
    }

    void ensure_data(wxSQLite3Database* db)
    {
        db->Begin();
        db->Commit();
    }
    
    struct TAGLINKID : public DB_Column<int64>
    { 
        static wxString name() { return "TAGLINKID"; } 
        explicit TAGLINKID(const int64 &v, OP op = EQUAL): DB_Column<int64>(v, op) {}
    };
    
    struct REFTYPE : public DB_Column<wxString>
    { 
        static wxString name() { return "REFTYPE"; } 
        explicit REFTYPE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct REFID : public DB_Column<int64>
    { 
        static wxString name() { return "REFID"; } 
        explicit REFID(const int64 &v, OP op = EQUAL): DB_Column<int64>(v, op) {}
    };
    
    struct TAGID : public DB_Column<int64>
    { 
        static wxString name() { return "TAGID"; } 
        explicit TAGID(const int64 &v, OP op = EQUAL): DB_Column<int64>(v, op) {}
    };
    
    typedef TAGLINKID PRIMARY;
    enum COLUMN
    {
        COL_TAGLINKID = 0
        , COL_REFTYPE = 1
        , COL_REFID = 2
        , COL_TAGID = 3
    };

    /** Returns the column name as a string*/
    static wxString column_to_name(const COLUMN col)
    {
        switch(col)
        {
            case COL_TAGLINKID: return "TAGLINKID";
            case COL_REFTYPE: return "REFTYPE";
            case COL_REFID: return "REFID";
            case COL_TAGID: return "TAGID";
            default: break;
        }
        
        return "UNKNOWN";
    }

    /** Returns the column number from the given column name*/
    static COLUMN name_to_column(const wxString& name)
    {
        if ("TAGLINKID" == name) return COL_TAGLINKID;
        else if ("REFTYPE" == name) return COL_REFTYPE;
        else if ("REFID" == name) return COL_REFID;
        else if ("TAGID" == name) return COL_TAGID;

        return COLUMN(-1);
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_TAGLINK_V1;
        /** This is a instance pointer to itself in memory. */
        Self* table_;
    
        int64 TAGLINKID;//  primary key
        wxString REFTYPE;
        int64 REFID;
        int64 TAGID;

        int64 id() const
        {
            return TAGLINKID;
        }

        void id(const int64 id)
        {
            TAGLINKID = id;
        }

        bool operator < (const Data& r) const
        {
            return this->id() < r.id();
        }
        
        bool operator < (const Data* r) const
        {
            return this->id() < r->id();
        }

        bool equals(const Data* r) const
        {
            if(TAGLINKID != r->TAGLINKID) return false;
            if(!REFTYPE.IsSameAs(r->REFTYPE)) return false;
            if(REFID != r->REFID) return false;
            if(TAGID != r->TAGID) return false;
            return true;
        }
        
        explicit Data(Self* table = nullptr ) 
        {
            table_ = table;
        
            TAGLINKID = -1;
            REFID = -1;
            TAGID = -1;
        }

        explicit Data(wxSQLite3ResultSet& q, Self* table = nullptr )
        {
            table_ = table;
        
            TAGLINKID = q.GetInt64(0); // TAGLINKID
            REFTYPE = q.GetString(1); // REFTYPE
            REFID = q.GetInt64(2); // REFID
            TAGID = q.GetInt64(3); // TAGID
        }

        Data& operator=(const Data& other)
        {
            if (this == &other) return *this;

            TAGLINKID = other.TAGLINKID;
            REFTYPE = other.REFTYPE;
            REFID = other.REFID;
            TAGID = other.TAGID;
            return *this;
        }

        template<typename C>
        bool match(const C &c) const
        {
            return false;
        }

        bool match(const Self::TAGLINKID &in) const
        {
            return this->TAGLINKID == in.v_;
        }

        bool match(const Self::REFTYPE &in) const
        {
            return this->REFTYPE.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::REFID &in) const
        {
            return this->REFID == in.v_;
        }

        bool match(const Self::TAGID &in) const
        {
            return this->TAGID == in.v_;
        }

        // Return the data record as a json string
        wxString to_json() const
        {
            StringBuffer json_buffer;
            PrettyWriter<StringBuffer> json_writer(json_buffer);

			json_writer.StartObject();			
			this->as_json(json_writer);
            json_writer.EndObject();

            return json_buffer.GetString();
        }

        // Add the field data as json key:value pairs
        void as_json(PrettyWriter<StringBuffer>& json_writer) const
        {
            json_writer.Key("TAGLINKID");
            json_writer.Int(this->TAGLINKID);
            json_writer.Key("REFTYPE");
            json_writer.String(this->REFTYPE.utf8_str());
            json_writer.Key("REFID");
            json_writer.Int(this->REFID);
            json_writer.Key("TAGID");
            json_writer.Int(this->TAGID);
        }

        row_t to_row_t() const
        {
            row_t row;
            row(L"TAGLINKID") = TAGLINKID;
            row(L"REFTYPE") = REFTYPE;
            row(L"REFID") = REFID;
            row(L"TAGID") = TAGID;
            return row;
        }

        void to_template(html_template& t) const
        {
            t(L"TAGLINKID") = TAGLINKID;
            t(L"REFTYPE") = REFTYPE;
            t(L"REFID") = REFID;
            t(L"TAGID") = TAGID;
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db)
        {
            if (db && db->IsReadOnly()) return false;
            if (!table_ || !db) 
            {
                wxLogError("can not save TAGLINK_V1");
                return false;
            }

            return table_->save(this, db);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!table_ || !db) 
            {
                wxLogError("can not remove TAGLINK_V1");
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

    /** Name of the table*/    
    wxString name() const { return "TAGLINK_V1"; }

    DB_Table_TAGLINK_V1() : fake_(new Data())
    {
        query_ = "SELECT TAGLINKID, REFTYPE, REFID, TAGID FROM TAGLINK_V1 ";
    }

    /** Create a new Data record and add to memory table (cache)*/
    Self::Data* create()
    {
        Self::Data* entity = new Self::Data(this);
        cache_.push_back(entity);
        return entity;
    }
    
    /** Create a copy of the Data record and add to memory table (cache)*/
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
            sql = "INSERT INTO TAGLINK_V1(REFTYPE, REFID, TAGID) VALUES(?, ?, ?)";
        }
        else
        {
            sql = "UPDATE TAGLINK_V1 SET REFTYPE = ?, REFID = ?, TAGID = ? WHERE TAGLINKID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->REFTYPE);
            stmt.Bind(2, entity->REFID);
            stmt.Bind(3, entity->TAGID);
            if (entity->id() > 0)
                stmt.Bind(4, entity->TAGLINKID);

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
            wxLogError("TAGLINK_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
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
    bool remove(const int id, wxSQLite3Database* db)
    {
        if (id <= 0) return false;
        try
        {
            wxString sql = "DELETE FROM TAGLINK_V1 WHERE TAGLINKID = ?";
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
            wxLogError("TAGLINK_V1: Exception %s", e.GetMessage().utf8_str());
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
    Self::Data* get(const int id, wxSQLite3Database* db)
    {
        if (id <= 0) 
        {
            ++ skip_;
            return nullptr;
        }

        Index_By_Id::iterator it = index_by_id_.find(id);
        if (it != index_by_id_.end())
        {
            ++ hit_;
            return it->second;
        }
        
        ++ miss_;
        Self::Data* entity = nullptr;
        wxString where = wxString::Format(" WHERE %s = ?", PRIMARY::name().utf8_str());
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
            wxLogError("%s: Exception %s", this->name().utf8_str(), e.GetMessage().utf8_str());
        }
        
        if (!entity) 
        {
            entity = this->fake_;
            // wxLogError("%s: %d not found", this->name().utf8_str(), id);
        }
 
        return entity;
    }
    /**
    * Search the database for the data record, bypassing the cache.
    */
    Self::Data* get_record(const int id, wxSQLite3Database* db)
    {
        if (id <= 0) 
        {
            ++ skip_;
            return nullptr;
        }

        Self::Data* entity = nullptr;
        wxString where = wxString::Format(" WHERE %s = ?", PRIMARY::name().utf8_str());
        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(this->query() + where);
            stmt.Bind(1, id);

            wxSQLite3ResultSet q = stmt.ExecuteQuery();
            if(q.NextRow())
            {
                entity = new Self::Data(q, this);
            }
            stmt.Finalize();
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("%s: Exception %s", this->name().utf8_str(), e.GetMessage().utf8_str());
        }
        
        if (!entity) 
        {
            entity = this->fake_;
            // wxLogError("%s: %d not found", this->name().utf8_str(), id);
        }
 
        return entity;
    }

    /**
    * Return a list of Data records (Data_Set) derived directly from the database.
    * The Data_Set is sorted based on the column number.
    */
    const Data_Set all(wxSQLite3Database* db, const COLUMN col = COLUMN(0), const bool asc = true)
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
            wxLogError("%s: Exception %s", this->name().utf8_str(), e.GetMessage().utf8_str());
        }

        return result;
    }
};

