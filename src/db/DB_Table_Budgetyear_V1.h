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
 *          AUTO GENERATED at 2013-10-20 09:15:08.699779.
 *          DO NOT EDIT!
 */
//=============================================================================

#ifndef DB_TABLE_BUDGETYEAR_V1_H
#define DB_TABLE_BUDGETYEAR_V1_H

#include "DB_Table.h"

struct DB_Table_BUDGETYEAR_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_BUDGETYEAR_V1 Self;
    typedef std::vector<Self::Data> Data_Set;
    typedef std::vector<Self::Data*> Cache;
    Cache cache_;
    ~DB_Table_BUDGETYEAR_V1() 
    {
        std::for_each(cache_.begin(), cache_.end(), std::mem_fun(&Data::destroy));
    }

    bool ensure(wxSQLite3Database* db) const
    {
        if (exists(db)) return true;

        try
        {
            db->ExecuteUpdate("CREATE TABLE BUDGETYEAR_V1(BUDGETYEARID integer primary key, BUDGETYEARNAME TEXT NOT NULL UNIQUE)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("BUDGETYEAR_V1: Exception %s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }

    struct BUDGETYEARID : public DB_Column<int>
    { 
        static wxString name() { return "BUDGETYEARID"; } 
        BUDGETYEARID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct BUDGETYEARNAME : public DB_Column<wxString>
    { 
        static wxString name() { return "BUDGETYEARNAME"; } 
        BUDGETYEARNAME(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    typedef BUDGETYEARID PRIMARY;
    enum COLUMN
    {
        COL_BUDGETYEARID = 0
        , COL_BUDGETYEARNAME = 1
    };

    std::vector<COLUMN> all_columns() const
    {
        std::vector<COLUMN> result;
        result.push_back(COL_BUDGETYEARID);
        result.push_back(COL_BUDGETYEARNAME);
        return result;
    }

    wxString column_to_name(COLUMN col) const
    {
        switch(col)
        {
            case COL_BUDGETYEARID: return "BUDGETYEARID";
            case COL_BUDGETYEARNAME: return "BUDGETYEARNAME";
            default: break;
        }
        
        return "UNKNOWN";
    }

    COLUMN name_to_column(const wxString& name) const
    {
        if ("BUDGETYEARID" == name) return COL_BUDGETYEARID;
        else if ("BUDGETYEARNAME" == name) return COL_BUDGETYEARNAME;

        return COLUMN(-1);
    }
    
    struct Data
    {
        friend struct DB_Table_BUDGETYEAR_V1;
        Self* view_;
    
        int BUDGETYEARID;//  primay key
        wxString BUDGETYEARNAME;
        int id() const { return BUDGETYEARID; }
        void id(int id) { BUDGETYEARID = id; }
        bool operator < (const Data& r) const
        {
            return this->id() < r.id();
        }
        bool operator < (const Data* r) const
        {
            return this->id() < r->id();
        }

        Data(Self* view) 
        {
            view_ = view;
        
            BUDGETYEARID = -1;
        }

        Data(wxSQLite3ResultSet& q, Self* view = 0)
        {
            view_ = view;
        
            BUDGETYEARID = q.GetInt("BUDGETYEARID");
            BUDGETYEARNAME = q.GetString("BUDGETYEARNAME");
        }

        wxString to_string(COLUMN col) const
        {
            wxString ret = wxEmptyString;
            switch(col)
            {
                case COL_BUDGETYEARID: ret << BUDGETYEARID; break;
                case COL_BUDGETYEARNAME: ret << BUDGETYEARNAME; break;
                default: break;
            }
            
            return ret;
        }

        wxString to_string(std::vector<COLUMN> columns, const wxString& delimiter = ",") const
        {
            wxString ret = wxEmptyString;
            std::vector<COLUMN>::const_iterator it = columns.begin(); 
            if (it != columns.end()) ret << to_string(*it);
            for ( ; it != columns.end(); ++ it) ret << delimiter << to_string(*it);

            return ret;
        }

        wxString to_string(const wxString& delimiter = ",") const
        {
            wxString ret = wxEmptyString;
            ret << BUDGETYEARID;
            ret << delimiter << BUDGETYEARNAME;
            return ret;
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
            o["BUDGETYEARID"] = json::Number(this->BUDGETYEARID);
            o["BUDGETYEARNAME"] = json::String(this->BUDGETYEARNAME.ToStdString());
            return 0;
        }
        bool save(wxSQLite3Database* db)
        {
            if (!view_ || !db) 
            {
                wxLogError("can not save");
                return false;
            }

            return view_->save(this, db);
        }

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
        NUM_COLUMNS = 2
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    wxString name() const { return "BUDGETYEAR_V1"; }

    DB_Table_BUDGETYEAR_V1() 
    {
        query_ = "SELECT BUDGETYEARID, BUDGETYEARNAME FROM BUDGETYEAR_V1 ";
    }

    Self::Data* create()
    {
        Self::Data* entity = new Self::Data(this);
        cache_.push_back(entity);
        return entity;
    }
    Self::Data* clone(const Data* e)
    {
        Self::Data* entity = create();
        *entity = *e;
        entity->id(-1);
        return entity;
    }

    bool save(Self::Data* entity, wxSQLite3Database* db)
    {
        wxString sql = wxEmptyString;
        if (entity->id() < 0) //  new & insert
        {
            sql = "INSERT INTO BUDGETYEAR_V1(BUDGETYEARNAME) VALUES(?)";
        }
        else
        {
            sql = "UPDATE BUDGETYEAR_V1 SET BUDGETYEARNAME = ? WHERE BUDGETYEARID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->BUDGETYEARNAME);
            if (entity->id() > 0)
                stmt.Bind(2, entity->BUDGETYEARID);

            wxLogDebug(stmt.GetSQL());
            stmt.ExecuteUpdate();
            stmt.Finalize();
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("BUDGETYEAR_V1: Exception %s, %s", e.GetMessage().c_str(), entity->to_json());
            return false;
        }

        if (entity->id() < 0) entity->id((db->GetLastRowId()).ToLong());
        return true;
    }

    bool remove(int id, wxSQLite3Database* db)
    {
        try
        {
            wxString sql = "DELETE FROM BUDGETYEAR_V1 WHERE BUDGETYEARID = ?";
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
            wxLogError("BUDGETYEAR_V1: Exception %s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }

    bool remove(Self::Data* entity, wxSQLite3Database* db)
    {
        if (remove(entity->id(), db))
        {
            entity->id(-1);
            return true;
        }

        return false;
    }

    
    Self::Data* get(int id, wxSQLite3Database* db)
    {
        if (id < 0) 
        {
            ++ skip_;
            wxLogDebug("%s :%d SKIP (hit %ld, miss %ld, skip %ld)", this->name(), id, this->hit_, this->miss_, this->skip_);
            return 0;
        }
        for(Cache::iterator it = cache_.begin(); it != cache_.end(); ++ it)
        {
            Self::Data* entity = *it;
            if (entity->id() == id) 
            {
                ++ hit_;
                wxLogDebug("%s :%d HIT (hit %ld, miss %ld, skip %ld)", this->name(), id, this->hit_, this->miss_, this->skip_);
                return entity;
            }
        }
        
        ++ miss_;
        wxLogDebug("%s :%d MISS (hit %ld, miss %ld, skip %ld)", this->name(), id, this->hit_, this->miss_, this->skip_);
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

    Data_Set all(wxSQLite3Database* db, COLUMN col = COLUMN(0), bool asc = true)
    {
        Data_Set result;
        try
        {
            wxSQLite3ResultSet q = db->ExecuteQuery(this->query() + " ORDER BY " + column_to_name(col) + " COLLATE NOCASE " + (asc ? " ASC " : " DESC ")
                + "," + PRIMARY::name());

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
