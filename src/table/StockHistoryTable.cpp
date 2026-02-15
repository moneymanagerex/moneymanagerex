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

#include "StockHistoryTable.h"

StockHistoryTable::Data::Data()
{
    HISTID = -1;
    VALUE = 0.0;
    UPDTYPE = -1;
}

StockHistoryTable::Data::Data(wxSQLite3ResultSet& q)
{
    HISTID = q.GetInt64(0);
    SYMBOL = q.GetString(1);
    DATE = q.GetString(2);
    VALUE = q.GetDouble(3);
    UPDTYPE = q.GetInt64(4);
}

bool StockHistoryTable::Data::equals(const StockHistoryTable::Data* r) const
{
    if (HISTID != r->HISTID) return false;
    if (!SYMBOL.IsSameAs(r->SYMBOL)) return false;
    if (!DATE.IsSameAs(r->DATE)) return false;
    if (VALUE != r->VALUE) return false;
    if (UPDTYPE != r->UPDTYPE) return false;

    return true;
}

StockHistoryTable::Data& StockHistoryTable::Data::operator=(const StockHistoryTable::Data& other)
{
    if (this == &other) return *this;

    HISTID = other.HISTID;
    SYMBOL = other.SYMBOL;
    DATE = other.DATE;
    VALUE = other.VALUE;
    UPDTYPE = other.UPDTYPE;

    return *this;
}

// Return the data record as a json string
wxString StockHistoryTable::Data::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();			
    this->as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void StockHistoryTable::Data::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("HISTID");
    json_writer.Int64(this->HISTID.GetValue());
    json_writer.Key("SYMBOL");
    json_writer.String(this->SYMBOL.utf8_str());
    json_writer.Key("DATE");
    json_writer.String(this->DATE.utf8_str());
    json_writer.Key("VALUE");
    json_writer.Double(this->VALUE);
    json_writer.Key("UPDTYPE");
    json_writer.Int64(this->UPDTYPE.GetValue());
}

row_t StockHistoryTable::Data::to_row_t() const
{
    row_t row;

    row(L"HISTID") = HISTID.GetValue();
    row(L"SYMBOL") = SYMBOL;
    row(L"DATE") = DATE;
    row(L"VALUE") = VALUE;
    row(L"UPDTYPE") = UPDTYPE.GetValue();

    return row;
}

void StockHistoryTable::Data::to_template(html_template& t) const
{
    t(L"HISTID") = HISTID.GetValue();
    t(L"SYMBOL") = SYMBOL;
    t(L"DATE") = DATE;
    t(L"VALUE") = VALUE;
    t(L"UPDTYPE") = UPDTYPE.GetValue();
}

void StockHistoryTable::Data::destroy()
{
    delete this;
}

// Return the data records as a json array string
wxString StockHistoryTable::Data_Set::to_json() const
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

StockHistoryTable::StockHistoryTable() :
    fake_(new Data())
{
    m_table_name = "STOCKHISTORY_V1";
    m_query_select = "SELECT HISTID, SYMBOL, DATE, VALUE, UPDTYPE FROM STOCKHISTORY_V1 ";
}

// Destructor: clears any data records stored in memory
StockHistoryTable::~StockHistoryTable()
{
    delete this->fake_;
    destroy_cache();
}

// Remove all records stored in memory (cache) for the table
void StockHistoryTable::destroy_cache()
{
    std::for_each(m_cache.begin(), m_cache.end(), std::mem_fn(&Data::destroy));
    m_cache.clear();
    m_cache_index.clear(); // no memory release since it just stores pointer and the according objects are in cache
}

// Creates the database table if the table does not exist
bool StockHistoryTable::ensure_table()
{
    if (!table_exists()) {
        try {
            m_db->ExecuteUpdate("CREATE TABLE STOCKHISTORY_V1(HISTID integer primary key, SYMBOL TEXT NOT NULL, DATE TEXT NOT NULL, VALUE numeric NOT NULL, UPDTYPE integer, UNIQUE(SYMBOL, DATE))");
            this->ensure_data();
        }
        catch(const wxSQLite3Exception &e) {
            wxLogError("STOCKHISTORY_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }
    }

    this->ensure_index();

    return true;
}

bool StockHistoryTable::ensure_index()
{
    try {
        m_db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_STOCKHISTORY_SYMBOL ON STOCKHISTORY_V1(SYMBOL)");
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("STOCKHISTORY_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

void StockHistoryTable::ensure_data()
{
    m_db->Begin();
    m_db->Commit();
}

// Returns the column name as a string
wxString StockHistoryTable::column_to_name(const StockHistoryTable::COLUMN col)
{
    switch(col) {
        case COL_HISTID: return "HISTID";
        case COL_SYMBOL: return "SYMBOL";
        case COL_DATE: return "DATE";
        case COL_VALUE: return "VALUE";
        case COL_UPDTYPE: return "UPDTYPE";
        default: break;
    }

    return "UNKNOWN";
}

// Returns the column number from the given column name
StockHistoryTable::COLUMN StockHistoryTable::name_to_column(const wxString& name)
{
    if (name == "HISTID") return COL_HISTID;
    else if (name == "SYMBOL") return COL_SYMBOL;
    else if (name == "DATE") return COL_DATE;
    else if (name == "VALUE") return COL_VALUE;
    else if (name == "UPDTYPE") return COL_UPDTYPE;

    return COLUMN(-1);
}

// Create a new Data record and add to memory table (cache)
StockHistoryTable::Data* StockHistoryTable::create()
{
    Data* entity = new Data();
    m_cache.push_back(entity);
    return entity;
}

// Create a copy of the Data record and add to memory table (cache)
StockHistoryTable::Data* StockHistoryTable::clone(const StockHistoryTable::Data* e)
{
    Data* entity = create();
    *entity = *e;
    entity->id(-1);
    return entity;
}

// Save the Data record to the database table.
// Either create a new record or update the existing record.
// Remove old record from the memory table (cache).
bool StockHistoryTable::save(StockHistoryTable::Data* entity)
{
    wxString sql = wxEmptyString;
    if (entity->id() <= 0) {
        //  new & insert
        sql = "INSERT INTO STOCKHISTORY_V1(SYMBOL, DATE, VALUE, UPDTYPE, HISTID) VALUES(?, ?, ?, ?, ?)";
    }
    else {
        sql = "UPDATE STOCKHISTORY_V1 SET SYMBOL = ?, DATE = ?, VALUE = ?, UPDTYPE = ? WHERE HISTID = ?";
    }

    try {
        wxSQLite3Statement stmt = m_db->PrepareStatement(sql);

        stmt.Bind(1, entity->SYMBOL);
        stmt.Bind(2, entity->DATE);
        stmt.Bind(3, entity->VALUE);
        stmt.Bind(4, entity->UPDTYPE);
        stmt.Bind(5, entity->id() > 0 ? entity->HISTID : newId());

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
        wxLogError("STOCKHISTORY_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
        return false;
    }

    if (entity->id() <= 0) {
        entity->id(m_db->GetLastRowId());
        m_cache_index.insert(std::make_pair(entity->id(), entity));
    }
    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool StockHistoryTable::remove(const int64 id)
{
    if (id <= 0) return false;
    try {
        wxString sql = "DELETE FROM STOCKHISTORY_V1 WHERE HISTID = ?";
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
        wxLogError("STOCKHISTORY_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool StockHistoryTable::remove(StockHistoryTable::Data* entity)
{
    if (remove(entity->id())) {
        entity->id(-1);
        return true;
    }

    return false;
}

// Search the memory table (Cache) for the data record.
// If not found in memory, search the database and update the cache.
StockHistoryTable::Data* StockHistoryTable::cache_id(const int64 id)
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
StockHistoryTable::Data* StockHistoryTable::get_id(const int64 id)
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
const StockHistoryTable::Data_Set StockHistoryTable::get_all(const COLUMN col, const bool asc)
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
