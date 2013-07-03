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
#include "db/DB_Table.h"
#include "singleton.h"

class wxString;
class wxSQLite3Statement;
class wxSQLite3Database;
class wxSQLite3ResultSet;

class Model
{
public:
    Model():table_(0) {};
    Model(DB_Table* table):table_(table) {};
    virtual ~Model() { if (table_) delete table_; };

protected:
    DB_Table* table_;
};
#endif // 
