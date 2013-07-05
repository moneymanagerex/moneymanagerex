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

class Model_Infotable : public Model, public DB_Table_INFOTABLE_V1
{
    using DB_Table_INFOTABLE_V1::all;
public:
    Model_Infotable(): Model(), DB_Table_INFOTABLE_V1() {};
    ~Model_Infotable() {};

public:
    static Model_Infotable& instance()
    {
        return Singleton<Model_Infotable>::instance();
    }

private:
    Data_Set all(COLUMN col = COLUMN(0), bool asc = true)
    {
        return this->all(this->db_, col, asc);
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
        for (const auto& record: this->all())
            if (record.INFONAME == key) return record.INFOVALUE;

        return default_value;
    }
};

#endif // 
