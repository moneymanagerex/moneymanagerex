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
 *          AUTO GENERATED at 2013-07-03 22:55:49.491495.
 *          DO NOT EDIT!
 */
//=============================================================================

#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <map>
#include <algorithm>
#include <wx/datetime.h>
#include "singleton.h"
#include <memory>

class wxString;
class wxSQLite3Statement;
class wxSQLite3Database;
class wxSQLite3ResultSet;

typedef wxDateTime wxDate;

class Model
{
public:
    Model():db_(0) {};
    virtual ~Model() {};

public:
    void Begin()
    {
        this->db_->Begin();
    }
    void Commit()
    {
        this->db_->Commit();
    }
public:
    wxDate to_date(const wxString& str_date)
    {
        wxDate date = wxDateTime::Today();
        date.ParseISODate(str_date); // the date in ISO 8601 format "YYYY-MM-DD".
        return date;
    }
public:
    wxSQLite3Database* db_;
};
#endif // 
