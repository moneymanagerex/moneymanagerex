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
 *          AUTO GENERATED at 2013-10-11 15:47:04.704673.
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
    typedef std::vector<Self::Data> Data_Set;
    typedef std::vector<Self::Data*> Cache;
    Cache cache_;
    ~DB_Table_ASSETS_V1() 
    {
        std::for_each(cache_.begin(), cache_.end(), std::mem_fun(&Data::destroy));
    }

    bool ensure(wxSQLite3Database* db) const
    {
        if (exists(db)) return true;

        try
        {
            db->ExecuteUpdate("CREATE TABLE ASSETS_V1(ASSETID integer primary key, STARTDATE TEXT NOT NULL , ASSETNAME TEXT COLLATE NOCASE NOT NULL UNIQUE, VALUE numeric, VALUECHANGE TEXT, NOTES TEXT, VALUECHANGERATE numeric, ASSETTYPE TEXT)");
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
        ASSETID(const int &v): DB_Column<int>(v) {}
    };
    struct STARTDATE : public DB_Column<wxString>
    { 
        static wxString name() { return "STARTDATE"; } 
        STARTDATE(const wxString &v): DB_Column<wxString>(v) {}
    };
    struct ASSETNAME : public DB_Column<wxString>
    { 
        static wxString name() { return "ASSETNAME"; } 
        ASSETNAME(const wxString &v): DB_Column<wxString>(v) {}
    };
    struct VALUE : public DB_Column<double>
    { 
        static wxString name() { return "VALUE"; } 
        VALUE(const double &v): DB_Column<double>(v) {}
    };
    struct VALUECHANGE : public DB_Column<wxString>
    { 
        static wxString name() { return "VALUECHANGE"; } 
        VALUECHANGE(const wxString &v): DB_Column<wxString>(v) {}
    };
    struct NOTES : public DB_Column<wxString>
    { 
        static wxString name() { return "NOTES"; } 
        NOTES(const wxString &v): DB_Column<wxString>(v) {}
    };
    struct VALUECHANGERATE : public DB_Column<double>
    { 
        static wxString name() { return "VALUECHANGERATE"; } 
        VALUECHANGERATE(const double &v): DB_Column<double>(v) {}
    };
    struct ASSETTYPE : public DB_Column<wxString>
    { 
        static wxString name() { return "ASSETTYPE"; } 
        ASSETTYPE(const wxString &v): DB_Column<wxString>(v) {}
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

    std::vector<COLUMN> all_columns() const
    {
        std::vector<COLUMN> result;
        result.push_back(COL_ASSETID);
        result.push_back(COL_STARTDATE);
        result.push_back(COL_ASSETNAME);
        result.push_back(COL_VALUE);
        result.push_back(COL_VALUECHANGE);
        result.push_back(COL_NOTES);
        result.push_back(COL_VALUECHANGERATE);
        result.push_back(COL_ASSETTYPE);
        return result;
    }

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
    
    struct Data
    {
        friend struct DB_Table_ASSETS_V1;
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

    private:
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
    public:

        wxString to_string(COLUMN col) const
        {
            wxString ret = wxEmptyString;
            switch(col)
            {
                case COL_ASSETID: ret << ASSETID; break;
                case COL_STARTDATE: ret << STARTDATE; break;
                case COL_ASSETNAME: ret << ASSETNAME; break;
                case COL_VALUE: ret << VALUE; break;
                case COL_VALUECHANGE: ret << VALUECHANGE; break;
                case COL_NOTES: ret << NOTES; break;
                case COL_VALUECHANGERATE: ret << VALUECHANGERATE; break;
                case COL_ASSETTYPE: ret << ASSETTYPE; break;
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
            ret << ASSETID;
            ret << delimiter << STARTDATE;
            ret << delimiter << ASSETNAME;
            ret << delimiter << VALUE;
            ret << delimiter << VALUECHANGE;
            ret << delimiter << NOTES;
            ret << delimiter << VALUECHANGERATE;
            ret << delimiter << ASSETTYPE;
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
        NUM_COLUMNS = 8
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    wxString name() const { return "ASSETS_V1"; }

    DB_Table_ASSETS_V1() 
    {
        query_ = "SELECT ASSETID, STARTDATE, ASSETNAME, VALUE, VALUECHANGE, NOTES, VALUECHANGERATE, ASSETTYPE FROM ASSETS_V1 ";
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

    bool remove(int id, wxSQLite3Database* db)
    {
        try
        {
            wxString sql = "DELETE FROM ASSETS_V1 WHERE ASSETID = ?";
            wxSQLite3Statement stmt = db->PrepareStatement(sql);
            stmt.Bind(1, id);
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
        if (id < 0) return 0;
        for(Cache::iterator it = cache_.begin(); it != cache_.end(); ++ it)
        {
            Self::Data* entity = *it;
            if (entity->id() == id) 
                return entity;
        }

        Self::Data* entity = 0;
        wxString where = wxString::Format(" WHERE ASSETID = ?");
        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(this->query() + where);
            stmt.Bind(1, id);

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
            wxLogError("ASSETS_V1: Exception %s", e.GetMessage().c_str());
        }
        
        if (!entity) 
        {
            wxLogError("%s: %d not found", this->name().c_str(), id);
        }
 
        return entity;
    }

    template<class V>
    Data_Set find(wxSQLite3Database* db, const V& v)
    {
        Data_Set result;
        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(this->query() + " WHERE " 
                + V::name() + " = ?"
                );
            stmt.Bind(1, v.v_);
            wxSQLite3ResultSet q = stmt.ExecuteQuery();

            while(q.NextRow())
            {
                Self::Data entity(q, this);
                result.push_back(entity);
            }

            q.Finalize();
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("%%s: Exception %%s", this->name().c_str(), e.GetMessage().c_str());
        }

        return result;
    }

    template<class V1, class V2>
    Data_Set find(wxSQLite3Database* db, const V1& v1, const V2& v2, bool op_and = true)
    {
        Data_Set result;
        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(this->query() + " WHERE "
                                                                + V1::name() + " = ? "
                                                                + (op_and ? " AND " : " OR ")
                                                                + V2::name() + " = ?"
                                                                + " ORDER BY " + V1::name()
                                                                + "," + V2::name()
                                                                );
            stmt.Bind(1, v1.v_);
            stmt.Bind(2, v2.v_);
            wxSQLite3ResultSet q = stmt.ExecuteQuery();

            while(q.NextRow())
            {
                Self::Data entity(q, this);
                result.push_back(entity);
            }

            q.Finalize();
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("%%s: Exception %%s", this->name(), e.GetMessage().c_str());
        }

        return result;
    }

    Data_Set all(wxSQLite3Database* db, COLUMN col = COLUMN(0), bool asc = true)
    {
        Data_Set result;
        try
        {
            wxSQLite3ResultSet q = db->ExecuteQuery(this->query() + " ORDER BY " + column_to_name(col) + (asc ? " ASC " : " DESC ")
                + "," + PRIMARY::name());

            while(q.NextRow())
            {
                Self::Data entity(q, this);
                result.push_back(entity);
            }

            q.Finalize();
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("ASSETS_V1: Exception %s", e.GetMessage().c_str());
        }

        return result;
    }

};
#endif //
