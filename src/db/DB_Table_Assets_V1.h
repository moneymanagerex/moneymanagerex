// -*- C++ -*-
//=============================================================================
/**
 *      Copyright (c) 2013 Guan Lisheng (guanlisheng@gmail.com)
 *
 *      @file
 *
 *      @author [sqlite2cpp.py]
 *
 *      @brief
 *
 *      Revision History:
 *          AUTO GENERATED at 2013-12-09 20:39:37.193577.
 *          DO NOT EDIT!
 */
//=============================================================================

#ifndef DB_TABLE_ASSETS_V1_H
#define DB_TABLE_ASSETS_V1_H

#include "DB_Table.h"

struct DB_Table_ASSETS_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_ASSETS_V1 Self;
    /** A container to hold list of Data records for the table*/
    typedef std::vector<Self::Data> Data_Set;
    /** A container to hold a list of Data record pointers for the table in memory*/
    typedef std::vector<Self::Data*> Cache;
    Cache cache_;

    /** Destructor: clears any data records stored in memory */
    ~DB_Table_ASSETS_V1() 
    {
        destroy_cache();
    }
    
    /** Show table statistics*/
	void show_statistics()
	{
		size_t cache_size = this->cache_.size();
#ifdef _WIN64
        wxLogDebug("%s : (cache %llu, hit %llu, miss %llu, skip %llu)", this->name(), cache_size, this->hit_, this->miss_, this->skip_);
#else
        wxLogDebug("%s : (cache %lu, hit %lu, miss %lu, skip %lu)", this->name(), cache_size, this->hit_, this->miss_, this->skip_);
#endif
	}
	 
    /** Removes all records stored in memory (cache) for the table*/ 
    void destroy_cache()
    {
        std::for_each(cache_.begin(), cache_.end(), std::mem_fun(&Data::destroy));
        cache_.clear();
    }

    /** Creates the database table if the table does not exist*/
    bool ensure(wxSQLite3Database* db)
    {
        if (!exists(db))
		{
			try
			{
				db->ExecuteUpdate("CREATE TABLE ASSETS_V1(ASSETID integer primary key, STARTDATE TEXT NOT NULL , ASSETNAME TEXT COLLATE NOCASE NOT NULL, VALUE numeric, VALUECHANGE TEXT, NOTES TEXT, VALUECHANGERATE numeric, ASSETTYPE TEXT)");
			}
			catch(const wxSQLite3Exception &e) 
			{ 
				wxLogError("ASSETS_V1: Exception %s", e.GetMessage().c_str());
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
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_ASSETS_ASSETTYPE ON ASSETS_V1(ASSETTYPE)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("ASSETS_V1: Exception %s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }

    struct ASSETID : public DB_Column<int>
    { 
        static wxString name() { return "ASSETID"; } 
        ASSETID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct STARTDATE : public DB_Column<wxString>
    { 
        static wxString name() { return "STARTDATE"; } 
        STARTDATE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct ASSETNAME : public DB_Column<wxString>
    { 
        static wxString name() { return "ASSETNAME"; } 
        ASSETNAME(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct VALUE : public DB_Column<double>
    { 
        static wxString name() { return "VALUE"; } 
        VALUE(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    struct VALUECHANGE : public DB_Column<wxString>
    { 
        static wxString name() { return "VALUECHANGE"; } 
        VALUECHANGE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct NOTES : public DB_Column<wxString>
    { 
        static wxString name() { return "NOTES"; } 
        NOTES(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct VALUECHANGERATE : public DB_Column<double>
    { 
        static wxString name() { return "VALUECHANGERATE"; } 
        VALUECHANGERATE(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    struct ASSETTYPE : public DB_Column<wxString>
    { 
        static wxString name() { return "ASSETTYPE"; } 
        ASSETTYPE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
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
    };

    /** Returns the column name as a string*/
    wxString column_to_name(COLUMN col) const
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

    /** Returns the comumn number from the given column name*/
    COLUMN name_to_column(const wxString& name) const
    {
        if ("ASSETID" == name) return COL_ASSETID;
        else if ("STARTDATE" == name) return COL_STARTDATE;
        else if ("ASSETNAME" == name) return COL_ASSETNAME;
        else if ("VALUE" == name) return COL_VALUE;
        else if ("VALUECHANGE" == name) return COL_VALUECHANGE;
        else if ("NOTES" == name) return COL_NOTES;
        else if ("VALUECHANGERATE" == name) return COL_VALUECHANGERATE;
        else if ("ASSETTYPE" == name) return COL_ASSETTYPE;

        return COLUMN(-1);
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_ASSETS_V1;
        /** This is a instance pointer to itself in memory. */
        Self* view_;
    
        int ASSETID;//  primay key
        wxString STARTDATE;
        wxString ASSETNAME;
        double VALUE;
        wxString VALUECHANGE;
        wxString NOTES;
        double VALUECHANGERATE;
        wxString ASSETTYPE;
        int id() const { return ASSETID; }
        void id(int id) { ASSETID = id; }
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
        
            ASSETID = -1;
            VALUE = 0.0;
            VALUECHANGERATE = 0.0;
        }

        Data(wxSQLite3ResultSet& q, Self* view = 0)
        {
            view_ = view;
        
            ASSETID = q.GetInt("ASSETID");
            STARTDATE = q.GetString("STARTDATE");
            ASSETNAME = q.GetString("ASSETNAME");
            VALUE = q.GetDouble("VALUE");
            VALUECHANGE = q.GetString("VALUECHANGE");
            NOTES = q.GetString("NOTES");
            VALUECHANGERATE = q.GetDouble("VALUECHANGERATE");
            ASSETTYPE = q.GetString("ASSETTYPE");
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
            o["ASSETID"] = json::Number(this->ASSETID);
            o["STARTDATE"] = json::String(this->STARTDATE.ToStdString());
            o["ASSETNAME"] = json::String(this->ASSETNAME.ToStdString());
            o["VALUE"] = json::Number(this->VALUE);
            o["VALUECHANGE"] = json::String(this->VALUECHANGE.ToStdString());
            o["NOTES"] = json::String(this->NOTES.ToStdString());
            o["VALUECHANGERATE"] = json::Number(this->VALUECHANGERATE);
            o["ASSETTYPE"] = json::String(this->ASSETTYPE.ToStdString());
            return 0;
        }
        row_t to_row_t() const
        {
            row_t row;
            row("ASSETID") = ASSETID;
            row("STARTDATE") = STARTDATE;
            row("ASSETNAME") = ASSETNAME;
            row("VALUE") = VALUE;
            row("VALUECHANGE") = VALUECHANGE;
            row("NOTES") = NOTES;
            row("VALUECHANGERATE") = VALUECHANGERATE;
            row("ASSETTYPE") = ASSETTYPE;
            return row;
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db)
        {
            if (!view_ || !db) 
            {
                wxLogError("can not save");
                return false;
            }

            return view_->save(this, db);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!view_ || !db) 
            {
                wxLogError("can not remove");
                return false;
            }
            
            return view_->remove(this, db);
        }

        void destroy() { delete this; }
    };

    enum
    {
        NUM_COLUMNS = 8
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    /** Name of the table*/    
    wxString name() const { return "ASSETS_V1"; }

    DB_Table_ASSETS_V1() 
    {
        query_ = "SELECT * FROM ASSETS_V1 ";
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
    */
    bool save(Self::Data* entity, wxSQLite3Database* db)
    {
        wxString sql = wxEmptyString;
        if (entity->id() < 0) //  new & insert
        {
            sql = "INSERT INTO ASSETS_V1(STARTDATE, ASSETNAME, VALUE, VALUECHANGE, NOTES, VALUECHANGERATE, ASSETTYPE) VALUES(?, ?, ?, ?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE ASSETS_V1 SET STARTDATE = ?, ASSETNAME = ?, VALUE = ?, VALUECHANGE = ?, NOTES = ?, VALUECHANGERATE = ?, ASSETTYPE = ? WHERE ASSETID = ?";
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

            wxLogDebug(stmt.GetSQL());
            stmt.ExecuteUpdate();
            stmt.Finalize();
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("ASSETS_V1: Exception %s, %s", e.GetMessage().c_str(), entity->to_json());
            return false;
        }

        if (entity->id() < 0) entity->id((db->GetLastRowId()).ToLong());
        return true;
    }

    /** Remove the Data record from the database and the memory table (cache) */
    bool remove(int id, wxSQLite3Database* db)
    {
        if (id < 0) return false;
        try
        {
            wxString sql = "DELETE FROM ASSETS_V1 WHERE ASSETID = ?";
            wxSQLite3Statement stmt = db->PrepareStatement(sql);
            stmt.Bind(1, id);
            wxLogDebug(stmt.GetSQL());
            stmt.ExecuteUpdate();
            stmt.Finalize();

            Cache c;
            for(Cache::iterator it = cache_.begin(); it != cache_.end(); ++ it)
            {
                Self::Data* entity = *it;
                if (entity->id() == id) 
                    delete entity;
                else 
                    c.push_back(entity);
            }
            cache_.clear();
            cache_.swap(c);
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("ASSETS_V1: Exception %s", e.GetMessage().c_str());
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

    
    /**
    * Search the memory table (Cache) for the data record.
    * If not found in memory, search the database and update the cache.
    */
    Self::Data* get(int id, wxSQLite3Database* db)
    {
        if (id < 0) 
        {
            ++ skip_;
            return 0;
        }
        for(Cache::reverse_iterator it = cache_.rbegin(); it != cache_.rend(); ++ it)
        {
            Self::Data* entity = *it;
            if (entity->id() == id) 
            {
                ++ hit_;
                return entity;
            }
        }
        
        ++ miss_;
        Self::Data* entity = 0;
        wxString where = wxString::Format(" WHERE %s = ?", PRIMARY::name().c_str());
        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(this->query() + where);
            stmt.Bind(1, id);

            wxLogDebug(stmt.GetSQL());
            wxSQLite3ResultSet q = stmt.ExecuteQuery();
            if(q.NextRow())
            {
                entity = new Self::Data(q, this);
                cache_.push_back(entity);
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

            wxLogDebug(q.GetSQL());
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
