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

struct CurrencyHistoryTable : public TableBase
{
    struct Data;

    enum COLUMN
    {
        COL_CURRHISTID = 0,
        COL_CURRENCYID,
        COL_CURRDATE,
        COL_CURRVALUE,
        COL_CURRUPDTYPE,
        COL_size
    };

    struct CURRHISTID : public TableOpV<int64>
    {
        static wxString name() { return "CURRHISTID"; }
        explicit CURRHISTID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit CURRHISTID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct CURRENCYID : public TableOpV<int64>
    {
        static wxString name() { return "CURRENCYID"; }
        explicit CURRENCYID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit CURRENCYID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct CURRDATE : public TableOpV<wxString>
    {
        static wxString name() { return "CURRDATE"; }
        explicit CURRDATE(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit CURRDATE(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct CURRVALUE : public TableOpV<double>
    {
        static wxString name() { return "CURRVALUE"; }
        explicit CURRVALUE(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit CURRVALUE(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    struct CURRUPDTYPE : public TableOpV<int64>
    {
        static wxString name() { return "CURRUPDTYPE"; }
        explicit CURRUPDTYPE(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit CURRUPDTYPE(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    typedef CURRHISTID PRIMARY;

    // Data is a single record in the database table
    struct Data
    {
        int64 CURRHISTID; // primary key
        int64 CURRENCYID;
        wxString CURRDATE;
        double CURRVALUE;
        int64 CURRUPDTYPE;

        explicit Data();
        explicit Data(wxSQLite3ResultSet& q);
        Data(const Data& other) = default;

        int64 id() const { return CURRHISTID; }
        void id(const int64 id) { CURRHISTID = id; }
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

    static bool match(const Data* data, const CURRHISTID& op)
    {
        return data->CURRHISTID == op.m_value;
    }

    static bool match(const Data* data, const CURRENCYID& op)
    {
        return data->CURRENCYID == op.m_value;
    }

    static bool match(const Data* data, const CURRDATE& op)
    {
        return data->CURRDATE.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const CURRVALUE& op)
    {
        return data->CURRVALUE == op.m_value;
    }

    static bool match(const Data* data, const CURRUPDTYPE& op)
    {
        return data->CURRUPDTYPE == op.m_value;
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

    CurrencyHistoryTable();
    ~CurrencyHistoryTable();

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
            if (item->id() > 0 && CurrencyHistoryTable::match(item, args...)) {
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

    struct SorterByCURRDATE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.CURRDATE < y.CURRDATE;
        }
    };

    struct SorterByCURRENCYID
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.CURRENCYID < y.CURRENCYID;
        }
    };

    struct SorterByCURRHISTID
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.CURRHISTID < y.CURRHISTID;
        }
    };

    struct SorterByCURRUPDTYPE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.CURRUPDTYPE < y.CURRUPDTYPE;
        }
    };

    struct SorterByCURRVALUE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.CURRVALUE < y.CURRVALUE;
        }
    };
};
