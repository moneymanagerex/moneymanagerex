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
 *          AUTO GENERATED at 2013-10-11 14:41:03.820623.
 *          DO NOT EDIT!
 */
//=============================================================================

#ifndef DB_TABLE_BUDGETSPLITTRANSACTIONS_V1_H
#define DB_TABLE_BUDGETSPLITTRANSACTIONS_V1_H

#include "DB_Table.h"

struct DB_Table_BUDGETSPLITTRANSACTIONS_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_BUDGETSPLITTRANSACTIONS_V1 Self;
    typedef std::vector<Self::Data> Data_Set;
    typedef std::vector<Self::Data*> Cache;
    Cache cache_;
    ~DB_Table_BUDGETSPLITTRANSACTIONS_V1() 
    {
        std::for_each(cache_.begin(), cache_.end(), std::mem_fun(&Data::destroy));
    }

    bool ensure(wxSQLite3Database* db) const
    {
        if (exists(db)) return true;

        try
        {
            db->ExecuteUpdate("CREATE TABLE BUDGETSPLITTRANSACTIONS_V1(SPLITTRANSID integer primary key, TRANSID integer NOT NULL, CATEGID integer, SUBCATEGID integer, SPLITTRANSAMOUNT numeric)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("BUDGETSPLITTRANSACTIONS_V1: Exception %s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }

    struct SPLITTRANSID : public DB_Column<int>
    { 
        static wxString name() { return "SPLITTRANSID"; } 
        SPLITTRANSID(const int &v): DB_Column<int>(v) {}
    };
    struct TRANSID : public DB_Column<int>
    { 
        static wxString name() { return "TRANSID"; } 
        TRANSID(const int &v): DB_Column<int>(v) {}
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
    struct SPLITTRANSAMOUNT : public DB_Column<double>
    { 
        static wxString name() { return "SPLITTRANSAMOUNT"; } 
        SPLITTRANSAMOUNT(const double &v): DB_Column<double>(v) {}
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

    std::vector<COLUMN> all_columns() const
    {
        std::vector<COLUMN> result;
        result.push_back(COL_SPLITTRANSID);
        result.push_back(COL_TRANSID);
        result.push_back(COL_CATEGID);
        result.push_back(COL_SUBCATEGID);
        result.push_back(COL_SPLITTRANSAMOUNT);
        return result;
    }

    wxString column_to_name(COLUMN col) const
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

    COLUMN name_to_column(const wxString& name) const
    {
        if ("SPLITTRANSID" == name) return COL_SPLITTRANSID;
        else if ("TRANSID" == name) return COL_TRANSID;
        else if ("CATEGID" == name) return COL_CATEGID;
        else if ("SUBCATEGID" == name) return COL_SUBCATEGID;
        else if ("SPLITTRANSAMOUNT" == name) return COL_SPLITTRANSAMOUNT;

        return COLUMN(-1);
    }
    
    struct Data
    {
        friend struct DB_Table_BUDGETSPLITTRANSACTIONS_V1;
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

    private:
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
        
            SPLITTRANSID = q.GetInt("SPLITTRANSID");
            TRANSID = q.GetInt("TRANSID");
            CATEGID = q.GetInt("CATEGID");
            SUBCATEGID = q.GetInt("SUBCATEGID");
            SPLITTRANSAMOUNT = q.GetDouble("SPLITTRANSAMOUNT");
        }
    public:

        wxString to_string(COLUMN col) const
        {
            wxString ret = wxEmptyString;
            switch(col)
            {
                case COL_SPLITTRANSID: ret << SPLITTRANSID; break;
                case COL_TRANSID: ret << TRANSID; break;
                case COL_CATEGID: ret << CATEGID; break;
                case COL_SUBCATEGID: ret << SUBCATEGID; break;
                case COL_SPLITTRANSAMOUNT: ret << SPLITTRANSAMOUNT; break;
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
            ret << SPLITTRANSID;
            ret << delimiter << TRANSID;
            ret << delimiter << CATEGID;
            ret << delimiter << SUBCATEGID;
            ret << delimiter << SPLITTRANSAMOUNT;
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
            o["SPLITTRANSID"] = json::Number(this->SPLITTRANSID);
            o["TRANSID"] = json::Number(this->TRANSID);
            o["CATEGID"] = json::Number(this->CATEGID);
            o["SUBCATEGID"] = json::Number(this->SUBCATEGID);
            o["SPLITTRANSAMOUNT"] = json::Number(this->SPLITTRANSAMOUNT);
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
        NUM_COLUMNS = 5
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    wxString name() const { return "BUDGETSPLITTRANSACTIONS_V1"; }

    DB_Table_BUDGETSPLITTRANSACTIONS_V1() 
    {
        query_ = "SELECT SPLITTRANSID, TRANSID, CATEGID, SUBCATEGID, SPLITTRANSAMOUNT FROM BUDGETSPLITTRANSACTIONS_V1 ";
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
            sql = "INSERT INTO BUDGETSPLITTRANSACTIONS_V1(TRANSID, CATEGID, SUBCATEGID, SPLITTRANSAMOUNT) VALUES(?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE BUDGETSPLITTRANSACTIONS_V1 SET TRANSID = ?, CATEGID = ?, SUBCATEGID = ?, SPLITTRANSAMOUNT = ? WHERE SPLITTRANSID = ?";
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

            stmt.ExecuteUpdate();
            stmt.Finalize();
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("BUDGETSPLITTRANSACTIONS_V1: Exception %s, %s", e.GetMessage().c_str(), entity->to_json());
            return false;
        }

        if (entity->id() < 0) entity->id((db->GetLastRowId()).ToLong());
        return true;
    }

    bool remove(int id, wxSQLite3Database* db)
    {
        try
        {
            wxString sql = "DELETE FROM BUDGETSPLITTRANSACTIONS_V1 WHERE SPLITTRANSID = ?";
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
            wxLogError("BUDGETSPLITTRANSACTIONS_V1: Exception %s", e.GetMessage().c_str());
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
        wxString where = wxString::Format(" WHERE SPLITTRANSID = ?");
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
            wxLogError("BUDGETSPLITTRANSACTIONS_V1: Exception %s", e.GetMessage().c_str());
        }
        
        if (!entity) 
        {
            wxLogError("%s: %d not found", this->name().c_str(), id);
        }
 
        return entity;
    }

    template<class V>
    Data_Set find(wxSQLite3Database* db, COLUMN col, const V& v)
    {
        Data_Set result;
        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(this->query() + " WHERE " 
                + column_to_name(col) + " = ?"
                + " ORDER BY " + column_to_name(col)
                );
            stmt.Bind(1, v);
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
            wxLogError("BUDGETSPLITTRANSACTIONS_V1: Exception %s", e.GetMessage().c_str());
        }

        return result;
    }

};
#endif //
