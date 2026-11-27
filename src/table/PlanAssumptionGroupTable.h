// -*- C++ -*-
//=============================================================================
/**
 *      Copyright: (c) 2013-2026 Guan Lisheng (guanlisheng@gmail.com)
 *      Copyright: (c) 2017-2018 Stefano Giorgio (stef145g)
 *      Copyright: (c) 2022      Mark Whalley (mark@ipx.co.uk)
 *      Copyright: (c) 2026      George Ef (george.a.ef@gmail.com)
 *
 *      PlanAssumptionGroupTable.h
 *
 *      Interface to database table PLANASSUMPTIONGROUP_V1
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

// Columns in database table PLANASSUMPTIONGROUP_V1
struct PlanAssumptionGroupCol
{
    enum COL_ID
    {
        COL_ID_GROUPID = 0,
        COL_ID_GROUPNAME,
        COL_ID_KIND,
        COL_ID_SCOPEKEY,
        COL_ID_UNIT,
        COL_ID_ACTIVEASSUMPTIONID,
        COL_ID_NOTES,
        COL_ID_ACTIVE,
        COL_ID_size
    };

    static const wxArrayString s_col_name_a;
    static const COL_ID s_primary_id;
    static const wxString s_primary_name;

    static wxString col_id_name(COL_ID col_id) { return s_col_name_a[col_id]; }

    // convenience variables

    static const wxString NAME_GROUPID;
    static const wxString NAME_GROUPNAME;
    static const wxString NAME_KIND;
    static const wxString NAME_SCOPEKEY;
    static const wxString NAME_UNIT;
    static const wxString NAME_ACTIVEASSUMPTIONID;
    static const wxString NAME_NOTES;
    static const wxString NAME_ACTIVE;

    // convenience methods

    static TableClauseV<int64> WHERE_GROUPID(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_GROUPID, op, value);
    }

    static TableClauseV<wxString> WHERE_GROUPNAME(OP op, const wxString& value) {
        return TableClause::WHERE<wxString>(NAME_GROUPNAME, op, value);
    }

    static TableClauseV<wxString> WHERE_KIND(OP op, const wxString& value) {
        return TableClause::WHERE<wxString>(NAME_KIND, op, value);
    }

    static TableClauseV<wxString> WHERE_SCOPEKEY(OP op, const wxString& value) {
        return TableClause::WHERE<wxString>(NAME_SCOPEKEY, op, value);
    }

    static TableClauseV<wxString> WHERE_UNIT(OP op, const wxString& value) {
        return TableClause::WHERE<wxString>(NAME_UNIT, op, value);
    }

    static TableClauseV<int64> WHERE_ACTIVEASSUMPTIONID(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_ACTIVEASSUMPTIONID, op, value);
    }

    static TableClauseV<wxString> WHERE_NOTES(OP op, const wxString& value) {
        return TableClause::WHERE<wxString>(NAME_NOTES, op, value);
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

    struct GROUPNAME : public TableOpV<wxString>
    {
        static COL_ID col_id() { return COL_ID_GROUPNAME; }
        static wxString col_name() { return s_col_name_a[COL_ID_GROUPNAME]; }
        explicit GROUPNAME(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit GROUPNAME(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct KIND : public TableOpV<wxString>
    {
        static COL_ID col_id() { return COL_ID_KIND; }
        static wxString col_name() { return s_col_name_a[COL_ID_KIND]; }
        explicit KIND(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit KIND(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct SCOPEKEY : public TableOpV<wxString>
    {
        static COL_ID col_id() { return COL_ID_SCOPEKEY; }
        static wxString col_name() { return s_col_name_a[COL_ID_SCOPEKEY]; }
        explicit SCOPEKEY(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit SCOPEKEY(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct UNIT : public TableOpV<wxString>
    {
        static COL_ID col_id() { return COL_ID_UNIT; }
        static wxString col_name() { return s_col_name_a[COL_ID_UNIT]; }
        explicit UNIT(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit UNIT(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct ACTIVEASSUMPTIONID : public TableOpV<int64>
    {
        static COL_ID col_id() { return COL_ID_ACTIVEASSUMPTIONID; }
        static wxString col_name() { return s_col_name_a[COL_ID_ACTIVEASSUMPTIONID]; }
        explicit ACTIVEASSUMPTIONID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit ACTIVEASSUMPTIONID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
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
};

// A single record in database table PLANASSUMPTIONGROUP_V1
struct PlanAssumptionGroupRow
{
    using Col = PlanAssumptionGroupCol;

    int64 GROUPID; // primary key
    wxString GROUPNAME;
    wxString KIND;
    wxString SCOPEKEY;
    wxString UNIT;
    int64 ACTIVEASSUMPTIONID;
    wxString NOTES;
    int64 ACTIVE;

    explicit PlanAssumptionGroupRow();
    explicit PlanAssumptionGroupRow(wxSQLite3ResultSet& q);
    PlanAssumptionGroupRow(const PlanAssumptionGroupRow& other) = default;

    int64 id() const { return GROUPID; }
    void id(const int64 id) { GROUPID = id; }
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    PlanAssumptionGroupRow& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    PlanAssumptionGroupRow& clone_from(const PlanAssumptionGroupRow& other);
    bool equals(const PlanAssumptionGroupRow* other) const;
    bool operator< (const PlanAssumptionGroupRow& other) const { return id() < other.id(); }
    bool operator< (const PlanAssumptionGroupRow* other) const { return id() < other->id(); }

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

    bool match(const Col::GROUPNAME& col)
    {
        return GROUPNAME.CmpNoCase(col.m_value) == 0;
    }

    bool match(const Col::KIND& col)
    {
        return KIND.CmpNoCase(col.m_value) == 0;
    }

    bool match(const Col::SCOPEKEY& col)
    {
        return SCOPEKEY.CmpNoCase(col.m_value) == 0;
    }

    bool match(const Col::UNIT& col)
    {
        return UNIT.CmpNoCase(col.m_value) == 0;
    }

    bool match(const Col::ACTIVEASSUMPTIONID& col)
    {
        return ACTIVEASSUMPTIONID == col.m_value;
    }

    bool match(const Col::NOTES& col)
    {
        return NOTES.CmpNoCase(col.m_value) == 0;
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
        bool operator()(const PlanAssumptionGroupRow& x, const PlanAssumptionGroupRow& y)
        {
            return x.GROUPID < y.GROUPID;
        }
    };

    struct SorterByGROUPNAME
    {
        bool operator()(const PlanAssumptionGroupRow& x, const PlanAssumptionGroupRow& y)
        {
            return x.GROUPNAME < y.GROUPNAME;
        }
    };

    struct SorterByKIND
    {
        bool operator()(const PlanAssumptionGroupRow& x, const PlanAssumptionGroupRow& y)
        {
            return x.KIND < y.KIND;
        }
    };

    struct SorterBySCOPEKEY
    {
        bool operator()(const PlanAssumptionGroupRow& x, const PlanAssumptionGroupRow& y)
        {
            return x.SCOPEKEY < y.SCOPEKEY;
        }
    };

    struct SorterByUNIT
    {
        bool operator()(const PlanAssumptionGroupRow& x, const PlanAssumptionGroupRow& y)
        {
            return x.UNIT < y.UNIT;
        }
    };

    struct SorterByACTIVEASSUMPTIONID
    {
        bool operator()(const PlanAssumptionGroupRow& x, const PlanAssumptionGroupRow& y)
        {
            return x.ACTIVEASSUMPTIONID < y.ACTIVEASSUMPTIONID;
        }
    };

    struct SorterByNOTES
    {
        bool operator()(const PlanAssumptionGroupRow& x, const PlanAssumptionGroupRow& y)
        {
            return x.NOTES < y.NOTES;
        }
    };

    struct SorterByACTIVE
    {
        bool operator()(const PlanAssumptionGroupRow& x, const PlanAssumptionGroupRow& y)
        {
            return x.ACTIVE < y.ACTIVE;
        }
    };
};

// Interface to database table PLANASSUMPTIONGROUP_V1
struct PlanAssumptionGroupTable : public TableBase
{
    using Row = PlanAssumptionGroupRow;
    using Col = typename Row::Col;

    PlanAssumptionGroupTable();
    ~PlanAssumptionGroupTable() {}
};

inline PlanAssumptionGroupRow::PlanAssumptionGroupRow(wxSQLite3ResultSet& q)
{
    from_select_result(q);
}

inline void PlanAssumptionGroupRow::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_insert_stmt(stmt, id());
}

inline PlanAssumptionGroupRow& PlanAssumptionGroupRow::clone_from(const PlanAssumptionGroupRow& other)
{
    *this = other;
    id(-1);
    return *this;
}
