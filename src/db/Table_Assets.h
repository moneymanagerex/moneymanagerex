// -*- C++ -*-
/** @file
 * @brief     CRUD implementation for ASSETS SQLite table
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

struct DB_Table_ASSETS : public DB_Table
{
    struct Data;
    typedef DB_Table_ASSETS Self;

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
    ~DB_Table_ASSETS() 
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
                db->ExecuteUpdate("CREATE TABLE ASSETS(ASSETID integer primary key, STARTDATE TEXT NOT NULL, ASSETNAME TEXT COLLATE NOCASE NOT NULL, VALUE numeric, VALUECHANGE TEXT /* None, Appreciates, Depreciates */, NOTES TEXT, VALUECHANGERATE numeric, ASSETTYPE TEXT /* Property, Automobile, Household Object, Art, Jewellery, Cash, Other */)");
                this->ensure_data(db);
            }
            catch(const wxSQLite3Exception &e) 
            { 
                wxLogError("ASSETS: Exception %s", e.GetMessage().c_str());
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
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_ASSETS_ASSETTYPE ON ASSETS(ASSETTYPE)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("ASSETS: Exception %s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }

    void ensure_data(wxSQLite3Database* db)
    {
        db->Begin();
        db->Commit();
    }
    
    struct ASSETID : public DB_Column<int>
    { 
        static wxString name() { return "ASSETID"; } 
        explicit ASSETID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct STARTDATE : public DB_Column<wxString>
    { 
        static wxString name() { return "STARTDATE"; } 
        explicit STARTDATE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct ASSETNAME : public DB_Column<wxString>
    { 
        static wxString name() { return "ASSETNAME"; } 
        explicit ASSETNAME(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct VALUE : public DB_Column<double>
    { 
        static wxString name() { return "VALUE"; } 
        explicit VALUE(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    
    struct VALUECHANGE : public DB_Column<wxString>
    { 
        static wxString name() { return "VALUECHANGE"; } 
        explicit VALUECHANGE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct NOTES : public DB_Column<wxString>
    { 
        static wxString name() { return "NOTES"; } 
        explicit NOTES(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct VALUECHANGERATE : public DB_Column<double>
    { 
        static wxString name() { return "VALUECHANGERATE"; } 
        explicit VALUECHANGERATE(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    
    struct ASSETTYPE : public DB_Column<wxString>
    { 
        static wxString name() { return "ASSETTYPE"; } 
        explicit ASSETTYPE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    typedef ASSETID PRIMARY;
    enum COLUMN
    {
        COL_ASSETID = 0
        , COL_STARTDATE = 1
        , COL_ASSETNAME = 2
        , COL_VALUE = 3
        , COL_VALUECHANGE = 4
        , COL_NOTES = 5
        , COL_VALUECHANGERATE = 6
        , COL_ASSETTYPE = 7
        , COL_UNKNOWN = -1
    };

    /** Returns the column name as a string*/
    static wxString column_to_name(COLUMN col)
    {
        switch(col)
        {
            case COL_ASSETID: return "ASSETID";
            case COL_STARTDATE: return "STARTDATE";
            case COL_ASSETNAME: return "ASSETNAME";
            case COL_VALUE: return "VALUE";
            case COL_VALUECHANGE: return "VALUECHANGE";
            case COL_NOTES: return "NOTES";
            case COL_VALUECHANGERATE: return "VALUECHANGERATE";
            case COL_ASSETTYPE: return "ASSETTYPE";
            default: break;
        }
        
        return "UNKNOWN";
    }

    /** Returns the column number from the given column name*/
    static COLUMN name_to_column(const wxString& name)
    {
        if ("ASSETID" == name) return COL_ASSETID;
        else if ("STARTDATE" == name) return COL_STARTDATE;
        else if ("ASSETNAME" == name) return COL_ASSETNAME;
        else if ("VALUE" == name) return COL_VALUE;
        else if ("VALUECHANGE" == name) return COL_VALUECHANGE;
        else if ("NOTES" == name) return COL_NOTES;
        else if ("VALUECHANGERATE" == name) return COL_VALUECHANGERATE;
        else if ("ASSETTYPE" == name) return COL_ASSETTYPE;

        return COL_UNKNOWN;
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_ASSETS;
        /** This is a instance pointer to itself in memory. */
        Self* table_;
    
        int ASSETID; // primary key
        wxString STARTDATE;
        wxString ASSETNAME;
        double VALUE;
        wxString VALUECHANGE;
        wxString NOTES;
        double VALUECHANGERATE;
        wxString ASSETTYPE;

        int id() const
        {
            return ASSETID;
        }

        void id(int id)
        {
            ASSETID = id;
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
        
            ASSETID = -1;
            VALUE = 0.0;
            VALUECHANGERATE = 0.0;
        }

        explicit Data(wxSQLite3ResultSet& q, Self* table = 0)
        {
            table_ = table;
        
            ASSETID = q.GetInt(0);
            STARTDATE = q.GetString(1);
            ASSETNAME = q.GetString(2);
            VALUE = q.GetDouble(3);
            VALUECHANGE = q.GetString(4);
            NOTES = q.GetString(5);
            VALUECHANGERATE = q.GetDouble(6);
            ASSETTYPE = q.GetString(7);
        }

        Data& operator=(const Data& other)
        {
            if (this == &other) return *this;

            ASSETID = other.ASSETID;
            STARTDATE = other.STARTDATE;
            ASSETNAME = other.ASSETNAME;
            VALUE = other.VALUE;
            VALUECHANGE = other.VALUECHANGE;
            NOTES = other.NOTES;
            VALUECHANGERATE = other.VALUECHANGERATE;
            ASSETTYPE = other.ASSETTYPE;
            return *this;
        }


        bool match(const Self::ASSETID &in) const
        {
            return this->ASSETID == in.v_;
        }

        bool match(const Self::STARTDATE &in) const
        {
            return this->STARTDATE.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::ASSETNAME &in) const
        {
            return this->ASSETNAME.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::VALUE &in) const
        {
            return this->VALUE == in.v_;
        }

        bool match(const Self::VALUECHANGE &in) const
        {
            return this->VALUECHANGE.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::NOTES &in) const
        {
            return this->NOTES.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::VALUECHANGERATE &in) const
        {
            return this->VALUECHANGERATE == in.v_;
        }

        bool match(const Self::ASSETTYPE &in) const
        {
            return this->ASSETTYPE.CmpNoCase(in.v_) == 0;
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
            json_writer.Key("ASSETID");
            json_writer.Int(this->ASSETID);
            json_writer.Key("STARTDATE");
            json_writer.String(this->STARTDATE.c_str());
            json_writer.Key("ASSETNAME");
            json_writer.String(this->ASSETNAME.c_str());
            json_writer.Key("VALUE");
            json_writer.Double(this->VALUE);
            json_writer.Key("VALUECHANGE");
            json_writer.String(this->VALUECHANGE.c_str());
            json_writer.Key("NOTES");
            json_writer.String(this->NOTES.c_str());
            json_writer.Key("VALUECHANGERATE");
            json_writer.Double(this->VALUECHANGERATE);
            json_writer.Key("ASSETTYPE");
            json_writer.String(this->ASSETTYPE.c_str());
        }

        row_t to_row_t() const
        {
            row_t row;
            row(L"ASSETID") = ASSETID;
            row(L"STARTDATE") = STARTDATE;
            row(L"ASSETNAME") = ASSETNAME;
            row(L"VALUE") = VALUE;
            row(L"VALUECHANGE") = VALUECHANGE;
            row(L"NOTES") = NOTES;
            row(L"VALUECHANGERATE") = VALUECHANGERATE;
            row(L"ASSETTYPE") = ASSETTYPE;
            return row;
        }

        void to_template(html_template& t) const
        {
            t(L"ASSETID") = ASSETID;
            t(L"STARTDATE") = STARTDATE;
            t(L"ASSETNAME") = ASSETNAME;
            t(L"VALUE") = VALUE;
            t(L"VALUECHANGE") = VALUECHANGE;
            t(L"NOTES") = NOTES;
            t(L"VALUECHANGERATE") = VALUECHANGERATE;
            t(L"ASSETTYPE") = ASSETTYPE;
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db)
        {
            if (db && db->IsReadOnly()) return false;
            if (!table_ || !db) 
            {
                wxLogError("can not save ASSETS");
                return false;
            }

            return table_->save(this, db);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!table_ || !db) 
            {
                wxLogError("can not remove ASSETS");
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

    /** Name of the table */
    wxString name() const { return "ASSETS"; }

    DB_Table_ASSETS() : fake_(new Data())
    {
        query_ = "SELECT ASSETID, STARTDATE, ASSETNAME, VALUE, VALUECHANGE, NOTES, VALUECHANGERATE, ASSETTYPE FROM ASSETS ";
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
            sql = "INSERT INTO ASSETS(STARTDATE, ASSETNAME, VALUE, VALUECHANGE, NOTES, VALUECHANGERATE, ASSETTYPE) VALUES(?, ?, ?, ?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE ASSETS SET STARTDATE = ?, ASSETNAME = ?, VALUE = ?, VALUECHANGE = ?, NOTES = ?, VALUECHANGERATE = ?, ASSETTYPE = ? WHERE ASSETID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->STARTDATE);
            stmt.Bind(2, entity->ASSETNAME);
            stmt.Bind(3, entity->VALUE);
            stmt.Bind(4, entity->VALUECHANGE);
            stmt.Bind(5, entity->NOTES);
            stmt.Bind(6, entity->VALUECHANGERATE);
            stmt.Bind(7, entity->ASSETTYPE);
            if (entity->id() > 0)
                stmt.Bind(8, entity->ASSETID);

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
            wxLogError("ASSETS: Exception %s, %s", e.GetMessage().c_str(), entity->to_json());
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
            wxString sql = "DELETE FROM ASSETS WHERE ASSETID = ?";
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
            wxLogError("ASSETS: Exception %s", e.GetMessage().c_str());
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

