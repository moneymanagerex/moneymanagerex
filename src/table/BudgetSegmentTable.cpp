// -*- C++ -*-
//=============================================================================
/**
 *      Copyright: (c) 2013-2026 Guan Lisheng (guanlisheng@gmail.com)
 *      Copyright: (c) 2017-2018 Stefano Giorgio (stef145g)
 *      Copyright: (c) 2022      Mark Whalley (mark@ipx.co.uk)
 *      Copyright: (c) 2026      George Ef (george.a.ef@gmail.com)
 *
 *      BudgetSegmentTable.cpp
 *
 *      Implementation of the interface to database table BUDGETSEGMENT_V1
 *
 *      @author [sqlite2cpp.py]
 *
 *      Revision History:
 *          AUTO GENERATED at 2026-08-08 11:34:44.604940.
 *          DO NOT EDIT!
 */
//=============================================================================

#include "_TableFactory.tpp"
#include "BudgetSegmentTable.h"
#include "data/BudgetSegmentData.h"

template class TableFactory<BudgetSegmentTable, BudgetSegmentData>;
template class mmCache<int64, BudgetSegmentData>;

// List of column names in database table BUDGETSEGMENT_V1,
// in the order of BudgetSegmentCol::COL_ID.
const wxArrayString BudgetSegmentCol::s_col_name_a = {
    "SEGMENTID",
    "BUDGETYEARID",
    "SEGMENTNAME",
    "STARTDAY",
    "ENDDAY",
    "SORTORDER",
    "ACTIVE"
};

const BudgetSegmentCol::COL_ID BudgetSegmentCol::s_primary_id = COL_ID_SEGMENTID;
const wxString BudgetSegmentCol::s_primary_name = s_col_name_a[COL_ID_SEGMENTID];

// convenience variables
const wxString BudgetSegmentCol::NAME_SEGMENTID = s_col_name_a[COL_ID_SEGMENTID];
const wxString BudgetSegmentCol::NAME_BUDGETYEARID = s_col_name_a[COL_ID_BUDGETYEARID];
const wxString BudgetSegmentCol::NAME_SEGMENTNAME = s_col_name_a[COL_ID_SEGMENTNAME];
const wxString BudgetSegmentCol::NAME_STARTDAY = s_col_name_a[COL_ID_STARTDAY];
const wxString BudgetSegmentCol::NAME_ENDDAY = s_col_name_a[COL_ID_ENDDAY];
const wxString BudgetSegmentCol::NAME_SORTORDER = s_col_name_a[COL_ID_SORTORDER];
const wxString BudgetSegmentCol::NAME_ACTIVE = s_col_name_a[COL_ID_ACTIVE];

BudgetSegmentRow::BudgetSegmentRow()
{
    SEGMENTID = -1;
    BUDGETYEARID = -1;
    STARTDAY = -1;
    ENDDAY = -1;
    SORTORDER = -1;
    ACTIVE = -1;
}

// Bind a Row record to database insert statement.
void BudgetSegmentRow::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    stmt.Bind(1, BUDGETYEARID);
    stmt.Bind(2, SEGMENTNAME);
    stmt.Bind(3, STARTDAY);
    stmt.Bind(4, ENDDAY);
    stmt.Bind(5, SORTORDER);
    stmt.Bind(6, ACTIVE);
    stmt.Bind(7, id);
}

BudgetSegmentRow& BudgetSegmentRow::from_select_result(wxSQLite3ResultSet& q)
{
    SEGMENTID = q.GetInt64(0);
    BUDGETYEARID = q.GetInt64(1);
    SEGMENTNAME = q.GetString(2);
    STARTDAY = q.GetInt64(3);
    ENDDAY = q.GetInt64(4);
    SORTORDER = q.GetInt64(5);
    ACTIVE = q.GetInt64(6);

    return *this;
}

// Return the data record as a json string
wxString BudgetSegmentRow::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();			
    as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void BudgetSegmentRow::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("SEGMENTID");
    json_writer.Int64(SEGMENTID.GetValue());

    json_writer.Key("BUDGETYEARID");
    json_writer.Int64(BUDGETYEARID.GetValue());

    json_writer.Key("SEGMENTNAME");
    json_writer.String(SEGMENTNAME.utf8_str());

    json_writer.Key("STARTDAY");
    json_writer.Int64(STARTDAY.GetValue());

    json_writer.Key("ENDDAY");
    json_writer.Int64(ENDDAY.GetValue());

    json_writer.Key("SORTORDER");
    json_writer.Int64(SORTORDER.GetValue());

    json_writer.Key("ACTIVE");
    json_writer.Int64(ACTIVE.GetValue());
}

row_t BudgetSegmentRow::to_html_row() const
{
    row_t row;

    row(L"SEGMENTID") = SEGMENTID.GetValue();
    row(L"BUDGETYEARID") = BUDGETYEARID.GetValue();
    row(L"SEGMENTNAME") = SEGMENTNAME;
    row(L"STARTDAY") = STARTDAY.GetValue();
    row(L"ENDDAY") = ENDDAY.GetValue();
    row(L"SORTORDER") = SORTORDER.GetValue();
    row(L"ACTIVE") = ACTIVE.GetValue();

    return row;
}

void BudgetSegmentRow::to_html_template(html_template& t) const
{
    t(L"SEGMENTID") = SEGMENTID.GetValue();
    t(L"BUDGETYEARID") = BUDGETYEARID.GetValue();
    t(L"SEGMENTNAME") = SEGMENTNAME;
    t(L"STARTDAY") = STARTDAY.GetValue();
    t(L"ENDDAY") = ENDDAY.GetValue();
    t(L"SORTORDER") = SORTORDER.GetValue();
    t(L"ACTIVE") = ACTIVE.GetValue();
}

bool BudgetSegmentRow::equals(const BudgetSegmentRow* other) const
{
    if ( SEGMENTID != other->SEGMENTID) return false;
    if ( BUDGETYEARID != other->BUDGETYEARID) return false;
    if (!SEGMENTNAME.IsSameAs(other->SEGMENTNAME)) return false;
    if ( STARTDAY != other->STARTDAY) return false;
    if ( ENDDAY != other->ENDDAY) return false;
    if ( SORTORDER != other->SORTORDER) return false;
    if ( ACTIVE != other->ACTIVE) return false;

    return true;
}

BudgetSegmentTable::BudgetSegmentTable()
{
    m_table_name = "BUDGETSEGMENT_V1";

    m_create_query = "CREATE TABLE BUDGETSEGMENT_V1(SEGMENTID integer primary key, BUDGETYEARID integer NOT NULL, SEGMENTNAME TEXT COLLATE NOCASE NOT NULL, STARTDAY integer NOT NULL DEFAULT 1, ENDDAY integer NOT NULL DEFAULT 31, SORTORDER integer, ACTIVE integer DEFAULT 1, UNIQUE(BUDGETYEARID, SEGMENTNAME))";

    m_drop_query = "DROP TABLE IF EXISTS BUDGETSEGMENT_V1";

    m_index_query_a = {
        "CREATE INDEX IF NOT EXISTS IDX_BUDGETSEGMENT_BUDGETYEARID ON BUDGETSEGMENT_V1(BUDGETYEARID)"
    };

    m_insert_query = "INSERT INTO BUDGETSEGMENT_V1(BUDGETYEARID, SEGMENTNAME, STARTDAY, ENDDAY, SORTORDER, ACTIVE, SEGMENTID) VALUES(?, ?, ?, ?, ?, ?, ?)";

    m_update_query = "UPDATE BUDGETSEGMENT_V1 SET BUDGETYEARID = ?, SEGMENTNAME = ?, STARTDAY = ?, ENDDAY = ?, SORTORDER = ?, ACTIVE = ? WHERE SEGMENTID = ?";

    m_delete_query = "DELETE FROM BUDGETSEGMENT_V1 WHERE SEGMENTID = ?";

    m_select_query = "SELECT SEGMENTID, BUDGETYEARID, SEGMENTNAME, STARTDAY, ENDDAY, SORTORDER, ACTIVE FROM BUDGETSEGMENT_V1";
}
