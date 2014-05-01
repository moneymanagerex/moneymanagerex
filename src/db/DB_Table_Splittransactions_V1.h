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
 *          AUTO GENERATED at 2014-05-02 07:03:07.427989.
 *          DO NOT EDIT!
 */
//=============================================================================

#ifndef DB_TABLE_SPLITTRANSACTIONS_V1_H
#define DB_TABLE_SPLITTRANSACTIONS_V1_H

#include "DB_Table.h"

struct DB_Table_SPLITTRANSACTIONS_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_SPLITTRANSACTIONS_V1 Self;
    /** A container to hold list of Data records for the table*/
    struct Data_Set : public std::vector<Self::Data>
    {
        std::string to_json() const
        {
            json::Array a;
            for (const auto & item: *this)
            {
                json::Object o;
                item.to_json(o);
                a.Insert(o);
            }
            std::stringstream ss;
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
    ~DB_Table_SPLITTRANSACTIONS_V1() 
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
				db->ExecuteUpdate("CREATE TABLE SPLITTRANSACTIONS_V1(SPLITTRANSID integer primary key, TRANSID integer NOT NULL, CATEGID integer, SUBCATEGID integer, SPLITTRANSAMOUNT numeric)");
			}
			catch(const wxSQLite3Exception &e) 
			{ 
				wxLogError("SPLITTRANSACTIONS_V1: Exception %s", e.GetMessage().c_str());
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
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_SPLITTRANSACTIONS_TRANSID ON SPLITTRANSACTIONS_V1(TRANSID)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("SPLITTRANSACTIONS_V1: Exception %s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }

    struct SPLITTRANSID : public DB_Column<int>
    { 
        static wxString name() { return "SPLITTRANSID"; } 
        SPLITTRANSID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct TRANSID : public DB_Column<int>
    { 
        static wxString name() { return "TRANSID"; } 
        TRANSID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct CATEGID : public DB_Column<int>
    { 
        static wxString name() { return "CATEGID"; } 
        CATEGID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct SUBCATEGID : public DB_Column<int>
    { 
        static wxString name() { return "SUBCATEGID"; } 
        SUBCATEGID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct SPLITTRANSAMOUNT : public DB_Column<double>
    { 
        static wxString name() { return "SPLITTRANSAMOUNT"; } 
        SPLITTRANSAMOUNT(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    typedef SPLITTRANSID PRIMARY;
    enum COLUMN
    {
        COL_SPLITTRANSID = 0
        , COL_TRANSID = 1
        , COL_CATEGID = 2
        , COL_SUBCATEGID = 3
        , COL_SPLITTRANSAMOUNT = 4
    };

    /** Returns the column name as a string*/
    static wxString column_to_name(COLUMN col)
    {
        switch(col)
        {
            case COL_SPLITTRANSID: return "SPLITTRANSID";
            case COL_TRANSID: return "TRANSID";
            case COL_CATEGID: return "CATEGID";
            case COL_SUBCATEGID: return "SUBCATEGID";
            case COL_SPLITTRANSAMOUNT: return "SPLITTRANSAMOUNT";
            default: break;
        }
        
        return "UNKNOWN";
    }

    /** Returns the comumn number from the given column name*/
    static COLUMN name_to_column(const wxString& name)
    {
        if ("SPLITTRANSID" == name) return COL_SPLITTRANSID;
        else if ("TRANSID" == name) return COL_TRANSID;
        else if ("CATEGID" == name) return COL_CATEGID;
        else if ("SUBCATEGID" == name) return COL_SUBCATEGID;
        else if ("SPLITTRANSAMOUNT" == name) return COL_SPLITTRANSAMOUNT;

        return COLUMN(-1);
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_SPLITTRANSACTIONS_V1;
        /** This is a instance pointer to itself in memory. */
        Self* view_;
    
        int SPLITTRANSID;//  primay key
        int TRANSID;
        int CATEGID;
        int SUBCATEGID;
        double SPLITTRANSAMOUNT;
        int id() const { return SPLITTRANSID; }
        void id(int id) { SPLITTRANSID = id; }
        bool operator < (const Data& r) const
        {
            return this->id() < r.id();
        }
        bool operator < (const Data* r) const
        {
            return this->id() < r->id();
        }

        Data(Self* view = 0) 
        {
            view_ = view;
        
            SPLITTRANSID = -1;
            TRANSID = -1;
            CATEGID = -1;
            SUBCATEGID = -1;
            SPLITTRANSAMOUNT = 0.0;
        }

        Data(wxSQLite3ResultSet& q, Self* view = 0)
        {
            view_ = view;
        
            SPLITTRANSID = q.GetInt(0); // SPLITTRANSID
            TRANSID = q.GetInt(1); // TRANSID
            CATEGID = q.GetInt(2); // CATEGID
            SUBCATEGID = q.GetInt(3); // SUBCATEGID
            SPLITTRANSAMOUNT = q.GetDouble(4); // SPLITTRANSAMOUNT
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
        wxString to_json() const
        {
            json::Object o;
            this->to_json(o);
            std::stringstream ss;
            json::Writer::Write(o, ss);
            return ss.str();
        }
        
        int to_json(json::Object& o) const
        {
            o["SPLITTRANSID"] = json::Number(this->SPLITTRANSID);
            o["TRANSID"] = json::Number(this->TRANSID);
            o["CATEGID"] = json::Number(this->CATEGID);
            o["SUBCATEGID"] = json::Number(this->SUBCATEGID);
            o["SPLITTRANSAMOUNT"] = json::Number(this->SPLITTRANSAMOUNT);
            return 0;
        }
        row_t to_row_t() const
        {
            row_t row;
            row(L"SPLITTRANSID") = SPLITTRANSID;
            row(L"TRANSID") = TRANSID;
            row(L"CATEGID") = CATEGID;
            row(L"SUBCATEGID") = SUBCATEGID;
            row(L"SPLITTRANSAMOUNT") = SPLITTRANSAMOUNT;
            return row;
        }
        void to_template(html_template& t) const
        {
            t(L"SPLITTRANSID") = SPLITTRANSID;
            t(L"TRANSID") = TRANSID;
            t(L"CATEGID") = CATEGID;
            t(L"SUBCATEGID") = SUBCATEGID;
            t(L"SPLITTRANSAMOUNT") = SPLITTRANSAMOUNT;
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db)
        {
            if (!view_ || !db) 
            {
                wxLogError("can not save SPLITTRANSACTIONS_V1");
                return false;
            }

            return view_->save(this, db);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!view_ || !db) 
            {
                wxLogError("can not remove SPLITTRANSACTIONS_V1");
                return false;
            }
            
            return view_->remove(this, db);
        }

        void destroy() { delete this; }
    };

    enum
    {
        NUM_COLUMNS = 5
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    /** Name of the table*/    
    wxString name() const { return "SPLITTRANSACTIONS_V1"; }

    DB_Table_SPLITTRANSACTIONS_V1() 
    {
        query_ = "SELECT * FROM SPLITTRANSACTIONS_V1 ";
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
            sql = "INSERT INTO SPLITTRANSACTIONS_V1(TRANSID, CATEGID, SUBCATEGID, SPLITTRANSAMOUNT) VALUES(?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE SPLITTRANSACTIONS_V1 SET TRANSID = ?, CATEGID = ?, SUBCATEGID = ?, SPLITTRANSAMOUNT = ? WHERE SPLITTRANSID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->TRANSID);
            stmt.Bind(2, entity->CATEGID);
            stmt.Bind(3, entity->SUBCATEGID);
            stmt.Bind(4, entity->SPLITTRANSAMOUNT);
            if (entity->id() > 0)
                stmt.Bind(5, entity->SPLITTRANSID);

            //wxLogDebug(stmt.GetSQL());
            stmt.ExecuteUpdate();
            stmt.Finalize();

            if (entity->id() > 0)
            {
                Cache c;
                for(Cache::iterator it = cache_.begin(); it != cache_.end(); ++ it)
                {
                    Self::Data* e = *it;
                    if (e->id() == entity->id() && e != entity) 
                    {
                        index_by_id_.erase(entity->id());
                        delete e;
                    }
                    else 
                    {
                        c.push_back(e);
                    }
                }
                cache_.clear();
                cache_.swap(c);
            }
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("SPLITTRANSACTIONS_V1: Exception %s, %s", e.GetMessage().c_str(), entity->to_json());
            return false;
        }

        if (entity->id() <= 0) entity->id((db->GetLastRowId()).ToLong());
        return true;
    }

    /** Remove the Data record from the database and the memory table (cache) */
    bool remove(int id, wxSQLite3Database* db)
    {
        if (id <= 0) return false;
        try
        {
            wxString sql = "DELETE FROM SPLITTRANSACTIONS_V1 WHERE SPLITTRANSID = ?";
            wxSQLite3Statement stmt = db->PrepareStatement(sql);
            stmt.Bind(1, id);
            //wxLogDebug(stmt.GetSQL());
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
            wxLogError("SPLITTRANSACTIONS_V1: Exception %s", e.GetMessage().c_str());
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

            //wxLogDebug(stmt.GetSQL());
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
    Data_Set all(wxSQLite3Database* db, COLUMN col = COLUMN(0), bool asc = true)
    {
        Data_Set result;
        try
        {
            wxSQLite3ResultSet q = db->ExecuteQuery(col == COLUMN(0) ? this->query() : this->query() + " ORDER BY " + column_to_name(col) + " COLLATE NOCASE " + (asc ? " ASC " : " DESC "));

            //wxLogDebug(q.GetSQL());
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
