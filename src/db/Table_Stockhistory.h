// -*- C++ -*-
/** @file
 * @brief     CRUD implementation for STOCKHISTORY SQLite table
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

struct DB_Table_STOCKHISTORY : public DB_Table
{
    struct Data;
    typedef DB_Table_STOCKHISTORY Self;

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
    ~DB_Table_STOCKHISTORY() 
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
                db->ExecuteUpdate("CREATE TABLE STOCKHISTORY(HISTID integer primary key, SYMBOL TEXT NOT NULL, DATE TEXT NOT NULL, VALUE numeric NOT NULL, UPDTYPE integer, UNIQUE(SYMBOL, DATE))");
                this->ensure_data(db);
            }
            catch(const wxSQLite3Exception &e) 
            { 
                wxLogError("STOCKHISTORY: Exception %s", e.GetMessage().c_str());
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
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_STOCKHISTORY_SYMBOL ON STOCKHISTORY(SYMBOL)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("STOCKHISTORY: Exception %s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }

    void ensure_data(wxSQLite3Database* db)
    {
        db->Begin();
        db->Commit();
    }
    
    struct HISTID : public DB_Column<int>
    { 
        static wxString name() { return "HISTID"; } 
        explicit HISTID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct SYMBOL : public DB_Column<wxString>
    { 
        static wxString name() { return "SYMBOL"; } 
        explicit SYMBOL(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct DATE : public DB_Column<wxString>
    { 
        static wxString name() { return "DATE"; } 
        explicit DATE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct VALUE : public DB_Column<double>
    { 
        static wxString name() { return "VALUE"; } 
        explicit VALUE(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    
    struct UPDTYPE : public DB_Column<int>
    { 
        static wxString name() { return "UPDTYPE"; } 
        explicit UPDTYPE(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    typedef HISTID PRIMARY;
    enum COLUMN
    {
        COL_HISTID = 0
        , COL_SYMBOL = 1
        , COL_DATE = 2
        , COL_VALUE = 3
        , COL_UPDTYPE = 4
        , COL_UNKNOWN = -1
    };

    /** Returns the column name as a string*/
    static wxString column_to_name(COLUMN col)
    {
        switch(col)
        {
            case COL_HISTID: return "HISTID";
            case COL_SYMBOL: return "SYMBOL";
            case COL_DATE: return "DATE";
            case COL_VALUE: return "VALUE";
            case COL_UPDTYPE: return "UPDTYPE";
            default: break;
        }
        
        return "UNKNOWN";
    }

    /** Returns the column number from the given column name*/
    static COLUMN name_to_column(const wxString& name)
    {
        if ("HISTID" == name) return COL_HISTID;
        else if ("SYMBOL" == name) return COL_SYMBOL;
        else if ("DATE" == name) return COL_DATE;
        else if ("VALUE" == name) return COL_VALUE;
        else if ("UPDTYPE" == name) return COL_UPDTYPE;

        return COL_UNKNOWN;
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_STOCKHISTORY;
        /** This is a instance pointer to itself in memory. */
        Self* table_;
    
        int HISTID; // primary key
        wxString SYMBOL;
        wxString DATE;
        double VALUE;
        int UPDTYPE;

        int id() const
        {
            return HISTID;
        }

        void id(int id)
        {
            HISTID = id;
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
        
            HISTID = -1;
            VALUE = 0.0;
            UPDTYPE = -1;
        }

        explicit Data(wxSQLite3ResultSet& q, Self* table = 0)
        {
            table_ = table;
        
            HISTID = q.GetInt(0);
            SYMBOL = q.GetString(1);
            DATE = q.GetString(2);
            VALUE = q.GetDouble(3);
            UPDTYPE = q.GetInt(4);
        }

        Data& operator=(const Data& other)
        {
            if (this == &other) return *this;

            HISTID = other.HISTID;
            SYMBOL = other.SYMBOL;
            DATE = other.DATE;
            VALUE = other.VALUE;
            UPDTYPE = other.UPDTYPE;
            return *this;
        }


        bool match(const Self::HISTID &in) const
        {
            return this->HISTID == in.v_;
        }

        bool match(const Self::SYMBOL &in) const
        {
            return this->SYMBOL.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::DATE &in) const
        {
            return this->DATE.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::VALUE &in) const
        {
            return this->VALUE == in.v_;
        }

        bool match(const Self::UPDTYPE &in) const
        {
            return this->UPDTYPE == in.v_;
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
            json_writer.Key("HISTID");
            json_writer.Int(this->HISTID);
            json_writer.Key("SYMBOL");
            json_writer.String(this->SYMBOL.c_str());
            json_writer.Key("DATE");
            json_writer.String(this->DATE.c_str());
            json_writer.Key("VALUE");
            json_writer.Double(this->VALUE);
            json_writer.Key("UPDTYPE");
            json_writer.Int(this->UPDTYPE);
        }

        row_t to_row_t() const
        {
            row_t row;
            row(L"HISTID") = HISTID;
            row(L"SYMBOL") = SYMBOL;
            row(L"DATE") = DATE;
            row(L"VALUE") = VALUE;
            row(L"UPDTYPE") = UPDTYPE;
            return row;
        }

        void to_template(html_template& t) const
        {
            t(L"HISTID") = HISTID;
            t(L"SYMBOL") = SYMBOL;
            t(L"DATE") = DATE;
            t(L"VALUE") = VALUE;
            t(L"UPDTYPE") = UPDTYPE;
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db)
        {
            if (db && db->IsReadOnly()) return false;
            if (!table_ || !db) 
            {
                wxLogError("can not save STOCKHISTORY");
                return false;
            }

            return table_->save(this, db);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!table_ || !db) 
            {
                wxLogError("can not remove STOCKHISTORY");
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

    /** Name of the table */
    wxString name() const { return "STOCKHISTORY"; }

    DB_Table_STOCKHISTORY() : fake_(new Data())
    {
        query_ = "SELECT HISTID, SYMBOL, DATE, VALUE, UPDTYPE FROM STOCKHISTORY ";
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
            sql = "INSERT INTO STOCKHISTORY(SYMBOL, DATE, VALUE, UPDTYPE) VALUES(?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE STOCKHISTORY SET SYMBOL = ?, DATE = ?, VALUE = ?, UPDTYPE = ? WHERE HISTID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->SYMBOL);
            stmt.Bind(2, entity->DATE);
            stmt.Bind(3, entity->VALUE);
            stmt.Bind(4, entity->UPDTYPE);
            if (entity->id() > 0)
                stmt.Bind(5, entity->HISTID);

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
            wxLogError("STOCKHISTORY: Exception %s, %s", e.GetMessage().c_str(), entity->to_json());
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
            wxString sql = "DELETE FROM STOCKHISTORY WHERE HISTID = ?";
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
            wxLogError("STOCKHISTORY: Exception %s", e.GetMessage().c_str());
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

