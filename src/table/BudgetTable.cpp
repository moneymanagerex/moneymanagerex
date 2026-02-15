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

#include "BudgetTable.h"

BudgetTable::Data::Data()
{
    BUDGETENTRYID = -1;
    BUDGETYEARID = -1;
    CATEGID = -1;
    AMOUNT = 0.0;
    ACTIVE = -1;
}

BudgetTable::Data::Data(wxSQLite3ResultSet& q)
{
    BUDGETENTRYID = q.GetInt64(0);
    BUDGETYEARID = q.GetInt64(1);
    CATEGID = q.GetInt64(2);
    PERIOD = q.GetString(3);
    AMOUNT = q.GetDouble(4);
    NOTES = q.GetString(5);
    ACTIVE = q.GetInt64(6);
}

bool BudgetTable::Data::equals(const BudgetTable::Data* r) const
{
    if (BUDGETENTRYID != r->BUDGETENTRYID) return false;
    if (BUDGETYEARID != r->BUDGETYEARID) return false;
    if (CATEGID != r->CATEGID) return false;
    if (!PERIOD.IsSameAs(r->PERIOD)) return false;
    if (AMOUNT != r->AMOUNT) return false;
    if (!NOTES.IsSameAs(r->NOTES)) return false;
    if (ACTIVE != r->ACTIVE) return false;

    return true;
}

BudgetTable::Data& BudgetTable::Data::operator=(const BudgetTable::Data& other)
{
    if (this == &other) return *this;

    BUDGETENTRYID = other.BUDGETENTRYID;
    BUDGETYEARID = other.BUDGETYEARID;
    CATEGID = other.CATEGID;
    PERIOD = other.PERIOD;
    AMOUNT = other.AMOUNT;
    NOTES = other.NOTES;
    ACTIVE = other.ACTIVE;

    return *this;
}

// Return the data record as a json string
wxString BudgetTable::Data::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();			
    this->as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void BudgetTable::Data::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("BUDGETENTRYID");
    json_writer.Int64(this->BUDGETENTRYID.GetValue());
    json_writer.Key("BUDGETYEARID");
    json_writer.Int64(this->BUDGETYEARID.GetValue());
    json_writer.Key("CATEGID");
    json_writer.Int64(this->CATEGID.GetValue());
    json_writer.Key("PERIOD");
    json_writer.String(this->PERIOD.utf8_str());
    json_writer.Key("AMOUNT");
    json_writer.Double(this->AMOUNT);
    json_writer.Key("NOTES");
    json_writer.String(this->NOTES.utf8_str());
    json_writer.Key("ACTIVE");
    json_writer.Int64(this->ACTIVE.GetValue());
}

row_t BudgetTable::Data::to_row_t() const
{
    row_t row;

    row(L"BUDGETENTRYID") = BUDGETENTRYID.GetValue();
    row(L"BUDGETYEARID") = BUDGETYEARID.GetValue();
    row(L"CATEGID") = CATEGID.GetValue();
    row(L"PERIOD") = PERIOD;
    row(L"AMOUNT") = AMOUNT;
    row(L"NOTES") = NOTES;
    row(L"ACTIVE") = ACTIVE.GetValue();

    return row;
}

void BudgetTable::Data::to_template(html_template& t) const
{
    t(L"BUDGETENTRYID") = BUDGETENTRYID.GetValue();
    t(L"BUDGETYEARID") = BUDGETYEARID.GetValue();
    t(L"CATEGID") = CATEGID.GetValue();
    t(L"PERIOD") = PERIOD;
    t(L"AMOUNT") = AMOUNT;
    t(L"NOTES") = NOTES;
    t(L"ACTIVE") = ACTIVE.GetValue();
}

void BudgetTable::Data::destroy()
{
    delete this;
}

// Return the data records as a json array string
wxString BudgetTable::Data_Set::to_json() const
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

BudgetTable::BudgetTable() :
    fake_(new Data())
{
    m_table_name = "BUDGETTABLE_V1";
    m_query_select = "SELECT BUDGETENTRYID, BUDGETYEARID, CATEGID, PERIOD, AMOUNT, NOTES, ACTIVE FROM BUDGETTABLE_V1 ";
}

// Destructor: clears any data records stored in memory
BudgetTable::~BudgetTable()
{
    delete this->fake_;
    destroy_cache();
}

// Remove all records stored in memory (cache) for the table
void BudgetTable::destroy_cache()
{
    std::for_each(m_cache.begin(), m_cache.end(), std::mem_fn(&Data::destroy));
    m_cache.clear();
    m_cache_index.clear(); // no memory release since it just stores pointer and the according objects are in cache
}

// Creates the database table if the table does not exist
bool BudgetTable::ensure_table()
{
    if (!table_exists()) {
        try {
            m_db->ExecuteUpdate("CREATE TABLE BUDGETTABLE_V1(BUDGETENTRYID integer primary key, BUDGETYEARID integer, CATEGID integer, PERIOD TEXT NOT NULL /* None, Weekly, Bi-Weekly, Monthly, Monthly, Bi-Monthly, Quarterly, Half-Yearly, Yearly, Daily*/, AMOUNT numeric NOT NULL, NOTES TEXT, ACTIVE integer)");
            this->ensure_data();
        }
        catch(const wxSQLite3Exception &e) {
            wxLogError("BUDGETTABLE_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }
    }

    this->ensure_index();

    return true;
}

bool BudgetTable::ensure_index()
{
    try {
        m_db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_BUDGETTABLE_BUDGETYEARID ON BUDGETTABLE_V1(BUDGETYEARID)");
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("BUDGETTABLE_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

void BudgetTable::ensure_data()
{
    m_db->Begin();
    m_db->Commit();
}

// Returns the column name as a string
wxString BudgetTable::column_to_name(const BudgetTable::COLUMN col)
{
    switch(col) {
        case COL_BUDGETENTRYID: return "BUDGETENTRYID";
        case COL_BUDGETYEARID: return "BUDGETYEARID";
        case COL_CATEGID: return "CATEGID";
        case COL_PERIOD: return "PERIOD";
        case COL_AMOUNT: return "AMOUNT";
        case COL_NOTES: return "NOTES";
        case COL_ACTIVE: return "ACTIVE";
        default: break;
    }

    return "UNKNOWN";
}

// Returns the column number from the given column name
BudgetTable::COLUMN BudgetTable::name_to_column(const wxString& name)
{
    if (name == "BUDGETENTRYID") return COL_BUDGETENTRYID;
    else if (name == "BUDGETYEARID") return COL_BUDGETYEARID;
    else if (name == "CATEGID") return COL_CATEGID;
    else if (name == "PERIOD") return COL_PERIOD;
    else if (name == "AMOUNT") return COL_AMOUNT;
    else if (name == "NOTES") return COL_NOTES;
    else if (name == "ACTIVE") return COL_ACTIVE;

    return COLUMN(-1);
}

// Create a new Data record and add to memory table (cache)
BudgetTable::Data* BudgetTable::create()
{
    Data* entity = new Data();
    m_cache.push_back(entity);
    return entity;
}

// Create a copy of the Data record and add to memory table (cache)
BudgetTable::Data* BudgetTable::clone(const BudgetTable::Data* e)
{
    Data* entity = create();
    *entity = *e;
    entity->id(-1);
    return entity;
}

// Save the Data record to the database table.
// Either create a new record or update the existing record.
// Remove old record from the memory table (cache).
bool BudgetTable::save(BudgetTable::Data* entity)
{
    wxString sql = wxEmptyString;
    if (entity->id() <= 0) {
        //  new & insert
        sql = "INSERT INTO BUDGETTABLE_V1(BUDGETYEARID, CATEGID, PERIOD, AMOUNT, NOTES, ACTIVE, BUDGETENTRYID) VALUES(?, ?, ?, ?, ?, ?, ?)";
    }
    else {
        sql = "UPDATE BUDGETTABLE_V1 SET BUDGETYEARID = ?, CATEGID = ?, PERIOD = ?, AMOUNT = ?, NOTES = ?, ACTIVE = ? WHERE BUDGETENTRYID = ?";
    }

    try {
        wxSQLite3Statement stmt = m_db->PrepareStatement(sql);

        stmt.Bind(1, entity->BUDGETYEARID);
        stmt.Bind(2, entity->CATEGID);
        stmt.Bind(3, entity->PERIOD);
        stmt.Bind(4, entity->AMOUNT);
        stmt.Bind(5, entity->NOTES);
        stmt.Bind(6, entity->ACTIVE);
        stmt.Bind(7, entity->id() > 0 ? entity->BUDGETENTRYID : newId());

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
        wxLogError("BUDGETTABLE_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
        return false;
    }

    if (entity->id() <= 0) {
        entity->id(m_db->GetLastRowId());
        m_cache_index.insert(std::make_pair(entity->id(), entity));
    }
    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool BudgetTable::remove(const int64 id)
{
    if (id <= 0) return false;
    try {
        wxString sql = "DELETE FROM BUDGETTABLE_V1 WHERE BUDGETENTRYID = ?";
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
        wxLogError("BUDGETTABLE_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool BudgetTable::remove(BudgetTable::Data* entity)
{
    if (remove(entity->id())) {
        entity->id(-1);
        return true;
    }

    return false;
}

// Search the memory table (Cache) for the data record.
// If not found in memory, search the database and update the cache.
BudgetTable::Data* BudgetTable::cache_id(const int64 id)
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
BudgetTable::Data* BudgetTable::get_id(const int64 id)
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
const BudgetTable::Data_Set BudgetTable::get_all(const COLUMN col, const bool asc)
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
