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
 *          AUTO GENERATED at 2013-10-11 15:31:37.712883.
 *          DO NOT EDIT!
 */
//=============================================================================

#ifndef DB_TABLE_PAYEE_V1_H
#define DB_TABLE_PAYEE_V1_H

#include "DB_Table.h"

struct DB_Table_PAYEE_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_PAYEE_V1 Self;
    typedef std::vector<Self::Data> Data_Set;
    typedef std::vector<Self::Data*> Cache;
    Cache cache_;
    ~DB_Table_PAYEE_V1() 
    {
        std::for_each(cache_.begin(), cache_.end(), std::mem_fun(&Data::destroy));
    }

    bool ensure(wxSQLite3Database* db) const
    {
        if (exists(db)) return true;

        try
        {
            db->ExecuteUpdate("CREATE TABLE PAYEE_V1(PAYEEID integer primary key, PAYEENAME TEXT COLLATE NOCASE NOT NULL UNIQUE, CATEGID integer, SUBCATEGID integer)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("PAYEE_V1: Exception %s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }

    struct PAYEEID : public DB_Column<int>
    { 
        static wxString name() { return "PAYEEID"; } 
        PAYEEID(const int &v): DB_Column<int>(v) {}
    };
    struct PAYEENAME : public DB_Column<wxString>
    { 
        static wxString name() { return "PAYEENAME"; } 
        PAYEENAME(const wxString &v): DB_Column<wxString>(v) {}
    };
    struct CATEGID : public DB_Column<int>
    { 
        static wxString name() { return "CATEGID"; } 
        CATEGID(const int &v): DB_Column<int>(v) {}
    };
    struct SUBCATEGID : public DB_Column<int>
    { 
        static wxString name() { return "SUBCATEGID"; } 
        SUBCATEGID(const int &v): DB_Column<int>(v) {}
    };
    typedef PAYEEID PRIMARY;
    enum COLUMN
    {
        COL_PAYEEID = 0
        , COL_PAYEENAME = 1
        , COL_CATEGID = 2
        , COL_SUBCATEGID = 3
    };

    std::vector<COLUMN> all_columns() const
    {
        std::vector<COLUMN> result;
        result.push_back(COL_PAYEEID);
        result.push_back(COL_PAYEENAME);
        result.push_back(COL_CATEGID);
        result.push_back(COL_SUBCATEGID);
        return result;
    }

    wxString column_to_name(COLUMN col) const
    {
        switch(col)
        {
            case COL_PAYEEID: return "PAYEEID";
            case COL_PAYEENAME: return "PAYEENAME";
            case COL_CATEGID: return "CATEGID";
            case COL_SUBCATEGID: return "SUBCATEGID";
            default: break;
        }
        
        return "UNKNOWN";
    }

    COLUMN name_to_column(const wxString& name) const
    {
        if ("PAYEEID" == name) return COL_PAYEEID;
        else if ("PAYEENAME" == name) return COL_PAYEENAME;
        else if ("CATEGID" == name) return COL_CATEGID;
        else if ("SUBCATEGID" == name) return COL_SUBCATEGID;

        return COLUMN(-1);
    }
    
    struct Data
    {
        friend struct DB_Table_PAYEE_V1;
        Self* view_;
    
        int PAYEEID;//  primay key
        wxString PAYEENAME;
        int CATEGID;
        int SUBCATEGID;
        int id() const { return PAYEEID; }
        void id(int id) { PAYEEID = id; }
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
        
            PAYEEID = -1;
            CATEGID = -1;
            SUBCATEGID = -1;
        }

        Data(wxSQLite3ResultSet& q, Self* view = 0)
        {
            view_ = view;
        
            PAYEEID = q.GetInt("PAYEEID");
            PAYEENAME = q.GetString("PAYEENAME");
            CATEGID = q.GetInt("CATEGID");
            SUBCATEGID = q.GetInt("SUBCATEGID");
        }
    public:

        wxString to_string(COLUMN col) const
        {
            wxString ret = wxEmptyString;
            switch(col)
            {
                case COL_PAYEEID: ret << PAYEEID; break;
                case COL_PAYEENAME: ret << PAYEENAME; break;
                case COL_CATEGID: ret << CATEGID; break;
                case COL_SUBCATEGID: ret << SUBCATEGID; break;
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
            ret << PAYEEID;
            ret << delimiter << PAYEENAME;
            ret << delimiter << CATEGID;
            ret << delimiter << SUBCATEGID;
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
            o["PAYEEID"] = json::Number(this->PAYEEID);
            o["PAYEENAME"] = json::String(this->PAYEENAME.ToStdString());
            o["CATEGID"] = json::Number(this->CATEGID);
            o["SUBCATEGID"] = json::Number(this->SUBCATEGID);
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
        NUM_COLUMNS = 4
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    wxString name() const { return "PAYEE_V1"; }

    DB_Table_PAYEE_V1() 
    {
        query_ = "SELECT PAYEEID, PAYEENAME, CATEGID, SUBCATEGID FROM PAYEE_V1 ";
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
            sql = "INSERT INTO PAYEE_V1(PAYEENAME, CATEGID, SUBCATEGID) VALUES(?, ?, ?)";
        }
        else
        {
            sql = "UPDATE PAYEE_V1 SET PAYEENAME = ?, CATEGID = ?, SUBCATEGID = ? WHERE PAYEEID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->PAYEENAME);
            stmt.Bind(2, entity->CATEGID);
            stmt.Bind(3, entity->SUBCATEGID);
            if (entity->id() > 0)
                stmt.Bind(4, entity->PAYEEID);

            stmt.ExecuteUpdate();
            stmt.Finalize();
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("PAYEE_V1: Exception %s, %s", e.GetMessage().c_str(), entity->to_json());
            return false;
        }

        if (entity->id() < 0) entity->id((db->GetLastRowId()).ToLong());
        return true;
    }

    bool remove(int id, wxSQLite3Database* db)
    {
        try
        {
            wxString sql = "DELETE FROM PAYEE_V1 WHERE PAYEEID = ?";
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
            wxLogError("PAYEE_V1: Exception %s", e.GetMessage().c_str());
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
        wxString where = wxString::Format(" WHERE PAYEEID = ?");
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
            wxLogError("PAYEE_V1: Exception %s", e.GetMessage().c_str());
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
    Data_Set find(wxSQLite3Database* db, COLUMN col1, const V1& v1, COLUMN col2, const V2& v2, bool op_and = true)
    {
        Data_Set result;
        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(this->query() + " WHERE "
                                                                + column_to_name(col1) + " = ? "
                                                                + (op_and ? " AND " : " OR ")
                                                                + column_to_name(col2) + " = ?"
                                                                + " ORDER BY " + column_to_name(col1)
                                                                + "," + column_to_name(col2)
                                                                );
            stmt.Bind(1, v1);
            stmt.Bind(2, v2);
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
            wxLogError("PAYEE_V1: Exception %s", e.GetMessage().c_str());
        }

        return result;
    }

};
#endif //
