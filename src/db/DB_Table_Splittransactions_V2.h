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

struct DB_Table_SPLITTRANSACTIONS_V2 : public DB_Table
{
    struct Data;
    typedef DB_Table_SPLITTRANSACTIONS_V2 Self;

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
    ~DB_Table_SPLITTRANSACTIONS_V2() 
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
                db->ExecuteUpdate("CREATE TABLE SPLITTRANSACTIONS_V2(SPLITTRANSID integer primary key, ACCOUNTID integer NOT NULL, TRANSID integer NOT NULL, CATEGID integer, SUBCATEGID integer, SPLITTRANSAMOUNT numeric, NOTES TEXT)");
                this->ensure_data(db);
            }
            catch(const wxSQLite3Exception &e) 
            { 
                wxLogError("SPLITTRANSACTIONS_V2: Exception %s", e.GetMessage().utf8_str());
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
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_SPLITTRANSACTIONS_V2_ACCOUNTID ON SPLITTRANSACTIONS_V2(ACCOUNTID)");
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_SPLITTRANSACTIONS_V2_TRANSID ON SPLITTRANSACTIONS_V2(TRANSID)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("SPLITTRANSACTIONS_V2: Exception %s", e.GetMessage().utf8_str());
            return false;
        }

        return true;
    }

    void ensure_data(wxSQLite3Database* db)
    {
        db->Begin();
        db->Commit();
    }
    
    struct SPLITTRANSID : public DB_Column<int>
    { 
        static wxString name() { return "SPLITTRANSID"; } 
        explicit SPLITTRANSID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct ACCOUNTID : public DB_Column<int>
    { 
        static wxString name() { return "ACCOUNTID"; } 
        explicit ACCOUNTID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct TRANSID : public DB_Column<int>
    { 
        static wxString name() { return "TRANSID"; } 
        explicit TRANSID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct CATEGID : public DB_Column<int>
    { 
        static wxString name() { return "CATEGID"; } 
        explicit CATEGID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct SUBCATEGID : public DB_Column<int>
    { 
        static wxString name() { return "SUBCATEGID"; } 
        explicit SUBCATEGID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct SPLITTRANSAMOUNT : public DB_Column<double>
    { 
        static wxString name() { return "SPLITTRANSAMOUNT"; } 
        explicit SPLITTRANSAMOUNT(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    
    struct NOTES : public DB_Column<wxString>
    { 
        static wxString name() { return "NOTES"; } 
        explicit NOTES(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    typedef SPLITTRANSID PRIMARY;
    enum COLUMN
    {
        COL_SPLITTRANSID = 0
        , COL_ACCOUNTID = 1
        , COL_TRANSID = 2
        , COL_CATEGID = 3
        , COL_SUBCATEGID = 4
        , COL_SPLITTRANSAMOUNT = 5
        , COL_NOTES = 6
    };

    /** Returns the column name as a string*/
    static wxString column_to_name(COLUMN col)
    {
        switch(col)
        {
            case COL_SPLITTRANSID: return "SPLITTRANSID";
            case COL_ACCOUNTID: return "ACCOUNTID";
            case COL_TRANSID: return "TRANSID";
            case COL_CATEGID: return "CATEGID";
            case COL_SUBCATEGID: return "SUBCATEGID";
            case COL_SPLITTRANSAMOUNT: return "SPLITTRANSAMOUNT";
            case COL_NOTES: return "NOTES";
            default: break;
        }
        
        return "UNKNOWN";
    }

    /** Returns the column number from the given column name*/
    static COLUMN name_to_column(const wxString& name)
    {
        if ("SPLITTRANSID" == name) return COL_SPLITTRANSID;
        else if ("ACCOUNTID" == name) return COL_ACCOUNTID;
        else if ("TRANSID" == name) return COL_TRANSID;
        else if ("CATEGID" == name) return COL_CATEGID;
        else if ("SUBCATEGID" == name) return COL_SUBCATEGID;
        else if ("SPLITTRANSAMOUNT" == name) return COL_SPLITTRANSAMOUNT;
        else if ("NOTES" == name) return COL_NOTES;

        return COLUMN(-1);
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_SPLITTRANSACTIONS_V2;
        /** This is a instance pointer to itself in memory. */
        Self* table_;
    
        int SPLITTRANSID;//  primary key
        int ACCOUNTID;
        int TRANSID;
        int CATEGID;
        int SUBCATEGID;
        double SPLITTRANSAMOUNT;
        wxString NOTES;

        int id() const
        {
            return SPLITTRANSID;
        }

        void id(int id)
        {
            SPLITTRANSID = id;
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
        
            SPLITTRANSID = -1;
            ACCOUNTID = -1;
            TRANSID = -1;
            CATEGID = -1;
            SUBCATEGID = -1;
            SPLITTRANSAMOUNT = 0.0;
        }

        explicit Data(wxSQLite3ResultSet& q, Self* table = 0)
        {
            table_ = table;
        
            SPLITTRANSID = q.GetInt(0); // SPLITTRANSID
            ACCOUNTID = q.GetInt(1); // ACCOUNTID
            TRANSID = q.GetInt(2); // TRANSID
            CATEGID = q.GetInt(3); // CATEGID
            SUBCATEGID = q.GetInt(4); // SUBCATEGID
            SPLITTRANSAMOUNT = q.GetDouble(5); // SPLITTRANSAMOUNT
            NOTES = q.GetString(6); // NOTES
        }

        Data& operator=(const Data& other)
        {
            if (this == &other) return *this;

            SPLITTRANSID = other.SPLITTRANSID;
            ACCOUNTID = other.ACCOUNTID;
            TRANSID = other.TRANSID;
            CATEGID = other.CATEGID;
            SUBCATEGID = other.SUBCATEGID;
            SPLITTRANSAMOUNT = other.SPLITTRANSAMOUNT;
            NOTES = other.NOTES;
            return *this;
        }

        template<typename C>
        bool match(const C &c) const
        {
            return false;
        }

        bool match(const Self::SPLITTRANSID &in) const
        {
            return this->SPLITTRANSID == in.v_;
        }

        bool match(const Self::ACCOUNTID &in) const
        {
            return this->ACCOUNTID == in.v_;
        }

        bool match(const Self::TRANSID &in) const
        {
            return this->TRANSID == in.v_;
        }

        bool match(const Self::CATEGID &in) const
        {
            return this->CATEGID == in.v_;
        }

        bool match(const Self::SUBCATEGID &in) const
        {
            return this->SUBCATEGID == in.v_;
        }

        bool match(const Self::SPLITTRANSAMOUNT &in) const
        {
            return this->SPLITTRANSAMOUNT == in.v_;
        }

        bool match(const Self::NOTES &in) const
        {
            return this->NOTES.CmpNoCase(in.v_) == 0;
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
            json_writer.Key("SPLITTRANSID");
            json_writer.Int(this->SPLITTRANSID);
            json_writer.Key("ACCOUNTID");
            json_writer.Int(this->ACCOUNTID);
            json_writer.Key("TRANSID");
            json_writer.Int(this->TRANSID);
            json_writer.Key("CATEGID");
            json_writer.Int(this->CATEGID);
            json_writer.Key("SUBCATEGID");
            json_writer.Int(this->SUBCATEGID);
            json_writer.Key("SPLITTRANSAMOUNT");
            json_writer.Double(this->SPLITTRANSAMOUNT);
            json_writer.Key("NOTES");
            json_writer.String(this->NOTES.utf8_str());
        }

        row_t to_row_t() const
        {
            row_t row;
            row(L"SPLITTRANSID") = SPLITTRANSID;
            row(L"ACCOUNTID") = ACCOUNTID;
            row(L"TRANSID") = TRANSID;
            row(L"CATEGID") = CATEGID;
            row(L"SUBCATEGID") = SUBCATEGID;
            row(L"SPLITTRANSAMOUNT") = SPLITTRANSAMOUNT;
            row(L"NOTES") = NOTES;
            return row;
        }

        void to_template(html_template& t) const
        {
            t(L"SPLITTRANSID") = SPLITTRANSID;
            t(L"ACCOUNTID") = ACCOUNTID;
            t(L"TRANSID") = TRANSID;
            t(L"CATEGID") = CATEGID;
            t(L"SUBCATEGID") = SUBCATEGID;
            t(L"SPLITTRANSAMOUNT") = SPLITTRANSAMOUNT;
            t(L"NOTES") = NOTES;
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db)
        {
            if (db && db->IsReadOnly()) return false;
            if (!table_ || !db) 
            {
                wxLogError("can not save SPLITTRANSACTIONS_V2");
                return false;
            }

            return table_->save(this, db);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!table_ || !db) 
            {
                wxLogError("can not remove SPLITTRANSACTIONS_V2");
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
        NUM_COLUMNS = 7
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    /** Name of the table*/    
    wxString name() const { return "SPLITTRANSACTIONS_V2"; }

    DB_Table_SPLITTRANSACTIONS_V2() : fake_(new Data())
    {
        query_ = "SELECT SPLITTRANSID, ACCOUNTID, TRANSID, CATEGID, SUBCATEGID, SPLITTRANSAMOUNT, NOTES FROM SPLITTRANSACTIONS_V2 ";
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
            sql = "INSERT INTO SPLITTRANSACTIONS_V2(ACCOUNTID, TRANSID, CATEGID, SUBCATEGID, SPLITTRANSAMOUNT, NOTES) VALUES(?, ?, ?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE SPLITTRANSACTIONS_V2 SET ACCOUNTID = ?, TRANSID = ?, CATEGID = ?, SUBCATEGID = ?, SPLITTRANSAMOUNT = ?, NOTES = ? WHERE SPLITTRANSID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->ACCOUNTID);
            stmt.Bind(2, entity->TRANSID);
            stmt.Bind(3, entity->CATEGID);
            stmt.Bind(4, entity->SUBCATEGID);
            stmt.Bind(5, entity->SPLITTRANSAMOUNT);
            stmt.Bind(6, entity->NOTES);
            if (entity->id() > 0)
                stmt.Bind(7, entity->SPLITTRANSID);

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
            wxLogError("SPLITTRANSACTIONS_V2: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
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
            wxString sql = "DELETE FROM SPLITTRANSACTIONS_V2 WHERE SPLITTRANSID = ?";
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
            wxLogError("SPLITTRANSACTIONS_V2: Exception %s", e.GetMessage().utf8_str());
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

