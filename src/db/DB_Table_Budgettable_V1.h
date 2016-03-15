// -*- C++ -*-
//=============================================================================
/**
 *      Copyright (c) 2013 - 2016 Guan Lisheng (guanlisheng@gmail.com)
 *
 *      @file
 *
 *      @author [sqlite2cpp.py]
 *
 *      @brief
 *
 *      Revision History:
 *          AUTO GENERATED at 2016-03-15 17:43:58.503181.
 *          DO NOT EDIT!
 */
//=============================================================================

#ifndef DB_TABLE_BUDGETTABLE_V1_H
#define DB_TABLE_BUDGETTABLE_V1_H

#include "DB_Table.h"

struct DB_Table_BUDGETTABLE_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_BUDGETTABLE_V1 Self;
    /** A container to hold list of Data records for the table*/
    struct Data_Set : public std::vector<Self::Data>
    {
        std::wstring to_json() const
        {
            json::Array a;
            for (const auto & item: *this)
            {
                json::Object o;
                item.to_json(o);
                a.Insert(o);
            }
            std::wstringstream ss;
            json::Writer::Write(a, ss);
            return ss.str();
        }
    };
    /** A container to hold a list of Data record pointers for the table in memory*/
    typedef std::vector<Self::Data*> Cache;
    typedef std::map<int, Self::Data*> Index_By_Id;
    Cache cache_;
    Index_By_Id index_by_id_;
    Data* fake_; // in case the entity not found

    /** Destructor: clears any data records stored in memory */
    ~DB_Table_BUDGETTABLE_V1() 
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
                db->ExecuteUpdate("CREATE TABLE BUDGETTABLE_V1(BUDGETENTRYID integer primary key, BUDGETYEARID integer, CATEGID integer, SUBCATEGID integer, PERIOD TEXT NOT NULL /* None, Weekly, Bi-Weekly, Monthly, Monthly, Bi-Monthly, Quarterly, Half-Yearly, Yearly, Daily*/, AMOUNT numeric NOT NULL)");
                this->ensure_data(db);
            }
            catch(const wxSQLite3Exception &e) 
            { 
                wxLogError("BUDGETTABLE_V1: Exception %s", e.GetMessage().c_str());
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
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_BUDGETTABLE_BUDGETYEARID ON BUDGETTABLE_V1(BUDGETYEARID)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("BUDGETTABLE_V1: Exception %s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }

    void ensure_data(wxSQLite3Database* db)
    {
        db->Begin();
        db->Commit();
    }
    
    struct BUDGETENTRYID : public DB_Column<int>
    { 
        static wxString name() { return "BUDGETENTRYID"; } 
        explicit BUDGETENTRYID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct BUDGETYEARID : public DB_Column<int>
    { 
        static wxString name() { return "BUDGETYEARID"; } 
        explicit BUDGETYEARID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
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
    struct PERIOD : public DB_Column<wxString>
    { 
        static wxString name() { return "PERIOD"; } 
        explicit PERIOD(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct AMOUNT : public DB_Column<double>
    { 
        static wxString name() { return "AMOUNT"; } 
        explicit AMOUNT(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    typedef BUDGETENTRYID PRIMARY;
    enum COLUMN
    {
        COL_BUDGETENTRYID = 0
        , COL_BUDGETYEARID = 1
        , COL_CATEGID = 2
        , COL_SUBCATEGID = 3
        , COL_PERIOD = 4
        , COL_AMOUNT = 5
    };

    /** Returns the column name as a string*/
    static wxString column_to_name(COLUMN col)
    {
        switch(col)
        {
            case COL_BUDGETENTRYID: return "BUDGETENTRYID";
            case COL_BUDGETYEARID: return "BUDGETYEARID";
            case COL_CATEGID: return "CATEGID";
            case COL_SUBCATEGID: return "SUBCATEGID";
            case COL_PERIOD: return "PERIOD";
            case COL_AMOUNT: return "AMOUNT";
            default: break;
        }
        
        return "UNKNOWN";
    }

    /** Returns the column number from the given column name*/
    static COLUMN name_to_column(const wxString& name)
    {
        if ("BUDGETENTRYID" == name) return COL_BUDGETENTRYID;
        else if ("BUDGETYEARID" == name) return COL_BUDGETYEARID;
        else if ("CATEGID" == name) return COL_CATEGID;
        else if ("SUBCATEGID" == name) return COL_SUBCATEGID;
        else if ("PERIOD" == name) return COL_PERIOD;
        else if ("AMOUNT" == name) return COL_AMOUNT;

        return COLUMN(-1);
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_BUDGETTABLE_V1;
        /** This is a instance pointer to itself in memory. */
        Self* table_;
    
        int BUDGETENTRYID;//  primary key
        int BUDGETYEARID;
        int CATEGID;
        int SUBCATEGID;
        wxString PERIOD;
        double AMOUNT;
        int id() const { return BUDGETENTRYID; }
        void id(int id) { BUDGETENTRYID = id; }
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
        
            BUDGETENTRYID = -1;
            BUDGETYEARID = -1;
            CATEGID = -1;
            SUBCATEGID = -1;
            AMOUNT = 0.0;
        }

        explicit Data(wxSQLite3ResultSet& q, Self* table = 0)
        {
            table_ = table;
        
            BUDGETENTRYID = q.GetInt(0); // BUDGETENTRYID
            BUDGETYEARID = q.GetInt(1); // BUDGETYEARID
            CATEGID = q.GetInt(2); // CATEGID
            SUBCATEGID = q.GetInt(3); // SUBCATEGID
            PERIOD = q.GetString(4); // PERIOD
            AMOUNT = q.GetDouble(5); // AMOUNT
        }

        Data& operator=(const Data& other)
        {
            if (this == &other) return *this;

            BUDGETENTRYID = other.BUDGETENTRYID;
            BUDGETYEARID = other.BUDGETYEARID;
            CATEGID = other.CATEGID;
            SUBCATEGID = other.SUBCATEGID;
            PERIOD = other.PERIOD;
            AMOUNT = other.AMOUNT;
            return *this;
        }

        template<typename C>
        bool match(const C &c) const
        {
            return false;
        }
        bool match(const Self::BUDGETENTRYID &in) const
        {
            return this->BUDGETENTRYID == in.v_;
        }
        bool match(const Self::BUDGETYEARID &in) const
        {
            return this->BUDGETYEARID == in.v_;
        }
        bool match(const Self::CATEGID &in) const
        {
            return this->CATEGID == in.v_;
        }
        bool match(const Self::SUBCATEGID &in) const
        {
            return this->SUBCATEGID == in.v_;
        }
        bool match(const Self::PERIOD &in) const
        {
            return this->PERIOD.CmpNoCase(in.v_) == 0;
        }
        bool match(const Self::AMOUNT &in) const
        {
            return this->AMOUNT == in.v_;
        }
        wxString to_json() const
        {
            json::Object o;
            this->to_json(o);
            std::wstringstream ss;
            json::Writer::Write(o, ss);
            return ss.str();
        }
        
        int to_json(json::Object& o) const
        {
            o[L"BUDGETENTRYID"] = json::Number(this->BUDGETENTRYID);
            o[L"BUDGETYEARID"] = json::Number(this->BUDGETYEARID);
            o[L"CATEGID"] = json::Number(this->CATEGID);
            o[L"SUBCATEGID"] = json::Number(this->SUBCATEGID);
            o[L"PERIOD"] = json::String(this->PERIOD.ToStdWstring());
            o[L"AMOUNT"] = json::Number(this->AMOUNT);
            return 0;
        }
        row_t to_row_t() const
        {
            row_t row;
            row(L"BUDGETENTRYID") = BUDGETENTRYID;
            row(L"BUDGETYEARID") = BUDGETYEARID;
            row(L"CATEGID") = CATEGID;
            row(L"SUBCATEGID") = SUBCATEGID;
            row(L"PERIOD") = PERIOD;
            row(L"AMOUNT") = AMOUNT;
            return row;
        }
        void to_template(html_template& t) const
        {
            t(L"BUDGETENTRYID") = BUDGETENTRYID;
            t(L"BUDGETYEARID") = BUDGETYEARID;
            t(L"CATEGID") = CATEGID;
            t(L"SUBCATEGID") = SUBCATEGID;
            t(L"PERIOD") = PERIOD;
            t(L"AMOUNT") = AMOUNT;
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db)
        {
            if (db && db->IsReadOnly()) return false;
            if (!table_ || !db) 
            {
                wxLogError("can not save BUDGETTABLE_V1");
                return false;
            }

            return table_->save(this, db);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!table_ || !db) 
            {
                wxLogError("can not remove BUDGETTABLE_V1");
                return false;
            }
            
            return table_->remove(this, db);
        }

        void destroy()
        {
            //if (this->id() < 0)
            //    wxSafeShowMessage("unsaved object", this->to_json());
            delete this;
        }
    };

    enum
    {
        NUM_COLUMNS = 6
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    /** Name of the table*/    
    wxString name() const { return "BUDGETTABLE_V1"; }

    DB_Table_BUDGETTABLE_V1() : fake_(new Data())
    {
        query_ = "SELECT BUDGETENTRYID, BUDGETYEARID, CATEGID, SUBCATEGID, PERIOD, AMOUNT FROM BUDGETTABLE_V1 ";
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
            sql = "INSERT INTO BUDGETTABLE_V1(BUDGETYEARID, CATEGID, SUBCATEGID, PERIOD, AMOUNT) VALUES(?, ?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE BUDGETTABLE_V1 SET BUDGETYEARID = ?, CATEGID = ?, SUBCATEGID = ?, PERIOD = ?, AMOUNT = ? WHERE BUDGETENTRYID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->BUDGETYEARID);
            stmt.Bind(2, entity->CATEGID);
            stmt.Bind(3, entity->SUBCATEGID);
            stmt.Bind(4, entity->PERIOD);
            stmt.Bind(5, entity->AMOUNT);
            if (entity->id() > 0)
                stmt.Bind(6, entity->BUDGETENTRYID);

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
            wxLogError("BUDGETTABLE_V1: Exception %s, %s", e.GetMessage().c_str(), entity->to_json());
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
            wxString sql = "DELETE FROM BUDGETTABLE_V1 WHERE BUDGETENTRYID = ?";
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
            wxLogError("BUDGETTABLE_V1: Exception %s", e.GetMessage().c_str());
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
#endif //
