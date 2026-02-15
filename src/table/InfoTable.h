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

struct InfoTable : public TableBase
{
    struct Data;

    enum COLUMN
    {
        COL_INFOID = 0,
        COL_INFONAME,
        COL_INFOVALUE,
        COL_size
    };

    struct INFOID : public TableOpV<int64>
    {
        static wxString name() { return "INFOID"; }
        explicit INFOID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit INFOID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct INFONAME : public TableOpV<wxString>
    {
        static wxString name() { return "INFONAME"; }
        explicit INFONAME(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit INFONAME(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct INFOVALUE : public TableOpV<wxString>
    {
        static wxString name() { return "INFOVALUE"; }
        explicit INFOVALUE(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit INFOVALUE(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    typedef INFOID PRIMARY;

    // Data is a single record in the database table
    struct Data
    {
        int64 INFOID; // primary key
        wxString INFONAME;
        wxString INFOVALUE;

        explicit Data();
        explicit Data(wxSQLite3ResultSet& q);
        Data(const Data& other) = default;

        int64 id() const { return INFOID; }
        void id(const int64 id) { INFOID = id; }
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

    static bool match(const Data* data, const INFOID& op)
    {
        return data->INFOID == op.m_value;
    }

    static bool match(const Data* data, const INFONAME& op)
    {
        return data->INFONAME.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const INFOVALUE& op)
    {
        return data->INFOVALUE.CmpNoCase(op.m_value) == 0;
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

    InfoTable();
    ~InfoTable();

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
            if (item->id() > 0 && InfoTable::match(item, args...)) {
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

    struct SorterByINFOID
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.INFOID < y.INFOID;
        }
    };

    struct SorterByINFONAME
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.INFONAME < y.INFONAME;
        }
    };

    struct SorterByINFOVALUE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.INFOVALUE < y.INFOVALUE;
        }
    };
};
