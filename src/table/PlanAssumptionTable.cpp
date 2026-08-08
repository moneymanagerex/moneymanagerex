// -*- C++ -*-
//=============================================================================
/**
 *      Copyright: (c) 2013-2026 Guan Lisheng (guanlisheng@gmail.com)
 *      Copyright: (c) 2017-2018 Stefano Giorgio (stef145g)
 *      Copyright: (c) 2022      Mark Whalley (mark@ipx.co.uk)
 *      Copyright: (c) 2026      George Ef (george.a.ef@gmail.com)
 *
 *      PlanAssumptionTable.cpp
 *
 *      Implementation of the interface to database table PLANASSUMPTION_V1
 *
 *      @author [sqlite2cpp.py]
 *
 *      Revision History:
 *          AUTO GENERATED at 2026-08-08 11:48:49.752940.
 *          DO NOT EDIT!
 */
//=============================================================================

#include "_TableFactory.tpp"
#include "PlanAssumptionTable.h"
#include "data/PlanAssumptionData.h"

template class TableFactory<PlanAssumptionTable, PlanAssumptionData>;
template class mmCache<int64, PlanAssumptionData>;

// List of column names in database table PLANASSUMPTION_V1,
// in the order of PlanAssumptionCol::COL_ID.
const wxArrayString PlanAssumptionCol::s_col_name_a = {
    "ASSUMPTIONID",
    "ASSUMPTIONNAME",
    "KIND",
    "VALUE",
    "SCOPEKEY",
    "NOTES",
    "ASOFDATE",
    "ACTIVE"
};

const PlanAssumptionCol::COL_ID PlanAssumptionCol::s_primary_id = COL_ID_ASSUMPTIONID;
const wxString PlanAssumptionCol::s_primary_name = s_col_name_a[COL_ID_ASSUMPTIONID];

// convenience variables
const wxString PlanAssumptionCol::NAME_ASSUMPTIONID = s_col_name_a[COL_ID_ASSUMPTIONID];
const wxString PlanAssumptionCol::NAME_ASSUMPTIONNAME = s_col_name_a[COL_ID_ASSUMPTIONNAME];
const wxString PlanAssumptionCol::NAME_KIND = s_col_name_a[COL_ID_KIND];
const wxString PlanAssumptionCol::NAME_VALUE = s_col_name_a[COL_ID_VALUE];
const wxString PlanAssumptionCol::NAME_SCOPEKEY = s_col_name_a[COL_ID_SCOPEKEY];
const wxString PlanAssumptionCol::NAME_NOTES = s_col_name_a[COL_ID_NOTES];
const wxString PlanAssumptionCol::NAME_ASOFDATE = s_col_name_a[COL_ID_ASOFDATE];
const wxString PlanAssumptionCol::NAME_ACTIVE = s_col_name_a[COL_ID_ACTIVE];

PlanAssumptionRow::PlanAssumptionRow()
{
    ASSUMPTIONID = -1;
    VALUE = 0.0;
    ACTIVE = -1;
}

// Bind a Row record to database insert statement.
void PlanAssumptionRow::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    stmt.Bind(1, ASSUMPTIONNAME);
    stmt.Bind(2, KIND);
    stmt.Bind(3, VALUE);
    stmt.Bind(4, SCOPEKEY);
    stmt.Bind(5, NOTES);
    stmt.Bind(6, ASOFDATE);
    stmt.Bind(7, ACTIVE);
    stmt.Bind(8, id);
}

PlanAssumptionRow& PlanAssumptionRow::from_select_result(wxSQLite3ResultSet& q)
{
    ASSUMPTIONID = q.GetInt64(0);
    ASSUMPTIONNAME = q.GetString(1);
    KIND = q.GetString(2);
    VALUE = q.GetDouble(3);
    SCOPEKEY = q.GetString(4);
    NOTES = q.GetString(5);
    ASOFDATE = q.GetString(6);
    ACTIVE = q.GetInt64(7);

    return *this;
}

// Return the data record as a json string
wxString PlanAssumptionRow::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();			
    as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void PlanAssumptionRow::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("ASSUMPTIONID");
    json_writer.Int64(ASSUMPTIONID.GetValue());

    json_writer.Key("ASSUMPTIONNAME");
    json_writer.String(ASSUMPTIONNAME.utf8_str());

    json_writer.Key("KIND");
    json_writer.String(KIND.utf8_str());

    json_writer.Key("VALUE");
    json_writer.Double(VALUE);

    json_writer.Key("SCOPEKEY");
    json_writer.String(SCOPEKEY.utf8_str());

    json_writer.Key("NOTES");
    json_writer.String(NOTES.utf8_str());

    json_writer.Key("ASOFDATE");
    json_writer.String(ASOFDATE.utf8_str());

    json_writer.Key("ACTIVE");
    json_writer.Int64(ACTIVE.GetValue());
}

row_t PlanAssumptionRow::to_html_row() const
{
    row_t row;

    row(L"ASSUMPTIONID") = ASSUMPTIONID.GetValue();
    row(L"ASSUMPTIONNAME") = ASSUMPTIONNAME;
    row(L"KIND") = KIND;
    row(L"VALUE") = VALUE;
    row(L"SCOPEKEY") = SCOPEKEY;
    row(L"NOTES") = NOTES;
    row(L"ASOFDATE") = ASOFDATE;
    row(L"ACTIVE") = ACTIVE.GetValue();

    return row;
}

void PlanAssumptionRow::to_html_template(html_template& t) const
{
    t(L"ASSUMPTIONID") = ASSUMPTIONID.GetValue();
    t(L"ASSUMPTIONNAME") = ASSUMPTIONNAME;
    t(L"KIND") = KIND;
    t(L"VALUE") = VALUE;
    t(L"SCOPEKEY") = SCOPEKEY;
    t(L"NOTES") = NOTES;
    t(L"ASOFDATE") = ASOFDATE;
    t(L"ACTIVE") = ACTIVE.GetValue();
}

bool PlanAssumptionRow::equals(const PlanAssumptionRow* other) const
{
    if ( ASSUMPTIONID != other->ASSUMPTIONID) return false;
    if (!ASSUMPTIONNAME.IsSameAs(other->ASSUMPTIONNAME)) return false;
    if (!KIND.IsSameAs(other->KIND)) return false;
    if ( VALUE != other->VALUE) return false;
    if (!SCOPEKEY.IsSameAs(other->SCOPEKEY)) return false;
    if (!NOTES.IsSameAs(other->NOTES)) return false;
    if (!ASOFDATE.IsSameAs(other->ASOFDATE)) return false;
    if ( ACTIVE != other->ACTIVE) return false;

    return true;
}

PlanAssumptionTable::PlanAssumptionTable()
{
    m_table_name = "PLANASSUMPTION_V1";

    m_create_query = "CREATE TABLE PLANASSUMPTION_V1(ASSUMPTIONID integer primary key, ASSUMPTIONNAME TEXT COLLATE NOCASE NOT NULL UNIQUE, KIND TEXT NOT NULL /* SharePrice, TaxRate, Inflation, ExchangeRate, Generic */, VALUE numeric NOT NULL, SCOPEKEY TEXT, NOTES TEXT, ASOFDATE TEXT, ACTIVE integer DEFAULT 1)";

    m_drop_query = "DROP TABLE IF EXISTS PLANASSUMPTION_V1";

    m_index_query_a = {
        "CREATE INDEX IF NOT EXISTS IDX_PLANASSUMPTION_KIND ON PLANASSUMPTION_V1(KIND)"
    };

    m_insert_query = "INSERT INTO PLANASSUMPTION_V1(ASSUMPTIONNAME, KIND, VALUE, SCOPEKEY, NOTES, ASOFDATE, ACTIVE, ASSUMPTIONID) VALUES(?, ?, ?, ?, ?, ?, ?, ?)";

    m_update_query = "UPDATE PLANASSUMPTION_V1 SET ASSUMPTIONNAME = ?, KIND = ?, VALUE = ?, SCOPEKEY = ?, NOTES = ?, ASOFDATE = ?, ACTIVE = ? WHERE ASSUMPTIONID = ?";

    m_delete_query = "DELETE FROM PLANASSUMPTION_V1 WHERE ASSUMPTIONID = ?";

    m_select_query = "SELECT ASSUMPTIONID, ASSUMPTIONNAME, KIND, VALUE, SCOPEKEY, NOTES, ASOFDATE, ACTIVE FROM PLANASSUMPTION_V1";
}
