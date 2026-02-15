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

#include "ScheduledTable.h"

ScheduledTable::Data::Data()
{
    BDID = -1;
    ACCOUNTID = -1;
    TOACCOUNTID = -1;
    PAYEEID = -1;
    TRANSAMOUNT = 0.0;
    CATEGID = -1;
    FOLLOWUPID = -1;
    TOTRANSAMOUNT = 0.0;
    REPEATS = -1;
    NUMOCCURRENCES = -1;
    COLOR = -1;
}

ScheduledTable::Data::Data(wxSQLite3ResultSet& q)
{
    BDID = q.GetInt64(0);
    ACCOUNTID = q.GetInt64(1);
    TOACCOUNTID = q.GetInt64(2);
    PAYEEID = q.GetInt64(3);
    TRANSCODE = q.GetString(4);
    TRANSAMOUNT = q.GetDouble(5);
    STATUS = q.GetString(6);
    TRANSACTIONNUMBER = q.GetString(7);
    NOTES = q.GetString(8);
    CATEGID = q.GetInt64(9);
    TRANSDATE = q.GetString(10);
    FOLLOWUPID = q.GetInt64(11);
    TOTRANSAMOUNT = q.GetDouble(12);
    REPEATS = q.GetInt64(13);
    NEXTOCCURRENCEDATE = q.GetString(14);
    NUMOCCURRENCES = q.GetInt64(15);
    COLOR = q.GetInt64(16);
}

bool ScheduledTable::Data::equals(const ScheduledTable::Data* r) const
{
    if (BDID != r->BDID) return false;
    if (ACCOUNTID != r->ACCOUNTID) return false;
    if (TOACCOUNTID != r->TOACCOUNTID) return false;
    if (PAYEEID != r->PAYEEID) return false;
    if (!TRANSCODE.IsSameAs(r->TRANSCODE)) return false;
    if (TRANSAMOUNT != r->TRANSAMOUNT) return false;
    if (!STATUS.IsSameAs(r->STATUS)) return false;
    if (!TRANSACTIONNUMBER.IsSameAs(r->TRANSACTIONNUMBER)) return false;
    if (!NOTES.IsSameAs(r->NOTES)) return false;
    if (CATEGID != r->CATEGID) return false;
    if (!TRANSDATE.IsSameAs(r->TRANSDATE)) return false;
    if (FOLLOWUPID != r->FOLLOWUPID) return false;
    if (TOTRANSAMOUNT != r->TOTRANSAMOUNT) return false;
    if (REPEATS != r->REPEATS) return false;
    if (!NEXTOCCURRENCEDATE.IsSameAs(r->NEXTOCCURRENCEDATE)) return false;
    if (NUMOCCURRENCES != r->NUMOCCURRENCES) return false;
    if (COLOR != r->COLOR) return false;

    return true;
}

ScheduledTable::Data& ScheduledTable::Data::operator=(const ScheduledTable::Data& other)
{
    if (this == &other) return *this;

    BDID = other.BDID;
    ACCOUNTID = other.ACCOUNTID;
    TOACCOUNTID = other.TOACCOUNTID;
    PAYEEID = other.PAYEEID;
    TRANSCODE = other.TRANSCODE;
    TRANSAMOUNT = other.TRANSAMOUNT;
    STATUS = other.STATUS;
    TRANSACTIONNUMBER = other.TRANSACTIONNUMBER;
    NOTES = other.NOTES;
    CATEGID = other.CATEGID;
    TRANSDATE = other.TRANSDATE;
    FOLLOWUPID = other.FOLLOWUPID;
    TOTRANSAMOUNT = other.TOTRANSAMOUNT;
    REPEATS = other.REPEATS;
    NEXTOCCURRENCEDATE = other.NEXTOCCURRENCEDATE;
    NUMOCCURRENCES = other.NUMOCCURRENCES;
    COLOR = other.COLOR;

    return *this;
}

// Return the data record as a json string
wxString ScheduledTable::Data::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();			
    this->as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void ScheduledTable::Data::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("BDID");
    json_writer.Int64(this->BDID.GetValue());
    json_writer.Key("ACCOUNTID");
    json_writer.Int64(this->ACCOUNTID.GetValue());
    json_writer.Key("TOACCOUNTID");
    json_writer.Int64(this->TOACCOUNTID.GetValue());
    json_writer.Key("PAYEEID");
    json_writer.Int64(this->PAYEEID.GetValue());
    json_writer.Key("TRANSCODE");
    json_writer.String(this->TRANSCODE.utf8_str());
    json_writer.Key("TRANSAMOUNT");
    json_writer.Double(this->TRANSAMOUNT);
    json_writer.Key("STATUS");
    json_writer.String(this->STATUS.utf8_str());
    json_writer.Key("TRANSACTIONNUMBER");
    json_writer.String(this->TRANSACTIONNUMBER.utf8_str());
    json_writer.Key("NOTES");
    json_writer.String(this->NOTES.utf8_str());
    json_writer.Key("CATEGID");
    json_writer.Int64(this->CATEGID.GetValue());
    json_writer.Key("TRANSDATE");
    json_writer.String(this->TRANSDATE.utf8_str());
    json_writer.Key("FOLLOWUPID");
    json_writer.Int64(this->FOLLOWUPID.GetValue());
    json_writer.Key("TOTRANSAMOUNT");
    json_writer.Double(this->TOTRANSAMOUNT);
    json_writer.Key("REPEATS");
    json_writer.Int64(this->REPEATS.GetValue());
    json_writer.Key("NEXTOCCURRENCEDATE");
    json_writer.String(this->NEXTOCCURRENCEDATE.utf8_str());
    json_writer.Key("NUMOCCURRENCES");
    json_writer.Int64(this->NUMOCCURRENCES.GetValue());
    json_writer.Key("COLOR");
    json_writer.Int64(this->COLOR.GetValue());
}

row_t ScheduledTable::Data::to_row_t() const
{
    row_t row;

    row(L"BDID") = BDID.GetValue();
    row(L"ACCOUNTID") = ACCOUNTID.GetValue();
    row(L"TOACCOUNTID") = TOACCOUNTID.GetValue();
    row(L"PAYEEID") = PAYEEID.GetValue();
    row(L"TRANSCODE") = TRANSCODE;
    row(L"TRANSAMOUNT") = TRANSAMOUNT;
    row(L"STATUS") = STATUS;
    row(L"TRANSACTIONNUMBER") = TRANSACTIONNUMBER;
    row(L"NOTES") = NOTES;
    row(L"CATEGID") = CATEGID.GetValue();
    row(L"TRANSDATE") = TRANSDATE;
    row(L"FOLLOWUPID") = FOLLOWUPID.GetValue();
    row(L"TOTRANSAMOUNT") = TOTRANSAMOUNT;
    row(L"REPEATS") = REPEATS.GetValue();
    row(L"NEXTOCCURRENCEDATE") = NEXTOCCURRENCEDATE;
    row(L"NUMOCCURRENCES") = NUMOCCURRENCES.GetValue();
    row(L"COLOR") = COLOR.GetValue();

    return row;
}

void ScheduledTable::Data::to_template(html_template& t) const
{
    t(L"BDID") = BDID.GetValue();
    t(L"ACCOUNTID") = ACCOUNTID.GetValue();
    t(L"TOACCOUNTID") = TOACCOUNTID.GetValue();
    t(L"PAYEEID") = PAYEEID.GetValue();
    t(L"TRANSCODE") = TRANSCODE;
    t(L"TRANSAMOUNT") = TRANSAMOUNT;
    t(L"STATUS") = STATUS;
    t(L"TRANSACTIONNUMBER") = TRANSACTIONNUMBER;
    t(L"NOTES") = NOTES;
    t(L"CATEGID") = CATEGID.GetValue();
    t(L"TRANSDATE") = TRANSDATE;
    t(L"FOLLOWUPID") = FOLLOWUPID.GetValue();
    t(L"TOTRANSAMOUNT") = TOTRANSAMOUNT;
    t(L"REPEATS") = REPEATS.GetValue();
    t(L"NEXTOCCURRENCEDATE") = NEXTOCCURRENCEDATE;
    t(L"NUMOCCURRENCES") = NUMOCCURRENCES.GetValue();
    t(L"COLOR") = COLOR.GetValue();
}

void ScheduledTable::Data::destroy()
{
    delete this;
}

// Return the data records as a json array string
wxString ScheduledTable::Data_Set::to_json() const
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

ScheduledTable::ScheduledTable() :
    fake_(new Data())
{
    m_table_name = "BILLSDEPOSITS_V1";
    m_query_select = "SELECT BDID, ACCOUNTID, TOACCOUNTID, PAYEEID, TRANSCODE, TRANSAMOUNT, STATUS, TRANSACTIONNUMBER, NOTES, CATEGID, TRANSDATE, FOLLOWUPID, TOTRANSAMOUNT, REPEATS, NEXTOCCURRENCEDATE, NUMOCCURRENCES, COLOR FROM BILLSDEPOSITS_V1 ";
}

// Destructor: clears any data records stored in memory
ScheduledTable::~ScheduledTable()
{
    delete this->fake_;
    destroy_cache();
}

// Remove all records stored in memory (cache) for the table
void ScheduledTable::destroy_cache()
{
    std::for_each(m_cache.begin(), m_cache.end(), std::mem_fn(&Data::destroy));
    m_cache.clear();
    m_cache_index.clear(); // no memory release since it just stores pointer and the according objects are in cache
}

// Creates the database table if the table does not exist
bool ScheduledTable::ensure_table()
{
    if (!table_exists()) {
        try {
            m_db->ExecuteUpdate("CREATE TABLE BILLSDEPOSITS_V1(BDID integer primary key, ACCOUNTID integer NOT NULL, TOACCOUNTID integer, PAYEEID integer NOT NULL, TRANSCODE TEXT NOT NULL /* Withdrawal, Deposit, Transfer */, TRANSAMOUNT numeric NOT NULL, STATUS TEXT /* None, Reconciled, Void, Follow up, Duplicate */, TRANSACTIONNUMBER TEXT, NOTES TEXT, CATEGID integer, TRANSDATE TEXT, FOLLOWUPID integer, TOTRANSAMOUNT numeric, REPEATS integer, NEXTOCCURRENCEDATE TEXT, NUMOCCURRENCES integer, COLOR integer DEFAULT -1)");
            this->ensure_data();
        }
        catch(const wxSQLite3Exception &e) {
            wxLogError("BILLSDEPOSITS_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }
    }

    this->ensure_index();

    return true;
}

bool ScheduledTable::ensure_index()
{
    try {
        m_db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_BILLSDEPOSITS_ACCOUNT ON BILLSDEPOSITS_V1 (ACCOUNTID, TOACCOUNTID)");
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("BILLSDEPOSITS_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

void ScheduledTable::ensure_data()
{
    m_db->Begin();
    m_db->Commit();
}

// Returns the column name as a string
wxString ScheduledTable::column_to_name(const ScheduledTable::COLUMN col)
{
    switch(col) {
        case COL_BDID: return "BDID";
        case COL_ACCOUNTID: return "ACCOUNTID";
        case COL_TOACCOUNTID: return "TOACCOUNTID";
        case COL_PAYEEID: return "PAYEEID";
        case COL_TRANSCODE: return "TRANSCODE";
        case COL_TRANSAMOUNT: return "TRANSAMOUNT";
        case COL_STATUS: return "STATUS";
        case COL_TRANSACTIONNUMBER: return "TRANSACTIONNUMBER";
        case COL_NOTES: return "NOTES";
        case COL_CATEGID: return "CATEGID";
        case COL_TRANSDATE: return "TRANSDATE";
        case COL_FOLLOWUPID: return "FOLLOWUPID";
        case COL_TOTRANSAMOUNT: return "TOTRANSAMOUNT";
        case COL_REPEATS: return "REPEATS";
        case COL_NEXTOCCURRENCEDATE: return "NEXTOCCURRENCEDATE";
        case COL_NUMOCCURRENCES: return "NUMOCCURRENCES";
        case COL_COLOR: return "COLOR";
        default: break;
    }

    return "UNKNOWN";
}

// Returns the column number from the given column name
ScheduledTable::COLUMN ScheduledTable::name_to_column(const wxString& name)
{
    if (name == "BDID") return COL_BDID;
    else if (name == "ACCOUNTID") return COL_ACCOUNTID;
    else if (name == "TOACCOUNTID") return COL_TOACCOUNTID;
    else if (name == "PAYEEID") return COL_PAYEEID;
    else if (name == "TRANSCODE") return COL_TRANSCODE;
    else if (name == "TRANSAMOUNT") return COL_TRANSAMOUNT;
    else if (name == "STATUS") return COL_STATUS;
    else if (name == "TRANSACTIONNUMBER") return COL_TRANSACTIONNUMBER;
    else if (name == "NOTES") return COL_NOTES;
    else if (name == "CATEGID") return COL_CATEGID;
    else if (name == "TRANSDATE") return COL_TRANSDATE;
    else if (name == "FOLLOWUPID") return COL_FOLLOWUPID;
    else if (name == "TOTRANSAMOUNT") return COL_TOTRANSAMOUNT;
    else if (name == "REPEATS") return COL_REPEATS;
    else if (name == "NEXTOCCURRENCEDATE") return COL_NEXTOCCURRENCEDATE;
    else if (name == "NUMOCCURRENCES") return COL_NUMOCCURRENCES;
    else if (name == "COLOR") return COL_COLOR;

    return COLUMN(-1);
}

// Create a new Data record and add to memory table (cache)
ScheduledTable::Data* ScheduledTable::create()
{
    Data* entity = new Data();
    m_cache.push_back(entity);
    return entity;
}

// Create a copy of the Data record and add to memory table (cache)
ScheduledTable::Data* ScheduledTable::clone(const ScheduledTable::Data* e)
{
    Data* entity = create();
    *entity = *e;
    entity->id(-1);
    return entity;
}

// Save the Data record to the database table.
// Either create a new record or update the existing record.
// Remove old record from the memory table (cache).
bool ScheduledTable::save(ScheduledTable::Data* entity)
{
    wxString sql = wxEmptyString;
    if (entity->id() <= 0) {
        //  new & insert
        sql = "INSERT INTO BILLSDEPOSITS_V1(ACCOUNTID, TOACCOUNTID, PAYEEID, TRANSCODE, TRANSAMOUNT, STATUS, TRANSACTIONNUMBER, NOTES, CATEGID, TRANSDATE, FOLLOWUPID, TOTRANSAMOUNT, REPEATS, NEXTOCCURRENCEDATE, NUMOCCURRENCES, COLOR, BDID) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    }
    else {
        sql = "UPDATE BILLSDEPOSITS_V1 SET ACCOUNTID = ?, TOACCOUNTID = ?, PAYEEID = ?, TRANSCODE = ?, TRANSAMOUNT = ?, STATUS = ?, TRANSACTIONNUMBER = ?, NOTES = ?, CATEGID = ?, TRANSDATE = ?, FOLLOWUPID = ?, TOTRANSAMOUNT = ?, REPEATS = ?, NEXTOCCURRENCEDATE = ?, NUMOCCURRENCES = ?, COLOR = ? WHERE BDID = ?";
    }

    try {
        wxSQLite3Statement stmt = m_db->PrepareStatement(sql);

        stmt.Bind(1, entity->ACCOUNTID);
        stmt.Bind(2, entity->TOACCOUNTID);
        stmt.Bind(3, entity->PAYEEID);
        stmt.Bind(4, entity->TRANSCODE);
        stmt.Bind(5, entity->TRANSAMOUNT);
        stmt.Bind(6, entity->STATUS);
        stmt.Bind(7, entity->TRANSACTIONNUMBER);
        stmt.Bind(8, entity->NOTES);
        stmt.Bind(9, entity->CATEGID);
        stmt.Bind(10, entity->TRANSDATE);
        stmt.Bind(11, entity->FOLLOWUPID);
        stmt.Bind(12, entity->TOTRANSAMOUNT);
        stmt.Bind(13, entity->REPEATS);
        stmt.Bind(14, entity->NEXTOCCURRENCEDATE);
        stmt.Bind(15, entity->NUMOCCURRENCES);
        stmt.Bind(16, entity->COLOR);
        stmt.Bind(17, entity->id() > 0 ? entity->BDID : newId());

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
        wxLogError("BILLSDEPOSITS_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
        return false;
    }

    if (entity->id() <= 0) {
        entity->id(m_db->GetLastRowId());
        m_cache_index.insert(std::make_pair(entity->id(), entity));
    }
    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool ScheduledTable::remove(const int64 id)
{
    if (id <= 0) return false;
    try {
        wxString sql = "DELETE FROM BILLSDEPOSITS_V1 WHERE BDID = ?";
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
        wxLogError("BILLSDEPOSITS_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool ScheduledTable::remove(ScheduledTable::Data* entity)
{
    if (remove(entity->id())) {
        entity->id(-1);
        return true;
    }

    return false;
}

// Search the memory table (Cache) for the data record.
// If not found in memory, search the database and update the cache.
ScheduledTable::Data* ScheduledTable::cache_id(const int64 id)
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
ScheduledTable::Data* ScheduledTable::get_id(const int64 id)
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
const ScheduledTable::Data_Set ScheduledTable::get_all(const COLUMN col, const bool asc)
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
