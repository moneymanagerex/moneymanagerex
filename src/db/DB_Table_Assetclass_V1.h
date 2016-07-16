// -*- C++ -*-
//=============================================================================
/**
 *      Copyright (c) 2013 - 2016 Guan Lisheng (guanlisheng@gmail.com)
 *
 *      @file
 *
 *      @author [sqlite2cpp.py]
 *
 *      @brief
 *
 *      Revision History:
 *          AUTO GENERATED at 2016-07-16 22:51:23.825000.
 *          DO NOT EDIT!
 */
//=============================================================================

#ifndef DB_TABLE_ASSETCLASS_V1_H
#define DB_TABLE_ASSETCLASS_V1_H

#include "DB_Table.h"

struct DB_Table_ASSETCLASS_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_ASSETCLASS_V1 Self;
    /** A container to hold list of Data records for the table*/
    struct Data_Set : public std::vector<Self::Data>
    {
        std::wstring to_json() const
        {
            json::Array a;
            for (const auto & item: *this)
            {
                json::Object o;
                item.to_json(o);
                a.Insert(o);
            }
            std::wstringstream ss;
            json::Writer::Write(a, ss);
            return ss.str();
        }
    };
    /** A container to hold a list of Data record pointers for the table in memory*/
    typedef std::vector<Self::Data*> Cache;
    typedef std::map<int, Self::Data*> Index_By_Id;
    Cache cache_;
    Index_By_Id index_by_id_;
    Data* fake_; // in case the entity not found

    /** Destructor: clears any data records stored in memory */
    ~DB_Table_ASSETCLASS_V1() 
    {
        delete this->fake_;
        destroy_cache();
    }
     
    /** Removes all records stored in memory (cache) for the table*/ 
    void destroy_cache()
    {
        std::for_each(cache_.begin(), cache_.end(), std::mem_fun(&Data::destroy));
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
                db->ExecuteUpdate("CREATE TABLE ASSETCLASS_V1 (ID INTEGER primary key, PARENTID INTEGER, NAME TEXT COLLATE NOCASE NOT NULL, ALLOCATION REAL, SORTORDER INTEGER)");
                this->ensure_data(db);
            }
            catch(const wxSQLite3Exception &e) 
            { 
                wxLogError("ASSETCLASS_V1: Exception %s", e.GetMessage().c_str());
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
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("ASSETCLASS_V1: Exception %s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }

    void ensure_data(wxSQLite3Database* db)
    {
        db->Begin();
        db->Commit();
    }
    
    struct ID : public DB_Column<int>
    { 
        static wxString name() { return "ID"; } 
        explicit ID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct PARENTID : public DB_Column<int>
    { 
        static wxString name() { return "PARENTID"; } 
        explicit PARENTID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    struct NAME : public DB_Column<wxString>
    { 
        static wxString name() { return "NAME"; } 
        explicit NAME(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    struct ALLOCATION : public DB_Column<double>
    { 
        static wxString name() { return "ALLOCATION"; } 
        explicit ALLOCATION(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    struct SORTORDER : public DB_Column<int>
    { 
        static wxString name() { return "SORTORDER"; } 
        explicit SORTORDER(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    typedef ID PRIMARY;
    enum COLUMN
    {
        COL_ID = 0
        , COL_PARENTID = 1
        , COL_NAME = 2
        , COL_ALLOCATION = 3
        , COL_SORTORDER = 4
    };

    /** Returns the column name as a string*/
    static wxString column_to_name(COLUMN col)
    {
        switch(col)
        {
            case COL_ID: return "ID";
            case COL_PARENTID: return "PARENTID";
            case COL_NAME: return "NAME";
            case COL_ALLOCATION: return "ALLOCATION";
            case COL_SORTORDER: return "SORTORDER";
            default: break;
        }
        
        return "UNKNOWN";
    }

    /** Returns the column number from the given column name*/
    static COLUMN name_to_column(const wxString& name)
    {
        if ("ID" == name) return COL_ID;
        else if ("PARENTID" == name) return COL_PARENTID;
        else if ("NAME" == name) return COL_NAME;
        else if ("ALLOCATION" == name) return COL_ALLOCATION;
        else if ("SORTORDER" == name) return COL_SORTORDER;

        return COLUMN(-1);
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_ASSETCLASS_V1;
        /** This is a instance pointer to itself in memory. */
        Self* table_;
    
        int ID;//  primary key
        int PARENTID;
        wxString NAME;
        double ALLOCATION;
        int SORTORDER;
        int id() const { return ID; }
        void id(int id) { ID = id; }
        bool operator < (const Data& r) const
        {
            return this->id() < r.id();
        }
        bool operator < (const Data* r) const
        {
            return this->id() < r->id();
        }

        explicit Data(Self* table = 0) 
        {
            table_ = table;
        
            ID = -1;
            PARENTID = -1;
            ALLOCATION = 0.0;
            SORTORDER = -1;
        }

        explicit Data(wxSQLite3ResultSet& q, Self* table = 0)
        {
            table_ = table;
        
            ID = q.GetInt(0); // ID
            PARENTID = q.GetInt(1); // PARENTID
            NAME = q.GetString(2); // NAME
            ALLOCATION = q.GetDouble(3); // ALLOCATION
            SORTORDER = q.GetInt(4); // SORTORDER
        }

        Data& operator=(const Data& other)
        {
            if (this == &other) return *this;

            ID = other.ID;
            PARENTID = other.PARENTID;
            NAME = other.NAME;
            ALLOCATION = other.ALLOCATION;
            SORTORDER = other.SORTORDER;
            return *this;
        }

        template<typename C>
        bool match(const C &c) const
        {
            return false;
        }
        bool match(const Self::ID &in) const
        {
            return this->ID == in.v_;
        }
        bool match(const Self::PARENTID &in) const
        {
            return this->PARENTID == in.v_;
        }
        bool match(const Self::NAME &in) const
        {
            return this->NAME.CmpNoCase(in.v_) == 0;
        }
        bool match(const Self::ALLOCATION &in) const
        {
            return this->ALLOCATION == in.v_;
        }
        bool match(const Self::SORTORDER &in) const
        {
            return this->SORTORDER == in.v_;
        }
        wxString to_json() const
        {
            json::Object o;
            this->to_json(o);
            std::wstringstream ss;
            json::Writer::Write(o, ss);
            return ss.str();
        }
        
        int to_json(json::Object& o) const
        {
            o[L"ID"] = json::Number(this->ID);
            o[L"PARENTID"] = json::Number(this->PARENTID);
            o[L"NAME"] = json::String(this->NAME.ToStdWstring());
            o[L"ALLOCATION"] = json::Number(this->ALLOCATION);
            o[L"SORTORDER"] = json::Number(this->SORTORDER);
            return 0;
        }
        row_t to_row_t() const
        {
            row_t row;
            row(L"ID") = ID;
            row(L"PARENTID") = PARENTID;
            row(L"NAME") = NAME;
            row(L"ALLOCATION") = ALLOCATION;
            row(L"SORTORDER") = SORTORDER;
            return row;
        }
        void to_template(html_template& t) const
        {
            t(L"ID") = ID;
            t(L"PARENTID") = PARENTID;
            t(L"NAME") = NAME;
            t(L"ALLOCATION") = ALLOCATION;
            t(L"SORTORDER") = SORTORDER;
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db)
        {
            if (db && db->IsReadOnly()) return false;
            if (!table_ || !db) 
            {
                wxLogError("can not save ASSETCLASS_V1");
                return false;
            }

            return table_->save(this, db);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!table_ || !db) 
            {
                wxLogError("can not remove ASSETCLASS_V1");
                return false;
            }
            
            return table_->remove(this, db);
        }

        void destroy()
        {
            //if (this->id() < 0)
            //    wxSafeShowMessage("unsaved object", this->to_json());
            delete this;
        }
    };

    enum
    {
        NUM_COLUMNS = 5
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    /** Name of the table*/    
    wxString name() const { return "ASSETCLASS_V1"; }

    DB_Table_ASSETCLASS_V1() : fake_(new Data())
    {
        query_ = "SELECT ID, PARENTID, NAME, ALLOCATION, SORTORDER FROM ASSETCLASS_V1 ";
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
    bool save(Self::Data* entity, wxSQLite3Database* db)
    {
        wxString sql = wxEmptyString;
        if (entity->id() <= 0) //  new & insert
        {
            sql = "INSERT INTO ASSETCLASS_V1(PARENTID, NAME, ALLOCATION, SORTORDER) VALUES(?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE ASSETCLASS_V1 SET PARENTID = ?, NAME = ?, ALLOCATION = ?, SORTORDER = ? WHERE ID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->PARENTID);
            stmt.Bind(2, entity->NAME);
            stmt.Bind(3, entity->ALLOCATION);
            stmt.Bind(4, entity->SORTORDER);
            if (entity->id() > 0)
                stmt.Bind(5, entity->ID);

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
            wxLogError("ASSETCLASS_V1: Exception %s, %s", e.GetMessage().c_str(), entity->to_json());
            return false;
        }

        if (entity->id() <= 0)
        {
            entity->id((db->GetLastRowId()).ToLong());
            index_by_id_.insert(std::make_pair(entity->id(), entity));
        }
        return true;
    }

    /** Remove the Data record from the database and the memory table (cache) */
    bool remove(int id, wxSQLite3Database* db)
    {
        if (id <= 0) return false;
        try
        {
            wxString sql = "DELETE FROM ASSETCLASS_V1 WHERE ID = ?";
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
            wxLogError("ASSETCLASS_V1: Exception %s", e.GetMessage().c_str());
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
    Self::Data* get(int id, wxSQLite3Database* db)
    {
        if (id <= 0) 
        {
            ++ skip_;
            return 0;
        }

        Index_By_Id::iterator it = index_by_id_.find(id);
        if (it != index_by_id_.end())
        {
            ++ hit_;
            return it->second;
        }
        
        ++ miss_;
        Self::Data* entity = 0;
        wxString where = wxString::Format(" WHERE %s = ?", PRIMARY::name().c_str());
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
            wxLogError("%s: Exception %s", this->name().c_str(), e.GetMessage().c_str());
        }
        
        if (!entity) 
        {
            entity = this->fake_;
            // wxLogError("%s: %d not found", this->name().c_str(), id);
        }
 
        return entity;
    }

    /**
    * Return a list of Data records (Data_Set) derived directly from the database.
    * The Data_Set is sorted based on the column number.
    */
    const Data_Set all(wxSQLite3Database* db, COLUMN col = COLUMN(0), bool asc = true)
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
            wxLogError("%s: Exception %s", this->name().c_str(), e.GetMessage().c_str());
        }

        return result;
    }
};
#endif //
