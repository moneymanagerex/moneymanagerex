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

struct StockTable : public TableBase
{
    struct Data;

    enum COLUMN
    {
        COL_STOCKID = 0,
        COL_HELDAT,
        COL_PURCHASEDATE,
        COL_STOCKNAME,
        COL_SYMBOL,
        COL_NUMSHARES,
        COL_PURCHASEPRICE,
        COL_NOTES,
        COL_CURRENTPRICE,
        COL_VALUE,
        COL_COMMISSION,
        COL_size
    };

    struct STOCKID : public TableOpV<int64>
    {
        static wxString name() { return "STOCKID"; }
        explicit STOCKID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit STOCKID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct HELDAT : public TableOpV<int64>
    {
        static wxString name() { return "HELDAT"; }
        explicit HELDAT(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit HELDAT(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct PURCHASEDATE : public TableOpV<wxString>
    {
        static wxString name() { return "PURCHASEDATE"; }
        explicit PURCHASEDATE(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit PURCHASEDATE(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct STOCKNAME : public TableOpV<wxString>
    {
        static wxString name() { return "STOCKNAME"; }
        explicit STOCKNAME(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit STOCKNAME(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct SYMBOL : public TableOpV<wxString>
    {
        static wxString name() { return "SYMBOL"; }
        explicit SYMBOL(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit SYMBOL(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct NUMSHARES : public TableOpV<double>
    {
        static wxString name() { return "NUMSHARES"; }
        explicit NUMSHARES(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit NUMSHARES(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    struct PURCHASEPRICE : public TableOpV<double>
    {
        static wxString name() { return "PURCHASEPRICE"; }
        explicit PURCHASEPRICE(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit PURCHASEPRICE(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    struct NOTES : public TableOpV<wxString>
    {
        static wxString name() { return "NOTES"; }
        explicit NOTES(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit NOTES(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct CURRENTPRICE : public TableOpV<double>
    {
        static wxString name() { return "CURRENTPRICE"; }
        explicit CURRENTPRICE(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit CURRENTPRICE(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    struct VALUE : public TableOpV<double>
    {
        static wxString name() { return "VALUE"; }
        explicit VALUE(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit VALUE(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    struct COMMISSION : public TableOpV<double>
    {
        static wxString name() { return "COMMISSION"; }
        explicit COMMISSION(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit COMMISSION(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    typedef STOCKID PRIMARY;

    // Data is a single record in the database table
    struct Data
    {
        int64 STOCKID; // primary key
        int64 HELDAT;
        wxString PURCHASEDATE;
        wxString STOCKNAME;
        wxString SYMBOL;
        double NUMSHARES;
        double PURCHASEPRICE;
        wxString NOTES;
        double CURRENTPRICE;
        double VALUE;
        double COMMISSION;

        explicit Data();
        explicit Data(wxSQLite3ResultSet& q);
        Data(const Data& other) = default;

        int64 id() const { return STOCKID; }
        void id(const int64 id) { STOCKID = id; }
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

    static bool match(const Data* data, const STOCKID& op)
    {
        return data->STOCKID == op.m_value;
    }

    static bool match(const Data* data, const HELDAT& op)
    {
        return data->HELDAT == op.m_value;
    }

    static bool match(const Data* data, const PURCHASEDATE& op)
    {
        return data->PURCHASEDATE.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const STOCKNAME& op)
    {
        return data->STOCKNAME.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const SYMBOL& op)
    {
        return data->SYMBOL.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const NUMSHARES& op)
    {
        return data->NUMSHARES == op.m_value;
    }

    static bool match(const Data* data, const PURCHASEPRICE& op)
    {
        return data->PURCHASEPRICE == op.m_value;
    }

    static bool match(const Data* data, const NOTES& op)
    {
        return data->NOTES.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const CURRENTPRICE& op)
    {
        return data->CURRENTPRICE == op.m_value;
    }

    static bool match(const Data* data, const VALUE& op)
    {
        return data->VALUE == op.m_value;
    }

    static bool match(const Data* data, const COMMISSION& op)
    {
        return data->COMMISSION == op.m_value;
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

    StockTable();
    ~StockTable();

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
            if (item->id() > 0 && StockTable::match(item, args...)) {
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

    struct SorterByCOMMISSION
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.COMMISSION < y.COMMISSION;
        }
    };

    struct SorterByCURRENTPRICE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.CURRENTPRICE < y.CURRENTPRICE;
        }
    };

    struct SorterByHELDAT
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.HELDAT < y.HELDAT;
        }
    };

    struct SorterByNOTES
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.NOTES < y.NOTES;
        }
    };

    struct SorterByNUMSHARES
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.NUMSHARES < y.NUMSHARES;
        }
    };

    struct SorterByPURCHASEDATE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.PURCHASEDATE < y.PURCHASEDATE;
        }
    };

    struct SorterByPURCHASEPRICE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.PURCHASEPRICE < y.PURCHASEPRICE;
        }
    };

    struct SorterBySTOCKID
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.STOCKID < y.STOCKID;
        }
    };

    struct SorterBySTOCKNAME
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.STOCKNAME < y.STOCKNAME;
        }
    };

    struct SorterBySYMBOL
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.SYMBOL < y.SYMBOL;
        }
    };

    struct SorterByVALUE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.VALUE < y.VALUE;
        }
    };
};
