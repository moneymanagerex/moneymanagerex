// -*- C++ -*-
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
 *          AUTO GENERATED at 2025-02-04 16:22:14.834591.
 *          DO NOT EDIT!
 */
//=============================================================================
#pragma once

#include "DB_Table.h"

struct DB_Table_CATEGORY_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_CATEGORY_V1 Self;

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
    ~DB_Table_CATEGORY_V1() 
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
                db->ExecuteUpdate("CREATE TABLE CATEGORY_V1( CATEGID INTEGER PRIMARY KEY,  CATEGNAME TEXT NOT NULL COLLATE NOCASE,  ACTIVE INTEGER,  PARENTID INTEGER,  UNIQUE(CATEGNAME, PARENTID))");
                this->ensure_data(db);
            }
            catch(const wxSQLite3Exception &e) 
            { 
                wxLogError("CATEGORY_V1: Exception %s", e.GetMessage().utf8_str());
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
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_CATEGORY_CATEGNAME ON CATEGORY_V1(CATEGNAME)");
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_CATEGORY_CATEGNAME_PARENTID ON CATEGORY_V1(CATEGNAME, PARENTID)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("CATEGORY_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }

        return true;
    }

    void ensure_data(wxSQLite3Database* db)
    {
        db->Begin();
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('1', '%s', '1', '-1')", _("Bills")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('2', '%s', '1', '1')", _("Telephone")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('3', '%s', '1', '1')", _("Electricity")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('4', '%s', '1', '1')", _("Gas")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('5', '%s', '1', '1')", _("Internet")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('6', '%s', '1', '1')", _("Rent")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('7', '%s', '1', '1')", _("Cable TV")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('8', '%s', '1', '1')", _("Water")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('9', '%s', '1', '-1')", _("Food")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('10', '%s', '1', '9')", _("Groceries")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('11', '%s', '1', '9')", _("Dining out")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('12', '%s', '1', '-1')", _("Leisure")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('13', '%s', '1', '12')", _("Movies")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('14', '%s', '1', '12')", _("Video Rental")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('15', '%s', '1', '12')", _("Magazines")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('16', '%s', '1', '-1')", _("Automobile")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('17', '%s', '1', '16')", _("Maintenance")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('18', '%s', '1', '16')", _("Gas")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('19', '%s', '1', '16')", _("Parking")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('20', '%s', '1', '16')", _("Registration")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('21', '%s', '1', '-1')", _("Education")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('22', '%s', '1', '21')", _("Books")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('23', '%s', '1', '21')", _("Tuition")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('24', '%s', '1', '21')", _("Others")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('25', '%s', '1', '-1')", _("Homeneeds")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('26', '%s', '1', '25')", _("Clothing")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('27', '%s', '1', '25')", _("Furnishing")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('28', '%s', '1', '25')", _("Others")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('29', '%s', '1', '-1')", _("Healthcare")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('30', '%s', '1', '29')", _("Health")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('31', '%s', '1', '29')", _("Dental")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('32', '%s', '1', '29')", _("Eyecare")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('33', '%s', '1', '29')", _("Physician")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('34', '%s', '1', '29')", _("Prescriptions")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('35', '%s', '1', '-1')", _("Insurance")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('36', '%s', '1', '35')", _("Auto")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('37', '%s', '1', '35')", _("Life")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('38', '%s', '1', '35')", _("Home")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('39', '%s', '1', '35')", _("Health")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('40', '%s', '1', '-1')", _("Vacation")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('41', '%s', '1', '40')", _("Travel")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('42', '%s', '1', '40')", _("Lodging")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('43', '%s', '1', '40')", _("Sightseeing")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('44', '%s', '1', '-1')", _("Taxes")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('45', '%s', '1', '44')", _("Income Tax")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('46', '%s', '1', '44')", _("House Tax")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('47', '%s', '1', '44')", _("Water Tax")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('48', '%s', '1', '44')", _("Others")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('49', '%s', '1', '-1')", _("Miscellaneous")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('50', '%s', '1', '-1')", _("Gifts")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('51', '%s', '1', '-1')", _("Income")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('52', '%s', '1', '51')", _("Salary")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('53', '%s', '1', '51')", _("Reimbursement/Refunds")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('54', '%s', '1', '51')", _("Investment Income")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('55', '%s', '1', '-1')", _("Other Income")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('56', '%s', '1', '-1')", _("Other Expenses")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('57', '%s', '1', '-1')", _("Transfer")));
        db->Commit();
    }
    
    struct CATEGID : public DB_Column<int64>
    { 
        static wxString name() { return "CATEGID"; } 
        explicit CATEGID(const int64 &v, OP op = EQUAL): DB_Column<int64>(v, op) {}
    };
    
    struct CATEGNAME : public DB_Column<wxString>
    { 
        static wxString name() { return "CATEGNAME"; } 
        explicit CATEGNAME(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct ACTIVE : public DB_Column<int64>
    { 
        static wxString name() { return "ACTIVE"; } 
        explicit ACTIVE(const int64 &v, OP op = EQUAL): DB_Column<int64>(v, op) {}
    };
    
    struct PARENTID : public DB_Column<int64>
    { 
        static wxString name() { return "PARENTID"; } 
        explicit PARENTID(const int64 &v, OP op = EQUAL): DB_Column<int64>(v, op) {}
    };
    
    typedef CATEGID PRIMARY;
    enum COLUMN
    {
        COL_CATEGID = 0
        , COL_CATEGNAME = 1
        , COL_ACTIVE = 2
        , COL_PARENTID = 3
    };

    /** Returns the column name as a string*/
    static wxString column_to_name(const COLUMN col)
    {
        switch(col)
        {
            case COL_CATEGID: return "CATEGID";
            case COL_CATEGNAME: return "CATEGNAME";
            case COL_ACTIVE: return "ACTIVE";
            case COL_PARENTID: return "PARENTID";
            default: break;
        }
        
        return "UNKNOWN";
    }

    /** Returns the column number from the given column name*/
    static COLUMN name_to_column(const wxString& name)
    {
        if ("CATEGID" == name) return COL_CATEGID;
        else if ("CATEGNAME" == name) return COL_CATEGNAME;
        else if ("ACTIVE" == name) return COL_ACTIVE;
        else if ("PARENTID" == name) return COL_PARENTID;

        return COLUMN(-1);
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_CATEGORY_V1;
        /** This is a instance pointer to itself in memory. */
        Self* table_;
    
        int64 CATEGID;//  primary key
        wxString CATEGNAME;
        int64 ACTIVE;
        int64 PARENTID;

        int64 id() const
        {
            return CATEGID;
        }

        void id(const int64 id)
        {
            CATEGID = id;
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
            if(CATEGID != r->CATEGID) return false;
            if(!CATEGNAME.IsSameAs(r->CATEGNAME)) return false;
            if(ACTIVE != r->ACTIVE) return false;
            if(PARENTID != r->PARENTID) return false;
            return true;
        }
        
        explicit Data(Self* table = nullptr ) 
        {
            table_ = table;
        
            CATEGID = -1;
            ACTIVE = -1;
            PARENTID = -1;
        }

        explicit Data(wxSQLite3ResultSet& q, Self* table = nullptr )
        {
            table_ = table;
        
            CATEGID = q.GetInt64(0); // CATEGID
            CATEGNAME = q.GetString(1); // CATEGNAME
            ACTIVE = q.GetInt64(2); // ACTIVE
            PARENTID = q.GetInt64(3); // PARENTID
        }

        Data(const Data& other) = default;

        Data& operator=(const Data& other)
        {
            if (this == &other) return *this;

            CATEGID = other.CATEGID;
            CATEGNAME = other.CATEGNAME;
            ACTIVE = other.ACTIVE;
            PARENTID = other.PARENTID;
            return *this;
        }

        template<typename C>
        bool match(const C &) const
        {
            return false;
        }

        bool match(const Self::CATEGID &in) const
        {
            return this->CATEGID == in.v_;
        }

        bool match(const Self::CATEGNAME &in) const
        {
            return this->CATEGNAME.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::ACTIVE &in) const
        {
            return this->ACTIVE == in.v_;
        }

        bool match(const Self::PARENTID &in) const
        {
            return this->PARENTID == in.v_;
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
            json_writer.Key("CATEGID");
            json_writer.Int64(this->CATEGID.GetValue());
            json_writer.Key("CATEGNAME");
            json_writer.String(this->CATEGNAME.utf8_str());
            json_writer.Key("ACTIVE");
            json_writer.Int64(this->ACTIVE.GetValue());
            json_writer.Key("PARENTID");
            json_writer.Int64(this->PARENTID.GetValue());
        }

        row_t to_row_t() const
        {
            row_t row;
            row(L"CATEGID") = CATEGID.GetValue();
            row(L"CATEGNAME") = CATEGNAME;
            row(L"ACTIVE") = ACTIVE.GetValue();
            row(L"PARENTID") = PARENTID.GetValue();
            return row;
        }

        void to_template(html_template& t) const
        {
            t(L"CATEGID") = CATEGID.GetValue();
            t(L"CATEGNAME") = CATEGNAME;
            t(L"ACTIVE") = ACTIVE.GetValue();
            t(L"PARENTID") = PARENTID.GetValue();
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db)
        {
            if (db && db->IsReadOnly()) return false;
            if (!table_ || !db) 
            {
                wxLogError("can not save CATEGORY_V1");
                return false;
            }

            return table_->save(this, db);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!table_ || !db) 
            {
                wxLogError("can not remove CATEGORY_V1");
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
        NUM_COLUMNS = 4
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    /** Name of the table*/    
    wxString name() const { return "CATEGORY_V1"; }

    DB_Table_CATEGORY_V1() : fake_(new Data())
    {
        query_ = "SELECT CATEGID, CATEGNAME, ACTIVE, PARENTID FROM CATEGORY_V1 ";
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
            sql = "INSERT INTO CATEGORY_V1(CATEGNAME, ACTIVE, PARENTID, CATEGID) VALUES(?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE CATEGORY_V1 SET CATEGNAME = ?, ACTIVE = ?, PARENTID = ? WHERE CATEGID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->CATEGNAME);
            stmt.Bind(2, entity->ACTIVE);
            stmt.Bind(3, entity->PARENTID);
            stmt.Bind(4, entity->id() > 0 ? entity->CATEGID : newId());

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
            wxLogError("CATEGORY_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
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
            wxString sql = "DELETE FROM CATEGORY_V1 WHERE CATEGID = ?";
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
            wxLogError("CATEGORY_V1: Exception %s", e.GetMessage().utf8_str());
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

        Index_By_Id::iterator it = index_by_id_.find(id);
        if (it != index_by_id_.end())
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

