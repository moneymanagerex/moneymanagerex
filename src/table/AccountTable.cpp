// -*- C++ -*-
//=============================================================================
/**
 *      Copyright: (c) 2013-2026 Guan Lisheng (guanlisheng@gmail.com)
 *      Copyright: (c) 2017-2018 Stefano Giorgio (stef145g)
 *      Copyright: (c) 2022      Mark Whalley (mark@ipx.co.uk)
 *      Copyright: (c) 2026      George Ef (george.a.ef@gmail.com)
 *
 *      @file
 *
 *      @author [sqlite2cpp.py]
 *
 *      @brief
 *
 *      Revision History:
 *          AUTO GENERATED at 2026-02-15 02:44:45.846505.
 *          DO NOT EDIT!
 */
//=============================================================================

#include "AccountTable.h"

AccountTable::Data::Data()
{
    ACCOUNTID = -1;
    INITIALBAL = 0.0;
    CURRENCYID = -1;
    STATEMENTLOCKED = -1;
    MINIMUMBALANCE = 0.0;
    CREDITLIMIT = 0.0;
    INTERESTRATE = 0.0;
    MINIMUMPAYMENT = 0.0;
}

AccountTable::Data::Data(wxSQLite3ResultSet& q)
{
    ACCOUNTID = q.GetInt64(0);
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
    INITIALDATE = q.GetString(11);
    FAVORITEACCT = q.GetString(12);
    CURRENCYID = q.GetInt64(13);
    STATEMENTLOCKED = q.GetInt64(14);
    STATEMENTDATE = q.GetString(15);
    MINIMUMBALANCE = q.GetDouble(16);
    CREDITLIMIT = q.GetDouble(17);
    INTERESTRATE = q.GetDouble(18);
    PAYMENTDUEDATE = q.GetString(19);
    MINIMUMPAYMENT = q.GetDouble(20);
}

bool AccountTable::Data::equals(const AccountTable::Data* r) const
{
    if (ACCOUNTID != r->ACCOUNTID) return false;
    if (!ACCOUNTNAME.IsSameAs(r->ACCOUNTNAME)) return false;
    if (!ACCOUNTTYPE.IsSameAs(r->ACCOUNTTYPE)) return false;
    if (!ACCOUNTNUM.IsSameAs(r->ACCOUNTNUM)) return false;
    if (!STATUS.IsSameAs(r->STATUS)) return false;
    if (!NOTES.IsSameAs(r->NOTES)) return false;
    if (!HELDAT.IsSameAs(r->HELDAT)) return false;
    if (!WEBSITE.IsSameAs(r->WEBSITE)) return false;
    if (!CONTACTINFO.IsSameAs(r->CONTACTINFO)) return false;
    if (!ACCESSINFO.IsSameAs(r->ACCESSINFO)) return false;
    if (INITIALBAL != r->INITIALBAL) return false;
    if (!INITIALDATE.IsSameAs(r->INITIALDATE)) return false;
    if (!FAVORITEACCT.IsSameAs(r->FAVORITEACCT)) return false;
    if (CURRENCYID != r->CURRENCYID) return false;
    if (STATEMENTLOCKED != r->STATEMENTLOCKED) return false;
    if (!STATEMENTDATE.IsSameAs(r->STATEMENTDATE)) return false;
    if (MINIMUMBALANCE != r->MINIMUMBALANCE) return false;
    if (CREDITLIMIT != r->CREDITLIMIT) return false;
    if (INTERESTRATE != r->INTERESTRATE) return false;
    if (!PAYMENTDUEDATE.IsSameAs(r->PAYMENTDUEDATE)) return false;
    if (MINIMUMPAYMENT != r->MINIMUMPAYMENT) return false;

    return true;
}

AccountTable::Data& AccountTable::Data::operator=(const AccountTable::Data& other)
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
    INITIALDATE = other.INITIALDATE;
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

// Return the data record as a json string
wxString AccountTable::Data::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();			
    this->as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void AccountTable::Data::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("ACCOUNTID");
    json_writer.Int64(this->ACCOUNTID.GetValue());
    json_writer.Key("ACCOUNTNAME");
    json_writer.String(this->ACCOUNTNAME.utf8_str());
    json_writer.Key("ACCOUNTTYPE");
    json_writer.String(this->ACCOUNTTYPE.utf8_str());
    json_writer.Key("ACCOUNTNUM");
    json_writer.String(this->ACCOUNTNUM.utf8_str());
    json_writer.Key("STATUS");
    json_writer.String(this->STATUS.utf8_str());
    json_writer.Key("NOTES");
    json_writer.String(this->NOTES.utf8_str());
    json_writer.Key("HELDAT");
    json_writer.String(this->HELDAT.utf8_str());
    json_writer.Key("WEBSITE");
    json_writer.String(this->WEBSITE.utf8_str());
    json_writer.Key("CONTACTINFO");
    json_writer.String(this->CONTACTINFO.utf8_str());
    json_writer.Key("ACCESSINFO");
    json_writer.String(this->ACCESSINFO.utf8_str());
    json_writer.Key("INITIALBAL");
    json_writer.Double(this->INITIALBAL);
    json_writer.Key("INITIALDATE");
    json_writer.String(this->INITIALDATE.utf8_str());
    json_writer.Key("FAVORITEACCT");
    json_writer.String(this->FAVORITEACCT.utf8_str());
    json_writer.Key("CURRENCYID");
    json_writer.Int64(this->CURRENCYID.GetValue());
    json_writer.Key("STATEMENTLOCKED");
    json_writer.Int64(this->STATEMENTLOCKED.GetValue());
    json_writer.Key("STATEMENTDATE");
    json_writer.String(this->STATEMENTDATE.utf8_str());
    json_writer.Key("MINIMUMBALANCE");
    json_writer.Double(this->MINIMUMBALANCE);
    json_writer.Key("CREDITLIMIT");
    json_writer.Double(this->CREDITLIMIT);
    json_writer.Key("INTERESTRATE");
    json_writer.Double(this->INTERESTRATE);
    json_writer.Key("PAYMENTDUEDATE");
    json_writer.String(this->PAYMENTDUEDATE.utf8_str());
    json_writer.Key("MINIMUMPAYMENT");
    json_writer.Double(this->MINIMUMPAYMENT);
}

row_t AccountTable::Data::to_row_t() const
{
    row_t row;

    row(L"ACCOUNTID") = ACCOUNTID.GetValue();
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
    row(L"INITIALDATE") = INITIALDATE;
    row(L"FAVORITEACCT") = FAVORITEACCT;
    row(L"CURRENCYID") = CURRENCYID.GetValue();
    row(L"STATEMENTLOCKED") = STATEMENTLOCKED.GetValue();
    row(L"STATEMENTDATE") = STATEMENTDATE;
    row(L"MINIMUMBALANCE") = MINIMUMBALANCE;
    row(L"CREDITLIMIT") = CREDITLIMIT;
    row(L"INTERESTRATE") = INTERESTRATE;
    row(L"PAYMENTDUEDATE") = PAYMENTDUEDATE;
    row(L"MINIMUMPAYMENT") = MINIMUMPAYMENT;

    return row;
}

void AccountTable::Data::to_template(html_template& t) const
{
    t(L"ACCOUNTID") = ACCOUNTID.GetValue();
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
    t(L"INITIALDATE") = INITIALDATE;
    t(L"FAVORITEACCT") = FAVORITEACCT;
    t(L"CURRENCYID") = CURRENCYID.GetValue();
    t(L"STATEMENTLOCKED") = STATEMENTLOCKED.GetValue();
    t(L"STATEMENTDATE") = STATEMENTDATE;
    t(L"MINIMUMBALANCE") = MINIMUMBALANCE;
    t(L"CREDITLIMIT") = CREDITLIMIT;
    t(L"INTERESTRATE") = INTERESTRATE;
    t(L"PAYMENTDUEDATE") = PAYMENTDUEDATE;
    t(L"MINIMUMPAYMENT") = MINIMUMPAYMENT;
}

void AccountTable::Data::destroy()
{
    delete this;
}

// Return the data records as a json array string
wxString AccountTable::Data_Set::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);

    json_writer.StartArray();
    for (const auto & item: *this) {
        json_writer.StartObject();
        item.as_json(json_writer);
        json_writer.EndObject();
    }
    json_writer.EndArray();

    return json_buffer.GetString();
}

AccountTable::AccountTable() :
    fake_(new Data())
{
    m_table_name = "ACCOUNTLIST_V1";
    m_query_select = "SELECT ACCOUNTID, ACCOUNTNAME, ACCOUNTTYPE, ACCOUNTNUM, STATUS, NOTES, HELDAT, WEBSITE, CONTACTINFO, ACCESSINFO, INITIALBAL, INITIALDATE, FAVORITEACCT, CURRENCYID, STATEMENTLOCKED, STATEMENTDATE, MINIMUMBALANCE, CREDITLIMIT, INTERESTRATE, PAYMENTDUEDATE, MINIMUMPAYMENT FROM ACCOUNTLIST_V1 ";
}

// Destructor: clears any data records stored in memory
AccountTable::~AccountTable()
{
    delete this->fake_;
    destroy_cache();
}

// Remove all records stored in memory (cache) for the table
void AccountTable::destroy_cache()
{
    std::for_each(m_cache.begin(), m_cache.end(), std::mem_fn(&Data::destroy));
    m_cache.clear();
    m_cache_index.clear(); // no memory release since it just stores pointer and the according objects are in cache
}

// Creates the database table if the table does not exist
bool AccountTable::ensure_table()
{
    if (!table_exists()) {
        try {
            m_db->ExecuteUpdate("CREATE TABLE ACCOUNTLIST_V1(ACCOUNTID integer primary key, ACCOUNTNAME TEXT COLLATE NOCASE NOT NULL UNIQUE, ACCOUNTTYPE TEXT NOT NULL /* Cash, Checking, Term, Investment, Credit Card, Loan, Asset, Shares */, ACCOUNTNUM TEXT, STATUS TEXT NOT NULL /* Open, Closed */, NOTES TEXT, HELDAT TEXT, WEBSITE TEXT, CONTACTINFO TEXT, ACCESSINFO TEXT, INITIALBAL numeric, INITIALDATE TEXT, FAVORITEACCT TEXT NOT NULL, CURRENCYID integer NOT NULL, STATEMENTLOCKED integer, STATEMENTDATE TEXT, MINIMUMBALANCE numeric, CREDITLIMIT numeric, INTERESTRATE numeric, PAYMENTDUEDATE text, MINIMUMPAYMENT numeric)");
            this->ensure_data();
        }
        catch(const wxSQLite3Exception &e) {
            wxLogError("ACCOUNTLIST_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }
    }

    this->ensure_index();

    return true;
}

bool AccountTable::ensure_index()
{
    try {
        m_db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_ACCOUNTLIST_ACCOUNTTYPE ON ACCOUNTLIST_V1(ACCOUNTTYPE)");
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("ACCOUNTLIST_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

void AccountTable::ensure_data()
{
    m_db->Begin();
    m_db->Commit();
}

// Returns the column name as a string
wxString AccountTable::column_to_name(const AccountTable::COLUMN col)
{
    switch(col) {
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
        case COL_INITIALDATE: return "INITIALDATE";
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

// Returns the column number from the given column name
AccountTable::COLUMN AccountTable::name_to_column(const wxString& name)
{
    if (name == "ACCOUNTID") return COL_ACCOUNTID;
    else if (name == "ACCOUNTNAME") return COL_ACCOUNTNAME;
    else if (name == "ACCOUNTTYPE") return COL_ACCOUNTTYPE;
    else if (name == "ACCOUNTNUM") return COL_ACCOUNTNUM;
    else if (name == "STATUS") return COL_STATUS;
    else if (name == "NOTES") return COL_NOTES;
    else if (name == "HELDAT") return COL_HELDAT;
    else if (name == "WEBSITE") return COL_WEBSITE;
    else if (name == "CONTACTINFO") return COL_CONTACTINFO;
    else if (name == "ACCESSINFO") return COL_ACCESSINFO;
    else if (name == "INITIALBAL") return COL_INITIALBAL;
    else if (name == "INITIALDATE") return COL_INITIALDATE;
    else if (name == "FAVORITEACCT") return COL_FAVORITEACCT;
    else if (name == "CURRENCYID") return COL_CURRENCYID;
    else if (name == "STATEMENTLOCKED") return COL_STATEMENTLOCKED;
    else if (name == "STATEMENTDATE") return COL_STATEMENTDATE;
    else if (name == "MINIMUMBALANCE") return COL_MINIMUMBALANCE;
    else if (name == "CREDITLIMIT") return COL_CREDITLIMIT;
    else if (name == "INTERESTRATE") return COL_INTERESTRATE;
    else if (name == "PAYMENTDUEDATE") return COL_PAYMENTDUEDATE;
    else if (name == "MINIMUMPAYMENT") return COL_MINIMUMPAYMENT;

    return COLUMN(-1);
}

// Create a new Data record and add to memory table (cache)
AccountTable::Data* AccountTable::create()
{
    Data* entity = new Data();
    m_cache.push_back(entity);
    return entity;
}

// Create a copy of the Data record and add to memory table (cache)
AccountTable::Data* AccountTable::clone(const AccountTable::Data* e)
{
    Data* entity = create();
    *entity = *e;
    entity->id(-1);
    return entity;
}

// Save the Data record to the database table.
// Either create a new record or update the existing record.
// Remove old record from the memory table (cache).
bool AccountTable::save(AccountTable::Data* entity)
{
    wxString sql = wxEmptyString;
    if (entity->id() <= 0) {
        //  new & insert
        sql = "INSERT INTO ACCOUNTLIST_V1(ACCOUNTNAME, ACCOUNTTYPE, ACCOUNTNUM, STATUS, NOTES, HELDAT, WEBSITE, CONTACTINFO, ACCESSINFO, INITIALBAL, INITIALDATE, FAVORITEACCT, CURRENCYID, STATEMENTLOCKED, STATEMENTDATE, MINIMUMBALANCE, CREDITLIMIT, INTERESTRATE, PAYMENTDUEDATE, MINIMUMPAYMENT, ACCOUNTID) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    }
    else {
        sql = "UPDATE ACCOUNTLIST_V1 SET ACCOUNTNAME = ?, ACCOUNTTYPE = ?, ACCOUNTNUM = ?, STATUS = ?, NOTES = ?, HELDAT = ?, WEBSITE = ?, CONTACTINFO = ?, ACCESSINFO = ?, INITIALBAL = ?, INITIALDATE = ?, FAVORITEACCT = ?, CURRENCYID = ?, STATEMENTLOCKED = ?, STATEMENTDATE = ?, MINIMUMBALANCE = ?, CREDITLIMIT = ?, INTERESTRATE = ?, PAYMENTDUEDATE = ?, MINIMUMPAYMENT = ? WHERE ACCOUNTID = ?";
    }

    try {
        wxSQLite3Statement stmt = m_db->PrepareStatement(sql);

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
        stmt.Bind(11, entity->INITIALDATE);
        stmt.Bind(12, entity->FAVORITEACCT);
        stmt.Bind(13, entity->CURRENCYID);
        stmt.Bind(14, entity->STATEMENTLOCKED);
        stmt.Bind(15, entity->STATEMENTDATE);
        stmt.Bind(16, entity->MINIMUMBALANCE);
        stmt.Bind(17, entity->CREDITLIMIT);
        stmt.Bind(18, entity->INTERESTRATE);
        stmt.Bind(19, entity->PAYMENTDUEDATE);
        stmt.Bind(20, entity->MINIMUMPAYMENT);
        stmt.Bind(21, entity->id() > 0 ? entity->ACCOUNTID : newId());

        stmt.ExecuteUpdate();
        stmt.Finalize();

        if (entity->id() > 0) {
            // existent
            for (Cache::iterator it = m_cache.begin(); it != m_cache.end(); ++ it) {
                Data* e = *it;
                if (e->id() == entity->id())
                    // update in place
                    *e = *entity;
            }
        }
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("ACCOUNTLIST_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
        return false;
    }

    if (entity->id() <= 0) {
        entity->id(m_db->GetLastRowId());
        m_cache_index.insert(std::make_pair(entity->id(), entity));
    }
    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool AccountTable::remove(const int64 id)
{
    if (id <= 0) return false;
    try {
        wxString sql = "DELETE FROM ACCOUNTLIST_V1 WHERE ACCOUNTID = ?";
        wxSQLite3Statement stmt = m_db->PrepareStatement(sql);
        stmt.Bind(1, id);
        stmt.ExecuteUpdate();
        stmt.Finalize();

        Cache c;
        for (Cache::iterator it = m_cache.begin(); it != m_cache.end(); ++ it) {
            Data* entity = *it;
            if (entity->id() == id) {
                m_cache_index.erase(entity->id());
                delete entity;
            }
            else {
                c.push_back(entity);
            }
        }
        m_cache.clear();
        m_cache.swap(c);
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("ACCOUNTLIST_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool AccountTable::remove(AccountTable::Data* entity)
{
    if (remove(entity->id())) {
        entity->id(-1);
        return true;
    }

    return false;
}

// Search the memory table (Cache) for the data record.
// If not found in memory, search the database and update the cache.
AccountTable::Data* AccountTable::cache_id(const int64 id)
{
    if (id <= 0) {
        ++m_skip;
        return nullptr;
    }

    if (auto it = m_cache_index.find(id); it != m_cache_index.end()) {
        ++m_hit;
        return it->second;
    }

    ++m_miss;
    Data* entity = nullptr;
    wxString where = wxString::Format(" WHERE %s = ?", PRIMARY::name().utf8_str());
    try {
        wxSQLite3Statement stmt = m_db->PrepareStatement(this->m_query_select + where);
        stmt.Bind(1, id);

        wxSQLite3ResultSet q = stmt.ExecuteQuery();
        if(q.NextRow()) {
            entity = new Data(q);
            m_cache.push_back(entity);
            m_cache_index.insert(std::make_pair(id, entity));
        }
        stmt.Finalize();
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("%s: Exception %s", m_table_name.utf8_str(), e.GetMessage().utf8_str());
    }

    if (!entity) {
        entity = fake_;
        // wxLogError("%s: %d not found", m_table_name.utf8_str(), id);
    }

    return entity;
}

// Search the database for the data record, bypassing the cache.
AccountTable::Data* AccountTable::get_id(const int64 id)
{
    if (id <= 0) {
        ++m_skip;
        return nullptr;
    }

    Data* entity = nullptr;
    wxString where = wxString::Format(" WHERE %s = ?", PRIMARY::name().utf8_str());
    try {
        wxSQLite3Statement stmt = m_db->PrepareStatement(this->m_query_select + where);
        stmt.Bind(1, id);

        wxSQLite3ResultSet q = stmt.ExecuteQuery();
        if (q.NextRow()) {
            entity = new Data(q);
        }
        stmt.Finalize();
    }
    catch (const wxSQLite3Exception &e) {
        wxLogError("%s: Exception %s", m_table_name.utf8_str(), e.GetMessage().utf8_str());
    }

    if (!entity) {
        entity = fake_;
        // wxLogError("%s: %d not found", m_table_name.utf8_str(), id);
    }

    return entity;
}

// Return a list of Data records (Data_Set) derived directly from the database.
// The Data_Set is sorted based on the column number.
const AccountTable::Data_Set AccountTable::get_all(const COLUMN col, const bool asc)
{
    Data_Set result;
    try {
        wxSQLite3ResultSet q = m_db->ExecuteQuery(col == COLUMN(0) ? this->m_query_select
            : this->m_query_select + " ORDER BY " + column_to_name(col) + " COLLATE NOCASE " + (asc ? " ASC " : " DESC ")
        );

        while(q.NextRow()) {
            Data entity(q);
            result.push_back(std::move(entity));
        }

        q.Finalize();
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("%s: Exception %s", m_table_name.utf8_str(), e.GetMessage().utf8_str());
    }

    return result;
}
