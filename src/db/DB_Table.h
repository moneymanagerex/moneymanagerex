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
 *          AUTO GENERATED at 2013-10-11 15:47:04.704673.
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
    DB_Table() {};
    virtual ~DB_Table() {};
    wxString query_;
    virtual wxString query() const { return this->query_; }
    virtual size_t num_columns() const = 0;
    virtual wxString name() const = 0;

    bool exists(wxSQLite3Database* db) const
    {
       return db->TableExists(this->name()); 
    }

    virtual void begin(wxSQLite3Database* db) const
    {
        db->Begin();
    }

    virtual void commit(wxSQLite3Database* db) const
    {
        db->Commit();
    }
};
#endif // 
