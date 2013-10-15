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
 *          AUTO GENERATED at 2013-10-15 21:02:48.018598.
 *          DO NOT EDIT!
 */
//=============================================================================

#ifndef DB_TABLE_BILLSDEPOSITS_V1_H
#define DB_TABLE_BILLSDEPOSITS_V1_H

#include "DB_Table.h"

struct DB_Table_BILLSDEPOSITS_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_BILLSDEPOSITS_V1 Self;
    typedef std::vector<Self::Data> Data_Set;
    typedef std::vector<Self::Data*> Cache;
    Cache cache_;
    ~DB_Table_BILLSDEPOSITS_V1() 
    {
        std::for_each(cache_.begin(), cache_.end(), std::mem_fun(&Data::destroy));
    }

    bool ensure(wxSQLite3Database* db) const
    {
        if (exists(db)) return true;

        try
        {
            db->ExecuteUpdate("CREATE TABLE BILLSDEPOSITS_V1(BDID integer primary key, ACCOUNTID integer NOT NULL, TOACCOUNTID integer, PAYEEID integer NOT NULL, TRANSCODE TEXT NOT NULL, TRANSAMOUNT numeric NOT NULL, STATUS TEXT, TRANSACTIONNUMBER TEXT, NOTES TEXT, CATEGID integer, SUBCATEGID integer, TRANSDATE TEXT, FOLLOWUPID integer, TOTRANSAMOUNT numeric, REPEATS integer, NEXTOCCURRENCEDATE TEXT, NUMOCCURRENCES integer )");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("BILLSDEPOSITS_V1: Exception %s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }

    struct BDID : public DB_Column<int>
    { 
        static wxString name() { return "BDID"; } 
        BDID(const int &v): DB_Column<int>(v) {}
        BDID(const int &v, OP op): DB_Column<int>(v, op) {}
    };
    struct ACCOUNTID : public DB_Column<int>
    { 
        static wxString name() { return "ACCOUNTID"; } 
        ACCOUNTID(const int &v): DB_Column<int>(v) {}
        ACCOUNTID(const int &v, OP op): DB_Column<int>(v, op) {}
    };
    struct TOACCOUNTID : public DB_Column<int>
    { 
        static wxString name() { return "TOACCOUNTID"; } 
        TOACCOUNTID(const int &v): DB_Column<int>(v) {}
        TOACCOUNTID(const int &v, OP op): DB_Column<int>(v, op) {}
    };
    struct PAYEEID : public DB_Column<int>
    { 
        static wxString name() { return "PAYEEID"; } 
        PAYEEID(const int &v): DB_Column<int>(v) {}
        PAYEEID(const int &v, OP op): DB_Column<int>(v, op) {}
    };
    struct TRANSCODE : public DB_Column<wxString>
    { 
        static wxString name() { return "TRANSCODE"; } 
        TRANSCODE(const wxString &v): DB_Column<wxString>(v) {}
        TRANSCODE(const wxString &v, OP op): DB_Column<wxString>(v, op) {}
    };
    struct TRANSAMOUNT : public DB_Column<double>
    { 
        static wxString name() { return "TRANSAMOUNT"; } 
        TRANSAMOUNT(const double &v): DB_Column<double>(v) {}
        TRANSAMOUNT(const double &v, OP op): DB_Column<double>(v, op) {}
    };
    struct STATUS : public DB_Column<wxString>
    { 
        static wxString name() { return "STATUS"; } 
        STATUS(const wxString &v): DB_Column<wxString>(v) {}
        STATUS(const wxString &v, OP op): DB_Column<wxString>(v, op) {}
    };
    struct TRANSACTIONNUMBER : public DB_Column<wxString>
    { 
        static wxString name() { return "TRANSACTIONNUMBER"; } 
        TRANSACTIONNUMBER(const wxString &v): DB_Column<wxString>(v) {}
        TRANSACTIONNUMBER(const wxString &v, OP op): DB_Column<wxString>(v, op) {}
    };
    struct NOTES : public DB_Column<wxString>
    { 
        static wxString name() { return "NOTES"; } 
        NOTES(const wxString &v): DB_Column<wxString>(v) {}
        NOTES(const wxString &v, OP op): DB_Column<wxString>(v, op) {}
    };
    struct CATEGID : public DB_Column<int>
    { 
        static wxString name() { return "CATEGID"; } 
        CATEGID(const int &v): DB_Column<int>(v) {}
        CATEGID(const int &v, OP op): DB_Column<int>(v, op) {}
    };
    struct SUBCATEGID : public DB_Column<int>
    { 
        static wxString name() { return "SUBCATEGID"; } 
        SUBCATEGID(const int &v): DB_Column<int>(v) {}
        SUBCATEGID(const int &v, OP op): DB_Column<int>(v, op) {}
    };
    struct TRANSDATE : public DB_Column<wxString>
    { 
        static wxString name() { return "TRANSDATE"; } 
        TRANSDATE(const wxString &v): DB_Column<wxString>(v) {}
        TRANSDATE(const wxString &v, OP op): DB_Column<wxString>(v, op) {}
    };
    struct FOLLOWUPID : public DB_Column<int>
    { 
        static wxString name() { return "FOLLOWUPID"; } 
        FOLLOWUPID(const int &v): DB_Column<int>(v) {}
        FOLLOWUPID(const int &v, OP op): DB_Column<int>(v, op) {}
    };
    struct TOTRANSAMOUNT : public DB_Column<double>
    { 
        static wxString name() { return "TOTRANSAMOUNT"; } 
        TOTRANSAMOUNT(const double &v): DB_Column<double>(v) {}
        TOTRANSAMOUNT(const double &v, OP op): DB_Column<double>(v, op) {}
    };
    struct REPEATS : public DB_Column<int>
    { 
        static wxString name() { return "REPEATS"; } 
        REPEATS(const int &v): DB_Column<int>(v) {}
        REPEATS(const int &v, OP op): DB_Column<int>(v, op) {}
    };
    struct NEXTOCCURRENCEDATE : public DB_Column<wxString>
    { 
        static wxString name() { return "NEXTOCCURRENCEDATE"; } 
        NEXTOCCURRENCEDATE(const wxString &v): DB_Column<wxString>(v) {}
        NEXTOCCURRENCEDATE(const wxString &v, OP op): DB_Column<wxString>(v, op) {}
    };
    struct NUMOCCURRENCES : public DB_Column<int>
    { 
        static wxString name() { return "NUMOCCURRENCES"; } 
        NUMOCCURRENCES(const int &v): DB_Column<int>(v) {}
        NUMOCCURRENCES(const int &v, OP op): DB_Column<int>(v, op) {}
    };
    typedef BDID PRIMARY;
    enum COLUMN
    {
        COL_BDID = 0
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
        , COL_REPEATS = 14
        , COL_NEXTOCCURRENCEDATE = 15
        , COL_NUMOCCURRENCES = 16
    };

    std::vector<COLUMN> all_columns() const
    {
        std::vector<COLUMN> result;
        result.push_back(COL_BDID);
        result.push_back(COL_ACCOUNTID);
        result.push_back(COL_TOACCOUNTID);
        result.push_back(COL_PAYEEID);
        result.push_back(COL_TRANSCODE);
        result.push_back(COL_TRANSAMOUNT);
        result.push_back(COL_STATUS);
        result.push_back(COL_TRANSACTIONNUMBER);
        result.push_back(COL_NOTES);
        result.push_back(COL_CATEGID);
        result.push_back(COL_SUBCATEGID);
        result.push_back(COL_TRANSDATE);
        result.push_back(COL_FOLLOWUPID);
        result.push_back(COL_TOTRANSAMOUNT);
        result.push_back(COL_REPEATS);
        result.push_back(COL_NEXTOCCURRENCEDATE);
        result.push_back(COL_NUMOCCURRENCES);
        return result;
    }

    wxString column_to_name(COLUMN col) const
    {
        switch(col)
        {
            case COL_BDID: return "BDID";
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
            case COL_REPEATS: return "REPEATS";
            case COL_NEXTOCCURRENCEDATE: return "NEXTOCCURRENCEDATE";
            case COL_NUMOCCURRENCES: return "NUMOCCURRENCES";
            default: break;
        }
        
        return "UNKNOWN";
    }

    COLUMN name_to_column(const wxString& name) const
    {
        if ("BDID" == name) return COL_BDID;
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
        else if ("REPEATS" == name) return COL_REPEATS;
        else if ("NEXTOCCURRENCEDATE" == name) return COL_NEXTOCCURRENCEDATE;
        else if ("NUMOCCURRENCES" == name) return COL_NUMOCCURRENCES;

        return COLUMN(-1);
    }
    
    struct Data
    {
        friend struct DB_Table_BILLSDEPOSITS_V1;
        Self* view_;
    
        int BDID;//  primay key
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
        int REPEATS;
        wxString NEXTOCCURRENCEDATE;
        int NUMOCCURRENCES;
        int id() const { return BDID; }
        void id(int id) { BDID = id; }
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
        
            BDID = -1;
            ACCOUNTID = -1;
            TOACCOUNTID = -1;
            PAYEEID = -1;
            TRANSAMOUNT = 0.0;
            CATEGID = -1;
            SUBCATEGID = -1;
            FOLLOWUPID = -1;
            TOTRANSAMOUNT = 0.0;
            REPEATS = -1;
            NUMOCCURRENCES = -1;
        }

        Data(wxSQLite3ResultSet& q, Self* view = 0)
        {
            view_ = view;
        
            BDID = q.GetInt("BDID");
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
            REPEATS = q.GetInt("REPEATS");
            NEXTOCCURRENCEDATE = q.GetString("NEXTOCCURRENCEDATE");
            NUMOCCURRENCES = q.GetInt("NUMOCCURRENCES");
        }

        wxString to_string(COLUMN col) const
        {
            wxString ret = wxEmptyString;
            switch(col)
            {
                case COL_BDID: ret << BDID; break;
                case COL_ACCOUNTID: ret << ACCOUNTID; break;
                case COL_TOACCOUNTID: ret << TOACCOUNTID; break;
                case COL_PAYEEID: ret << PAYEEID; break;
                case COL_TRANSCODE: ret << TRANSCODE; break;
                case COL_TRANSAMOUNT: ret << TRANSAMOUNT; break;
                case COL_STATUS: ret << STATUS; break;
                case COL_TRANSACTIONNUMBER: ret << TRANSACTIONNUMBER; break;
                case COL_NOTES: ret << NOTES; break;
                case COL_CATEGID: ret << CATEGID; break;
                case COL_SUBCATEGID: ret << SUBCATEGID; break;
                case COL_TRANSDATE: ret << TRANSDATE; break;
                case COL_FOLLOWUPID: ret << FOLLOWUPID; break;
                case COL_TOTRANSAMOUNT: ret << TOTRANSAMOUNT; break;
                case COL_REPEATS: ret << REPEATS; break;
                case COL_NEXTOCCURRENCEDATE: ret << NEXTOCCURRENCEDATE; break;
                case COL_NUMOCCURRENCES: ret << NUMOCCURRENCES; break;
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
            ret << BDID;
            ret << delimiter << ACCOUNTID;
            ret << delimiter << TOACCOUNTID;
            ret << delimiter << PAYEEID;
            ret << delimiter << TRANSCODE;
            ret << delimiter << TRANSAMOUNT;
            ret << delimiter << STATUS;
            ret << delimiter << TRANSACTIONNUMBER;
            ret << delimiter << NOTES;
            ret << delimiter << CATEGID;
            ret << delimiter << SUBCATEGID;
            ret << delimiter << TRANSDATE;
            ret << delimiter << FOLLOWUPID;
            ret << delimiter << TOTRANSAMOUNT;
            ret << delimiter << REPEATS;
            ret << delimiter << NEXTOCCURRENCEDATE;
            ret << delimiter << NUMOCCURRENCES;
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
            o["BDID"] = json::Number(this->BDID);
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
            o["REPEATS"] = json::Number(this->REPEATS);
            o["NEXTOCCURRENCEDATE"] = json::String(this->NEXTOCCURRENCEDATE.ToStdString());
            o["NUMOCCURRENCES"] = json::Number(this->NUMOCCURRENCES);
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
        NUM_COLUMNS = 17
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    wxString name() const { return "BILLSDEPOSITS_V1"; }

    DB_Table_BILLSDEPOSITS_V1() 
    {
        query_ = "SELECT BDID, ACCOUNTID, TOACCOUNTID, PAYEEID, TRANSCODE, TRANSAMOUNT, STATUS, TRANSACTIONNUMBER, NOTES, CATEGID, SUBCATEGID, TRANSDATE, FOLLOWUPID, TOTRANSAMOUNT, REPEATS, NEXTOCCURRENCEDATE, NUMOCCURRENCES FROM BILLSDEPOSITS_V1 ";
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
            sql = "INSERT INTO BILLSDEPOSITS_V1(ACCOUNTID, TOACCOUNTID, PAYEEID, TRANSCODE, TRANSAMOUNT, STATUS, TRANSACTIONNUMBER, NOTES, CATEGID, SUBCATEGID, TRANSDATE, FOLLOWUPID, TOTRANSAMOUNT, REPEATS, NEXTOCCURRENCEDATE, NUMOCCURRENCES) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE BILLSDEPOSITS_V1 SET ACCOUNTID = ?, TOACCOUNTID = ?, PAYEEID = ?, TRANSCODE = ?, TRANSAMOUNT = ?, STATUS = ?, TRANSACTIONNUMBER = ?, NOTES = ?, CATEGID = ?, SUBCATEGID = ?, TRANSDATE = ?, FOLLOWUPID = ?, TOTRANSAMOUNT = ?, REPEATS = ?, NEXTOCCURRENCEDATE = ?, NUMOCCURRENCES = ? WHERE BDID = ?";
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
            stmt.Bind(14, entity->REPEATS);
            stmt.Bind(15, entity->NEXTOCCURRENCEDATE);
            stmt.Bind(16, entity->NUMOCCURRENCES);
            if (entity->id() > 0)
                stmt.Bind(17, entity->BDID);

            wxLogDebug(stmt.GetSQL());
            stmt.ExecuteUpdate();
            stmt.Finalize();
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("BILLSDEPOSITS_V1: Exception %s, %s", e.GetMessage().c_str(), entity->to_json());
            return false;
        }

        if (entity->id() < 0) entity->id((db->GetLastRowId()).ToLong());
        return true;
    }

    bool remove(int id, wxSQLite3Database* db)
    {
        try
        {
            wxString sql = "DELETE FROM BILLSDEPOSITS_V1 WHERE BDID = ?";
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
            wxLogError("BILLSDEPOSITS_V1: Exception %s", e.GetMessage().c_str());
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
