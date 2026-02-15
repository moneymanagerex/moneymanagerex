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

#include "TransactionTable.h"

TransactionTable::Data::Data()
{
    TRANSID = -1;
    ACCOUNTID = -1;
    TOACCOUNTID = -1;
    PAYEEID = -1;
    TRANSAMOUNT = 0.0;
    CATEGID = -1;
    FOLLOWUPID = -1;
    TOTRANSAMOUNT = 0.0;
    COLOR = -1;
}

TransactionTable::Data::Data(wxSQLite3ResultSet& q)
{
    TRANSID = q.GetInt64(0);
    ACCOUNTID = q.GetInt64(1);
    TOACCOUNTID = q.GetInt64(2);
    PAYEEID = q.GetInt64(3);
    TRANSCODE = q.GetString(4);
    TRANSAMOUNT = q.GetDouble(5);
    STATUS = q.GetString(6);
    TRANSACTIONNUMBER = q.GetString(7);
    NOTES = q.GetString(8);
    CATEGID = q.GetInt64(9);
    TRANSDATE = q.GetString(10);
    LASTUPDATEDTIME = q.GetString(11);
    DELETEDTIME = q.GetString(12);
    FOLLOWUPID = q.GetInt64(13);
    TOTRANSAMOUNT = q.GetDouble(14);
    COLOR = q.GetInt64(15);
}

bool TransactionTable::Data::equals(const TransactionTable::Data* r) const
{
    if (TRANSID != r->TRANSID) return false;
    if (ACCOUNTID != r->ACCOUNTID) return false;
    if (TOACCOUNTID != r->TOACCOUNTID) return false;
    if (PAYEEID != r->PAYEEID) return false;
    if (!TRANSCODE.IsSameAs(r->TRANSCODE)) return false;
    if (TRANSAMOUNT != r->TRANSAMOUNT) return false;
    if (!STATUS.IsSameAs(r->STATUS)) return false;
    if (!TRANSACTIONNUMBER.IsSameAs(r->TRANSACTIONNUMBER)) return false;
    if (!NOTES.IsSameAs(r->NOTES)) return false;
    if (CATEGID != r->CATEGID) return false;
    if (!TRANSDATE.IsSameAs(r->TRANSDATE)) return false;
    if (!LASTUPDATEDTIME.IsSameAs(r->LASTUPDATEDTIME)) return false;
    if (!DELETEDTIME.IsSameAs(r->DELETEDTIME)) return false;
    if (FOLLOWUPID != r->FOLLOWUPID) return false;
    if (TOTRANSAMOUNT != r->TOTRANSAMOUNT) return false;
    if (COLOR != r->COLOR) return false;

    return true;
}

TransactionTable::Data& TransactionTable::Data::operator=(const TransactionTable::Data& other)
{
    if (this == &other) return *this;

    TRANSID = other.TRANSID;
    ACCOUNTID = other.ACCOUNTID;
    TOACCOUNTID = other.TOACCOUNTID;
    PAYEEID = other.PAYEEID;
    TRANSCODE = other.TRANSCODE;
    TRANSAMOUNT = other.TRANSAMOUNT;
    STATUS = other.STATUS;
    TRANSACTIONNUMBER = other.TRANSACTIONNUMBER;
    NOTES = other.NOTES;
    CATEGID = other.CATEGID;
    TRANSDATE = other.TRANSDATE;
    LASTUPDATEDTIME = other.LASTUPDATEDTIME;
    DELETEDTIME = other.DELETEDTIME;
    FOLLOWUPID = other.FOLLOWUPID;
    TOTRANSAMOUNT = other.TOTRANSAMOUNT;
    COLOR = other.COLOR;

    return *this;
}

// Return the data record as a json string
wxString TransactionTable::Data::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();			
    this->as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void TransactionTable::Data::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("TRANSID");
    json_writer.Int64(this->TRANSID.GetValue());
    json_writer.Key("ACCOUNTID");
    json_writer.Int64(this->ACCOUNTID.GetValue());
    json_writer.Key("TOACCOUNTID");
    json_writer.Int64(this->TOACCOUNTID.GetValue());
    json_writer.Key("PAYEEID");
    json_writer.Int64(this->PAYEEID.GetValue());
    json_writer.Key("TRANSCODE");
    json_writer.String(this->TRANSCODE.utf8_str());
    json_writer.Key("TRANSAMOUNT");
    json_writer.Double(this->TRANSAMOUNT);
    json_writer.Key("STATUS");
    json_writer.String(this->STATUS.utf8_str());
    json_writer.Key("TRANSACTIONNUMBER");
    json_writer.String(this->TRANSACTIONNUMBER.utf8_str());
    json_writer.Key("NOTES");
    json_writer.String(this->NOTES.utf8_str());
    json_writer.Key("CATEGID");
    json_writer.Int64(this->CATEGID.GetValue());
    json_writer.Key("TRANSDATE");
    json_writer.String(this->TRANSDATE.utf8_str());
    json_writer.Key("LASTUPDATEDTIME");
    json_writer.String(this->LASTUPDATEDTIME.utf8_str());
    json_writer.Key("DELETEDTIME");
    json_writer.String(this->DELETEDTIME.utf8_str());
    json_writer.Key("FOLLOWUPID");
    json_writer.Int64(this->FOLLOWUPID.GetValue());
    json_writer.Key("TOTRANSAMOUNT");
    json_writer.Double(this->TOTRANSAMOUNT);
    json_writer.Key("COLOR");
    json_writer.Int64(this->COLOR.GetValue());
}

row_t TransactionTable::Data::to_row_t() const
{
    row_t row;

    row(L"TRANSID") = TRANSID.GetValue();
    row(L"ACCOUNTID") = ACCOUNTID.GetValue();
    row(L"TOACCOUNTID") = TOACCOUNTID.GetValue();
    row(L"PAYEEID") = PAYEEID.GetValue();
    row(L"TRANSCODE") = TRANSCODE;
    row(L"TRANSAMOUNT") = TRANSAMOUNT;
    row(L"STATUS") = STATUS;
    row(L"TRANSACTIONNUMBER") = TRANSACTIONNUMBER;
    row(L"NOTES") = NOTES;
    row(L"CATEGID") = CATEGID.GetValue();
    row(L"TRANSDATE") = TRANSDATE;
    row(L"LASTUPDATEDTIME") = LASTUPDATEDTIME;
    row(L"DELETEDTIME") = DELETEDTIME;
    row(L"FOLLOWUPID") = FOLLOWUPID.GetValue();
    row(L"TOTRANSAMOUNT") = TOTRANSAMOUNT;
    row(L"COLOR") = COLOR.GetValue();

    return row;
}

void TransactionTable::Data::to_template(html_template& t) const
{
    t(L"TRANSID") = TRANSID.GetValue();
    t(L"ACCOUNTID") = ACCOUNTID.GetValue();
    t(L"TOACCOUNTID") = TOACCOUNTID.GetValue();
    t(L"PAYEEID") = PAYEEID.GetValue();
    t(L"TRANSCODE") = TRANSCODE;
    t(L"TRANSAMOUNT") = TRANSAMOUNT;
    t(L"STATUS") = STATUS;
    t(L"TRANSACTIONNUMBER") = TRANSACTIONNUMBER;
    t(L"NOTES") = NOTES;
    t(L"CATEGID") = CATEGID.GetValue();
    t(L"TRANSDATE") = TRANSDATE;
    t(L"LASTUPDATEDTIME") = LASTUPDATEDTIME;
    t(L"DELETEDTIME") = DELETEDTIME;
    t(L"FOLLOWUPID") = FOLLOWUPID.GetValue();
    t(L"TOTRANSAMOUNT") = TOTRANSAMOUNT;
    t(L"COLOR") = COLOR.GetValue();
}

void TransactionTable::Data::destroy()
{
    delete this;
}

// Return the data records as a json array string
wxString TransactionTable::Data_Set::to_json() const
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

TransactionTable::TransactionTable() :
    fake_(new Data())
{
    m_table_name = "CHECKINGACCOUNT_V1";
    m_query_select = "SELECT TRANSID, ACCOUNTID, TOACCOUNTID, PAYEEID, TRANSCODE, TRANSAMOUNT, STATUS, TRANSACTIONNUMBER, NOTES, CATEGID, TRANSDATE, LASTUPDATEDTIME, DELETEDTIME, FOLLOWUPID, TOTRANSAMOUNT, COLOR FROM CHECKINGACCOUNT_V1 ";
}

// Destructor: clears any data records stored in memory
TransactionTable::~TransactionTable()
{
    delete this->fake_;
    destroy_cache();
}

// Remove all records stored in memory (cache) for the table
void TransactionTable::destroy_cache()
{
    std::for_each(m_cache.begin(), m_cache.end(), std::mem_fn(&Data::destroy));
    m_cache.clear();
    m_cache_index.clear(); // no memory release since it just stores pointer and the according objects are in cache
}

// Creates the database table if the table does not exist
bool TransactionTable::ensure_table()
{
    if (!table_exists()) {
        try {
            m_db->ExecuteUpdate("CREATE TABLE CHECKINGACCOUNT_V1(TRANSID integer primary key, ACCOUNTID integer NOT NULL, TOACCOUNTID integer, PAYEEID integer NOT NULL, TRANSCODE TEXT NOT NULL /* Withdrawal, Deposit, Transfer */, TRANSAMOUNT numeric NOT NULL, STATUS TEXT /* None, Reconciled, Void, Follow up, Duplicate */, TRANSACTIONNUMBER TEXT, NOTES TEXT, CATEGID integer, TRANSDATE TEXT, LASTUPDATEDTIME TEXT, DELETEDTIME TEXT, FOLLOWUPID integer, TOTRANSAMOUNT numeric, COLOR integer DEFAULT -1)");
            this->ensure_data();
        }
        catch(const wxSQLite3Exception &e) {
            wxLogError("CHECKINGACCOUNT_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }
    }

    this->ensure_index();

    return true;
}

bool TransactionTable::ensure_index()
{
    try {
        m_db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_CHECKINGACCOUNT_ACCOUNT ON CHECKINGACCOUNT_V1 (ACCOUNTID, TOACCOUNTID)");
        m_db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_CHECKINGACCOUNT_TRANSDATE ON CHECKINGACCOUNT_V1 (TRANSDATE)");
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("CHECKINGACCOUNT_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

void TransactionTable::ensure_data()
{
    m_db->Begin();
    m_db->Commit();
}

// Returns the column name as a string
wxString TransactionTable::column_to_name(const TransactionTable::COLUMN col)
{
    switch(col) {
        case COL_TRANSID: return "TRANSID";
        case COL_ACCOUNTID: return "ACCOUNTID";
        case COL_TOACCOUNTID: return "TOACCOUNTID";
        case COL_PAYEEID: return "PAYEEID";
        case COL_TRANSCODE: return "TRANSCODE";
        case COL_TRANSAMOUNT: return "TRANSAMOUNT";
        case COL_STATUS: return "STATUS";
        case COL_TRANSACTIONNUMBER: return "TRANSACTIONNUMBER";
        case COL_NOTES: return "NOTES";
        case COL_CATEGID: return "CATEGID";
        case COL_TRANSDATE: return "TRANSDATE";
        case COL_LASTUPDATEDTIME: return "LASTUPDATEDTIME";
        case COL_DELETEDTIME: return "DELETEDTIME";
        case COL_FOLLOWUPID: return "FOLLOWUPID";
        case COL_TOTRANSAMOUNT: return "TOTRANSAMOUNT";
        case COL_COLOR: return "COLOR";
        default: break;
    }

    return "UNKNOWN";
}

// Returns the column number from the given column name
TransactionTable::COLUMN TransactionTable::name_to_column(const wxString& name)
{
    if (name == "TRANSID") return COL_TRANSID;
    else if (name == "ACCOUNTID") return COL_ACCOUNTID;
    else if (name == "TOACCOUNTID") return COL_TOACCOUNTID;
    else if (name == "PAYEEID") return COL_PAYEEID;
    else if (name == "TRANSCODE") return COL_TRANSCODE;
    else if (name == "TRANSAMOUNT") return COL_TRANSAMOUNT;
    else if (name == "STATUS") return COL_STATUS;
    else if (name == "TRANSACTIONNUMBER") return COL_TRANSACTIONNUMBER;
    else if (name == "NOTES") return COL_NOTES;
    else if (name == "CATEGID") return COL_CATEGID;
    else if (name == "TRANSDATE") return COL_TRANSDATE;
    else if (name == "LASTUPDATEDTIME") return COL_LASTUPDATEDTIME;
    else if (name == "DELETEDTIME") return COL_DELETEDTIME;
    else if (name == "FOLLOWUPID") return COL_FOLLOWUPID;
    else if (name == "TOTRANSAMOUNT") return COL_TOTRANSAMOUNT;
    else if (name == "COLOR") return COL_COLOR;

    return COLUMN(-1);
}

// Create a new Data record and add to memory table (cache)
TransactionTable::Data* TransactionTable::create()
{
    Data* entity = new Data();
    m_cache.push_back(entity);
    return entity;
}

// Create a copy of the Data record and add to memory table (cache)
TransactionTable::Data* TransactionTable::clone(const TransactionTable::Data* e)
{
    Data* entity = create();
    *entity = *e;
    entity->id(-1);
    return entity;
}

// Save the Data record to the database table.
// Either create a new record or update the existing record.
// Remove old record from the memory table (cache).
bool TransactionTable::save(TransactionTable::Data* entity)
{
    wxString sql = wxEmptyString;
    if (entity->id() <= 0) {
        //  new & insert
        sql = "INSERT INTO CHECKINGACCOUNT_V1(ACCOUNTID, TOACCOUNTID, PAYEEID, TRANSCODE, TRANSAMOUNT, STATUS, TRANSACTIONNUMBER, NOTES, CATEGID, TRANSDATE, LASTUPDATEDTIME, DELETEDTIME, FOLLOWUPID, TOTRANSAMOUNT, COLOR, TRANSID) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    }
    else {
        sql = "UPDATE CHECKINGACCOUNT_V1 SET ACCOUNTID = ?, TOACCOUNTID = ?, PAYEEID = ?, TRANSCODE = ?, TRANSAMOUNT = ?, STATUS = ?, TRANSACTIONNUMBER = ?, NOTES = ?, CATEGID = ?, TRANSDATE = ?, LASTUPDATEDTIME = ?, DELETEDTIME = ?, FOLLOWUPID = ?, TOTRANSAMOUNT = ?, COLOR = ? WHERE TRANSID = ?";
    }

    try {
        wxSQLite3Statement stmt = m_db->PrepareStatement(sql);

        stmt.Bind(1, entity->ACCOUNTID);
        stmt.Bind(2, entity->TOACCOUNTID);
        stmt.Bind(3, entity->PAYEEID);
        stmt.Bind(4, entity->TRANSCODE);
        stmt.Bind(5, entity->TRANSAMOUNT);
        stmt.Bind(6, entity->STATUS);
        stmt.Bind(7, entity->TRANSACTIONNUMBER);
        stmt.Bind(8, entity->NOTES);
        stmt.Bind(9, entity->CATEGID);
        stmt.Bind(10, entity->TRANSDATE);
        stmt.Bind(11, entity->LASTUPDATEDTIME);
        stmt.Bind(12, entity->DELETEDTIME);
        stmt.Bind(13, entity->FOLLOWUPID);
        stmt.Bind(14, entity->TOTRANSAMOUNT);
        stmt.Bind(15, entity->COLOR);
        stmt.Bind(16, entity->id() > 0 ? entity->TRANSID : newId());

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
        wxLogError("CHECKINGACCOUNT_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
        return false;
    }

    if (entity->id() <= 0) {
        entity->id(m_db->GetLastRowId());
        m_cache_index.insert(std::make_pair(entity->id(), entity));
    }
    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool TransactionTable::remove(const int64 id)
{
    if (id <= 0) return false;
    try {
        wxString sql = "DELETE FROM CHECKINGACCOUNT_V1 WHERE TRANSID = ?";
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
        wxLogError("CHECKINGACCOUNT_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool TransactionTable::remove(TransactionTable::Data* entity)
{
    if (remove(entity->id())) {
        entity->id(-1);
        return true;
    }

    return false;
}

// Search the memory table (Cache) for the data record.
// If not found in memory, search the database and update the cache.
TransactionTable::Data* TransactionTable::cache_id(const int64 id)
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
TransactionTable::Data* TransactionTable::get_id(const int64 id)
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
const TransactionTable::Data_Set TransactionTable::get_all(const COLUMN col, const bool asc)
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
