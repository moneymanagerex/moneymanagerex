// -*- C++ -*-
//=============================================================================
/**
 *      Copyright: (c) 2013-2026 Guan Lisheng (guanlisheng@gmail.com)
 *      Copyright: (c) 2017-2018 Stefano Giorgio (stef145g)
 *      Copyright: (c) 2022      Mark Whalley (mark@ipx.co.uk)
 *      Copyright: (c) 2026      George Ef (george.a.ef@gmail.com)
 *
 *      PlanGroupTable.cpp
 *
 *      Implementation of the interface to database table PLANGROUP_V1
 *
 *      @author [sqlite2cpp.py]
 *
 *      Revision History:
 *          AUTO GENERATED at 2026-08-08 11:34:44.604940.
 *          DO NOT EDIT!
 */
//=============================================================================

#include "_TableFactory.tpp"
#include "PlanGroupTable.h"
#include "data/PlanGroupData.h"

template class TableFactory<PlanGroupTable, PlanGroupData>;
template class mmCache<int64, PlanGroupData>;

// List of column names in database table PLANGROUP_V1,
// in the order of PlanGroupCol::COL_ID.
const wxArrayString PlanGroupCol::s_col_name_a = {
    "GROUPID",
    "PARENTID",
    "GROUPNAME",
    "NOTES",
    "STATUS",
    "TARGETDATE",
    "SORTORDER",
    "ACTIVE"
};

const PlanGroupCol::COL_ID PlanGroupCol::s_primary_id = COL_ID_GROUPID;
const wxString PlanGroupCol::s_primary_name = s_col_name_a[COL_ID_GROUPID];

// convenience variables
const wxString PlanGroupCol::NAME_GROUPID = s_col_name_a[COL_ID_GROUPID];
const wxString PlanGroupCol::NAME_PARENTID = s_col_name_a[COL_ID_PARENTID];
const wxString PlanGroupCol::NAME_GROUPNAME = s_col_name_a[COL_ID_GROUPNAME];
const wxString PlanGroupCol::NAME_NOTES = s_col_name_a[COL_ID_NOTES];
const wxString PlanGroupCol::NAME_STATUS = s_col_name_a[COL_ID_STATUS];
const wxString PlanGroupCol::NAME_TARGETDATE = s_col_name_a[COL_ID_TARGETDATE];
const wxString PlanGroupCol::NAME_SORTORDER = s_col_name_a[COL_ID_SORTORDER];
const wxString PlanGroupCol::NAME_ACTIVE = s_col_name_a[COL_ID_ACTIVE];

PlanGroupRow::PlanGroupRow()
{
    GROUPID = -1;
    PARENTID = -1;
    SORTORDER = -1;
    ACTIVE = -1;
}

// Bind a Row record to database insert statement.
void PlanGroupRow::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    stmt.Bind(1, PARENTID);
    stmt.Bind(2, GROUPNAME);
    stmt.Bind(3, NOTES);
    stmt.Bind(4, STATUS);
    stmt.Bind(5, TARGETDATE);
    stmt.Bind(6, SORTORDER);
    stmt.Bind(7, ACTIVE);
    stmt.Bind(8, id);
}

PlanGroupRow& PlanGroupRow::from_select_result(wxSQLite3ResultSet& q)
{
    GROUPID = q.GetInt64(0);
    PARENTID = q.GetInt64(1);
    GROUPNAME = q.GetString(2);
    NOTES = q.GetString(3);
    STATUS = q.GetString(4);
    TARGETDATE = q.GetString(5);
    SORTORDER = q.GetInt64(6);
    ACTIVE = q.GetInt64(7);

    return *this;
}

// Return the data record as a json string
wxString PlanGroupRow::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();			
    as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void PlanGroupRow::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("GROUPID");
    json_writer.Int64(GROUPID.GetValue());

    json_writer.Key("PARENTID");
    json_writer.Int64(PARENTID.GetValue());

    json_writer.Key("GROUPNAME");
    json_writer.String(GROUPNAME.utf8_str());

    json_writer.Key("NOTES");
    json_writer.String(NOTES.utf8_str());

    json_writer.Key("STATUS");
    json_writer.String(STATUS.utf8_str());

    json_writer.Key("TARGETDATE");
    json_writer.String(TARGETDATE.utf8_str());

    json_writer.Key("SORTORDER");
    json_writer.Int64(SORTORDER.GetValue());

    json_writer.Key("ACTIVE");
    json_writer.Int64(ACTIVE.GetValue());
}

row_t PlanGroupRow::to_html_row() const
{
    row_t row;

    row(L"GROUPID") = GROUPID.GetValue();
    row(L"PARENTID") = PARENTID.GetValue();
    row(L"GROUPNAME") = GROUPNAME;
    row(L"NOTES") = NOTES;
    row(L"STATUS") = STATUS;
    row(L"TARGETDATE") = TARGETDATE;
    row(L"SORTORDER") = SORTORDER.GetValue();
    row(L"ACTIVE") = ACTIVE.GetValue();

    return row;
}

void PlanGroupRow::to_html_template(html_template& t) const
{
    t(L"GROUPID") = GROUPID.GetValue();
    t(L"PARENTID") = PARENTID.GetValue();
    t(L"GROUPNAME") = GROUPNAME;
    t(L"NOTES") = NOTES;
    t(L"STATUS") = STATUS;
    t(L"TARGETDATE") = TARGETDATE;
    t(L"SORTORDER") = SORTORDER.GetValue();
    t(L"ACTIVE") = ACTIVE.GetValue();
}

bool PlanGroupRow::equals(const PlanGroupRow* other) const
{
    if ( GROUPID != other->GROUPID) return false;
    if ( PARENTID != other->PARENTID) return false;
    if (!GROUPNAME.IsSameAs(other->GROUPNAME)) return false;
    if (!NOTES.IsSameAs(other->NOTES)) return false;
    if (!STATUS.IsSameAs(other->STATUS)) return false;
    if (!TARGETDATE.IsSameAs(other->TARGETDATE)) return false;
    if ( SORTORDER != other->SORTORDER) return false;
    if ( ACTIVE != other->ACTIVE) return false;

    return true;
}

PlanGroupTable::PlanGroupTable()
{
    m_table_name = "PLANGROUP_V1";

    m_create_query = "CREATE TABLE PLANGROUP_V1(GROUPID integer primary key, PARENTID integer, GROUPNAME TEXT COLLATE NOCASE NOT NULL, NOTES TEXT, STATUS TEXT /* Planned, Committed, Wishlist, Done, Cancelled */, TARGETDATE TEXT, SORTORDER integer, ACTIVE integer DEFAULT 1)";

    m_drop_query = "DROP TABLE IF EXISTS PLANGROUP_V1";

    m_index_query_a = {
        "CREATE INDEX IF NOT EXISTS IDX_PLANGROUP_PARENTID ON PLANGROUP_V1(PARENTID)"
    };

    m_insert_query = "INSERT INTO PLANGROUP_V1(PARENTID, GROUPNAME, NOTES, STATUS, TARGETDATE, SORTORDER, ACTIVE, GROUPID) VALUES(?, ?, ?, ?, ?, ?, ?, ?)";

    m_update_query = "UPDATE PLANGROUP_V1 SET PARENTID = ?, GROUPNAME = ?, NOTES = ?, STATUS = ?, TARGETDATE = ?, SORTORDER = ?, ACTIVE = ? WHERE GROUPID = ?";

    m_delete_query = "DELETE FROM PLANGROUP_V1 WHERE GROUPID = ?";

    m_select_query = "SELECT GROUPID, PARENTID, GROUPNAME, NOTES, STATUS, TARGETDATE, SORTORDER, ACTIVE FROM PLANGROUP_V1";
}
