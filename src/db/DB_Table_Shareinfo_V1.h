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

struct DB_Table_SHAREINFO_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_SHAREINFO_V1 Self;

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
    ~DB_Table_SHAREINFO_V1() 
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
                db->ExecuteUpdate("CREATE TABLE SHAREINFO_V1 (SHAREINFOID integer NOT NULL primary key, CHECKINGACCOUNTID integer NOT NULL, SHARENUMBER numeric, SHAREPRICE numeric, SHARECOMMISSION numeric, SHARELOT TEXT)");
                this->ensure_data(db);
            }
            catch(const wxSQLite3Exception &e) 
            { 
                wxLogError("SHAREINFO_V1: Exception %s", e.GetMessage().utf8_str());
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
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_SHAREINFO ON SHAREINFO_V1 (CHECKINGACCOUNTID)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("SHAREINFO_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }

        return true;
    }

    void ensure_data(wxSQLite3Database* db)
    {
        db->Begin();
        db->Commit();
    }
    
    struct SHAREINFOID : public DB_Column<int>
    { 
        static wxString name() { return "SHAREINFOID"; } 
        explicit SHAREINFOID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct CHECKINGACCOUNTID : public DB_Column<int>
    { 
        static wxString name() { return "CHECKINGACCOUNTID"; } 
        explicit CHECKINGACCOUNTID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct SHARENUMBER : public DB_Column<double>
    { 
        static wxString name() { return "SHARENUMBER"; } 
        explicit SHARENUMBER(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    
    struct SHAREPRICE : public DB_Column<double>
    { 
        static wxString name() { return "SHAREPRICE"; } 
        explicit SHAREPRICE(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    
    struct SHARECOMMISSION : public DB_Column<double>
    { 
        static wxString name() { return "SHARECOMMISSION"; } 
        explicit SHARECOMMISSION(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    
    struct SHARELOT : public DB_Column<wxString>
    { 
        static wxString name() { return "SHARELOT"; } 
        explicit SHARELOT(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    typedef SHAREINFOID PRIMARY;
    enum COLUMN
    {
        COL_SHAREINFOID = 0
        , COL_CHECKINGACCOUNTID = 1
        , COL_SHARENUMBER = 2
        , COL_SHAREPRICE = 3
        , COL_SHARECOMMISSION = 4
        , COL_SHARELOT = 5
    };

    /** Returns the column name as a string*/
    static wxString column_to_name(COLUMN col)
    {
        switch(col)
        {
            case COL_SHAREINFOID: return "SHAREINFOID";
            case COL_CHECKINGACCOUNTID: return "CHECKINGACCOUNTID";
            case COL_SHARENUMBER: return "SHARENUMBER";
            case COL_SHAREPRICE: return "SHAREPRICE";
            case COL_SHARECOMMISSION: return "SHARECOMMISSION";
            case COL_SHARELOT: return "SHARELOT";
            default: break;
        }
        
        return "UNKNOWN";
    }

    /** Returns the column number from the given column name*/
    static COLUMN name_to_column(const wxString& name)
    {
        if ("SHAREINFOID" == name) return COL_SHAREINFOID;
        else if ("CHECKINGACCOUNTID" == name) return COL_CHECKINGACCOUNTID;
        else if ("SHARENUMBER" == name) return COL_SHARENUMBER;
        else if ("SHAREPRICE" == name) return COL_SHAREPRICE;
        else if ("SHARECOMMISSION" == name) return COL_SHARECOMMISSION;
        else if ("SHARELOT" == name) return COL_SHARELOT;

        return COLUMN(-1);
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_SHAREINFO_V1;
        /** This is a instance pointer to itself in memory. */
        Self* table_;
    
        int SHAREINFOID;//  primary key
        int CHECKINGACCOUNTID;
        double SHARENUMBER;
        double SHAREPRICE;
        double SHARECOMMISSION;
        wxString SHARELOT;

        int id() const
        {
            return SHAREINFOID;
        }

        void id(int id)
        {
            SHAREINFOID = id;
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
        
            SHAREINFOID = -1;
            CHECKINGACCOUNTID = -1;
            SHARENUMBER = 0.0;
            SHAREPRICE = 0.0;
            SHARECOMMISSION = 0.0;
        }

        explicit Data(wxSQLite3ResultSet& q, Self* table = 0)
        {
            table_ = table;
        
            SHAREINFOID = q.GetInt(0); // SHAREINFOID
            CHECKINGACCOUNTID = q.GetInt(1); // CHECKINGACCOUNTID
            SHARENUMBER = q.GetDouble(2); // SHARENUMBER
            SHAREPRICE = q.GetDouble(3); // SHAREPRICE
            SHARECOMMISSION = q.GetDouble(4); // SHARECOMMISSION
            SHARELOT = q.GetString(5); // SHARELOT
        }

        Data& operator=(const Data& other)
        {
            if (this == &other) return *this;

            SHAREINFOID = other.SHAREINFOID;
            CHECKINGACCOUNTID = other.CHECKINGACCOUNTID;
            SHARENUMBER = other.SHARENUMBER;
            SHAREPRICE = other.SHAREPRICE;
            SHARECOMMISSION = other.SHARECOMMISSION;
            SHARELOT = other.SHARELOT;
            return *this;
        }

        template<typename C>
        bool match(const C &c) const
        {
            return false;
        }

        bool match(const Self::SHAREINFOID &in) const
        {
            return this->SHAREINFOID == in.v_;
        }

        bool match(const Self::CHECKINGACCOUNTID &in) const
        {
            return this->CHECKINGACCOUNTID == in.v_;
        }

        bool match(const Self::SHARENUMBER &in) const
        {
            return this->SHARENUMBER == in.v_;
        }

        bool match(const Self::SHAREPRICE &in) const
        {
            return this->SHAREPRICE == in.v_;
        }

        bool match(const Self::SHARECOMMISSION &in) const
        {
            return this->SHARECOMMISSION == in.v_;
        }

        bool match(const Self::SHARELOT &in) const
        {
            return this->SHARELOT.CmpNoCase(in.v_) == 0;
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
            json_writer.Key("SHAREINFOID");
            json_writer.Int(this->SHAREINFOID);
            json_writer.Key("CHECKINGACCOUNTID");
            json_writer.Int(this->CHECKINGACCOUNTID);
            json_writer.Key("SHARENUMBER");
            json_writer.Double(this->SHARENUMBER);
            json_writer.Key("SHAREPRICE");
            json_writer.Double(this->SHAREPRICE);
            json_writer.Key("SHARECOMMISSION");
            json_writer.Double(this->SHARECOMMISSION);
            json_writer.Key("SHARELOT");
            json_writer.String(this->SHARELOT.utf8_str());
        }

        row_t to_row_t() const
        {
            row_t row;
            row(L"SHAREINFOID") = SHAREINFOID;
            row(L"CHECKINGACCOUNTID") = CHECKINGACCOUNTID;
            row(L"SHARENUMBER") = SHARENUMBER;
            row(L"SHAREPRICE") = SHAREPRICE;
            row(L"SHARECOMMISSION") = SHARECOMMISSION;
            row(L"SHARELOT") = SHARELOT;
            return row;
        }

        void to_template(html_template& t) const
        {
            t(L"SHAREINFOID") = SHAREINFOID;
            t(L"CHECKINGACCOUNTID") = CHECKINGACCOUNTID;
            t(L"SHARENUMBER") = SHARENUMBER;
            t(L"SHAREPRICE") = SHAREPRICE;
            t(L"SHARECOMMISSION") = SHARECOMMISSION;
            t(L"SHARELOT") = SHARELOT;
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db)
        {
            if (db && db->IsReadOnly()) return false;
            if (!table_ || !db) 
            {
                wxLogError("can not save SHAREINFO_V1");
                return false;
            }

            return table_->save(this, db);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!table_ || !db) 
            {
                wxLogError("can not remove SHAREINFO_V1");
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
        NUM_COLUMNS = 6
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    /** Name of the table*/    
    wxString name() const { return "SHAREINFO_V1"; }

    DB_Table_SHAREINFO_V1() : fake_(new Data())
    {
        query_ = "SELECT SHAREINFOID, CHECKINGACCOUNTID, SHARENUMBER, SHAREPRICE, SHARECOMMISSION, SHARELOT FROM SHAREINFO_V1 ";
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
            sql = "INSERT INTO SHAREINFO_V1(CHECKINGACCOUNTID, SHARENUMBER, SHAREPRICE, SHARECOMMISSION, SHARELOT) VALUES(?, ?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE SHAREINFO_V1 SET CHECKINGACCOUNTID = ?, SHARENUMBER = ?, SHAREPRICE = ?, SHARECOMMISSION = ?, SHARELOT = ? WHERE SHAREINFOID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->CHECKINGACCOUNTID);
            stmt.Bind(2, entity->SHARENUMBER);
            stmt.Bind(3, entity->SHAREPRICE);
            stmt.Bind(4, entity->SHARECOMMISSION);
            stmt.Bind(5, entity->SHARELOT);
            if (entity->id() > 0)
                stmt.Bind(6, entity->SHAREINFOID);

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
            wxLogError("SHAREINFO_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
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
            wxString sql = "DELETE FROM SHAREINFO_V1 WHERE SHAREINFOID = ?";
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
            wxLogError("SHAREINFO_V1: Exception %s", e.GetMessage().utf8_str());
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

