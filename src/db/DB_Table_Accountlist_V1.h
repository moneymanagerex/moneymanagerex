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
 *          AUTO GENERATED at 2014-02-23 21:14:57.787845.
 *          DO NOT EDIT!
 */
//=============================================================================

#ifndef DB_TABLE_ACCOUNTLIST_V1_H
#define DB_TABLE_ACCOUNTLIST_V1_H

#include "DB_Table.h"

struct DB_Table_ACCOUNTLIST_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_ACCOUNTLIST_V1 Self;
    /** A container to hold list of Data records for the table*/
    typedef std::vector<Self::Data> Data_Set;
    /** A container to hold a list of Data record pointers for the table in memory*/
    typedef std::vector<Self::Data*> Cache;
    Cache cache_;

    /** Destructor: clears any data records stored in memory */
    ~DB_Table_ACCOUNTLIST_V1() 
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
				db->ExecuteUpdate("CREATE TABLE ACCOUNTLIST_V1(ACCOUNTID integer primary key, ACCOUNTNAME TEXT COLLATE NOCASE NOT NULL UNIQUE, ACCOUNTTYPE TEXT NOT NULL , ACCOUNTNUM TEXT, STATUS TEXT NOT NULL, NOTES TEXT , HELDAT TEXT , WEBSITE TEXT , CONTACTINFO TEXT, ACCESSINFO TEXT , INITIALBAL numeric , FAVORITEACCT TEXT NOT NULL, CURRENCYID integer NOT NULL)");
			}
			catch(const wxSQLite3Exception &e) 
			{ 
				wxLogError("ACCOUNTLIST_V1: Exception %s", e.GetMessage().c_str());
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
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_ACCOUNTLIST_ACCOUNTTYPE ON ACCOUNTLIST_V1(ACCOUNTTYPE)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("ACCOUNTLIST_V1: Exception %s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }

    struct ACCOUNTID : public DB_Column<int>
    { 
        static wxString name() { return "ACCOUNTID"; } 
        ACCOUNTID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct ACCOUNTNAME : public DB_Column<wxString>
    { 
        static wxString name() { return "ACCOUNTNAME"; } 
        ACCOUNTNAME(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct ACCOUNTTYPE : public DB_Column<wxString>
    { 
        static wxString name() { return "ACCOUNTTYPE"; } 
        ACCOUNTTYPE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct ACCOUNTNUM : public DB_Column<wxString>
    { 
        static wxString name() { return "ACCOUNTNUM"; } 
        ACCOUNTNUM(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct STATUS : public DB_Column<wxString>
    { 
        static wxString name() { return "STATUS"; } 
        STATUS(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct NOTES : public DB_Column<wxString>
    { 
        static wxString name() { return "NOTES"; } 
        NOTES(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct HELDAT : public DB_Column<wxString>
    { 
        static wxString name() { return "HELDAT"; } 
        HELDAT(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct WEBSITE : public DB_Column<wxString>
    { 
        static wxString name() { return "WEBSITE"; } 
        WEBSITE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct CONTACTINFO : public DB_Column<wxString>
    { 
        static wxString name() { return "CONTACTINFO"; } 
        CONTACTINFO(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct ACCESSINFO : public DB_Column<wxString>
    { 
        static wxString name() { return "ACCESSINFO"; } 
        ACCESSINFO(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct INITIALBAL : public DB_Column<double>
    { 
        static wxString name() { return "INITIALBAL"; } 
        INITIALBAL(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    struct FAVORITEACCT : public DB_Column<wxString>
    { 
        static wxString name() { return "FAVORITEACCT"; } 
        FAVORITEACCT(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct CURRENCYID : public DB_Column<int>
    { 
        static wxString name() { return "CURRENCYID"; } 
        CURRENCYID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    typedef ACCOUNTID PRIMARY;
    enum COLUMN
    {
        COL_ACCOUNTID = 0
        , COL_ACCOUNTNAME = 1
        , COL_ACCOUNTTYPE = 2
        , COL_ACCOUNTNUM = 3
        , COL_STATUS = 4
        , COL_NOTES = 5
        , COL_HELDAT = 6
        , COL_WEBSITE = 7
        , COL_CONTACTINFO = 8
        , COL_ACCESSINFO = 9
        , COL_INITIALBAL = 10
        , COL_FAVORITEACCT = 11
        , COL_CURRENCYID = 12
    };

    /** Returns the column name as a string*/
    static wxString column_to_name(COLUMN col)
    {
        switch(col)
        {
            case COL_ACCOUNTID: return "ACCOUNTID";
            case COL_ACCOUNTNAME: return "ACCOUNTNAME";
            case COL_ACCOUNTTYPE: return "ACCOUNTTYPE";
            case COL_ACCOUNTNUM: return "ACCOUNTNUM";
            case COL_STATUS: return "STATUS";
            case COL_NOTES: return "NOTES";
            case COL_HELDAT: return "HELDAT";
            case COL_WEBSITE: return "WEBSITE";
            case COL_CONTACTINFO: return "CONTACTINFO";
            case COL_ACCESSINFO: return "ACCESSINFO";
            case COL_INITIALBAL: return "INITIALBAL";
            case COL_FAVORITEACCT: return "FAVORITEACCT";
            case COL_CURRENCYID: return "CURRENCYID";
            default: break;
        }
        
        return "UNKNOWN";
    }

    /** Returns the comumn number from the given column name*/
    static COLUMN name_to_column(const wxString& name)
    {
        if ("ACCOUNTID" == name) return COL_ACCOUNTID;
        else if ("ACCOUNTNAME" == name) return COL_ACCOUNTNAME;
        else if ("ACCOUNTTYPE" == name) return COL_ACCOUNTTYPE;
        else if ("ACCOUNTNUM" == name) return COL_ACCOUNTNUM;
        else if ("STATUS" == name) return COL_STATUS;
        else if ("NOTES" == name) return COL_NOTES;
        else if ("HELDAT" == name) return COL_HELDAT;
        else if ("WEBSITE" == name) return COL_WEBSITE;
        else if ("CONTACTINFO" == name) return COL_CONTACTINFO;
        else if ("ACCESSINFO" == name) return COL_ACCESSINFO;
        else if ("INITIALBAL" == name) return COL_INITIALBAL;
        else if ("FAVORITEACCT" == name) return COL_FAVORITEACCT;
        else if ("CURRENCYID" == name) return COL_CURRENCYID;

        return COLUMN(-1);
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_ACCOUNTLIST_V1;
        /** This is a instance pointer to itself in memory. */
        Self* view_;
    
        int ACCOUNTID;//  primay key
        wxString ACCOUNTNAME;
        wxString ACCOUNTTYPE;
        wxString ACCOUNTNUM;
        wxString STATUS;
        wxString NOTES;
        wxString HELDAT;
        wxString WEBSITE;
        wxString CONTACTINFO;
        wxString ACCESSINFO;
        double INITIALBAL;
        wxString FAVORITEACCT;
        int CURRENCYID;
        int id() const { return ACCOUNTID; }
        void id(int id) { ACCOUNTID = id; }
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
        
            ACCOUNTID = -1;
            INITIALBAL = 0.0;
            CURRENCYID = -1;
        }

        Data(wxSQLite3ResultSet& q, Self* view = 0)
        {
            view_ = view;
        
            ACCOUNTID = q.GetInt(0); // ACCOUNTID
            ACCOUNTNAME = q.GetString(1); // ACCOUNTNAME
            ACCOUNTTYPE = q.GetString(2); // ACCOUNTTYPE
            ACCOUNTNUM = q.GetString(3); // ACCOUNTNUM
            STATUS = q.GetString(4); // STATUS
            NOTES = q.GetString(5); // NOTES
            HELDAT = q.GetString(6); // HELDAT
            WEBSITE = q.GetString(7); // WEBSITE
            CONTACTINFO = q.GetString(8); // CONTACTINFO
            ACCESSINFO = q.GetString(9); // ACCESSINFO
            INITIALBAL = q.GetDouble(10); // INITIALBAL
            FAVORITEACCT = q.GetString(11); // FAVORITEACCT
            CURRENCYID = q.GetInt(12); // CURRENCYID
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
            o["ACCOUNTID"] = json::Number(this->ACCOUNTID);
            o["ACCOUNTNAME"] = json::String(this->ACCOUNTNAME.ToStdString());
            o["ACCOUNTTYPE"] = json::String(this->ACCOUNTTYPE.ToStdString());
            o["ACCOUNTNUM"] = json::String(this->ACCOUNTNUM.ToStdString());
            o["STATUS"] = json::String(this->STATUS.ToStdString());
            o["NOTES"] = json::String(this->NOTES.ToStdString());
            o["HELDAT"] = json::String(this->HELDAT.ToStdString());
            o["WEBSITE"] = json::String(this->WEBSITE.ToStdString());
            o["CONTACTINFO"] = json::String(this->CONTACTINFO.ToStdString());
            o["ACCESSINFO"] = json::String(this->ACCESSINFO.ToStdString());
            o["INITIALBAL"] = json::Number(this->INITIALBAL);
            o["FAVORITEACCT"] = json::String(this->FAVORITEACCT.ToStdString());
            o["CURRENCYID"] = json::Number(this->CURRENCYID);
            return 0;
        }
        row_t to_row_t() const
        {
            row_t row;
            row("ACCOUNTID") = ACCOUNTID;
            row("ACCOUNTNAME") = ACCOUNTNAME;
            row("ACCOUNTTYPE") = ACCOUNTTYPE;
            row("ACCOUNTNUM") = ACCOUNTNUM;
            row("STATUS") = STATUS;
            row("NOTES") = NOTES;
            row("HELDAT") = HELDAT;
            row("WEBSITE") = WEBSITE;
            row("CONTACTINFO") = CONTACTINFO;
            row("ACCESSINFO") = ACCESSINFO;
            row("INITIALBAL") = INITIALBAL;
            row("FAVORITEACCT") = FAVORITEACCT;
            row("CURRENCYID") = CURRENCYID;
            return row;
        }
        void to_template(html_template& t) const
        {
            t("ACCOUNTID") = ACCOUNTID;
            t("ACCOUNTNAME") = ACCOUNTNAME;
            t("ACCOUNTTYPE") = ACCOUNTTYPE;
            t("ACCOUNTNUM") = ACCOUNTNUM;
            t("STATUS") = STATUS;
            t("NOTES") = NOTES;
            t("HELDAT") = HELDAT;
            t("WEBSITE") = WEBSITE;
            t("CONTACTINFO") = CONTACTINFO;
            t("ACCESSINFO") = ACCESSINFO;
            t("INITIALBAL") = INITIALBAL;
            t("FAVORITEACCT") = FAVORITEACCT;
            t("CURRENCYID") = CURRENCYID;
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db)
        {
            if (!view_ || !db) 
            {
                wxLogError("can not save ACCOUNTLIST_V1");
                return false;
            }

            return view_->save(this, db);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!view_ || !db) 
            {
                wxLogError("can not remove ACCOUNTLIST_V1");
                return false;
            }
            
            return view_->remove(this, db);
        }

        void destroy() { delete this; }
    };

    enum
    {
        NUM_COLUMNS = 13
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    /** Name of the table*/    
    wxString name() const { return "ACCOUNTLIST_V1"; }

    DB_Table_ACCOUNTLIST_V1() 
    {
        query_ = "SELECT * FROM ACCOUNTLIST_V1 ";
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
            sql = "INSERT INTO ACCOUNTLIST_V1(ACCOUNTNAME, ACCOUNTTYPE, ACCOUNTNUM, STATUS, NOTES, HELDAT, WEBSITE, CONTACTINFO, ACCESSINFO, INITIALBAL, FAVORITEACCT, CURRENCYID) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE ACCOUNTLIST_V1 SET ACCOUNTNAME = ?, ACCOUNTTYPE = ?, ACCOUNTNUM = ?, STATUS = ?, NOTES = ?, HELDAT = ?, WEBSITE = ?, CONTACTINFO = ?, ACCESSINFO = ?, INITIALBAL = ?, FAVORITEACCT = ?, CURRENCYID = ? WHERE ACCOUNTID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->ACCOUNTNAME);
            stmt.Bind(2, entity->ACCOUNTTYPE);
            stmt.Bind(3, entity->ACCOUNTNUM);
            stmt.Bind(4, entity->STATUS);
            stmt.Bind(5, entity->NOTES);
            stmt.Bind(6, entity->HELDAT);
            stmt.Bind(7, entity->WEBSITE);
            stmt.Bind(8, entity->CONTACTINFO);
            stmt.Bind(9, entity->ACCESSINFO);
            stmt.Bind(10, entity->INITIALBAL);
            stmt.Bind(11, entity->FAVORITEACCT);
            stmt.Bind(12, entity->CURRENCYID);
            if (entity->id() > 0)
                stmt.Bind(13, entity->ACCOUNTID);

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
            wxLogError("ACCOUNTLIST_V1: Exception %s, %s", e.GetMessage().c_str(), entity->to_json());
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
            wxString sql = "DELETE FROM ACCOUNTLIST_V1 WHERE ACCOUNTID = ?";
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
                    delete entity;
                else 
                    c.push_back(entity);
            }
            cache_.clear();
            cache_.swap(c);
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("ACCOUNTLIST_V1: Exception %s", e.GetMessage().c_str());
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
        if (id <= 0) 
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

            //wxLogDebug(stmt.GetSQL());
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
