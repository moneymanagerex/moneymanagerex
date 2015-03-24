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

#include "Model_Setting.h"
#include "constants.h"
#include "mmOption.h"
#include "paths.h"

Model_Setting::Model_Setting()
: Model<DB_Table_SETTING_V1>()
{
}

Model_Setting::~Model_Setting()
{
}

/**
* Initialize the global Model_Setting table.
* Reset the Model_Setting table or create the table if it does not exist.
*/
Model_Setting& Model_Setting::instance(wxSQLite3Database* db)
{
    Model_Setting& ins = Singleton<Model_Setting>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);
    ins.preload();

    return ins;
}

/** Return the static instance of Model_Setting table. */
Model_Setting& Model_Setting::instance()
{
    return Singleton<Model_Setting>::instance();
}

// Setter
void Model_Setting::Set(const wxString& key, int value)
{
    this->Set(key, wxString::Format("%d", value));
}

void Model_Setting::Set(const wxString& key, bool value)
{
    this->Set(key, wxString::Format("%s", value ? "TRUE" : "FALSE"));
}

void Model_Setting::Set(const wxString& key, const wxColour& value)
{
    this->Set(key, wxString::Format("%d,%d,%d", value.Red(), value.Green(), value.Blue()));
}

void Model_Setting::Set(const wxString& key, const wxString& value)
{
    Data* setting = this->get_one(SETTINGNAME(key));
    if (!setting) // not cached
    {
        Data_Set items = this->find(SETTINGNAME(key));
        if (!items.empty()) setting = this->get(items[0].SETTINGID, this->db_);
    }
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

// Getter
bool Model_Setting::GetBoolSetting(const wxString& key, bool default_value)
{
    wxString value = this->GetStringSetting(key, "");
    if (value == "TRUE") return true;
    if (value == "FALSE") return false;

    return default_value; 
}

int Model_Setting::GetIntSetting(const wxString& key, int default_value)
{
    wxString value = this->GetStringSetting(key, "");
    if (!value.IsEmpty() && value.IsNumber()) return wxAtoi(value);

    return default_value;
}

wxString Model_Setting::GetStringSetting(const wxString& key, const wxString& default_value)
{
    Data* setting = this->get_one(SETTINGNAME(key));
    if (!setting) // not cached
    {
        Data_Set items = this->find(SETTINGNAME(key));
        if (!items.empty()) return items[0].SETTINGVALUE;
    }
    else
    {
        return setting->SETTINGVALUE;
    }
    return default_value;
}

wxString Model_Setting::getLastDbPath()
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
bool Model_Setting::ContainsSetting(const wxString& key)
{
    return !this->find(SETTINGNAME(key)).empty();
}

row_t Model_Setting::to_row_t()
{
    row_t row;
    for (const auto &r: instance().all())
        row(r.SETTINGNAME.ToStdWstring()) = r.SETTINGVALUE;
    return row;
}
//-------------------------------------------------------------------

bool Model_Setting::DisplayInternetNews()
{
    return GetBoolSetting(INIDB_DISPLAY_INTERNET_NEWS, true);
}

void Model_Setting::SetDisplayInternetNews(bool display)
{
    Set(INIDB_DISPLAY_INTERNET_NEWS, display);
}

//-------------------------------------------------------------------
wxString Model_Setting::ViewAccounts()
{
    return GetStringSetting("VIEWACCOUNTS", VIEW_ACCOUNTS_ALL_STR);
}

void Model_Setting::SetViewAccounts(const wxString& value)
{
    Set("VIEWACCOUNTS", value);
}

//-------------------------------------------------------------------
wxString Model_Setting::ViewTransactions()
{
    return GetStringSetting("VIEWTRANSACTIONS", VIEW_TRANS_ALL_STR);
}

void Model_Setting::SetViewTransactions(const wxString& value)
{
    Set("VIEWTRANSACTIONS", value);
}

//-------------------------------------------------------------------
wxString Model_Setting::HtmlFontSize()
{
    return GetStringSetting("HTMLFONTSIZE", "Small");
}

void Model_Setting::SetHtmlFontSize(const wxString& size)
{
    Set("HTMLFONTSIZE", size);
    mmIniOptions::instance().html_font_size_ = size;
}

//-------------------------------------------------------------------
bool Model_Setting::BudgetFinancialYears()
{
    return GetBoolSetting(INIDB_BUDGET_FINANCIAL_YEARS, false);
}

void Model_Setting::SetBudgetFinancialYears(bool value)
{
    Set(INIDB_BUDGET_FINANCIAL_YEARS, value);
    mmIniOptions::instance().budgetFinancialYears_ = value;
}

//-------------------------------------------------------------------
bool Model_Setting::BudgetIncludeTransfers()
{
    return GetBoolSetting(INIDB_BUDGET_INCLUDE_TRANSFERS, false);
}

void Model_Setting::SetBudgetIncludeTransfers(bool value)
{
    Set(INIDB_BUDGET_INCLUDE_TRANSFERS, value);
    mmIniOptions::instance().budgetIncludeTransfers_ = value;
}

//-------------------------------------------------------------------
bool Model_Setting::BudgetSetupWithoutSummary()
{
    return GetBoolSetting(INIDB_BUDGET_SETUP_WITHOUT_SUMMARY, false);
}

void Model_Setting::SetBudgetSetupWithoutSummary(bool value)
{
    Set(INIDB_BUDGET_SETUP_WITHOUT_SUMMARY, value);
    mmIniOptions::instance().budgetSetupWithoutSummaries_ = value;
}

//-------------------------------------------------------------------
bool Model_Setting::BudgetSummaryWithoutCategory()
{
    return GetBoolSetting(INIDB_BUDGET_SUMMARY_WITHOUT_CATEG, true);
}

void Model_Setting::SetBudgetSummaryWithoutCategory(bool value)
{
    Set(INIDB_BUDGET_SUMMARY_WITHOUT_CATEG, value);
    mmIniOptions::instance().budgetReportWithSummaries_ = value;
}

//-------------------------------------------------------------------
bool Model_Setting::IgnoreFutureTransactions()
{
    return GetBoolSetting(INIDB_IGNORE_FUTURE_TRANSACTIONS, false);
}

void Model_Setting::SetIgnoreFutureTransactions(bool value)
{
    Set(INIDB_IGNORE_FUTURE_TRANSACTIONS, value);
    mmIniOptions::instance().ignoreFutureTransactions_ = value;
}
