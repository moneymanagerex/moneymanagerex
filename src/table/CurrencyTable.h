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

struct CurrencyTable : public TableBase
{
    struct Data;

    enum COLUMN
    {
        COL_CURRENCYID = 0,
        COL_CURRENCYNAME,
        COL_PFX_SYMBOL,
        COL_SFX_SYMBOL,
        COL_DECIMAL_POINT,
        COL_GROUP_SEPARATOR,
        COL_UNIT_NAME,
        COL_CENT_NAME,
        COL_SCALE,
        COL_BASECONVRATE,
        COL_CURRENCY_SYMBOL,
        COL_CURRENCY_TYPE,
        COL_size
    };

    struct CURRENCYID : public TableOpV<int64>
    {
        static wxString name() { return "CURRENCYID"; }
        explicit CURRENCYID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit CURRENCYID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct CURRENCYNAME : public TableOpV<wxString>
    {
        static wxString name() { return "CURRENCYNAME"; }
        explicit CURRENCYNAME(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit CURRENCYNAME(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct PFX_SYMBOL : public TableOpV<wxString>
    {
        static wxString name() { return "PFX_SYMBOL"; }
        explicit PFX_SYMBOL(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit PFX_SYMBOL(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct SFX_SYMBOL : public TableOpV<wxString>
    {
        static wxString name() { return "SFX_SYMBOL"; }
        explicit SFX_SYMBOL(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit SFX_SYMBOL(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct DECIMAL_POINT : public TableOpV<wxString>
    {
        static wxString name() { return "DECIMAL_POINT"; }
        explicit DECIMAL_POINT(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit DECIMAL_POINT(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct GROUP_SEPARATOR : public TableOpV<wxString>
    {
        static wxString name() { return "GROUP_SEPARATOR"; }
        explicit GROUP_SEPARATOR(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit GROUP_SEPARATOR(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct UNIT_NAME : public TableOpV<wxString>
    {
        static wxString name() { return "UNIT_NAME"; }
        explicit UNIT_NAME(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit UNIT_NAME(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct CENT_NAME : public TableOpV<wxString>
    {
        static wxString name() { return "CENT_NAME"; }
        explicit CENT_NAME(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit CENT_NAME(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct SCALE : public TableOpV<int64>
    {
        static wxString name() { return "SCALE"; }
        explicit SCALE(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit SCALE(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct BASECONVRATE : public TableOpV<double>
    {
        static wxString name() { return "BASECONVRATE"; }
        explicit BASECONVRATE(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit BASECONVRATE(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    struct CURRENCY_SYMBOL : public TableOpV<wxString>
    {
        static wxString name() { return "CURRENCY_SYMBOL"; }
        explicit CURRENCY_SYMBOL(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit CURRENCY_SYMBOL(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct CURRENCY_TYPE : public TableOpV<wxString>
    {
        static wxString name() { return "CURRENCY_TYPE"; }
        explicit CURRENCY_TYPE(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit CURRENCY_TYPE(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    typedef CURRENCYID PRIMARY;

    // Data is a single record in the database table
    struct Data
    {
        int64 CURRENCYID; // primary key
        wxString CURRENCYNAME;
        wxString PFX_SYMBOL;
        wxString SFX_SYMBOL;
        wxString DECIMAL_POINT;
        wxString GROUP_SEPARATOR;
        wxString UNIT_NAME;
        wxString CENT_NAME;
        int64 SCALE;
        double BASECONVRATE;
        wxString CURRENCY_SYMBOL;
        wxString CURRENCY_TYPE;

        explicit Data();
        explicit Data(wxSQLite3ResultSet& q);
        Data(const Data& other) = default;

        int64 id() const { return CURRENCYID; }
        void id(const int64 id) { CURRENCYID = id; }
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

    static bool match(const Data* data, const CURRENCYID& op)
    {
        return data->CURRENCYID == op.m_value;
    }

    static bool match(const Data* data, const CURRENCYNAME& op)
    {
        return data->CURRENCYNAME.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const PFX_SYMBOL& op)
    {
        return data->PFX_SYMBOL.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const SFX_SYMBOL& op)
    {
        return data->SFX_SYMBOL.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const DECIMAL_POINT& op)
    {
        return data->DECIMAL_POINT.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const GROUP_SEPARATOR& op)
    {
        return data->GROUP_SEPARATOR.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const UNIT_NAME& op)
    {
        return data->UNIT_NAME.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const CENT_NAME& op)
    {
        return data->CENT_NAME.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const SCALE& op)
    {
        return data->SCALE == op.m_value;
    }

    static bool match(const Data* data, const BASECONVRATE& op)
    {
        return data->BASECONVRATE == op.m_value;
    }

    static bool match(const Data* data, const CURRENCY_SYMBOL& op)
    {
        return data->CURRENCY_SYMBOL.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const CURRENCY_TYPE& op)
    {
        return data->CURRENCY_TYPE.CmpNoCase(op.m_value) == 0;
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

    CurrencyTable();
    ~CurrencyTable();

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
            if (item->id() > 0 && CurrencyTable::match(item, args...)) {
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

    struct SorterByBASECONVRATE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.BASECONVRATE < y.BASECONVRATE;
        }
    };

    struct SorterByCENT_NAME
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.CENT_NAME < y.CENT_NAME;
        }
    };

    struct SorterByCURRENCYID
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.CURRENCYID < y.CURRENCYID;
        }
    };

    struct SorterByCURRENCYNAME
    {
        bool operator()(const Data& x, const Data& y)
        {
            return wxGetTranslation(x.CURRENCYNAME) < wxGetTranslation(y.CURRENCYNAME);
        }
    };

    struct SorterByCURRENCY_SYMBOL
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.CURRENCY_SYMBOL < y.CURRENCY_SYMBOL;
        }
    };

    struct SorterByCURRENCY_TYPE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.CURRENCY_TYPE < y.CURRENCY_TYPE;
        }
    };

    struct SorterByDECIMAL_POINT
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.DECIMAL_POINT < y.DECIMAL_POINT;
        }
    };

    struct SorterByGROUP_SEPARATOR
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.GROUP_SEPARATOR < y.GROUP_SEPARATOR;
        }
    };

    struct SorterByPFX_SYMBOL
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.PFX_SYMBOL < y.PFX_SYMBOL;
        }
    };

    struct SorterBySCALE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.SCALE < y.SCALE;
        }
    };

    struct SorterBySFX_SYMBOL
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.SFX_SYMBOL < y.SFX_SYMBOL;
        }
    };

    struct SorterByUNIT_NAME
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.UNIT_NAME < y.UNIT_NAME;
        }
    };
};
