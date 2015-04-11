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

#ifndef DB_TABLE_ASSIGNMENT_V1_H
#define DB_TABLE_ASSIGNMENT_V1_H

#include "DB_Table.h"

struct DB_Table_ASSIGNMENT_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_ASSIGNMENT_V1 Self;
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
    ~DB_Table_ASSIGNMENT_V1() 
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
				db->ExecuteUpdate("CREATE TABLE ASSIGNMENT_V1 (  ASSIGNMENTID INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL  UNIQUE ,  ASSIGNMENTNAME VARCHAR UNIQUE,  FIELDID INTEGER NOT NULL,  CONDITIONID INTEGER NOT NULL,  CRITERION VARCHAR,  PAYEEID INTEGER NOT NULL DEFAULT (-1),  PAYEE_OVERWRITE_FLAG INTEGER NOT NULL DEFAULT (0),  SUBCATEGID INTEGER NOT NULL DEFAULT (-1),   SUBCATEG_OVERWRITE_FLAG INTEGER NOT NULL DEFAULT (0),   FOREIGN KEY(PAYEEID) REFERENCES PAYEE_V1(PAYEEID),  FOREIGN KEY(SUBCATEGID) REFERENCES SUBCATEGORY_V1(SUBCATEGID),  FOREIGN KEY(FIELDID) REFERENCES FIELD_V1(FIELDID),  FOREIGN KEY(CONDITIONID) REFERENCES CONDITION_V1(CONDITIONID))");
			}
			catch(const wxSQLite3Exception &e) 
			{ 
				wxLogError("ASSIGNMENT_V1: Exception %s", e.GetMessage().c_str());
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
            wxLogError("ASSIGNMENT_V1: Exception %s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }

    struct ASSIGNMENTID : public DB_Column<int>
    { 
        static wxString name() { return "ASSIGNMENTID"; } 
        explicit ASSIGNMENTID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct ASSIGNMENTNAME : public DB_Column<wxString>
    { 
        static wxString name() { return "ASSIGNMENTNAME"; } 
        explicit ASSIGNMENTNAME(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct FIELDID : public DB_Column<int>
    { 
        static wxString name() { return "FIELDID"; } 
        explicit FIELDID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct CONDITIONID : public DB_Column<int>
    { 
        static wxString name() { return "CONDITIONID"; } 
        explicit CONDITIONID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct CRITERION : public DB_Column<wxString>
    { 
        static wxString name() { return "CRITERION"; } 
        explicit CRITERION(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct PAYEEID : public DB_Column<int>
    { 
        static wxString name() { return "PAYEEID"; } 
        explicit PAYEEID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct PAYEE_OVERWRITE_FLAG : public DB_Column<int>
    { 
        static wxString name() { return "PAYEE_OVERWRITE_FLAG"; } 
        explicit PAYEE_OVERWRITE_FLAG(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct SUBCATEGID : public DB_Column<int>
    { 
        static wxString name() { return "SUBCATEGID"; } 
        explicit SUBCATEGID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct SUBCATEG_OVERWRITE_FLAG : public DB_Column<int>
    { 
        static wxString name() { return "SUBCATEG_OVERWRITE_FLAG"; } 
        explicit SUBCATEG_OVERWRITE_FLAG(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    typedef ASSIGNMENTID PRIMARY;
    enum COLUMN
    {
        COL_ASSIGNMENTID = 0
        , COL_ASSIGNMENTNAME = 1
        , COL_FIELDID = 2
        , COL_CONDITIONID = 3
        , COL_CRITERION = 4
        , COL_PAYEEID = 5
        , COL_PAYEE_OVERWRITE_FLAG = 6
        , COL_SUBCATEGID = 7
        , COL_SUBCATEG_OVERWRITE_FLAG = 8
    };

    /** Returns the column name as a string*/
    static wxString column_to_name(COLUMN col)
    {
        switch(col)
        {
            case COL_ASSIGNMENTID: return "ASSIGNMENTID";
            case COL_ASSIGNMENTNAME: return "ASSIGNMENTNAME";
            case COL_FIELDID: return "FIELDID";
            case COL_CONDITIONID: return "CONDITIONID";
            case COL_CRITERION: return "CRITERION";
            case COL_PAYEEID: return "PAYEEID";
            case COL_PAYEE_OVERWRITE_FLAG: return "PAYEE_OVERWRITE_FLAG";
            case COL_SUBCATEGID: return "SUBCATEGID";
            case COL_SUBCATEG_OVERWRITE_FLAG: return "SUBCATEG_OVERWRITE_FLAG";
            default: break;
        }
        
        return "UNKNOWN";
    }

    /** Returns the column number from the given column name*/
    static COLUMN name_to_column(const wxString& name)
    {
        if ("ASSIGNMENTID" == name) return COL_ASSIGNMENTID;
        else if ("ASSIGNMENTNAME" == name) return COL_ASSIGNMENTNAME;
        else if ("FIELDID" == name) return COL_FIELDID;
        else if ("CONDITIONID" == name) return COL_CONDITIONID;
        else if ("CRITERION" == name) return COL_CRITERION;
        else if ("PAYEEID" == name) return COL_PAYEEID;
        else if ("PAYEE_OVERWRITE_FLAG" == name) return COL_PAYEE_OVERWRITE_FLAG;
        else if ("SUBCATEGID" == name) return COL_SUBCATEGID;
        else if ("SUBCATEG_OVERWRITE_FLAG" == name) return COL_SUBCATEG_OVERWRITE_FLAG;

        return COLUMN(-1);
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_ASSIGNMENT_V1;
        /** This is a instance pointer to itself in memory. */
        Self* view_;
    
        int ASSIGNMENTID;//  primary key
        wxString ASSIGNMENTNAME;
        int FIELDID;
        int CONDITIONID;
        wxString CRITERION;
        int PAYEEID;
        int PAYEE_OVERWRITE_FLAG;
        int SUBCATEGID;
        int SUBCATEG_OVERWRITE_FLAG;
        int id() const { return ASSIGNMENTID; }
        void id(int id) { ASSIGNMENTID = id; }
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
        
            ASSIGNMENTID = -1;
            FIELDID = -1;
            CONDITIONID = -1;
            PAYEEID = -1;
            PAYEE_OVERWRITE_FLAG = -1;
            SUBCATEGID = -1;
            SUBCATEG_OVERWRITE_FLAG = -1;
        }

        explicit Data(wxSQLite3ResultSet& q, Self* view = 0)
        {
            view_ = view;
        
            ASSIGNMENTID = q.GetInt(0); // ASSIGNMENTID
            ASSIGNMENTNAME = q.GetString(1); // ASSIGNMENTNAME
            FIELDID = q.GetInt(2); // FIELDID
            CONDITIONID = q.GetInt(3); // CONDITIONID
            CRITERION = q.GetString(4); // CRITERION
            PAYEEID = q.GetInt(5); // PAYEEID
            PAYEE_OVERWRITE_FLAG = q.GetInt(6); // PAYEE_OVERWRITE_FLAG
            SUBCATEGID = q.GetInt(7); // SUBCATEGID
            SUBCATEG_OVERWRITE_FLAG = q.GetInt(8); // SUBCATEG_OVERWRITE_FLAG
        }

        Data& operator=(const Data& other)
        {
            if (this == &other) return *this;

            ASSIGNMENTID = other.ASSIGNMENTID;
            ASSIGNMENTNAME = other.ASSIGNMENTNAME;
            FIELDID = other.FIELDID;
            CONDITIONID = other.CONDITIONID;
            CRITERION = other.CRITERION;
            PAYEEID = other.PAYEEID;
            PAYEE_OVERWRITE_FLAG = other.PAYEE_OVERWRITE_FLAG;
            SUBCATEGID = other.SUBCATEGID;
            SUBCATEG_OVERWRITE_FLAG = other.SUBCATEG_OVERWRITE_FLAG;
            return *this;
        }

        template<typename C>
        bool match(const C &c) const
        {
            return false;
        }
        bool match(const Self::ASSIGNMENTID &in) const
        {
            return this->ASSIGNMENTID == in.v_;
        }
        bool match(const Self::ASSIGNMENTNAME &in) const
        {
            return this->ASSIGNMENTNAME.CmpNoCase(in.v_) == 0;
        }
        bool match(const Self::FIELDID &in) const
        {
            return this->FIELDID == in.v_;
        }
        bool match(const Self::CONDITIONID &in) const
        {
            return this->CONDITIONID == in.v_;
        }
        bool match(const Self::CRITERION &in) const
        {
            return this->CRITERION.CmpNoCase(in.v_) == 0;
        }
        bool match(const Self::PAYEEID &in) const
        {
            return this->PAYEEID == in.v_;
        }
        bool match(const Self::PAYEE_OVERWRITE_FLAG &in) const
        {
            return this->PAYEE_OVERWRITE_FLAG == in.v_;
        }
        bool match(const Self::SUBCATEGID &in) const
        {
            return this->SUBCATEGID == in.v_;
        }
        bool match(const Self::SUBCATEG_OVERWRITE_FLAG &in) const
        {
            return this->SUBCATEG_OVERWRITE_FLAG == in.v_;
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
            o[L"ASSIGNMENTID"] = json::Number(this->ASSIGNMENTID);
            o[L"ASSIGNMENTNAME"] = json::String(this->ASSIGNMENTNAME.ToStdWstring());
            o[L"FIELDID"] = json::Number(this->FIELDID);
            o[L"CONDITIONID"] = json::Number(this->CONDITIONID);
            o[L"CRITERION"] = json::String(this->CRITERION.ToStdWstring());
            o[L"PAYEEID"] = json::Number(this->PAYEEID);
            o[L"PAYEE_OVERWRITE_FLAG"] = json::Number(this->PAYEE_OVERWRITE_FLAG);
            o[L"SUBCATEGID"] = json::Number(this->SUBCATEGID);
            o[L"SUBCATEG_OVERWRITE_FLAG"] = json::Number(this->SUBCATEG_OVERWRITE_FLAG);
            return 0;
        }
        row_t to_row_t() const
        {
            row_t row;
            row(L"ASSIGNMENTID") = ASSIGNMENTID;
            row(L"ASSIGNMENTNAME") = ASSIGNMENTNAME;
            row(L"FIELDID") = FIELDID;
            row(L"CONDITIONID") = CONDITIONID;
            row(L"CRITERION") = CRITERION;
            row(L"PAYEEID") = PAYEEID;
            row(L"PAYEE_OVERWRITE_FLAG") = PAYEE_OVERWRITE_FLAG;
            row(L"SUBCATEGID") = SUBCATEGID;
            row(L"SUBCATEG_OVERWRITE_FLAG") = SUBCATEG_OVERWRITE_FLAG;
            return row;
        }
        void to_template(html_template& t) const
        {
            t(L"ASSIGNMENTID") = ASSIGNMENTID;
            t(L"ASSIGNMENTNAME") = ASSIGNMENTNAME;
            t(L"FIELDID") = FIELDID;
            t(L"CONDITIONID") = CONDITIONID;
            t(L"CRITERION") = CRITERION;
            t(L"PAYEEID") = PAYEEID;
            t(L"PAYEE_OVERWRITE_FLAG") = PAYEE_OVERWRITE_FLAG;
            t(L"SUBCATEGID") = SUBCATEGID;
            t(L"SUBCATEG_OVERWRITE_FLAG") = SUBCATEG_OVERWRITE_FLAG;
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db)
        {
            if (db && db->IsReadOnly()) return false;
            if (!view_ || !db) 
            {
                wxLogError("can not save ASSIGNMENT_V1");
                return false;
            }

            return view_->save(this, db);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!view_ || !db) 
            {
                wxLogError("can not remove ASSIGNMENT_V1");
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
        NUM_COLUMNS = 9
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    /** Name of the table*/    
    wxString name() const { return "ASSIGNMENT_V1"; }

    DB_Table_ASSIGNMENT_V1() 
    {
        query_ = "SELECT * FROM ASSIGNMENT_V1 ";
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
            sql = "INSERT INTO ASSIGNMENT_V1(ASSIGNMENTNAME, FIELDID, CONDITIONID, CRITERION, PAYEEID, PAYEE_OVERWRITE_FLAG, SUBCATEGID, SUBCATEG_OVERWRITE_FLAG) VALUES(?, ?, ?, ?, ?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE ASSIGNMENT_V1 SET ASSIGNMENTNAME = ?, FIELDID = ?, CONDITIONID = ?, CRITERION = ?, PAYEEID = ?, PAYEE_OVERWRITE_FLAG = ?, SUBCATEGID = ?, SUBCATEG_OVERWRITE_FLAG = ? WHERE ASSIGNMENTID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->ASSIGNMENTNAME);
            stmt.Bind(2, entity->FIELDID);
            stmt.Bind(3, entity->CONDITIONID);
            stmt.Bind(4, entity->CRITERION);
            stmt.Bind(5, entity->PAYEEID);
            stmt.Bind(6, entity->PAYEE_OVERWRITE_FLAG);
            stmt.Bind(7, entity->SUBCATEGID);
            stmt.Bind(8, entity->SUBCATEG_OVERWRITE_FLAG);
            if (entity->id() > 0)
                stmt.Bind(9, entity->ASSIGNMENTID);

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
            wxLogError("ASSIGNMENT_V1: Exception %s, %s", e.GetMessage().c_str(), entity->to_json());
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
            wxString sql = "DELETE FROM ASSIGNMENT_V1 WHERE ASSIGNMENTID = ?";
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
            wxLogError("ASSIGNMENT_V1: Exception %s", e.GetMessage().c_str());
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
