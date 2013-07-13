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
#include "paths.h"

class Model_Setting : public Model, public DB_Table_SETTING_V1
{
    using DB_Table_SETTING_V1::all;
public:
    Model_Setting(): Model(), DB_Table_SETTING_V1() {};
    ~Model_Setting() {};

public:
    static Model_Setting& instance()
    {
        return Singleton<Model_Setting>::instance();
    }

public:
    Data_Set all(COLUMN col = COLUMN(0), bool asc = true)
    {
        this->ensure(this->db_);
        return this->all(this->db_, col, asc);
    }

public:
    // Setter
    void Set(const wxString& key, int value)
    {
        this->Set(key, wxString::Format("%d", value));
    }

    void Set(const wxString& key, bool value)
    {
        this->Set(key, wxString::Format("%s", value ? "TRUE" : "FALSE"));
    }

    void Set(const wxString& key, const wxColour& value)
    {
        this->Set(key, wxString::Format("%d,%d,%d", value.Red(), value.Green(), value.Blue()));
    }

    void Set(const wxString& key, const wxString& value)
    {
        Data* setting = 0;
        Data_Set items = this->find(this->db_, COL_SETTINGNAME, key);
        if (!items.empty()) setting = this->get(items[0].SETTINGID, this->db_);
        if (setting)
        {
            setting->SETTINGVALUE = value;
            setting->save(this->db_);
        }
        else
        {
            setting = this->create();
            setting->SETTINGNAME = key;
            setting->SETTINGVALUE = value;
            setting->save(this->db_);
        }
    }
public:
    // Getter
    bool GetBoolSetting(const wxString& key, bool default_value)
    {
        wxString value = this->GetStringSetting(key, "");
        if (value == "TRUE") return true;
        if (value == "FALSE") return false;

        return default_value; 
    }

    int GetIntSetting(const wxString& key, int default_value)
    {
        wxString value = this->GetStringSetting(key, "");
        if (!value.IsEmpty() && value.IsNumber()) return wxAtoi(value);

        return default_value;
    }

    wxColour GetColourSetting(const wxString& key, const wxColour& default_value = wxColour(255, 255,255))
    {
        wxString value = this->GetStringSetting(key, "");
        if (!value.IsEmpty())
        {
            wxRegEx pattern("([0-9]{1,3}),([0-9]{1,3}),([0-9]{1,3})");
            if (pattern.Matches(value))
            {
                wxString red = pattern.GetMatch(value, 1);
                wxString green = pattern.GetMatch(value, 2);
                wxString blue = pattern.GetMatch(value, 3);

                return wxColour(wxAtoi(red), wxAtoi(green), wxAtoi(blue));
            }
            else
            {
                return wxColor(value);
            }
        }

        return default_value;
    }

    wxString GetStringSetting(const wxString& key, const wxString& default_value)
    {
        for (const auto& record: this->all())
        {
            if (record.SETTINGNAME == key)
                return record.SETTINGVALUE;
        }
        return default_value;
    }
    wxString getLastDbPath()
    {
        wxString path = this->GetStringSetting("LASTFILENAME", "");

        if (!mmex::isPortableMode()) return path;

        wxString vol = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetVolume();

        if (!vol.IsEmpty())
        {

            wxFileName fname(path);
            fname.SetVolume(vol); // database should be on portable device

            if (fname.FileExists()) {
                path = fname.GetFullPath();
            }
        }

        return path;
    }
};

#endif // 
