// -*- C++ -*-
//=============================================================================
/**
 *      Copyright: (c) 2013-2026 Guan Lisheng (guanlisheng@gmail.com)
 *      Copyright: (c) 2017-2018 Stefano Giorgio (stef145g)
 *      Copyright: (c) 2022      Mark Whalley (mark@ipx.co.uk)
 *      Copyright: (c) 2026      George Ef (george.a.ef@gmail.com)
 *
 *      BudgetTable.cpp
 *
 *      Implementation of the interface to database table BUDGETTABLE_V1
 *
 *      @author [sqlite2cpp.py]
 *
 *      Revision History:
 *          AUTO GENERATED at 2026-08-08 11:34:44.604940.
 *          DO NOT EDIT!
 */
//=============================================================================

#include "_TableFactory.tpp"
#include "BudgetTable.h"
#include "data/BudgetData.h"

template class TableFactory<BudgetTable, BudgetData>;
template class mmCache<int64, BudgetData>;

// List of column names in database table BUDGETTABLE_V1,
// in the order of BudgetCol::COL_ID.
const wxArrayString BudgetCol::s_col_name_a = {
    "BUDGETENTRYID",
    "BUDGETYEARID",
    "CATEGID",
    "PERIOD",
    "AMOUNT",
    "NOTES",
    "ACTIVE",
    "SEGMENTID",
    "AMOUNTTYPE",
    "AUTOSOURCE"
};

const BudgetCol::COL_ID BudgetCol::s_primary_id = COL_ID_BUDGETENTRYID;
const wxString BudgetCol::s_primary_name = s_col_name_a[COL_ID_BUDGETENTRYID];

// convenience variables
const wxString BudgetCol::NAME_BUDGETENTRYID = s_col_name_a[COL_ID_BUDGETENTRYID];
const wxString BudgetCol::NAME_BUDGETYEARID = s_col_name_a[COL_ID_BUDGETYEARID];
const wxString BudgetCol::NAME_CATEGID = s_col_name_a[COL_ID_CATEGID];
const wxString BudgetCol::NAME_PERIOD = s_col_name_a[COL_ID_PERIOD];
const wxString BudgetCol::NAME_AMOUNT = s_col_name_a[COL_ID_AMOUNT];
const wxString BudgetCol::NAME_NOTES = s_col_name_a[COL_ID_NOTES];
const wxString BudgetCol::NAME_ACTIVE = s_col_name_a[COL_ID_ACTIVE];
const wxString BudgetCol::NAME_SEGMENTID = s_col_name_a[COL_ID_SEGMENTID];
const wxString BudgetCol::NAME_AMOUNTTYPE = s_col_name_a[COL_ID_AMOUNTTYPE];
const wxString BudgetCol::NAME_AUTOSOURCE = s_col_name_a[COL_ID_AUTOSOURCE];

BudgetRow::BudgetRow()
{
    BUDGETENTRYID = -1;
    BUDGETYEARID = -1;
    CATEGID = -1;
    AMOUNT = 0.0;
    ACTIVE = -1;
    SEGMENTID = -1;
}

// Bind a Row record to database insert statement.
void BudgetRow::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    stmt.Bind(1, BUDGETYEARID);
    stmt.Bind(2, CATEGID);
    stmt.Bind(3, PERIOD);
    stmt.Bind(4, AMOUNT);
    stmt.Bind(5, NOTES);
    stmt.Bind(6, ACTIVE);
    stmt.Bind(7, SEGMENTID);
    stmt.Bind(8, AMOUNTTYPE);
    stmt.Bind(9, AUTOSOURCE);
    stmt.Bind(10, id);
}

BudgetRow& BudgetRow::from_select_result(wxSQLite3ResultSet& q)
{
    BUDGETENTRYID = q.GetInt64(0);
    BUDGETYEARID = q.GetInt64(1);
    CATEGID = q.GetInt64(2);
    PERIOD = q.GetString(3);
    AMOUNT = q.GetDouble(4);
    NOTES = q.GetString(5);
    ACTIVE = q.GetInt64(6);
    SEGMENTID = q.GetInt64(7);
    AMOUNTTYPE = q.GetString(8);
    AUTOSOURCE = q.GetString(9);

    return *this;
}

// Return the data record as a json string
wxString BudgetRow::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();			
    as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void BudgetRow::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("BUDGETENTRYID");
    json_writer.Int64(BUDGETENTRYID.GetValue());

    json_writer.Key("BUDGETYEARID");
    json_writer.Int64(BUDGETYEARID.GetValue());

    json_writer.Key("CATEGID");
    json_writer.Int64(CATEGID.GetValue());

    json_writer.Key("PERIOD");
    json_writer.String(PERIOD.utf8_str());

    json_writer.Key("AMOUNT");
    json_writer.Double(AMOUNT);

    json_writer.Key("NOTES");
    json_writer.String(NOTES.utf8_str());

    json_writer.Key("ACTIVE");
    json_writer.Int64(ACTIVE.GetValue());

    json_writer.Key("SEGMENTID");
    json_writer.Int64(SEGMENTID.GetValue());

    json_writer.Key("AMOUNTTYPE");
    json_writer.String(AMOUNTTYPE.utf8_str());

    json_writer.Key("AUTOSOURCE");
    json_writer.String(AUTOSOURCE.utf8_str());
}

row_t BudgetRow::to_html_row() const
{
    row_t row;

    row(L"BUDGETENTRYID") = BUDGETENTRYID.GetValue();
    row(L"BUDGETYEARID") = BUDGETYEARID.GetValue();
    row(L"CATEGID") = CATEGID.GetValue();
    row(L"PERIOD") = PERIOD;
    row(L"AMOUNT") = AMOUNT;
    row(L"NOTES") = NOTES;
    row(L"ACTIVE") = ACTIVE.GetValue();
    row(L"SEGMENTID") = SEGMENTID.GetValue();
    row(L"AMOUNTTYPE") = AMOUNTTYPE;
    row(L"AUTOSOURCE") = AUTOSOURCE;

    return row;
}

void BudgetRow::to_html_template(html_template& t) const
{
    t(L"BUDGETENTRYID") = BUDGETENTRYID.GetValue();
    t(L"BUDGETYEARID") = BUDGETYEARID.GetValue();
    t(L"CATEGID") = CATEGID.GetValue();
    t(L"PERIOD") = PERIOD;
    t(L"AMOUNT") = AMOUNT;
    t(L"NOTES") = NOTES;
    t(L"ACTIVE") = ACTIVE.GetValue();
    t(L"SEGMENTID") = SEGMENTID.GetValue();
    t(L"AMOUNTTYPE") = AMOUNTTYPE;
    t(L"AUTOSOURCE") = AUTOSOURCE;
}

bool BudgetRow::equals(const BudgetRow* other) const
{
    if ( BUDGETENTRYID != other->BUDGETENTRYID) return false;
    if ( BUDGETYEARID != other->BUDGETYEARID) return false;
    if ( CATEGID != other->CATEGID) return false;
    if (!PERIOD.IsSameAs(other->PERIOD)) return false;
    if ( AMOUNT != other->AMOUNT) return false;
    if (!NOTES.IsSameAs(other->NOTES)) return false;
    if ( ACTIVE != other->ACTIVE) return false;
    if ( SEGMENTID != other->SEGMENTID) return false;
    if (!AMOUNTTYPE.IsSameAs(other->AMOUNTTYPE)) return false;
    if (!AUTOSOURCE.IsSameAs(other->AUTOSOURCE)) return false;

    return true;
}

BudgetTable::BudgetTable()
{
    m_table_name = "BUDGETTABLE_V1";

    m_create_query = "CREATE TABLE BUDGETTABLE_V1(BUDGETENTRYID integer primary key, BUDGETYEARID integer, CATEGID integer, PERIOD TEXT NOT NULL /* None, Weekly, Bi-Weekly, Monthly, Monthly, Bi-Monthly, Quarterly, Half-Yearly, Yearly, Daily*/, AMOUNT numeric NOT NULL, NOTES TEXT, ACTIVE integer, SEGMENTID integer, AMOUNTTYPE TEXT /* Fixed, Estimated, Auto */, AUTOSOURCE TEXT)";

    m_drop_query = "DROP TABLE IF EXISTS BUDGETTABLE_V1";

    m_index_query_a = {
        "CREATE INDEX IF NOT EXISTS IDX_BUDGETTABLE_BUDGETYEARID ON BUDGETTABLE_V1(BUDGETYEARID)"
    };

    m_insert_query = "INSERT INTO BUDGETTABLE_V1(BUDGETYEARID, CATEGID, PERIOD, AMOUNT, NOTES, ACTIVE, SEGMENTID, AMOUNTTYPE, AUTOSOURCE, BUDGETENTRYID) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    m_update_query = "UPDATE BUDGETTABLE_V1 SET BUDGETYEARID = ?, CATEGID = ?, PERIOD = ?, AMOUNT = ?, NOTES = ?, ACTIVE = ?, SEGMENTID = ?, AMOUNTTYPE = ?, AUTOSOURCE = ? WHERE BUDGETENTRYID = ?";

    m_delete_query = "DELETE FROM BUDGETTABLE_V1 WHERE BUDGETENTRYID = ?";

    m_select_query = "SELECT BUDGETENTRYID, BUDGETYEARID, CATEGID, PERIOD, AMOUNT, NOTES, ACTIVE, SEGMENTID, AMOUNTTYPE, AUTOSOURCE FROM BUDGETTABLE_V1";
}
