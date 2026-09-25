// -*- C++ -*-
//=============================================================================
/**
 *      Copyright: (c) 2013-2026 Guan Lisheng (guanlisheng@gmail.com)
 *      Copyright: (c) 2017-2018 Stefano Giorgio (stef145g)
 *      Copyright: (c) 2022      Mark Whalley (mark@ipx.co.uk)
 *      Copyright: (c) 2026      George Ef (george.a.ef@gmail.com)
 *
 *      PlanAssumptionGroupTable.cpp
 *
 *      Implementation of the interface to database table PLANASSUMPTIONGROUP_V1
 *
 *      @author [sqlite2cpp.py]
 *
 *      Revision History:
 *          AUTO GENERATED at 2026-08-10 13:39:52.649608.
 *          DO NOT EDIT!
 */
//=============================================================================

#include "_TableFactory.tpp"
#include "PlanAssumptionGroupTable.h"
#include "data/PlanAssumptionGroupData.h"

template class TableFactory<PlanAssumptionGroupTable, PlanAssumptionGroupData>;
template class mmCache<int64, PlanAssumptionGroupData>;

// List of column names in database table PLANASSUMPTIONGROUP_V1,
// in the order of PlanAssumptionGroupCol::COL_ID.
const wxArrayString PlanAssumptionGroupCol::s_col_name_a = {
    "GROUPID",
    "GROUPNAME",
    "KIND",
    "SCOPEKEY",
    "UNIT",
    "ACTIVEASSUMPTIONID",
    "NOTES",
    "ACTIVE"
};

const PlanAssumptionGroupCol::COL_ID PlanAssumptionGroupCol::s_primary_id = COL_ID_GROUPID;
const wxString PlanAssumptionGroupCol::s_primary_name = s_col_name_a[COL_ID_GROUPID];

// convenience variables
const wxString PlanAssumptionGroupCol::NAME_GROUPID = s_col_name_a[COL_ID_GROUPID];
const wxString PlanAssumptionGroupCol::NAME_GROUPNAME = s_col_name_a[COL_ID_GROUPNAME];
const wxString PlanAssumptionGroupCol::NAME_KIND = s_col_name_a[COL_ID_KIND];
const wxString PlanAssumptionGroupCol::NAME_SCOPEKEY = s_col_name_a[COL_ID_SCOPEKEY];
const wxString PlanAssumptionGroupCol::NAME_UNIT = s_col_name_a[COL_ID_UNIT];
const wxString PlanAssumptionGroupCol::NAME_ACTIVEASSUMPTIONID = s_col_name_a[COL_ID_ACTIVEASSUMPTIONID];
const wxString PlanAssumptionGroupCol::NAME_NOTES = s_col_name_a[COL_ID_NOTES];
const wxString PlanAssumptionGroupCol::NAME_ACTIVE = s_col_name_a[COL_ID_ACTIVE];

PlanAssumptionGroupRow::PlanAssumptionGroupRow()
{
    GROUPID = -1;
    ACTIVEASSUMPTIONID = -1;
    ACTIVE = -1;
}

// Bind a Row record to database insert statement.
void PlanAssumptionGroupRow::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    stmt.Bind(1, GROUPNAME);
    stmt.Bind(2, KIND);
    stmt.Bind(3, SCOPEKEY);
    stmt.Bind(4, UNIT);
    stmt.Bind(5, ACTIVEASSUMPTIONID);
    stmt.Bind(6, NOTES);
    stmt.Bind(7, ACTIVE);
    stmt.Bind(8, id);
}

PlanAssumptionGroupRow& PlanAssumptionGroupRow::from_select_result(wxSQLite3ResultSet& q)
{
    GROUPID = q.GetInt64(0);
    GROUPNAME = q.GetString(1);
    KIND = q.GetString(2);
    SCOPEKEY = q.GetString(3);
    UNIT = q.GetString(4);
    ACTIVEASSUMPTIONID = q.GetInt64(5);
    NOTES = q.GetString(6);
    ACTIVE = q.GetInt64(7);

    return *this;
}

// Return the data record as a json string
wxString PlanAssumptionGroupRow::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();			
    as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void PlanAssumptionGroupRow::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("GROUPID");
    json_writer.Int64(GROUPID.GetValue());

    json_writer.Key("GROUPNAME");
    json_writer.String(GROUPNAME.utf8_str());

    json_writer.Key("KIND");
    json_writer.String(KIND.utf8_str());

    json_writer.Key("SCOPEKEY");
    json_writer.String(SCOPEKEY.utf8_str());

    json_writer.Key("UNIT");
    json_writer.String(UNIT.utf8_str());

    json_writer.Key("ACTIVEASSUMPTIONID");
    json_writer.Int64(ACTIVEASSUMPTIONID.GetValue());

    json_writer.Key("NOTES");
    json_writer.String(NOTES.utf8_str());

    json_writer.Key("ACTIVE");
    json_writer.Int64(ACTIVE.GetValue());
}

row_t PlanAssumptionGroupRow::to_html_row() const
{
    row_t row;

    row(L"GROUPID") = GROUPID.GetValue();
    row(L"GROUPNAME") = GROUPNAME;
    row(L"KIND") = KIND;
    row(L"SCOPEKEY") = SCOPEKEY;
    row(L"UNIT") = UNIT;
    row(L"ACTIVEASSUMPTIONID") = ACTIVEASSUMPTIONID.GetValue();
    row(L"NOTES") = NOTES;
    row(L"ACTIVE") = ACTIVE.GetValue();

    return row;
}

void PlanAssumptionGroupRow::to_html_template(html_template& t) const
{
    t(L"GROUPID") = GROUPID.GetValue();
    t(L"GROUPNAME") = GROUPNAME;
    t(L"KIND") = KIND;
    t(L"SCOPEKEY") = SCOPEKEY;
    t(L"UNIT") = UNIT;
    t(L"ACTIVEASSUMPTIONID") = ACTIVEASSUMPTIONID.GetValue();
    t(L"NOTES") = NOTES;
    t(L"ACTIVE") = ACTIVE.GetValue();
}

bool PlanAssumptionGroupRow::equals(const PlanAssumptionGroupRow* other) const
{
    if ( GROUPID != other->GROUPID) return false;
    if (!GROUPNAME.IsSameAs(other->GROUPNAME)) return false;
    if (!KIND.IsSameAs(other->KIND)) return false;
    if (!SCOPEKEY.IsSameAs(other->SCOPEKEY)) return false;
    if (!UNIT.IsSameAs(other->UNIT)) return false;
    if ( ACTIVEASSUMPTIONID != other->ACTIVEASSUMPTIONID) return false;
    if (!NOTES.IsSameAs(other->NOTES)) return false;
    if ( ACTIVE != other->ACTIVE) return false;

    return true;
}

PlanAssumptionGroupTable::PlanAssumptionGroupTable()
{
    m_table_name = "PLANASSUMPTIONGROUP_V1";

    m_create_query = "CREATE TABLE PLANASSUMPTIONGROUP_V1(GROUPID integer primary key, GROUPNAME TEXT COLLATE NOCASE NOT NULL UNIQUE, KIND TEXT NOT NULL /* SharePrice, TaxRate, Inflation, ExchangeRate, Generic */, SCOPEKEY TEXT, UNIT TEXT, ACTIVEASSUMPTIONID integer, NOTES TEXT, ACTIVE integer DEFAULT 1)";

    m_drop_query = "DROP TABLE IF EXISTS PLANASSUMPTIONGROUP_V1";

    m_index_query_a = {
        "CREATE INDEX IF NOT EXISTS IDX_PLANASSUMPTIONGROUP_KIND ON PLANASSUMPTIONGROUP_V1(KIND)",
        "CREATE INDEX IF NOT EXISTS IDX_PLANASSUMPTIONGROUP_SCOPEKEY ON PLANASSUMPTIONGROUP_V1(SCOPEKEY)"
    };

    m_insert_query = "INSERT INTO PLANASSUMPTIONGROUP_V1(GROUPNAME, KIND, SCOPEKEY, UNIT, ACTIVEASSUMPTIONID, NOTES, ACTIVE, GROUPID) VALUES(?, ?, ?, ?, ?, ?, ?, ?)";

    m_update_query = "UPDATE PLANASSUMPTIONGROUP_V1 SET GROUPNAME = ?, KIND = ?, SCOPEKEY = ?, UNIT = ?, ACTIVEASSUMPTIONID = ?, NOTES = ?, ACTIVE = ? WHERE GROUPID = ?";

    m_delete_query = "DELETE FROM PLANASSUMPTIONGROUP_V1 WHERE GROUPID = ?";

    m_select_query = "SELECT GROUPID, GROUPNAME, KIND, SCOPEKEY, UNIT, ACTIVEASSUMPTIONID, NOTES, ACTIVE FROM PLANASSUMPTIONGROUP_V1";
}
