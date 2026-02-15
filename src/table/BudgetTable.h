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

struct BudgetTable : public TableBase
{
    struct Data;

    enum COLUMN
    {
        COL_BUDGETENTRYID = 0,
        COL_BUDGETYEARID,
        COL_CATEGID,
        COL_PERIOD,
        COL_AMOUNT,
        COL_NOTES,
        COL_ACTIVE,
        COL_size
    };

    struct BUDGETENTRYID : public TableOpV<int64>
    {
        static wxString name() { return "BUDGETENTRYID"; }
        explicit BUDGETENTRYID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit BUDGETENTRYID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct BUDGETYEARID : public TableOpV<int64>
    {
        static wxString name() { return "BUDGETYEARID"; }
        explicit BUDGETYEARID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit BUDGETYEARID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct CATEGID : public TableOpV<int64>
    {
        static wxString name() { return "CATEGID"; }
        explicit CATEGID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit CATEGID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct PERIOD : public TableOpV<wxString>
    {
        static wxString name() { return "PERIOD"; }
        explicit PERIOD(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit PERIOD(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct AMOUNT : public TableOpV<double>
    {
        static wxString name() { return "AMOUNT"; }
        explicit AMOUNT(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit AMOUNT(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    struct NOTES : public TableOpV<wxString>
    {
        static wxString name() { return "NOTES"; }
        explicit NOTES(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit NOTES(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct ACTIVE : public TableOpV<int64>
    {
        static wxString name() { return "ACTIVE"; }
        explicit ACTIVE(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit ACTIVE(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    typedef BUDGETENTRYID PRIMARY;

    // Data is a single record in the database table
    struct Data
    {
        int64 BUDGETENTRYID; // primary key
        int64 BUDGETYEARID;
        int64 CATEGID;
        wxString PERIOD;
        double AMOUNT;
        wxString NOTES;
        int64 ACTIVE;

        explicit Data();
        explicit Data(wxSQLite3ResultSet& q);
        Data(const Data& other) = default;

        int64 id() const { return BUDGETENTRYID; }
        void id(const int64 id) { BUDGETENTRYID = id; }
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

    static bool match(const Data* data, const BUDGETENTRYID& op)
    {
        return data->BUDGETENTRYID == op.m_value;
    }

    static bool match(const Data* data, const BUDGETYEARID& op)
    {
        return data->BUDGETYEARID == op.m_value;
    }

    static bool match(const Data* data, const CATEGID& op)
    {
        return data->CATEGID == op.m_value;
    }

    static bool match(const Data* data, const PERIOD& op)
    {
        return data->PERIOD.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const AMOUNT& op)
    {
        return data->AMOUNT == op.m_value;
    }

    static bool match(const Data* data, const NOTES& op)
    {
        return data->NOTES.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const ACTIVE& op)
    {
        return data->ACTIVE == op.m_value;
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

    BudgetTable();
    ~BudgetTable();

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
            if (item->id() > 0 && BudgetTable::match(item, args...)) {
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

    struct SorterByAMOUNT
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.AMOUNT < y.AMOUNT;
        }
    };

    struct SorterByBUDGETENTRYID
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.BUDGETENTRYID < y.BUDGETENTRYID;
        }
    };

    struct SorterByBUDGETYEARID
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.BUDGETYEARID < y.BUDGETYEARID;
        }
    };

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

    struct SorterByPERIOD
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.PERIOD < y.PERIOD;
        }
    };
};
