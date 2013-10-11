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
 *          AUTO GENERATED at 2013-10-11 15:10:52.121003.
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
    typedef std::vector<Self::Data> Data_Set;
    typedef std::vector<Self::Data*> Cache;
    Cache cache_;
    ~DB_Table_ACCOUNTLIST_V1() 
    {
        std::for_each(cache_.begin(), cache_.end(), std::mem_fun(&Data::destroy));
    }

    bool ensure(wxSQLite3Database* db) const
    {
        if (exists(db)) return true;

        try
        {
            db->ExecuteUpdate("CREATE TABLE ACCOUNTLIST_V1(ACCOUNTID integer primary key, ACCOUNTNAME TEXT COLLATE NOCASE NOT NULL UNIQUE, ACCOUNTTYPE TEXT NOT NULL , ACCOUNTNUM TEXT, STATUS TEXT NOT NULL, NOTES TEXT , HELDAT TEXT , WEBSITE TEXT , CONTACTINFO TEXT, ACCESSINFO TEXT , INITIALBAL numeric , FAVORITEACCT TEXT NOT NULL, CURRENCYID integer NOT NULL)");
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
        ACCOUNTID(const int &v): DB_Column<int>(v) {}
    };
    struct ACCOUNTNAME : public DB_Column<wxString>
    { 
        static wxString name() { return "ACCOUNTNAME"; } 
        ACCOUNTNAME(const wxString &v): DB_Column<wxString>(v) {}
    };
    struct ACCOUNTTYPE : public DB_Column<wxString>
    { 
        static wxString name() { return "ACCOUNTTYPE"; } 
        ACCOUNTTYPE(const wxString &v): DB_Column<wxString>(v) {}
    };
    struct ACCOUNTNUM : public DB_Column<wxString>
    { 
        static wxString name() { return "ACCOUNTNUM"; } 
        ACCOUNTNUM(const wxString &v): DB_Column<wxString>(v) {}
    };
    struct STATUS : public DB_Column<wxString>
    { 
        static wxString name() { return "STATUS"; } 
        STATUS(const wxString &v): DB_Column<wxString>(v) {}
    };
    struct NOTES : public DB_Column<wxString>
    { 
        static wxString name() { return "NOTES"; } 
        NOTES(const wxString &v): DB_Column<wxString>(v) {}
    };
    struct HELDAT : public DB_Column<wxString>
    { 
        static wxString name() { return "HELDAT"; } 
        HELDAT(const wxString &v): DB_Column<wxString>(v) {}
    };
    struct WEBSITE : public DB_Column<wxString>
    { 
        static wxString name() { return "WEBSITE"; } 
        WEBSITE(const wxString &v): DB_Column<wxString>(v) {}
    };
    struct CONTACTINFO : public DB_Column<wxString>
    { 
        static wxString name() { return "CONTACTINFO"; } 
        CONTACTINFO(const wxString &v): DB_Column<wxString>(v) {}
    };
    struct ACCESSINFO : public DB_Column<wxString>
    { 
        static wxString name() { return "ACCESSINFO"; } 
        ACCESSINFO(const wxString &v): DB_Column<wxString>(v) {}
    };
    struct INITIALBAL : public DB_Column<double>
    { 
        static wxString name() { return "INITIALBAL"; } 
        INITIALBAL(const double &v): DB_Column<double>(v) {}
    };
    struct FAVORITEACCT : public DB_Column<wxString>
    { 
        static wxString name() { return "FAVORITEACCT"; } 
        FAVORITEACCT(const wxString &v): DB_Column<wxString>(v) {}
    };
    struct CURRENCYID : public DB_Column<int>
    { 
        static wxString name() { return "CURRENCYID"; } 
        CURRENCYID(const int &v): DB_Column<int>(v) {}
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

    std::vector<COLUMN> all_columns() const
    {
        std::vector<COLUMN> result;
        result.push_back(COL_ACCOUNTID);
        result.push_back(COL_ACCOUNTNAME);
        result.push_back(COL_ACCOUNTTYPE);
        result.push_back(COL_ACCOUNTNUM);
        result.push_back(COL_STATUS);
        result.push_back(COL_NOTES);
        result.push_back(COL_HELDAT);
        result.push_back(COL_WEBSITE);
        result.push_back(COL_CONTACTINFO);
        result.push_back(COL_ACCESSINFO);
        result.push_back(COL_INITIALBAL);
        result.push_back(COL_FAVORITEACCT);
        result.push_back(COL_CURRENCYID);
        return result;
    }

    wxString column_to_name(COLUMN col) const
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

    COLUMN name_to_column(const wxString& name) const
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
    
    struct Data
    {
        friend struct DB_Table_ACCOUNTLIST_V1;
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

    private:
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
        
            ACCOUNTID = q.GetInt("ACCOUNTID");
            ACCOUNTNAME = q.GetString("ACCOUNTNAME");
            ACCOUNTTYPE = q.GetString("ACCOUNTTYPE");
            ACCOUNTNUM = q.GetString("ACCOUNTNUM");
            STATUS = q.GetString("STATUS");
            NOTES = q.GetString("NOTES");
            HELDAT = q.GetString("HELDAT");
            WEBSITE = q.GetString("WEBSITE");
            CONTACTINFO = q.GetString("CONTACTINFO");
            ACCESSINFO = q.GetString("ACCESSINFO");
            INITIALBAL = q.GetDouble("INITIALBAL");
            FAVORITEACCT = q.GetString("FAVORITEACCT");
            CURRENCYID = q.GetInt("CURRENCYID");
        }
    public:

        wxString to_string(COLUMN col) const
        {
            wxString ret = wxEmptyString;
            switch(col)
            {
                case COL_ACCOUNTID: ret << ACCOUNTID; break;
                case COL_ACCOUNTNAME: ret << ACCOUNTNAME; break;
                case COL_ACCOUNTTYPE: ret << ACCOUNTTYPE; break;
                case COL_ACCOUNTNUM: ret << ACCOUNTNUM; break;
                case COL_STATUS: ret << STATUS; break;
                case COL_NOTES: ret << NOTES; break;
                case COL_HELDAT: ret << HELDAT; break;
                case COL_WEBSITE: ret << WEBSITE; break;
                case COL_CONTACTINFO: ret << CONTACTINFO; break;
                case COL_ACCESSINFO: ret << ACCESSINFO; break;
                case COL_INITIALBAL: ret << INITIALBAL; break;
                case COL_FAVORITEACCT: ret << FAVORITEACCT; break;
                case COL_CURRENCYID: ret << CURRENCYID; break;
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
            ret << ACCOUNTID;
            ret << delimiter << ACCOUNTNAME;
            ret << delimiter << ACCOUNTTYPE;
            ret << delimiter << ACCOUNTNUM;
            ret << delimiter << STATUS;
            ret << delimiter << NOTES;
            ret << delimiter << HELDAT;
            ret << delimiter << WEBSITE;
            ret << delimiter << CONTACTINFO;
            ret << delimiter << ACCESSINFO;
            ret << delimiter << INITIALBAL;
            ret << delimiter << FAVORITEACCT;
            ret << delimiter << CURRENCYID;
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
        NUM_COLUMNS = 13
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    wxString name() const { return "ACCOUNTLIST_V1"; }

    DB_Table_ACCOUNTLIST_V1() 
    {
        query_ = "SELECT ACCOUNTID, ACCOUNTNAME, ACCOUNTTYPE, ACCOUNTNUM, STATUS, NOTES, HELDAT, WEBSITE, CONTACTINFO, ACCESSINFO, INITIALBAL, FAVORITEACCT, CURRENCYID FROM ACCOUNTLIST_V1 ";
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

            stmt.ExecuteUpdate();
            stmt.Finalize();
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("ACCOUNTLIST_V1: Exception %s, %s", e.GetMessage().c_str(), entity->to_json());
            return false;
        }

        if (entity->id() < 0) entity->id((db->GetLastRowId()).ToLong());
        return true;
    }

    bool remove(int id, wxSQLite3Database* db)
    {
        try
        {
            wxString sql = "DELETE FROM ACCOUNTLIST_V1 WHERE ACCOUNTID = ?";
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
            wxLogError("ACCOUNTLIST_V1: Exception %s", e.GetMessage().c_str());
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
        wxString where = wxString::Format(" WHERE ACCOUNTID = ?");
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
            wxLogError("ACCOUNTLIST_V1: Exception %s", e.GetMessage().c_str());
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
            wxLogError("ACCOUNTLIST_V1: Exception %s", e.GetMessage().c_str());
        }

        return result;
    }

};
#endif //
