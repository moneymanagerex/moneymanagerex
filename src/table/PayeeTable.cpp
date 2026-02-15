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

#include "PayeeTable.h"

PayeeTable::Data::Data()
{
    PAYEEID = -1;
    CATEGID = -1;
    ACTIVE = -1;
}

PayeeTable::Data::Data(wxSQLite3ResultSet& q)
{
    PAYEEID = q.GetInt64(0);
    PAYEENAME = q.GetString(1);
    CATEGID = q.GetInt64(2);
    NUMBER = q.GetString(3);
    WEBSITE = q.GetString(4);
    NOTES = q.GetString(5);
    ACTIVE = q.GetInt64(6);
    PATTERN = q.GetString(7);
}

bool PayeeTable::Data::equals(const PayeeTable::Data* r) const
{
    if (PAYEEID != r->PAYEEID) return false;
    if (!PAYEENAME.IsSameAs(r->PAYEENAME)) return false;
    if (CATEGID != r->CATEGID) return false;
    if (!NUMBER.IsSameAs(r->NUMBER)) return false;
    if (!WEBSITE.IsSameAs(r->WEBSITE)) return false;
    if (!NOTES.IsSameAs(r->NOTES)) return false;
    if (ACTIVE != r->ACTIVE) return false;
    if (!PATTERN.IsSameAs(r->PATTERN)) return false;

    return true;
}

PayeeTable::Data& PayeeTable::Data::operator=(const PayeeTable::Data& other)
{
    if (this == &other) return *this;

    PAYEEID = other.PAYEEID;
    PAYEENAME = other.PAYEENAME;
    CATEGID = other.CATEGID;
    NUMBER = other.NUMBER;
    WEBSITE = other.WEBSITE;
    NOTES = other.NOTES;
    ACTIVE = other.ACTIVE;
    PATTERN = other.PATTERN;

    return *this;
}

// Return the data record as a json string
wxString PayeeTable::Data::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();			
    this->as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void PayeeTable::Data::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("PAYEEID");
    json_writer.Int64(this->PAYEEID.GetValue());
    json_writer.Key("PAYEENAME");
    json_writer.String(this->PAYEENAME.utf8_str());
    json_writer.Key("CATEGID");
    json_writer.Int64(this->CATEGID.GetValue());
    json_writer.Key("NUMBER");
    json_writer.String(this->NUMBER.utf8_str());
    json_writer.Key("WEBSITE");
    json_writer.String(this->WEBSITE.utf8_str());
    json_writer.Key("NOTES");
    json_writer.String(this->NOTES.utf8_str());
    json_writer.Key("ACTIVE");
    json_writer.Int64(this->ACTIVE.GetValue());
    json_writer.Key("PATTERN");
    json_writer.String(this->PATTERN.utf8_str());
}

row_t PayeeTable::Data::to_row_t() const
{
    row_t row;

    row(L"PAYEEID") = PAYEEID.GetValue();
    row(L"PAYEENAME") = PAYEENAME;
    row(L"CATEGID") = CATEGID.GetValue();
    row(L"NUMBER") = NUMBER;
    row(L"WEBSITE") = WEBSITE;
    row(L"NOTES") = NOTES;
    row(L"ACTIVE") = ACTIVE.GetValue();
    row(L"PATTERN") = PATTERN;

    return row;
}

void PayeeTable::Data::to_template(html_template& t) const
{
    t(L"PAYEEID") = PAYEEID.GetValue();
    t(L"PAYEENAME") = PAYEENAME;
    t(L"CATEGID") = CATEGID.GetValue();
    t(L"NUMBER") = NUMBER;
    t(L"WEBSITE") = WEBSITE;
    t(L"NOTES") = NOTES;
    t(L"ACTIVE") = ACTIVE.GetValue();
    t(L"PATTERN") = PATTERN;
}

void PayeeTable::Data::destroy()
{
    delete this;
}

// Return the data records as a json array string
wxString PayeeTable::Data_Set::to_json() const
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

PayeeTable::PayeeTable() :
    fake_(new Data())
{
    m_table_name = "PAYEE_V1";
    m_query_select = "SELECT PAYEEID, PAYEENAME, CATEGID, NUMBER, WEBSITE, NOTES, ACTIVE, PATTERN FROM PAYEE_V1 ";
}

// Destructor: clears any data records stored in memory
PayeeTable::~PayeeTable()
{
    delete this->fake_;
    destroy_cache();
}

// Remove all records stored in memory (cache) for the table
void PayeeTable::destroy_cache()
{
    std::for_each(m_cache.begin(), m_cache.end(), std::mem_fn(&Data::destroy));
    m_cache.clear();
    m_cache_index.clear(); // no memory release since it just stores pointer and the according objects are in cache
}

// Creates the database table if the table does not exist
bool PayeeTable::ensure_table()
{
    if (!table_exists()) {
        try {
            m_db->ExecuteUpdate("CREATE TABLE PAYEE_V1(PAYEEID integer primary key, PAYEENAME TEXT COLLATE NOCASE NOT NULL UNIQUE, CATEGID integer, NUMBER TEXT, WEBSITE TEXT, NOTES TEXT, ACTIVE integer, PATTERN TEXT DEFAULT '')");
            this->ensure_data();
        }
        catch(const wxSQLite3Exception &e) {
            wxLogError("PAYEE_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }
    }

    this->ensure_index();

    return true;
}

bool PayeeTable::ensure_index()
{
    try {
        m_db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_PAYEE_INFONAME ON PAYEE_V1(PAYEENAME)");
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("PAYEE_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

void PayeeTable::ensure_data()
{
    m_db->Begin();
    m_db->Commit();
}

// Returns the column name as a string
wxString PayeeTable::column_to_name(const PayeeTable::COLUMN col)
{
    switch(col) {
        case COL_PAYEEID: return "PAYEEID";
        case COL_PAYEENAME: return "PAYEENAME";
        case COL_CATEGID: return "CATEGID";
        case COL_NUMBER: return "NUMBER";
        case COL_WEBSITE: return "WEBSITE";
        case COL_NOTES: return "NOTES";
        case COL_ACTIVE: return "ACTIVE";
        case COL_PATTERN: return "PATTERN";
        default: break;
    }

    return "UNKNOWN";
}

// Returns the column number from the given column name
PayeeTable::COLUMN PayeeTable::name_to_column(const wxString& name)
{
    if (name == "PAYEEID") return COL_PAYEEID;
    else if (name == "PAYEENAME") return COL_PAYEENAME;
    else if (name == "CATEGID") return COL_CATEGID;
    else if (name == "NUMBER") return COL_NUMBER;
    else if (name == "WEBSITE") return COL_WEBSITE;
    else if (name == "NOTES") return COL_NOTES;
    else if (name == "ACTIVE") return COL_ACTIVE;
    else if (name == "PATTERN") return COL_PATTERN;

    return COLUMN(-1);
}

// Create a new Data record and add to memory table (cache)
PayeeTable::Data* PayeeTable::create()
{
    Data* entity = new Data();
    m_cache.push_back(entity);
    return entity;
}

// Create a copy of the Data record and add to memory table (cache)
PayeeTable::Data* PayeeTable::clone(const PayeeTable::Data* e)
{
    Data* entity = create();
    *entity = *e;
    entity->id(-1);
    return entity;
}

// Save the Data record to the database table.
// Either create a new record or update the existing record.
// Remove old record from the memory table (cache).
bool PayeeTable::save(PayeeTable::Data* entity)
{
    wxString sql = wxEmptyString;
    if (entity->id() <= 0) {
        //  new & insert
        sql = "INSERT INTO PAYEE_V1(PAYEENAME, CATEGID, NUMBER, WEBSITE, NOTES, ACTIVE, PATTERN, PAYEEID) VALUES(?, ?, ?, ?, ?, ?, ?, ?)";
    }
    else {
        sql = "UPDATE PAYEE_V1 SET PAYEENAME = ?, CATEGID = ?, NUMBER = ?, WEBSITE = ?, NOTES = ?, ACTIVE = ?, PATTERN = ? WHERE PAYEEID = ?";
    }

    try {
        wxSQLite3Statement stmt = m_db->PrepareStatement(sql);

        stmt.Bind(1, entity->PAYEENAME);
        stmt.Bind(2, entity->CATEGID);
        stmt.Bind(3, entity->NUMBER);
        stmt.Bind(4, entity->WEBSITE);
        stmt.Bind(5, entity->NOTES);
        stmt.Bind(6, entity->ACTIVE);
        stmt.Bind(7, entity->PATTERN);
        stmt.Bind(8, entity->id() > 0 ? entity->PAYEEID : newId());

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
        wxLogError("PAYEE_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
        return false;
    }

    if (entity->id() <= 0) {
        entity->id(m_db->GetLastRowId());
        m_cache_index.insert(std::make_pair(entity->id(), entity));
    }
    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool PayeeTable::remove(const int64 id)
{
    if (id <= 0) return false;
    try {
        wxString sql = "DELETE FROM PAYEE_V1 WHERE PAYEEID = ?";
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
        wxLogError("PAYEE_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool PayeeTable::remove(PayeeTable::Data* entity)
{
    if (remove(entity->id())) {
        entity->id(-1);
        return true;
    }

    return false;
}

// Search the memory table (Cache) for the data record.
// If not found in memory, search the database and update the cache.
PayeeTable::Data* PayeeTable::cache_id(const int64 id)
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
PayeeTable::Data* PayeeTable::get_id(const int64 id)
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
const PayeeTable::Data_Set PayeeTable::get_all(const COLUMN col, const bool asc)
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
