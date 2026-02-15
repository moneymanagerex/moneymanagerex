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

struct TransactionTable : public TableBase
{
    struct Data;

    enum COLUMN
    {
        COL_TRANSID = 0,
        COL_ACCOUNTID,
        COL_TOACCOUNTID,
        COL_PAYEEID,
        COL_TRANSCODE,
        COL_TRANSAMOUNT,
        COL_STATUS,
        COL_TRANSACTIONNUMBER,
        COL_NOTES,
        COL_CATEGID,
        COL_TRANSDATE,
        COL_LASTUPDATEDTIME,
        COL_DELETEDTIME,
        COL_FOLLOWUPID,
        COL_TOTRANSAMOUNT,
        COL_COLOR,
        COL_size
    };

    struct TRANSID : public TableOpV<int64>
    {
        static wxString name() { return "TRANSID"; }
        explicit TRANSID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit TRANSID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct ACCOUNTID : public TableOpV<int64>
    {
        static wxString name() { return "ACCOUNTID"; }
        explicit ACCOUNTID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit ACCOUNTID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct TOACCOUNTID : public TableOpV<int64>
    {
        static wxString name() { return "TOACCOUNTID"; }
        explicit TOACCOUNTID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit TOACCOUNTID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct PAYEEID : public TableOpV<int64>
    {
        static wxString name() { return "PAYEEID"; }
        explicit PAYEEID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit PAYEEID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct TRANSCODE : public TableOpV<wxString>
    {
        static wxString name() { return "TRANSCODE"; }
        explicit TRANSCODE(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit TRANSCODE(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct TRANSAMOUNT : public TableOpV<double>
    {
        static wxString name() { return "TRANSAMOUNT"; }
        explicit TRANSAMOUNT(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit TRANSAMOUNT(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    struct STATUS : public TableOpV<wxString>
    {
        static wxString name() { return "STATUS"; }
        explicit STATUS(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit STATUS(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct TRANSACTIONNUMBER : public TableOpV<wxString>
    {
        static wxString name() { return "TRANSACTIONNUMBER"; }
        explicit TRANSACTIONNUMBER(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit TRANSACTIONNUMBER(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct NOTES : public TableOpV<wxString>
    {
        static wxString name() { return "NOTES"; }
        explicit NOTES(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit NOTES(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct CATEGID : public TableOpV<int64>
    {
        static wxString name() { return "CATEGID"; }
        explicit CATEGID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit CATEGID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct TRANSDATE : public TableOpV<wxString>
    {
        static wxString name() { return "TRANSDATE"; }
        explicit TRANSDATE(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit TRANSDATE(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct LASTUPDATEDTIME : public TableOpV<wxString>
    {
        static wxString name() { return "LASTUPDATEDTIME"; }
        explicit LASTUPDATEDTIME(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit LASTUPDATEDTIME(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct DELETEDTIME : public TableOpV<wxString>
    {
        static wxString name() { return "DELETEDTIME"; }
        explicit DELETEDTIME(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit DELETEDTIME(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct FOLLOWUPID : public TableOpV<int64>
    {
        static wxString name() { return "FOLLOWUPID"; }
        explicit FOLLOWUPID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit FOLLOWUPID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct TOTRANSAMOUNT : public TableOpV<double>
    {
        static wxString name() { return "TOTRANSAMOUNT"; }
        explicit TOTRANSAMOUNT(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit TOTRANSAMOUNT(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    struct COLOR : public TableOpV<int64>
    {
        static wxString name() { return "COLOR"; }
        explicit COLOR(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit COLOR(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    typedef TRANSID PRIMARY;

    // Data is a single record in the database table
    struct Data
    {
        int64 TRANSID; // primary key
        int64 ACCOUNTID;
        int64 TOACCOUNTID;
        int64 PAYEEID;
        wxString TRANSCODE;
        double TRANSAMOUNT;
        wxString STATUS;
        wxString TRANSACTIONNUMBER;
        wxString NOTES;
        int64 CATEGID;
        wxString TRANSDATE;
        wxString LASTUPDATEDTIME;
        wxString DELETEDTIME;
        int64 FOLLOWUPID;
        double TOTRANSAMOUNT;
        int64 COLOR;

        explicit Data();
        explicit Data(wxSQLite3ResultSet& q);
        Data(const Data& other) = default;

        int64 id() const { return TRANSID; }
        void id(const int64 id) { TRANSID = id; }
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

    static bool match(const Data* data, const TRANSID& op)
    {
        return data->TRANSID == op.m_value;
    }

    static bool match(const Data* data, const ACCOUNTID& op)
    {
        return data->ACCOUNTID == op.m_value;
    }

    static bool match(const Data* data, const TOACCOUNTID& op)
    {
        return data->TOACCOUNTID == op.m_value;
    }

    static bool match(const Data* data, const PAYEEID& op)
    {
        return data->PAYEEID == op.m_value;
    }

    static bool match(const Data* data, const TRANSCODE& op)
    {
        return data->TRANSCODE.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const TRANSAMOUNT& op)
    {
        return data->TRANSAMOUNT == op.m_value;
    }

    static bool match(const Data* data, const STATUS& op)
    {
        return data->STATUS.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const TRANSACTIONNUMBER& op)
    {
        return data->TRANSACTIONNUMBER.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const NOTES& op)
    {
        return data->NOTES.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const CATEGID& op)
    {
        return data->CATEGID == op.m_value;
    }

    static bool match(const Data* data, const TRANSDATE& op)
    {
        return data->TRANSDATE.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const LASTUPDATEDTIME& op)
    {
        return data->LASTUPDATEDTIME.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const DELETEDTIME& op)
    {
        return data->DELETEDTIME.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const FOLLOWUPID& op)
    {
        return data->FOLLOWUPID == op.m_value;
    }

    static bool match(const Data* data, const TOTRANSAMOUNT& op)
    {
        return data->TOTRANSAMOUNT == op.m_value;
    }

    static bool match(const Data* data, const COLOR& op)
    {
        return data->COLOR == op.m_value;
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

    TransactionTable();
    ~TransactionTable();

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
            if (item->id() > 0 && TransactionTable::match(item, args...)) {
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

    struct SorterByACCOUNTID
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.ACCOUNTID < y.ACCOUNTID;
        }
    };

    struct SorterByCATEGID
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.CATEGID < y.CATEGID;
        }
    };

    struct SorterByCOLOR
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.COLOR < y.COLOR;
        }
    };

    struct SorterByDELETEDTIME
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.DELETEDTIME < y.DELETEDTIME;
        }
    };

    struct SorterByFOLLOWUPID
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.FOLLOWUPID < y.FOLLOWUPID;
        }
    };

    struct SorterByLASTUPDATEDTIME
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.LASTUPDATEDTIME < y.LASTUPDATEDTIME;
        }
    };

    struct SorterByNOTES
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.NOTES < y.NOTES;
        }
    };

    struct SorterByPAYEEID
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.PAYEEID < y.PAYEEID;
        }
    };

    struct SorterBySTATUS
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.STATUS < y.STATUS;
        }
    };

    struct SorterByTOACCOUNTID
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.TOACCOUNTID < y.TOACCOUNTID;
        }
    };

    struct SorterByTOTRANSAMOUNT
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.TOTRANSAMOUNT < y.TOTRANSAMOUNT;
        }
    };

    struct SorterByTRANSACTIONNUMBER
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.TRANSACTIONNUMBER < y.TRANSACTIONNUMBER;
        }
    };

    struct SorterByTRANSAMOUNT
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.TRANSAMOUNT < y.TRANSAMOUNT;
        }
    };

    struct SorterByTRANSCODE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.TRANSCODE < y.TRANSCODE;
        }
    };

    struct SorterByTRANSDATE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.TRANSDATE < y.TRANSDATE;
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
