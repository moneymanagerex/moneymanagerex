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

#include "SettingTable.h"

SettingTable::Data::Data()
{
    SETTINGID = -1;
}

SettingTable::Data::Data(wxSQLite3ResultSet& q)
{
    SETTINGID = q.GetInt64(0);
    SETTINGNAME = q.GetString(1);
    SETTINGVALUE = q.GetString(2);
}

bool SettingTable::Data::equals(const SettingTable::Data* r) const
{
    if (SETTINGID != r->SETTINGID) return false;
    if (!SETTINGNAME.IsSameAs(r->SETTINGNAME)) return false;
    if (!SETTINGVALUE.IsSameAs(r->SETTINGVALUE)) return false;

    return true;
}

SettingTable::Data& SettingTable::Data::operator=(const SettingTable::Data& other)
{
    if (this == &other) return *this;

    SETTINGID = other.SETTINGID;
    SETTINGNAME = other.SETTINGNAME;
    SETTINGVALUE = other.SETTINGVALUE;

    return *this;
}

// Return the data record as a json string
wxString SettingTable::Data::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();			
    this->as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void SettingTable::Data::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("SETTINGID");
    json_writer.Int64(this->SETTINGID.GetValue());
    json_writer.Key("SETTINGNAME");
    json_writer.String(this->SETTINGNAME.utf8_str());
    json_writer.Key("SETTINGVALUE");
    json_writer.String(this->SETTINGVALUE.utf8_str());
}

row_t SettingTable::Data::to_row_t() const
{
    row_t row;

    row(L"SETTINGID") = SETTINGID.GetValue();
    row(L"SETTINGNAME") = SETTINGNAME;
    row(L"SETTINGVALUE") = SETTINGVALUE;

    return row;
}

void SettingTable::Data::to_template(html_template& t) const
{
    t(L"SETTINGID") = SETTINGID.GetValue();
    t(L"SETTINGNAME") = SETTINGNAME;
    t(L"SETTINGVALUE") = SETTINGVALUE;
}

void SettingTable::Data::destroy()
{
    delete this;
}

// Return the data records as a json array string
wxString SettingTable::Data_Set::to_json() const
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

SettingTable::SettingTable() :
    fake_(new Data())
{
    m_table_name = "SETTING_V1";
    m_query_select = "SELECT SETTINGID, SETTINGNAME, SETTINGVALUE FROM SETTING_V1 ";
}

// Destructor: clears any data records stored in memory
SettingTable::~SettingTable()
{
    delete this->fake_;
    destroy_cache();
}

// Remove all records stored in memory (cache) for the table
void SettingTable::destroy_cache()
{
    std::for_each(m_cache.begin(), m_cache.end(), std::mem_fn(&Data::destroy));
    m_cache.clear();
    m_cache_index.clear(); // no memory release since it just stores pointer and the according objects are in cache
}

// Creates the database table if the table does not exist
bool SettingTable::ensure_table()
{
    if (!table_exists()) {
        try {
            m_db->ExecuteUpdate("CREATE TABLE SETTING_V1(SETTINGID integer not null primary key, SETTINGNAME TEXT COLLATE NOCASE NOT NULL UNIQUE, SETTINGVALUE TEXT)");
            this->ensure_data();
        }
        catch(const wxSQLite3Exception &e) {
            wxLogError("SETTING_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }
    }

    this->ensure_index();

    return true;
}

bool SettingTable::ensure_index()
{
    try {
        m_db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_SETTING_SETTINGNAME ON SETTING_V1(SETTINGNAME)");
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("SETTING_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

void SettingTable::ensure_data()
{
    m_db->Begin();
    m_db->Commit();
}

// Returns the column name as a string
wxString SettingTable::column_to_name(const SettingTable::COLUMN col)
{
    switch(col) {
        case COL_SETTINGID: return "SETTINGID";
        case COL_SETTINGNAME: return "SETTINGNAME";
        case COL_SETTINGVALUE: return "SETTINGVALUE";
        default: break;
    }

    return "UNKNOWN";
}

// Returns the column number from the given column name
SettingTable::COLUMN SettingTable::name_to_column(const wxString& name)
{
    if (name == "SETTINGID") return COL_SETTINGID;
    else if (name == "SETTINGNAME") return COL_SETTINGNAME;
    else if (name == "SETTINGVALUE") return COL_SETTINGVALUE;

    return COLUMN(-1);
}

// Create a new Data record and add to memory table (cache)
SettingTable::Data* SettingTable::create()
{
    Data* entity = new Data();
    m_cache.push_back(entity);
    return entity;
}

// Create a copy of the Data record and add to memory table (cache)
SettingTable::Data* SettingTable::clone(const SettingTable::Data* e)
{
    Data* entity = create();
    *entity = *e;
    entity->id(-1);
    return entity;
}

// Save the Data record to the database table.
// Either create a new record or update the existing record.
// Remove old record from the memory table (cache).
bool SettingTable::save(SettingTable::Data* entity)
{
    wxString sql = wxEmptyString;
    if (entity->id() <= 0) {
        //  new & insert
        sql = "INSERT INTO SETTING_V1(SETTINGNAME, SETTINGVALUE, SETTINGID) VALUES(?, ?, ?)";
    }
    else {
        sql = "UPDATE SETTING_V1 SET SETTINGNAME = ?, SETTINGVALUE = ? WHERE SETTINGID = ?";
    }

    try {
        wxSQLite3Statement stmt = m_db->PrepareStatement(sql);

        stmt.Bind(1, entity->SETTINGNAME);
        stmt.Bind(2, entity->SETTINGVALUE);
        stmt.Bind(3, entity->id() > 0 ? entity->SETTINGID : newId());

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
        wxLogError("SETTING_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
        return false;
    }

    if (entity->id() <= 0) {
        entity->id(m_db->GetLastRowId());
        m_cache_index.insert(std::make_pair(entity->id(), entity));
    }
    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool SettingTable::remove(const int64 id)
{
    if (id <= 0) return false;
    try {
        wxString sql = "DELETE FROM SETTING_V1 WHERE SETTINGID = ?";
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
        wxLogError("SETTING_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool SettingTable::remove(SettingTable::Data* entity)
{
    if (remove(entity->id())) {
        entity->id(-1);
        return true;
    }

    return false;
}

// Search the memory table (Cache) for the data record.
// If not found in memory, search the database and update the cache.
SettingTable::Data* SettingTable::cache_id(const int64 id)
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
SettingTable::Data* SettingTable::get_id(const int64 id)
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
const SettingTable::Data_Set SettingTable::get_all(const COLUMN col, const bool asc)
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
