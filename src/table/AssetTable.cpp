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

#include "AssetTable.h"

AssetTable::Data::Data()
{
    ASSETID = -1;
    CURRENCYID = -1;
    VALUE = 0.0;
    VALUECHANGERATE = 0.0;
}

AssetTable::Data::Data(wxSQLite3ResultSet& q)
{
    ASSETID = q.GetInt64(0);
    STARTDATE = q.GetString(1);
    ASSETNAME = q.GetString(2);
    ASSETSTATUS = q.GetString(3);
    CURRENCYID = q.GetInt64(4);
    VALUECHANGEMODE = q.GetString(5);
    VALUE = q.GetDouble(6);
    VALUECHANGE = q.GetString(7);
    NOTES = q.GetString(8);
    VALUECHANGERATE = q.GetDouble(9);
    ASSETTYPE = q.GetString(10);
}

bool AssetTable::Data::equals(const AssetTable::Data* r) const
{
    if (ASSETID != r->ASSETID) return false;
    if (!STARTDATE.IsSameAs(r->STARTDATE)) return false;
    if (!ASSETNAME.IsSameAs(r->ASSETNAME)) return false;
    if (!ASSETSTATUS.IsSameAs(r->ASSETSTATUS)) return false;
    if (CURRENCYID != r->CURRENCYID) return false;
    if (!VALUECHANGEMODE.IsSameAs(r->VALUECHANGEMODE)) return false;
    if (VALUE != r->VALUE) return false;
    if (!VALUECHANGE.IsSameAs(r->VALUECHANGE)) return false;
    if (!NOTES.IsSameAs(r->NOTES)) return false;
    if (VALUECHANGERATE != r->VALUECHANGERATE) return false;
    if (!ASSETTYPE.IsSameAs(r->ASSETTYPE)) return false;

    return true;
}

AssetTable::Data& AssetTable::Data::operator=(const AssetTable::Data& other)
{
    if (this == &other) return *this;

    ASSETID = other.ASSETID;
    STARTDATE = other.STARTDATE;
    ASSETNAME = other.ASSETNAME;
    ASSETSTATUS = other.ASSETSTATUS;
    CURRENCYID = other.CURRENCYID;
    VALUECHANGEMODE = other.VALUECHANGEMODE;
    VALUE = other.VALUE;
    VALUECHANGE = other.VALUECHANGE;
    NOTES = other.NOTES;
    VALUECHANGERATE = other.VALUECHANGERATE;
    ASSETTYPE = other.ASSETTYPE;

    return *this;
}

// Return the data record as a json string
wxString AssetTable::Data::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();			
    this->as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void AssetTable::Data::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("ASSETID");
    json_writer.Int64(this->ASSETID.GetValue());
    json_writer.Key("STARTDATE");
    json_writer.String(this->STARTDATE.utf8_str());
    json_writer.Key("ASSETNAME");
    json_writer.String(this->ASSETNAME.utf8_str());
    json_writer.Key("ASSETSTATUS");
    json_writer.String(this->ASSETSTATUS.utf8_str());
    json_writer.Key("CURRENCYID");
    json_writer.Int64(this->CURRENCYID.GetValue());
    json_writer.Key("VALUECHANGEMODE");
    json_writer.String(this->VALUECHANGEMODE.utf8_str());
    json_writer.Key("VALUE");
    json_writer.Double(this->VALUE);
    json_writer.Key("VALUECHANGE");
    json_writer.String(this->VALUECHANGE.utf8_str());
    json_writer.Key("NOTES");
    json_writer.String(this->NOTES.utf8_str());
    json_writer.Key("VALUECHANGERATE");
    json_writer.Double(this->VALUECHANGERATE);
    json_writer.Key("ASSETTYPE");
    json_writer.String(this->ASSETTYPE.utf8_str());
}

row_t AssetTable::Data::to_row_t() const
{
    row_t row;

    row(L"ASSETID") = ASSETID.GetValue();
    row(L"STARTDATE") = STARTDATE;
    row(L"ASSETNAME") = ASSETNAME;
    row(L"ASSETSTATUS") = ASSETSTATUS;
    row(L"CURRENCYID") = CURRENCYID.GetValue();
    row(L"VALUECHANGEMODE") = VALUECHANGEMODE;
    row(L"VALUE") = VALUE;
    row(L"VALUECHANGE") = VALUECHANGE;
    row(L"NOTES") = NOTES;
    row(L"VALUECHANGERATE") = VALUECHANGERATE;
    row(L"ASSETTYPE") = ASSETTYPE;

    return row;
}

void AssetTable::Data::to_template(html_template& t) const
{
    t(L"ASSETID") = ASSETID.GetValue();
    t(L"STARTDATE") = STARTDATE;
    t(L"ASSETNAME") = ASSETNAME;
    t(L"ASSETSTATUS") = ASSETSTATUS;
    t(L"CURRENCYID") = CURRENCYID.GetValue();
    t(L"VALUECHANGEMODE") = VALUECHANGEMODE;
    t(L"VALUE") = VALUE;
    t(L"VALUECHANGE") = VALUECHANGE;
    t(L"NOTES") = NOTES;
    t(L"VALUECHANGERATE") = VALUECHANGERATE;
    t(L"ASSETTYPE") = ASSETTYPE;
}

void AssetTable::Data::destroy()
{
    delete this;
}

// Return the data records as a json array string
wxString AssetTable::Data_Set::to_json() const
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

AssetTable::AssetTable() :
    fake_(new Data())
{
    m_table_name = "ASSETS_V1";
    m_query_select = "SELECT ASSETID, STARTDATE, ASSETNAME, ASSETSTATUS, CURRENCYID, VALUECHANGEMODE, VALUE, VALUECHANGE, NOTES, VALUECHANGERATE, ASSETTYPE FROM ASSETS_V1 ";
}

// Destructor: clears any data records stored in memory
AssetTable::~AssetTable()
{
    delete this->fake_;
    destroy_cache();
}

// Remove all records stored in memory (cache) for the table
void AssetTable::destroy_cache()
{
    std::for_each(m_cache.begin(), m_cache.end(), std::mem_fn(&Data::destroy));
    m_cache.clear();
    m_cache_index.clear(); // no memory release since it just stores pointer and the according objects are in cache
}

// Creates the database table if the table does not exist
bool AssetTable::ensure_table()
{
    if (!table_exists()) {
        try {
            m_db->ExecuteUpdate("CREATE TABLE ASSETS_V1(ASSETID integer primary key, STARTDATE TEXT NOT NULL, ASSETNAME TEXT COLLATE NOCASE NOT NULL, ASSETSTATUS TEXT /* Open, Closed */, CURRENCYID integer, VALUECHANGEMODE TEXT /* Percentage, Linear */, VALUE numeric, VALUECHANGE TEXT /* None, Appreciates, Depreciates */, NOTES TEXT, VALUECHANGERATE numeric, ASSETTYPE TEXT /* Property, Automobile, Household Object, Art, Jewellery, Cash, Other */)");
            this->ensure_data();
        }
        catch(const wxSQLite3Exception &e) {
            wxLogError("ASSETS_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }
    }

    this->ensure_index();

    return true;
}

bool AssetTable::ensure_index()
{
    try {
        m_db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_ASSETS_ASSETTYPE ON ASSETS_V1(ASSETTYPE)");
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("ASSETS_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

void AssetTable::ensure_data()
{
    m_db->Begin();
    m_db->Commit();
}

// Returns the column name as a string
wxString AssetTable::column_to_name(const AssetTable::COLUMN col)
{
    switch(col) {
        case COL_ASSETID: return "ASSETID";
        case COL_STARTDATE: return "STARTDATE";
        case COL_ASSETNAME: return "ASSETNAME";
        case COL_ASSETSTATUS: return "ASSETSTATUS";
        case COL_CURRENCYID: return "CURRENCYID";
        case COL_VALUECHANGEMODE: return "VALUECHANGEMODE";
        case COL_VALUE: return "VALUE";
        case COL_VALUECHANGE: return "VALUECHANGE";
        case COL_NOTES: return "NOTES";
        case COL_VALUECHANGERATE: return "VALUECHANGERATE";
        case COL_ASSETTYPE: return "ASSETTYPE";
        default: break;
    }

    return "UNKNOWN";
}

// Returns the column number from the given column name
AssetTable::COLUMN AssetTable::name_to_column(const wxString& name)
{
    if (name == "ASSETID") return COL_ASSETID;
    else if (name == "STARTDATE") return COL_STARTDATE;
    else if (name == "ASSETNAME") return COL_ASSETNAME;
    else if (name == "ASSETSTATUS") return COL_ASSETSTATUS;
    else if (name == "CURRENCYID") return COL_CURRENCYID;
    else if (name == "VALUECHANGEMODE") return COL_VALUECHANGEMODE;
    else if (name == "VALUE") return COL_VALUE;
    else if (name == "VALUECHANGE") return COL_VALUECHANGE;
    else if (name == "NOTES") return COL_NOTES;
    else if (name == "VALUECHANGERATE") return COL_VALUECHANGERATE;
    else if (name == "ASSETTYPE") return COL_ASSETTYPE;

    return COLUMN(-1);
}

// Create a new Data record and add to memory table (cache)
AssetTable::Data* AssetTable::create()
{
    Data* entity = new Data();
    m_cache.push_back(entity);
    return entity;
}

// Create a copy of the Data record and add to memory table (cache)
AssetTable::Data* AssetTable::clone(const AssetTable::Data* e)
{
    Data* entity = create();
    *entity = *e;
    entity->id(-1);
    return entity;
}

// Save the Data record to the database table.
// Either create a new record or update the existing record.
// Remove old record from the memory table (cache).
bool AssetTable::save(AssetTable::Data* entity)
{
    wxString sql = wxEmptyString;
    if (entity->id() <= 0) {
        //  new & insert
        sql = "INSERT INTO ASSETS_V1(STARTDATE, ASSETNAME, ASSETSTATUS, CURRENCYID, VALUECHANGEMODE, VALUE, VALUECHANGE, NOTES, VALUECHANGERATE, ASSETTYPE, ASSETID) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    }
    else {
        sql = "UPDATE ASSETS_V1 SET STARTDATE = ?, ASSETNAME = ?, ASSETSTATUS = ?, CURRENCYID = ?, VALUECHANGEMODE = ?, VALUE = ?, VALUECHANGE = ?, NOTES = ?, VALUECHANGERATE = ?, ASSETTYPE = ? WHERE ASSETID = ?";
    }

    try {
        wxSQLite3Statement stmt = m_db->PrepareStatement(sql);

        stmt.Bind(1, entity->STARTDATE);
        stmt.Bind(2, entity->ASSETNAME);
        stmt.Bind(3, entity->ASSETSTATUS);
        stmt.Bind(4, entity->CURRENCYID);
        stmt.Bind(5, entity->VALUECHANGEMODE);
        stmt.Bind(6, entity->VALUE);
        stmt.Bind(7, entity->VALUECHANGE);
        stmt.Bind(8, entity->NOTES);
        stmt.Bind(9, entity->VALUECHANGERATE);
        stmt.Bind(10, entity->ASSETTYPE);
        stmt.Bind(11, entity->id() > 0 ? entity->ASSETID : newId());

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
        wxLogError("ASSETS_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
        return false;
    }

    if (entity->id() <= 0) {
        entity->id(m_db->GetLastRowId());
        m_cache_index.insert(std::make_pair(entity->id(), entity));
    }
    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool AssetTable::remove(const int64 id)
{
    if (id <= 0) return false;
    try {
        wxString sql = "DELETE FROM ASSETS_V1 WHERE ASSETID = ?";
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
        wxLogError("ASSETS_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool AssetTable::remove(AssetTable::Data* entity)
{
    if (remove(entity->id())) {
        entity->id(-1);
        return true;
    }

    return false;
}

// Search the memory table (Cache) for the data record.
// If not found in memory, search the database and update the cache.
AssetTable::Data* AssetTable::cache_id(const int64 id)
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
AssetTable::Data* AssetTable::get_id(const int64 id)
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
const AssetTable::Data_Set AssetTable::get_all(const COLUMN col, const bool asc)
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
