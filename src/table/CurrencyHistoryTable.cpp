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

#include "CurrencyHistoryTable.h"

CurrencyHistoryTable::Data::Data()
{
    CURRHISTID = -1;
    CURRENCYID = -1;
    CURRVALUE = 0.0;
    CURRUPDTYPE = -1;
}

CurrencyHistoryTable::Data::Data(wxSQLite3ResultSet& q)
{
    CURRHISTID = q.GetInt64(0);
    CURRENCYID = q.GetInt64(1);
    CURRDATE = q.GetString(2);
    CURRVALUE = q.GetDouble(3);
    CURRUPDTYPE = q.GetInt64(4);
}

bool CurrencyHistoryTable::Data::equals(const CurrencyHistoryTable::Data* r) const
{
    if (CURRHISTID != r->CURRHISTID) return false;
    if (CURRENCYID != r->CURRENCYID) return false;
    if (!CURRDATE.IsSameAs(r->CURRDATE)) return false;
    if (CURRVALUE != r->CURRVALUE) return false;
    if (CURRUPDTYPE != r->CURRUPDTYPE) return false;

    return true;
}

CurrencyHistoryTable::Data& CurrencyHistoryTable::Data::operator=(const CurrencyHistoryTable::Data& other)
{
    if (this == &other) return *this;

    CURRHISTID = other.CURRHISTID;
    CURRENCYID = other.CURRENCYID;
    CURRDATE = other.CURRDATE;
    CURRVALUE = other.CURRVALUE;
    CURRUPDTYPE = other.CURRUPDTYPE;

    return *this;
}

// Return the data record as a json string
wxString CurrencyHistoryTable::Data::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();			
    this->as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void CurrencyHistoryTable::Data::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("CURRHISTID");
    json_writer.Int64(this->CURRHISTID.GetValue());
    json_writer.Key("CURRENCYID");
    json_writer.Int64(this->CURRENCYID.GetValue());
    json_writer.Key("CURRDATE");
    json_writer.String(this->CURRDATE.utf8_str());
    json_writer.Key("CURRVALUE");
    json_writer.Double(this->CURRVALUE);
    json_writer.Key("CURRUPDTYPE");
    json_writer.Int64(this->CURRUPDTYPE.GetValue());
}

row_t CurrencyHistoryTable::Data::to_row_t() const
{
    row_t row;

    row(L"CURRHISTID") = CURRHISTID.GetValue();
    row(L"CURRENCYID") = CURRENCYID.GetValue();
    row(L"CURRDATE") = CURRDATE;
    row(L"CURRVALUE") = CURRVALUE;
    row(L"CURRUPDTYPE") = CURRUPDTYPE.GetValue();

    return row;
}

void CurrencyHistoryTable::Data::to_template(html_template& t) const
{
    t(L"CURRHISTID") = CURRHISTID.GetValue();
    t(L"CURRENCYID") = CURRENCYID.GetValue();
    t(L"CURRDATE") = CURRDATE;
    t(L"CURRVALUE") = CURRVALUE;
    t(L"CURRUPDTYPE") = CURRUPDTYPE.GetValue();
}

void CurrencyHistoryTable::Data::destroy()
{
    delete this;
}

// Return the data records as a json array string
wxString CurrencyHistoryTable::Data_Set::to_json() const
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

CurrencyHistoryTable::CurrencyHistoryTable() :
    fake_(new Data())
{
    m_table_name = "CURRENCYHISTORY_V1";
    m_query_select = "SELECT CURRHISTID, CURRENCYID, CURRDATE, CURRVALUE, CURRUPDTYPE FROM CURRENCYHISTORY_V1 ";
}

// Destructor: clears any data records stored in memory
CurrencyHistoryTable::~CurrencyHistoryTable()
{
    delete this->fake_;
    destroy_cache();
}

// Remove all records stored in memory (cache) for the table
void CurrencyHistoryTable::destroy_cache()
{
    std::for_each(m_cache.begin(), m_cache.end(), std::mem_fn(&Data::destroy));
    m_cache.clear();
    m_cache_index.clear(); // no memory release since it just stores pointer and the according objects are in cache
}

// Creates the database table if the table does not exist
bool CurrencyHistoryTable::ensure_table()
{
    if (!table_exists()) {
        try {
            m_db->ExecuteUpdate("CREATE TABLE CURRENCYHISTORY_V1(CURRHISTID INTEGER PRIMARY KEY, CURRENCYID INTEGER NOT NULL, CURRDATE TEXT NOT NULL, CURRVALUE NUMERIC NOT NULL, CURRUPDTYPE INTEGER, UNIQUE(CURRENCYID, CURRDATE))");
            this->ensure_data();
        }
        catch(const wxSQLite3Exception &e) {
            wxLogError("CURRENCYHISTORY_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }
    }

    this->ensure_index();

    return true;
}

bool CurrencyHistoryTable::ensure_index()
{
    try {
        m_db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_CURRENCYHISTORY_CURRENCYID_CURRDATE ON CURRENCYHISTORY_V1(CURRENCYID, CURRDATE)");
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("CURRENCYHISTORY_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

void CurrencyHistoryTable::ensure_data()
{
    m_db->Begin();
    m_db->Commit();
}

// Returns the column name as a string
wxString CurrencyHistoryTable::column_to_name(const CurrencyHistoryTable::COLUMN col)
{
    switch(col) {
        case COL_CURRHISTID: return "CURRHISTID";
        case COL_CURRENCYID: return "CURRENCYID";
        case COL_CURRDATE: return "CURRDATE";
        case COL_CURRVALUE: return "CURRVALUE";
        case COL_CURRUPDTYPE: return "CURRUPDTYPE";
        default: break;
    }

    return "UNKNOWN";
}

// Returns the column number from the given column name
CurrencyHistoryTable::COLUMN CurrencyHistoryTable::name_to_column(const wxString& name)
{
    if (name == "CURRHISTID") return COL_CURRHISTID;
    else if (name == "CURRENCYID") return COL_CURRENCYID;
    else if (name == "CURRDATE") return COL_CURRDATE;
    else if (name == "CURRVALUE") return COL_CURRVALUE;
    else if (name == "CURRUPDTYPE") return COL_CURRUPDTYPE;

    return COLUMN(-1);
}

// Create a new Data record and add to memory table (cache)
CurrencyHistoryTable::Data* CurrencyHistoryTable::create()
{
    Data* entity = new Data();
    m_cache.push_back(entity);
    return entity;
}

// Create a copy of the Data record and add to memory table (cache)
CurrencyHistoryTable::Data* CurrencyHistoryTable::clone(const CurrencyHistoryTable::Data* e)
{
    Data* entity = create();
    *entity = *e;
    entity->id(-1);
    return entity;
}

// Save the Data record to the database table.
// Either create a new record or update the existing record.
// Remove old record from the memory table (cache).
bool CurrencyHistoryTable::save(CurrencyHistoryTable::Data* entity)
{
    wxString sql = wxEmptyString;
    if (entity->id() <= 0) {
        //  new & insert
        sql = "INSERT INTO CURRENCYHISTORY_V1(CURRENCYID, CURRDATE, CURRVALUE, CURRUPDTYPE, CURRHISTID) VALUES(?, ?, ?, ?, ?)";
    }
    else {
        sql = "UPDATE CURRENCYHISTORY_V1 SET CURRENCYID = ?, CURRDATE = ?, CURRVALUE = ?, CURRUPDTYPE = ? WHERE CURRHISTID = ?";
    }

    try {
        wxSQLite3Statement stmt = m_db->PrepareStatement(sql);

        stmt.Bind(1, entity->CURRENCYID);
        stmt.Bind(2, entity->CURRDATE);
        stmt.Bind(3, entity->CURRVALUE);
        stmt.Bind(4, entity->CURRUPDTYPE);
        stmt.Bind(5, entity->id() > 0 ? entity->CURRHISTID : newId());

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
        wxLogError("CURRENCYHISTORY_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
        return false;
    }

    if (entity->id() <= 0) {
        entity->id(m_db->GetLastRowId());
        m_cache_index.insert(std::make_pair(entity->id(), entity));
    }
    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool CurrencyHistoryTable::remove(const int64 id)
{
    if (id <= 0) return false;
    try {
        wxString sql = "DELETE FROM CURRENCYHISTORY_V1 WHERE CURRHISTID = ?";
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
        wxLogError("CURRENCYHISTORY_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool CurrencyHistoryTable::remove(CurrencyHistoryTable::Data* entity)
{
    if (remove(entity->id())) {
        entity->id(-1);
        return true;
    }

    return false;
}

// Search the memory table (Cache) for the data record.
// If not found in memory, search the database and update the cache.
CurrencyHistoryTable::Data* CurrencyHistoryTable::cache_id(const int64 id)
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
CurrencyHistoryTable::Data* CurrencyHistoryTable::get_id(const int64 id)
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
const CurrencyHistoryTable::Data_Set CurrencyHistoryTable::get_all(const COLUMN col, const bool asc)
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
