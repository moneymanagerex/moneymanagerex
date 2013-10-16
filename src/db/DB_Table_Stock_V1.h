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
 *          AUTO GENERATED at 2013-10-16 21:00:32.104748.
 *          DO NOT EDIT!
 */
//=============================================================================

#ifndef DB_TABLE_STOCK_V1_H
#define DB_TABLE_STOCK_V1_H

#include "DB_Table.h"

struct DB_Table_STOCK_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_STOCK_V1 Self;
    typedef std::vector<Self::Data> Data_Set;
    typedef std::vector<Self::Data*> Cache;
    Cache cache_;
    ~DB_Table_STOCK_V1() 
    {
        std::for_each(cache_.begin(), cache_.end(), std::mem_fun(&Data::destroy));
    }

    bool ensure(wxSQLite3Database* db) const
    {
        if (exists(db)) return true;

        try
        {
            db->ExecuteUpdate("CREATE TABLE STOCK_V1(STOCKID integer primary key, HELDAT integer , PURCHASEDATE TEXT NOT NULL, STOCKNAME TEXT COLLATE NOCASE NOT NULL UNIQUE, SYMBOL TEXT, NUMSHARES numeric, PURCHASEPRICE numeric NOT NULL, NOTES TEXT, CURRENTPRICE numeric NOT NULL, VALUE numeric, COMMISSION numeric)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("STOCK_V1: Exception %s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }

    struct STOCKID : public DB_Column<int>
    { 
        static wxString name() { return "STOCKID"; } 
        STOCKID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct HELDAT : public DB_Column<int>
    { 
        static wxString name() { return "HELDAT"; } 
        HELDAT(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct PURCHASEDATE : public DB_Column<wxString>
    { 
        static wxString name() { return "PURCHASEDATE"; } 
        PURCHASEDATE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct STOCKNAME : public DB_Column<wxString>
    { 
        static wxString name() { return "STOCKNAME"; } 
        STOCKNAME(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct SYMBOL : public DB_Column<wxString>
    { 
        static wxString name() { return "SYMBOL"; } 
        SYMBOL(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct NUMSHARES : public DB_Column<double>
    { 
        static wxString name() { return "NUMSHARES"; } 
        NUMSHARES(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    struct PURCHASEPRICE : public DB_Column<double>
    { 
        static wxString name() { return "PURCHASEPRICE"; } 
        PURCHASEPRICE(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    struct NOTES : public DB_Column<wxString>
    { 
        static wxString name() { return "NOTES"; } 
        NOTES(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct CURRENTPRICE : public DB_Column<double>
    { 
        static wxString name() { return "CURRENTPRICE"; } 
        CURRENTPRICE(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    struct VALUE : public DB_Column<double>
    { 
        static wxString name() { return "VALUE"; } 
        VALUE(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    struct COMMISSION : public DB_Column<double>
    { 
        static wxString name() { return "COMMISSION"; } 
        COMMISSION(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    typedef STOCKID PRIMARY;
    enum COLUMN
    {
        COL_STOCKID = 0
        , COL_HELDAT = 1
        , COL_PURCHASEDATE = 2
        , COL_STOCKNAME = 3
        , COL_SYMBOL = 4
        , COL_NUMSHARES = 5
        , COL_PURCHASEPRICE = 6
        , COL_NOTES = 7
        , COL_CURRENTPRICE = 8
        , COL_VALUE = 9
        , COL_COMMISSION = 10
    };

    std::vector<COLUMN> all_columns() const
    {
        std::vector<COLUMN> result;
        result.push_back(COL_STOCKID);
        result.push_back(COL_HELDAT);
        result.push_back(COL_PURCHASEDATE);
        result.push_back(COL_STOCKNAME);
        result.push_back(COL_SYMBOL);
        result.push_back(COL_NUMSHARES);
        result.push_back(COL_PURCHASEPRICE);
        result.push_back(COL_NOTES);
        result.push_back(COL_CURRENTPRICE);
        result.push_back(COL_VALUE);
        result.push_back(COL_COMMISSION);
        return result;
    }

    wxString column_to_name(COLUMN col) const
    {
        switch(col)
        {
            case COL_STOCKID: return "STOCKID";
            case COL_HELDAT: return "HELDAT";
            case COL_PURCHASEDATE: return "PURCHASEDATE";
            case COL_STOCKNAME: return "STOCKNAME";
            case COL_SYMBOL: return "SYMBOL";
            case COL_NUMSHARES: return "NUMSHARES";
            case COL_PURCHASEPRICE: return "PURCHASEPRICE";
            case COL_NOTES: return "NOTES";
            case COL_CURRENTPRICE: return "CURRENTPRICE";
            case COL_VALUE: return "VALUE";
            case COL_COMMISSION: return "COMMISSION";
            default: break;
        }
        
        return "UNKNOWN";
    }

    COLUMN name_to_column(const wxString& name) const
    {
        if ("STOCKID" == name) return COL_STOCKID;
        else if ("HELDAT" == name) return COL_HELDAT;
        else if ("PURCHASEDATE" == name) return COL_PURCHASEDATE;
        else if ("STOCKNAME" == name) return COL_STOCKNAME;
        else if ("SYMBOL" == name) return COL_SYMBOL;
        else if ("NUMSHARES" == name) return COL_NUMSHARES;
        else if ("PURCHASEPRICE" == name) return COL_PURCHASEPRICE;
        else if ("NOTES" == name) return COL_NOTES;
        else if ("CURRENTPRICE" == name) return COL_CURRENTPRICE;
        else if ("VALUE" == name) return COL_VALUE;
        else if ("COMMISSION" == name) return COL_COMMISSION;

        return COLUMN(-1);
    }
    
    struct Data
    {
        friend struct DB_Table_STOCK_V1;
        Self* view_;
    
        int STOCKID;//  primay key
        int HELDAT;
        wxString PURCHASEDATE;
        wxString STOCKNAME;
        wxString SYMBOL;
        double NUMSHARES;
        double PURCHASEPRICE;
        wxString NOTES;
        double CURRENTPRICE;
        double VALUE;
        double COMMISSION;
        int id() const { return STOCKID; }
        void id(int id) { STOCKID = id; }
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
        
            STOCKID = -1;
            HELDAT = -1;
            NUMSHARES = 0.0;
            PURCHASEPRICE = 0.0;
            CURRENTPRICE = 0.0;
            VALUE = 0.0;
            COMMISSION = 0.0;
        }

        Data(wxSQLite3ResultSet& q, Self* view = 0)
        {
            view_ = view;
        
            STOCKID = q.GetInt("STOCKID");
            HELDAT = q.GetInt("HELDAT");
            PURCHASEDATE = q.GetString("PURCHASEDATE");
            STOCKNAME = q.GetString("STOCKNAME");
            SYMBOL = q.GetString("SYMBOL");
            NUMSHARES = q.GetDouble("NUMSHARES");
            PURCHASEPRICE = q.GetDouble("PURCHASEPRICE");
            NOTES = q.GetString("NOTES");
            CURRENTPRICE = q.GetDouble("CURRENTPRICE");
            VALUE = q.GetDouble("VALUE");
            COMMISSION = q.GetDouble("COMMISSION");
        }

        wxString to_string(COLUMN col) const
        {
            wxString ret = wxEmptyString;
            switch(col)
            {
                case COL_STOCKID: ret << STOCKID; break;
                case COL_HELDAT: ret << HELDAT; break;
                case COL_PURCHASEDATE: ret << PURCHASEDATE; break;
                case COL_STOCKNAME: ret << STOCKNAME; break;
                case COL_SYMBOL: ret << SYMBOL; break;
                case COL_NUMSHARES: ret << NUMSHARES; break;
                case COL_PURCHASEPRICE: ret << PURCHASEPRICE; break;
                case COL_NOTES: ret << NOTES; break;
                case COL_CURRENTPRICE: ret << CURRENTPRICE; break;
                case COL_VALUE: ret << VALUE; break;
                case COL_COMMISSION: ret << COMMISSION; break;
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
            ret << STOCKID;
            ret << delimiter << HELDAT;
            ret << delimiter << PURCHASEDATE;
            ret << delimiter << STOCKNAME;
            ret << delimiter << SYMBOL;
            ret << delimiter << NUMSHARES;
            ret << delimiter << PURCHASEPRICE;
            ret << delimiter << NOTES;
            ret << delimiter << CURRENTPRICE;
            ret << delimiter << VALUE;
            ret << delimiter << COMMISSION;
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
            o["STOCKID"] = json::Number(this->STOCKID);
            o["HELDAT"] = json::Number(this->HELDAT);
            o["PURCHASEDATE"] = json::String(this->PURCHASEDATE.ToStdString());
            o["STOCKNAME"] = json::String(this->STOCKNAME.ToStdString());
            o["SYMBOL"] = json::String(this->SYMBOL.ToStdString());
            o["NUMSHARES"] = json::Number(this->NUMSHARES);
            o["PURCHASEPRICE"] = json::Number(this->PURCHASEPRICE);
            o["NOTES"] = json::String(this->NOTES.ToStdString());
            o["CURRENTPRICE"] = json::Number(this->CURRENTPRICE);
            o["VALUE"] = json::Number(this->VALUE);
            o["COMMISSION"] = json::Number(this->COMMISSION);
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
        NUM_COLUMNS = 11
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    wxString name() const { return "STOCK_V1"; }

    DB_Table_STOCK_V1() 
    {
        query_ = "SELECT STOCKID, HELDAT, PURCHASEDATE, STOCKNAME, SYMBOL, NUMSHARES, PURCHASEPRICE, NOTES, CURRENTPRICE, VALUE, COMMISSION FROM STOCK_V1 ";
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
            sql = "INSERT INTO STOCK_V1(HELDAT, PURCHASEDATE, STOCKNAME, SYMBOL, NUMSHARES, PURCHASEPRICE, NOTES, CURRENTPRICE, VALUE, COMMISSION) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE STOCK_V1 SET HELDAT = ?, PURCHASEDATE = ?, STOCKNAME = ?, SYMBOL = ?, NUMSHARES = ?, PURCHASEPRICE = ?, NOTES = ?, CURRENTPRICE = ?, VALUE = ?, COMMISSION = ? WHERE STOCKID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->HELDAT);
            stmt.Bind(2, entity->PURCHASEDATE);
            stmt.Bind(3, entity->STOCKNAME);
            stmt.Bind(4, entity->SYMBOL);
            stmt.Bind(5, entity->NUMSHARES);
            stmt.Bind(6, entity->PURCHASEPRICE);
            stmt.Bind(7, entity->NOTES);
            stmt.Bind(8, entity->CURRENTPRICE);
            stmt.Bind(9, entity->VALUE);
            stmt.Bind(10, entity->COMMISSION);
            if (entity->id() > 0)
                stmt.Bind(11, entity->STOCKID);

            wxLogDebug(stmt.GetSQL());
            stmt.ExecuteUpdate();
            stmt.Finalize();
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("STOCK_V1: Exception %s, %s", e.GetMessage().c_str(), entity->to_json());
            return false;
        }

        if (entity->id() < 0) entity->id((db->GetLastRowId()).ToLong());
        return true;
    }

    bool remove(int id, wxSQLite3Database* db)
    {
        try
        {
            wxString sql = "DELETE FROM STOCK_V1 WHERE STOCKID = ?";
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
            wxLogError("STOCK_V1: Exception %s", e.GetMessage().c_str());
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
            wxSQLite3ResultSet q = db->ExecuteQuery(this->query() + " ORDER BY " + column_to_name(col) + (asc ? " ASC " : " DESC ")
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
