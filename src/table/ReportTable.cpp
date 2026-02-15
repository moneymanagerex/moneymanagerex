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

#include "ReportTable.h"

ReportTable::Data::Data()
{
    REPORTID = -1;
    ACTIVE = -1;
}

ReportTable::Data::Data(wxSQLite3ResultSet& q)
{
    REPORTID = q.GetInt64(0);
    REPORTNAME = q.GetString(1);
    GROUPNAME = q.GetString(2);
    ACTIVE = q.GetInt64(3);
    SQLCONTENT = q.GetString(4);
    LUACONTENT = q.GetString(5);
    TEMPLATECONTENT = q.GetString(6);
    DESCRIPTION = q.GetString(7);
}

bool ReportTable::Data::equals(const ReportTable::Data* r) const
{
    if (REPORTID != r->REPORTID) return false;
    if (!REPORTNAME.IsSameAs(r->REPORTNAME)) return false;
    if (!GROUPNAME.IsSameAs(r->GROUPNAME)) return false;
    if (ACTIVE != r->ACTIVE) return false;
    if (!SQLCONTENT.IsSameAs(r->SQLCONTENT)) return false;
    if (!LUACONTENT.IsSameAs(r->LUACONTENT)) return false;
    if (!TEMPLATECONTENT.IsSameAs(r->TEMPLATECONTENT)) return false;
    if (!DESCRIPTION.IsSameAs(r->DESCRIPTION)) return false;

    return true;
}

ReportTable::Data& ReportTable::Data::operator=(const ReportTable::Data& other)
{
    if (this == &other) return *this;

    REPORTID = other.REPORTID;
    REPORTNAME = other.REPORTNAME;
    GROUPNAME = other.GROUPNAME;
    ACTIVE = other.ACTIVE;
    SQLCONTENT = other.SQLCONTENT;
    LUACONTENT = other.LUACONTENT;
    TEMPLATECONTENT = other.TEMPLATECONTENT;
    DESCRIPTION = other.DESCRIPTION;

    return *this;
}

// Return the data record as a json string
wxString ReportTable::Data::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();			
    this->as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void ReportTable::Data::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("REPORTID");
    json_writer.Int64(this->REPORTID.GetValue());
    json_writer.Key("REPORTNAME");
    json_writer.String(this->REPORTNAME.utf8_str());
    json_writer.Key("GROUPNAME");
    json_writer.String(this->GROUPNAME.utf8_str());
    json_writer.Key("ACTIVE");
    json_writer.Int64(this->ACTIVE.GetValue());
    json_writer.Key("SQLCONTENT");
    json_writer.String(this->SQLCONTENT.utf8_str());
    json_writer.Key("LUACONTENT");
    json_writer.String(this->LUACONTENT.utf8_str());
    json_writer.Key("TEMPLATECONTENT");
    json_writer.String(this->TEMPLATECONTENT.utf8_str());
    json_writer.Key("DESCRIPTION");
    json_writer.String(this->DESCRIPTION.utf8_str());
}

row_t ReportTable::Data::to_row_t() const
{
    row_t row;

    row(L"REPORTID") = REPORTID.GetValue();
    row(L"REPORTNAME") = REPORTNAME;
    row(L"GROUPNAME") = GROUPNAME;
    row(L"ACTIVE") = ACTIVE.GetValue();
    row(L"SQLCONTENT") = SQLCONTENT;
    row(L"LUACONTENT") = LUACONTENT;
    row(L"TEMPLATECONTENT") = TEMPLATECONTENT;
    row(L"DESCRIPTION") = DESCRIPTION;

    return row;
}

void ReportTable::Data::to_template(html_template& t) const
{
    t(L"REPORTID") = REPORTID.GetValue();
    t(L"REPORTNAME") = REPORTNAME;
    t(L"GROUPNAME") = GROUPNAME;
    t(L"ACTIVE") = ACTIVE.GetValue();
    t(L"SQLCONTENT") = SQLCONTENT;
    t(L"LUACONTENT") = LUACONTENT;
    t(L"TEMPLATECONTENT") = TEMPLATECONTENT;
    t(L"DESCRIPTION") = DESCRIPTION;
}

void ReportTable::Data::destroy()
{
    delete this;
}

// Return the data records as a json array string
wxString ReportTable::Data_Set::to_json() const
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

ReportTable::ReportTable() :
    fake_(new Data())
{
    m_table_name = "REPORT_V1";
    m_query_select = "SELECT REPORTID, REPORTNAME, GROUPNAME, ACTIVE, SQLCONTENT, LUACONTENT, TEMPLATECONTENT, DESCRIPTION FROM REPORT_V1 ";
}

// Destructor: clears any data records stored in memory
ReportTable::~ReportTable()
{
    delete this->fake_;
    destroy_cache();
}

// Remove all records stored in memory (cache) for the table
void ReportTable::destroy_cache()
{
    std::for_each(m_cache.begin(), m_cache.end(), std::mem_fn(&Data::destroy));
    m_cache.clear();
    m_cache_index.clear(); // no memory release since it just stores pointer and the according objects are in cache
}

// Creates the database table if the table does not exist
bool ReportTable::ensure_table()
{
    if (!table_exists()) {
        try {
            m_db->ExecuteUpdate("CREATE TABLE REPORT_V1(REPORTID integer not null primary key, REPORTNAME TEXT COLLATE NOCASE NOT NULL UNIQUE, GROUPNAME TEXT COLLATE NOCASE, ACTIVE integer, SQLCONTENT TEXT, LUACONTENT TEXT, TEMPLATECONTENT TEXT, DESCRIPTION TEXT)");
            this->ensure_data();
        }
        catch(const wxSQLite3Exception &e) {
            wxLogError("REPORT_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }
    }

    this->ensure_index();

    return true;
}

bool ReportTable::ensure_index()
{
    try {
        m_db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS INDEX_REPORT_NAME ON REPORT_V1(REPORTNAME)");
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("REPORT_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

void ReportTable::ensure_data()
{
    m_db->Begin();
    m_db->Commit();
}

// Returns the column name as a string
wxString ReportTable::column_to_name(const ReportTable::COLUMN col)
{
    switch(col) {
        case COL_REPORTID: return "REPORTID";
        case COL_REPORTNAME: return "REPORTNAME";
        case COL_GROUPNAME: return "GROUPNAME";
        case COL_ACTIVE: return "ACTIVE";
        case COL_SQLCONTENT: return "SQLCONTENT";
        case COL_LUACONTENT: return "LUACONTENT";
        case COL_TEMPLATECONTENT: return "TEMPLATECONTENT";
        case COL_DESCRIPTION: return "DESCRIPTION";
        default: break;
    }

    return "UNKNOWN";
}

// Returns the column number from the given column name
ReportTable::COLUMN ReportTable::name_to_column(const wxString& name)
{
    if (name == "REPORTID") return COL_REPORTID;
    else if (name == "REPORTNAME") return COL_REPORTNAME;
    else if (name == "GROUPNAME") return COL_GROUPNAME;
    else if (name == "ACTIVE") return COL_ACTIVE;
    else if (name == "SQLCONTENT") return COL_SQLCONTENT;
    else if (name == "LUACONTENT") return COL_LUACONTENT;
    else if (name == "TEMPLATECONTENT") return COL_TEMPLATECONTENT;
    else if (name == "DESCRIPTION") return COL_DESCRIPTION;

    return COLUMN(-1);
}

// Create a new Data record and add to memory table (cache)
ReportTable::Data* ReportTable::create()
{
    Data* entity = new Data();
    m_cache.push_back(entity);
    return entity;
}

// Create a copy of the Data record and add to memory table (cache)
ReportTable::Data* ReportTable::clone(const ReportTable::Data* e)
{
    Data* entity = create();
    *entity = *e;
    entity->id(-1);
    return entity;
}

// Save the Data record to the database table.
// Either create a new record or update the existing record.
// Remove old record from the memory table (cache).
bool ReportTable::save(ReportTable::Data* entity)
{
    wxString sql = wxEmptyString;
    if (entity->id() <= 0) {
        //  new & insert
        sql = "INSERT INTO REPORT_V1(REPORTNAME, GROUPNAME, ACTIVE, SQLCONTENT, LUACONTENT, TEMPLATECONTENT, DESCRIPTION, REPORTID) VALUES(?, ?, ?, ?, ?, ?, ?, ?)";
    }
    else {
        sql = "UPDATE REPORT_V1 SET REPORTNAME = ?, GROUPNAME = ?, ACTIVE = ?, SQLCONTENT = ?, LUACONTENT = ?, TEMPLATECONTENT = ?, DESCRIPTION = ? WHERE REPORTID = ?";
    }

    try {
        wxSQLite3Statement stmt = m_db->PrepareStatement(sql);

        stmt.Bind(1, entity->REPORTNAME);
        stmt.Bind(2, entity->GROUPNAME);
        stmt.Bind(3, entity->ACTIVE);
        stmt.Bind(4, entity->SQLCONTENT);
        stmt.Bind(5, entity->LUACONTENT);
        stmt.Bind(6, entity->TEMPLATECONTENT);
        stmt.Bind(7, entity->DESCRIPTION);
        stmt.Bind(8, entity->id() > 0 ? entity->REPORTID : newId());

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
        wxLogError("REPORT_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
        return false;
    }

    if (entity->id() <= 0) {
        entity->id(m_db->GetLastRowId());
        m_cache_index.insert(std::make_pair(entity->id(), entity));
    }
    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool ReportTable::remove(const int64 id)
{
    if (id <= 0) return false;
    try {
        wxString sql = "DELETE FROM REPORT_V1 WHERE REPORTID = ?";
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
        wxLogError("REPORT_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool ReportTable::remove(ReportTable::Data* entity)
{
    if (remove(entity->id())) {
        entity->id(-1);
        return true;
    }

    return false;
}

// Search the memory table (Cache) for the data record.
// If not found in memory, search the database and update the cache.
ReportTable::Data* ReportTable::cache_id(const int64 id)
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
ReportTable::Data* ReportTable::get_id(const int64 id)
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
const ReportTable::Data_Set ReportTable::get_all(const COLUMN col, const bool asc)
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
