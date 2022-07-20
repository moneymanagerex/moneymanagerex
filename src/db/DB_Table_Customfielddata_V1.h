// -*- C++ -*-
//=============================================================================
/**
 *      Copyright: (c) 2013 - 2022 Guan Lisheng (guanlisheng@gmail.com)
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
 *          AUTO GENERATED at 2022-07-20 15:29:27.776453.
 *          DO NOT EDIT!
 */
//=============================================================================
#pragma once

#include "DB_Table.h"

struct DB_Table_CUSTOMFIELDDATA_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_CUSTOMFIELDDATA_V1 Self;

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
    typedef std::map<int, Self::Data*> Index_By_Id;
    Cache cache_;
    Index_By_Id index_by_id_;
    Data* fake_; // in case the entity not found

    /** Destructor: clears any data records stored in memory */
    ~DB_Table_CUSTOMFIELDDATA_V1() 
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
                db->ExecuteUpdate("CREATE TABLE CUSTOMFIELDDATA_V1 (FIELDATADID INTEGER NOT NULL PRIMARY KEY, FIELDID INTEGER NOT NULL, REFID INTEGER NOT NULL, CONTENT TEXT, UNIQUE(FIELDID, REFID))");
                this->ensure_data(db);
            }
            catch(const wxSQLite3Exception &e) 
            { 
                wxLogError("CUSTOMFIELDDATA_V1: Exception %s", e.GetMessage().utf8_str());
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
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_CUSTOMFIELDDATA_REF ON CUSTOMFIELDDATA_V1 (FIELDID, REFID)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("CUSTOMFIELDDATA_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }

        return true;
    }

    void ensure_data(wxSQLite3Database* db)
    {
        db->Begin();
        db->Commit();
    }
    
    struct FIELDATADID : public DB_Column<int>
    { 
        static wxString name() { return "FIELDATADID"; } 
        explicit FIELDATADID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct FIELDID : public DB_Column<int>
    { 
        static wxString name() { return "FIELDID"; } 
        explicit FIELDID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct REFID : public DB_Column<int>
    { 
        static wxString name() { return "REFID"; } 
        explicit REFID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct CONTENT : public DB_Column<wxString>
    { 
        static wxString name() { return "CONTENT"; } 
        explicit CONTENT(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    typedef FIELDATADID PRIMARY;
    enum COLUMN
    {
        COL_FIELDATADID = 0
        , COL_FIELDID = 1
        , COL_REFID = 2
        , COL_CONTENT = 3
    };

    /** Returns the column name as a string*/
    static wxString column_to_name(COLUMN col)
    {
        switch(col)
        {
            case COL_FIELDATADID: return "FIELDATADID";
            case COL_FIELDID: return "FIELDID";
            case COL_REFID: return "REFID";
            case COL_CONTENT: return "CONTENT";
            default: break;
        }
        
        return "UNKNOWN";
    }

    /** Returns the column number from the given column name*/
    static COLUMN name_to_column(const wxString& name)
    {
        if ("FIELDATADID" == name) return COL_FIELDATADID;
        else if ("FIELDID" == name) return COL_FIELDID;
        else if ("REFID" == name) return COL_REFID;
        else if ("CONTENT" == name) return COL_CONTENT;

        return COLUMN(-1);
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_CUSTOMFIELDDATA_V1;
        /** This is a instance pointer to itself in memory. */
        Self* table_;
    
        int FIELDATADID;//  primary key
        int FIELDID;
        int REFID;
        wxString CONTENT;

        int id() const
        {
            return FIELDATADID;
        }

        void id(int id)
        {
            FIELDATADID = id;
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
        
            FIELDATADID = -1;
            FIELDID = -1;
            REFID = -1;
        }

        explicit Data(wxSQLite3ResultSet& q, Self* table = 0)
        {
            table_ = table;
        
            FIELDATADID = q.GetInt(0); // FIELDATADID
            FIELDID = q.GetInt(1); // FIELDID
            REFID = q.GetInt(2); // REFID
            CONTENT = q.GetString(3); // CONTENT
        }

        Data& operator=(const Data& other)
        {
            if (this == &other) return *this;

            FIELDATADID = other.FIELDATADID;
            FIELDID = other.FIELDID;
            REFID = other.REFID;
            CONTENT = other.CONTENT;
            return *this;
        }

        template<typename C>
        bool match(const C &c) const
        {
            return false;
        }

        bool match(const Self::FIELDATADID &in) const
        {
            return this->FIELDATADID == in.v_;
        }

        bool match(const Self::FIELDID &in) const
        {
            return this->FIELDID == in.v_;
        }

        bool match(const Self::REFID &in) const
        {
            return this->REFID == in.v_;
        }

        bool match(const Self::CONTENT &in) const
        {
            return this->CONTENT.CmpNoCase(in.v_) == 0;
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
            json_writer.Key("FIELDATADID");
            json_writer.Int(this->FIELDATADID);
            json_writer.Key("FIELDID");
            json_writer.Int(this->FIELDID);
            json_writer.Key("REFID");
            json_writer.Int(this->REFID);
            json_writer.Key("CONTENT");
            json_writer.String(this->CONTENT.utf8_str());
        }

        row_t to_row_t() const
        {
            row_t row;
            row(L"FIELDATADID") = FIELDATADID;
            row(L"FIELDID") = FIELDID;
            row(L"REFID") = REFID;
            row(L"CONTENT") = CONTENT;
            return row;
        }

        void to_template(html_template& t) const
        {
            t(L"FIELDATADID") = FIELDATADID;
            t(L"FIELDID") = FIELDID;
            t(L"REFID") = REFID;
            t(L"CONTENT") = CONTENT;
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db)
        {
            if (db && db->IsReadOnly()) return false;
            if (!table_ || !db) 
            {
                wxLogError("can not save CUSTOMFIELDDATA_V1");
                return false;
            }

            return table_->save(this, db);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!table_ || !db) 
            {
                wxLogError("can not remove CUSTOMFIELDDATA_V1");
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
    wxString name() const { return "CUSTOMFIELDDATA_V1"; }

    DB_Table_CUSTOMFIELDDATA_V1() : fake_(new Data())
    {
        query_ = "SELECT FIELDATADID, FIELDID, REFID, CONTENT FROM CUSTOMFIELDDATA_V1 ";
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
            sql = "INSERT INTO CUSTOMFIELDDATA_V1(FIELDID, REFID, CONTENT) VALUES(?, ?, ?)";
        }
        else
        {
            sql = "UPDATE CUSTOMFIELDDATA_V1 SET FIELDID = ?, REFID = ?, CONTENT = ? WHERE FIELDATADID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->FIELDID);
            stmt.Bind(2, entity->REFID);
            stmt.Bind(3, entity->CONTENT);
            if (entity->id() > 0)
                stmt.Bind(4, entity->FIELDATADID);

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
            wxLogError("CUSTOMFIELDDATA_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
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
            wxString sql = "DELETE FROM CUSTOMFIELDDATA_V1 WHERE FIELDATADID = ?";
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
            wxLogError("CUSTOMFIELDDATA_V1: Exception %s", e.GetMessage().utf8_str());
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
            wxLogError("%s: Exception %s", this->name().utf8_str(), e.GetMessage().utf8_str());
        }

        return result;
    }
};

