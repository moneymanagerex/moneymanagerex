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
 *          AUTO GENERATED at 2025-04-21 15:32:55.694774.
 *          DO NOT EDIT!
 */
//=============================================================================
#pragma once

#include "DB_Table.h"

struct DB_Table_CUSTOMFIELD_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_CUSTOMFIELD_V1 Self;

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
    ~DB_Table_CUSTOMFIELD_V1() 
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
                db->ExecuteUpdate("CREATE TABLE CUSTOMFIELD_V1 (FIELDID INTEGER NOT NULL PRIMARY KEY, REFTYPE TEXT NOT NULL /* Transaction, Stock, Asset, Bank Account, Repeating Transaction, Payee */, DESCRIPTION TEXT COLLATE NOCASE, TYPE TEXT NOT NULL /* String, Integer, Decimal, Boolean, Date, Time, SingleChoice, MultiChoice */, PROPERTIES TEXT NOT NULL)");
                this->ensure_data(db);
            }
            catch(const wxSQLite3Exception &e) 
            { 
                wxLogError("CUSTOMFIELD_V1: Exception %s", e.GetMessage().utf8_str());
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
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_CUSTOMFIELD_REF ON CUSTOMFIELD_V1 (REFTYPE)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("CUSTOMFIELD_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }

        return true;
    }

    void ensure_data(wxSQLite3Database* db)
    {
        db->Begin();
        db->Commit();
    }
    
    struct FIELDID : public DB_Column<int64>
    { 
        static wxString name() { return "FIELDID"; } 
        explicit FIELDID(const int64 &v, OP op = EQUAL): DB_Column<int64>(v, op) {}
    };
    
    struct REFTYPE : public DB_Column<wxString>
    { 
        static wxString name() { return "REFTYPE"; } 
        explicit REFTYPE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct DESCRIPTION : public DB_Column<wxString>
    { 
        static wxString name() { return "DESCRIPTION"; } 
        explicit DESCRIPTION(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct TYPE : public DB_Column<wxString>
    { 
        static wxString name() { return "TYPE"; } 
        explicit TYPE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct PROPERTIES : public DB_Column<wxString>
    { 
        static wxString name() { return "PROPERTIES"; } 
        explicit PROPERTIES(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    typedef FIELDID PRIMARY;
    enum COLUMN
    {
        COL_FIELDID = 0
        , COL_REFTYPE = 1
        , COL_DESCRIPTION = 2
        , COL_TYPE = 3
        , COL_PROPERTIES = 4
    };

    /** Returns the column name as a string*/
    static wxString column_to_name(const COLUMN col)
    {
        switch(col)
        {
            case COL_FIELDID: return "FIELDID";
            case COL_REFTYPE: return "REFTYPE";
            case COL_DESCRIPTION: return "DESCRIPTION";
            case COL_TYPE: return "TYPE";
            case COL_PROPERTIES: return "PROPERTIES";
            default: break;
        }
        
        return "UNKNOWN";
    }

    /** Returns the column number from the given column name*/
    static COLUMN name_to_column(const wxString& name)
    {
        if ("FIELDID" == name) return COL_FIELDID;
        else if ("REFTYPE" == name) return COL_REFTYPE;
        else if ("DESCRIPTION" == name) return COL_DESCRIPTION;
        else if ("TYPE" == name) return COL_TYPE;
        else if ("PROPERTIES" == name) return COL_PROPERTIES;

        return COLUMN(-1);
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_CUSTOMFIELD_V1;
        /** This is a instance pointer to itself in memory. */
        Self* table_;
    
        int64 FIELDID;//  primary key
        wxString REFTYPE;
        wxString DESCRIPTION;
        wxString TYPE;
        wxString PROPERTIES;

        int64 id() const
        {
            return FIELDID;
        }

        void id(const int64 id)
        {
            FIELDID = id;
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
            if(FIELDID != r->FIELDID) return false;
            if(!REFTYPE.IsSameAs(r->REFTYPE)) return false;
            if(!DESCRIPTION.IsSameAs(r->DESCRIPTION)) return false;
            if(!TYPE.IsSameAs(r->TYPE)) return false;
            if(!PROPERTIES.IsSameAs(r->PROPERTIES)) return false;
            return true;
        }
        
        explicit Data(Self* table = nullptr ) 
        {
            table_ = table;
        
            FIELDID = -1;
        }

        explicit Data(wxSQLite3ResultSet& q, Self* table = nullptr )
        {
            table_ = table;
        
            FIELDID = q.GetInt64(0); // FIELDID
            REFTYPE = q.GetString(1); // REFTYPE
            DESCRIPTION = q.GetString(2); // DESCRIPTION
            TYPE = q.GetString(3); // TYPE
            PROPERTIES = q.GetString(4); // PROPERTIES
        }

        Data(const Data& other) = default;

        Data& operator=(const Data& other)
        {
            if (this == &other) return *this;

            FIELDID = other.FIELDID;
            REFTYPE = other.REFTYPE;
            DESCRIPTION = other.DESCRIPTION;
            TYPE = other.TYPE;
            PROPERTIES = other.PROPERTIES;
            return *this;
        }

        template<typename C>
        bool match(const C &) const
        {
            return false;
        }

        bool match(const Self::FIELDID &in) const
        {
            return this->FIELDID == in.v_;
        }

        bool match(const Self::REFTYPE &in) const
        {
            return this->REFTYPE.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::DESCRIPTION &in) const
        {
            return this->DESCRIPTION.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::TYPE &in) const
        {
            return this->TYPE.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::PROPERTIES &in) const
        {
            return this->PROPERTIES.CmpNoCase(in.v_) == 0;
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
            json_writer.Key("FIELDID");
            json_writer.Int64(this->FIELDID.GetValue());
            json_writer.Key("REFTYPE");
            json_writer.String(this->REFTYPE.utf8_str());
            json_writer.Key("DESCRIPTION");
            json_writer.String(this->DESCRIPTION.utf8_str());
            json_writer.Key("TYPE");
            json_writer.String(this->TYPE.utf8_str());
            json_writer.Key("PROPERTIES");
            json_writer.String(this->PROPERTIES.utf8_str());
        }

        row_t to_row_t() const
        {
            row_t row;
            row(L"FIELDID") = FIELDID.GetValue();
            row(L"REFTYPE") = REFTYPE;
            row(L"DESCRIPTION") = DESCRIPTION;
            row(L"TYPE") = TYPE;
            row(L"PROPERTIES") = PROPERTIES;
            return row;
        }

        void to_template(html_template& t) const
        {
            t(L"FIELDID") = FIELDID.GetValue();
            t(L"REFTYPE") = REFTYPE;
            t(L"DESCRIPTION") = DESCRIPTION;
            t(L"TYPE") = TYPE;
            t(L"PROPERTIES") = PROPERTIES;
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db, bool force_insert = false)
        {
            if (db && db->IsReadOnly()) return false;
            if (!table_ || !db) 
            {
                wxLogError("can not save CUSTOMFIELD_V1");
                return false;
            }

            return table_->save(this, db, force_insert);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!table_ || !db) 
            {
                wxLogError("can not remove CUSTOMFIELD_V1");
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
        NUM_COLUMNS = 5
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    /** Name of the table*/    
    wxString name() const { return "CUSTOMFIELD_V1"; }

    DB_Table_CUSTOMFIELD_V1() : fake_(new Data())
    {
        query_ = "SELECT FIELDID, REFTYPE, DESCRIPTION, TYPE, PROPERTIES FROM CUSTOMFIELD_V1 ";
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
            sql = "INSERT INTO CUSTOMFIELD_V1(REFTYPE, DESCRIPTION, TYPE, PROPERTIES, FIELDID) VALUES(?, ?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE CUSTOMFIELD_V1 SET REFTYPE = ?, DESCRIPTION = ?, TYPE = ?, PROPERTIES = ? WHERE FIELDID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->REFTYPE);
            stmt.Bind(2, entity->DESCRIPTION);
            stmt.Bind(3, entity->TYPE);
            stmt.Bind(4, entity->PROPERTIES);
            stmt.Bind(5, entity->id() > 0 ? entity->FIELDID : newId());

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
            wxLogError("CUSTOMFIELD_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
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
            wxString sql = "DELETE FROM CUSTOMFIELD_V1 WHERE FIELDID = ?";
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
            wxLogError("CUSTOMFIELD_V1: Exception %s", e.GetMessage().utf8_str());
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

