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
 *          AUTO GENERATED at 2025-04-19 13:00:37.989546.
 *          DO NOT EDIT!
 */
//=============================================================================
#pragma once

#include "DB_Table.h"

struct DB_Table_BUDGETTABLE_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_BUDGETTABLE_V1 Self;

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
    ~DB_Table_BUDGETTABLE_V1() 
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
                db->ExecuteUpdate("CREATE TABLE BUDGETTABLE_V1(BUDGETENTRYID integer primary key, BUDGETYEARID integer, CATEGID integer, PERIOD TEXT NOT NULL /* None, Weekly, Bi-Weekly, Monthly, Monthly, Bi-Monthly, Quarterly, Half-Yearly, Yearly, Daily*/, AMOUNT numeric NOT NULL, NOTES TEXT, ACTIVE integer)");
                this->ensure_data(db);
            }
            catch(const wxSQLite3Exception &e) 
            { 
                wxLogError("BUDGETTABLE_V1: Exception %s", e.GetMessage().utf8_str());
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
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_BUDGETTABLE_BUDGETYEARID ON BUDGETTABLE_V1(BUDGETYEARID)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("BUDGETTABLE_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }

        return true;
    }

    void ensure_data(wxSQLite3Database* db)
    {
        db->Begin();
        db->Commit();
    }
    
    struct BUDGETENTRYID : public DB_Column<int64>
    { 
        static wxString name() { return "BUDGETENTRYID"; } 
        explicit BUDGETENTRYID(const int64 &v, OP op = EQUAL): DB_Column<int64>(v, op) {}
    };
    
    struct BUDGETYEARID : public DB_Column<int64>
    { 
        static wxString name() { return "BUDGETYEARID"; } 
        explicit BUDGETYEARID(const int64 &v, OP op = EQUAL): DB_Column<int64>(v, op) {}
    };
    
    struct CATEGID : public DB_Column<int64>
    { 
        static wxString name() { return "CATEGID"; } 
        explicit CATEGID(const int64 &v, OP op = EQUAL): DB_Column<int64>(v, op) {}
    };
    
    struct PERIOD : public DB_Column<wxString>
    { 
        static wxString name() { return "PERIOD"; } 
        explicit PERIOD(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct AMOUNT : public DB_Column<double>
    { 
        static wxString name() { return "AMOUNT"; } 
        explicit AMOUNT(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    
    struct NOTES : public DB_Column<wxString>
    { 
        static wxString name() { return "NOTES"; } 
        explicit NOTES(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct ACTIVE : public DB_Column<int64>
    { 
        static wxString name() { return "ACTIVE"; } 
        explicit ACTIVE(const int64 &v, OP op = EQUAL): DB_Column<int64>(v, op) {}
    };
    
    typedef BUDGETENTRYID PRIMARY;
    enum COLUMN
    {
        COL_BUDGETENTRYID = 0
        , COL_BUDGETYEARID = 1
        , COL_CATEGID = 2
        , COL_PERIOD = 3
        , COL_AMOUNT = 4
        , COL_NOTES = 5
        , COL_ACTIVE = 6
    };

    /** Returns the column name as a string*/
    static wxString column_to_name(const COLUMN col)
    {
        switch(col)
        {
            case COL_BUDGETENTRYID: return "BUDGETENTRYID";
            case COL_BUDGETYEARID: return "BUDGETYEARID";
            case COL_CATEGID: return "CATEGID";
            case COL_PERIOD: return "PERIOD";
            case COL_AMOUNT: return "AMOUNT";
            case COL_NOTES: return "NOTES";
            case COL_ACTIVE: return "ACTIVE";
            default: break;
        }
        
        return "UNKNOWN";
    }

    /** Returns the column number from the given column name*/
    static COLUMN name_to_column(const wxString& name)
    {
        if ("BUDGETENTRYID" == name) return COL_BUDGETENTRYID;
        else if ("BUDGETYEARID" == name) return COL_BUDGETYEARID;
        else if ("CATEGID" == name) return COL_CATEGID;
        else if ("PERIOD" == name) return COL_PERIOD;
        else if ("AMOUNT" == name) return COL_AMOUNT;
        else if ("NOTES" == name) return COL_NOTES;
        else if ("ACTIVE" == name) return COL_ACTIVE;

        return COLUMN(-1);
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_BUDGETTABLE_V1;
        /** This is a instance pointer to itself in memory. */
        Self* table_;
    
        int64 BUDGETENTRYID;//  primary key
        int64 BUDGETYEARID;
        int64 CATEGID;
        wxString PERIOD;
        double AMOUNT;
        wxString NOTES;
        int64 ACTIVE;

        int64 id() const
        {
            return BUDGETENTRYID;
        }

        void id(const int64 id)
        {
            BUDGETENTRYID = id;
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
            if(BUDGETENTRYID != r->BUDGETENTRYID) return false;
            if(BUDGETYEARID != r->BUDGETYEARID) return false;
            if(CATEGID != r->CATEGID) return false;
            if(!PERIOD.IsSameAs(r->PERIOD)) return false;
            if(AMOUNT != r->AMOUNT) return false;
            if(!NOTES.IsSameAs(r->NOTES)) return false;
            if(ACTIVE != r->ACTIVE) return false;
            return true;
        }
        
        explicit Data(Self* table = nullptr ) 
        {
            table_ = table;
        
            BUDGETENTRYID = -1;
            BUDGETYEARID = -1;
            CATEGID = -1;
            AMOUNT = 0.0;
            ACTIVE = -1;
        }

        explicit Data(wxSQLite3ResultSet& q, Self* table = nullptr )
        {
            table_ = table;
        
            BUDGETENTRYID = q.GetInt64(0); // BUDGETENTRYID
            BUDGETYEARID = q.GetInt64(1); // BUDGETYEARID
            CATEGID = q.GetInt64(2); // CATEGID
            PERIOD = q.GetString(3); // PERIOD
            AMOUNT = q.GetDouble(4); // AMOUNT
            NOTES = q.GetString(5); // NOTES
            ACTIVE = q.GetInt64(6); // ACTIVE
        }

        Data(const Data& other) = default;

        Data& operator=(const Data& other)
        {
            if (this == &other) return *this;

            BUDGETENTRYID = other.BUDGETENTRYID;
            BUDGETYEARID = other.BUDGETYEARID;
            CATEGID = other.CATEGID;
            PERIOD = other.PERIOD;
            AMOUNT = other.AMOUNT;
            NOTES = other.NOTES;
            ACTIVE = other.ACTIVE;
            return *this;
        }

        template<typename C>
        bool match(const C &) const
        {
            return false;
        }

        bool match(const Self::BUDGETENTRYID &in) const
        {
            return this->BUDGETENTRYID == in.v_;
        }

        bool match(const Self::BUDGETYEARID &in) const
        {
            return this->BUDGETYEARID == in.v_;
        }

        bool match(const Self::CATEGID &in) const
        {
            return this->CATEGID == in.v_;
        }

        bool match(const Self::PERIOD &in) const
        {
            return this->PERIOD.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::AMOUNT &in) const
        {
            return this->AMOUNT == in.v_;
        }

        bool match(const Self::NOTES &in) const
        {
            return this->NOTES.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::ACTIVE &in) const
        {
            return this->ACTIVE == in.v_;
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
            json_writer.Key("BUDGETENTRYID");
            json_writer.Int64(this->BUDGETENTRYID.GetValue());
            json_writer.Key("BUDGETYEARID");
            json_writer.Int64(this->BUDGETYEARID.GetValue());
            json_writer.Key("CATEGID");
            json_writer.Int64(this->CATEGID.GetValue());
            json_writer.Key("PERIOD");
            json_writer.String(this->PERIOD.utf8_str());
            json_writer.Key("AMOUNT");
            json_writer.Double(this->AMOUNT);
            json_writer.Key("NOTES");
            json_writer.String(this->NOTES.utf8_str());
            json_writer.Key("ACTIVE");
            json_writer.Int64(this->ACTIVE.GetValue());
        }

        row_t to_row_t() const
        {
            row_t row;
            row(L"BUDGETENTRYID") = BUDGETENTRYID.GetValue();
            row(L"BUDGETYEARID") = BUDGETYEARID.GetValue();
            row(L"CATEGID") = CATEGID.GetValue();
            row(L"PERIOD") = PERIOD;
            row(L"AMOUNT") = AMOUNT;
            row(L"NOTES") = NOTES;
            row(L"ACTIVE") = ACTIVE.GetValue();
            return row;
        }

        void to_template(html_template& t) const
        {
            t(L"BUDGETENTRYID") = BUDGETENTRYID.GetValue();
            t(L"BUDGETYEARID") = BUDGETYEARID.GetValue();
            t(L"CATEGID") = CATEGID.GetValue();
            t(L"PERIOD") = PERIOD;
            t(L"AMOUNT") = AMOUNT;
            t(L"NOTES") = NOTES;
            t(L"ACTIVE") = ACTIVE.GetValue();
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db, bool force_insert = false)
        {
            if (db && db->IsReadOnly()) return false;
            if (!table_ || !db) 
            {
                wxLogError("can not save BUDGETTABLE_V1");
                return false;
            }

            return table_->save(this, db);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!table_ || !db) 
            {
                wxLogError("can not remove BUDGETTABLE_V1");
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
        NUM_COLUMNS = 7
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    /** Name of the table*/    
    wxString name() const { return "BUDGETTABLE_V1"; }

    DB_Table_BUDGETTABLE_V1() : fake_(new Data())
    {
        query_ = "SELECT BUDGETENTRYID, BUDGETYEARID, CATEGID, PERIOD, AMOUNT, NOTES, ACTIVE FROM BUDGETTABLE_V1 ";
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
            sql = "INSERT INTO BUDGETTABLE_V1(BUDGETYEARID, CATEGID, PERIOD, AMOUNT, NOTES, ACTIVE, BUDGETENTRYID) VALUES(?, ?, ?, ?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE BUDGETTABLE_V1 SET BUDGETYEARID = ?, CATEGID = ?, PERIOD = ?, AMOUNT = ?, NOTES = ?, ACTIVE = ? WHERE BUDGETENTRYID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->BUDGETYEARID);
            stmt.Bind(2, entity->CATEGID);
            stmt.Bind(3, entity->PERIOD);
            stmt.Bind(4, entity->AMOUNT);
            stmt.Bind(5, entity->NOTES);
            stmt.Bind(6, entity->ACTIVE);
            stmt.Bind(7, entity->id() > 0 ? entity->BUDGETENTRYID : newId());

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
            wxLogError("BUDGETTABLE_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
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
            wxLogError("BUDGETTABLE_V1: Exception %s", e.GetMessage().utf8_str());
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
        for (Index_By_Id::iterator it = index_by_id_.begin(); it != index_by_id_.end(); ++ it)
        {
            Self::Data* item = it->second;
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

