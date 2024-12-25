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
 *          AUTO GENERATED at 2024-12-25 12:56:03.116570.
 *          DO NOT EDIT!
 */
//=============================================================================
#pragma once

#include "DB_Table.h"

struct DB_Table_REPORT_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_REPORT_V1 Self;

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
    ~DB_Table_REPORT_V1() 
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
                db->ExecuteUpdate("CREATE TABLE REPORT_V1(REPORTID integer not null primary key, REPORTNAME TEXT COLLATE NOCASE NOT NULL UNIQUE, GROUPNAME TEXT COLLATE NOCASE, ACTIVE integer, SQLCONTENT TEXT, LUACONTENT TEXT, TEMPLATECONTENT TEXT, DESCRIPTION TEXT)");
                this->ensure_data(db);
            }
            catch(const wxSQLite3Exception &e) 
            { 
                wxLogError("REPORT_V1: Exception %s", e.GetMessage().utf8_str());
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
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS INDEX_REPORT_NAME ON REPORT_V1(REPORTNAME)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("REPORT_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }

        return true;
    }

    void ensure_data(wxSQLite3Database* db)
    {
        db->Begin();
        db->Commit();
    }
    
    struct REPORTID : public DB_Column<int64>
    { 
        static wxString name() { return "REPORTID"; } 
        explicit REPORTID(const int64 &v, OP op = EQUAL): DB_Column<int64>(v, op) {}
    };
    
    struct REPORTNAME : public DB_Column<wxString>
    { 
        static wxString name() { return "REPORTNAME"; } 
        explicit REPORTNAME(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct GROUPNAME : public DB_Column<wxString>
    { 
        static wxString name() { return "GROUPNAME"; } 
        explicit GROUPNAME(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct ACTIVE : public DB_Column<int64>
    { 
        static wxString name() { return "ACTIVE"; } 
        explicit ACTIVE(const int64 &v, OP op = EQUAL): DB_Column<int64>(v, op) {}
    };
    
    struct SQLCONTENT : public DB_Column<wxString>
    { 
        static wxString name() { return "SQLCONTENT"; } 
        explicit SQLCONTENT(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct LUACONTENT : public DB_Column<wxString>
    { 
        static wxString name() { return "LUACONTENT"; } 
        explicit LUACONTENT(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct TEMPLATECONTENT : public DB_Column<wxString>
    { 
        static wxString name() { return "TEMPLATECONTENT"; } 
        explicit TEMPLATECONTENT(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct DESCRIPTION : public DB_Column<wxString>
    { 
        static wxString name() { return "DESCRIPTION"; } 
        explicit DESCRIPTION(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    typedef REPORTID PRIMARY;
    enum COLUMN
    {
        COL_REPORTID = 0
        , COL_REPORTNAME = 1
        , COL_GROUPNAME = 2
        , COL_ACTIVE = 3
        , COL_SQLCONTENT = 4
        , COL_LUACONTENT = 5
        , COL_TEMPLATECONTENT = 6
        , COL_DESCRIPTION = 7
    };

    /** Returns the column name as a string*/
    static wxString column_to_name(const COLUMN col)
    {
        switch(col)
        {
            case COL_REPORTID: return "REPORTID";
            case COL_REPORTNAME: return "REPORTNAME";
            case COL_GROUPNAME: return "GROUPNAME";
            case COL_ACTIVE: return "ACTIVE";
            case COL_SQLCONTENT: return "SQLCONTENT";
            case COL_LUACONTENT: return "LUACONTENT";
            case COL_TEMPLATECONTENT: return "TEMPLATECONTENT";
            case COL_DESCRIPTION: return "DESCRIPTION";
            default: break;
        }
        
        return "UNKNOWN";
    }

    /** Returns the column number from the given column name*/
    static COLUMN name_to_column(const wxString& name)
    {
        if ("REPORTID" == name) return COL_REPORTID;
        else if ("REPORTNAME" == name) return COL_REPORTNAME;
        else if ("GROUPNAME" == name) return COL_GROUPNAME;
        else if ("ACTIVE" == name) return COL_ACTIVE;
        else if ("SQLCONTENT" == name) return COL_SQLCONTENT;
        else if ("LUACONTENT" == name) return COL_LUACONTENT;
        else if ("TEMPLATECONTENT" == name) return COL_TEMPLATECONTENT;
        else if ("DESCRIPTION" == name) return COL_DESCRIPTION;

        return COLUMN(-1);
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_REPORT_V1;
        /** This is a instance pointer to itself in memory. */
        Self* table_;
    
        int64 REPORTID;//  primary key
        wxString REPORTNAME;
        wxString GROUPNAME;
        int64 ACTIVE;
        wxString SQLCONTENT;
        wxString LUACONTENT;
        wxString TEMPLATECONTENT;
        wxString DESCRIPTION;

        int64 id() const
        {
            return REPORTID;
        }

        void id(const int64 id)
        {
            REPORTID = id;
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
            if(REPORTID != r->REPORTID) return false;
            if(!REPORTNAME.IsSameAs(r->REPORTNAME)) return false;
            if(!GROUPNAME.IsSameAs(r->GROUPNAME)) return false;
            if(ACTIVE != r->ACTIVE) return false;
            if(!SQLCONTENT.IsSameAs(r->SQLCONTENT)) return false;
            if(!LUACONTENT.IsSameAs(r->LUACONTENT)) return false;
            if(!TEMPLATECONTENT.IsSameAs(r->TEMPLATECONTENT)) return false;
            if(!DESCRIPTION.IsSameAs(r->DESCRIPTION)) return false;
            return true;
        }
        
        explicit Data(Self* table = nullptr ) 
        {
            table_ = table;
        
            REPORTID = -1;
            ACTIVE = -1;
        }

        explicit Data(wxSQLite3ResultSet& q, Self* table = nullptr )
        {
            table_ = table;
        
            REPORTID = q.GetInt64(0); // REPORTID
            REPORTNAME = q.GetString(1); // REPORTNAME
            GROUPNAME = q.GetString(2); // GROUPNAME
            ACTIVE = q.GetInt64(3); // ACTIVE
            SQLCONTENT = q.GetString(4); // SQLCONTENT
            LUACONTENT = q.GetString(5); // LUACONTENT
            TEMPLATECONTENT = q.GetString(6); // TEMPLATECONTENT
            DESCRIPTION = q.GetString(7); // DESCRIPTION
        }

        Data(const Data& other) = default;

        Data& operator=(const Data& other)
        {
            if (this == &other) return *this;

            REPORTID = other.REPORTID;
            REPORTNAME = other.REPORTNAME;
            GROUPNAME = other.GROUPNAME;
            ACTIVE = other.ACTIVE;
            SQLCONTENT = other.SQLCONTENT;
            LUACONTENT = other.LUACONTENT;
            TEMPLATECONTENT = other.TEMPLATECONTENT;
            DESCRIPTION = other.DESCRIPTION;
            return *this;
        }

        template<typename C>
        bool match(const C &) const
        {
            return false;
        }

        bool match(const Self::REPORTID &in) const
        {
            return this->REPORTID == in.v_;
        }

        bool match(const Self::REPORTNAME &in) const
        {
            return this->REPORTNAME.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::GROUPNAME &in) const
        {
            return this->GROUPNAME.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::ACTIVE &in) const
        {
            return this->ACTIVE == in.v_;
        }

        bool match(const Self::SQLCONTENT &in) const
        {
            return this->SQLCONTENT.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::LUACONTENT &in) const
        {
            return this->LUACONTENT.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::TEMPLATECONTENT &in) const
        {
            return this->TEMPLATECONTENT.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::DESCRIPTION &in) const
        {
            return this->DESCRIPTION.CmpNoCase(in.v_) == 0;
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
            json_writer.Key("REPORTID");
            json_writer.Int64(this->REPORTID.GetValue());
            json_writer.Key("REPORTNAME");
            json_writer.String(this->REPORTNAME.utf8_str());
            json_writer.Key("GROUPNAME");
            json_writer.String(this->GROUPNAME.utf8_str());
            json_writer.Key("ACTIVE");
            json_writer.Int64(this->ACTIVE.GetValue());
            json_writer.Key("SQLCONTENT");
            json_writer.String(this->SQLCONTENT.utf8_str());
            json_writer.Key("LUACONTENT");
            json_writer.String(this->LUACONTENT.utf8_str());
            json_writer.Key("TEMPLATECONTENT");
            json_writer.String(this->TEMPLATECONTENT.utf8_str());
            json_writer.Key("DESCRIPTION");
            json_writer.String(this->DESCRIPTION.utf8_str());
        }

        row_t to_row_t() const
        {
            row_t row;
            row(L"REPORTID") = REPORTID.GetValue();
            row(L"REPORTNAME") = REPORTNAME;
            row(L"GROUPNAME") = GROUPNAME;
            row(L"ACTIVE") = ACTIVE.GetValue();
            row(L"SQLCONTENT") = SQLCONTENT;
            row(L"LUACONTENT") = LUACONTENT;
            row(L"TEMPLATECONTENT") = TEMPLATECONTENT;
            row(L"DESCRIPTION") = DESCRIPTION;
            return row;
        }

        void to_template(html_template& t) const
        {
            t(L"REPORTID") = REPORTID.GetValue();
            t(L"REPORTNAME") = REPORTNAME;
            t(L"GROUPNAME") = GROUPNAME;
            t(L"ACTIVE") = ACTIVE.GetValue();
            t(L"SQLCONTENT") = SQLCONTENT;
            t(L"LUACONTENT") = LUACONTENT;
            t(L"TEMPLATECONTENT") = TEMPLATECONTENT;
            t(L"DESCRIPTION") = DESCRIPTION;
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db)
        {
            if (db && db->IsReadOnly()) return false;
            if (!table_ || !db) 
            {
                wxLogError("can not save REPORT_V1");
                return false;
            }

            return table_->save(this, db);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!table_ || !db) 
            {
                wxLogError("can not remove REPORT_V1");
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
        NUM_COLUMNS = 8
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    /** Name of the table*/    
    wxString name() const { return "REPORT_V1"; }

    DB_Table_REPORT_V1() : fake_(new Data())
    {
        query_ = "SELECT REPORTID, REPORTNAME, GROUPNAME, ACTIVE, SQLCONTENT, LUACONTENT, TEMPLATECONTENT, DESCRIPTION FROM REPORT_V1 ";
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
            sql = "INSERT INTO REPORT_V1(REPORTNAME, GROUPNAME, ACTIVE, SQLCONTENT, LUACONTENT, TEMPLATECONTENT, DESCRIPTION, REPORTID) VALUES(?, ?, ?, ?, ?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE REPORT_V1 SET REPORTNAME = ?, GROUPNAME = ?, ACTIVE = ?, SQLCONTENT = ?, LUACONTENT = ?, TEMPLATECONTENT = ?, DESCRIPTION = ? WHERE REPORTID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->REPORTNAME);
            stmt.Bind(2, entity->GROUPNAME);
            stmt.Bind(3, entity->ACTIVE);
            stmt.Bind(4, entity->SQLCONTENT);
            stmt.Bind(5, entity->LUACONTENT);
            stmt.Bind(6, entity->TEMPLATECONTENT);
            stmt.Bind(7, entity->DESCRIPTION);
            stmt.Bind(8, entity->id() > 0 ? entity->REPORTID : newId());

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
            wxLogError("REPORT_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
            return false;
        }

        if (entity->id() <= 0)
        {
            entity->id(db->GetLastRowId());
            index_by_id_.insert(std::make_pair(entity->id(), entity));
        }
        return true;
    }

    /** Remove the Data record from the database and the memory table (cache) */
    bool remove(const int64 id, wxSQLite3Database* db)
    {
        if (id <= 0) return false;
        try
        {
            wxString sql = "DELETE FROM REPORT_V1 WHERE REPORTID = ?";
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
            wxLogError("REPORT_V1: Exception %s", e.GetMessage().utf8_str());
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
    Self::Data* get(const int64 id, wxSQLite3Database* db)
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
    Self::Data* get_record(const int64 id, wxSQLite3Database* db)
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

