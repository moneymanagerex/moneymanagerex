/*******************************************************
 Copyright (C) 2013 Guan Lisheng (guanlisheng@gmail.com)

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

#ifndef MODEL_INFOTABLE_H
#define MODEL_INFOTABLE_H

#include "Model.h"
#include "db/DB_Table_Infotable_V1.h"
#include "defs.h"
#include "constants.h"

class Model_Infotable : public Model<DB_Table_INFOTABLE_V1>
{
public:
    Model_Infotable(): Model<DB_Table_INFOTABLE_V1>() {};
    ~Model_Infotable() 
    {
    };

public:
    /** Return the static instance of Model_Infotable */
    static Model_Infotable& instance()
    {
        return Singleton<Model_Infotable>::instance();
    }

    /**
    * Initialize the global Model_Infotable.
    * Reset the Model_Infotable or create the table if it does not exist.
    */
    static Model_Infotable& instance(wxSQLite3Database* db)
    {
        Model_Infotable& ins = Singleton<Model_Infotable>::instance();
        ins.db_ = db;
        ins.destroy_cache();
        ins.ensure(db);
        if (!ins.KeyExists("MMEXVERSION"))
        {
            ins.Set("MMEXVERSION", mmex::getProgramVersion());
            ins.Set("DATAVERSION", mmex::DATAVERSION);
            ins.Set("CREATEDATE", wxDateTime::Now());
            ins.Set("DATEFORMAT", mmex::DEFDATEFORMAT);
        }

        return ins;
    }

public:
    // Setter
    void Set(const wxString& key, int value)
    {
        this->Set(key, wxString::Format("%d", value));
    }

    void Set(const wxString& key, const wxDateTime& date)
    {
        this->Set(key, date.FormatISODate());
    }
    void Set(const wxString& key, const wxString& value)
    {
        Data* info = 0;
        Data_Set items = this->find(INFONAME(key));
        if (!items.empty())
            info = this->get(items[0].INFOID);
        if (info)
        {
            info->INFOVALUE= value;
            info->save(this->db_);
        }
        else
        {
            info = this->create();
            info->INFONAME = key;
            info->INFOVALUE = value;
            info->save(this->db_);
        }
    }
public:
    // Getter
    bool GetBoolInfo(const wxString& key, bool default_value)
    {
        wxString value = this->GetStringInfo(key, "");
        if (value == "1" || value.CmpNoCase("TRUE") == 0) return true;
        if (value == "0" || value.CmpNoCase("FALSE") == 0) return false;

        return default_value; 
    }
    int GetIntInfo(const wxString& key, int default_value)
    {
        wxString value = this->GetStringInfo(key, "");
        if (!value.IsEmpty() && value.IsNumber()) return wxAtoi(value);

        return default_value;
    }
    wxString GetStringInfo(const wxString& key, const wxString& default_value)
    {
        Data_Set items = this->find(INFONAME(key));
        if (!items.empty()) return items[0].INFOVALUE;

        return default_value;
    }
    int GetBaseCurrencyId()
    {
        return this->GetIntInfo("BASECURRENCYID", -1);
    }

    /* Returns true if key setting found */
    bool KeyExists(const wxString& key)
    {
        return !this->find(INFONAME(key)).empty();
    }

    bool checkDBVersion()
    {
        if (!this->KeyExists("DATAVERSION")) return false;

        return this->GetIntInfo("DATAVERSION", 0) >= mmex::MIN_DATAVERSION;
    }
public:
    static loop_t to_loop_t()
    {
        loop_t loop;
        for (const auto &r: instance().all())
            loop += r.to_row_t();
        return loop;
    }
};

#endif // 
