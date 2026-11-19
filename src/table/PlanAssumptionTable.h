// -*- C++ -*-
//=============================================================================
/**
 *      Copyright: (c) 2013-2026 Guan Lisheng (guanlisheng@gmail.com)
 *      Copyright: (c) 2017-2018 Stefano Giorgio (stef145g)
 *      Copyright: (c) 2022      Mark Whalley (mark@ipx.co.uk)
 *      Copyright: (c) 2026      George Ef (george.a.ef@gmail.com)
 *
 *      PlanAssumptionTable.h
 *
 *      Interface to database table PLANASSUMPTION_V1
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

// Columns in database table PLANASSUMPTION_V1
struct PlanAssumptionCol
{
    enum COL_ID
    {
        COL_ID_ASSUMPTIONID = 0,
        COL_ID_ASSUMPTIONNAME,
        COL_ID_KIND,
        COL_ID_VALUE,
        COL_ID_SCOPEKEY,
        COL_ID_NOTES,
        COL_ID_ASOFDATE,
        COL_ID_ACTIVE,
        COL_ID_UNIT,
        COL_ID_GROUPID,
        COL_ID_size
    };

    static const wxArrayString s_col_name_a;
    static const COL_ID s_primary_id;
    static const wxString s_primary_name;

    static wxString col_id_name(COL_ID col_id) { return s_col_name_a[col_id]; }

    // convenience variables

    static const wxString NAME_ASSUMPTIONID;
    static const wxString NAME_ASSUMPTIONNAME;
    static const wxString NAME_KIND;
    static const wxString NAME_VALUE;
    static const wxString NAME_SCOPEKEY;
    static const wxString NAME_NOTES;
    static const wxString NAME_ASOFDATE;
    static const wxString NAME_ACTIVE;
    static const wxString NAME_UNIT;
    static const wxString NAME_GROUPID;

    // convenience methods

    static TableClauseV<int64> WHERE_ASSUMPTIONID(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_ASSUMPTIONID, op, value);
    }

    static TableClauseV<wxString> WHERE_ASSUMPTIONNAME(OP op, const wxString& value) {
        return TableClause::WHERE<wxString>(NAME_ASSUMPTIONNAME, op, value);
    }

    static TableClauseV<wxString> WHERE_KIND(OP op, const wxString& value) {
        return TableClause::WHERE<wxString>(NAME_KIND, op, value);
    }

    static TableClauseV<double> WHERE_VALUE(OP op, const double& value) {
        return TableClause::WHERE<double>(NAME_VALUE, op, value);
    }

    static TableClauseV<wxString> WHERE_SCOPEKEY(OP op, const wxString& value) {
        return TableClause::WHERE<wxString>(NAME_SCOPEKEY, op, value);
    }

    static TableClauseV<wxString> WHERE_NOTES(OP op, const wxString& value) {
        return TableClause::WHERE<wxString>(NAME_NOTES, op, value);
    }

    static TableClauseV<wxString> WHERE_ASOFDATE(OP op, const wxString& value) {
        return TableClause::WHERE<wxString>(NAME_ASOFDATE, op, value);
    }

    static TableClauseV<int64> WHERE_ACTIVE(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_ACTIVE, op, value);
    }

    static TableClauseV<wxString> WHERE_UNIT(OP op, const wxString& value) {
        return TableClause::WHERE<wxString>(NAME_UNIT, op, value);
    }

    static TableClauseV<int64> WHERE_GROUPID(OP op, const int64& value) {
        return TableClause::WHERE<int64>(NAME_GROUPID, op, value);
    }

    // deprecated

    struct ASSUMPTIONID : public TableOpV<int64>
    {
        static COL_ID col_id() { return COL_ID_ASSUMPTIONID; }
        static wxString col_name() { return s_col_name_a[COL_ID_ASSUMPTIONID]; }
        explicit ASSUMPTIONID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit ASSUMPTIONID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct ASSUMPTIONNAME : public TableOpV<wxString>
    {
        static COL_ID col_id() { return COL_ID_ASSUMPTIONNAME; }
        static wxString col_name() { return s_col_name_a[COL_ID_ASSUMPTIONNAME]; }
        explicit ASSUMPTIONNAME(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit ASSUMPTIONNAME(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct KIND : public TableOpV<wxString>
    {
        static COL_ID col_id() { return COL_ID_KIND; }
        static wxString col_name() { return s_col_name_a[COL_ID_KIND]; }
        explicit KIND(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit KIND(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct VALUE : public TableOpV<double>
    {
        static COL_ID col_id() { return COL_ID_VALUE; }
        static wxString col_name() { return s_col_name_a[COL_ID_VALUE]; }
        explicit VALUE(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit VALUE(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    struct SCOPEKEY : public TableOpV<wxString>
    {
        static COL_ID col_id() { return COL_ID_SCOPEKEY; }
        static wxString col_name() { return s_col_name_a[COL_ID_SCOPEKEY]; }
        explicit SCOPEKEY(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit SCOPEKEY(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct NOTES : public TableOpV<wxString>
    {
        static COL_ID col_id() { return COL_ID_NOTES; }
        static wxString col_name() { return s_col_name_a[COL_ID_NOTES]; }
        explicit NOTES(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit NOTES(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct ASOFDATE : public TableOpV<wxString>
    {
        static COL_ID col_id() { return COL_ID_ASOFDATE; }
        static wxString col_name() { return s_col_name_a[COL_ID_ASOFDATE]; }
        explicit ASOFDATE(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit ASOFDATE(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct ACTIVE : public TableOpV<int64>
    {
        static COL_ID col_id() { return COL_ID_ACTIVE; }
        static wxString col_name() { return s_col_name_a[COL_ID_ACTIVE]; }
        explicit ACTIVE(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit ACTIVE(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct UNIT : public TableOpV<wxString>
    {
        static COL_ID col_id() { return COL_ID_UNIT; }
        static wxString col_name() { return s_col_name_a[COL_ID_UNIT]; }
        explicit UNIT(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit UNIT(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct GROUPID : public TableOpV<int64>
    {
        static COL_ID col_id() { return COL_ID_GROUPID; }
        static wxString col_name() { return s_col_name_a[COL_ID_GROUPID]; }
        explicit GROUPID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit GROUPID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };
};

// A single record in database table PLANASSUMPTION_V1
struct PlanAssumptionRow
{
    using Col = PlanAssumptionCol;

    int64 ASSUMPTIONID; // primary key
    wxString ASSUMPTIONNAME;
    wxString KIND;
    double VALUE;
    wxString SCOPEKEY;
    wxString NOTES;
    wxString ASOFDATE;
    int64 ACTIVE;
    wxString UNIT;
    int64 GROUPID;

    explicit PlanAssumptionRow();
    explicit PlanAssumptionRow(wxSQLite3ResultSet& q);
    PlanAssumptionRow(const PlanAssumptionRow& other) = default;

    int64 id() const { return ASSUMPTIONID; }
    void id(const int64 id) { ASSUMPTIONID = id; }
    void to_insert_stmt(wxSQLite3Statement& stmt, int64 id) const;
    void to_update_stmt(wxSQLite3Statement& stmt) const;
    PlanAssumptionRow& from_select_result(wxSQLite3ResultSet& q);
    wxString to_json() const;
    void as_json(PrettyWriter<StringBuffer>& json_writer) const;
    row_t to_html_row() const;
    void to_html_template(html_template& t) const;
    void destroy() { delete this; }

    PlanAssumptionRow& clone_from(const PlanAssumptionRow& other);
    bool equals(const PlanAssumptionRow* other) const;
    bool operator< (const PlanAssumptionRow& other) const { return id() < other.id(); }
    bool operator< (const PlanAssumptionRow* other) const { return id() < other->id(); }

    template<typename C>
    bool match(const C&)
    {
        return false;
    }

    // TODO: check if col.m_operator == OP_EQ

    bool match(const Col::ASSUMPTIONID& col)
    {
        return ASSUMPTIONID == col.m_value;
    }

    bool match(const Col::ASSUMPTIONNAME& col)
    {
        return ASSUMPTIONNAME.CmpNoCase(col.m_value) == 0;
    }

    bool match(const Col::KIND& col)
    {
        return KIND.CmpNoCase(col.m_value) == 0;
    }

    bool match(const Col::VALUE& col)
    {
        return VALUE == col.m_value;
    }

    bool match(const Col::SCOPEKEY& col)
    {
        return SCOPEKEY.CmpNoCase(col.m_value) == 0;
    }

    bool match(const Col::NOTES& col)
    {
        return NOTES.CmpNoCase(col.m_value) == 0;
    }

    bool match(const Col::ASOFDATE& col)
    {
        return ASOFDATE.CmpNoCase(col.m_value) == 0;
    }

    bool match(const Col::ACTIVE& col)
    {
        return ACTIVE == col.m_value;
    }

    bool match(const Col::UNIT& col)
    {
        return UNIT.CmpNoCase(col.m_value) == 0;
    }

    bool match(const Col::GROUPID& col)
    {
        return GROUPID == col.m_value;
    }

    template<typename Arg1, typename... Args>
    bool match(const Arg1& arg1, const Args&... args)
    {
        return (match(arg1) && ... && match(args));
    }

    struct SorterByASSUMPTIONID
    {
        bool operator()(const PlanAssumptionRow& x, const PlanAssumptionRow& y)
        {
            return x.ASSUMPTIONID < y.ASSUMPTIONID;
        }
    };

    struct SorterByASSUMPTIONNAME
    {
        bool operator()(const PlanAssumptionRow& x, const PlanAssumptionRow& y)
        {
            return x.ASSUMPTIONNAME < y.ASSUMPTIONNAME;
        }
    };

    struct SorterByKIND
    {
        bool operator()(const PlanAssumptionRow& x, const PlanAssumptionRow& y)
        {
            return x.KIND < y.KIND;
        }
    };

    struct SorterByVALUE
    {
        bool operator()(const PlanAssumptionRow& x, const PlanAssumptionRow& y)
        {
            return x.VALUE < y.VALUE;
        }
    };

    struct SorterBySCOPEKEY
    {
        bool operator()(const PlanAssumptionRow& x, const PlanAssumptionRow& y)
        {
            return x.SCOPEKEY < y.SCOPEKEY;
        }
    };

    struct SorterByNOTES
    {
        bool operator()(const PlanAssumptionRow& x, const PlanAssumptionRow& y)
        {
            return x.NOTES < y.NOTES;
        }
    };

    struct SorterByASOFDATE
    {
        bool operator()(const PlanAssumptionRow& x, const PlanAssumptionRow& y)
        {
            return x.ASOFDATE < y.ASOFDATE;
        }
    };

    struct SorterByACTIVE
    {
        bool operator()(const PlanAssumptionRow& x, const PlanAssumptionRow& y)
        {
            return x.ACTIVE < y.ACTIVE;
        }
    };

    struct SorterByUNIT
    {
        bool operator()(const PlanAssumptionRow& x, const PlanAssumptionRow& y)
        {
            return x.UNIT < y.UNIT;
        }
    };

    struct SorterByGROUPID
    {
        bool operator()(const PlanAssumptionRow& x, const PlanAssumptionRow& y)
        {
            return x.GROUPID < y.GROUPID;
        }
    };
};

// Interface to database table PLANASSUMPTION_V1
struct PlanAssumptionTable : public TableBase
{
    using Row = PlanAssumptionRow;
    using Col = typename Row::Col;

    PlanAssumptionTable();
    ~PlanAssumptionTable() {}
};

inline PlanAssumptionRow::PlanAssumptionRow(wxSQLite3ResultSet& q)
{
    from_select_result(q);
}

inline void PlanAssumptionRow::to_update_stmt(wxSQLite3Statement& stmt) const
{
    to_insert_stmt(stmt, id());
}

inline PlanAssumptionRow& PlanAssumptionRow::clone_from(const PlanAssumptionRow& other)
{
    *this = other;
    id(-1);
    return *this;
}
