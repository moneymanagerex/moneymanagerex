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

#include "StockTable.h"

StockTable::Data::Data()
{
    STOCKID = -1;
    HELDAT = -1;
    NUMSHARES = 0.0;
    PURCHASEPRICE = 0.0;
    CURRENTPRICE = 0.0;
    VALUE = 0.0;
    COMMISSION = 0.0;
}

StockTable::Data::Data(wxSQLite3ResultSet& q)
{
    STOCKID = q.GetInt64(0);
    HELDAT = q.GetInt64(1);
    PURCHASEDATE = q.GetString(2);
    STOCKNAME = q.GetString(3);
    SYMBOL = q.GetString(4);
    NUMSHARES = q.GetDouble(5);
    PURCHASEPRICE = q.GetDouble(6);
    NOTES = q.GetString(7);
    CURRENTPRICE = q.GetDouble(8);
    VALUE = q.GetDouble(9);
    COMMISSION = q.GetDouble(10);
}

bool StockTable::Data::equals(const StockTable::Data* r) const
{
    if (STOCKID != r->STOCKID) return false;
    if (HELDAT != r->HELDAT) return false;
    if (!PURCHASEDATE.IsSameAs(r->PURCHASEDATE)) return false;
    if (!STOCKNAME.IsSameAs(r->STOCKNAME)) return false;
    if (!SYMBOL.IsSameAs(r->SYMBOL)) return false;
    if (NUMSHARES != r->NUMSHARES) return false;
    if (PURCHASEPRICE != r->PURCHASEPRICE) return false;
    if (!NOTES.IsSameAs(r->NOTES)) return false;
    if (CURRENTPRICE != r->CURRENTPRICE) return false;
    if (VALUE != r->VALUE) return false;
    if (COMMISSION != r->COMMISSION) return false;

    return true;
}

StockTable::Data& StockTable::Data::operator=(const StockTable::Data& other)
{
    if (this == &other) return *this;

    STOCKID = other.STOCKID;
    HELDAT = other.HELDAT;
    PURCHASEDATE = other.PURCHASEDATE;
    STOCKNAME = other.STOCKNAME;
    SYMBOL = other.SYMBOL;
    NUMSHARES = other.NUMSHARES;
    PURCHASEPRICE = other.PURCHASEPRICE;
    NOTES = other.NOTES;
    CURRENTPRICE = other.CURRENTPRICE;
    VALUE = other.VALUE;
    COMMISSION = other.COMMISSION;

    return *this;
}

// Return the data record as a json string
wxString StockTable::Data::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();			
    this->as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void StockTable::Data::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("STOCKID");
    json_writer.Int64(this->STOCKID.GetValue());
    json_writer.Key("HELDAT");
    json_writer.Int64(this->HELDAT.GetValue());
    json_writer.Key("PURCHASEDATE");
    json_writer.String(this->PURCHASEDATE.utf8_str());
    json_writer.Key("STOCKNAME");
    json_writer.String(this->STOCKNAME.utf8_str());
    json_writer.Key("SYMBOL");
    json_writer.String(this->SYMBOL.utf8_str());
    json_writer.Key("NUMSHARES");
    json_writer.Double(this->NUMSHARES);
    json_writer.Key("PURCHASEPRICE");
    json_writer.Double(this->PURCHASEPRICE);
    json_writer.Key("NOTES");
    json_writer.String(this->NOTES.utf8_str());
    json_writer.Key("CURRENTPRICE");
    json_writer.Double(this->CURRENTPRICE);
    json_writer.Key("VALUE");
    json_writer.Double(this->VALUE);
    json_writer.Key("COMMISSION");
    json_writer.Double(this->COMMISSION);
}

row_t StockTable::Data::to_row_t() const
{
    row_t row;

    row(L"STOCKID") = STOCKID.GetValue();
    row(L"HELDAT") = HELDAT.GetValue();
    row(L"PURCHASEDATE") = PURCHASEDATE;
    row(L"STOCKNAME") = STOCKNAME;
    row(L"SYMBOL") = SYMBOL;
    row(L"NUMSHARES") = NUMSHARES;
    row(L"PURCHASEPRICE") = PURCHASEPRICE;
    row(L"NOTES") = NOTES;
    row(L"CURRENTPRICE") = CURRENTPRICE;
    row(L"VALUE") = VALUE;
    row(L"COMMISSION") = COMMISSION;

    return row;
}

void StockTable::Data::to_template(html_template& t) const
{
    t(L"STOCKID") = STOCKID.GetValue();
    t(L"HELDAT") = HELDAT.GetValue();
    t(L"PURCHASEDATE") = PURCHASEDATE;
    t(L"STOCKNAME") = STOCKNAME;
    t(L"SYMBOL") = SYMBOL;
    t(L"NUMSHARES") = NUMSHARES;
    t(L"PURCHASEPRICE") = PURCHASEPRICE;
    t(L"NOTES") = NOTES;
    t(L"CURRENTPRICE") = CURRENTPRICE;
    t(L"VALUE") = VALUE;
    t(L"COMMISSION") = COMMISSION;
}

void StockTable::Data::destroy()
{
    delete this;
}

// Return the data records as a json array string
wxString StockTable::Data_Set::to_json() const
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

StockTable::StockTable() :
    fake_(new Data())
{
    m_table_name = "STOCK_V1";
    m_query_select = "SELECT STOCKID, HELDAT, PURCHASEDATE, STOCKNAME, SYMBOL, NUMSHARES, PURCHASEPRICE, NOTES, CURRENTPRICE, VALUE, COMMISSION FROM STOCK_V1 ";
}

// Destructor: clears any data records stored in memory
StockTable::~StockTable()
{
    delete this->fake_;
    destroy_cache();
}

// Remove all records stored in memory (cache) for the table
void StockTable::destroy_cache()
{
    std::for_each(m_cache.begin(), m_cache.end(), std::mem_fn(&Data::destroy));
    m_cache.clear();
    m_cache_index.clear(); // no memory release since it just stores pointer and the according objects are in cache
}

// Creates the database table if the table does not exist
bool StockTable::ensure_table()
{
    if (!table_exists()) {
        try {
            m_db->ExecuteUpdate("CREATE TABLE STOCK_V1(STOCKID integer primary key, HELDAT integer, PURCHASEDATE TEXT NOT NULL, STOCKNAME TEXT COLLATE NOCASE NOT NULL, SYMBOL TEXT, NUMSHARES numeric, PURCHASEPRICE numeric NOT NULL, NOTES TEXT, CURRENTPRICE numeric NOT NULL, VALUE numeric, COMMISSION numeric)");
            this->ensure_data();
        }
        catch(const wxSQLite3Exception &e) {
            wxLogError("STOCK_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }
    }

    this->ensure_index();

    return true;
}

bool StockTable::ensure_index()
{
    try {
        m_db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_STOCK_HELDAT ON STOCK_V1(HELDAT)");
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("STOCK_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

void StockTable::ensure_data()
{
    m_db->Begin();
    m_db->Commit();
}

// Returns the column name as a string
wxString StockTable::column_to_name(const StockTable::COLUMN col)
{
    switch(col) {
        case COL_STOCKID: return "STOCKID";
        case COL_HELDAT: return "HELDAT";
        case COL_PURCHASEDATE: return "PURCHASEDATE";
        case COL_STOCKNAME: return "STOCKNAME";
        case COL_SYMBOL: return "SYMBOL";
        case COL_NUMSHARES: return "NUMSHARES";
        case COL_PURCHASEPRICE: return "PURCHASEPRICE";
        case COL_NOTES: return "NOTES";
        case COL_CURRENTPRICE: return "CURRENTPRICE";
        case COL_VALUE: return "VALUE";
        case COL_COMMISSION: return "COMMISSION";
        default: break;
    }

    return "UNKNOWN";
}

// Returns the column number from the given column name
StockTable::COLUMN StockTable::name_to_column(const wxString& name)
{
    if (name == "STOCKID") return COL_STOCKID;
    else if (name == "HELDAT") return COL_HELDAT;
    else if (name == "PURCHASEDATE") return COL_PURCHASEDATE;
    else if (name == "STOCKNAME") return COL_STOCKNAME;
    else if (name == "SYMBOL") return COL_SYMBOL;
    else if (name == "NUMSHARES") return COL_NUMSHARES;
    else if (name == "PURCHASEPRICE") return COL_PURCHASEPRICE;
    else if (name == "NOTES") return COL_NOTES;
    else if (name == "CURRENTPRICE") return COL_CURRENTPRICE;
    else if (name == "VALUE") return COL_VALUE;
    else if (name == "COMMISSION") return COL_COMMISSION;

    return COLUMN(-1);
}

// Create a new Data record and add to memory table (cache)
StockTable::Data* StockTable::create()
{
    Data* entity = new Data();
    m_cache.push_back(entity);
    return entity;
}

// Create a copy of the Data record and add to memory table (cache)
StockTable::Data* StockTable::clone(const StockTable::Data* e)
{
    Data* entity = create();
    *entity = *e;
    entity->id(-1);
    return entity;
}

// Save the Data record to the database table.
// Either create a new record or update the existing record.
// Remove old record from the memory table (cache).
bool StockTable::save(StockTable::Data* entity)
{
    wxString sql = wxEmptyString;
    if (entity->id() <= 0) {
        //  new & insert
        sql = "INSERT INTO STOCK_V1(HELDAT, PURCHASEDATE, STOCKNAME, SYMBOL, NUMSHARES, PURCHASEPRICE, NOTES, CURRENTPRICE, VALUE, COMMISSION, STOCKID) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    }
    else {
        sql = "UPDATE STOCK_V1 SET HELDAT = ?, PURCHASEDATE = ?, STOCKNAME = ?, SYMBOL = ?, NUMSHARES = ?, PURCHASEPRICE = ?, NOTES = ?, CURRENTPRICE = ?, VALUE = ?, COMMISSION = ? WHERE STOCKID = ?";
    }

    try {
        wxSQLite3Statement stmt = m_db->PrepareStatement(sql);

        stmt.Bind(1, entity->HELDAT);
        stmt.Bind(2, entity->PURCHASEDATE);
        stmt.Bind(3, entity->STOCKNAME);
        stmt.Bind(4, entity->SYMBOL);
        stmt.Bind(5, entity->NUMSHARES);
        stmt.Bind(6, entity->PURCHASEPRICE);
        stmt.Bind(7, entity->NOTES);
        stmt.Bind(8, entity->CURRENTPRICE);
        stmt.Bind(9, entity->VALUE);
        stmt.Bind(10, entity->COMMISSION);
        stmt.Bind(11, entity->id() > 0 ? entity->STOCKID : newId());

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
        wxLogError("STOCK_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
        return false;
    }

    if (entity->id() <= 0) {
        entity->id(m_db->GetLastRowId());
        m_cache_index.insert(std::make_pair(entity->id(), entity));
    }
    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool StockTable::remove(const int64 id)
{
    if (id <= 0) return false;
    try {
        wxString sql = "DELETE FROM STOCK_V1 WHERE STOCKID = ?";
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
        wxLogError("STOCK_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool StockTable::remove(StockTable::Data* entity)
{
    if (remove(entity->id())) {
        entity->id(-1);
        return true;
    }

    return false;
}

// Search the memory table (Cache) for the data record.
// If not found in memory, search the database and update the cache.
StockTable::Data* StockTable::cache_id(const int64 id)
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
StockTable::Data* StockTable::get_id(const int64 id)
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
const StockTable::Data_Set StockTable::get_all(const COLUMN col, const bool asc)
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
