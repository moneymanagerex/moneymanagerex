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

struct AssetTable : public TableBase
{
    struct Data;

    enum COLUMN
    {
        COL_ASSETID = 0,
        COL_STARTDATE,
        COL_ASSETNAME,
        COL_ASSETSTATUS,
        COL_CURRENCYID,
        COL_VALUECHANGEMODE,
        COL_VALUE,
        COL_VALUECHANGE,
        COL_NOTES,
        COL_VALUECHANGERATE,
        COL_ASSETTYPE,
        COL_size
    };

    struct ASSETID : public TableOpV<int64>
    {
        static wxString name() { return "ASSETID"; }
        explicit ASSETID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit ASSETID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct STARTDATE : public TableOpV<wxString>
    {
        static wxString name() { return "STARTDATE"; }
        explicit STARTDATE(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit STARTDATE(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct ASSETNAME : public TableOpV<wxString>
    {
        static wxString name() { return "ASSETNAME"; }
        explicit ASSETNAME(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit ASSETNAME(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct ASSETSTATUS : public TableOpV<wxString>
    {
        static wxString name() { return "ASSETSTATUS"; }
        explicit ASSETSTATUS(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit ASSETSTATUS(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct CURRENCYID : public TableOpV<int64>
    {
        static wxString name() { return "CURRENCYID"; }
        explicit CURRENCYID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit CURRENCYID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct VALUECHANGEMODE : public TableOpV<wxString>
    {
        static wxString name() { return "VALUECHANGEMODE"; }
        explicit VALUECHANGEMODE(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit VALUECHANGEMODE(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct VALUE : public TableOpV<double>
    {
        static wxString name() { return "VALUE"; }
        explicit VALUE(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit VALUE(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    struct VALUECHANGE : public TableOpV<wxString>
    {
        static wxString name() { return "VALUECHANGE"; }
        explicit VALUECHANGE(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit VALUECHANGE(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct NOTES : public TableOpV<wxString>
    {
        static wxString name() { return "NOTES"; }
        explicit NOTES(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit NOTES(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct VALUECHANGERATE : public TableOpV<double>
    {
        static wxString name() { return "VALUECHANGERATE"; }
        explicit VALUECHANGERATE(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit VALUECHANGERATE(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    struct ASSETTYPE : public TableOpV<wxString>
    {
        static wxString name() { return "ASSETTYPE"; }
        explicit ASSETTYPE(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit ASSETTYPE(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    typedef ASSETID PRIMARY;

    // Data is a single record in the database table
    struct Data
    {
        int64 ASSETID; // primary key
        wxString STARTDATE;
        wxString ASSETNAME;
        wxString ASSETSTATUS;
        int64 CURRENCYID;
        wxString VALUECHANGEMODE;
        double VALUE;
        wxString VALUECHANGE;
        wxString NOTES;
        double VALUECHANGERATE;
        wxString ASSETTYPE;

        explicit Data();
        explicit Data(wxSQLite3ResultSet& q);
        Data(const Data& other) = default;

        int64 id() const { return ASSETID; }
        void id(const int64 id) { ASSETID = id; }
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

    static bool match(const Data* data, const ASSETID& op)
    {
        return data->ASSETID == op.m_value;
    }

    static bool match(const Data* data, const STARTDATE& op)
    {
        return data->STARTDATE.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const ASSETNAME& op)
    {
        return data->ASSETNAME.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const ASSETSTATUS& op)
    {
        return data->ASSETSTATUS.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const CURRENCYID& op)
    {
        return data->CURRENCYID == op.m_value;
    }

    static bool match(const Data* data, const VALUECHANGEMODE& op)
    {
        return data->VALUECHANGEMODE.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const VALUE& op)
    {
        return data->VALUE == op.m_value;
    }

    static bool match(const Data* data, const VALUECHANGE& op)
    {
        return data->VALUECHANGE.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const NOTES& op)
    {
        return data->NOTES.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const VALUECHANGERATE& op)
    {
        return data->VALUECHANGERATE == op.m_value;
    }

    static bool match(const Data* data, const ASSETTYPE& op)
    {
        return data->ASSETTYPE.CmpNoCase(op.m_value) == 0;
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

    AssetTable();
    ~AssetTable();

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
            if (item->id() > 0 && AssetTable::match(item, args...)) {
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

    struct SorterByASSETID
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.ASSETID < y.ASSETID;
        }
    };

    struct SorterByASSETNAME
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.ASSETNAME < y.ASSETNAME;
        }
    };

    struct SorterByASSETSTATUS
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.ASSETSTATUS < y.ASSETSTATUS;
        }
    };

    struct SorterByASSETTYPE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.ASSETTYPE < y.ASSETTYPE;
        }
    };

    struct SorterByCURRENCYID
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.CURRENCYID < y.CURRENCYID;
        }
    };

    struct SorterByNOTES
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.NOTES < y.NOTES;
        }
    };

    struct SorterBySTARTDATE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.STARTDATE < y.STARTDATE;
        }
    };

    struct SorterByVALUE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.VALUE < y.VALUE;
        }
    };

    struct SorterByVALUECHANGE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.VALUECHANGE < y.VALUECHANGE;
        }
    };

    struct SorterByVALUECHANGEMODE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.VALUECHANGEMODE < y.VALUECHANGEMODE;
        }
    };

    struct SorterByVALUECHANGERATE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.VALUECHANGERATE < y.VALUECHANGERATE;
        }
    };
};
