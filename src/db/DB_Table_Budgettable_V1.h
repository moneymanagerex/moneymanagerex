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
 *          AUTO GENERATED at 2013-09-20 14:02:02.201724.
 *          DO NOT EDIT!
 */
//=============================================================================

#ifndef DB_TABLE_BUDGETTABLE_V1_H
#define DB_TABLE_BUDGETTABLE_V1_H

#include "DB_Table.h"

struct DB_Table_BUDGETTABLE_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_BUDGETTABLE_V1 Self;
    typedef std::vector<Self::Data> Data_Set;
    typedef std::vector<Self::Data*> Cache;
    Cache cache_;
    ~DB_Table_BUDGETTABLE_V1() 
    {
        std::for_each(cache_.begin(), cache_.end(), std::mem_fun(&Data::destroy));
    }

    bool ensure(wxSQLite3Database* db) const
    {
        if (exists(db)) return true;

        try
        {
            db->ExecuteUpdate("CREATE TABLE BUDGETTABLE_V1(BUDGETENTRYID integer primary key, BUDGETYEARID integer, CATEGID integer, SUBCATEGID integer, PERIOD TEXT NOT NULL, AMOUNT numeric NOT NULL)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("BUDGETTABLE_V1: Exception %s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }

    struct BUDGETENTRYID { wxString name() const { return "BUDGETENTRYID"; } };
    struct BUDGETYEARID { wxString name() const { return "BUDGETYEARID"; } };
    struct CATEGID { wxString name() const { return "CATEGID"; } };
    struct SUBCATEGID { wxString name() const { return "SUBCATEGID"; } };
    struct PERIOD { wxString name() const { return "PERIOD"; } };
    struct AMOUNT { wxString name() const { return "AMOUNT"; } };
    typedef BUDGETENTRYID PRIMARY;
    enum COLUMN
    {
        COL_BUDGETENTRYID = 0
        , COL_BUDGETYEARID = 1
        , COL_CATEGID = 2
        , COL_SUBCATEGID = 3
        , COL_PERIOD = 4
        , COL_AMOUNT = 5
    };

    std::vector<COLUMN> all_columns() const
    {
        std::vector<COLUMN> result;
        result.push_back(COL_BUDGETENTRYID);
        result.push_back(COL_BUDGETYEARID);
        result.push_back(COL_CATEGID);
        result.push_back(COL_SUBCATEGID);
        result.push_back(COL_PERIOD);
        result.push_back(COL_AMOUNT);
        return result;
    }

    wxString column_to_name(COLUMN col) const
    {
        switch(col)
        {
            case COL_BUDGETENTRYID: return "BUDGETENTRYID";
            case COL_BUDGETYEARID: return "BUDGETYEARID";
            case COL_CATEGID: return "CATEGID";
            case COL_SUBCATEGID: return "SUBCATEGID";
            case COL_PERIOD: return "PERIOD";
            case COL_AMOUNT: return "AMOUNT";
            default: break;
        }
        
        return "UNKNOWN";
    }

    COLUMN name_to_column(const wxString& name) const
    {
        if ("BUDGETENTRYID" == name) return COL_BUDGETENTRYID;
        else if ("BUDGETYEARID" == name) return COL_BUDGETYEARID;
        else if ("CATEGID" == name) return COL_CATEGID;
        else if ("SUBCATEGID" == name) return COL_SUBCATEGID;
        else if ("PERIOD" == name) return COL_PERIOD;
        else if ("AMOUNT" == name) return COL_AMOUNT;

        return COLUMN(-1);
    }
    
    struct Data
    {
        Self* view_;
    
        int BUDGETENTRYID;//  primay key
        int BUDGETYEARID;
        int CATEGID;
        int SUBCATEGID;
        wxString PERIOD;
        double AMOUNT;
        int id() const { return BUDGETENTRYID; }
        void id(int id) { BUDGETENTRYID = id; }

        Data(Self* view = 0) 
        {
            view_ = view;
        
            BUDGETENTRYID = -1;
            BUDGETYEARID = -1;
            CATEGID = -1;
            SUBCATEGID = -1;
            AMOUNT = 0.0;
        }

        Data(wxSQLite3ResultSet& q, Self* view = 0)
        {
            view_ = view;
        
            BUDGETENTRYID = q.GetInt("BUDGETENTRYID");
            BUDGETYEARID = q.GetInt("BUDGETYEARID");
            CATEGID = q.GetInt("CATEGID");
            SUBCATEGID = q.GetInt("SUBCATEGID");
            PERIOD = q.GetString("PERIOD");
            AMOUNT = q.GetDouble("AMOUNT");
        }

        wxString to_string(COLUMN col) const
        {
            wxString ret = wxEmptyString;
            switch(col)
            {
                case COL_BUDGETENTRYID: ret << BUDGETENTRYID; break;
                case COL_BUDGETYEARID: ret << BUDGETYEARID; break;
                case COL_CATEGID: ret << CATEGID; break;
                case COL_SUBCATEGID: ret << SUBCATEGID; break;
                case COL_PERIOD: ret << PERIOD; break;
                case COL_AMOUNT: ret << AMOUNT; break;
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
            ret << BUDGETENTRYID;
            ret << delimiter << BUDGETYEARID;
            ret << delimiter << CATEGID;
            ret << delimiter << SUBCATEGID;
            ret << delimiter << PERIOD;
            ret << delimiter << AMOUNT;
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
            o["BUDGETENTRYID"] = json::Number(this->BUDGETENTRYID);
            o["BUDGETYEARID"] = json::Number(this->BUDGETYEARID);
            o["CATEGID"] = json::Number(this->CATEGID);
            o["SUBCATEGID"] = json::Number(this->SUBCATEGID);
            o["PERIOD"] = json::String(this->PERIOD.ToStdString());
            o["AMOUNT"] = json::Number(this->AMOUNT);
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
        NUM_COLUMNS = 6
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    wxString name() const { return "BUDGETTABLE_V1"; }

    DB_Table_BUDGETTABLE_V1() 
    {
        query_ = "SELECT BUDGETENTRYID, BUDGETYEARID, CATEGID, SUBCATEGID, PERIOD, AMOUNT FROM BUDGETTABLE_V1 ";
    }

    Self::Data* create()
    {
        Self::Data* entity = new Self::Data(this);
        cache_.push_back(entity);
        return entity;
    }

    bool save(Self::Data* entity, wxSQLite3Database* db)
    {
        wxString sql = wxEmptyString;
        if (entity->id() < 0) //  new & insert
        {
            sql = "INSERT INTO BUDGETTABLE_V1(BUDGETYEARID, CATEGID, SUBCATEGID, PERIOD, AMOUNT) VALUES(?, ?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE BUDGETTABLE_V1 SET BUDGETYEARID = ?, CATEGID = ?, SUBCATEGID = ?, PERIOD = ?, AMOUNT = ? WHERE BUDGETENTRYID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->BUDGETYEARID);
            stmt.Bind(2, entity->CATEGID);
            stmt.Bind(3, entity->SUBCATEGID);
            stmt.Bind(4, entity->PERIOD);
            stmt.Bind(5, entity->AMOUNT);
            if (entity->id() > 0)
                stmt.Bind(6, entity->BUDGETENTRYID);

            stmt.ExecuteUpdate();
            stmt.Finalize();
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("BUDGETTABLE_V1: Exception %s", e.GetMessage().c_str());
            return false;
        }

        if (entity->id() < 0) entity->id((db->GetLastRowId()).ToLong());
        return true;
    }

    bool remove(int id, wxSQLite3Database* db)
    {
        try
        {
            wxString sql = "DELETE FROM BUDGETTABLE_V1 WHERE BUDGETENTRYID = ?";
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
            wxLogError("BUDGETTABLE_V1: Exception %s", e.GetMessage().c_str());
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
        for(Cache::iterator it = cache_.begin(); it != cache_.end(); ++ it)
        {
            Self::Data* entity = *it;
            if (entity->id() == id) 
                return entity;
        }

        Self::Data* entity = 0;
        wxString where = wxString::Format(" WHERE BUDGETENTRYID = ?");
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
            wxLogError("BUDGETTABLE_V1: Exception %s", e.GetMessage().c_str());
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
            wxLogError("BUDGETTABLE_V1: Exception %s", e.GetMessage().c_str());
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
            wxLogError("BUDGETTABLE_V1: Exception %s", e.GetMessage().c_str());
        }

        return result;
    }

    Data_Set all(wxSQLite3Database* db, COLUMN col = COLUMN(0), bool asc = true)
    {
        Data_Set result;
        PRIMARY primay;
        try
        {
            wxSQLite3ResultSet q = db->ExecuteQuery(this->query() + " ORDER BY " + column_to_name(col) + (asc ? " ASC " : " DESC ")
                + "," + primay.name());

            while(q.NextRow())
            {
                Self::Data entity(q, this);
                result.push_back(entity);
            }

            q.Finalize();
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("BUDGETTABLE_V1: Exception %s", e.GetMessage().c_str());
        }

        return result;
    }

};
#endif //
