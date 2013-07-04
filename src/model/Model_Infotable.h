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

#ifndef MODEL_INFOTABLE_H
#define MODEL_INFOTABLE_H

#include "Model.h"
#include "db/DB_Table_Infotable_V1.h"
#include "defs.h"

class Model_Infotable : public Model
{
public:
    typedef DB_Table_INFOTABLE_V1::COLUMN COLUMN;
    typedef DB_Table_INFOTABLE_V1::Data Data;
    typedef DB_Table_INFOTABLE_V1::Data_Set Data_Set;
public:
    Model_Infotable(): Model(new DB_Table_INFOTABLE_V1()) {};
    ~Model_Infotable() {};

public:
    static Model_Infotable& instance()
    {
        return Singleton<Model_Infotable>::instance();
    }

public:
    Data_Set all(COLUMN col = COLUMN(0), bool asc = true)
    {
        return dynamic_cast<DB_Table_INFOTABLE_V1*>(this->table_)->all(this->db_, col, asc);
    }

public:
    // Setter
    void Set(const wxString& key, int value)
    {
        this->Set(key, wxString::Format("%d", value));
    }

    void Set(const wxString& key, const wxString& value)
    {
        
    }
public:
    // Getter
    bool GetIntInfotable(const wxString& key, int default_value);
    {
        return default_value;
    }
    wxString GetStringInfotable(const wxString& key, const wxString& default_value)
    {
        return default_value;
    }
};

#endif // 
