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

#include "UsageTable.h"

UsageTable::Data::Data()
{
    USAGEID = -1;
}

UsageTable::Data::Data(wxSQLite3ResultSet& q)
{
    USAGEID = q.GetInt64(0);
    USAGEDATE = q.GetString(1);
    JSONCONTENT = q.GetString(2);
}

bool UsageTable::Data::equals(const UsageTable::Data* r) const
{
    if (USAGEID != r->USAGEID) return false;
    if (!USAGEDATE.IsSameAs(r->USAGEDATE)) return false;
    if (!JSONCONTENT.IsSameAs(r->JSONCONTENT)) return false;

    return true;
}

UsageTable::Data& UsageTable::Data::operator=(const UsageTable::Data& other)
{
    if (this == &other) return *this;

    USAGEID = other.USAGEID;
    USAGEDATE = other.USAGEDATE;
    JSONCONTENT = other.JSONCONTENT;

    return *this;
}

// Return the data record as a json string
wxString UsageTable::Data::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();			
    this->as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void UsageTable::Data::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("USAGEID");
    json_writer.Int64(this->USAGEID.GetValue());
    json_writer.Key("USAGEDATE");
    json_writer.String(this->USAGEDATE.utf8_str());
    json_writer.Key("JSONCONTENT");
    json_writer.String(this->JSONCONTENT.utf8_str());
}

row_t UsageTable::Data::to_row_t() const
{
    row_t row;

    row(L"USAGEID") = USAGEID.GetValue();
    row(L"USAGEDATE") = USAGEDATE;
    row(L"JSONCONTENT") = JSONCONTENT;

    return row;
}

void UsageTable::Data::to_template(html_template& t) const
{
    t(L"USAGEID") = USAGEID.GetValue();
    t(L"USAGEDATE") = USAGEDATE;
    t(L"JSONCONTENT") = JSONCONTENT;
}

void UsageTable::Data::destroy()
{
    delete this;
}

// Return the data records as a json array string
wxString UsageTable::Data_Set::to_json() const
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

UsageTable::UsageTable() :
    fake_(new Data())
{
    m_table_name = "USAGE_V1";
    m_query_select = "SELECT USAGEID, USAGEDATE, JSONCONTENT FROM USAGE_V1 ";
}

// Destructor: clears any data records stored in memory
UsageTable::~UsageTable()
{
    delete this->fake_;
    destroy_cache();
}

// Remove all records stored in memory (cache) for the table
void UsageTable::destroy_cache()
{
    std::for_each(m_cache.begin(), m_cache.end(), std::mem_fn(&Data::destroy));
    m_cache.clear();
    m_cache_index.clear(); // no memory release since it just stores pointer and the according objects are in cache
}

// Creates the database table if the table does not exist
bool UsageTable::ensure_table()
{
    if (!table_exists()) {
        try {
            m_db->ExecuteUpdate("CREATE TABLE USAGE_V1 (USAGEID INTEGER NOT NULL PRIMARY KEY, USAGEDATE TEXT NOT NULL, JSONCONTENT TEXT NOT NULL)");
            this->ensure_data();
        }
        catch(const wxSQLite3Exception &e) {
            wxLogError("USAGE_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }
    }

    this->ensure_index();

    return true;
}

bool UsageTable::ensure_index()
{
    try {
        m_db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_USAGE_DATE ON USAGE_V1 (USAGEDATE)");
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("USAGE_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

void UsageTable::ensure_data()
{
    m_db->Begin();
    m_db->Commit();
}

// Returns the column name as a string
wxString UsageTable::column_to_name(const UsageTable::COLUMN col)
{
    switch(col) {
        case COL_USAGEID: return "USAGEID";
        case COL_USAGEDATE: return "USAGEDATE";
        case COL_JSONCONTENT: return "JSONCONTENT";
        default: break;
    }

    return "UNKNOWN";
}

// Returns the column number from the given column name
UsageTable::COLUMN UsageTable::name_to_column(const wxString& name)
{
    if (name == "USAGEID") return COL_USAGEID;
    else if (name == "USAGEDATE") return COL_USAGEDATE;
    else if (name == "JSONCONTENT") return COL_JSONCONTENT;

    return COLUMN(-1);
}

// Create a new Data record and add to memory table (cache)
UsageTable::Data* UsageTable::create()
{
    Data* entity = new Data();
    m_cache.push_back(entity);
    return entity;
}

// Create a copy of the Data record and add to memory table (cache)
UsageTable::Data* UsageTable::clone(const UsageTable::Data* e)
{
    Data* entity = create();
    *entity = *e;
    entity->id(-1);
    return entity;
}

// Save the Data record to the database table.
// Either create a new record or update the existing record.
// Remove old record from the memory table (cache).
bool UsageTable::save(UsageTable::Data* entity)
{
    wxString sql = wxEmptyString;
    if (entity->id() <= 0) {
        //  new & insert
        sql = "INSERT INTO USAGE_V1(USAGEDATE, JSONCONTENT, USAGEID) VALUES(?, ?, ?)";
    }
    else {
        sql = "UPDATE USAGE_V1 SET USAGEDATE = ?, JSONCONTENT = ? WHERE USAGEID = ?";
    }

    try {
        wxSQLite3Statement stmt = m_db->PrepareStatement(sql);

        stmt.Bind(1, entity->USAGEDATE);
        stmt.Bind(2, entity->JSONCONTENT);
        stmt.Bind(3, entity->id() > 0 ? entity->USAGEID : newId());

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
        wxLogError("USAGE_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
        return false;
    }

    if (entity->id() <= 0) {
        entity->id(m_db->GetLastRowId());
        m_cache_index.insert(std::make_pair(entity->id(), entity));
    }
    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool UsageTable::remove(const int64 id)
{
    if (id <= 0) return false;
    try {
        wxString sql = "DELETE FROM USAGE_V1 WHERE USAGEID = ?";
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
        wxLogError("USAGE_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool UsageTable::remove(UsageTable::Data* entity)
{
    if (remove(entity->id())) {
        entity->id(-1);
        return true;
    }

    return false;
}

// Search the memory table (Cache) for the data record.
// If not found in memory, search the database and update the cache.
UsageTable::Data* UsageTable::cache_id(const int64 id)
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
UsageTable::Data* UsageTable::get_id(const int64 id)
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
const UsageTable::Data_Set UsageTable::get_all(const COLUMN col, const bool asc)
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
