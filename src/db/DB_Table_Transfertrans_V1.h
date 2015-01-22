// -*- C++ -*-
//=============================================================================
/**
 *      Copyright (c) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 *
 *      @file
 *
 *      @author [sqlite2cpp.py]
 *
 *      @brief
 *
 *      Revision History:
 *          AUTO GENERATED at 2015-01-22 15:13:14.809000.
 *          DO NOT EDIT!
 */
//=============================================================================

#ifndef DB_TABLE_TRANSFERTRANS_V1_H
#define DB_TABLE_TRANSFERTRANS_V1_H

#include "DB_Table.h"

struct DB_Table_TRANSFERTRANS_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_TRANSFERTRANS_V1 Self;
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

    /** Destructor: clears any data records stored in memory */
    ~DB_Table_TRANSFERTRANS_V1() 
    {
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
				db->ExecuteUpdate("CREATE TABLE TRANSFERTRANS_V1 (ID INTEGER NOT NULL PRIMARY KEY, TABLE_TYPE TEXT NOT NULL UNIQUE, ID_TABLE INTEGER NOT NULL, ID_CHECKINGACCOUNT INTEGER NOT NULL, ID_CURRENCY INTEGER NOT NULL)");
			}
			catch(const wxSQLite3Exception &e) 
			{ 
				wxLogError("TRANSFERTRANS_V1: Exception %s", e.GetMessage().c_str());
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
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_ID_CHECKINGACCOUNT ON TRANSFERTRANS_V1 (ID_CHECKINGACCOUNT)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("TRANSFERTRANS_V1: Exception %s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }

    struct ID : public DB_Column<int>
    { 
        static wxString name() { return "ID"; } 
        explicit ID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct TABLE_TYPE : public DB_Column<wxString>
    { 
        static wxString name() { return "TABLE_TYPE"; } 
        explicit TABLE_TYPE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct ID_TABLE : public DB_Column<int>
    { 
        static wxString name() { return "ID_TABLE"; } 
        explicit ID_TABLE(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct ID_CHECKINGACCOUNT : public DB_Column<int>
    { 
        static wxString name() { return "ID_CHECKINGACCOUNT"; } 
        explicit ID_CHECKINGACCOUNT(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct ID_CURRENCY : public DB_Column<int>
    { 
        static wxString name() { return "ID_CURRENCY"; } 
        explicit ID_CURRENCY(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    typedef ID PRIMARY;
    enum COLUMN
    {
        COL_ID = 0
        , COL_TABLE_TYPE = 1
        , COL_ID_TABLE = 2
        , COL_ID_CHECKINGACCOUNT = 3
        , COL_ID_CURRENCY = 4
    };

    /** Returns the column name as a string*/
    static wxString column_to_name(COLUMN col)
    {
        switch(col)
        {
            case COL_ID: return "ID";
            case COL_TABLE_TYPE: return "TABLE_TYPE";
            case COL_ID_TABLE: return "ID_TABLE";
            case COL_ID_CHECKINGACCOUNT: return "ID_CHECKINGACCOUNT";
            case COL_ID_CURRENCY: return "ID_CURRENCY";
            default: break;
        }
        
        return "UNKNOWN";
    }

    /** Returns the column number from the given column name*/
    static COLUMN name_to_column(const wxString& name)
    {
        if ("ID" == name) return COL_ID;
        else if ("TABLE_TYPE" == name) return COL_TABLE_TYPE;
        else if ("ID_TABLE" == name) return COL_ID_TABLE;
        else if ("ID_CHECKINGACCOUNT" == name) return COL_ID_CHECKINGACCOUNT;
        else if ("ID_CURRENCY" == name) return COL_ID_CURRENCY;

        return COLUMN(-1);
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_TRANSFERTRANS_V1;
        /** This is a instance pointer to itself in memory. */
        Self* view_;
    
        int ID;//  primary key
        wxString TABLE_TYPE;
        int ID_TABLE;
        int ID_CHECKINGACCOUNT;
        int ID_CURRENCY;
        int id() const { return ID; }
        void id(int id) { ID = id; }
        bool operator < (const Data& r) const
        {
            return this->id() < r.id();
        }
        bool operator < (const Data* r) const
        {
            return this->id() < r->id();
        }

        explicit Data(Self* view = 0) 
        {
            view_ = view;
        
            ID = -1;
            ID_TABLE = -1;
            ID_CHECKINGACCOUNT = -1;
            ID_CURRENCY = -1;
        }

        explicit Data(wxSQLite3ResultSet& q, Self* view = 0)
        {
            view_ = view;
        
            ID = q.GetInt(0); // ID
            TABLE_TYPE = q.GetString(1); // TABLE_TYPE
            ID_TABLE = q.GetInt(2); // ID_TABLE
            ID_CHECKINGACCOUNT = q.GetInt(3); // ID_CHECKINGACCOUNT
            ID_CURRENCY = q.GetInt(4); // ID_CURRENCY
        }

        Data& operator=(const Data& other)
        {
            if (this == &other) return *this;

            ID = other.ID;
            TABLE_TYPE = other.TABLE_TYPE;
            ID_TABLE = other.ID_TABLE;
            ID_CHECKINGACCOUNT = other.ID_CHECKINGACCOUNT;
            ID_CURRENCY = other.ID_CURRENCY;
            return *this;
        }

        template<typename C>
        bool match(const C &c) const
        {
            return false;
        }
        bool match(const Self::ID &in) const
        {
            return this->ID == in.v_;
        }
        bool match(const Self::TABLE_TYPE &in) const
        {
            return this->TABLE_TYPE.CmpNoCase(in.v_) == 0;
        }
        bool match(const Self::ID_TABLE &in) const
        {
            return this->ID_TABLE == in.v_;
        }
        bool match(const Self::ID_CHECKINGACCOUNT &in) const
        {
            return this->ID_CHECKINGACCOUNT == in.v_;
        }
        bool match(const Self::ID_CURRENCY &in) const
        {
            return this->ID_CURRENCY == in.v_;
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
            o[L"ID"] = json::Number(this->ID);
            o[L"TABLE_TYPE"] = json::String(this->TABLE_TYPE.ToStdWstring());
            o[L"ID_TABLE"] = json::Number(this->ID_TABLE);
            o[L"ID_CHECKINGACCOUNT"] = json::Number(this->ID_CHECKINGACCOUNT);
            o[L"ID_CURRENCY"] = json::Number(this->ID_CURRENCY);
            return 0;
        }
        row_t to_row_t() const
        {
            row_t row;
            row(L"ID") = ID;
            row(L"TABLE_TYPE") = TABLE_TYPE;
            row(L"ID_TABLE") = ID_TABLE;
            row(L"ID_CHECKINGACCOUNT") = ID_CHECKINGACCOUNT;
            row(L"ID_CURRENCY") = ID_CURRENCY;
            return row;
        }
        void to_template(html_template& t) const
        {
            t(L"ID") = ID;
            t(L"TABLE_TYPE") = TABLE_TYPE;
            t(L"ID_TABLE") = ID_TABLE;
            t(L"ID_CHECKINGACCOUNT") = ID_CHECKINGACCOUNT;
            t(L"ID_CURRENCY") = ID_CURRENCY;
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db)
        {
            if (db && db->IsReadOnly()) return false;
            if (!view_ || !db) 
            {
                wxLogError("can not save TRANSFERTRANS_V1");
                return false;
            }

            return view_->save(this, db);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!view_ || !db) 
            {
                wxLogError("can not remove TRANSFERTRANS_V1");
                return false;
            }
            
            return view_->remove(this, db);
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
        NUM_COLUMNS = 5
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    /** Name of the table*/    
    wxString name() const { return "TRANSFERTRANS_V1"; }

    DB_Table_TRANSFERTRANS_V1() 
    {
        query_ = "SELECT * FROM TRANSFERTRANS_V1 ";
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
            sql = "INSERT INTO TRANSFERTRANS_V1(TABLE_TYPE, ID_TABLE, ID_CHECKINGACCOUNT, ID_CURRENCY) VALUES(?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE TRANSFERTRANS_V1 SET TABLE_TYPE = ?, ID_TABLE = ?, ID_CHECKINGACCOUNT = ?, ID_CURRENCY = ? WHERE ID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->TABLE_TYPE);
            stmt.Bind(2, entity->ID_TABLE);
            stmt.Bind(3, entity->ID_CHECKINGACCOUNT);
            stmt.Bind(4, entity->ID_CURRENCY);
            if (entity->id() > 0)
                stmt.Bind(5, entity->ID);

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
            wxLogError("TRANSFERTRANS_V1: Exception %s, %s", e.GetMessage().c_str(), entity->to_json());
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
            wxString sql = "DELETE FROM TRANSFERTRANS_V1 WHERE ID = ?";
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
            wxLogError("TRANSFERTRANS_V1: Exception %s", e.GetMessage().c_str());
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
            wxLogError("%s: %d not found", this->name().c_str(), id);
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
                result.push_back(entity);
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
