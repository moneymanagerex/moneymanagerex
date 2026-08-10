// -*- C++ -*-
//=============================================================================
/**
 *      Copyright: (c) 2013-2026 Guan Lisheng (guanlisheng@gmail.com)
 *      Copyright: (c) 2017-2018 Stefano Giorgio (stef145g)
 *      Copyright: (c) 2022      Mark Whalley (mark@ipx.co.uk)
 *      Copyright: (c) 2026      George Ef (george.a.ef@gmail.com)
 *
 *      PlanItemTable.cpp
 *
 *      Implementation of the interface to database table PLANITEM_V1
 *
 *      @author [sqlite2cpp.py]
 *
 *      Revision History:
 *          AUTO GENERATED at 2026-08-10 13:39:52.649608.
 *          DO NOT EDIT!
 */
//=============================================================================

#include "_TableFactory.tpp"
#include "PlanItemTable.h"
#include "data/PlanItemData.h"

template class TableFactory<PlanItemTable, PlanItemData>;
template class mmCache<int64, PlanItemData>;

// List of column names in database table PLANITEM_V1,
// in the order of PlanItemCol::COL_ID.
const wxArrayString PlanItemCol::s_col_name_a = {
    "PLANITEMID",
    "GROUPID",
    "ITEMNAME",
    "NOTES",
    "KIND",
    "STATUS",
    "TARGETDATE",
    "AMOUNT",
    "CURRENCYID",
    "CATEGID",
    "ACCOUNTID",
    "UNITS",
    "UNITPRICE",
    "TAXRATE",
    "STOCKSYMBOL",
    "CONFIDENCE",
    "SORTORDER",
    "ACTIVE",
    "PRICEASSUMPTIONID",
    "TAXASSUMPTIONID",
    "PRICEASSUMPTIONGROUPID",
    "TAXASSUMPTIONGROUPID"
};

const PlanItemCol::COL_ID PlanItemCol::s_primary_id = COL_ID_PLANITEMID;
const wxString PlanItemCol::s_primary_name = s_col_name_a[COL_ID_PLANITEMID];

// convenience variables
const wxString PlanItemCol::NAME_PLANITEMID = s_col_name_a[COL_ID_PLANITEMID];
const wxString PlanItemCol::NAME_GROUPID = s_col_name_a[COL_ID_GROUPID];
const wxString PlanItemCol::NAME_ITEMNAME = s_col_name_a[COL_ID_ITEMNAME];
const wxString PlanItemCol::NAME_NOTES = s_col_name_a[COL_ID_NOTES];
const wxString PlanItemCol::NAME_KIND = s_col_name_a[COL_ID_KIND];
const wxString PlanItemCol::NAME_STATUS = s_col_name_a[COL_ID_STATUS];
const wxString PlanItemCol::NAME_TARGETDATE = s_col_name_a[COL_ID_TARGETDATE];
const wxString PlanItemCol::NAME_AMOUNT = s_col_name_a[COL_ID_AMOUNT];
const wxString PlanItemCol::NAME_CURRENCYID = s_col_name_a[COL_ID_CURRENCYID];
const wxString PlanItemCol::NAME_CATEGID = s_col_name_a[COL_ID_CATEGID];
const wxString PlanItemCol::NAME_ACCOUNTID = s_col_name_a[COL_ID_ACCOUNTID];
const wxString PlanItemCol::NAME_UNITS = s_col_name_a[COL_ID_UNITS];
const wxString PlanItemCol::NAME_UNITPRICE = s_col_name_a[COL_ID_UNITPRICE];
const wxString PlanItemCol::NAME_TAXRATE = s_col_name_a[COL_ID_TAXRATE];
const wxString PlanItemCol::NAME_STOCKSYMBOL = s_col_name_a[COL_ID_STOCKSYMBOL];
const wxString PlanItemCol::NAME_CONFIDENCE = s_col_name_a[COL_ID_CONFIDENCE];
const wxString PlanItemCol::NAME_SORTORDER = s_col_name_a[COL_ID_SORTORDER];
const wxString PlanItemCol::NAME_ACTIVE = s_col_name_a[COL_ID_ACTIVE];
const wxString PlanItemCol::NAME_PRICEASSUMPTIONID = s_col_name_a[COL_ID_PRICEASSUMPTIONID];
const wxString PlanItemCol::NAME_TAXASSUMPTIONID = s_col_name_a[COL_ID_TAXASSUMPTIONID];
const wxString PlanItemCol::NAME_PRICEASSUMPTIONGROUPID = s_col_name_a[COL_ID_PRICEASSUMPTIONGROUPID];
const wxString PlanItemCol::NAME_TAXASSUMPTIONGROUPID = s_col_name_a[COL_ID_TAXASSUMPTIONGROUPID];

PlanItemRow::PlanItemRow()
{
    PLANITEMID = -1;
    GROUPID = -1;
    AMOUNT = 0.0;
    CURRENCYID = -1;
    CATEGID = -1;
    ACCOUNTID = -1;
    UNITS = 0.0;
    UNITPRICE = 0.0;
    TAXRATE = 0.0;
    CONFIDENCE = 0.0;
    SORTORDER = -1;
    ACTIVE = -1;
    PRICEASSUMPTIONID = -1;
    TAXASSUMPTIONID = -1;
    PRICEASSUMPTIONGROUPID = -1;
    TAXASSUMPTIONGROUPID = -1;
}

// Bind a Row record to database insert statement.
void PlanItemRow::to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const
{
    stmt.Bind(1, GROUPID);
    stmt.Bind(2, ITEMNAME);
    stmt.Bind(3, NOTES);
    stmt.Bind(4, KIND);
    stmt.Bind(5, STATUS);
    stmt.Bind(6, TARGETDATE);
    stmt.Bind(7, AMOUNT);
    stmt.Bind(8, CURRENCYID);
    stmt.Bind(9, CATEGID);
    stmt.Bind(10, ACCOUNTID);
    stmt.Bind(11, UNITS);
    stmt.Bind(12, UNITPRICE);
    stmt.Bind(13, TAXRATE);
    stmt.Bind(14, STOCKSYMBOL);
    stmt.Bind(15, CONFIDENCE);
    stmt.Bind(16, SORTORDER);
    stmt.Bind(17, ACTIVE);
    stmt.Bind(18, PRICEASSUMPTIONID);
    stmt.Bind(19, TAXASSUMPTIONID);
    stmt.Bind(20, PRICEASSUMPTIONGROUPID);
    stmt.Bind(21, TAXASSUMPTIONGROUPID);
    stmt.Bind(22, id);
}

PlanItemRow& PlanItemRow::from_select_result(wxSQLite3ResultSet& q)
{
    PLANITEMID = q.GetInt64(0);
    GROUPID = q.GetInt64(1);
    ITEMNAME = q.GetString(2);
    NOTES = q.GetString(3);
    KIND = q.GetString(4);
    STATUS = q.GetString(5);
    TARGETDATE = q.GetString(6);
    AMOUNT = q.GetDouble(7);
    CURRENCYID = q.GetInt64(8);
    CATEGID = q.GetInt64(9);
    ACCOUNTID = q.GetInt64(10);
    UNITS = q.GetDouble(11);
    UNITPRICE = q.GetDouble(12);
    TAXRATE = q.GetDouble(13);
    STOCKSYMBOL = q.GetString(14);
    CONFIDENCE = q.GetDouble(15);
    SORTORDER = q.GetInt64(16);
    ACTIVE = q.GetInt64(17);
    PRICEASSUMPTIONID = q.GetInt64(18);
    TAXASSUMPTIONID = q.GetInt64(19);
    PRICEASSUMPTIONGROUPID = q.GetInt64(20);
    TAXASSUMPTIONGROUPID = q.GetInt64(21);

    return *this;
}

// Return the data record as a json string
wxString PlanItemRow::to_json() const
{
    StringBuffer json_buffer;
    PrettyWriter<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();			
    as_json(json_writer);
    json_writer.EndObject();

    return json_buffer.GetString();
}

// Add the field data as json key:value pairs
void PlanItemRow::as_json(PrettyWriter<StringBuffer>& json_writer) const
{
    json_writer.Key("PLANITEMID");
    json_writer.Int64(PLANITEMID.GetValue());

    json_writer.Key("GROUPID");
    json_writer.Int64(GROUPID.GetValue());

    json_writer.Key("ITEMNAME");
    json_writer.String(ITEMNAME.utf8_str());

    json_writer.Key("NOTES");
    json_writer.String(NOTES.utf8_str());

    json_writer.Key("KIND");
    json_writer.String(KIND.utf8_str());

    json_writer.Key("STATUS");
    json_writer.String(STATUS.utf8_str());

    json_writer.Key("TARGETDATE");
    json_writer.String(TARGETDATE.utf8_str());

    json_writer.Key("AMOUNT");
    json_writer.Double(AMOUNT);

    json_writer.Key("CURRENCYID");
    json_writer.Int64(CURRENCYID.GetValue());

    json_writer.Key("CATEGID");
    json_writer.Int64(CATEGID.GetValue());

    json_writer.Key("ACCOUNTID");
    json_writer.Int64(ACCOUNTID.GetValue());

    json_writer.Key("UNITS");
    json_writer.Double(UNITS);

    json_writer.Key("UNITPRICE");
    json_writer.Double(UNITPRICE);

    json_writer.Key("TAXRATE");
    json_writer.Double(TAXRATE);

    json_writer.Key("STOCKSYMBOL");
    json_writer.String(STOCKSYMBOL.utf8_str());

    json_writer.Key("CONFIDENCE");
    json_writer.Double(CONFIDENCE);

    json_writer.Key("SORTORDER");
    json_writer.Int64(SORTORDER.GetValue());

    json_writer.Key("ACTIVE");
    json_writer.Int64(ACTIVE.GetValue());

    json_writer.Key("PRICEASSUMPTIONID");
    json_writer.Int64(PRICEASSUMPTIONID.GetValue());

    json_writer.Key("TAXASSUMPTIONID");
    json_writer.Int64(TAXASSUMPTIONID.GetValue());

    json_writer.Key("PRICEASSUMPTIONGROUPID");
    json_writer.Int64(PRICEASSUMPTIONGROUPID.GetValue());

    json_writer.Key("TAXASSUMPTIONGROUPID");
    json_writer.Int64(TAXASSUMPTIONGROUPID.GetValue());
}

row_t PlanItemRow::to_html_row() const
{
    row_t row;

    row(L"PLANITEMID") = PLANITEMID.GetValue();
    row(L"GROUPID") = GROUPID.GetValue();
    row(L"ITEMNAME") = ITEMNAME;
    row(L"NOTES") = NOTES;
    row(L"KIND") = KIND;
    row(L"STATUS") = STATUS;
    row(L"TARGETDATE") = TARGETDATE;
    row(L"AMOUNT") = AMOUNT;
    row(L"CURRENCYID") = CURRENCYID.GetValue();
    row(L"CATEGID") = CATEGID.GetValue();
    row(L"ACCOUNTID") = ACCOUNTID.GetValue();
    row(L"UNITS") = UNITS;
    row(L"UNITPRICE") = UNITPRICE;
    row(L"TAXRATE") = TAXRATE;
    row(L"STOCKSYMBOL") = STOCKSYMBOL;
    row(L"CONFIDENCE") = CONFIDENCE;
    row(L"SORTORDER") = SORTORDER.GetValue();
    row(L"ACTIVE") = ACTIVE.GetValue();
    row(L"PRICEASSUMPTIONID") = PRICEASSUMPTIONID.GetValue();
    row(L"TAXASSUMPTIONID") = TAXASSUMPTIONID.GetValue();
    row(L"PRICEASSUMPTIONGROUPID") = PRICEASSUMPTIONGROUPID.GetValue();
    row(L"TAXASSUMPTIONGROUPID") = TAXASSUMPTIONGROUPID.GetValue();

    return row;
}

void PlanItemRow::to_html_template(html_template& t) const
{
    t(L"PLANITEMID") = PLANITEMID.GetValue();
    t(L"GROUPID") = GROUPID.GetValue();
    t(L"ITEMNAME") = ITEMNAME;
    t(L"NOTES") = NOTES;
    t(L"KIND") = KIND;
    t(L"STATUS") = STATUS;
    t(L"TARGETDATE") = TARGETDATE;
    t(L"AMOUNT") = AMOUNT;
    t(L"CURRENCYID") = CURRENCYID.GetValue();
    t(L"CATEGID") = CATEGID.GetValue();
    t(L"ACCOUNTID") = ACCOUNTID.GetValue();
    t(L"UNITS") = UNITS;
    t(L"UNITPRICE") = UNITPRICE;
    t(L"TAXRATE") = TAXRATE;
    t(L"STOCKSYMBOL") = STOCKSYMBOL;
    t(L"CONFIDENCE") = CONFIDENCE;
    t(L"SORTORDER") = SORTORDER.GetValue();
    t(L"ACTIVE") = ACTIVE.GetValue();
    t(L"PRICEASSUMPTIONID") = PRICEASSUMPTIONID.GetValue();
    t(L"TAXASSUMPTIONID") = TAXASSUMPTIONID.GetValue();
    t(L"PRICEASSUMPTIONGROUPID") = PRICEASSUMPTIONGROUPID.GetValue();
    t(L"TAXASSUMPTIONGROUPID") = TAXASSUMPTIONGROUPID.GetValue();
}

bool PlanItemRow::equals(const PlanItemRow* other) const
{
    if ( PLANITEMID != other->PLANITEMID) return false;
    if ( GROUPID != other->GROUPID) return false;
    if (!ITEMNAME.IsSameAs(other->ITEMNAME)) return false;
    if (!NOTES.IsSameAs(other->NOTES)) return false;
    if (!KIND.IsSameAs(other->KIND)) return false;
    if (!STATUS.IsSameAs(other->STATUS)) return false;
    if (!TARGETDATE.IsSameAs(other->TARGETDATE)) return false;
    if ( AMOUNT != other->AMOUNT) return false;
    if ( CURRENCYID != other->CURRENCYID) return false;
    if ( CATEGID != other->CATEGID) return false;
    if ( ACCOUNTID != other->ACCOUNTID) return false;
    if ( UNITS != other->UNITS) return false;
    if ( UNITPRICE != other->UNITPRICE) return false;
    if ( TAXRATE != other->TAXRATE) return false;
    if (!STOCKSYMBOL.IsSameAs(other->STOCKSYMBOL)) return false;
    if ( CONFIDENCE != other->CONFIDENCE) return false;
    if ( SORTORDER != other->SORTORDER) return false;
    if ( ACTIVE != other->ACTIVE) return false;
    if ( PRICEASSUMPTIONID != other->PRICEASSUMPTIONID) return false;
    if ( TAXASSUMPTIONID != other->TAXASSUMPTIONID) return false;
    if ( PRICEASSUMPTIONGROUPID != other->PRICEASSUMPTIONGROUPID) return false;
    if ( TAXASSUMPTIONGROUPID != other->TAXASSUMPTIONGROUPID) return false;

    return true;
}

PlanItemTable::PlanItemTable()
{
    m_table_name = "PLANITEM_V1";

    m_create_query = "CREATE TABLE PLANITEM_V1(PLANITEMID integer primary key, GROUPID integer, ITEMNAME TEXT COLLATE NOCASE NOT NULL, NOTES TEXT, KIND TEXT NOT NULL /* Expense, Income */, STATUS TEXT /* Planned, Committed, Wishlist, Done, Cancelled */, TARGETDATE TEXT, AMOUNT numeric, CURRENCYID integer, CATEGID integer, ACCOUNTID integer, UNITS numeric, UNITPRICE numeric, TAXRATE numeric, STOCKSYMBOL TEXT, CONFIDENCE numeric, SORTORDER integer, ACTIVE integer DEFAULT 1, PRICEASSUMPTIONID integer, TAXASSUMPTIONID integer, PRICEASSUMPTIONGROUPID integer, TAXASSUMPTIONGROUPID integer)";

    m_drop_query = "DROP TABLE IF EXISTS PLANITEM_V1";

    m_index_query_a = {
        "CREATE INDEX IF NOT EXISTS IDX_PLANITEM_GROUPID ON PLANITEM_V1(GROUPID)",
        "CREATE INDEX IF NOT EXISTS IDX_PLANITEM_TARGETDATE ON PLANITEM_V1(TARGETDATE)"
    };

    m_insert_query = "INSERT INTO PLANITEM_V1(GROUPID, ITEMNAME, NOTES, KIND, STATUS, TARGETDATE, AMOUNT, CURRENCYID, CATEGID, ACCOUNTID, UNITS, UNITPRICE, TAXRATE, STOCKSYMBOL, CONFIDENCE, SORTORDER, ACTIVE, PRICEASSUMPTIONID, TAXASSUMPTIONID, PRICEASSUMPTIONGROUPID, TAXASSUMPTIONGROUPID, PLANITEMID) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    m_update_query = "UPDATE PLANITEM_V1 SET GROUPID = ?, ITEMNAME = ?, NOTES = ?, KIND = ?, STATUS = ?, TARGETDATE = ?, AMOUNT = ?, CURRENCYID = ?, CATEGID = ?, ACCOUNTID = ?, UNITS = ?, UNITPRICE = ?, TAXRATE = ?, STOCKSYMBOL = ?, CONFIDENCE = ?, SORTORDER = ?, ACTIVE = ?, PRICEASSUMPTIONID = ?, TAXASSUMPTIONID = ?, PRICEASSUMPTIONGROUPID = ?, TAXASSUMPTIONGROUPID = ? WHERE PLANITEMID = ?";

    m_delete_query = "DELETE FROM PLANITEM_V1 WHERE PLANITEMID = ?";

    m_select_query = "SELECT PLANITEMID, GROUPID, ITEMNAME, NOTES, KIND, STATUS, TARGETDATE, AMOUNT, CURRENCYID, CATEGID, ACCOUNTID, UNITS, UNITPRICE, TAXRATE, STOCKSYMBOL, CONFIDENCE, SORTORDER, ACTIVE, PRICEASSUMPTIONID, TAXASSUMPTIONID, PRICEASSUMPTIONGROUPID, TAXASSUMPTIONGROUPID FROM PLANITEM_V1";
}
