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

#include "TransactionShareTable.h"

TransactionShareTable::Data::Data()
{
    SHAREINFOID = -1;
    CHECKINGACCOUNTID = -1;
    SHARENUMBER = 0.0;
    SHAREPRICE = 0.0;
    SHARECOMMISSION = 0.0;
}

TransactionShareTable::Data::Data(wxSQLite3ResultSet& q)
{
    SHAREINFOID = q.GetInt64(0);
    CHECKINGACCOUNTID = q.GetInt64(1);
    SHARENUMBER = q.GetDouble(2);
    SHAREPRICE = q.GetDouble(3);
    SHARECOMMISSION = q.GetDouble(4);
    SHARELOT = q.GetString(5);
}

bool TransactionShareTable::Data::equals(const TransactionShareTable::Data* r) const
{
    if (SHAREINFOID != r->SHAREINFOID) return false;
    if (CHECKINGACCOUNTID != r->CHECKINGACCOUNTID) return false;
    if (SHARENUMBER != r->SHARENUMBER) return false;
    if (SHAREPRICE != r->SHAREPRICE) return false;
    if (SHARECOMMISSION != r->SHARECOMMISSION) return false;
    if (!SHARELOT.IsSameAs(r->SHARELOT)) return false;

    return true;
}

TransactionShareTable::Data& TransactionShareTable::Data::operator=(const TransactionShareTable::Data& other)
{
    if (this == &other) return *this;

    SHAREINFOID = other.SHAREINFOID;
    CHECKINGACCOUNTID = other.CHECKINGACCOUNTID;
    SHARENUMBER = other.SHARENUMBER;
    SHAREPRICE = other.SHAREPRICE;
    SHARECOMMISSION = other.SHARECOMMISSION;
    SHARELOT = other.SHARELOT;

    return *this;
}

// Return the data record as a json string
wxString TransactionShareTable::Data::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();			
    this->as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void TransactionShareTable::Data::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("SHAREINFOID");
    json_writer.Int64(this->SHAREINFOID.GetValue());
    json_writer.Key("CHECKINGACCOUNTID");
    json_writer.Int64(this->CHECKINGACCOUNTID.GetValue());
    json_writer.Key("SHARENUMBER");
    json_writer.Double(this->SHARENUMBER);
    json_writer.Key("SHAREPRICE");
    json_writer.Double(this->SHAREPRICE);
    json_writer.Key("SHARECOMMISSION");
    json_writer.Double(this->SHARECOMMISSION);
    json_writer.Key("SHARELOT");
    json_writer.String(this->SHARELOT.utf8_str());
}

row_t TransactionShareTable::Data::to_row_t() const
{
    row_t row;

    row(L"SHAREINFOID") = SHAREINFOID.GetValue();
    row(L"CHECKINGACCOUNTID") = CHECKINGACCOUNTID.GetValue();
    row(L"SHARENUMBER") = SHARENUMBER;
    row(L"SHAREPRICE") = SHAREPRICE;
    row(L"SHARECOMMISSION") = SHARECOMMISSION;
    row(L"SHARELOT") = SHARELOT;

    return row;
}

void TransactionShareTable::Data::to_template(html_template& t) const
{
    t(L"SHAREINFOID") = SHAREINFOID.GetValue();
    t(L"CHECKINGACCOUNTID") = CHECKINGACCOUNTID.GetValue();
    t(L"SHARENUMBER") = SHARENUMBER;
    t(L"SHAREPRICE") = SHAREPRICE;
    t(L"SHARECOMMISSION") = SHARECOMMISSION;
    t(L"SHARELOT") = SHARELOT;
}

void TransactionShareTable::Data::destroy()
{
    delete this;
}

// Return the data records as a json array string
wxString TransactionShareTable::Data_Set::to_json() const
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

TransactionShareTable::TransactionShareTable() :
    fake_(new Data())
{
    m_table_name = "SHAREINFO_V1";
    m_query_select = "SELECT SHAREINFOID, CHECKINGACCOUNTID, SHARENUMBER, SHAREPRICE, SHARECOMMISSION, SHARELOT FROM SHAREINFO_V1 ";
}

// Destructor: clears any data records stored in memory
TransactionShareTable::~TransactionShareTable()
{
    delete this->fake_;
    destroy_cache();
}

// Remove all records stored in memory (cache) for the table
void TransactionShareTable::destroy_cache()
{
    std::for_each(m_cache.begin(), m_cache.end(), std::mem_fn(&Data::destroy));
    m_cache.clear();
    m_cache_index.clear(); // no memory release since it just stores pointer and the according objects are in cache
}

// Creates the database table if the table does not exist
bool TransactionShareTable::ensure_table()
{
    if (!table_exists()) {
        try {
            m_db->ExecuteUpdate("CREATE TABLE SHAREINFO_V1 (SHAREINFOID integer NOT NULL primary key, CHECKINGACCOUNTID integer NOT NULL, SHARENUMBER numeric, SHAREPRICE numeric, SHARECOMMISSION numeric, SHARELOT TEXT)");
            this->ensure_data();
        }
        catch(const wxSQLite3Exception &e) {
            wxLogError("SHAREINFO_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }
    }

    this->ensure_index();

    return true;
}

bool TransactionShareTable::ensure_index()
{
    try {
        m_db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_SHAREINFO ON SHAREINFO_V1 (CHECKINGACCOUNTID)");
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("SHAREINFO_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

void TransactionShareTable::ensure_data()
{
    m_db->Begin();
    m_db->Commit();
}

// Returns the column name as a string
wxString TransactionShareTable::column_to_name(const TransactionShareTable::COLUMN col)
{
    switch(col) {
        case COL_SHAREINFOID: return "SHAREINFOID";
        case COL_CHECKINGACCOUNTID: return "CHECKINGACCOUNTID";
        case COL_SHARENUMBER: return "SHARENUMBER";
        case COL_SHAREPRICE: return "SHAREPRICE";
        case COL_SHARECOMMISSION: return "SHARECOMMISSION";
        case COL_SHARELOT: return "SHARELOT";
        default: break;
    }

    return "UNKNOWN";
}

// Returns the column number from the given column name
TransactionShareTable::COLUMN TransactionShareTable::name_to_column(const wxString& name)
{
    if (name == "SHAREINFOID") return COL_SHAREINFOID;
    else if (name == "CHECKINGACCOUNTID") return COL_CHECKINGACCOUNTID;
    else if (name == "SHARENUMBER") return COL_SHARENUMBER;
    else if (name == "SHAREPRICE") return COL_SHAREPRICE;
    else if (name == "SHARECOMMISSION") return COL_SHARECOMMISSION;
    else if (name == "SHARELOT") return COL_SHARELOT;

    return COLUMN(-1);
}

// Create a new Data record and add to memory table (cache)
TransactionShareTable::Data* TransactionShareTable::create()
{
    Data* entity = new Data();
    m_cache.push_back(entity);
    return entity;
}

// Create a copy of the Data record and add to memory table (cache)
TransactionShareTable::Data* TransactionShareTable::clone(const TransactionShareTable::Data* e)
{
    Data* entity = create();
    *entity = *e;
    entity->id(-1);
    return entity;
}

// Save the Data record to the database table.
// Either create a new record or update the existing record.
// Remove old record from the memory table (cache).
bool TransactionShareTable::save(TransactionShareTable::Data* entity)
{
    wxString sql = wxEmptyString;
    if (entity->id() <= 0) {
        //  new & insert
        sql = "INSERT INTO SHAREINFO_V1(CHECKINGACCOUNTID, SHARENUMBER, SHAREPRICE, SHARECOMMISSION, SHARELOT, SHAREINFOID) VALUES(?, ?, ?, ?, ?, ?)";
    }
    else {
        sql = "UPDATE SHAREINFO_V1 SET CHECKINGACCOUNTID = ?, SHARENUMBER = ?, SHAREPRICE = ?, SHARECOMMISSION = ?, SHARELOT = ? WHERE SHAREINFOID = ?";
    }

    try {
        wxSQLite3Statement stmt = m_db->PrepareStatement(sql);

        stmt.Bind(1, entity->CHECKINGACCOUNTID);
        stmt.Bind(2, entity->SHARENUMBER);
        stmt.Bind(3, entity->SHAREPRICE);
        stmt.Bind(4, entity->SHARECOMMISSION);
        stmt.Bind(5, entity->SHARELOT);
        stmt.Bind(6, entity->id() > 0 ? entity->SHAREINFOID : newId());

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
        wxLogError("SHAREINFO_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
        return false;
    }

    if (entity->id() <= 0) {
        entity->id(m_db->GetLastRowId());
        m_cache_index.insert(std::make_pair(entity->id(), entity));
    }
    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool TransactionShareTable::remove(const int64 id)
{
    if (id <= 0) return false;
    try {
        wxString sql = "DELETE FROM SHAREINFO_V1 WHERE SHAREINFOID = ?";
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
        wxLogError("SHAREINFO_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool TransactionShareTable::remove(TransactionShareTable::Data* entity)
{
    if (remove(entity->id())) {
        entity->id(-1);
        return true;
    }

    return false;
}

// Search the memory table (Cache) for the data record.
// If not found in memory, search the database and update the cache.
TransactionShareTable::Data* TransactionShareTable::cache_id(const int64 id)
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
TransactionShareTable::Data* TransactionShareTable::get_id(const int64 id)
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
const TransactionShareTable::Data_Set TransactionShareTable::get_all(const COLUMN col, const bool asc)
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
