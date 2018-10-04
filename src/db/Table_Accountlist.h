// -*- C++ -*-
/** @file
 * @brief     CRUD implementation for ACCOUNTLIST SQLite table
 * @warning   Auto generated with sqlite2cpp.py script. DO NOT EDIT!
 * @copyright © 2013-2018 Guan Lisheng
 * @copyright © 2017-2018 Stefano Giorgio
 * @author    Guan Lisheng (guanlisheng@gmail.com)
 * @author    Stefano Giorgio (stef145g)
 * @author    Tomasz Słodkowicz
 * @date      2018-10-07 02:45:31.001407
 */
#pragma once

#include "Table.h"

struct DB_Table_ACCOUNTLIST : public DB_Table
{
    struct Data;
    typedef DB_Table_ACCOUNTLIST Self;

    /** A container to hold list of Data records for the table*/
    struct Data_Set : public std::vector<Self::Data>
    {
        /** Return the data records as a json array string */
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
    ~DB_Table_ACCOUNTLIST() 
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
                db->ExecuteUpdate("CREATE TABLE ACCOUNTLIST(ACCOUNTID integer primary key, ACCOUNTNAME TEXT COLLATE NOCASE NOT NULL UNIQUE, ACCOUNTTYPE TEXT NOT NULL /* Checking, Term, Investment, Credit Card */, ACCOUNTNUM TEXT, STATUS TEXT NOT NULL /* Open, Closed */, NOTES TEXT, HELDAT TEXT, WEBSITE TEXT, CONTACTINFO TEXT, ACCESSINFO TEXT, INITIALBAL numeric, FAVORITEACCT TEXT NOT NULL, CURRENCYID integer NOT NULL, STATEMENTLOCKED integer, STATEMENTDATE TEXT, MINIMUMBALANCE numeric, CREDITLIMIT numeric, INTERESTRATE numeric, PAYMENTDUEDATE text, MINIMUMPAYMENT numeric)");
                this->ensure_data(db);
            }
            catch(const wxSQLite3Exception &e) 
            { 
                wxLogError("ACCOUNTLIST: Exception %s", e.GetMessage().c_str());
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
            db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_ACCOUNTLIST_ACCOUNTTYPE ON ACCOUNTLIST(ACCOUNTTYPE)");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("ACCOUNTLIST: Exception %s", e.GetMessage().c_str());
            return false;
        }

        return true;
    }

    void ensure_data(wxSQLite3Database* db)
    {
        db->Begin();
        db->Commit();
    }
    
    struct ACCOUNTID : public DB_Column<int>
    { 
        static wxString name() { return "ACCOUNTID"; } 
        explicit ACCOUNTID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct ACCOUNTNAME : public DB_Column<wxString>
    { 
        static wxString name() { return "ACCOUNTNAME"; } 
        explicit ACCOUNTNAME(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct ACCOUNTTYPE : public DB_Column<wxString>
    { 
        static wxString name() { return "ACCOUNTTYPE"; } 
        explicit ACCOUNTTYPE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct ACCOUNTNUM : public DB_Column<wxString>
    { 
        static wxString name() { return "ACCOUNTNUM"; } 
        explicit ACCOUNTNUM(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct STATUS : public DB_Column<wxString>
    { 
        static wxString name() { return "STATUS"; } 
        explicit STATUS(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct NOTES : public DB_Column<wxString>
    { 
        static wxString name() { return "NOTES"; } 
        explicit NOTES(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct HELDAT : public DB_Column<wxString>
    { 
        static wxString name() { return "HELDAT"; } 
        explicit HELDAT(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct WEBSITE : public DB_Column<wxString>
    { 
        static wxString name() { return "WEBSITE"; } 
        explicit WEBSITE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct CONTACTINFO : public DB_Column<wxString>
    { 
        static wxString name() { return "CONTACTINFO"; } 
        explicit CONTACTINFO(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct ACCESSINFO : public DB_Column<wxString>
    { 
        static wxString name() { return "ACCESSINFO"; } 
        explicit ACCESSINFO(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct INITIALBAL : public DB_Column<double>
    { 
        static wxString name() { return "INITIALBAL"; } 
        explicit INITIALBAL(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    
    struct FAVORITEACCT : public DB_Column<wxString>
    { 
        static wxString name() { return "FAVORITEACCT"; } 
        explicit FAVORITEACCT(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct CURRENCYID : public DB_Column<int>
    { 
        static wxString name() { return "CURRENCYID"; } 
        explicit CURRENCYID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct STATEMENTLOCKED : public DB_Column<int>
    { 
        static wxString name() { return "STATEMENTLOCKED"; } 
        explicit STATEMENTLOCKED(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct STATEMENTDATE : public DB_Column<wxString>
    { 
        static wxString name() { return "STATEMENTDATE"; } 
        explicit STATEMENTDATE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct MINIMUMBALANCE : public DB_Column<double>
    { 
        static wxString name() { return "MINIMUMBALANCE"; } 
        explicit MINIMUMBALANCE(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    
    struct CREDITLIMIT : public DB_Column<double>
    { 
        static wxString name() { return "CREDITLIMIT"; } 
        explicit CREDITLIMIT(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    
    struct INTERESTRATE : public DB_Column<double>
    { 
        static wxString name() { return "INTERESTRATE"; } 
        explicit INTERESTRATE(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    
    struct PAYMENTDUEDATE : public DB_Column<wxString>
    { 
        static wxString name() { return "PAYMENTDUEDATE"; } 
        explicit PAYMENTDUEDATE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct MINIMUMPAYMENT : public DB_Column<double>
    { 
        static wxString name() { return "MINIMUMPAYMENT"; } 
        explicit MINIMUMPAYMENT(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
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
        , COL_STATEMENTLOCKED = 13
        , COL_STATEMENTDATE = 14
        , COL_MINIMUMBALANCE = 15
        , COL_CREDITLIMIT = 16
        , COL_INTERESTRATE = 17
        , COL_PAYMENTDUEDATE = 18
        , COL_MINIMUMPAYMENT = 19
        , COL_UNKNOWN = -1
    };

    /** Returns the column name as a string*/
    static wxString column_to_name(COLUMN col)
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
            case COL_STATEMENTLOCKED: return "STATEMENTLOCKED";
            case COL_STATEMENTDATE: return "STATEMENTDATE";
            case COL_MINIMUMBALANCE: return "MINIMUMBALANCE";
            case COL_CREDITLIMIT: return "CREDITLIMIT";
            case COL_INTERESTRATE: return "INTERESTRATE";
            case COL_PAYMENTDUEDATE: return "PAYMENTDUEDATE";
            case COL_MINIMUMPAYMENT: return "MINIMUMPAYMENT";
            default: break;
        }
        
        return "UNKNOWN";
    }

    /** Returns the column number from the given column name*/
    static COLUMN name_to_column(const wxString& name)
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
        else if ("STATEMENTLOCKED" == name) return COL_STATEMENTLOCKED;
        else if ("STATEMENTDATE" == name) return COL_STATEMENTDATE;
        else if ("MINIMUMBALANCE" == name) return COL_MINIMUMBALANCE;
        else if ("CREDITLIMIT" == name) return COL_CREDITLIMIT;
        else if ("INTERESTRATE" == name) return COL_INTERESTRATE;
        else if ("PAYMENTDUEDATE" == name) return COL_PAYMENTDUEDATE;
        else if ("MINIMUMPAYMENT" == name) return COL_MINIMUMPAYMENT;

        return COL_UNKNOWN;
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_ACCOUNTLIST;
        /** This is a instance pointer to itself in memory. */
        Self* table_;
    
        int ACCOUNTID; // primary key
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
        int STATEMENTLOCKED;
        wxString STATEMENTDATE;
        double MINIMUMBALANCE;
        double CREDITLIMIT;
        double INTERESTRATE;
        wxString PAYMENTDUEDATE;
        double MINIMUMPAYMENT;

        int id() const
        {
            return ACCOUNTID;
        }

        void id(int id)
        {
            ACCOUNTID = id;
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
        
            ACCOUNTID = -1;
            INITIALBAL = 0.0;
            CURRENCYID = -1;
            STATEMENTLOCKED = -1;
            MINIMUMBALANCE = 0.0;
            CREDITLIMIT = 0.0;
            INTERESTRATE = 0.0;
            MINIMUMPAYMENT = 0.0;
        }

        explicit Data(wxSQLite3ResultSet& q, Self* table = 0)
        {
            table_ = table;
        
            ACCOUNTID = q.GetInt(0);
            ACCOUNTNAME = q.GetString(1);
            ACCOUNTTYPE = q.GetString(2);
            ACCOUNTNUM = q.GetString(3);
            STATUS = q.GetString(4);
            NOTES = q.GetString(5);
            HELDAT = q.GetString(6);
            WEBSITE = q.GetString(7);
            CONTACTINFO = q.GetString(8);
            ACCESSINFO = q.GetString(9);
            INITIALBAL = q.GetDouble(10);
            FAVORITEACCT = q.GetString(11);
            CURRENCYID = q.GetInt(12);
            STATEMENTLOCKED = q.GetInt(13);
            STATEMENTDATE = q.GetString(14);
            MINIMUMBALANCE = q.GetDouble(15);
            CREDITLIMIT = q.GetDouble(16);
            INTERESTRATE = q.GetDouble(17);
            PAYMENTDUEDATE = q.GetString(18);
            MINIMUMPAYMENT = q.GetDouble(19);
        }

        Data& operator=(const Data& other)
        {
            if (this == &other) return *this;

            ACCOUNTID = other.ACCOUNTID;
            ACCOUNTNAME = other.ACCOUNTNAME;
            ACCOUNTTYPE = other.ACCOUNTTYPE;
            ACCOUNTNUM = other.ACCOUNTNUM;
            STATUS = other.STATUS;
            NOTES = other.NOTES;
            HELDAT = other.HELDAT;
            WEBSITE = other.WEBSITE;
            CONTACTINFO = other.CONTACTINFO;
            ACCESSINFO = other.ACCESSINFO;
            INITIALBAL = other.INITIALBAL;
            FAVORITEACCT = other.FAVORITEACCT;
            CURRENCYID = other.CURRENCYID;
            STATEMENTLOCKED = other.STATEMENTLOCKED;
            STATEMENTDATE = other.STATEMENTDATE;
            MINIMUMBALANCE = other.MINIMUMBALANCE;
            CREDITLIMIT = other.CREDITLIMIT;
            INTERESTRATE = other.INTERESTRATE;
            PAYMENTDUEDATE = other.PAYMENTDUEDATE;
            MINIMUMPAYMENT = other.MINIMUMPAYMENT;
            return *this;
        }


        bool match(const Self::ACCOUNTID &in) const
        {
            return this->ACCOUNTID == in.v_;
        }

        bool match(const Self::ACCOUNTNAME &in) const
        {
            return this->ACCOUNTNAME.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::ACCOUNTTYPE &in) const
        {
            return this->ACCOUNTTYPE.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::ACCOUNTNUM &in) const
        {
            return this->ACCOUNTNUM.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::STATUS &in) const
        {
            return this->STATUS.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::NOTES &in) const
        {
            return this->NOTES.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::HELDAT &in) const
        {
            return this->HELDAT.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::WEBSITE &in) const
        {
            return this->WEBSITE.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::CONTACTINFO &in) const
        {
            return this->CONTACTINFO.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::ACCESSINFO &in) const
        {
            return this->ACCESSINFO.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::INITIALBAL &in) const
        {
            return this->INITIALBAL == in.v_;
        }

        bool match(const Self::FAVORITEACCT &in) const
        {
            return this->FAVORITEACCT.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::CURRENCYID &in) const
        {
            return this->CURRENCYID == in.v_;
        }

        bool match(const Self::STATEMENTLOCKED &in) const
        {
            return this->STATEMENTLOCKED == in.v_;
        }

        bool match(const Self::STATEMENTDATE &in) const
        {
            return this->STATEMENTDATE.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::MINIMUMBALANCE &in) const
        {
            return this->MINIMUMBALANCE == in.v_;
        }

        bool match(const Self::CREDITLIMIT &in) const
        {
            return this->CREDITLIMIT == in.v_;
        }

        bool match(const Self::INTERESTRATE &in) const
        {
            return this->INTERESTRATE == in.v_;
        }

        bool match(const Self::PAYMENTDUEDATE &in) const
        {
            return this->PAYMENTDUEDATE.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::MINIMUMPAYMENT &in) const
        {
            return this->MINIMUMPAYMENT == in.v_;
        }

        /** Return the data record as a json string */
        wxString to_json() const
        {
            StringBuffer json_buffer;
            PrettyWriter<StringBuffer> json_writer(json_buffer);

            json_writer.StartObject();
            this->as_json(json_writer);
            json_writer.EndObject();

            return json_buffer.GetString();
        }

        /** Add the field data as json key:value pairs */
        void as_json(PrettyWriter<StringBuffer>& json_writer) const
        {
            json_writer.Key("ACCOUNTID");
            json_writer.Int(this->ACCOUNTID);
            json_writer.Key("ACCOUNTNAME");
            json_writer.String(this->ACCOUNTNAME.c_str());
            json_writer.Key("ACCOUNTTYPE");
            json_writer.String(this->ACCOUNTTYPE.c_str());
            json_writer.Key("ACCOUNTNUM");
            json_writer.String(this->ACCOUNTNUM.c_str());
            json_writer.Key("STATUS");
            json_writer.String(this->STATUS.c_str());
            json_writer.Key("NOTES");
            json_writer.String(this->NOTES.c_str());
            json_writer.Key("HELDAT");
            json_writer.String(this->HELDAT.c_str());
            json_writer.Key("WEBSITE");
            json_writer.String(this->WEBSITE.c_str());
            json_writer.Key("CONTACTINFO");
            json_writer.String(this->CONTACTINFO.c_str());
            json_writer.Key("ACCESSINFO");
            json_writer.String(this->ACCESSINFO.c_str());
            json_writer.Key("INITIALBAL");
            json_writer.Double(this->INITIALBAL);
            json_writer.Key("FAVORITEACCT");
            json_writer.String(this->FAVORITEACCT.c_str());
            json_writer.Key("CURRENCYID");
            json_writer.Int(this->CURRENCYID);
            json_writer.Key("STATEMENTLOCKED");
            json_writer.Int(this->STATEMENTLOCKED);
            json_writer.Key("STATEMENTDATE");
            json_writer.String(this->STATEMENTDATE.c_str());
            json_writer.Key("MINIMUMBALANCE");
            json_writer.Double(this->MINIMUMBALANCE);
            json_writer.Key("CREDITLIMIT");
            json_writer.Double(this->CREDITLIMIT);
            json_writer.Key("INTERESTRATE");
            json_writer.Double(this->INTERESTRATE);
            json_writer.Key("PAYMENTDUEDATE");
            json_writer.String(this->PAYMENTDUEDATE.c_str());
            json_writer.Key("MINIMUMPAYMENT");
            json_writer.Double(this->MINIMUMPAYMENT);
        }

        row_t to_row_t() const
        {
            row_t row;
            row(L"ACCOUNTID") = ACCOUNTID;
            row(L"ACCOUNTNAME") = ACCOUNTNAME;
            row(L"ACCOUNTTYPE") = ACCOUNTTYPE;
            row(L"ACCOUNTNUM") = ACCOUNTNUM;
            row(L"STATUS") = STATUS;
            row(L"NOTES") = NOTES;
            row(L"HELDAT") = HELDAT;
            row(L"WEBSITE") = WEBSITE;
            row(L"CONTACTINFO") = CONTACTINFO;
            row(L"ACCESSINFO") = ACCESSINFO;
            row(L"INITIALBAL") = INITIALBAL;
            row(L"FAVORITEACCT") = FAVORITEACCT;
            row(L"CURRENCYID") = CURRENCYID;
            row(L"STATEMENTLOCKED") = STATEMENTLOCKED;
            row(L"STATEMENTDATE") = STATEMENTDATE;
            row(L"MINIMUMBALANCE") = MINIMUMBALANCE;
            row(L"CREDITLIMIT") = CREDITLIMIT;
            row(L"INTERESTRATE") = INTERESTRATE;
            row(L"PAYMENTDUEDATE") = PAYMENTDUEDATE;
            row(L"MINIMUMPAYMENT") = MINIMUMPAYMENT;
            return row;
        }

        void to_template(html_template& t) const
        {
            t(L"ACCOUNTID") = ACCOUNTID;
            t(L"ACCOUNTNAME") = ACCOUNTNAME;
            t(L"ACCOUNTTYPE") = ACCOUNTTYPE;
            t(L"ACCOUNTNUM") = ACCOUNTNUM;
            t(L"STATUS") = STATUS;
            t(L"NOTES") = NOTES;
            t(L"HELDAT") = HELDAT;
            t(L"WEBSITE") = WEBSITE;
            t(L"CONTACTINFO") = CONTACTINFO;
            t(L"ACCESSINFO") = ACCESSINFO;
            t(L"INITIALBAL") = INITIALBAL;
            t(L"FAVORITEACCT") = FAVORITEACCT;
            t(L"CURRENCYID") = CURRENCYID;
            t(L"STATEMENTLOCKED") = STATEMENTLOCKED;
            t(L"STATEMENTDATE") = STATEMENTDATE;
            t(L"MINIMUMBALANCE") = MINIMUMBALANCE;
            t(L"CREDITLIMIT") = CREDITLIMIT;
            t(L"INTERESTRATE") = INTERESTRATE;
            t(L"PAYMENTDUEDATE") = PAYMENTDUEDATE;
            t(L"MINIMUMPAYMENT") = MINIMUMPAYMENT;
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db)
        {
            if (db && db->IsReadOnly()) return false;
            if (!table_ || !db) 
            {
                wxLogError("can not save ACCOUNTLIST");
                return false;
            }

            return table_->save(this, db);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!table_ || !db) 
            {
                wxLogError("can not remove ACCOUNTLIST");
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
        NUM_COLUMNS = 20
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    /** Name of the table */
    wxString name() const { return "ACCOUNTLIST"; }

    DB_Table_ACCOUNTLIST() : fake_(new Data())
    {
        query_ = "SELECT ACCOUNTID, ACCOUNTNAME, ACCOUNTTYPE, ACCOUNTNUM, STATUS, NOTES, HELDAT, WEBSITE, CONTACTINFO, ACCESSINFO, INITIALBAL, FAVORITEACCT, CURRENCYID, STATEMENTLOCKED, STATEMENTDATE, MINIMUMBALANCE, CREDITLIMIT, INTERESTRATE, PAYMENTDUEDATE, MINIMUMPAYMENT FROM ACCOUNTLIST ";
    }

    /** Create a new Data record and add to memory table (cache) */
    Self::Data* create()
    {
        Self::Data* entity = new Self::Data(this);
        cache_.push_back(entity);
        return entity;
    }
    
    /** Create a copy of the Data record and add to memory table (cache) */
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
            sql = "INSERT INTO ACCOUNTLIST(ACCOUNTNAME, ACCOUNTTYPE, ACCOUNTNUM, STATUS, NOTES, HELDAT, WEBSITE, CONTACTINFO, ACCESSINFO, INITIALBAL, FAVORITEACCT, CURRENCYID, STATEMENTLOCKED, STATEMENTDATE, MINIMUMBALANCE, CREDITLIMIT, INTERESTRATE, PAYMENTDUEDATE, MINIMUMPAYMENT) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE ACCOUNTLIST SET ACCOUNTNAME = ?, ACCOUNTTYPE = ?, ACCOUNTNUM = ?, STATUS = ?, NOTES = ?, HELDAT = ?, WEBSITE = ?, CONTACTINFO = ?, ACCESSINFO = ?, INITIALBAL = ?, FAVORITEACCT = ?, CURRENCYID = ?, STATEMENTLOCKED = ?, STATEMENTDATE = ?, MINIMUMBALANCE = ?, CREDITLIMIT = ?, INTERESTRATE = ?, PAYMENTDUEDATE = ?, MINIMUMPAYMENT = ? WHERE ACCOUNTID = ?";
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
            stmt.Bind(13, entity->STATEMENTLOCKED);
            stmt.Bind(14, entity->STATEMENTDATE);
            stmt.Bind(15, entity->MINIMUMBALANCE);
            stmt.Bind(16, entity->CREDITLIMIT);
            stmt.Bind(17, entity->INTERESTRATE);
            stmt.Bind(18, entity->PAYMENTDUEDATE);
            stmt.Bind(19, entity->MINIMUMPAYMENT);
            if (entity->id() > 0)
                stmt.Bind(20, entity->ACCOUNTID);

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
            wxLogError("ACCOUNTLIST: Exception %s, %s", e.GetMessage().c_str(), entity->to_json());
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
            wxString sql = "DELETE FROM ACCOUNTLIST WHERE ACCOUNTID = ?";
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
            wxLogError("ACCOUNTLIST: Exception %s", e.GetMessage().c_str());
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

