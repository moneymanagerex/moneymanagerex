// -*- C++ -*-
//=============================================================================
/**
 *      Copyright: (c) 2013-2026 Guan Lisheng (guanlisheng@gmail.com)
 *      Copyright: (c) 2017-2018 Stefano Giorgio (stef145g)
 *      Copyright: (c) 2022      Mark Whalley (mark@ipx.co.uk)
 *      Copyright: (c) 2026      George Ef (george.a.ef@gmail.com)
 *
 *      BudgetSegmentTable.h
 *
 *      Interface to database table BUDGETSEGMENT_V1
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

// Columns in database table BUDGETSEGMENT_V1
struct BudgetSegmentCol
{
    enum COL_ID
    {
        COL_ID_SEGMENTID = 0,
        COL_ID_BUDGETYEARID,
        COL_ID_SEGMENTNAME,
        COL_ID_STARTDAY,
        COL_ID_ENDDAY,
        COL_ID_SORTORDER,
        COL_ID_ACTIVE,
        COL_ID_size
    };

    static const wxArrayString s_col_name_a;
    static const COL_ID s_primary_id;
    static const wxString s_primary_name;

    static wxString col_id_name(COL_ID col_id) { return s_col_name_a[col_id]; }

    // convenience variables

    static const wxString NAME_SEGMENTID;
    static const wxString NAME_BUDGETYEARID;
    static const wxString NAME_SEGMENTNAME;
    static const wxString NAME_STARTDAY;
    static const wxString NAME_ENDDAY;
    static const wxString NAME_SORTORDER;
    static const wxString NAME_ACTIVE;

    // convenience methods

    static TableClauseV<int64> WHERE_SEGMENTID(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_SEGMENTID, op, value);
    }

    static TableClauseV<int64> WHERE_BUDGETYEARID(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_BUDGETYEARID, op, value);
    }

    static TableClauseV<wxString> WHERE_SEGMENTNAME(OP op, const wxString& value) {
        return TableClause::WHERE<wxString>(NAME_SEGMENTNAME, op, value);
    }

    static TableClauseV<int64> WHERE_STARTDAY(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_STARTDAY, op, value);
    }

    static TableClauseV<int64> WHERE_ENDDAY(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_ENDDAY, op, value);
    }

    static TableClauseV<int64> WHERE_SORTORDER(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_SORTORDER, op, value);
    }

    static TableClauseV<int64> WHERE_ACTIVE(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_ACTIVE, op, value);
    }

    // deprecated

    struct SEGMENTID : public TableOpV<int64>
    {
        static COL_ID col_id() { return COL_ID_SEGMENTID; }
        static wxString col_name() { return s_col_name_a[COL_ID_SEGMENTID]; }
        explicit SEGMENTID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit SEGMENTID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct BUDGETYEARID : public TableOpV<int64>
    {
        static COL_ID col_id() { return COL_ID_BUDGETYEARID; }
        static wxString col_name() { return s_col_name_a[COL_ID_BUDGETYEARID]; }
        explicit BUDGETYEARID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit BUDGETYEARID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct SEGMENTNAME : public TableOpV<wxString>
    {
        static COL_ID col_id() { return COL_ID_SEGMENTNAME; }
        static wxString col_name() { return s_col_name_a[COL_ID_SEGMENTNAME]; }
        explicit SEGMENTNAME(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit SEGMENTNAME(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct STARTDAY : public TableOpV<int64>
    {
        static COL_ID col_id() { return COL_ID_STARTDAY; }
        static wxString col_name() { return s_col_name_a[COL_ID_STARTDAY]; }
        explicit STARTDAY(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit STARTDAY(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct ENDDAY : public TableOpV<int64>
    {
        static COL_ID col_id() { return COL_ID_ENDDAY; }
        static wxString col_name() { return s_col_name_a[COL_ID_ENDDAY]; }
        explicit ENDDAY(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit ENDDAY(OP op, const int64 &v): TableOpV<int64>(op, v) {}
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
};

// A single record in database table BUDGETSEGMENT_V1
struct BudgetSegmentRow
{
    using Col = BudgetSegmentCol;

    int64 SEGMENTID; // primary key
    int64 BUDGETYEARID;
    wxString SEGMENTNAME;
    int64 STARTDAY;
    int64 ENDDAY;
    int64 SORTORDER;
    int64 ACTIVE;

    explicit BudgetSegmentRow();
    explicit BudgetSegmentRow(wxSQLite3ResultSet& q);
    BudgetSegmentRow(const BudgetSegmentRow& other) = default;

    int64 id() const { return SEGMENTID; }
    void id(const int64 id) { SEGMENTID = id; }
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    BudgetSegmentRow& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    BudgetSegmentRow& clone_from(const BudgetSegmentRow& other);
    bool equals(const BudgetSegmentRow* other) const;
    bool operator< (const BudgetSegmentRow& other) const { return id() < other.id(); }
    bool operator< (const BudgetSegmentRow* other) const { return id() < other->id(); }

    template<typename C>
    bool match(const C&)
    {
        return false;
    }

    // TODO: check if col.m_operator == OP_EQ

    bool match(const Col::SEGMENTID& col)
    {
        return SEGMENTID == col.m_value;
    }

    bool match(const Col::BUDGETYEARID& col)
    {
        return BUDGETYEARID == col.m_value;
    }

    bool match(const Col::SEGMENTNAME& col)
    {
        return SEGMENTNAME.CmpNoCase(col.m_value) == 0;
    }

    bool match(const Col::STARTDAY& col)
    {
        return STARTDAY == col.m_value;
    }

    bool match(const Col::ENDDAY& col)
    {
        return ENDDAY == col.m_value;
    }

    bool match(const Col::SORTORDER& col)
    {
        return SORTORDER == col.m_value;
    }

    bool match(const Col::ACTIVE& col)
    {
        return ACTIVE == col.m_value;
    }

    template<typename Arg1, typename... Args>
    bool match(const Arg1& arg1, const Args&... args)
    {
        return (match(arg1) && ... && match(args));
    }

    struct SorterBySEGMENTID
    {
        bool operator()(const BudgetSegmentRow& x, const BudgetSegmentRow& y)
        {
            return x.SEGMENTID < y.SEGMENTID;
        }
    };

    struct SorterByBUDGETYEARID
    {
        bool operator()(const BudgetSegmentRow& x, const BudgetSegmentRow& y)
        {
            return x.BUDGETYEARID < y.BUDGETYEARID;
        }
    };

    struct SorterBySEGMENTNAME
    {
        bool operator()(const BudgetSegmentRow& x, const BudgetSegmentRow& y)
        {
            return x.SEGMENTNAME < y.SEGMENTNAME;
        }
    };

    struct SorterBySTARTDAY
    {
        bool operator()(const BudgetSegmentRow& x, const BudgetSegmentRow& y)
        {
            return x.STARTDAY < y.STARTDAY;
        }
    };

    struct SorterByENDDAY
    {
        bool operator()(const BudgetSegmentRow& x, const BudgetSegmentRow& y)
        {
            return x.ENDDAY < y.ENDDAY;
        }
    };

    struct SorterBySORTORDER
    {
        bool operator()(const BudgetSegmentRow& x, const BudgetSegmentRow& y)
        {
            return x.SORTORDER < y.SORTORDER;
        }
    };

    struct SorterByACTIVE
    {
        bool operator()(const BudgetSegmentRow& x, const BudgetSegmentRow& y)
        {
            return x.ACTIVE < y.ACTIVE;
        }
    };
};

// Interface to database table BUDGETSEGMENT_V1
struct BudgetSegmentTable : public TableBase
{
    using Row = BudgetSegmentRow;
    using Col = typename Row::Col;

    BudgetSegmentTable();
    ~BudgetSegmentTable() {}
};

inline BudgetSegmentRow::BudgetSegmentRow(wxSQLite3ResultSet& q)
{
    from_select_result(q);
}

inline void BudgetSegmentRow::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_insert_stmt(stmt, id());
}

inline BudgetSegmentRow& BudgetSegmentRow::clone_from(const BudgetSegmentRow& other)
{
    *this = other;
    id(-1);
    return *this;
}
