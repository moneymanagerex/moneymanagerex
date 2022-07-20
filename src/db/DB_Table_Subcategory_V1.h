// -*- C++ -*-
//=============================================================================
/**
 *      Copyright: (c) 2013 - 2022 Guan Lisheng (guanlisheng@gmail.com)
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
 *          AUTO GENERATED at 2022-07-20 15:29:27.776453.
 *          DO NOT EDIT!
 */
//=============================================================================
#pragma once

#include "DB_Table.h"

struct DB_Table_SUBCATEGORY_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_SUBCATEGORY_V1 Self;

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
    typedef std::map<int, Self::Data*> Index_By_Id;
    Cache cache_;
    Index_By_Id index_by_id_;
    Data* fake_; // in case the entity not found

    /** Destructor: clears any data records stored in memory */
    ~DB_Table_SUBCATEGORY_V1() 
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
                db->ExecuteUpdate("CREATE TABLE SUBCATEGORY_V1(SUBCATEGID integer primary key, SUBCATEGNAME TEXT COLLATE NOCASE NOT NULL, CATEGID integer NOT NULL, UNIQUE(CATEGID, SUBCATEGNAME))");
                this->ensure_data(db);
            }
            catch(const wxSQLite3Exception &e) 
            { 
                wxLogError("SUBCATEGORY_V1: Exception %s", e.GetMessage().utf8_str());
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
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_SUBCATEGORY_CATEGID ON SUBCATEGORY_V1(CATEGID)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("SUBCATEGORY_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }

        return true;
    }

    void ensure_data(wxSQLite3Database* db)
    {
        db->Begin();
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('1', '%s', '1')", _("Telephone")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('2', '%s', '1')", _("Electricity")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('3', '%s', '1')", _("Gas")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('4', '%s', '1')", _("Internet")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('5', '%s', '1')", _("Rent")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('6', '%s', '1')", _("Cable TV")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('7', '%s', '1')", _("Water")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('8', '%s', '2')", _("Groceries")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('9', '%s', '2')", _("Dining out")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('10', '%s', '3')", _("Movies")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('11', '%s', '3')", _("Video Rental")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('12', '%s', '3')", _("Magazines")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('13', '%s', '4')", _("Maintenance")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('14', '%s', '4')", _("Gas")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('15', '%s', '4')", _("Parking")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('16', '%s', '4')", _("Registration")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('17', '%s', '5')", _("Books")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('18', '%s', '5')", _("Tuition")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('19', '%s', '5')", _("Others")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('20', '%s', '6')", _("Clothing")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('21', '%s', '6')", _("Furnishing")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('22', '%s', '6')", _("Others")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('23', '%s', '7')", _("Health")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('24', '%s', '7')", _("Dental")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('25', '%s', '7')", _("Eyecare")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('26', '%s', '7')", _("Physician")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('27', '%s', '7')", _("Prescriptions")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('28', '%s', '8')", _("Auto")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('29', '%s', '8')", _("Life")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('30', '%s', '8')", _("Home")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('31', '%s', '8')", _("Health")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('32', '%s', '9')", _("Travel")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('33', '%s', '9')", _("Lodging")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('34', '%s', '9')", _("Sightseeing")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('35', '%s', '10')", _("Income Tax")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('36', '%s', '10')", _("House Tax")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('37', '%s', '10')", _("Water Tax")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('38', '%s', '10')", _("Others")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('39', '%s', '13')", _("Salary")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('40', '%s', '13')", _("Reimbursement/Refunds")));
        db->ExecuteUpdate(wxString::Format("INSERT INTO SUBCATEGORY_V1 VALUES ('41', '%s', '13')", _("Investment Income")));
        db->Commit();
    }
    
    struct SUBCATEGID : public DB_Column<int>
    { 
        static wxString name() { return "SUBCATEGID"; } 
        explicit SUBCATEGID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct SUBCATEGNAME : public DB_Column<wxString>
    { 
        static wxString name() { return "SUBCATEGNAME"; } 
        explicit SUBCATEGNAME(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct CATEGID : public DB_Column<int>
    { 
        static wxString name() { return "CATEGID"; } 
        explicit CATEGID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    typedef SUBCATEGID PRIMARY;
    enum COLUMN
    {
        COL_SUBCATEGID = 0
        , COL_SUBCATEGNAME = 1
        , COL_CATEGID = 2
    };

    /** Returns the column name as a string*/
    static wxString column_to_name(COLUMN col)
    {
        switch(col)
        {
            case COL_SUBCATEGID: return "SUBCATEGID";
            case COL_SUBCATEGNAME: return "SUBCATEGNAME";
            case COL_CATEGID: return "CATEGID";
            default: break;
        }
        
        return "UNKNOWN";
    }

    /** Returns the column number from the given column name*/
    static COLUMN name_to_column(const wxString& name)
    {
        if ("SUBCATEGID" == name) return COL_SUBCATEGID;
        else if ("SUBCATEGNAME" == name) return COL_SUBCATEGNAME;
        else if ("CATEGID" == name) return COL_CATEGID;

        return COLUMN(-1);
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_SUBCATEGORY_V1;
        /** This is a instance pointer to itself in memory. */
        Self* table_;
    
        int SUBCATEGID;//  primary key
        wxString SUBCATEGNAME;
        int CATEGID;

        int id() const
        {
            return SUBCATEGID;
        }

        void id(int id)
        {
            SUBCATEGID = id;
        }

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
        
            SUBCATEGID = -1;
            CATEGID = -1;
        }

        explicit Data(wxSQLite3ResultSet& q, Self* table = 0)
        {
            table_ = table;
        
            SUBCATEGID = q.GetInt(0); // SUBCATEGID
            SUBCATEGNAME = q.GetString(1); // SUBCATEGNAME
            CATEGID = q.GetInt(2); // CATEGID
        }

        Data& operator=(const Data& other)
        {
            if (this == &other) return *this;

            SUBCATEGID = other.SUBCATEGID;
            SUBCATEGNAME = other.SUBCATEGNAME;
            CATEGID = other.CATEGID;
            return *this;
        }

        template<typename C>
        bool match(const C &c) const
        {
            return false;
        }

        bool match(const Self::SUBCATEGID &in) const
        {
            return this->SUBCATEGID == in.v_;
        }

        bool match(const Self::SUBCATEGNAME &in) const
        {
            return this->SUBCATEGNAME.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::CATEGID &in) const
        {
            return this->CATEGID == in.v_;
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
            json_writer.Key("SUBCATEGID");
            json_writer.Int(this->SUBCATEGID);
            json_writer.Key("SUBCATEGNAME");
            json_writer.String(this->SUBCATEGNAME.utf8_str());
            json_writer.Key("CATEGID");
            json_writer.Int(this->CATEGID);
        }

        row_t to_row_t() const
        {
            row_t row;
            row(L"SUBCATEGID") = SUBCATEGID;
            row(L"SUBCATEGNAME") = SUBCATEGNAME;
            row(L"CATEGID") = CATEGID;
            return row;
        }

        void to_template(html_template& t) const
        {
            t(L"SUBCATEGID") = SUBCATEGID;
            t(L"SUBCATEGNAME") = SUBCATEGNAME;
            t(L"CATEGID") = CATEGID;
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db)
        {
            if (db && db->IsReadOnly()) return false;
            if (!table_ || !db) 
            {
                wxLogError("can not save SUBCATEGORY_V1");
                return false;
            }

            return table_->save(this, db);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!table_ || !db) 
            {
                wxLogError("can not remove SUBCATEGORY_V1");
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
        NUM_COLUMNS = 3
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    /** Name of the table*/    
    wxString name() const { return "SUBCATEGORY_V1"; }

    DB_Table_SUBCATEGORY_V1() : fake_(new Data())
    {
        query_ = "SELECT SUBCATEGID, SUBCATEGNAME, CATEGID FROM SUBCATEGORY_V1 ";
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
            sql = "INSERT INTO SUBCATEGORY_V1(SUBCATEGNAME, CATEGID) VALUES(?, ?)";
        }
        else
        {
            sql = "UPDATE SUBCATEGORY_V1 SET SUBCATEGNAME = ?, CATEGID = ? WHERE SUBCATEGID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->SUBCATEGNAME);
            stmt.Bind(2, entity->CATEGID);
            if (entity->id() > 0)
                stmt.Bind(3, entity->SUBCATEGID);

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
            wxLogError("SUBCATEGORY_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
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
            wxString sql = "DELETE FROM SUBCATEGORY_V1 WHERE SUBCATEGID = ?";
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
            wxLogError("SUBCATEGORY_V1: Exception %s", e.GetMessage().utf8_str());
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
            wxLogError("%s: Exception %s", this->name().utf8_str(), e.GetMessage().utf8_str());
        }

        return result;
    }
};

