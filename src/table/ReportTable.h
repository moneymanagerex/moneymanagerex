// -*- C++ -*-
//=============================================================================
/**
 *      Copyright: (c) 2013-2026 Guan Lisheng (guanlisheng@gmail.com)
 *      Copyright: (c) 2017-2018 Stefano Giorgio (stef145g)
 *      Copyright: (c) 2022      Mark Whalley (mark@ipx.co.uk)
 *      Copyright: (c) 2026      George Ef (george.a.ef@gmail.com)
 *
 *      @file
 *
 *      @author [sqlite2cpp.py]
 *
 *      @brief
 *
 *      Revision History:
 *          AUTO GENERATED at 2026-02-15 02:44:45.846505.
 *          DO NOT EDIT!
 */
//=============================================================================

#pragma once

#include "_TableBase.h"

struct ReportTable : public TableBase
{
    struct Data;

    enum COLUMN
    {
        COL_REPORTID = 0,
        COL_REPORTNAME,
        COL_GROUPNAME,
        COL_ACTIVE,
        COL_SQLCONTENT,
        COL_LUACONTENT,
        COL_TEMPLATECONTENT,
        COL_DESCRIPTION,
        COL_size
    };

    struct REPORTID : public TableOpV<int64>
    {
        static wxString name() { return "REPORTID"; }
        explicit REPORTID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit REPORTID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct REPORTNAME : public TableOpV<wxString>
    {
        static wxString name() { return "REPORTNAME"; }
        explicit REPORTNAME(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit REPORTNAME(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct GROUPNAME : public TableOpV<wxString>
    {
        static wxString name() { return "GROUPNAME"; }
        explicit GROUPNAME(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit GROUPNAME(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct ACTIVE : public TableOpV<int64>
    {
        static wxString name() { return "ACTIVE"; }
        explicit ACTIVE(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit ACTIVE(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct SQLCONTENT : public TableOpV<wxString>
    {
        static wxString name() { return "SQLCONTENT"; }
        explicit SQLCONTENT(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit SQLCONTENT(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct LUACONTENT : public TableOpV<wxString>
    {
        static wxString name() { return "LUACONTENT"; }
        explicit LUACONTENT(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit LUACONTENT(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct TEMPLATECONTENT : public TableOpV<wxString>
    {
        static wxString name() { return "TEMPLATECONTENT"; }
        explicit TEMPLATECONTENT(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit TEMPLATECONTENT(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct DESCRIPTION : public TableOpV<wxString>
    {
        static wxString name() { return "DESCRIPTION"; }
        explicit DESCRIPTION(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit DESCRIPTION(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    typedef REPORTID PRIMARY;

    // Data is a single record in the database table
    struct Data
    {
        int64 REPORTID; // primary key
        wxString REPORTNAME;
        wxString GROUPNAME;
        int64 ACTIVE;
        wxString SQLCONTENT;
        wxString LUACONTENT;
        wxString TEMPLATECONTENT;
        wxString DESCRIPTION;

        explicit Data();
        explicit Data(wxSQLite3ResultSet& q);
        Data(const Data& other) = default;

        int64 id() const { return REPORTID; }
        void id(const int64 id) { REPORTID = id; }
        bool equals(const Data* r) const;
        wxString to_json() const;
        void as_json(PrettyWriter<StringBuffer>& json_writer) const;
        row_t to_row_t() const;
        void to_template(html_template& t) const;
        void destroy();

        Data& operator=(const Data& other);

        auto operator < (const Data& other) const
        {
            return id() < other.id();
        }

        auto operator < (const Data* other) const
        {
            return id() < other->id();
        }
    };

    // A container to hold list of Data records for the table
    struct Data_Set : public std::vector<Data>
    {
        wxString to_json() const;
    };

    static wxString column_to_name(const COLUMN col);
    static COLUMN name_to_column(const wxString& name);

    template<typename C>
    static bool match(const Data* r, const C&)
    {
        return false;
    }

    static bool match(const Data* data, const REPORTID& op)
    {
        return data->REPORTID == op.m_value;
    }

    static bool match(const Data* data, const REPORTNAME& op)
    {
        return data->REPORTNAME.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const GROUPNAME& op)
    {
        return data->GROUPNAME.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const ACTIVE& op)
    {
        return data->ACTIVE == op.m_value;
    }

    static bool match(const Data* data, const SQLCONTENT& op)
    {
        return data->SQLCONTENT.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const LUACONTENT& op)
    {
        return data->LUACONTENT.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const TEMPLATECONTENT& op)
    {
        return data->TEMPLATECONTENT.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const DESCRIPTION& op)
    {
        return data->DESCRIPTION.CmpNoCase(op.m_value) == 0;
    }

    template<typename Arg1, typename... Args>
    static bool match(const Data* data, const Arg1& arg1, const Args&... args)
    {
        return (match(data, arg1) && ... && match(data, args));
    }

    // TODO: in the above match() functions, check if op.m_operator == OP_EQ

    // A container to hold a list of Data record pointers for the table in memory
    typedef std::vector<Data*> Cache;
    typedef std::map<int64, Data*> CacheIndex;
    Cache m_cache;
    CacheIndex m_cache_index;
    Data* fake_; // in case the entity not found

    ReportTable();
    ~ReportTable();

    size_t num_columns() const { return COL_size; }
    void destroy_cache();
    bool ensure_table();
    bool ensure_index();
    void ensure_data();
    Data* create();
    Data* clone(const Data* e);
    bool save(Data* entity);
    bool remove(const int64 id);
    bool remove(Data* entity);

    template<typename... Args>
    Data* search_cache(const Args& ... args)
    {
        for (auto& [_, item] : m_cache_index) {
            if (item->id() > 0 && ReportTable::match(item, args...)) {
                ++m_hit;
                return item;
            }
        }
        ++m_miss;
        return 0;
    }

    Data* cache_id(const int64 id);
    Data* get_id(const int64 id);
    const Data_Set get_all(const COLUMN col = COLUMN(0), const bool asc = true);

    struct SorterByACTIVE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.ACTIVE < y.ACTIVE;
        }
    };

    struct SorterByDESCRIPTION
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.DESCRIPTION < y.DESCRIPTION;
        }
    };

    struct SorterByGROUPNAME
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.GROUPNAME < y.GROUPNAME;
        }
    };

    struct SorterByLUACONTENT
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.LUACONTENT < y.LUACONTENT;
        }
    };

    struct SorterByREPORTID
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.REPORTID < y.REPORTID;
        }
    };

    struct SorterByREPORTNAME
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.REPORTNAME < y.REPORTNAME;
        }
    };

    struct SorterBySQLCONTENT
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.SQLCONTENT < y.SQLCONTENT;
        }
    };

    struct SorterByTEMPLATECONTENT
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.TEMPLATECONTENT < y.TEMPLATECONTENT;
        }
    };
};
