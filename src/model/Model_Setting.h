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

#ifndef MODEL_SETTING_H
#define MODEL_SETTING_H

#include "Model.h"
#include "db/DB_Table_Setting_V1.h"
#include "defs.h"

class Model_Setting : public Model
{
public:
    typedef DB_Table_SETTING_V1::COLUMN COLUMN;
    typedef DB_Table_SETTING_V1::Data Data;
    typedef DB_Table_SETTING_V1::Data_Set Data_Set;
public:
    Model_Setting(): Model(new DB_Table_SETTING_V1()) {};
    ~Model_Setting() {};

public:
    static Model_Setting& instance()
    {
        return Singleton<Model_Setting>::instance();
    }

public:
    Data_Set all(COLUMN col = COLUMN(0), bool asc = true)
    {
        return dynamic_cast<DB_Table_SETTING_V1*>(this->table_)->all(this->db_, col, asc);
    }

public:
    // Setter
    void Set(const wxString& key, int value)
    {
        this->Set(key, wxString::Format("%d", value));
    }

    void Set(const wxString& key, bool value)
    {
        this->Set(key, (value) ? "TRUE" : "FALSE"); 
    }

    void Set(const wxString& key, const wxColour& value)
    {
    }

    void Set(const wxString& key, const wxString& value)
    {
        
    }
public:
    // Getter
    bool GetBoolSetting(const wxString& key, bool default_value);
    {
        return default_value; 
    }
    bool GetIntSetting(const wxString& key, int default_value);
    {
        return default_value;
    }
    wxColour GetColourSetting(const wxString& key, const wxColour& default_value = wxColour(255, 255,255))
    {
        return default_value;
    }
    wxString GetStringSetting(const wxString& key, const wxString& default_value)
    {
        return default_value;
    }
};

#endif // 
