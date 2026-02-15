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

#include "TransactionLinkTable.h"

TransactionLinkTable::Data::Data()
{
    TRANSLINKID = -1;
    CHECKINGACCOUNTID = -1;
    LINKRECORDID = -1;
}

TransactionLinkTable::Data::Data(wxSQLite3ResultSet& q)
{
    TRANSLINKID = q.GetInt64(0);
    CHECKINGACCOUNTID = q.GetInt64(1);
    LINKTYPE = q.GetString(2);
    LINKRECORDID = q.GetInt64(3);
}

bool TransactionLinkTable::Data::equals(const TransactionLinkTable::Data* r) const
{
    if (TRANSLINKID != r->TRANSLINKID) return false;
    if (CHECKINGACCOUNTID != r->CHECKINGACCOUNTID) return false;
    if (!LINKTYPE.IsSameAs(r->LINKTYPE)) return false;
    if (LINKRECORDID != r->LINKRECORDID) return false;

    return true;
}

TransactionLinkTable::Data& TransactionLinkTable::Data::operator=(const TransactionLinkTable::Data& other)
{
    if (this == &other) return *this;

    TRANSLINKID = other.TRANSLINKID;
    CHECKINGACCOUNTID = other.CHECKINGACCOUNTID;
    LINKTYPE = other.LINKTYPE;
    LINKRECORDID = other.LINKRECORDID;

    return *this;
}

// Return the data record as a json string
wxString TransactionLinkTable::Data::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();			
    this->as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void TransactionLinkTable::Data::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("TRANSLINKID");
    json_writer.Int64(this->TRANSLINKID.GetValue());
    json_writer.Key("CHECKINGACCOUNTID");
    json_writer.Int64(this->CHECKINGACCOUNTID.GetValue());
    json_writer.Key("LINKTYPE");
    json_writer.String(this->LINKTYPE.utf8_str());
    json_writer.Key("LINKRECORDID");
    json_writer.Int64(this->LINKRECORDID.GetValue());
}

row_t TransactionLinkTable::Data::to_row_t() const
{
    row_t row;

    row(L"TRANSLINKID") = TRANSLINKID.GetValue();
    row(L"CHECKINGACCOUNTID") = CHECKINGACCOUNTID.GetValue();
    row(L"LINKTYPE") = LINKTYPE;
    row(L"LINKRECORDID") = LINKRECORDID.GetValue();

    return row;
}

void TransactionLinkTable::Data::to_template(html_template& t) const
{
    t(L"TRANSLINKID") = TRANSLINKID.GetValue();
    t(L"CHECKINGACCOUNTID") = CHECKINGACCOUNTID.GetValue();
    t(L"LINKTYPE") = LINKTYPE;
    t(L"LINKRECORDID") = LINKRECORDID.GetValue();
}

void TransactionLinkTable::Data::destroy()
{
    delete this;
}

// Return the data records as a json array string
wxString TransactionLinkTable::Data_Set::to_json() const
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

TransactionLinkTable::TransactionLinkTable() :
    fake_(new Data())
{
    m_table_name = "TRANSLINK_V1";
    m_query_select = "SELECT TRANSLINKID, CHECKINGACCOUNTID, LINKTYPE, LINKRECORDID FROM TRANSLINK_V1 ";
}

// Destructor: clears any data records stored in memory
TransactionLinkTable::~TransactionLinkTable()
{
    delete this->fake_;
    destroy_cache();
}

// Remove all records stored in memory (cache) for the table
void TransactionLinkTable::destroy_cache()
{
    std::for_each(m_cache.begin(), m_cache.end(), std::mem_fn(&Data::destroy));
    m_cache.clear();
    m_cache_index.clear(); // no memory release since it just stores pointer and the according objects are in cache
}

// Creates the database table if the table does not exist
bool TransactionLinkTable::ensure_table()
{
    if (!table_exists()) {
        try {
            m_db->ExecuteUpdate("CREATE TABLE TRANSLINK_V1 (TRANSLINKID  integer NOT NULL primary key, CHECKINGACCOUNTID integer NOT NULL, LINKTYPE TEXT NOT NULL /* Asset, Stock */, LINKRECORDID integer NOT NULL)");
            this->ensure_data();
        }
        catch(const wxSQLite3Exception &e) {
            wxLogError("TRANSLINK_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }
    }

    this->ensure_index();

    return true;
}

bool TransactionLinkTable::ensure_index()
{
    try {
        m_db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_CHECKINGACCOUNT ON TRANSLINK_V1 (CHECKINGACCOUNTID)");
        m_db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_LINKRECORD ON TRANSLINK_V1 (LINKTYPE, LINKRECORDID)");
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("TRANSLINK_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

void TransactionLinkTable::ensure_data()
{
    m_db->Begin();
    m_db->Commit();
}

// Returns the column name as a string
wxString TransactionLinkTable::column_to_name(const TransactionLinkTable::COLUMN col)
{
    switch(col) {
        case COL_TRANSLINKID: return "TRANSLINKID";
        case COL_CHECKINGACCOUNTID: return "CHECKINGACCOUNTID";
        case COL_LINKTYPE: return "LINKTYPE";
        case COL_LINKRECORDID: return "LINKRECORDID";
        default: break;
    }

    return "UNKNOWN";
}

// Returns the column number from the given column name
TransactionLinkTable::COLUMN TransactionLinkTable::name_to_column(const wxString& name)
{
    if (name == "TRANSLINKID") return COL_TRANSLINKID;
    else if (name == "CHECKINGACCOUNTID") return COL_CHECKINGACCOUNTID;
    else if (name == "LINKTYPE") return COL_LINKTYPE;
    else if (name == "LINKRECORDID") return COL_LINKRECORDID;

    return COLUMN(-1);
}

// Create a new Data record and add to memory table (cache)
TransactionLinkTable::Data* TransactionLinkTable::create()
{
    Data* entity = new Data();
    m_cache.push_back(entity);
    return entity;
}

// Create a copy of the Data record and add to memory table (cache)
TransactionLinkTable::Data* TransactionLinkTable::clone(const TransactionLinkTable::Data* e)
{
    Data* entity = create();
    *entity = *e;
    entity->id(-1);
    return entity;
}

// Save the Data record to the database table.
// Either create a new record or update the existing record.
// Remove old record from the memory table (cache).
bool TransactionLinkTable::save(TransactionLinkTable::Data* entity)
{
    wxString sql = wxEmptyString;
    if (entity->id() <= 0) {
        //  new & insert
        sql = "INSERT INTO TRANSLINK_V1(CHECKINGACCOUNTID, LINKTYPE, LINKRECORDID, TRANSLINKID) VALUES(?, ?, ?, ?)";
    }
    else {
        sql = "UPDATE TRANSLINK_V1 SET CHECKINGACCOUNTID = ?, LINKTYPE = ?, LINKRECORDID = ? WHERE TRANSLINKID = ?";
    }

    try {
        wxSQLite3Statement stmt = m_db->PrepareStatement(sql);

        stmt.Bind(1, entity->CHECKINGACCOUNTID);
        stmt.Bind(2, entity->LINKTYPE);
        stmt.Bind(3, entity->LINKRECORDID);
        stmt.Bind(4, entity->id() > 0 ? entity->TRANSLINKID : newId());

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
        wxLogError("TRANSLINK_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
        return false;
    }

    if (entity->id() <= 0) {
        entity->id(m_db->GetLastRowId());
        m_cache_index.insert(std::make_pair(entity->id(), entity));
    }
    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool TransactionLinkTable::remove(const int64 id)
{
    if (id <= 0) return false;
    try {
        wxString sql = "DELETE FROM TRANSLINK_V1 WHERE TRANSLINKID = ?";
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
        wxLogError("TRANSLINK_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool TransactionLinkTable::remove(TransactionLinkTable::Data* entity)
{
    if (remove(entity->id())) {
        entity->id(-1);
        return true;
    }

    return false;
}

// Search the memory table (Cache) for the data record.
// If not found in memory, search the database and update the cache.
TransactionLinkTable::Data* TransactionLinkTable::cache_id(const int64 id)
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
TransactionLinkTable::Data* TransactionLinkTable::get_id(const int64 id)
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
const TransactionLinkTable::Data_Set TransactionLinkTable::get_all(const COLUMN col, const bool asc)
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
