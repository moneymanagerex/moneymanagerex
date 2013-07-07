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

#ifndef MODEL_BUDGETYEAR_H
#define MODEL_BUDGETYEAR_H

#include "Model.h"
#include "db/DB_Table_Budgetyear_V1.h"

class Model_Budgetyear : public Model, public DB_Table_BUDGETYEAR_V1
{
    using DB_Table_BUDGETYEAR_V1::all;
public:
    Model_Budgetyear(): Model(), DB_Table_BUDGETYEAR_V1() {};
    ~Model_Budgetyear() 
    {
    };

public:
    static Model_Budgetyear& instance()
    {
        return Singleton<Model_Budgetyear>::instance();
    }

    Data_Set all(COLUMN col = COLUMN(0), bool asc = true)
    {
        this->ensure(this->db_);
        return this->all(this->db_, col, asc);
    }

public:
    // Setter
    void Set(int year_id, const wxString& value)
    {
        Data* info = this->get(year_id, this->db_);
        if (info)
        {
            info->BUDGETYEARNAME = value;
            info->save(this->db_);
        }
        else
        {
            info = this->create();
            info->BUDGETYEARID = year_id;
            info->BUDGETYEARNAME = value;
            info->save(this->db_);
        }
    }

    int Add(const wxString& value)
    {
        int year_id = this->Get(value);
        if (year_id < 0)
        {
            Data* e = this->create();
            e->BUDGETYEARNAME = value;
            e->save(this->db_);
            year_id = e->id();
        }
        return year_id;
    }
public:
    // Getter
    wxString Get(int year_id)
    {
        Data* e = this->get(year_id, this->db_);
        if (e) return e->BUDGETYEARNAME;

        return "";
    }
    int Get(const wxString& year_name)
    {
        for (const auto& record: this->all())
        {
            if (record.BUDGETYEARNAME == year_name)
                return record.BUDGETYEARID;
        }

        return -1;
    }
    bool Exists(int year_id)
    {
        Data* e = this->get(year_id, this->db_);
        if (e) return true;

        return false;
    }
    bool Exists(const wxString& year_name)
    {
        for (const auto& record: this->all())
        {
            if (record.BUDGETYEARNAME == year_name) 
                return true;
        }
        return false;
    }
};

#endif // 
