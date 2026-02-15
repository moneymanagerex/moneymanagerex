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

struct TransactionShareTable : public TableBase
{
    struct Data;

    enum COLUMN
    {
        COL_SHAREINFOID = 0,
        COL_CHECKINGACCOUNTID,
        COL_SHARENUMBER,
        COL_SHAREPRICE,
        COL_SHARECOMMISSION,
        COL_SHARELOT,
        COL_size
    };

    struct SHAREINFOID : public TableOpV<int64>
    {
        static wxString name() { return "SHAREINFOID"; }
        explicit SHAREINFOID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit SHAREINFOID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct CHECKINGACCOUNTID : public TableOpV<int64>
    {
        static wxString name() { return "CHECKINGACCOUNTID"; }
        explicit CHECKINGACCOUNTID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit CHECKINGACCOUNTID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct SHARENUMBER : public TableOpV<double>
    {
        static wxString name() { return "SHARENUMBER"; }
        explicit SHARENUMBER(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit SHARENUMBER(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    struct SHAREPRICE : public TableOpV<double>
    {
        static wxString name() { return "SHAREPRICE"; }
        explicit SHAREPRICE(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit SHAREPRICE(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    struct SHARECOMMISSION : public TableOpV<double>
    {
        static wxString name() { return "SHARECOMMISSION"; }
        explicit SHARECOMMISSION(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit SHARECOMMISSION(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    struct SHARELOT : public TableOpV<wxString>
    {
        static wxString name() { return "SHARELOT"; }
        explicit SHARELOT(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit SHARELOT(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    typedef SHAREINFOID PRIMARY;

    // Data is a single record in the database table
    struct Data
    {
        int64 SHAREINFOID; // primary key
        int64 CHECKINGACCOUNTID;
        double SHARENUMBER;
        double SHAREPRICE;
        double SHARECOMMISSION;
        wxString SHARELOT;

        explicit Data();
        explicit Data(wxSQLite3ResultSet& q);
        Data(const Data& other) = default;

        int64 id() const { return SHAREINFOID; }
        void id(const int64 id) { SHAREINFOID = id; }
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

    static bool match(const Data* data, const SHAREINFOID& op)
    {
        return data->SHAREINFOID == op.m_value;
    }

    static bool match(const Data* data, const CHECKINGACCOUNTID& op)
    {
        return data->CHECKINGACCOUNTID == op.m_value;
    }

    static bool match(const Data* data, const SHARENUMBER& op)
    {
        return data->SHARENUMBER == op.m_value;
    }

    static bool match(const Data* data, const SHAREPRICE& op)
    {
        return data->SHAREPRICE == op.m_value;
    }

    static bool match(const Data* data, const SHARECOMMISSION& op)
    {
        return data->SHARECOMMISSION == op.m_value;
    }

    static bool match(const Data* data, const SHARELOT& op)
    {
        return data->SHARELOT.CmpNoCase(op.m_value) == 0;
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

    TransactionShareTable();
    ~TransactionShareTable();

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
            if (item->id() > 0 && TransactionShareTable::match(item, args...)) {
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

    struct SorterByCHECKINGACCOUNTID
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.CHECKINGACCOUNTID < y.CHECKINGACCOUNTID;
        }
    };

    struct SorterBySHARECOMMISSION
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.SHARECOMMISSION < y.SHARECOMMISSION;
        }
    };

    struct SorterBySHAREINFOID
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.SHAREINFOID < y.SHAREINFOID;
        }
    };

    struct SorterBySHARELOT
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.SHARELOT < y.SHARELOT;
        }
    };

    struct SorterBySHARENUMBER
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.SHARENUMBER < y.SHARENUMBER;
        }
    };

    struct SorterBySHAREPRICE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.SHAREPRICE < y.SHAREPRICE;
        }
    };
};
