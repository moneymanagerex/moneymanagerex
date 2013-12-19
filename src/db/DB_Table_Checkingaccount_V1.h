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
 *          AUTO GENERATED at 2013-12-19 14:43:31.114225.
 *          DO NOT EDIT!
 */
//=============================================================================

#ifndef DB_TABLE_CHECKINGACCOUNT_V1_H
#define DB_TABLE_CHECKINGACCOUNT_V1_H

#include "DB_Table.h"

struct DB_Table_CHECKINGACCOUNT_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_CHECKINGACCOUNT_V1 Self;
    /** A container to hold list of Data records for the table*/
    typedef std::vector<Self::Data> Data_Set;
    /** A container to hold a list of Data record pointers for the table in memory*/
    typedef std::vector<Self::Data*> Cache;
    Cache cache_;

    /** Destructor: clears any data records stored in memory */
    ~DB_Table_CHECKINGACCOUNT_V1() 
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
				db->ExecuteUpdate("CREATE TABLE CHECKINGACCOUNT_V1(TRANSID integer primary key, ACCOUNTID integer NOT NULL, TOACCOUNTID integer, PAYEEID integer NOT NULL, TRANSCODE TEXT NOT NULL, TRANSAMOUNT numeric NOT NULL, STATUS TEXT, TRANSACTIONNUMBER TEXT, NOTES TEXT, CATEGID integer, SUBCATEGID integer, TRANSDATE TEXT, FOLLOWUPID integer, TOTRANSAMOUNT numeric)");
			}
			catch(const wxSQLite3Exception &e) 
			{ 
				wxLogError("CHECKINGACCOUNT_V1: Exception %s", e.GetMessage().c_str());
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
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_CHECKINGACCOUNT_ACCOUNT ON CHECKINGACCOUNT_V1 (ACCOUNTID, TOACCOUNTID)");
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_CHECKINGACCOUNT_TRANSDATE ON CHECKINGACCOUNT_V1 (TRANSDATE)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("CHECKINGACCOUNT_V1: Exception %s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }

    struct TRANSID : public DB_Column<int>
    { 
        static wxString name() { return "TRANSID"; } 
        TRANSID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct ACCOUNTID : public DB_Column<int>
    { 
        static wxString name() { return "ACCOUNTID"; } 
        ACCOUNTID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct TOACCOUNTID : public DB_Column<int>
    { 
        static wxString name() { return "TOACCOUNTID"; } 
        TOACCOUNTID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct PAYEEID : public DB_Column<int>
    { 
        static wxString name() { return "PAYEEID"; } 
        PAYEEID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct TRANSCODE : public DB_Column<wxString>
    { 
        static wxString name() { return "TRANSCODE"; } 
        TRANSCODE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct TRANSAMOUNT : public DB_Column<double>
    { 
        static wxString name() { return "TRANSAMOUNT"; } 
        TRANSAMOUNT(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    struct STATUS : public DB_Column<wxString>
    { 
        static wxString name() { return "STATUS"; } 
        STATUS(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct TRANSACTIONNUMBER : public DB_Column<wxString>
    { 
        static wxString name() { return "TRANSACTIONNUMBER"; } 
        TRANSACTIONNUMBER(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct NOTES : public DB_Column<wxString>
    { 
        static wxString name() { return "NOTES"; } 
        NOTES(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
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
    struct TRANSDATE : public DB_Column<wxString>
    { 
        static wxString name() { return "TRANSDATE"; } 
        TRANSDATE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct FOLLOWUPID : public DB_Column<int>
    { 
        static wxString name() { return "FOLLOWUPID"; } 
        FOLLOWUPID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct TOTRANSAMOUNT : public DB_Column<double>
    { 
        static wxString name() { return "TOTRANSAMOUNT"; } 
        TOTRANSAMOUNT(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    typedef TRANSID PRIMARY;
    enum COLUMN
    {
        COL_TRANSID = 0
        , COL_ACCOUNTID = 1
        , COL_TOACCOUNTID = 2
        , COL_PAYEEID = 3
        , COL_TRANSCODE = 4
        , COL_TRANSAMOUNT = 5
        , COL_STATUS = 6
        , COL_TRANSACTIONNUMBER = 7
        , COL_NOTES = 8
        , COL_CATEGID = 9
        , COL_SUBCATEGID = 10
        , COL_TRANSDATE = 11
        , COL_FOLLOWUPID = 12
        , COL_TOTRANSAMOUNT = 13
    };

    /** Returns the column name as a string*/
    wxString column_to_name(COLUMN col) const
    {
        switch(col)
        {
            case COL_TRANSID: return "TRANSID";
            case COL_ACCOUNTID: return "ACCOUNTID";
            case COL_TOACCOUNTID: return "TOACCOUNTID";
            case COL_PAYEEID: return "PAYEEID";
            case COL_TRANSCODE: return "TRANSCODE";
            case COL_TRANSAMOUNT: return "TRANSAMOUNT";
            case COL_STATUS: return "STATUS";
            case COL_TRANSACTIONNUMBER: return "TRANSACTIONNUMBER";
            case COL_NOTES: return "NOTES";
            case COL_CATEGID: return "CATEGID";
            case COL_SUBCATEGID: return "SUBCATEGID";
            case COL_TRANSDATE: return "TRANSDATE";
            case COL_FOLLOWUPID: return "FOLLOWUPID";
            case COL_TOTRANSAMOUNT: return "TOTRANSAMOUNT";
            default: break;
        }
        
        return "UNKNOWN";
    }

    /** Returns the comumn number from the given column name*/
    COLUMN name_to_column(const wxString& name) const
    {
        if ("TRANSID" == name) return COL_TRANSID;
        else if ("ACCOUNTID" == name) return COL_ACCOUNTID;
        else if ("TOACCOUNTID" == name) return COL_TOACCOUNTID;
        else if ("PAYEEID" == name) return COL_PAYEEID;
        else if ("TRANSCODE" == name) return COL_TRANSCODE;
        else if ("TRANSAMOUNT" == name) return COL_TRANSAMOUNT;
        else if ("STATUS" == name) return COL_STATUS;
        else if ("TRANSACTIONNUMBER" == name) return COL_TRANSACTIONNUMBER;
        else if ("NOTES" == name) return COL_NOTES;
        else if ("CATEGID" == name) return COL_CATEGID;
        else if ("SUBCATEGID" == name) return COL_SUBCATEGID;
        else if ("TRANSDATE" == name) return COL_TRANSDATE;
        else if ("FOLLOWUPID" == name) return COL_FOLLOWUPID;
        else if ("TOTRANSAMOUNT" == name) return COL_TOTRANSAMOUNT;

        return COLUMN(-1);
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_CHECKINGACCOUNT_V1;
        /** This is a instance pointer to itself in memory. */
        Self* view_;
    
        int TRANSID;//  primay key
        int ACCOUNTID;
        int TOACCOUNTID;
        int PAYEEID;
        wxString TRANSCODE;
        double TRANSAMOUNT;
        wxString STATUS;
        wxString TRANSACTIONNUMBER;
        wxString NOTES;
        int CATEGID;
        int SUBCATEGID;
        wxString TRANSDATE;
        int FOLLOWUPID;
        double TOTRANSAMOUNT;
        int id() const { return TRANSID; }
        void id(int id) { TRANSID = id; }
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
        
            TRANSID = -1;
            ACCOUNTID = -1;
            TOACCOUNTID = -1;
            PAYEEID = -1;
            TRANSAMOUNT = 0.0;
            CATEGID = -1;
            SUBCATEGID = -1;
            FOLLOWUPID = -1;
            TOTRANSAMOUNT = 0.0;
        }

        Data(wxSQLite3ResultSet& q, Self* view = 0)
        {
            view_ = view;
        
            TRANSID = q.GetInt("TRANSID");
            ACCOUNTID = q.GetInt("ACCOUNTID");
            TOACCOUNTID = q.GetInt("TOACCOUNTID");
            PAYEEID = q.GetInt("PAYEEID");
            TRANSCODE = q.GetString("TRANSCODE");
            TRANSAMOUNT = q.GetDouble("TRANSAMOUNT");
            STATUS = q.GetString("STATUS");
            TRANSACTIONNUMBER = q.GetString("TRANSACTIONNUMBER");
            NOTES = q.GetString("NOTES");
            CATEGID = q.GetInt("CATEGID");
            SUBCATEGID = q.GetInt("SUBCATEGID");
            TRANSDATE = q.GetString("TRANSDATE");
            FOLLOWUPID = q.GetInt("FOLLOWUPID");
            TOTRANSAMOUNT = q.GetDouble("TOTRANSAMOUNT");
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
            o["TRANSID"] = json::Number(this->TRANSID);
            o["ACCOUNTID"] = json::Number(this->ACCOUNTID);
            o["TOACCOUNTID"] = json::Number(this->TOACCOUNTID);
            o["PAYEEID"] = json::Number(this->PAYEEID);
            o["TRANSCODE"] = json::String(this->TRANSCODE.ToStdString());
            o["TRANSAMOUNT"] = json::Number(this->TRANSAMOUNT);
            o["STATUS"] = json::String(this->STATUS.ToStdString());
            o["TRANSACTIONNUMBER"] = json::String(this->TRANSACTIONNUMBER.ToStdString());
            o["NOTES"] = json::String(this->NOTES.ToStdString());
            o["CATEGID"] = json::Number(this->CATEGID);
            o["SUBCATEGID"] = json::Number(this->SUBCATEGID);
            o["TRANSDATE"] = json::String(this->TRANSDATE.ToStdString());
            o["FOLLOWUPID"] = json::Number(this->FOLLOWUPID);
            o["TOTRANSAMOUNT"] = json::Number(this->TOTRANSAMOUNT);
            return 0;
        }
        row_t to_row_t() const
        {
            row_t row;
            row("TRANSID") = TRANSID;
            row("ACCOUNTID") = ACCOUNTID;
            row("TOACCOUNTID") = TOACCOUNTID;
            row("PAYEEID") = PAYEEID;
            row("TRANSCODE") = TRANSCODE;
            row("TRANSAMOUNT") = TRANSAMOUNT;
            row("STATUS") = STATUS;
            row("TRANSACTIONNUMBER") = TRANSACTIONNUMBER;
            row("NOTES") = NOTES;
            row("CATEGID") = CATEGID;
            row("SUBCATEGID") = SUBCATEGID;
            row("TRANSDATE") = TRANSDATE;
            row("FOLLOWUPID") = FOLLOWUPID;
            row("TOTRANSAMOUNT") = TOTRANSAMOUNT;
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
        NUM_COLUMNS = 14
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    /** Name of the table*/    
    wxString name() const { return "CHECKINGACCOUNT_V1"; }

    DB_Table_CHECKINGACCOUNT_V1() 
    {
        query_ = "SELECT * FROM CHECKINGACCOUNT_V1 ";
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
        if (entity->id() < 0) //  new & insert
        {
            sql = "INSERT INTO CHECKINGACCOUNT_V1(ACCOUNTID, TOACCOUNTID, PAYEEID, TRANSCODE, TRANSAMOUNT, STATUS, TRANSACTIONNUMBER, NOTES, CATEGID, SUBCATEGID, TRANSDATE, FOLLOWUPID, TOTRANSAMOUNT) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE CHECKINGACCOUNT_V1 SET ACCOUNTID = ?, TOACCOUNTID = ?, PAYEEID = ?, TRANSCODE = ?, TRANSAMOUNT = ?, STATUS = ?, TRANSACTIONNUMBER = ?, NOTES = ?, CATEGID = ?, SUBCATEGID = ?, TRANSDATE = ?, FOLLOWUPID = ?, TOTRANSAMOUNT = ? WHERE TRANSID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->ACCOUNTID);
            stmt.Bind(2, entity->TOACCOUNTID);
            stmt.Bind(3, entity->PAYEEID);
            stmt.Bind(4, entity->TRANSCODE);
            stmt.Bind(5, entity->TRANSAMOUNT);
            stmt.Bind(6, entity->STATUS);
            stmt.Bind(7, entity->TRANSACTIONNUMBER);
            stmt.Bind(8, entity->NOTES);
            stmt.Bind(9, entity->CATEGID);
            stmt.Bind(10, entity->SUBCATEGID);
            stmt.Bind(11, entity->TRANSDATE);
            stmt.Bind(12, entity->FOLLOWUPID);
            stmt.Bind(13, entity->TOTRANSAMOUNT);
            if (entity->id() > 0)
                stmt.Bind(14, entity->TRANSID);

            wxLogDebug(stmt.GetSQL());
            stmt.ExecuteUpdate();
            stmt.Finalize();

            if (entity->id() > 0)
            {
                Cache c;
                for(Cache::iterator it = cache_.begin(); it != cache_.end(); ++ it)
                {
                    Self::Data* e = *it;
                    if (e->id() == entity->id() && e != entity) 
                        delete e;
                    else 
                        c.push_back(e);
                }
                cache_.clear();
                cache_.swap(c);
            }
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("CHECKINGACCOUNT_V1: Exception %s, %s", e.GetMessage().c_str(), entity->to_json());
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
            wxString sql = "DELETE FROM CHECKINGACCOUNT_V1 WHERE TRANSID = ?";
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
            wxLogError("CHECKINGACCOUNT_V1: Exception %s", e.GetMessage().c_str());
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
