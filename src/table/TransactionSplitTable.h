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

struct TransactionSplitTable : public TableBase
{
    struct Data;

    enum COLUMN
    {
        COL_SPLITTRANSID = 0,
        COL_TRANSID,
        COL_CATEGID,
        COL_SPLITTRANSAMOUNT,
        COL_NOTES,
        COL_size
    };

    struct SPLITTRANSID : public TableOpV<int64>
    {
        static wxString name() { return "SPLITTRANSID"; }
        explicit SPLITTRANSID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit SPLITTRANSID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct TRANSID : public TableOpV<int64>
    {
        static wxString name() { return "TRANSID"; }
        explicit TRANSID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit TRANSID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct CATEGID : public TableOpV<int64>
    {
        static wxString name() { return "CATEGID"; }
        explicit CATEGID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit CATEGID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct SPLITTRANSAMOUNT : public TableOpV<double>
    {
        static wxString name() { return "SPLITTRANSAMOUNT"; }
        explicit SPLITTRANSAMOUNT(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit SPLITTRANSAMOUNT(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    struct NOTES : public TableOpV<wxString>
    {
        static wxString name() { return "NOTES"; }
        explicit NOTES(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit NOTES(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    typedef SPLITTRANSID PRIMARY;

    // Data is a single record in the database table
    struct Data
    {
        int64 SPLITTRANSID; // primary key
        int64 TRANSID;
        int64 CATEGID;
        double SPLITTRANSAMOUNT;
        wxString NOTES;

        explicit Data();
        explicit Data(wxSQLite3ResultSet& q);
        Data(const Data& other) = default;

        int64 id() const { return SPLITTRANSID; }
        void id(const int64 id) { SPLITTRANSID = id; }
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

    static bool match(const Data* data, const SPLITTRANSID& op)
    {
        return data->SPLITTRANSID == op.m_value;
    }

    static bool match(const Data* data, const TRANSID& op)
    {
        return data->TRANSID == op.m_value;
    }

    static bool match(const Data* data, const CATEGID& op)
    {
        return data->CATEGID == op.m_value;
    }

    static bool match(const Data* data, const SPLITTRANSAMOUNT& op)
    {
        return data->SPLITTRANSAMOUNT == op.m_value;
    }

    static bool match(const Data* data, const NOTES& op)
    {
        return data->NOTES.CmpNoCase(op.m_value) == 0;
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

    TransactionSplitTable();
    ~TransactionSplitTable();

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
            if (item->id() > 0 && TransactionSplitTable::match(item, args...)) {
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

    struct SorterByCATEGID
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.CATEGID < y.CATEGID;
        }
    };

    struct SorterByNOTES
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.NOTES < y.NOTES;
        }
    };

    struct SorterBySPLITTRANSAMOUNT
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.SPLITTRANSAMOUNT < y.SPLITTRANSAMOUNT;
        }
    };

    struct SorterBySPLITTRANSID
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.SPLITTRANSID < y.SPLITTRANSID;
        }
    };

    struct SorterByTRANSID
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.TRANSID < y.TRANSID;
        }
    };
};
