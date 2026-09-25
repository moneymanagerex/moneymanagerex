// -*- C++ -*-
//=============================================================================
/**
 *      Copyright: (c) 2013-2026 Guan Lisheng (guanlisheng@gmail.com)
 *      Copyright: (c) 2017-2018 Stefano Giorgio (stef145g)
 *      Copyright: (c) 2022      Mark Whalley (mark@ipx.co.uk)
 *      Copyright: (c) 2026      George Ef (george.a.ef@gmail.com)
 *
 *      PlanItemTable.h
 *
 *      Interface to database table PLANITEM_V1
 *
 *      @author [sqlite2cpp.py]
 *
 *      Revision History:
 *          AUTO GENERATED at 2026-08-10 13:39:52.649608.
 *          DO NOT EDIT!
 */
//=============================================================================

#pragma once

#include "_TableBase.h"

// Columns in database table PLANITEM_V1
struct PlanItemCol
{
    enum COL_ID
    {
        COL_ID_PLANITEMID = 0,
        COL_ID_GROUPID,
        COL_ID_ITEMNAME,
        COL_ID_NOTES,
        COL_ID_KIND,
        COL_ID_STATUS,
        COL_ID_TARGETDATE,
        COL_ID_AMOUNT,
        COL_ID_CURRENCYID,
        COL_ID_CATEGID,
        COL_ID_ACCOUNTID,
        COL_ID_UNITS,
        COL_ID_UNITPRICE,
        COL_ID_TAXRATE,
        COL_ID_STOCKSYMBOL,
        COL_ID_CONFIDENCE,
        COL_ID_SORTORDER,
        COL_ID_ACTIVE,
        COL_ID_PRICEASSUMPTIONID,
        COL_ID_TAXASSUMPTIONID,
        COL_ID_PRICEASSUMPTIONGROUPID,
        COL_ID_TAXASSUMPTIONGROUPID,
        COL_ID_size
    };

    static const wxArrayString s_col_name_a;
    static const COL_ID s_primary_id;
    static const wxString s_primary_name;

    static wxString col_id_name(COL_ID col_id) { return s_col_name_a[col_id]; }

    // convenience variables

    static const wxString NAME_PLANITEMID;
    static const wxString NAME_GROUPID;
    static const wxString NAME_ITEMNAME;
    static const wxString NAME_NOTES;
    static const wxString NAME_KIND;
    static const wxString NAME_STATUS;
    static const wxString NAME_TARGETDATE;
    static const wxString NAME_AMOUNT;
    static const wxString NAME_CURRENCYID;
    static const wxString NAME_CATEGID;
    static const wxString NAME_ACCOUNTID;
    static const wxString NAME_UNITS;
    static const wxString NAME_UNITPRICE;
    static const wxString NAME_TAXRATE;
    static const wxString NAME_STOCKSYMBOL;
    static const wxString NAME_CONFIDENCE;
    static const wxString NAME_SORTORDER;
    static const wxString NAME_ACTIVE;
    static const wxString NAME_PRICEASSUMPTIONID;
    static const wxString NAME_TAXASSUMPTIONID;
    static const wxString NAME_PRICEASSUMPTIONGROUPID;
    static const wxString NAME_TAXASSUMPTIONGROUPID;

    // convenience methods

    static TableClauseV<int64> WHERE_PLANITEMID(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_PLANITEMID, op, value);
    }

    static TableClauseV<int64> WHERE_GROUPID(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_GROUPID, op, value);
    }

    static TableClauseV<wxString> WHERE_ITEMNAME(OP op, const wxString& value) {
        return TableClause::WHERE<wxString>(NAME_ITEMNAME, op, value);
    }

    static TableClauseV<wxString> WHERE_NOTES(OP op, const wxString& value) {
        return TableClause::WHERE<wxString>(NAME_NOTES, op, value);
    }

    static TableClauseV<wxString> WHERE_KIND(OP op, const wxString& value) {
        return TableClause::WHERE<wxString>(NAME_KIND, op, value);
    }

    static TableClauseV<wxString> WHERE_STATUS(OP op, const wxString& value) {
        return TableClause::WHERE<wxString>(NAME_STATUS, op, value);
    }

    static TableClauseV<wxString> WHERE_TARGETDATE(OP op, const wxString& value) {
        return TableClause::WHERE<wxString>(NAME_TARGETDATE, op, value);
    }

    static TableClauseV<double> WHERE_AMOUNT(OP op, const double& value) {
        return TableClause::WHERE<double>(NAME_AMOUNT, op, value);
    }

    static TableClauseV<int64> WHERE_CURRENCYID(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_CURRENCYID, op, value);
    }

    static TableClauseV<int64> WHERE_CATEGID(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_CATEGID, op, value);
    }

    static TableClauseV<int64> WHERE_ACCOUNTID(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_ACCOUNTID, op, value);
    }

    static TableClauseV<double> WHERE_UNITS(OP op, const double& value) {
        return TableClause::WHERE<double>(NAME_UNITS, op, value);
    }

    static TableClauseV<double> WHERE_UNITPRICE(OP op, const double& value) {
        return TableClause::WHERE<double>(NAME_UNITPRICE, op, value);
    }

    static TableClauseV<double> WHERE_TAXRATE(OP op, const double& value) {
        return TableClause::WHERE<double>(NAME_TAXRATE, op, value);
    }

    static TableClauseV<wxString> WHERE_STOCKSYMBOL(OP op, const wxString& value) {
        return TableClause::WHERE<wxString>(NAME_STOCKSYMBOL, op, value);
    }

    static TableClauseV<double> WHERE_CONFIDENCE(OP op, const double& value) {
        return TableClause::WHERE<double>(NAME_CONFIDENCE, op, value);
    }

    static TableClauseV<int64> WHERE_SORTORDER(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_SORTORDER, op, value);
    }

    static TableClauseV<int64> WHERE_ACTIVE(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_ACTIVE, op, value);
    }

    static TableClauseV<int64> WHERE_PRICEASSUMPTIONID(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_PRICEASSUMPTIONID, op, value);
    }

    static TableClauseV<int64> WHERE_TAXASSUMPTIONID(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_TAXASSUMPTIONID, op, value);
    }

    static TableClauseV<int64> WHERE_PRICEASSUMPTIONGROUPID(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_PRICEASSUMPTIONGROUPID, op, value);
    }

    static TableClauseV<int64> WHERE_TAXASSUMPTIONGROUPID(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_TAXASSUMPTIONGROUPID, op, value);
    }

    // deprecated

    struct PLANITEMID : public TableOpV<int64>
    {
        static COL_ID col_id() { return COL_ID_PLANITEMID; }
        static wxString col_name() { return s_col_name_a[COL_ID_PLANITEMID]; }
        explicit PLANITEMID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit PLANITEMID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct GROUPID : public TableOpV<int64>
    {
        static COL_ID col_id() { return COL_ID_GROUPID; }
        static wxString col_name() { return s_col_name_a[COL_ID_GROUPID]; }
        explicit GROUPID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit GROUPID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct ITEMNAME : public TableOpV<wxString>
    {
        static COL_ID col_id() { return COL_ID_ITEMNAME; }
        static wxString col_name() { return s_col_name_a[COL_ID_ITEMNAME]; }
        explicit ITEMNAME(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit ITEMNAME(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct NOTES : public TableOpV<wxString>
    {
        static COL_ID col_id() { return COL_ID_NOTES; }
        static wxString col_name() { return s_col_name_a[COL_ID_NOTES]; }
        explicit NOTES(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit NOTES(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct KIND : public TableOpV<wxString>
    {
        static COL_ID col_id() { return COL_ID_KIND; }
        static wxString col_name() { return s_col_name_a[COL_ID_KIND]; }
        explicit KIND(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit KIND(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct STATUS : public TableOpV<wxString>
    {
        static COL_ID col_id() { return COL_ID_STATUS; }
        static wxString col_name() { return s_col_name_a[COL_ID_STATUS]; }
        explicit STATUS(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit STATUS(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct TARGETDATE : public TableOpV<wxString>
    {
        static COL_ID col_id() { return COL_ID_TARGETDATE; }
        static wxString col_name() { return s_col_name_a[COL_ID_TARGETDATE]; }
        explicit TARGETDATE(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit TARGETDATE(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct AMOUNT : public TableOpV<double>
    {
        static COL_ID col_id() { return COL_ID_AMOUNT; }
        static wxString col_name() { return s_col_name_a[COL_ID_AMOUNT]; }
        explicit AMOUNT(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit AMOUNT(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    struct CURRENCYID : public TableOpV<int64>
    {
        static COL_ID col_id() { return COL_ID_CURRENCYID; }
        static wxString col_name() { return s_col_name_a[COL_ID_CURRENCYID]; }
        explicit CURRENCYID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit CURRENCYID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct CATEGID : public TableOpV<int64>
    {
        static COL_ID col_id() { return COL_ID_CATEGID; }
        static wxString col_name() { return s_col_name_a[COL_ID_CATEGID]; }
        explicit CATEGID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit CATEGID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct ACCOUNTID : public TableOpV<int64>
    {
        static COL_ID col_id() { return COL_ID_ACCOUNTID; }
        static wxString col_name() { return s_col_name_a[COL_ID_ACCOUNTID]; }
        explicit ACCOUNTID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit ACCOUNTID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct UNITS : public TableOpV<double>
    {
        static COL_ID col_id() { return COL_ID_UNITS; }
        static wxString col_name() { return s_col_name_a[COL_ID_UNITS]; }
        explicit UNITS(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit UNITS(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    struct UNITPRICE : public TableOpV<double>
    {
        static COL_ID col_id() { return COL_ID_UNITPRICE; }
        static wxString col_name() { return s_col_name_a[COL_ID_UNITPRICE]; }
        explicit UNITPRICE(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit UNITPRICE(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    struct TAXRATE : public TableOpV<double>
    {
        static COL_ID col_id() { return COL_ID_TAXRATE; }
        static wxString col_name() { return s_col_name_a[COL_ID_TAXRATE]; }
        explicit TAXRATE(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit TAXRATE(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    struct STOCKSYMBOL : public TableOpV<wxString>
    {
        static COL_ID col_id() { return COL_ID_STOCKSYMBOL; }
        static wxString col_name() { return s_col_name_a[COL_ID_STOCKSYMBOL]; }
        explicit STOCKSYMBOL(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit STOCKSYMBOL(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct CONFIDENCE : public TableOpV<double>
    {
        static COL_ID col_id() { return COL_ID_CONFIDENCE; }
        static wxString col_name() { return s_col_name_a[COL_ID_CONFIDENCE]; }
        explicit CONFIDENCE(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit CONFIDENCE(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    struct SORTORDER : public TableOpV<int64>
    {
        static COL_ID col_id() { return COL_ID_SORTORDER; }
        static wxString col_name() { return s_col_name_a[COL_ID_SORTORDER]; }
        explicit SORTORDER(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit SORTORDER(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct ACTIVE : public TableOpV<int64>
    {
        static COL_ID col_id() { return COL_ID_ACTIVE; }
        static wxString col_name() { return s_col_name_a[COL_ID_ACTIVE]; }
        explicit ACTIVE(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit ACTIVE(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct PRICEASSUMPTIONID : public TableOpV<int64>
    {
        static COL_ID col_id() { return COL_ID_PRICEASSUMPTIONID; }
        static wxString col_name() { return s_col_name_a[COL_ID_PRICEASSUMPTIONID]; }
        explicit PRICEASSUMPTIONID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit PRICEASSUMPTIONID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct TAXASSUMPTIONID : public TableOpV<int64>
    {
        static COL_ID col_id() { return COL_ID_TAXASSUMPTIONID; }
        static wxString col_name() { return s_col_name_a[COL_ID_TAXASSUMPTIONID]; }
        explicit TAXASSUMPTIONID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit TAXASSUMPTIONID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct PRICEASSUMPTIONGROUPID : public TableOpV<int64>
    {
        static COL_ID col_id() { return COL_ID_PRICEASSUMPTIONGROUPID; }
        static wxString col_name() { return s_col_name_a[COL_ID_PRICEASSUMPTIONGROUPID]; }
        explicit PRICEASSUMPTIONGROUPID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit PRICEASSUMPTIONGROUPID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct TAXASSUMPTIONGROUPID : public TableOpV<int64>
    {
        static COL_ID col_id() { return COL_ID_TAXASSUMPTIONGROUPID; }
        static wxString col_name() { return s_col_name_a[COL_ID_TAXASSUMPTIONGROUPID]; }
        explicit TAXASSUMPTIONGROUPID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit TAXASSUMPTIONGROUPID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };
};

// A single record in database table PLANITEM_V1
struct PlanItemRow
{
    using Col = PlanItemCol;

    int64 PLANITEMID; // primary key
    int64 GROUPID;
    wxString ITEMNAME;
    wxString NOTES;
    wxString KIND;
    wxString STATUS;
    wxString TARGETDATE;
    double AMOUNT;
    int64 CURRENCYID;
    int64 CATEGID;
    int64 ACCOUNTID;
    double UNITS;
    double UNITPRICE;
    double TAXRATE;
    wxString STOCKSYMBOL;
    double CONFIDENCE;
    int64 SORTORDER;
    int64 ACTIVE;
    int64 PRICEASSUMPTIONID;
    int64 TAXASSUMPTIONID;
    int64 PRICEASSUMPTIONGROUPID;
    int64 TAXASSUMPTIONGROUPID;

    explicit PlanItemRow();
    explicit PlanItemRow(wxSQLite3ResultSet& q);
    PlanItemRow(const PlanItemRow& other) = default;

    int64 id() const { return PLANITEMID; }
    void id(const int64 id) { PLANITEMID = id; }
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    PlanItemRow& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    PlanItemRow& clone_from(const PlanItemRow& other);
    bool equals(const PlanItemRow* other) const;
    bool operator< (const PlanItemRow& other) const { return id() < other.id(); }
    bool operator< (const PlanItemRow* other) const { return id() < other->id(); }

    template<typename C>
    bool match(const C&)
    {
        return false;
    }

    // TODO: check if col.m_operator == OP_EQ

    bool match(const Col::PLANITEMID& col)
    {
        return PLANITEMID == col.m_value;
    }

    bool match(const Col::GROUPID& col)
    {
        return GROUPID == col.m_value;
    }

    bool match(const Col::ITEMNAME& col)
    {
        return ITEMNAME.CmpNoCase(col.m_value) == 0;
    }

    bool match(const Col::NOTES& col)
    {
        return NOTES.CmpNoCase(col.m_value) == 0;
    }

    bool match(const Col::KIND& col)
    {
        return KIND.CmpNoCase(col.m_value) == 0;
    }

    bool match(const Col::STATUS& col)
    {
        return STATUS.CmpNoCase(col.m_value) == 0;
    }

    bool match(const Col::TARGETDATE& col)
    {
        return TARGETDATE.CmpNoCase(col.m_value) == 0;
    }

    bool match(const Col::AMOUNT& col)
    {
        return AMOUNT == col.m_value;
    }

    bool match(const Col::CURRENCYID& col)
    {
        return CURRENCYID == col.m_value;
    }

    bool match(const Col::CATEGID& col)
    {
        return CATEGID == col.m_value;
    }

    bool match(const Col::ACCOUNTID& col)
    {
        return ACCOUNTID == col.m_value;
    }

    bool match(const Col::UNITS& col)
    {
        return UNITS == col.m_value;
    }

    bool match(const Col::UNITPRICE& col)
    {
        return UNITPRICE == col.m_value;
    }

    bool match(const Col::TAXRATE& col)
    {
        return TAXRATE == col.m_value;
    }

    bool match(const Col::STOCKSYMBOL& col)
    {
        return STOCKSYMBOL.CmpNoCase(col.m_value) == 0;
    }

    bool match(const Col::CONFIDENCE& col)
    {
        return CONFIDENCE == col.m_value;
    }

    bool match(const Col::SORTORDER& col)
    {
        return SORTORDER == col.m_value;
    }

    bool match(const Col::ACTIVE& col)
    {
        return ACTIVE == col.m_value;
    }

    bool match(const Col::PRICEASSUMPTIONID& col)
    {
        return PRICEASSUMPTIONID == col.m_value;
    }

    bool match(const Col::TAXASSUMPTIONID& col)
    {
        return TAXASSUMPTIONID == col.m_value;
    }

    bool match(const Col::PRICEASSUMPTIONGROUPID& col)
    {
        return PRICEASSUMPTIONGROUPID == col.m_value;
    }

    bool match(const Col::TAXASSUMPTIONGROUPID& col)
    {
        return TAXASSUMPTIONGROUPID == col.m_value;
    }

    template<typename Arg1, typename... Args>
    bool match(const Arg1& arg1, const Args&... args)
    {
        return (match(arg1) && ... && match(args));
    }

    struct SorterByPLANITEMID
    {
        bool operator()(const PlanItemRow& x, const PlanItemRow& y)
        {
            return x.PLANITEMID < y.PLANITEMID;
        }
    };

    struct SorterByGROUPID
    {
        bool operator()(const PlanItemRow& x, const PlanItemRow& y)
        {
            return x.GROUPID < y.GROUPID;
        }
    };

    struct SorterByITEMNAME
    {
        bool operator()(const PlanItemRow& x, const PlanItemRow& y)
        {
            return x.ITEMNAME < y.ITEMNAME;
        }
    };

    struct SorterByNOTES
    {
        bool operator()(const PlanItemRow& x, const PlanItemRow& y)
        {
            return x.NOTES < y.NOTES;
        }
    };

    struct SorterByKIND
    {
        bool operator()(const PlanItemRow& x, const PlanItemRow& y)
        {
            return x.KIND < y.KIND;
        }
    };

    struct SorterBySTATUS
    {
        bool operator()(const PlanItemRow& x, const PlanItemRow& y)
        {
            return x.STATUS < y.STATUS;
        }
    };

    struct SorterByTARGETDATE
    {
        bool operator()(const PlanItemRow& x, const PlanItemRow& y)
        {
            return x.TARGETDATE < y.TARGETDATE;
        }
    };

    struct SorterByAMOUNT
    {
        bool operator()(const PlanItemRow& x, const PlanItemRow& y)
        {
            return x.AMOUNT < y.AMOUNT;
        }
    };

    struct SorterByCURRENCYID
    {
        bool operator()(const PlanItemRow& x, const PlanItemRow& y)
        {
            return x.CURRENCYID < y.CURRENCYID;
        }
    };

    struct SorterByCATEGID
    {
        bool operator()(const PlanItemRow& x, const PlanItemRow& y)
        {
            return x.CATEGID < y.CATEGID;
        }
    };

    struct SorterByACCOUNTID
    {
        bool operator()(const PlanItemRow& x, const PlanItemRow& y)
        {
            return x.ACCOUNTID < y.ACCOUNTID;
        }
    };

    struct SorterByUNITS
    {
        bool operator()(const PlanItemRow& x, const PlanItemRow& y)
        {
            return x.UNITS < y.UNITS;
        }
    };

    struct SorterByUNITPRICE
    {
        bool operator()(const PlanItemRow& x, const PlanItemRow& y)
        {
            return x.UNITPRICE < y.UNITPRICE;
        }
    };

    struct SorterByTAXRATE
    {
        bool operator()(const PlanItemRow& x, const PlanItemRow& y)
        {
            return x.TAXRATE < y.TAXRATE;
        }
    };

    struct SorterBySTOCKSYMBOL
    {
        bool operator()(const PlanItemRow& x, const PlanItemRow& y)
        {
            return x.STOCKSYMBOL < y.STOCKSYMBOL;
        }
    };

    struct SorterByCONFIDENCE
    {
        bool operator()(const PlanItemRow& x, const PlanItemRow& y)
        {
            return x.CONFIDENCE < y.CONFIDENCE;
        }
    };

    struct SorterBySORTORDER
    {
        bool operator()(const PlanItemRow& x, const PlanItemRow& y)
        {
            return x.SORTORDER < y.SORTORDER;
        }
    };

    struct SorterByACTIVE
    {
        bool operator()(const PlanItemRow& x, const PlanItemRow& y)
        {
            return x.ACTIVE < y.ACTIVE;
        }
    };

    struct SorterByPRICEASSUMPTIONID
    {
        bool operator()(const PlanItemRow& x, const PlanItemRow& y)
        {
            return x.PRICEASSUMPTIONID < y.PRICEASSUMPTIONID;
        }
    };

    struct SorterByTAXASSUMPTIONID
    {
        bool operator()(const PlanItemRow& x, const PlanItemRow& y)
        {
            return x.TAXASSUMPTIONID < y.TAXASSUMPTIONID;
        }
    };

    struct SorterByPRICEASSUMPTIONGROUPID
    {
        bool operator()(const PlanItemRow& x, const PlanItemRow& y)
        {
            return x.PRICEASSUMPTIONGROUPID < y.PRICEASSUMPTIONGROUPID;
        }
    };

    struct SorterByTAXASSUMPTIONGROUPID
    {
        bool operator()(const PlanItemRow& x, const PlanItemRow& y)
        {
            return x.TAXASSUMPTIONGROUPID < y.TAXASSUMPTIONGROUPID;
        }
    };
};

// Interface to database table PLANITEM_V1
struct PlanItemTable : public TableBase
{
    using Row = PlanItemRow;
    using Col = typename Row::Col;

    PlanItemTable();
    ~PlanItemTable() {}
};

inline PlanItemRow::PlanItemRow(wxSQLite3ResultSet& q)
{
    from_select_result(q);
}

inline void PlanItemRow::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_insert_stmt(stmt, id());
}

inline PlanItemRow& PlanItemRow::clone_from(const PlanItemRow& other)
{
    *this = other;
    id(-1);
    return *this;
}
