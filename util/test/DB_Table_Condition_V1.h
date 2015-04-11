// -*- C++ -*-
//=============================================================================
/**
 *      Copyright (c) 2013,2014,2015 Guan Lisheng (guanlisheng@gmail.com)
 *
 *      @file
 *
 *      @author [sqlite2cpp.py]
 *
 *      @brief
 *
 *      Revision History:
 *          AUTO GENERATED at 2015-02-24 23:27:58.822267.
 *          DO NOT EDIT!
 */
//=============================================================================

#ifndef DB_TABLE_CONDITION_V1_H
#define DB_TABLE_CONDITION_V1_H

#include "DB_Table.h"

struct DB_Table_CONDITION_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_CONDITION_V1 Self;
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
    ~DB_Table_CONDITION_V1() 
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
				db->ExecuteUpdate("CREATE TABLE CONDITION_V1 (  CONDITIONID INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL  UNIQUE ,  CONDITIONNAME VARCHAR NOT NULL UNIQUE,  SQLCODE VARCHAR NOT NULL)");
			}
			catch(const wxSQLite3Exception &e) 
			{ 
				wxLogError("CONDITION_V1: Exception %s", e.GetMessage().c_str());
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
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("CONDITION_V1: Exception %s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }

    struct CONDITIONID : public DB_Column<int>
    { 
        static wxString name() { return "CONDITIONID"; } 
        explicit CONDITIONID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct CONDITIONNAME : public DB_Column<wxString>
    { 
        static wxString name() { return "CONDITIONNAME"; } 
        explicit CONDITIONNAME(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct SQLCODE : public DB_Column<wxString>
    { 
        static wxString name() { return "SQLCODE"; } 
        explicit SQLCODE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    typedef CONDITIONID PRIMARY;
    enum COLUMN
    {
        COL_CONDITIONID = 0
        , COL_CONDITIONNAME = 1
        , COL_SQLCODE = 2
    };

    /** Returns the column name as a string*/
    static wxString column_to_name(COLUMN col)
    {
        switch(col)
        {
            case COL_CONDITIONID: return "CONDITIONID";
            case COL_CONDITIONNAME: return "CONDITIONNAME";
            case COL_SQLCODE: return "SQLCODE";
            default: break;
        }
        
        return "UNKNOWN";
    }

    /** Returns the column number from the given column name*/
    static COLUMN name_to_column(const wxString& name)
    {
        if ("CONDITIONID" == name) return COL_CONDITIONID;
        else if ("CONDITIONNAME" == name) return COL_CONDITIONNAME;
        else if ("SQLCODE" == name) return COL_SQLCODE;

        return COLUMN(-1);
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_CONDITION_V1;
        /** This is a instance pointer to itself in memory. */
        Self* view_;
    
        int CONDITIONID;//  primary key
        wxString CONDITIONNAME;
        wxString SQLCODE;
        int id() const { return CONDITIONID; }
        void id(int id) { CONDITIONID = id; }
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
        
            CONDITIONID = -1;
        }

        explicit Data(wxSQLite3ResultSet& q, Self* view = 0)
        {
            view_ = view;
        
            CONDITIONID = q.GetInt(0); // CONDITIONID
            CONDITIONNAME = q.GetString(1); // CONDITIONNAME
            SQLCODE = q.GetString(2); // SQLCODE
        }

        Data& operator=(const Data& other)
        {
            if (this == &other) return *this;

            CONDITIONID = other.CONDITIONID;
            CONDITIONNAME = other.CONDITIONNAME;
            SQLCODE = other.SQLCODE;
            return *this;
        }

        template<typename C>
        bool match(const C &c) const
        {
            return false;
        }
        bool match(const Self::CONDITIONID &in) const
        {
            return this->CONDITIONID == in.v_;
        }
        bool match(const Self::CONDITIONNAME &in) const
        {
            return this->CONDITIONNAME.CmpNoCase(in.v_) == 0;
        }
        bool match(const Self::SQLCODE &in) const
        {
            return this->SQLCODE.CmpNoCase(in.v_) == 0;
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
            o[L"CONDITIONID"] = json::Number(this->CONDITIONID);
            o[L"CONDITIONNAME"] = json::String(this->CONDITIONNAME.ToStdWstring());
            o[L"SQLCODE"] = json::String(this->SQLCODE.ToStdWstring());
            return 0;
        }
        row_t to_row_t() const
        {
            row_t row;
            row(L"CONDITIONID") = CONDITIONID;
            row(L"CONDITIONNAME") = CONDITIONNAME;
            row(L"SQLCODE") = SQLCODE;
            return row;
        }
        void to_template(html_template& t) const
        {
            t(L"CONDITIONID") = CONDITIONID;
            t(L"CONDITIONNAME") = CONDITIONNAME;
            t(L"SQLCODE") = SQLCODE;
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db)
        {
            if (db && db->IsReadOnly()) return false;
            if (!view_ || !db) 
            {
                wxLogError("can not save CONDITION_V1");
                return false;
            }

            return view_->save(this, db);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!view_ || !db) 
            {
                wxLogError("can not remove CONDITION_V1");
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
        NUM_COLUMNS = 3
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    /** Name of the table*/    
    wxString name() const { return "CONDITION_V1"; }

    DB_Table_CONDITION_V1() 
    {
        query_ = "SELECT * FROM CONDITION_V1 ";
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
            sql = "INSERT INTO CONDITION_V1(CONDITIONNAME, SQLCODE) VALUES(?, ?)";
        }
        else
        {
            sql = "UPDATE CONDITION_V1 SET CONDITIONNAME = ?, SQLCODE = ? WHERE CONDITIONID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->CONDITIONNAME);
            stmt.Bind(2, entity->SQLCODE);
            if (entity->id() > 0)
                stmt.Bind(3, entity->CONDITIONID);

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
            wxLogError("CONDITION_V1: Exception %s, %s", e.GetMessage().c_str(), entity->to_json());
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
            wxString sql = "DELETE FROM CONDITION_V1 WHERE CONDITIONID = ?";
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
            wxLogError("CONDITION_V1: Exception %s", e.GetMessage().c_str());
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
