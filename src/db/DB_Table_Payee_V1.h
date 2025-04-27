// -*- C++ -*-
//=============================================================================
/**
 *      Copyright: (c) 2013 - 2025 Guan Lisheng (guanlisheng@gmail.com)
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
 *          AUTO GENERATED at 2025-04-27 10:42:05.082216.
 *          DO NOT EDIT!
 */
//=============================================================================
#pragma once

#include "DB_Table.h"

struct DB_Table_PAYEE_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_PAYEE_V1 Self;

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
    ~DB_Table_PAYEE_V1() 
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
                db->ExecuteUpdate("CREATE TABLE PAYEE_V1(PAYEEID integer primary key, PAYEENAME TEXT COLLATE NOCASE NOT NULL UNIQUE, CATEGID integer, NUMBER TEXT, WEBSITE TEXT, NOTES TEXT, ACTIVE integer, PATTERN TEXT DEFAULT '')");
                this->ensure_data(db);
            }
            catch(const wxSQLite3Exception &e) 
            { 
                wxLogError("PAYEE_V1: Exception %s", e.GetMessage().utf8_str());
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
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_PAYEE_INFONAME ON PAYEE_V1(PAYEENAME)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("PAYEE_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }

        return true;
    }

    void ensure_data(wxSQLite3Database* db)
    {
        db->Begin();
        db->Commit();
    }
    
    struct PAYEEID : public DB_Column<int64>
    { 
        static wxString name() { return "PAYEEID"; } 
        explicit PAYEEID(const int64 &v, OP op = EQUAL): DB_Column<int64>(v, op) {}
    };
    
    struct PAYEENAME : public DB_Column<wxString>
    { 
        static wxString name() { return "PAYEENAME"; } 
        explicit PAYEENAME(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct CATEGID : public DB_Column<int64>
    { 
        static wxString name() { return "CATEGID"; } 
        explicit CATEGID(const int64 &v, OP op = EQUAL): DB_Column<int64>(v, op) {}
    };
    
    struct NUMBER : public DB_Column<wxString>
    { 
        static wxString name() { return "NUMBER"; } 
        explicit NUMBER(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct WEBSITE : public DB_Column<wxString>
    { 
        static wxString name() { return "WEBSITE"; } 
        explicit WEBSITE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct NOTES : public DB_Column<wxString>
    { 
        static wxString name() { return "NOTES"; } 
        explicit NOTES(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct ACTIVE : public DB_Column<int64>
    { 
        static wxString name() { return "ACTIVE"; } 
        explicit ACTIVE(const int64 &v, OP op = EQUAL): DB_Column<int64>(v, op) {}
    };
    
    struct PATTERN : public DB_Column<wxString>
    { 
        static wxString name() { return "PATTERN"; } 
        explicit PATTERN(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    typedef PAYEEID PRIMARY;
    enum COLUMN
    {
        COL_PAYEEID = 0
        , COL_PAYEENAME = 1
        , COL_CATEGID = 2
        , COL_NUMBER = 3
        , COL_WEBSITE = 4
        , COL_NOTES = 5
        , COL_ACTIVE = 6
        , COL_PATTERN = 7
    };

    /** Returns the column name as a string*/
    static wxString column_to_name(const COLUMN col)
    {
        switch(col)
        {
            case COL_PAYEEID: return "PAYEEID";
            case COL_PAYEENAME: return "PAYEENAME";
            case COL_CATEGID: return "CATEGID";
            case COL_NUMBER: return "NUMBER";
            case COL_WEBSITE: return "WEBSITE";
            case COL_NOTES: return "NOTES";
            case COL_ACTIVE: return "ACTIVE";
            case COL_PATTERN: return "PATTERN";
            default: break;
        }
        
        return "UNKNOWN";
    }

    /** Returns the column number from the given column name*/
    static COLUMN name_to_column(const wxString& name)
    {
        if ("PAYEEID" == name) return COL_PAYEEID;
        else if ("PAYEENAME" == name) return COL_PAYEENAME;
        else if ("CATEGID" == name) return COL_CATEGID;
        else if ("NUMBER" == name) return COL_NUMBER;
        else if ("WEBSITE" == name) return COL_WEBSITE;
        else if ("NOTES" == name) return COL_NOTES;
        else if ("ACTIVE" == name) return COL_ACTIVE;
        else if ("PATTERN" == name) return COL_PATTERN;

        return COLUMN(-1);
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_PAYEE_V1;
        /** This is a instance pointer to itself in memory. */
        Self* table_;
    
        int64 PAYEEID;//  primary key
        wxString PAYEENAME;
        int64 CATEGID;
        wxString NUMBER;
        wxString WEBSITE;
        wxString NOTES;
        int64 ACTIVE;
        wxString PATTERN;

        int64 id() const
        {
            return PAYEEID;
        }

        void id(const int64 id)
        {
            PAYEEID = id;
        }

        auto operator<=>(const Data& other) const
        {
            return this->id().GetValue() <=> other.id().GetValue();
        }

        auto operator<=>(const Data* other) const
        {
            return this->id().GetValue() <=> other->id().GetValue();
        }

        bool equals(const Data* r) const
        {
            if(PAYEEID != r->PAYEEID) return false;
            if(!PAYEENAME.IsSameAs(r->PAYEENAME)) return false;
            if(CATEGID != r->CATEGID) return false;
            if(!NUMBER.IsSameAs(r->NUMBER)) return false;
            if(!WEBSITE.IsSameAs(r->WEBSITE)) return false;
            if(!NOTES.IsSameAs(r->NOTES)) return false;
            if(ACTIVE != r->ACTIVE) return false;
            if(!PATTERN.IsSameAs(r->PATTERN)) return false;
            return true;
        }
        
        explicit Data(Self* table = nullptr ) 
        {
            table_ = table;
        
            PAYEEID = -1;
            CATEGID = -1;
            ACTIVE = -1;
        }

        explicit Data(wxSQLite3ResultSet& q, Self* table = nullptr )
        {
            table_ = table;
        
            PAYEEID = q.GetInt64(0); // PAYEEID
            PAYEENAME = q.GetString(1); // PAYEENAME
            CATEGID = q.GetInt64(2); // CATEGID
            NUMBER = q.GetString(3); // NUMBER
            WEBSITE = q.GetString(4); // WEBSITE
            NOTES = q.GetString(5); // NOTES
            ACTIVE = q.GetInt64(6); // ACTIVE
            PATTERN = q.GetString(7); // PATTERN
        }

        Data(const Data& other) = default;

        Data& operator=(const Data& other)
        {
            if (this == &other) return *this;

            PAYEEID = other.PAYEEID;
            PAYEENAME = other.PAYEENAME;
            CATEGID = other.CATEGID;
            NUMBER = other.NUMBER;
            WEBSITE = other.WEBSITE;
            NOTES = other.NOTES;
            ACTIVE = other.ACTIVE;
            PATTERN = other.PATTERN;
            return *this;
        }

        template<typename C>
        bool match(const C &) const
        {
            return false;
        }

        bool match(const Self::PAYEEID &in) const
        {
            return this->PAYEEID == in.v_;
        }

        bool match(const Self::PAYEENAME &in) const
        {
            return this->PAYEENAME.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::CATEGID &in) const
        {
            return this->CATEGID == in.v_;
        }

        bool match(const Self::NUMBER &in) const
        {
            return this->NUMBER.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::WEBSITE &in) const
        {
            return this->WEBSITE.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::NOTES &in) const
        {
            return this->NOTES.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::ACTIVE &in) const
        {
            return this->ACTIVE == in.v_;
        }

        bool match(const Self::PATTERN &in) const
        {
            return this->PATTERN.CmpNoCase(in.v_) == 0;
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
            json_writer.Key("PAYEEID");
            json_writer.Int64(this->PAYEEID.GetValue());
            json_writer.Key("PAYEENAME");
            json_writer.String(this->PAYEENAME.utf8_str());
            json_writer.Key("CATEGID");
            json_writer.Int64(this->CATEGID.GetValue());
            json_writer.Key("NUMBER");
            json_writer.String(this->NUMBER.utf8_str());
            json_writer.Key("WEBSITE");
            json_writer.String(this->WEBSITE.utf8_str());
            json_writer.Key("NOTES");
            json_writer.String(this->NOTES.utf8_str());
            json_writer.Key("ACTIVE");
            json_writer.Int64(this->ACTIVE.GetValue());
            json_writer.Key("PATTERN");
            json_writer.String(this->PATTERN.utf8_str());
        }

        row_t to_row_t() const
        {
            row_t row;
            row(L"PAYEEID") = PAYEEID.GetValue();
            row(L"PAYEENAME") = PAYEENAME;
            row(L"CATEGID") = CATEGID.GetValue();
            row(L"NUMBER") = NUMBER;
            row(L"WEBSITE") = WEBSITE;
            row(L"NOTES") = NOTES;
            row(L"ACTIVE") = ACTIVE.GetValue();
            row(L"PATTERN") = PATTERN;
            return row;
        }

        void to_template(html_template& t) const
        {
            t(L"PAYEEID") = PAYEEID.GetValue();
            t(L"PAYEENAME") = PAYEENAME;
            t(L"CATEGID") = CATEGID.GetValue();
            t(L"NUMBER") = NUMBER;
            t(L"WEBSITE") = WEBSITE;
            t(L"NOTES") = NOTES;
            t(L"ACTIVE") = ACTIVE.GetValue();
            t(L"PATTERN") = PATTERN;
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db, bool force_insert = false)
        {
            if (db && db->IsReadOnly()) return false;
            if (!table_ || !db) 
            {
                wxLogError("can not save PAYEE_V1");
                return false;
            }

            return table_->save(this, db, force_insert);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!table_ || !db) 
            {
                wxLogError("can not remove PAYEE_V1");
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
    wxString name() const { return "PAYEE_V1"; }

    DB_Table_PAYEE_V1() : fake_(new Data())
    {
        query_ = "SELECT PAYEEID, PAYEENAME, CATEGID, NUMBER, WEBSITE, NOTES, ACTIVE, PATTERN FROM PAYEE_V1 ";
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
    bool save(Self::Data* entity, wxSQLite3Database* db, bool force_insert = false)
    {
        wxString sql = wxEmptyString;
        if (entity->id() <= 0 || force_insert) //  new & insert
        {
            sql = "INSERT INTO PAYEE_V1(PAYEENAME, CATEGID, NUMBER, WEBSITE, NOTES, ACTIVE, PATTERN, PAYEEID) VALUES(?, ?, ?, ?, ?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE PAYEE_V1 SET PAYEENAME = ?, CATEGID = ?, NUMBER = ?, WEBSITE = ?, NOTES = ?, ACTIVE = ?, PATTERN = ? WHERE PAYEEID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->PAYEENAME);
            stmt.Bind(2, entity->CATEGID);
            stmt.Bind(3, entity->NUMBER);
            stmt.Bind(4, entity->WEBSITE);
            stmt.Bind(5, entity->NOTES);
            stmt.Bind(6, entity->ACTIVE);
            stmt.Bind(7, entity->PATTERN);
            stmt.Bind(8, entity->id() > 0 ? entity->PAYEEID : newId());

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
            wxLogError("PAYEE_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
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
            wxString sql = "DELETE FROM PAYEE_V1 WHERE PAYEEID = ?";
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
            wxLogError("PAYEE_V1: Exception %s", e.GetMessage().utf8_str());
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
        for (auto& [_, item] : index_by_id_)
        {
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

        if (auto it = index_by_id_.find(id); it != index_by_id_.end())
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

