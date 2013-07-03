/****************************************************************************
 Copyright (C) 2012 Stefano Giorgio

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
 ****************************************************************************/

/****************************************************************************
 Revision of last commit: $Revision$
 Author of last commit:   $Author$
 ****************************************************************************/
#ifndef _MM_EX_SETTINGS_H_
#define _MM_EX_SETTINGS_H_

#include "defs.h"
#include <memory>
#include <vector>

/****************************************************************************
 MMEX_IniRecord Class

 This class holds a single record for the Initalization database
 ****************************************************************************/
class MMEX_IniRecord
{
public:
    /// Create a new record setting.
    MMEX_IniRecord(std::shared_ptr<wxSQLite3Database> ini_db
                   , bool main_db
                   , const wxString& name);

    /// Create an existing record setting.
    MMEX_IniRecord(std::shared_ptr<wxSQLite3Database> ini_db
                   , bool main_db
                   , wxSQLite3ResultSet& q1);

    /// Set the value of the record
    void SetValue(int value);
    void SetValue(const wxString& value);
    void Save();
    wxString Name();
    wxString Value();

private:
    std::shared_ptr<wxSQLite3Database> iniDb_;
    bool main_db_;
    int settingId_;
    wxString settingName_;
    wxString settingValue_;
};

/****************************************************************************
 MMEX_IniSettings Class

 This class holds all the records for the Initalization database
 ****************************************************************************/
class MMEX_IniSettings
{
public:
    /// Constructor
    MMEX_IniSettings(std::shared_ptr<wxSQLite3Database> ini_db, bool main_db = false);
    ~MMEX_IniSettings();

    bool GetBoolSetting(const wxString& name, bool default_value);
    int GetIntSetting(const wxString& name, int default_value);
    wxColour GetColourSetting(const wxString& name, const wxColour& color = wxColour(255, 255,255));
    wxString GetStringSetting(const wxString& name, const wxString& default_value);

    /// Save to existing value
    void SetSetting(const wxString& name, bool value);
    void SetSetting(const wxString& name, int value);
    void SetSetting(const wxString& name, const wxDateTime& date);
    void SetSetting(const wxString& name, const wxColour& color);
    void SetSetting(const wxString& name, const wxString& value);

    bool Exists(const wxString& name);
    void Load();
    void Save();
private:
    std::shared_ptr<wxSQLite3Database> ini_db_;
    bool main_db_;
    std::vector< std::shared_ptr<MMEX_IniRecord> > ini_records_;

    MMEX_IniRecord* GetRecord(const wxString& name);
};

#endif // _MM_EX_SETTINGS_H_
