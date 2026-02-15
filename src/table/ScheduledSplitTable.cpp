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

#include "ScheduledSplitTable.h"

ScheduledSplitTable::Data::Data()
{
    SPLITTRANSID = -1;
    TRANSID = -1;
    CATEGID = -1;
    SPLITTRANSAMOUNT = 0.0;
}

ScheduledSplitTable::Data::Data(wxSQLite3ResultSet& q)
{
    SPLITTRANSID = q.GetInt64(0);
    TRANSID = q.GetInt64(1);
    CATEGID = q.GetInt64(2);
    SPLITTRANSAMOUNT = q.GetDouble(3);
    NOTES = q.GetString(4);
}

bool ScheduledSplitTable::Data::equals(const ScheduledSplitTable::Data* r) const
{
    if (SPLITTRANSID != r->SPLITTRANSID) return false;
    if (TRANSID != r->TRANSID) return false;
    if (CATEGID != r->CATEGID) return false;
    if (SPLITTRANSAMOUNT != r->SPLITTRANSAMOUNT) return false;
    if (!NOTES.IsSameAs(r->NOTES)) return false;

    return true;
}

ScheduledSplitTable::Data& ScheduledSplitTable::Data::operator=(const ScheduledSplitTable::Data& other)
{
    if (this == &other) return *this;

    SPLITTRANSID = other.SPLITTRANSID;
    TRANSID = other.TRANSID;
    CATEGID = other.CATEGID;
    SPLITTRANSAMOUNT = other.SPLITTRANSAMOUNT;
    NOTES = other.NOTES;

    return *this;
}

// Return the data record as a json string
wxString ScheduledSplitTable::Data::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();			
    this->as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void ScheduledSplitTable::Data::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("SPLITTRANSID");
    json_writer.Int64(this->SPLITTRANSID.GetValue());
    json_writer.Key("TRANSID");
    json_writer.Int64(this->TRANSID.GetValue());
    json_writer.Key("CATEGID");
    json_writer.Int64(this->CATEGID.GetValue());
    json_writer.Key("SPLITTRANSAMOUNT");
    json_writer.Double(this->SPLITTRANSAMOUNT);
    json_writer.Key("NOTES");
    json_writer.String(this->NOTES.utf8_str());
}

row_t ScheduledSplitTable::Data::to_row_t() const
{
    row_t row;

    row(L"SPLITTRANSID") = SPLITTRANSID.GetValue();
    row(L"TRANSID") = TRANSID.GetValue();
    row(L"CATEGID") = CATEGID.GetValue();
    row(L"SPLITTRANSAMOUNT") = SPLITTRANSAMOUNT;
    row(L"NOTES") = NOTES;

    return row;
}

void ScheduledSplitTable::Data::to_template(html_template& t) const
{
    t(L"SPLITTRANSID") = SPLITTRANSID.GetValue();
    t(L"TRANSID") = TRANSID.GetValue();
    t(L"CATEGID") = CATEGID.GetValue();
    t(L"SPLITTRANSAMOUNT") = SPLITTRANSAMOUNT;
    t(L"NOTES") = NOTES;
}

void ScheduledSplitTable::Data::destroy()
{
    delete this;
}

// Return the data records as a json array string
wxString ScheduledSplitTable::Data_Set::to_json() const
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

ScheduledSplitTable::ScheduledSplitTable() :
    fake_(new Data())
{
    m_table_name = "BUDGETSPLITTRANSACTIONS_V1";
    m_query_select = "SELECT SPLITTRANSID, TRANSID, CATEGID, SPLITTRANSAMOUNT, NOTES FROM BUDGETSPLITTRANSACTIONS_V1 ";
}

// Destructor: clears any data records stored in memory
ScheduledSplitTable::~ScheduledSplitTable()
{
    delete this->fake_;
    destroy_cache();
}

// Remove all records stored in memory (cache) for the table
void ScheduledSplitTable::destroy_cache()
{
    std::for_each(m_cache.begin(), m_cache.end(), std::mem_fn(&Data::destroy));
    m_cache.clear();
    m_cache_index.clear(); // no memory release since it just stores pointer and the according objects are in cache
}

// Creates the database table if the table does not exist
bool ScheduledSplitTable::ensure_table()
{
    if (!table_exists()) {
        try {
            m_db->ExecuteUpdate("CREATE TABLE BUDGETSPLITTRANSACTIONS_V1(SPLITTRANSID integer primary key, TRANSID integer NOT NULL, CATEGID integer, SPLITTRANSAMOUNT numeric, NOTES TEXT)");
            this->ensure_data();
        }
        catch(const wxSQLite3Exception &e) {
            wxLogError("BUDGETSPLITTRANSACTIONS_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }
    }

    this->ensure_index();

    return true;
}

bool ScheduledSplitTable::ensure_index()
{
    try {
        m_db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_BUDGETSPLITTRANSACTIONS_TRANSID ON BUDGETSPLITTRANSACTIONS_V1(TRANSID)");
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("BUDGETSPLITTRANSACTIONS_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

void ScheduledSplitTable::ensure_data()
{
    m_db->Begin();
    m_db->Commit();
}

// Returns the column name as a string
wxString ScheduledSplitTable::column_to_name(const ScheduledSplitTable::COLUMN col)
{
    switch(col) {
        case COL_SPLITTRANSID: return "SPLITTRANSID";
        case COL_TRANSID: return "TRANSID";
        case COL_CATEGID: return "CATEGID";
        case COL_SPLITTRANSAMOUNT: return "SPLITTRANSAMOUNT";
        case COL_NOTES: return "NOTES";
        default: break;
    }

    return "UNKNOWN";
}

// Returns the column number from the given column name
ScheduledSplitTable::COLUMN ScheduledSplitTable::name_to_column(const wxString& name)
{
    if (name == "SPLITTRANSID") return COL_SPLITTRANSID;
    else if (name == "TRANSID") return COL_TRANSID;
    else if (name == "CATEGID") return COL_CATEGID;
    else if (name == "SPLITTRANSAMOUNT") return COL_SPLITTRANSAMOUNT;
    else if (name == "NOTES") return COL_NOTES;

    return COLUMN(-1);
}

// Create a new Data record and add to memory table (cache)
ScheduledSplitTable::Data* ScheduledSplitTable::create()
{
    Data* entity = new Data();
    m_cache.push_back(entity);
    return entity;
}

// Create a copy of the Data record and add to memory table (cache)
ScheduledSplitTable::Data* ScheduledSplitTable::clone(const ScheduledSplitTable::Data* e)
{
    Data* entity = create();
    *entity = *e;
    entity->id(-1);
    return entity;
}

// Save the Data record to the database table.
// Either create a new record or update the existing record.
// Remove old record from the memory table (cache).
bool ScheduledSplitTable::save(ScheduledSplitTable::Data* entity)
{
    wxString sql = wxEmptyString;
    if (entity->id() <= 0) {
        //  new & insert
        sql = "INSERT INTO BUDGETSPLITTRANSACTIONS_V1(TRANSID, CATEGID, SPLITTRANSAMOUNT, NOTES, SPLITTRANSID) VALUES(?, ?, ?, ?, ?)";
    }
    else {
        sql = "UPDATE BUDGETSPLITTRANSACTIONS_V1 SET TRANSID = ?, CATEGID = ?, SPLITTRANSAMOUNT = ?, NOTES = ? WHERE SPLITTRANSID = ?";
    }

    try {
        wxSQLite3Statement stmt = m_db->PrepareStatement(sql);

        stmt.Bind(1, entity->TRANSID);
        stmt.Bind(2, entity->CATEGID);
        stmt.Bind(3, entity->SPLITTRANSAMOUNT);
        stmt.Bind(4, entity->NOTES);
        stmt.Bind(5, entity->id() > 0 ? entity->SPLITTRANSID : newId());

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
        wxLogError("BUDGETSPLITTRANSACTIONS_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
        return false;
    }

    if (entity->id() <= 0) {
        entity->id(m_db->GetLastRowId());
        m_cache_index.insert(std::make_pair(entity->id(), entity));
    }
    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool ScheduledSplitTable::remove(const int64 id)
{
    if (id <= 0) return false;
    try {
        wxString sql = "DELETE FROM BUDGETSPLITTRANSACTIONS_V1 WHERE SPLITTRANSID = ?";
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
        wxLogError("BUDGETSPLITTRANSACTIONS_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool ScheduledSplitTable::remove(ScheduledSplitTable::Data* entity)
{
    if (remove(entity->id())) {
        entity->id(-1);
        return true;
    }

    return false;
}

// Search the memory table (Cache) for the data record.
// If not found in memory, search the database and update the cache.
ScheduledSplitTable::Data* ScheduledSplitTable::cache_id(const int64 id)
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
ScheduledSplitTable::Data* ScheduledSplitTable::get_id(const int64 id)
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
const ScheduledSplitTable::Data_Set ScheduledSplitTable::get_all(const COLUMN col, const bool asc)
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
