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

#include "AttachmentTable.h"

AttachmentTable::Data::Data()
{
    ATTACHMENTID = -1;
    REFID = -1;
}

AttachmentTable::Data::Data(wxSQLite3ResultSet& q)
{
    ATTACHMENTID = q.GetInt64(0);
    REFTYPE = q.GetString(1);
    REFID = q.GetInt64(2);
    DESCRIPTION = q.GetString(3);
    FILENAME = q.GetString(4);
}

bool AttachmentTable::Data::equals(const AttachmentTable::Data* r) const
{
    if (ATTACHMENTID != r->ATTACHMENTID) return false;
    if (!REFTYPE.IsSameAs(r->REFTYPE)) return false;
    if (REFID != r->REFID) return false;
    if (!DESCRIPTION.IsSameAs(r->DESCRIPTION)) return false;
    if (!FILENAME.IsSameAs(r->FILENAME)) return false;

    return true;
}

AttachmentTable::Data& AttachmentTable::Data::operator=(const AttachmentTable::Data& other)
{
    if (this == &other) return *this;

    ATTACHMENTID = other.ATTACHMENTID;
    REFTYPE = other.REFTYPE;
    REFID = other.REFID;
    DESCRIPTION = other.DESCRIPTION;
    FILENAME = other.FILENAME;

    return *this;
}

// Return the data record as a json string
wxString AttachmentTable::Data::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();			
    this->as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void AttachmentTable::Data::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("ATTACHMENTID");
    json_writer.Int64(this->ATTACHMENTID.GetValue());
    json_writer.Key("REFTYPE");
    json_writer.String(this->REFTYPE.utf8_str());
    json_writer.Key("REFID");
    json_writer.Int64(this->REFID.GetValue());
    json_writer.Key("DESCRIPTION");
    json_writer.String(this->DESCRIPTION.utf8_str());
    json_writer.Key("FILENAME");
    json_writer.String(this->FILENAME.utf8_str());
}

row_t AttachmentTable::Data::to_row_t() const
{
    row_t row;

    row(L"ATTACHMENTID") = ATTACHMENTID.GetValue();
    row(L"REFTYPE") = REFTYPE;
    row(L"REFID") = REFID.GetValue();
    row(L"DESCRIPTION") = DESCRIPTION;
    row(L"FILENAME") = FILENAME;

    return row;
}

void AttachmentTable::Data::to_template(html_template& t) const
{
    t(L"ATTACHMENTID") = ATTACHMENTID.GetValue();
    t(L"REFTYPE") = REFTYPE;
    t(L"REFID") = REFID.GetValue();
    t(L"DESCRIPTION") = DESCRIPTION;
    t(L"FILENAME") = FILENAME;
}

void AttachmentTable::Data::destroy()
{
    delete this;
}

// Return the data records as a json array string
wxString AttachmentTable::Data_Set::to_json() const
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

AttachmentTable::AttachmentTable() :
    fake_(new Data())
{
    m_table_name = "ATTACHMENT_V1";
    m_query_select = "SELECT ATTACHMENTID, REFTYPE, REFID, DESCRIPTION, FILENAME FROM ATTACHMENT_V1 ";
}

// Destructor: clears any data records stored in memory
AttachmentTable::~AttachmentTable()
{
    delete this->fake_;
    destroy_cache();
}

// Remove all records stored in memory (cache) for the table
void AttachmentTable::destroy_cache()
{
    std::for_each(m_cache.begin(), m_cache.end(), std::mem_fn(&Data::destroy));
    m_cache.clear();
    m_cache_index.clear(); // no memory release since it just stores pointer and the according objects are in cache
}

// Creates the database table if the table does not exist
bool AttachmentTable::ensure_table()
{
    if (!table_exists()) {
        try {
            m_db->ExecuteUpdate("CREATE TABLE ATTACHMENT_V1 (ATTACHMENTID INTEGER NOT NULL PRIMARY KEY, REFTYPE TEXT NOT NULL /* Transaction, Stock, Asset, Bank Account, Repeating Transaction, Payee */, REFID INTEGER NOT NULL, DESCRIPTION TEXT COLLATE NOCASE, FILENAME TEXT NOT NULL COLLATE NOCASE)");
            this->ensure_data();
        }
        catch(const wxSQLite3Exception &e) {
            wxLogError("ATTACHMENT_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }
    }

    this->ensure_index();

    return true;
}

bool AttachmentTable::ensure_index()
{
    try {
        m_db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_ATTACHMENT_REF ON ATTACHMENT_V1 (REFTYPE, REFID)");
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("ATTACHMENT_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

void AttachmentTable::ensure_data()
{
    m_db->Begin();
    m_db->Commit();
}

// Returns the column name as a string
wxString AttachmentTable::column_to_name(const AttachmentTable::COLUMN col)
{
    switch(col) {
        case COL_ATTACHMENTID: return "ATTACHMENTID";
        case COL_REFTYPE: return "REFTYPE";
        case COL_REFID: return "REFID";
        case COL_DESCRIPTION: return "DESCRIPTION";
        case COL_FILENAME: return "FILENAME";
        default: break;
    }

    return "UNKNOWN";
}

// Returns the column number from the given column name
AttachmentTable::COLUMN AttachmentTable::name_to_column(const wxString& name)
{
    if (name == "ATTACHMENTID") return COL_ATTACHMENTID;
    else if (name == "REFTYPE") return COL_REFTYPE;
    else if (name == "REFID") return COL_REFID;
    else if (name == "DESCRIPTION") return COL_DESCRIPTION;
    else if (name == "FILENAME") return COL_FILENAME;

    return COLUMN(-1);
}

// Create a new Data record and add to memory table (cache)
AttachmentTable::Data* AttachmentTable::create()
{
    Data* entity = new Data();
    m_cache.push_back(entity);
    return entity;
}

// Create a copy of the Data record and add to memory table (cache)
AttachmentTable::Data* AttachmentTable::clone(const AttachmentTable::Data* e)
{
    Data* entity = create();
    *entity = *e;
    entity->id(-1);
    return entity;
}

// Save the Data record to the database table.
// Either create a new record or update the existing record.
// Remove old record from the memory table (cache).
bool AttachmentTable::save(AttachmentTable::Data* entity)
{
    wxString sql = wxEmptyString;
    if (entity->id() <= 0) {
        //  new & insert
        sql = "INSERT INTO ATTACHMENT_V1(REFTYPE, REFID, DESCRIPTION, FILENAME, ATTACHMENTID) VALUES(?, ?, ?, ?, ?)";
    }
    else {
        sql = "UPDATE ATTACHMENT_V1 SET REFTYPE = ?, REFID = ?, DESCRIPTION = ?, FILENAME = ? WHERE ATTACHMENTID = ?";
    }

    try {
        wxSQLite3Statement stmt = m_db->PrepareStatement(sql);

        stmt.Bind(1, entity->REFTYPE);
        stmt.Bind(2, entity->REFID);
        stmt.Bind(3, entity->DESCRIPTION);
        stmt.Bind(4, entity->FILENAME);
        stmt.Bind(5, entity->id() > 0 ? entity->ATTACHMENTID : newId());

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
        wxLogError("ATTACHMENT_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
        return false;
    }

    if (entity->id() <= 0) {
        entity->id(m_db->GetLastRowId());
        m_cache_index.insert(std::make_pair(entity->id(), entity));
    }
    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool AttachmentTable::remove(const int64 id)
{
    if (id <= 0) return false;
    try {
        wxString sql = "DELETE FROM ATTACHMENT_V1 WHERE ATTACHMENTID = ?";
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
        wxLogError("ATTACHMENT_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool AttachmentTable::remove(AttachmentTable::Data* entity)
{
    if (remove(entity->id())) {
        entity->id(-1);
        return true;
    }

    return false;
}

// Search the memory table (Cache) for the data record.
// If not found in memory, search the database and update the cache.
AttachmentTable::Data* AttachmentTable::cache_id(const int64 id)
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
AttachmentTable::Data* AttachmentTable::get_id(const int64 id)
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
const AttachmentTable::Data_Set AttachmentTable::get_all(const COLUMN col, const bool asc)
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
