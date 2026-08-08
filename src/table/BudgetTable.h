// -*- C++ -*-
//=============================================================================
/**
 *      Copyright: (c) 2013-2026 Guan Lisheng (guanlisheng@gmail.com)
 *      Copyright: (c) 2017-2018 Stefano Giorgio (stef145g)
 *      Copyright: (c) 2022      Mark Whalley (mark@ipx.co.uk)
 *      Copyright: (c) 2026      George Ef (george.a.ef@gmail.com)
 *
 *      BudgetTable.h
 *
 *      Interface to database table BUDGETTABLE_V1
 *
 *      @author [sqlite2cpp.py]
 *
 *      Revision History:
 *          AUTO GENERATED at 2026-08-08 11:34:44.604940.
 *          DO NOT EDIT!
 */
//=============================================================================

#pragma once

#include "_TableBase.h"

// Columns in database table BUDGETTABLE_V1
struct BudgetCol
{
    enum COL_ID
    {
        COL_ID_BUDGETENTRYID = 0,
        COL_ID_BUDGETYEARID,
        COL_ID_CATEGID,
        COL_ID_PERIOD,
        COL_ID_AMOUNT,
        COL_ID_NOTES,
        COL_ID_ACTIVE,
        COL_ID_SEGMENTID,
        COL_ID_AMOUNTTYPE,
        COL_ID_AUTOSOURCE,
        COL_ID_size
    };

    static const wxArrayString s_col_name_a;
    static const COL_ID s_primary_id;
    static const wxString s_primary_name;

    static wxString col_id_name(COL_ID col_id) { return s_col_name_a[col_id]; }

    // convenience variables

    static const wxString NAME_BUDGETENTRYID;
    static const wxString NAME_BUDGETYEARID;
    static const wxString NAME_CATEGID;
    static const wxString NAME_PERIOD;
    static const wxString NAME_AMOUNT;
    static const wxString NAME_NOTES;
    static const wxString NAME_ACTIVE;
    static const wxString NAME_SEGMENTID;
    static const wxString NAME_AMOUNTTYPE;
    static const wxString NAME_AUTOSOURCE;

    // convenience methods

    static TableClauseV<int64> WHERE_BUDGETENTRYID(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_BUDGETENTRYID, op, value);
    }

    static TableClauseV<int64> WHERE_BUDGETYEARID(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_BUDGETYEARID, op, value);
    }

    static TableClauseV<int64> WHERE_CATEGID(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_CATEGID, op, value);
    }

    static TableClauseV<wxString> WHERE_PERIOD(OP op, const wxString& value) {
        return TableClause::WHERE<wxString>(NAME_PERIOD, op, value);
    }

    static TableClauseV<double> WHERE_AMOUNT(OP op, const double& value) {
        return TableClause::WHERE<double>(NAME_AMOUNT, op, value);
    }

    static TableClauseV<wxString> WHERE_NOTES(OP op, const wxString& value) {
        return TableClause::WHERE<wxString>(NAME_NOTES, op, value);
    }

    static TableClauseV<int64> WHERE_ACTIVE(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_ACTIVE, op, value);
    }

    static TableClauseV<int64> WHERE_SEGMENTID(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_SEGMENTID, op, value);
    }

    static TableClauseV<wxString> WHERE_AMOUNTTYPE(OP op, const wxString& value) {
        return TableClause::WHERE<wxString>(NAME_AMOUNTTYPE, op, value);
    }

    static TableClauseV<wxString> WHERE_AUTOSOURCE(OP op, const wxString& value) {
        return TableClause::WHERE<wxString>(NAME_AUTOSOURCE, op, value);
    }

    // deprecated

    struct BUDGETENTRYID : public TableOpV<int64>
    {
        static COL_ID col_id() { return COL_ID_BUDGETENTRYID; }
        static wxString col_name() { return s_col_name_a[COL_ID_BUDGETENTRYID]; }
        explicit BUDGETENTRYID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit BUDGETENTRYID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct BUDGETYEARID : public TableOpV<int64>
    {
        static COL_ID col_id() { return COL_ID_BUDGETYEARID; }
        static wxString col_name() { return s_col_name_a[COL_ID_BUDGETYEARID]; }
        explicit BUDGETYEARID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit BUDGETYEARID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct CATEGID : public TableOpV<int64>
    {
        static COL_ID col_id() { return COL_ID_CATEGID; }
        static wxString col_name() { return s_col_name_a[COL_ID_CATEGID]; }
        explicit CATEGID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit CATEGID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct PERIOD : public TableOpV<wxString>
    {
        static COL_ID col_id() { return COL_ID_PERIOD; }
        static wxString col_name() { return s_col_name_a[COL_ID_PERIOD]; }
        explicit PERIOD(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit PERIOD(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct AMOUNT : public TableOpV<double>
    {
        static COL_ID col_id() { return COL_ID_AMOUNT; }
        static wxString col_name() { return s_col_name_a[COL_ID_AMOUNT]; }
        explicit AMOUNT(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit AMOUNT(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    struct NOTES : public TableOpV<wxString>
    {
        static COL_ID col_id() { return COL_ID_NOTES; }
        static wxString col_name() { return s_col_name_a[COL_ID_NOTES]; }
        explicit NOTES(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit NOTES(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct ACTIVE : public TableOpV<int64>
    {
        static COL_ID col_id() { return COL_ID_ACTIVE; }
        static wxString col_name() { return s_col_name_a[COL_ID_ACTIVE]; }
        explicit ACTIVE(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit ACTIVE(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct SEGMENTID : public TableOpV<int64>
    {
        static COL_ID col_id() { return COL_ID_SEGMENTID; }
        static wxString col_name() { return s_col_name_a[COL_ID_SEGMENTID]; }
        explicit SEGMENTID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit SEGMENTID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct AMOUNTTYPE : public TableOpV<wxString>
    {
        static COL_ID col_id() { return COL_ID_AMOUNTTYPE; }
        static wxString col_name() { return s_col_name_a[COL_ID_AMOUNTTYPE]; }
        explicit AMOUNTTYPE(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit AMOUNTTYPE(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct AUTOSOURCE : public TableOpV<wxString>
    {
        static COL_ID col_id() { return COL_ID_AUTOSOURCE; }
        static wxString col_name() { return s_col_name_a[COL_ID_AUTOSOURCE]; }
        explicit AUTOSOURCE(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit AUTOSOURCE(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };
};

// A single record in database table BUDGETTABLE_V1
struct BudgetRow
{
    using Col = BudgetCol;

    int64 BUDGETENTRYID; // primary key
    int64 BUDGETYEARID;
    int64 CATEGID;
    wxString PERIOD;
    double AMOUNT;
    wxString NOTES;
    int64 ACTIVE;
    int64 SEGMENTID;
    wxString AMOUNTTYPE;
    wxString AUTOSOURCE;

    explicit BudgetRow();
    explicit BudgetRow(wxSQLite3ResultSet& q);
    BudgetRow(const BudgetRow& other) = default;

    int64 id() const { return BUDGETENTRYID; }
    void id(const int64 id) { BUDGETENTRYID = id; }
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    BudgetRow& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    BudgetRow& clone_from(const BudgetRow& other);
    bool equals(const BudgetRow* other) const;
    bool operator< (const BudgetRow& other) const { return id() < other.id(); }
    bool operator< (const BudgetRow* other) const { return id() < other->id(); }

    template<typename C>
    bool match(const C&)
    {
        return false;
    }

    // TODO: check if col.m_operator == OP_EQ

    bool match(const Col::BUDGETENTRYID& col)
    {
        return BUDGETENTRYID == col.m_value;
    }

    bool match(const Col::BUDGETYEARID& col)
    {
        return BUDGETYEARID == col.m_value;
    }

    bool match(const Col::CATEGID& col)
    {
        return CATEGID == col.m_value;
    }

    bool match(const Col::PERIOD& col)
    {
        return PERIOD.CmpNoCase(col.m_value) == 0;
    }

    bool match(const Col::AMOUNT& col)
    {
        return AMOUNT == col.m_value;
    }

    bool match(const Col::NOTES& col)
    {
        return NOTES.CmpNoCase(col.m_value) == 0;
    }

    bool match(const Col::ACTIVE& col)
    {
        return ACTIVE == col.m_value;
    }

    bool match(const Col::SEGMENTID& col)
    {
        return SEGMENTID == col.m_value;
    }

    bool match(const Col::AMOUNTTYPE& col)
    {
        return AMOUNTTYPE.CmpNoCase(col.m_value) == 0;
    }

    bool match(const Col::AUTOSOURCE& col)
    {
        return AUTOSOURCE.CmpNoCase(col.m_value) == 0;
    }

    template<typename Arg1, typename... Args>
    bool match(const Arg1& arg1, const Args&... args)
    {
        return (match(arg1) && ... && match(args));
    }

    struct SorterByBUDGETENTRYID
    {
        bool operator()(const BudgetRow& x, const BudgetRow& y)
        {
            return x.BUDGETENTRYID < y.BUDGETENTRYID;
        }
    };

    struct SorterByBUDGETYEARID
    {
        bool operator()(const BudgetRow& x, const BudgetRow& y)
        {
            return x.BUDGETYEARID < y.BUDGETYEARID;
        }
    };

    struct SorterByCATEGID
    {
        bool operator()(const BudgetRow& x, const BudgetRow& y)
        {
            return x.CATEGID < y.CATEGID;
        }
    };

    struct SorterByPERIOD
    {
        bool operator()(const BudgetRow& x, const BudgetRow& y)
        {
            return x.PERIOD < y.PERIOD;
        }
    };

    struct SorterByAMOUNT
    {
        bool operator()(const BudgetRow& x, const BudgetRow& y)
        {
            return x.AMOUNT < y.AMOUNT;
        }
    };

    struct SorterByNOTES
    {
        bool operator()(const BudgetRow& x, const BudgetRow& y)
        {
            return x.NOTES < y.NOTES;
        }
    };

    struct SorterByACTIVE
    {
        bool operator()(const BudgetRow& x, const BudgetRow& y)
        {
            return x.ACTIVE < y.ACTIVE;
        }
    };

    struct SorterBySEGMENTID
    {
        bool operator()(const BudgetRow& x, const BudgetRow& y)
        {
            return x.SEGMENTID < y.SEGMENTID;
        }
    };

    struct SorterByAMOUNTTYPE
    {
        bool operator()(const BudgetRow& x, const BudgetRow& y)
        {
            return x.AMOUNTTYPE < y.AMOUNTTYPE;
        }
    };

    struct SorterByAUTOSOURCE
    {
        bool operator()(const BudgetRow& x, const BudgetRow& y)
        {
            return x.AUTOSOURCE < y.AUTOSOURCE;
        }
    };
};

// Interface to database table BUDGETTABLE_V1
struct BudgetTable : public TableBase
{
    using Row = BudgetRow;
    using Col = typename Row::Col;

    BudgetTable();
    ~BudgetTable() {}
};

inline BudgetRow::BudgetRow(wxSQLite3ResultSet& q)
{
    from_select_result(q);
}

inline void BudgetRow::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_insert_stmt(stmt, id());
}

inline BudgetRow& BudgetRow::clone_from(const BudgetRow& other)
{
    *this = other;
    id(-1);
    return *this;
}
