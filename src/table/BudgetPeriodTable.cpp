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

#include "BudgetPeriodTable.h"

BudgetPeriodTable::Data::Data()
{
    BUDGETYEARID = -1;
}

BudgetPeriodTable::Data::Data(wxSQLite3ResultSet& q)
{
    BUDGETYEARID = q.GetInt64(0);
    BUDGETYEARNAME = q.GetString(1);
}

bool BudgetPeriodTable::Data::equals(const BudgetPeriodTable::Data* r) const
{
    if (BUDGETYEARID != r->BUDGETYEARID) return false;
    if (!BUDGETYEARNAME.IsSameAs(r->BUDGETYEARNAME)) return false;

    return true;
}

BudgetPeriodTable::Data& BudgetPeriodTable::Data::operator=(const BudgetPeriodTable::Data& other)
{
    if (this == &other) return *this;

    BUDGETYEARID = other.BUDGETYEARID;
    BUDGETYEARNAME = other.BUDGETYEARNAME;

    return *this;
}

// Return the data record as a json string
wxString BudgetPeriodTable::Data::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();			
    this->as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void BudgetPeriodTable::Data::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("BUDGETYEARID");
    json_writer.Int64(this->BUDGETYEARID.GetValue());
    json_writer.Key("BUDGETYEARNAME");
    json_writer.String(this->BUDGETYEARNAME.utf8_str());
}

row_t BudgetPeriodTable::Data::to_row_t() const
{
    row_t row;

    row(L"BUDGETYEARID") = BUDGETYEARID.GetValue();
    row(L"BUDGETYEARNAME") = BUDGETYEARNAME;

    return row;
}

void BudgetPeriodTable::Data::to_template(html_template& t) const
{
    t(L"BUDGETYEARID") = BUDGETYEARID.GetValue();
    t(L"BUDGETYEARNAME") = BUDGETYEARNAME;
}

void BudgetPeriodTable::Data::destroy()
{
    delete this;
}

// Return the data records as a json array string
wxString BudgetPeriodTable::Data_Set::to_json() const
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

BudgetPeriodTable::BudgetPeriodTable() :
    fake_(new Data())
{
    m_table_name = "BUDGETYEAR_V1";
    m_query_select = "SELECT BUDGETYEARID, BUDGETYEARNAME FROM BUDGETYEAR_V1 ";
}

// Destructor: clears any data records stored in memory
BudgetPeriodTable::~BudgetPeriodTable()
{
    delete this->fake_;
    destroy_cache();
}

// Remove all records stored in memory (cache) for the table
void BudgetPeriodTable::destroy_cache()
{
    std::for_each(m_cache.begin(), m_cache.end(), std::mem_fn(&Data::destroy));
    m_cache.clear();
    m_cache_index.clear(); // no memory release since it just stores pointer and the according objects are in cache
}

// Creates the database table if the table does not exist
bool BudgetPeriodTable::ensure_table()
{
    if (!table_exists()) {
        try {
            m_db->ExecuteUpdate("CREATE TABLE BUDGETYEAR_V1(BUDGETYEARID integer primary key, BUDGETYEARNAME TEXT NOT NULL UNIQUE)");
            this->ensure_data();
        }
        catch(const wxSQLite3Exception &e) {
            wxLogError("BUDGETYEAR_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }
    }

    this->ensure_index();

    return true;
}

bool BudgetPeriodTable::ensure_index()
{
    try {
        m_db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_BUDGETYEAR_BUDGETYEARNAME ON BUDGETYEAR_V1(BUDGETYEARNAME)");
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("BUDGETYEAR_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

void BudgetPeriodTable::ensure_data()
{
    m_db->Begin();
    m_db->Commit();
}

// Returns the column name as a string
wxString BudgetPeriodTable::column_to_name(const BudgetPeriodTable::COLUMN col)
{
    switch(col) {
        case COL_BUDGETYEARID: return "BUDGETYEARID";
        case COL_BUDGETYEARNAME: return "BUDGETYEARNAME";
        default: break;
    }

    return "UNKNOWN";
}

// Returns the column number from the given column name
BudgetPeriodTable::COLUMN BudgetPeriodTable::name_to_column(const wxString& name)
{
    if (name == "BUDGETYEARID") return COL_BUDGETYEARID;
    else if (name == "BUDGETYEARNAME") return COL_BUDGETYEARNAME;

    return COLUMN(-1);
}

// Create a new Data record and add to memory table (cache)
BudgetPeriodTable::Data* BudgetPeriodTable::create()
{
    Data* entity = new Data();
    m_cache.push_back(entity);
    return entity;
}

// Create a copy of the Data record and add to memory table (cache)
BudgetPeriodTable::Data* BudgetPeriodTable::clone(const BudgetPeriodTable::Data* e)
{
    Data* entity = create();
    *entity = *e;
    entity->id(-1);
    return entity;
}

// Save the Data record to the database table.
// Either create a new record or update the existing record.
// Remove old record from the memory table (cache).
bool BudgetPeriodTable::save(BudgetPeriodTable::Data* entity)
{
    wxString sql = wxEmptyString;
    if (entity->id() <= 0) {
        //  new & insert
        sql = "INSERT INTO BUDGETYEAR_V1(BUDGETYEARNAME, BUDGETYEARID) VALUES(?, ?)";
    }
    else {
        sql = "UPDATE BUDGETYEAR_V1 SET BUDGETYEARNAME = ? WHERE BUDGETYEARID = ?";
    }

    try {
        wxSQLite3Statement stmt = m_db->PrepareStatement(sql);

        stmt.Bind(1, entity->BUDGETYEARNAME);
        stmt.Bind(2, entity->id() > 0 ? entity->BUDGETYEARID : newId());

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
        wxLogError("BUDGETYEAR_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
        return false;
    }

    if (entity->id() <= 0) {
        entity->id(m_db->GetLastRowId());
        m_cache_index.insert(std::make_pair(entity->id(), entity));
    }
    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool BudgetPeriodTable::remove(const int64 id)
{
    if (id <= 0) return false;
    try {
        wxString sql = "DELETE FROM BUDGETYEAR_V1 WHERE BUDGETYEARID = ?";
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
        wxLogError("BUDGETYEAR_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool BudgetPeriodTable::remove(BudgetPeriodTable::Data* entity)
{
    if (remove(entity->id())) {
        entity->id(-1);
        return true;
    }

    return false;
}

// Search the memory table (Cache) for the data record.
// If not found in memory, search the database and update the cache.
BudgetPeriodTable::Data* BudgetPeriodTable::cache_id(const int64 id)
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
BudgetPeriodTable::Data* BudgetPeriodTable::get_id(const int64 id)
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
const BudgetPeriodTable::Data_Set BudgetPeriodTable::get_all(const COLUMN col, const bool asc)
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
