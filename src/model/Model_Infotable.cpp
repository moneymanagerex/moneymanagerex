/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)

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

#include "Model_Infotable.h"
#include "constants.h"
#include "util.h"

Model_Infotable::Model_Infotable()
: Model<DB_Table_INFOTABLE>()
{
}

Model_Infotable::~Model_Infotable()
{
}

/**
* Initialize the global Model_Infotable.
* Reset the Model_Infotable or create the table if it does not exist.
*/
Model_Infotable& Model_Infotable::instance(wxSQLite3Database* db)
{
    Model_Infotable& ins = Singleton<Model_Infotable>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);
    ins.preload();
    if (!ins.KeyExists("MMEXVERSION"))
    {
        ins.Set("MMEXVERSION", mmex::version::string);
        ins.Set("DATAVERSION", mmex::DATAVERSION);
        ins.Set("CREATEDATE", wxDateTime::Now());
        ins.Set("DATEFORMAT", mmex::DEFDATEFORMAT);
    }

    return ins;
}

/** Return the static instance of Model_Infotable */
Model_Infotable& Model_Infotable::instance()
{
    return Singleton<Model_Infotable>::instance();
}

// Setter
void Model_Infotable::Set(const wxString& key, int value)
{
    this->Set(key, wxString::Format("%d", value));
}

void Model_Infotable::Set(const wxString& key, const wxDateTime& date)
{
    this->Set(key, date.FormatISODate());
}

void Model_Infotable::Set(const wxString& key, const wxString& value)
{
    if (!this->db_) return;

    Data* info = this->get_one(INFONAME(key));
    if (!info) // not cached
    {
        Data_Set items = this->find(INFONAME(key));
        if (!items.empty()) info = this->get(items[0].INFOID);
    }
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

void Model_Infotable::Set(const wxString& key, const wxColour& value)
{
    this->Set(key, wxString::Format("%d,%d,%d", value.Red(), value.Green(), value.Blue()));
}

// Getter
bool Model_Infotable::GetBoolInfo(const wxString& key, bool default_value)
{
    const wxString value = this->GetStringInfo(key, "");
    if (value == "1" || value.CmpNoCase("TRUE") == 0)
        return true;
    else if (value == "0" || value.CmpNoCase("FALSE") == 0)
        return false;
    else
        return default_value;
}

int Model_Infotable::GetIntInfo(const wxString& key, int default_value)
{
    const wxString value = this->GetStringInfo(key, "");
    if (!value.IsEmpty() && value.IsNumber())
        return wxAtoi(value);

    return default_value;
}

wxString Model_Infotable::GetStringInfo(const wxString& key, const wxString& default_value)
{
    Data* info = this->get_one(INFONAME(key));
    if (info)
        return info->INFOVALUE;
    else // not cached
    {
        Data_Set items = this->find(INFONAME(key));
        if (!items.empty())
            return items[0].INFOVALUE;
    }

    return default_value;
}

const wxColour Model_Infotable::GetColourSetting(const wxString& key, const wxColour& default_value)
{
    const wxString value = this->GetStringInfo(key, "");
    if (!value.IsEmpty())
    {
        wxRegEx pattern("([0-9]{1,3}),([0-9]{1,3}),([0-9]{1,3})");
        if (pattern.Matches(value))
        {
            const wxString red = pattern.GetMatch(value, 1);
            const wxString green = pattern.GetMatch(value, 2);
            const wxString blue = pattern.GetMatch(value, 3);

            return wxColour(wxAtoi(red), wxAtoi(green), wxAtoi(blue));
        }
        else
        {
            return wxColor(value);
        }
    }

    return default_value;
}

/* Returns true if key setting found */
bool Model_Infotable::KeyExists(const wxString& key)
{
    return !this->find(INFONAME(key)).empty();
}

bool Model_Infotable::checkDBVersion()
{
    if (!this->KeyExists("DATAVERSION")) return false;

    return this->GetIntInfo("DATAVERSION", 0) >= mmex::MIN_DATAVERSION;
}

loop_t Model_Infotable::to_loop_t()
{
    loop_t loop;
    for (const auto &r: instance().all())
        loop += r.to_row_t();
    return loop;
}

//-------------------------------------------------------------------
bool Model_Infotable::OpenCustomDialog(const wxString& RefType)
{
    return GetBoolInfo("CUSTOMDIALOG_OPEN:" + RefType, false);
}

void Model_Infotable::SetOpenCustomDialog(const wxString& RefType, const bool Status)
{
    Set("CUSTOMDIALOG_OPEN:" + RefType, Status);
}

wxSize Model_Infotable::CustomDialogSize(const wxString& RefType)
{
    wxString strSize = GetStringInfo("CUSTOMDIALOG_SIZE:" + RefType, "0;0");
    return wxSize(wxAtoi(strSize.BeforeFirst(';')), wxAtoi(strSize.AfterFirst(';')));
}

void Model_Infotable::SetCustomDialogSize(const wxString& RefType, const wxSize& Size)
{
    wxString strSize;
    strSize << Size.GetWidth() << ";" << Size.GetHeight();
    Set("CUSTOMDIALOG_SIZE:" + RefType, strSize);
}