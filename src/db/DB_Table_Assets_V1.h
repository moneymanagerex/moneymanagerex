﻿// -*- C++ -*-
//=============================================================================
/**
 *      Copyright: (c) 2013 - 2025 Guan Lisheng (guanlisheng@gmail.com)
 *      Copyright: (c) 2017 - 2018 Stefano Giorgio (stef145g)
 *      Copyright: (c) 2022 Mark Whalley (mark@ipx.co.uk)
 *
 *      @file
 *
 *      @author [sqlite2cpp.py]
 *
 *      @brief
 *
 *      Revision History:
 *          AUTO GENERATED at 2025-04-21 15:32:55.694774.
 *          DO NOT EDIT!
 */
//=============================================================================
#pragma once

#include "DB_Table.h"

struct DB_Table_ASSETS_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_ASSETS_V1 Self;

    /** A container to hold list of Data records for the table*/
    struct Data_Set : public std::vector<Self::Data>
    {
        /**Return the data records as a json array string */
        wxString to_json() const
        {
            StringBuffer json_buffer;
            PrettyWriter<StringBuffer> json_writer(json_buffer);

            json_writer.StartArray();
            for (const auto & item: *this)
            {
                json_writer.StartObject();
                item.as_json(json_writer);
                json_writer.EndObject();
            }
            json_writer.EndArray();

            return json_buffer.GetString();
        }
    };

    /** A container to hold a list of Data record pointers for the table in memory*/
    typedef std::vector<Self::Data*> Cache;
    typedef std::map<int64, Self::Data*> Index_By_Id;
    Cache cache_;
    Index_By_Id index_by_id_;
    Data* fake_; // in case the entity not found

    /** Destructor: clears any data records stored in memory */
    ~DB_Table_ASSETS_V1() 
    {
        delete this->fake_;
        destroy_cache();
    }
     
    /** Removes all records stored in memory (cache) for the table*/ 
    void destroy_cache()
    {
        std::for_each(cache_.begin(), cache_.end(), std::mem_fn(&Data::destroy));
        cache_.clear();
        index_by_id_.clear(); // no memory release since it just stores pointer and the according objects are in cache
    }

    /** Creates the database table if the table does not exist*/
    bool ensure(wxSQLite3Database* db)
    {
        if (!exists(db))
        {
            try
            {
                db->ExecuteUpdate("CREATE TABLE ASSETS_V1(ASSETID integer primary key, STARTDATE TEXT NOT NULL, ASSETNAME TEXT COLLATE NOCASE NOT NULL, ASSETSTATUS TEXT /* Open, Closed */, CURRENCYID integer, VALUECHANGEMODE TEXT /* Percentage, Linear */, VALUE numeric, VALUECHANGE TEXT /* None, Appreciates, Depreciates */, NOTES TEXT, VALUECHANGERATE numeric, ASSETTYPE TEXT /* Property, Automobile, Household Object, Art, Jewellery, Cash, Other */)");
                this->ensure_data(db);
            }
            catch(const wxSQLite3Exception &e) 
            { 
                wxLogError("ASSETS_V1: Exception %s", e.GetMessage().utf8_str());
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
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_ASSETS_ASSETTYPE ON ASSETS_V1(ASSETTYPE)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("ASSETS_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }

        return true;
    }

    void ensure_data(wxSQLite3Database* db)
    {
        db->Begin();
        db->Commit();
    }
    
    struct ASSETID : public DB_Column<int64>
    { 
        static wxString name() { return "ASSETID"; } 
        explicit ASSETID(const int64 &v, OP op = EQUAL): DB_Column<int64>(v, op) {}
    };
    
    struct STARTDATE : public DB_Column<wxString>
    { 
        static wxString name() { return "STARTDATE"; } 
        explicit STARTDATE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct ASSETNAME : public DB_Column<wxString>
    { 
        static wxString name() { return "ASSETNAME"; } 
        explicit ASSETNAME(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct ASSETSTATUS : public DB_Column<wxString>
    { 
        static wxString name() { return "ASSETSTATUS"; } 
        explicit ASSETSTATUS(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct CURRENCYID : public DB_Column<int64>
    { 
        static wxString name() { return "CURRENCYID"; } 
        explicit CURRENCYID(const int64 &v, OP op = EQUAL): DB_Column<int64>(v, op) {}
    };
    
    struct VALUECHANGEMODE : public DB_Column<wxString>
    { 
        static wxString name() { return "VALUECHANGEMODE"; } 
        explicit VALUECHANGEMODE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct VALUE : public DB_Column<double>
    { 
        static wxString name() { return "VALUE"; } 
        explicit VALUE(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    
    struct VALUECHANGE : public DB_Column<wxString>
    { 
        static wxString name() { return "VALUECHANGE"; } 
        explicit VALUECHANGE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct NOTES : public DB_Column<wxString>
    { 
        static wxString name() { return "NOTES"; } 
        explicit NOTES(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct VALUECHANGERATE : public DB_Column<double>
    { 
        static wxString name() { return "VALUECHANGERATE"; } 
        explicit VALUECHANGERATE(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    
    struct ASSETTYPE : public DB_Column<wxString>
    { 
        static wxString name() { return "ASSETTYPE"; } 
        explicit ASSETTYPE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    typedef ASSETID PRIMARY;
    enum COLUMN
    {
        COL_ASSETID = 0
        , COL_STARTDATE = 1
        , COL_ASSETNAME = 2
        , COL_ASSETSTATUS = 3
        , COL_CURRENCYID = 4
        , COL_VALUECHANGEMODE = 5
        , COL_VALUE = 6
        , COL_VALUECHANGE = 7
        , COL_NOTES = 8
        , COL_VALUECHANGERATE = 9
        , COL_ASSETTYPE = 10
    };

    /** Returns the column name as a string*/
    static wxString column_to_name(const COLUMN col)
    {
        switch(col)
        {
            case COL_ASSETID: return "ASSETID";
            case COL_STARTDATE: return "STARTDATE";
            case COL_ASSETNAME: return "ASSETNAME";
            case COL_ASSETSTATUS: return "ASSETSTATUS";
            case COL_CURRENCYID: return "CURRENCYID";
            case COL_VALUECHANGEMODE: return "VALUECHANGEMODE";
            case COL_VALUE: return "VALUE";
            case COL_VALUECHANGE: return "VALUECHANGE";
            case COL_NOTES: return "NOTES";
            case COL_VALUECHANGERATE: return "VALUECHANGERATE";
            case COL_ASSETTYPE: return "ASSETTYPE";
            default: break;
        }
        
        return "UNKNOWN";
    }

    /** Returns the column number from the given column name*/
    static COLUMN name_to_column(const wxString& name)
    {
        if ("ASSETID" == name) return COL_ASSETID;
        else if ("STARTDATE" == name) return COL_STARTDATE;
        else if ("ASSETNAME" == name) return COL_ASSETNAME;
        else if ("ASSETSTATUS" == name) return COL_ASSETSTATUS;
        else if ("CURRENCYID" == name) return COL_CURRENCYID;
        else if ("VALUECHANGEMODE" == name) return COL_VALUECHANGEMODE;
        else if ("VALUE" == name) return COL_VALUE;
        else if ("VALUECHANGE" == name) return COL_VALUECHANGE;
        else if ("NOTES" == name) return COL_NOTES;
        else if ("VALUECHANGERATE" == name) return COL_VALUECHANGERATE;
        else if ("ASSETTYPE" == name) return COL_ASSETTYPE;

        return COLUMN(-1);
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_ASSETS_V1;
        /** This is a instance pointer to itself in memory. */
        Self* table_;
    
        int64 ASSETID;//  primary key
        wxString STARTDATE;
        wxString ASSETNAME;
        wxString ASSETSTATUS;
        int64 CURRENCYID;
        wxString VALUECHANGEMODE;
        double VALUE;
        wxString VALUECHANGE;
        wxString NOTES;
        double VALUECHANGERATE;
        wxString ASSETTYPE;

        int64 id() const
        {
            return ASSETID;
        }

        void id(const int64 id)
        {
            ASSETID = id;
        }

        bool operator < (const Data& r) const
        {
            return this->id() < r.id();
        }
        
        bool operator < (const Data* r) const
        {
            return this->id() < r->id();
        }

        bool equals(const Data* r) const
        {
            if(ASSETID != r->ASSETID) return false;
            if(!STARTDATE.IsSameAs(r->STARTDATE)) return false;
            if(!ASSETNAME.IsSameAs(r->ASSETNAME)) return false;
            if(!ASSETSTATUS.IsSameAs(r->ASSETSTATUS)) return false;
            if(CURRENCYID != r->CURRENCYID) return false;
            if(!VALUECHANGEMODE.IsSameAs(r->VALUECHANGEMODE)) return false;
            if(VALUE != r->VALUE) return false;
            if(!VALUECHANGE.IsSameAs(r->VALUECHANGE)) return false;
            if(!NOTES.IsSameAs(r->NOTES)) return false;
            if(VALUECHANGERATE != r->VALUECHANGERATE) return false;
            if(!ASSETTYPE.IsSameAs(r->ASSETTYPE)) return false;
            return true;
        }
        
        explicit Data(Self* table = nullptr ) 
        {
            table_ = table;
        
            ASSETID = -1;
            CURRENCYID = -1;
            VALUE = 0.0;
            VALUECHANGERATE = 0.0;
        }

        explicit Data(wxSQLite3ResultSet& q, Self* table = nullptr )
        {
            table_ = table;
        
            ASSETID = q.GetInt64(0); // ASSETID
            STARTDATE = q.GetString(1); // STARTDATE
            ASSETNAME = q.GetString(2); // ASSETNAME
            ASSETSTATUS = q.GetString(3); // ASSETSTATUS
            CURRENCYID = q.GetInt64(4); // CURRENCYID
            VALUECHANGEMODE = q.GetString(5); // VALUECHANGEMODE
            VALUE = q.GetDouble(6); // VALUE
            VALUECHANGE = q.GetString(7); // VALUECHANGE
            NOTES = q.GetString(8); // NOTES
            VALUECHANGERATE = q.GetDouble(9); // VALUECHANGERATE
            ASSETTYPE = q.GetString(10); // ASSETTYPE
        }

        Data(const Data& other) = default;

        Data& operator=(const Data& other)
        {
            if (this == &other) return *this;

            ASSETID = other.ASSETID;
            STARTDATE = other.STARTDATE;
            ASSETNAME = other.ASSETNAME;
            ASSETSTATUS = other.ASSETSTATUS;
            CURRENCYID = other.CURRENCYID;
            VALUECHANGEMODE = other.VALUECHANGEMODE;
            VALUE = other.VALUE;
            VALUECHANGE = other.VALUECHANGE;
            NOTES = other.NOTES;
            VALUECHANGERATE = other.VALUECHANGERATE;
            ASSETTYPE = other.ASSETTYPE;
            return *this;
        }

        template<typename C>
        bool match(const C &) const
        {
            return false;
        }

        bool match(const Self::ASSETID &in) const
        {
            return this->ASSETID == in.v_;
        }

        bool match(const Self::STARTDATE &in) const
        {
            return this->STARTDATE.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::ASSETNAME &in) const
        {
            return this->ASSETNAME.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::ASSETSTATUS &in) const
        {
            return this->ASSETSTATUS.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::CURRENCYID &in) const
        {
            return this->CURRENCYID == in.v_;
        }

        bool match(const Self::VALUECHANGEMODE &in) const
        {
            return this->VALUECHANGEMODE.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::VALUE &in) const
        {
            return this->VALUE == in.v_;
        }

        bool match(const Self::VALUECHANGE &in) const
        {
            return this->VALUECHANGE.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::NOTES &in) const
        {
            return this->NOTES.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::VALUECHANGERATE &in) const
        {
            return this->VALUECHANGERATE == in.v_;
        }

        bool match(const Self::ASSETTYPE &in) const
        {
            return this->ASSETTYPE.CmpNoCase(in.v_) == 0;
        }

        // Return the data record as a json string
        wxString to_json() const
        {
            StringBuffer json_buffer;
            PrettyWriter<StringBuffer> json_writer(json_buffer);

			json_writer.StartObject();			
			this->as_json(json_writer);
            json_writer.EndObject();

            return json_buffer.GetString();
        }

        // Add the field data as json key:value pairs
        void as_json(PrettyWriter<StringBuffer>& json_writer) const
        {
            json_writer.Key("ASSETID");
            json_writer.Int64(this->ASSETID.GetValue());
            json_writer.Key("STARTDATE");
            json_writer.String(this->STARTDATE.utf8_str());
            json_writer.Key("ASSETNAME");
            json_writer.String(this->ASSETNAME.utf8_str());
            json_writer.Key("ASSETSTATUS");
            json_writer.String(this->ASSETSTATUS.utf8_str());
            json_writer.Key("CURRENCYID");
            json_writer.Int64(this->CURRENCYID.GetValue());
            json_writer.Key("VALUECHANGEMODE");
            json_writer.String(this->VALUECHANGEMODE.utf8_str());
            json_writer.Key("VALUE");
            json_writer.Double(this->VALUE);
            json_writer.Key("VALUECHANGE");
            json_writer.String(this->VALUECHANGE.utf8_str());
            json_writer.Key("NOTES");
            json_writer.String(this->NOTES.utf8_str());
            json_writer.Key("VALUECHANGERATE");
            json_writer.Double(this->VALUECHANGERATE);
            json_writer.Key("ASSETTYPE");
            json_writer.String(this->ASSETTYPE.utf8_str());
        }

        row_t to_row_t() const
        {
            row_t row;
            row(L"ASSETID") = ASSETID.GetValue();
            row(L"STARTDATE") = STARTDATE;
            row(L"ASSETNAME") = ASSETNAME;
            row(L"ASSETSTATUS") = ASSETSTATUS;
            row(L"CURRENCYID") = CURRENCYID.GetValue();
            row(L"VALUECHANGEMODE") = VALUECHANGEMODE;
            row(L"VALUE") = VALUE;
            row(L"VALUECHANGE") = VALUECHANGE;
            row(L"NOTES") = NOTES;
            row(L"VALUECHANGERATE") = VALUECHANGERATE;
            row(L"ASSETTYPE") = ASSETTYPE;
            return row;
        }

        void to_template(html_template& t) const
        {
            t(L"ASSETID") = ASSETID.GetValue();
            t(L"STARTDATE") = STARTDATE;
            t(L"ASSETNAME") = ASSETNAME;
            t(L"ASSETSTATUS") = ASSETSTATUS;
            t(L"CURRENCYID") = CURRENCYID.GetValue();
            t(L"VALUECHANGEMODE") = VALUECHANGEMODE;
            t(L"VALUE") = VALUE;
            t(L"VALUECHANGE") = VALUECHANGE;
            t(L"NOTES") = NOTES;
            t(L"VALUECHANGERATE") = VALUECHANGERATE;
            t(L"ASSETTYPE") = ASSETTYPE;
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db, bool force_insert = false)
        {
            if (db && db->IsReadOnly()) return false;
            if (!table_ || !db) 
            {
                wxLogError("can not save ASSETS_V1");
                return false;
            }

            return table_->save(this, db, force_insert);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!table_ || !db) 
            {
                wxLogError("can not remove ASSETS_V1");
                return false;
            }
            
            return table_->remove(this, db);
        }

        void destroy()
        {
            delete this;
        }
    };

    enum
    {
        NUM_COLUMNS = 11
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    /** Name of the table*/    
    wxString name() const { return "ASSETS_V1"; }

    DB_Table_ASSETS_V1() : fake_(new Data())
    {
        query_ = "SELECT ASSETID, STARTDATE, ASSETNAME, ASSETSTATUS, CURRENCYID, VALUECHANGEMODE, VALUE, VALUECHANGE, NOTES, VALUECHANGERATE, ASSETTYPE FROM ASSETS_V1 ";
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
    bool save(Self::Data* entity, wxSQLite3Database* db, bool force_insert = false)
    {
        wxString sql = wxEmptyString;
        if (entity->id() <= 0 || force_insert) //  new & insert
        {
            sql = "INSERT INTO ASSETS_V1(STARTDATE, ASSETNAME, ASSETSTATUS, CURRENCYID, VALUECHANGEMODE, VALUE, VALUECHANGE, NOTES, VALUECHANGERATE, ASSETTYPE, ASSETID) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE ASSETS_V1 SET STARTDATE = ?, ASSETNAME = ?, ASSETSTATUS = ?, CURRENCYID = ?, VALUECHANGEMODE = ?, VALUE = ?, VALUECHANGE = ?, NOTES = ?, VALUECHANGERATE = ?, ASSETTYPE = ? WHERE ASSETID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->STARTDATE);
            stmt.Bind(2, entity->ASSETNAME);
            stmt.Bind(3, entity->ASSETSTATUS);
            stmt.Bind(4, entity->CURRENCYID);
            stmt.Bind(5, entity->VALUECHANGEMODE);
            stmt.Bind(6, entity->VALUE);
            stmt.Bind(7, entity->VALUECHANGE);
            stmt.Bind(8, entity->NOTES);
            stmt.Bind(9, entity->VALUECHANGERATE);
            stmt.Bind(10, entity->ASSETTYPE);
            stmt.Bind(11, entity->id() > 0 ? entity->ASSETID : newId());

            stmt.ExecuteUpdate();
            stmt.Finalize();

            if (entity->id() > 0) // existent
            {
                for(Cache::iterator it = cache_.begin(); it != cache_.end(); ++ it)
                {
                    Self::Data* e = *it;
                    if (e->id() == entity->id()) 
                        *e = *entity;  // in-place update
                }
            }
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("ASSETS_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
            return false;
        }

        if (entity->id() <= 0)
        {
            entity->id(db->GetLastRowId());
            index_by_id_.insert(std::make_pair(entity->id(), entity));
        }
        return true;
    }

    /** Remove the Data record from the database and the memory table (cache) */
    bool remove(const int64 id, wxSQLite3Database* db)
    {
        if (id <= 0) return false;
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
                {
                    index_by_id_.erase(entity->id());
                    delete entity;
                }
                else 
                {
                    c.push_back(entity);
                }
            }
            cache_.clear();
            cache_.swap(c);
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("ASSETS_V1: Exception %s", e.GetMessage().utf8_str());
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

    template<typename... Args>
    Self::Data* get_one(const Args& ... args)
    {
        for (auto& [_, item] : index_by_id_)
        {
            if (item->id() > 0 && match(item, args...)) 
            {
                ++ hit_;
                return item;
            }
        }

        ++ miss_;

        return 0;
    }
    
    /**
    * Search the memory table (Cache) for the data record.
    * If not found in memory, search the database and update the cache.
    */
    Self::Data* get(const int64 id, wxSQLite3Database* db)
    {
        if (id <= 0) 
        {
            ++ skip_;
            return nullptr;
        }

        if (auto it = index_by_id_.find(id); it != index_by_id_.end())
        {
            ++ hit_;
            return it->second;
        }
        
        ++ miss_;
        Self::Data* entity = nullptr;
        wxString where = wxString::Format(" WHERE %s = ?", PRIMARY::name().utf8_str());
        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(this->query() + where);
            stmt.Bind(1, id);

            wxSQLite3ResultSet q = stmt.ExecuteQuery();
            if(q.NextRow())
            {
                entity = new Self::Data(q, this);
                cache_.push_back(entity);
                index_by_id_.insert(std::make_pair(id, entity));
            }
            stmt.Finalize();
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("%s: Exception %s", this->name().utf8_str(), e.GetMessage().utf8_str());
        }
        
        if (!entity) 
        {
            entity = this->fake_;
            // wxLogError("%s: %d not found", this->name().utf8_str(), id);
        }
 
        return entity;
    }
    /**
    * Search the database for the data record, bypassing the cache.
    */
    Self::Data* get_record(const int64 id, wxSQLite3Database* db)
    {
        if (id <= 0) 
        {
            ++ skip_;
            return nullptr;
        }

        Self::Data* entity = nullptr;
        wxString where = wxString::Format(" WHERE %s = ?", PRIMARY::name().utf8_str());
        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(this->query() + where);
            stmt.Bind(1, id);

            wxSQLite3ResultSet q = stmt.ExecuteQuery();
            if(q.NextRow())
            {
                entity = new Self::Data(q, this);
            }
            stmt.Finalize();
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("%s: Exception %s", this->name().utf8_str(), e.GetMessage().utf8_str());
        }
        
        if (!entity) 
        {
            entity = this->fake_;
            // wxLogError("%s: %d not found", this->name().utf8_str(), id);
        }
 
        return entity;
    }

    /**
    * Return a list of Data records (Data_Set) derived directly from the database.
    * The Data_Set is sorted based on the column number.
    */
    const Data_Set all(wxSQLite3Database* db, const COLUMN col = COLUMN(0), const bool asc = true)
    {
        Data_Set result;
        try
        {
            wxSQLite3ResultSet q = db->ExecuteQuery(col == COLUMN(0) ? this->query() : this->query() + " ORDER BY " + column_to_name(col) + " COLLATE NOCASE " + (asc ? " ASC " : " DESC "));

            while(q.NextRow())
            {
                Self::Data entity(q, this);
                result.push_back(std::move(entity));
            }

            q.Finalize();
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("%s: Exception %s", this->name().utf8_str(), e.GetMessage().utf8_str());
        }

        return result;
    }
};

