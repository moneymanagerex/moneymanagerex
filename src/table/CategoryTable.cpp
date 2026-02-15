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

#include "CategoryTable.h"

CategoryTable::Data::Data()
{
    CATEGID = -1;
    ACTIVE = -1;
    PARENTID = -1;
}

CategoryTable::Data::Data(wxSQLite3ResultSet& q)
{
    CATEGID = q.GetInt64(0);
    CATEGNAME = q.GetString(1);
    ACTIVE = q.GetInt64(2);
    PARENTID = q.GetInt64(3);
}

bool CategoryTable::Data::equals(const CategoryTable::Data* r) const
{
    if (CATEGID != r->CATEGID) return false;
    if (!CATEGNAME.IsSameAs(r->CATEGNAME)) return false;
    if (ACTIVE != r->ACTIVE) return false;
    if (PARENTID != r->PARENTID) return false;

    return true;
}

CategoryTable::Data& CategoryTable::Data::operator=(const CategoryTable::Data& other)
{
    if (this == &other) return *this;

    CATEGID = other.CATEGID;
    CATEGNAME = other.CATEGNAME;
    ACTIVE = other.ACTIVE;
    PARENTID = other.PARENTID;

    return *this;
}

// Return the data record as a json string
wxString CategoryTable::Data::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);
    json_writer.StartObject();			
    this->as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void CategoryTable::Data::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("CATEGID");
    json_writer.Int64(this->CATEGID.GetValue());
    json_writer.Key("CATEGNAME");
    json_writer.String(this->CATEGNAME.utf8_str());
    json_writer.Key("ACTIVE");
    json_writer.Int64(this->ACTIVE.GetValue());
    json_writer.Key("PARENTID");
    json_writer.Int64(this->PARENTID.GetValue());
}

row_t CategoryTable::Data::to_row_t() const
{
    row_t row;

    row(L"CATEGID") = CATEGID.GetValue();
    row(L"CATEGNAME") = CATEGNAME;
    row(L"ACTIVE") = ACTIVE.GetValue();
    row(L"PARENTID") = PARENTID.GetValue();

    return row;
}

void CategoryTable::Data::to_template(html_template& t) const
{
    t(L"CATEGID") = CATEGID.GetValue();
    t(L"CATEGNAME") = CATEGNAME;
    t(L"ACTIVE") = ACTIVE.GetValue();
    t(L"PARENTID") = PARENTID.GetValue();
}

void CategoryTable::Data::destroy()
{
    delete this;
}

// Return the data records as a json array string
wxString CategoryTable::Data_Set::to_json() const
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

CategoryTable::CategoryTable() :
    fake_(new Data())
{
    m_table_name = "CATEGORY_V1";
    m_query_select = "SELECT CATEGID, CATEGNAME, ACTIVE, PARENTID FROM CATEGORY_V1 ";
}

// Destructor: clears any data records stored in memory
CategoryTable::~CategoryTable()
{
    delete this->fake_;
    destroy_cache();
}

// Remove all records stored in memory (cache) for the table
void CategoryTable::destroy_cache()
{
    std::for_each(m_cache.begin(), m_cache.end(), std::mem_fn(&Data::destroy));
    m_cache.clear();
    m_cache_index.clear(); // no memory release since it just stores pointer and the according objects are in cache
}

// Creates the database table if the table does not exist
bool CategoryTable::ensure_table()
{
    if (!table_exists()) {
        try {
            m_db->ExecuteUpdate("CREATE TABLE CATEGORY_V1( CATEGID INTEGER PRIMARY KEY,  CATEGNAME TEXT NOT NULL COLLATE NOCASE,  ACTIVE INTEGER,  PARENTID INTEGER,  UNIQUE(CATEGNAME, PARENTID))");
            this->ensure_data();
        }
        catch(const wxSQLite3Exception &e) {
            wxLogError("CATEGORY_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }
    }

    this->ensure_index();

    return true;
}

bool CategoryTable::ensure_index()
{
    try {
        m_db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_CATEGORY_CATEGNAME ON CATEGORY_V1(CATEGNAME)");
        m_db->ExecuteUpdate("CREATE INDEX IF NOT EXISTS IDX_CATEGORY_CATEGNAME_PARENTID ON CATEGORY_V1(CATEGNAME, PARENTID)");
    }
    catch(const wxSQLite3Exception &e) {
        wxLogError("CATEGORY_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

void CategoryTable::ensure_data()
{
    m_db->Begin();
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('1', '%s', '1', '-1')", _("Bills")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('2', '%s', '1', '1')", _("Telephone")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('3', '%s', '1', '1')", _("Electricity")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('4', '%s', '1', '1')", _("Gas")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('5', '%s', '1', '1')", _("Internet")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('6', '%s', '1', '1')", _("Rent")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('7', '%s', '1', '1')", _("Cable TV")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('8', '%s', '1', '1')", _("Water")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('9', '%s', '1', '-1')", _("Food")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('10', '%s', '1', '9')", _("Groceries")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('11', '%s', '1', '9')", _("Dining out")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('12', '%s', '1', '-1')", _("Leisure")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('13', '%s', '1', '12')", _("Movies")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('14', '%s', '1', '12')", _("Video Rental")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('15', '%s', '1', '12')", _("Magazines")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('16', '%s', '1', '-1')", _("Automobile")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('17', '%s', '1', '16')", _("Maintenance")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('18', '%s', '1', '16')", _("Gas")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('19', '%s', '1', '16')", _("Parking")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('20', '%s', '1', '16')", _("Registration")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('21', '%s', '1', '-1')", _("Education")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('22', '%s', '1', '21')", _("Books")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('23', '%s', '1', '21')", _("Tuition")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('24', '%s', '1', '21')", _("Others")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('25', '%s', '1', '-1')", _("Homeneeds")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('26', '%s', '1', '25')", _("Clothing")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('27', '%s', '1', '25')", _("Furnishing")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('28', '%s', '1', '25')", _("Others")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('29', '%s', '1', '-1')", _("Healthcare")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('30', '%s', '1', '29')", _("Health")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('31', '%s', '1', '29')", _("Dental")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('32', '%s', '1', '29')", _("Eyecare")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('33', '%s', '1', '29')", _("Physician")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('34', '%s', '1', '29')", _("Prescriptions")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('35', '%s', '1', '-1')", _("Insurance")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('36', '%s', '1', '35')", _("Auto")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('37', '%s', '1', '35')", _("Life")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('38', '%s', '1', '35')", _("Home")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('39', '%s', '1', '35')", _("Health")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('40', '%s', '1', '-1')", _("Vacation")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('41', '%s', '1', '40')", _("Travel")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('42', '%s', '1', '40')", _("Lodging")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('43', '%s', '1', '40')", _("Sightseeing")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('44', '%s', '1', '-1')", _("Taxes")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('45', '%s', '1', '44')", _("Income Tax")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('46', '%s', '1', '44')", _("House Tax")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('47', '%s', '1', '44')", _("Water Tax")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('48', '%s', '1', '44')", _("Others")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('49', '%s', '1', '-1')", _("Miscellaneous")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('50', '%s', '1', '-1')", _("Gifts")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('51', '%s', '1', '-1')", _("Income")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('52', '%s', '1', '51')", _("Salary")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('53', '%s', '1', '51')", _("Reimbursement/Refunds")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('54', '%s', '1', '51')", _("Investment Income")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('55', '%s', '1', '-1')", _("Other Income")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('56', '%s', '1', '-1')", _("Other Expenses")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('57', '%s', '1', '-1')", _("Transfer")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('58', '%s', '1', '-1')", _("Investment")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('59', '%s', '1', '58')", _("Purchase")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('60', '%s', '1', '58')", _("Sale")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('61', '%s', '1', '58')", _("Dividend")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('62', '%s', '1', '58')", _("Capital Gains")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('63', '%s', '1', '58')", _("Brokerage Fees")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('64', '%s', '1', '58')", _("Interest")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('65', '%s', '1', '58')", _("Taxes")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('66', '%s', '1', '58')", _("Split")));
    m_db->ExecuteUpdate(wxString::Format("INSERT INTO CATEGORY_V1 VALUES ('67', '%s', '1', '58')", _("Merger")));
    m_db->Commit();
}

// Returns the column name as a string
wxString CategoryTable::column_to_name(const CategoryTable::COLUMN col)
{
    switch(col) {
        case COL_CATEGID: return "CATEGID";
        case COL_CATEGNAME: return "CATEGNAME";
        case COL_ACTIVE: return "ACTIVE";
        case COL_PARENTID: return "PARENTID";
        default: break;
    }

    return "UNKNOWN";
}

// Returns the column number from the given column name
CategoryTable::COLUMN CategoryTable::name_to_column(const wxString& name)
{
    if (name == "CATEGID") return COL_CATEGID;
    else if (name == "CATEGNAME") return COL_CATEGNAME;
    else if (name == "ACTIVE") return COL_ACTIVE;
    else if (name == "PARENTID") return COL_PARENTID;

    return COLUMN(-1);
}

// Create a new Data record and add to memory table (cache)
CategoryTable::Data* CategoryTable::create()
{
    Data* entity = new Data();
    m_cache.push_back(entity);
    return entity;
}

// Create a copy of the Data record and add to memory table (cache)
CategoryTable::Data* CategoryTable::clone(const CategoryTable::Data* e)
{
    Data* entity = create();
    *entity = *e;
    entity->id(-1);
    return entity;
}

// Save the Data record to the database table.
// Either create a new record or update the existing record.
// Remove old record from the memory table (cache).
bool CategoryTable::save(CategoryTable::Data* entity)
{
    wxString sql = wxEmptyString;
    if (entity->id() <= 0) {
        //  new & insert
        sql = "INSERT INTO CATEGORY_V1(CATEGNAME, ACTIVE, PARENTID, CATEGID) VALUES(?, ?, ?, ?)";
    }
    else {
        sql = "UPDATE CATEGORY_V1 SET CATEGNAME = ?, ACTIVE = ?, PARENTID = ? WHERE CATEGID = ?";
    }

    try {
        wxSQLite3Statement stmt = m_db->PrepareStatement(sql);

        stmt.Bind(1, entity->CATEGNAME);
        stmt.Bind(2, entity->ACTIVE);
        stmt.Bind(3, entity->PARENTID);
        stmt.Bind(4, entity->id() > 0 ? entity->CATEGID : newId());

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
        wxLogError("CATEGORY_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
        return false;
    }

    if (entity->id() <= 0) {
        entity->id(m_db->GetLastRowId());
        m_cache_index.insert(std::make_pair(entity->id(), entity));
    }
    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool CategoryTable::remove(const int64 id)
{
    if (id <= 0) return false;
    try {
        wxString sql = "DELETE FROM CATEGORY_V1 WHERE CATEGID = ?";
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
        wxLogError("CATEGORY_V1: Exception %s", e.GetMessage().utf8_str());
        return false;
    }

    return true;
}

// Remove the Data record from the database and the memory table (cache)
bool CategoryTable::remove(CategoryTable::Data* entity)
{
    if (remove(entity->id())) {
        entity->id(-1);
        return true;
    }

    return false;
}

// Search the memory table (Cache) for the data record.
// If not found in memory, search the database and update the cache.
CategoryTable::Data* CategoryTable::cache_id(const int64 id)
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
CategoryTable::Data* CategoryTable::get_id(const int64 id)
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
const CategoryTable::Data_Set CategoryTable::get_all(const COLUMN col, const bool asc)
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
