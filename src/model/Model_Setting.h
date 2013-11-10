/*******************************************************
 Copyright (C) 2013 Guan Lisheng

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ********************************************************/

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
    /** Return the static instance of Model_Setting table. */
    static Model_Setting& instance()
    {
        return Singleton<Model_Setting>::instance();
    }

    /**
    * Initialize the global Model_Setting table.
    * Reset the Model_Setting table or create the table if it does not exist.
    */
    static Model_Setting& instance(wxSQLite3Database* db)
    {
        Model_Setting& ins = Singleton<Model_Setting>::instance();
        ins.db_ = db;
        ins.ensure(db);

        return ins;
    }

public:
    /** Return a list of Data records (Data_Set) derived directly from the database. */
    Data_Set all(COLUMN col = COLUMN(0), bool asc = true)
    {
        this->ensure(this->db_);
        return this->all(this->db_, col, asc);
    }

    template<typename... Args>
    Data_Set find(const Args&... args)
    {
        return find_by(this, db_, true, args...);
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
        Data_Set items = this->find(SETTINGNAME(key));
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
        Data_Set items = this->find(SETTINGNAME(key));
        if (!items.empty()) return items[0].SETTINGVALUE;
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

    /* Returns true if key setting found */
    bool ContainsSetting(const wxString& key)
    {
        return !this->find(SETTINGNAME(key)).empty();
    }
};

#endif // 
