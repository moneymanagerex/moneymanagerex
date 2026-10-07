// -*- C++ -*-
//=============================================================================
/**
 *      Copyright: (c) 2013-2026 Guan Lisheng (guanlisheng@gmail.com)
 *      Copyright: (c) 2017-2018 Stefano Giorgio (stef145g)
 *      Copyright: (c) 2022      Mark Whalley (mark@ipx.co.uk)
 *      Copyright: (c) 2026      George Ef (george.a.ef@gmail.com)
 *
 *      PlanGroupTable.h
 *
 *      Interface to database table PLANGROUP_V1
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

// Columns in database table PLANGROUP_V1
struct PlanGroupCol
{
    enum COL_ID
    {
        COL_ID_GROUPID = 0,
        COL_ID_PARENTID,
        COL_ID_GROUPNAME,
        COL_ID_NOTES,
        COL_ID_STATUS,
        COL_ID_TARGETDATE,
        COL_ID_SORTORDER,
        COL_ID_ACTIVE,
        COL_ID_size
    };

    static const wxArrayString s_col_name_a;
    static const COL_ID s_primary_id;
    static const wxString s_primary_name;

    static wxString col_id_name(COL_ID col_id) { return s_col_name_a[col_id]; }

    // convenience variables

    static const wxString NAME_GROUPID;
    static const wxString NAME_PARENTID;
    static const wxString NAME_GROUPNAME;
    static const wxString NAME_NOTES;
    static const wxString NAME_STATUS;
    static const wxString NAME_TARGETDATE;
    static const wxString NAME_SORTORDER;
    static const wxString NAME_ACTIVE;

    // convenience methods

    static TableClauseV<int64> WHERE_GROUPID(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_GROUPID, op, value);
    }

    static TableClauseV<int64> WHERE_PARENTID(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_PARENTID, op, value);
    }

    static TableClauseV<wxString> WHERE_GROUPNAME(OP op, const wxString& value) {
        return TableClause::WHERE<wxString>(NAME_GROUPNAME, op, value);
    }

    static TableClauseV<wxString> WHERE_NOTES(OP op, const wxString& value) {
        return TableClause::WHERE<wxString>(NAME_NOTES, op, value);
    }

    static TableClauseV<wxString> WHERE_STATUS(OP op, const wxString& value) {
        return TableClause::WHERE<wxString>(NAME_STATUS, op, value);
    }

    static TableClauseV<wxString> WHERE_TARGETDATE(OP op, const wxString& value) {
        return TableClause::WHERE<wxString>(NAME_TARGETDATE, op, value);
    }

    static TableClauseV<int64> WHERE_SORTORDER(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_SORTORDER, op, value);
    }

    static TableClauseV<int64> WHERE_ACTIVE(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_ACTIVE, op, value);
    }

    // deprecated

    struct GROUPID : public TableOpV<int64>
    {
        static COL_ID col_id() { return COL_ID_GROUPID; }
        static wxString col_name() { return s_col_name_a[COL_ID_GROUPID]; }
        explicit GROUPID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit GROUPID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct PARENTID : public TableOpV<int64>
    {
        static COL_ID col_id() { return COL_ID_PARENTID; }
        static wxString col_name() { return s_col_name_a[COL_ID_PARENTID]; }
        explicit PARENTID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit PARENTID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct GROUPNAME : public TableOpV<wxString>
    {
        static COL_ID col_id() { return COL_ID_GROUPNAME; }
        static wxString col_name() { return s_col_name_a[COL_ID_GROUPNAME]; }
        explicit GROUPNAME(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit GROUPNAME(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct NOTES : public TableOpV<wxString>
    {
        static COL_ID col_id() { return COL_ID_NOTES; }
        static wxString col_name() { return s_col_name_a[COL_ID_NOTES]; }
        explicit NOTES(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit NOTES(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
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

// A single record in database table PLANGROUP_V1
struct PlanGroupRow
{
    using Col = PlanGroupCol;

    int64 GROUPID; // primary key
    int64 PARENTID;
    wxString GROUPNAME;
    wxString NOTES;
    wxString STATUS;
    wxString TARGETDATE;
    int64 SORTORDER;
    int64 ACTIVE;

    explicit PlanGroupRow();
    explicit PlanGroupRow(wxSQLite3ResultSet& q);
    PlanGroupRow(const PlanGroupRow& other) = default;

    int64 id() const { return GROUPID; }
    void id(const int64 id) { GROUPID = id; }
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    PlanGroupRow& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    PlanGroupRow& clone_from(const PlanGroupRow& other);
    bool equals(const PlanGroupRow* other) const;
    bool operator< (const PlanGroupRow& other) const { return id() < other.id(); }
    bool operator< (const PlanGroupRow* other) const { return id() < other->id(); }

    template<typename C>
    bool match(const C&)
    {
        return false;
    }

    // TODO: check if col.m_operator == OP_EQ

    bool match(const Col::GROUPID& col)
    {
        return GROUPID == col.m_value;
    }

    bool match(const Col::PARENTID& col)
    {
        return PARENTID == col.m_value;
    }

    bool match(const Col::GROUPNAME& col)
    {
        return GROUPNAME.CmpNoCase(col.m_value) == 0;
    }

    bool match(const Col::NOTES& col)
    {
        return NOTES.CmpNoCase(col.m_value) == 0;
    }

    bool match(const Col::STATUS& col)
    {
        return STATUS.CmpNoCase(col.m_value) == 0;
    }

    bool match(const Col::TARGETDATE& col)
    {
        return TARGETDATE.CmpNoCase(col.m_value) == 0;
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

    struct SorterByGROUPID
    {
        bool operator()(const PlanGroupRow& x, const PlanGroupRow& y)
        {
            return x.GROUPID < y.GROUPID;
        }
    };

    struct SorterByPARENTID
    {
        bool operator()(const PlanGroupRow& x, const PlanGroupRow& y)
        {
            return x.PARENTID < y.PARENTID;
        }
    };

    struct SorterByGROUPNAME
    {
        bool operator()(const PlanGroupRow& x, const PlanGroupRow& y)
        {
            return x.GROUPNAME < y.GROUPNAME;
        }
    };

    struct SorterByNOTES
    {
        bool operator()(const PlanGroupRow& x, const PlanGroupRow& y)
        {
            return x.NOTES < y.NOTES;
        }
    };

    struct SorterBySTATUS
    {
        bool operator()(const PlanGroupRow& x, const PlanGroupRow& y)
        {
            return x.STATUS < y.STATUS;
        }
    };

    struct SorterByTARGETDATE
    {
        bool operator()(const PlanGroupRow& x, const PlanGroupRow& y)
        {
            return x.TARGETDATE < y.TARGETDATE;
        }
    };

    struct SorterBySORTORDER
    {
        bool operator()(const PlanGroupRow& x, const PlanGroupRow& y)
        {
            return x.SORTORDER < y.SORTORDER;
        }
    };

    struct SorterByACTIVE
    {
        bool operator()(const PlanGroupRow& x, const PlanGroupRow& y)
        {
            return x.ACTIVE < y.ACTIVE;
        }
    };
};

// Interface to database table PLANGROUP_V1
struct PlanGroupTable : public TableBase
{
    using Row = PlanGroupRow;
    using Col = typename Row::Col;

    PlanGroupTable();
    ~PlanGroupTable() {}
};

inline PlanGroupRow::PlanGroupRow(wxSQLite3ResultSet& q)
{
    from_select_result(q);
}

inline void PlanGroupRow::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_insert_stmt(stmt, id());
}

inline PlanGroupRow& PlanGroupRow::clone_from(const PlanGroupRow& other)
{
    *this = other;
    id(-1);
    return *this;
}
