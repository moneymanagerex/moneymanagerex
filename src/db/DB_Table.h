// -*- C++ -*-
//=============================================================================
/**
 *      Copyright (c) 2013 Guan Lisheng (guanlisheng@gmail.com)
 *
 *      @file
 *
 *      @author [sqlite2cpp.py]
 *
 *      @brief
 *
 *      Revision History:
 *          AUTO GENERATED at 2013-10-15 13:58:18.154138.
 *          DO NOT EDIT!
 */
//=============================================================================

#ifndef DB_TABLE_H
#define DB_TABLE_H

#include <vector>
#include <map>
#include <algorithm>
#include <functional>
#include <wx/wxsqlite3.h>

#include "cajun/json/elements.h"
#include "cajun/json/reader.h"
#include "cajun/json/writer.h"

class wxString;
template<class V>
struct DB_Column
{
    V v_;
    DB_Column(const V& v): v_(v)
    {}
};

struct DB_Table
{
    DB_Table(): hit_(0), miss_(0), skip_(0) {};
    virtual ~DB_Table() {};
    wxString query_;
    size_t hit_, miss_, skip_;
    virtual wxString query() const { return this->query_; }
    virtual size_t num_columns() const = 0;
    virtual wxString name() const = 0;

    bool exists(wxSQLite3Database* db) const
    {
       return db->TableExists(this->name()); 
    }
};

template<typename Arg1>
void condition(wxString& out, bool op_and, const Arg1& arg1)
{
    out += Arg1::name() + " = ? ";
}

template<typename Arg1, typename... Args>
void condition(wxString& out, bool op_and, const Arg1& arg1, const Args&... args) 
{
    out += Arg1::name() + " = ? ";
    out += op_and? " AND " : " OR ";
    condition(out, op_and, args...);
}

template<typename Arg1>
void bind(wxSQLite3Statement& stmt, int index, const Arg1& arg1)
{
    stmt.Bind(index, arg1.v_);
}

template<typename Arg1, typename... Args>
void bind(wxSQLite3Statement& stmt, int index, const Arg1& arg1, const Args&... args)
{
    stmt.Bind(index, arg1.v_); 
    bind(stmt, index+1, args...);
}

template<typename TABLE, typename... Args>
typename TABLE::Data_Set find_by(TABLE* table, wxSQLite3Database* db, bool op_and, const Args&... args)
{
    typename TABLE::Data_Set result;
    try
    {
        wxString query = table->query() + " WHERE ";
        condition(query, op_and, args...);
        wxSQLite3Statement stmt = db->PrepareStatement(query);
        bind(stmt, 1, args...);

        wxLogDebug(stmt.GetSQL());
        wxSQLite3ResultSet q = stmt.ExecuteQuery();

        while(q.NextRow())
        {
            typename TABLE::Data entity(q, table);
            result.push_back(entity);
        }

        q.Finalize();
    }
    catch(const wxSQLite3Exception &e) 
    { 
        wxLogError("%s: Exception %s", table->name().c_str(), e.GetMessage().c_str());
    }
 
    return result;
}
#endif // 
