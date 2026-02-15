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

struct AccountTable : public TableBase
{
    struct Data;

    enum COLUMN
    {
        COL_ACCOUNTID = 0,
        COL_ACCOUNTNAME,
        COL_ACCOUNTTYPE,
        COL_ACCOUNTNUM,
        COL_STATUS,
        COL_NOTES,
        COL_HELDAT,
        COL_WEBSITE,
        COL_CONTACTINFO,
        COL_ACCESSINFO,
        COL_INITIALBAL,
        COL_INITIALDATE,
        COL_FAVORITEACCT,
        COL_CURRENCYID,
        COL_STATEMENTLOCKED,
        COL_STATEMENTDATE,
        COL_MINIMUMBALANCE,
        COL_CREDITLIMIT,
        COL_INTERESTRATE,
        COL_PAYMENTDUEDATE,
        COL_MINIMUMPAYMENT,
        COL_size
    };

    struct ACCOUNTID : public TableOpV<int64>
    {
        static wxString name() { return "ACCOUNTID"; }
        explicit ACCOUNTID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit ACCOUNTID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct ACCOUNTNAME : public TableOpV<wxString>
    {
        static wxString name() { return "ACCOUNTNAME"; }
        explicit ACCOUNTNAME(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit ACCOUNTNAME(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct ACCOUNTTYPE : public TableOpV<wxString>
    {
        static wxString name() { return "ACCOUNTTYPE"; }
        explicit ACCOUNTTYPE(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit ACCOUNTTYPE(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct ACCOUNTNUM : public TableOpV<wxString>
    {
        static wxString name() { return "ACCOUNTNUM"; }
        explicit ACCOUNTNUM(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit ACCOUNTNUM(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct STATUS : public TableOpV<wxString>
    {
        static wxString name() { return "STATUS"; }
        explicit STATUS(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit STATUS(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct NOTES : public TableOpV<wxString>
    {
        static wxString name() { return "NOTES"; }
        explicit NOTES(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit NOTES(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct HELDAT : public TableOpV<wxString>
    {
        static wxString name() { return "HELDAT"; }
        explicit HELDAT(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit HELDAT(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct WEBSITE : public TableOpV<wxString>
    {
        static wxString name() { return "WEBSITE"; }
        explicit WEBSITE(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit WEBSITE(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct CONTACTINFO : public TableOpV<wxString>
    {
        static wxString name() { return "CONTACTINFO"; }
        explicit CONTACTINFO(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit CONTACTINFO(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct ACCESSINFO : public TableOpV<wxString>
    {
        static wxString name() { return "ACCESSINFO"; }
        explicit ACCESSINFO(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit ACCESSINFO(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct INITIALBAL : public TableOpV<double>
    {
        static wxString name() { return "INITIALBAL"; }
        explicit INITIALBAL(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit INITIALBAL(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    struct INITIALDATE : public TableOpV<wxString>
    {
        static wxString name() { return "INITIALDATE"; }
        explicit INITIALDATE(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit INITIALDATE(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct FAVORITEACCT : public TableOpV<wxString>
    {
        static wxString name() { return "FAVORITEACCT"; }
        explicit FAVORITEACCT(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit FAVORITEACCT(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct CURRENCYID : public TableOpV<int64>
    {
        static wxString name() { return "CURRENCYID"; }
        explicit CURRENCYID(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit CURRENCYID(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct STATEMENTLOCKED : public TableOpV<int64>
    {
        static wxString name() { return "STATEMENTLOCKED"; }
        explicit STATEMENTLOCKED(const int64 &v): TableOpV<int64>(OP_EQ, v) {}
        explicit STATEMENTLOCKED(OP op, const int64 &v): TableOpV<int64>(op, v) {}
    };

    struct STATEMENTDATE : public TableOpV<wxString>
    {
        static wxString name() { return "STATEMENTDATE"; }
        explicit STATEMENTDATE(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit STATEMENTDATE(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct MINIMUMBALANCE : public TableOpV<double>
    {
        static wxString name() { return "MINIMUMBALANCE"; }
        explicit MINIMUMBALANCE(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit MINIMUMBALANCE(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    struct CREDITLIMIT : public TableOpV<double>
    {
        static wxString name() { return "CREDITLIMIT"; }
        explicit CREDITLIMIT(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit CREDITLIMIT(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    struct INTERESTRATE : public TableOpV<double>
    {
        static wxString name() { return "INTERESTRATE"; }
        explicit INTERESTRATE(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit INTERESTRATE(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    struct PAYMENTDUEDATE : public TableOpV<wxString>
    {
        static wxString name() { return "PAYMENTDUEDATE"; }
        explicit PAYMENTDUEDATE(const wxString &v): TableOpV<wxString>(OP_EQ, v) {}
        explicit PAYMENTDUEDATE(OP op, const wxString &v): TableOpV<wxString>(op, v) {}
    };

    struct MINIMUMPAYMENT : public TableOpV<double>
    {
        static wxString name() { return "MINIMUMPAYMENT"; }
        explicit MINIMUMPAYMENT(const double &v): TableOpV<double>(OP_EQ, v) {}
        explicit MINIMUMPAYMENT(OP op, const double &v): TableOpV<double>(op, v) {}
    };

    typedef ACCOUNTID PRIMARY;

    // Data is a single record in the database table
    struct Data
    {
        int64 ACCOUNTID; // primary key
        wxString ACCOUNTNAME;
        wxString ACCOUNTTYPE;
        wxString ACCOUNTNUM;
        wxString STATUS;
        wxString NOTES;
        wxString HELDAT;
        wxString WEBSITE;
        wxString CONTACTINFO;
        wxString ACCESSINFO;
        double INITIALBAL;
        wxString INITIALDATE;
        wxString FAVORITEACCT;
        int64 CURRENCYID;
        int64 STATEMENTLOCKED;
        wxString STATEMENTDATE;
        double MINIMUMBALANCE;
        double CREDITLIMIT;
        double INTERESTRATE;
        wxString PAYMENTDUEDATE;
        double MINIMUMPAYMENT;

        explicit Data();
        explicit Data(wxSQLite3ResultSet& q);
        Data(const Data& other) = default;

        int64 id() const { return ACCOUNTID; }
        void id(const int64 id) { ACCOUNTID = id; }
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

    static bool match(const Data* data, const ACCOUNTID& op)
    {
        return data->ACCOUNTID == op.m_value;
    }

    static bool match(const Data* data, const ACCOUNTNAME& op)
    {
        return data->ACCOUNTNAME.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const ACCOUNTTYPE& op)
    {
        return data->ACCOUNTTYPE.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const ACCOUNTNUM& op)
    {
        return data->ACCOUNTNUM.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const STATUS& op)
    {
        return data->STATUS.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const NOTES& op)
    {
        return data->NOTES.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const HELDAT& op)
    {
        return data->HELDAT.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const WEBSITE& op)
    {
        return data->WEBSITE.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const CONTACTINFO& op)
    {
        return data->CONTACTINFO.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const ACCESSINFO& op)
    {
        return data->ACCESSINFO.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const INITIALBAL& op)
    {
        return data->INITIALBAL == op.m_value;
    }

    static bool match(const Data* data, const INITIALDATE& op)
    {
        return data->INITIALDATE.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const FAVORITEACCT& op)
    {
        return data->FAVORITEACCT.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const CURRENCYID& op)
    {
        return data->CURRENCYID == op.m_value;
    }

    static bool match(const Data* data, const STATEMENTLOCKED& op)
    {
        return data->STATEMENTLOCKED == op.m_value;
    }

    static bool match(const Data* data, const STATEMENTDATE& op)
    {
        return data->STATEMENTDATE.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const MINIMUMBALANCE& op)
    {
        return data->MINIMUMBALANCE == op.m_value;
    }

    static bool match(const Data* data, const CREDITLIMIT& op)
    {
        return data->CREDITLIMIT == op.m_value;
    }

    static bool match(const Data* data, const INTERESTRATE& op)
    {
        return data->INTERESTRATE == op.m_value;
    }

    static bool match(const Data* data, const PAYMENTDUEDATE& op)
    {
        return data->PAYMENTDUEDATE.CmpNoCase(op.m_value) == 0;
    }

    static bool match(const Data* data, const MINIMUMPAYMENT& op)
    {
        return data->MINIMUMPAYMENT == op.m_value;
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

    AccountTable();
    ~AccountTable();

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
            if (item->id() > 0 && AccountTable::match(item, args...)) {
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

    struct SorterByACCESSINFO
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.ACCESSINFO < y.ACCESSINFO;
        }
    };

    struct SorterByACCOUNTID
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.ACCOUNTID < y.ACCOUNTID;
        }
    };

    struct SorterByACCOUNTNAME
    {
        bool operator()(const Data& x, const Data& y)
        {
            // Locale case-insensitive
            return std::wcscoll(x.ACCOUNTNAME.Lower().wc_str(), y.ACCOUNTNAME.Lower().wc_str()) < 0;
        }
    };

    struct SorterByACCOUNTNUM
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.ACCOUNTNUM < y.ACCOUNTNUM;
        }
    };

    struct SorterByACCOUNTTYPE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.ACCOUNTTYPE < y.ACCOUNTTYPE;
        }
    };

    struct SorterByCONTACTINFO
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.CONTACTINFO < y.CONTACTINFO;
        }
    };

    struct SorterByCREDITLIMIT
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.CREDITLIMIT < y.CREDITLIMIT;
        }
    };

    struct SorterByCURRENCYID
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.CURRENCYID < y.CURRENCYID;
        }
    };

    struct SorterByFAVORITEACCT
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.FAVORITEACCT < y.FAVORITEACCT;
        }
    };

    struct SorterByHELDAT
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.HELDAT < y.HELDAT;
        }
    };

    struct SorterByINITIALBAL
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.INITIALBAL < y.INITIALBAL;
        }
    };

    struct SorterByINITIALDATE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.INITIALDATE < y.INITIALDATE;
        }
    };

    struct SorterByINTERESTRATE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.INTERESTRATE < y.INTERESTRATE;
        }
    };

    struct SorterByMINIMUMBALANCE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.MINIMUMBALANCE < y.MINIMUMBALANCE;
        }
    };

    struct SorterByMINIMUMPAYMENT
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.MINIMUMPAYMENT < y.MINIMUMPAYMENT;
        }
    };

    struct SorterByNOTES
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.NOTES < y.NOTES;
        }
    };

    struct SorterByPAYMENTDUEDATE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.PAYMENTDUEDATE < y.PAYMENTDUEDATE;
        }
    };

    struct SorterBySTATEMENTDATE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.STATEMENTDATE < y.STATEMENTDATE;
        }
    };

    struct SorterBySTATEMENTLOCKED
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.STATEMENTLOCKED < y.STATEMENTLOCKED;
        }
    };

    struct SorterBySTATUS
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.STATUS < y.STATUS;
        }
    };

    struct SorterByWEBSITE
    {
        bool operator()(const Data& x, const Data& y)
        {
            return x.WEBSITE < y.WEBSITE;
        }
    };
};
