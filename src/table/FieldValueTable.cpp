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

#include "FieldValueTable.h"

FieldValueTable::Data::Data()
{
    FIELDATADID = -1;
    FIELDID = -1;
    REFID = -1;
}

FieldValueTable::Data::Data(wxSQLite3ResultSet& q)
{
    FIELDATADID = q.GetInt64(0);
    FIELDID = q.GetInt64(1);
    REFID = q.GetInt64(2);
    CONTENT = q.GetString(3);
}

bool FieldValueTable::Data::equals(const FieldValueTable::Data* r) const
{
    if (FIELDATADID != r->FIELDATADID) return false;
    if (FIELDID != r->FIELDID) return false;
    if (REFID != r->REFID) return false;
    if (!CONTENT.IsSameAs(r->CONTENT)) return false;

    return true;
}

FieldValueTable::Data& FieldValueTable::Data::operator=(const FieldValueTable::Data& other)
{
    if (this == &other) return *this;

    FIELDATADID = other.FIELDATADID;
    FIELDID = other.FIELDID;
    REFID = other.REFID;
    CONTENT = other.CONTENT;

    return *this;
}

// Return the data record as a json string
wxString FieldValueTable::Data::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();			
    this->as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void FieldValueTable::Data::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("FIELDATADID");
    json_writer.Int64(this->FIELDATADID.GetValue());
    json_writer.Key("FIELDID");
    json_writer.Int64(this->FIELDID.GetValue());
    json_writer.Key("REFID");
    json_writer.Int64(this->REFID.GetValue());
    json_writer.Key("CONTENT");
    json_writer.String(this->CONTENT.utf8_str());
}

row_t FieldValueTable::Data::to_row_t() const
{
    row_t row;

    row(L"FIELDATADID") = FIELDATADID.GetValue();
    row(L"FIELDID") = FIELDID.GetValue();
    row(L"REFID") = REFID.GetValue();
    row(L"CONTENT") = CONTENT;

    return row;
}

void FieldValueTable::Data::to_template(html_template& t) const
{
    t(L"FIELDATADID") = FIELDATADID.GetValue();
    t(L"FIELDID") = FIELDID.GetValue();
    t(L"REFID") = REFID.GetValue();
    t(L"CONTENT") = CONTENT;
}

void FieldValueTable::Data::destroy()
{
    delete this;
}

// Return the data records as a json array string
wxString FieldValueTable::Data_Set::to_json() const
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

FieldValueTable::FieldValueTable() :
    fake_(new Data())
{
    m_table_name = "CUSTOMFIELDDATA_V1";
    m_query_select = "SELECT FIELDATADID, FIELDID, REFID, CONTENT FROM CUSTOMFIELDDATA_V1 ";
}

// Destructor: clears any data records stored in memory
FieldValueTable::~FieldValueTable()
{
    delete this->fake_;
    destroy_cache();
}

// Remove all records stored in memory (cache) for the table
void FieldValueTable::destroy_cache()
{
    std::for_each(m_cache.begin(), m_cache.end(), std::mem_fn(&Data::destroy));
    m_cache.clear();
    m_cache_index.clear(); // no memory release since it just stores pointer and the according objects are in cache
}

// Creates the database table if the table does not exist
bool FieldValueTable::ensure_table()
{
    if (!table_exists()) {
        try {
            m_db->ExecuteUpdate("CREATE TABLE CUSTOMFIELDDATA_V1 (FIELDATADID INTEGER NOT NULL PRIMARY KEY, FIELDID INTEGER NOT NULL, REFID INTEGER NOT NULL, CONTENT TEXT, UNIQUE(FIELDID, REFID))");
            this->ensure_data();
        }
        catch(const wxSQLite3Exception &e) {
            wxLogError("CUSTOMFIELDDATA_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }
    }

    this->ensure_index();

    return true;
}

bool FieldValueTable::ensure_index()
{
    try {
        m_db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_CUSTOMFIELDDATA_REF ON CUSTOMFIELDDATA_V1 (FIELDID, REFID)");
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("CUSTOMFIELDDATA_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

void FieldValueTable::ensure_data()
{
    m_db->Begin();
    m_db->Commit();
}

// Returns the column name as a string
wxString FieldValueTable::column_to_name(const FieldValueTable::COLUMN col)
{
    switch(col) {
        case COL_FIELDATADID: return "FIELDATADID";
        case COL_FIELDID: return "FIELDID";
        case COL_REFID: return "REFID";
        case COL_CONTENT: return "CONTENT";
        default: break;
    }

    return "UNKNOWN";
}

// Returns the column number from the given column name
FieldValueTable::COLUMN FieldValueTable::name_to_column(const wxString& name)
{
    if (name == "FIELDATADID") return COL_FIELDATADID;
    else if (name == "FIELDID") return COL_FIELDID;
    else if (name == "REFID") return COL_REFID;
    else if (name == "CONTENT") return COL_CONTENT;

    return COLUMN(-1);
}

// Create a new Data record and add to memory table (cache)
FieldValueTable::Data* FieldValueTable::create()
{
    Data* entity = new Data();
    m_cache.push_back(entity);
    return entity;
}

// Create a copy of the Data record and add to memory table (cache)
FieldValueTable::Data* FieldValueTable::clone(const FieldValueTable::Data* e)
{
    Data* entity = create();
    *entity = *e;
    entity->id(-1);
    return entity;
}

// Save the Data record to the database table.
// Either create a new record or update the existing record.
// Remove old record from the memory table (cache).
bool FieldValueTable::save(FieldValueTable::Data* entity)
{
    wxString sql = wxEmptyString;
    if (entity->id() <= 0) {
        //  new & insert
        sql = "INSERT INTO CUSTOMFIELDDATA_V1(FIELDID, REFID, CONTENT, FIELDATADID) VALUES(?, ?, ?, ?)";
    }
    else {
        sql = "UPDATE CUSTOMFIELDDATA_V1 SET FIELDID = ?, REFID = ?, CONTENT = ? WHERE FIELDATADID = ?";
    }

    try {
        wxSQLite3Statement stmt = m_db->PrepareStatement(sql);

        stmt.Bind(1, entity->FIELDID);
        stmt.Bind(2, entity->REFID);
        stmt.Bind(3, entity->CONTENT);
        stmt.Bind(4, entity->id() > 0 ? entity->FIELDATADID : newId());

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
        wxLogError("CUSTOMFIELDDATA_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
        return false;
    }

    if (entity->id() <= 0) {
        entity->id(m_db->GetLastRowId());
        m_cache_index.insert(std::make_pair(entity->id(), entity));
    }
    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool FieldValueTable::remove(const int64 id)
{
    if (id <= 0) return false;
    try {
        wxString sql = "DELETE FROM CUSTOMFIELDDATA_V1 WHERE FIELDATADID = ?";
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
        wxLogError("CUSTOMFIELDDATA_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool FieldValueTable::remove(FieldValueTable::Data* entity)
{
    if (remove(entity->id())) {
        entity->id(-1);
        return true;
    }

    return false;
}

// Search the memory table (Cache) for the data record.
// If not found in memory, search the database and update the cache.
FieldValueTable::Data* FieldValueTable::cache_id(const int64 id)
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
FieldValueTable::Data* FieldValueTable::get_id(const int64 id)
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
const FieldValueTable::Data_Set FieldValueTable::get_all(const COLUMN col, const bool asc)
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
