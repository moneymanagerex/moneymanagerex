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

#include "FieldTable.h"

FieldTable::Data::Data()
{
    FIELDID = -1;
}

FieldTable::Data::Data(wxSQLite3ResultSet& q)
{
    FIELDID = q.GetInt64(0);
    REFTYPE = q.GetString(1);
    DESCRIPTION = q.GetString(2);
    TYPE = q.GetString(3);
    PROPERTIES = q.GetString(4);
}

bool FieldTable::Data::equals(const FieldTable::Data* r) const
{
    if (FIELDID != r->FIELDID) return false;
    if (!REFTYPE.IsSameAs(r->REFTYPE)) return false;
    if (!DESCRIPTION.IsSameAs(r->DESCRIPTION)) return false;
    if (!TYPE.IsSameAs(r->TYPE)) return false;
    if (!PROPERTIES.IsSameAs(r->PROPERTIES)) return false;

    return true;
}

FieldTable::Data& FieldTable::Data::operator=(const FieldTable::Data& other)
{
    if (this == &other) return *this;

    FIELDID = other.FIELDID;
    REFTYPE = other.REFTYPE;
    DESCRIPTION = other.DESCRIPTION;
    TYPE = other.TYPE;
    PROPERTIES = other.PROPERTIES;

    return *this;
}

// Return the data record as a json string
wxString FieldTable::Data::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();			
    this->as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void FieldTable::Data::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("FIELDID");
    json_writer.Int64(this->FIELDID.GetValue());
    json_writer.Key("REFTYPE");
    json_writer.String(this->REFTYPE.utf8_str());
    json_writer.Key("DESCRIPTION");
    json_writer.String(this->DESCRIPTION.utf8_str());
    json_writer.Key("TYPE");
    json_writer.String(this->TYPE.utf8_str());
    json_writer.Key("PROPERTIES");
    json_writer.String(this->PROPERTIES.utf8_str());
}

row_t FieldTable::Data::to_row_t() const
{
    row_t row;

    row(L"FIELDID") = FIELDID.GetValue();
    row(L"REFTYPE") = REFTYPE;
    row(L"DESCRIPTION") = DESCRIPTION;
    row(L"TYPE") = TYPE;
    row(L"PROPERTIES") = PROPERTIES;

    return row;
}

void FieldTable::Data::to_template(html_template& t) const
{
    t(L"FIELDID") = FIELDID.GetValue();
    t(L"REFTYPE") = REFTYPE;
    t(L"DESCRIPTION") = DESCRIPTION;
    t(L"TYPE") = TYPE;
    t(L"PROPERTIES") = PROPERTIES;
}

void FieldTable::Data::destroy()
{
    delete this;
}

// Return the data records as a json array string
wxString FieldTable::Data_Set::to_json() const
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

FieldTable::FieldTable() :
    fake_(new Data())
{
    m_table_name = "CUSTOMFIELD_V1";
    m_query_select = "SELECT FIELDID, REFTYPE, DESCRIPTION, TYPE, PROPERTIES FROM CUSTOMFIELD_V1 ";
}

// Destructor: clears any data records stored in memory
FieldTable::~FieldTable()
{
    delete this->fake_;
    destroy_cache();
}

// Remove all records stored in memory (cache) for the table
void FieldTable::destroy_cache()
{
    std::for_each(m_cache.begin(), m_cache.end(), std::mem_fn(&Data::destroy));
    m_cache.clear();
    m_cache_index.clear(); // no memory release since it just stores pointer and the according objects are in cache
}

// Creates the database table if the table does not exist
bool FieldTable::ensure_table()
{
    if (!table_exists()) {
        try {
            m_db->ExecuteUpdate("CREATE TABLE CUSTOMFIELD_V1 (FIELDID INTEGER NOT NULL PRIMARY KEY, REFTYPE TEXT NOT NULL /* Transaction, Stock, Asset, Bank Account, Repeating Transaction, Payee */, DESCRIPTION TEXT COLLATE NOCASE, TYPE TEXT NOT NULL /* String, Integer, Decimal, Boolean, Date, Time, SingleChoice, MultiChoice */, PROPERTIES TEXT NOT NULL)");
            this->ensure_data();
        }
        catch(const wxSQLite3Exception &e) {
            wxLogError("CUSTOMFIELD_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }
    }

    this->ensure_index();

    return true;
}

bool FieldTable::ensure_index()
{
    try {
        m_db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_CUSTOMFIELD_REF ON CUSTOMFIELD_V1 (REFTYPE)");
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("CUSTOMFIELD_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

void FieldTable::ensure_data()
{
    m_db->Begin();
    m_db->Commit();
}

// Returns the column name as a string
wxString FieldTable::column_to_name(const FieldTable::COLUMN col)
{
    switch(col) {
        case COL_FIELDID: return "FIELDID";
        case COL_REFTYPE: return "REFTYPE";
        case COL_DESCRIPTION: return "DESCRIPTION";
        case COL_TYPE: return "TYPE";
        case COL_PROPERTIES: return "PROPERTIES";
        default: break;
    }

    return "UNKNOWN";
}

// Returns the column number from the given column name
FieldTable::COLUMN FieldTable::name_to_column(const wxString& name)
{
    if (name == "FIELDID") return COL_FIELDID;
    else if (name == "REFTYPE") return COL_REFTYPE;
    else if (name == "DESCRIPTION") return COL_DESCRIPTION;
    else if (name == "TYPE") return COL_TYPE;
    else if (name == "PROPERTIES") return COL_PROPERTIES;

    return COLUMN(-1);
}

// Create a new Data record and add to memory table (cache)
FieldTable::Data* FieldTable::create()
{
    Data* entity = new Data();
    m_cache.push_back(entity);
    return entity;
}

// Create a copy of the Data record and add to memory table (cache)
FieldTable::Data* FieldTable::clone(const FieldTable::Data* e)
{
    Data* entity = create();
    *entity = *e;
    entity->id(-1);
    return entity;
}

// Save the Data record to the database table.
// Either create a new record or update the existing record.
// Remove old record from the memory table (cache).
bool FieldTable::save(FieldTable::Data* entity)
{
    wxString sql = wxEmptyString;
    if (entity->id() <= 0) {
        //  new & insert
        sql = "INSERT INTO CUSTOMFIELD_V1(REFTYPE, DESCRIPTION, TYPE, PROPERTIES, FIELDID) VALUES(?, ?, ?, ?, ?)";
    }
    else {
        sql = "UPDATE CUSTOMFIELD_V1 SET REFTYPE = ?, DESCRIPTION = ?, TYPE = ?, PROPERTIES = ? WHERE FIELDID = ?";
    }

    try {
        wxSQLite3Statement stmt = m_db->PrepareStatement(sql);

        stmt.Bind(1, entity->REFTYPE);
        stmt.Bind(2, entity->DESCRIPTION);
        stmt.Bind(3, entity->TYPE);
        stmt.Bind(4, entity->PROPERTIES);
        stmt.Bind(5, entity->id() > 0 ? entity->FIELDID : newId());

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
        wxLogError("CUSTOMFIELD_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
        return false;
    }

    if (entity->id() <= 0) {
        entity->id(m_db->GetLastRowId());
        m_cache_index.insert(std::make_pair(entity->id(), entity));
    }
    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool FieldTable::remove(const int64 id)
{
    if (id <= 0) return false;
    try {
        wxString sql = "DELETE FROM CUSTOMFIELD_V1 WHERE FIELDID = ?";
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
        wxLogError("CUSTOMFIELD_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool FieldTable::remove(FieldTable::Data* entity)
{
    if (remove(entity->id())) {
        entity->id(-1);
        return true;
    }

    return false;
}

// Search the memory table (Cache) for the data record.
// If not found in memory, search the database and update the cache.
FieldTable::Data* FieldTable::cache_id(const int64 id)
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
FieldTable::Data* FieldTable::get_id(const int64 id)
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
const FieldTable::Data_Set FieldTable::get_all(const COLUMN col, const bool asc)
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
