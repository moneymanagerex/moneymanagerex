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

#include "Model_Report.h"
#include "constants.h"
#include "paths.h"
#include "option.h"
#include "platfdep.h"
#include "attachmentdialog.h"
#include "reports/htmlbuilder.h"
#include "reports/reportbase.h"
#include "Model_Setting.h"
#include <LuaGlue/LuaGlue.h>
#include "sqlite3.h"
#include "mmreportspanel.h"

#if defined (__WXMSW__)
    #include <wx/msw/registry.h>
#endif

class Record : public std::map<std::wstring, std::wstring>
{
public:
    Record(){}
    ~Record(){}
    /* Access functions for LuaGlue (The required conversion between char and wchar_t is done through wxString.) */
    std::string get(const char* index)
    { 
        return std::string(wxString((*this)[wxString(index).ToStdWstring()]).ToUTF8());
    }
    void set(const char* index, const char * val)
    {
        (*this)[wxString(index).ToStdWstring()] = wxString::FromUTF8(val).ToStdWstring();
    }
};

Model_Report::Model_Report(): Model<DB_Table_REPORT>()
{
}

Model_Report::~Model_Report() 
{
}

/** Return the static instance of Model_Report table */
Model_Report& Model_Report::instance()
{
    return Singleton<Model_Report>::instance();
}

/**
* Initialize the global Model_Report table.
* Reset the Model_Report table or create the table if it does not exist.
*/
Model_Report& Model_Report::instance(wxSQLite3Database* db)
{
    Model_Report& ins = Singleton<Model_Report>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);

    return ins;
}

bool Model_Report::get_objects_from_sql(const wxString& query, PrettyWriter<StringBuffer>& json_writer)
{
    wxSQLite3Statement stmt;
    try
    {
        stmt = this->db_->PrepareStatement(query);
        if (!stmt.IsReadOnly())
        {
            json_writer.Key("msg");
            json_writer.String("the sql is not readonly");
            return false;
        }
    }
    catch (const wxSQLite3Exception& e)
    {
        json_writer.Key("msg");
        json_writer.String(e.GetMessage().c_str());
        return false;
    }

    try
    {
        json_writer.Key("results");
        json_writer.StartArray();

        wxSQLite3ResultSet q = stmt.ExecuteQuery();
        int columns = q.GetColumnCount();
        while (q.NextRow())
        {
            json_writer.StartObject();

            for (int i = 0; i < columns; ++i)
            {
                wxString column_name = q.GetColumnName(i);

                switch (q.GetColumnType(i))
                {
                    case WXSQLITE_INTEGER:
                        json_writer.Key(column_name.c_str());
                        json_writer.Int(q.GetInt(i));
                        break;
                    case WXSQLITE_FLOAT:
                        json_writer.Key(column_name.c_str());
                        json_writer.Double(q.GetDouble(i));
                        break;
                    default:
                        json_writer.Key(column_name.c_str());
                        json_writer.String(q.GetString(i).c_str());
                        break;
                }
            }

            json_writer.EndObject();
        }
        q.Finalize();

        json_writer.EndArray();
    }
    catch (const wxSQLite3Exception& e)
    {
        json_writer.Key("msg");
        json_writer.String(e.GetMessage().c_str());
        return false;
    }

    return true;
}

wxArrayString Model_Report::allGroupNames()
{
    wxArrayString groups;
    wxString PreviousGroup;
    for (const auto &report : this->all(COL_GROUPNAME))
    {
        if (report.GROUPNAME != PreviousGroup)
        {
            groups.Add(report.GROUPNAME);
            PreviousGroup = report.GROUPNAME;
        }
    }
    return groups;
}

bool Model_Report::PrepareSQL(wxString& sql, std::map <wxString, wxString>& rep_params)
{
    sql.Trim();
    if (sql.empty()) return false;
    if (sql.Last() != ';') sql += ';';

    int pos = sql.Lower().Find("&begin_date");
    size_t len = wxString("&begin_date").size();

    if (pos != wxNOT_FOUND)
    {
        wxDatePickerCtrl* start_date = (wxDatePickerCtrl*)
            wxWindow::FindWindowById(mmReportsPanel::RepPanel::ID_CHOICE_START_DATE);
        wxString date = wxDateTime::Today().FormatISODate();
        if (start_date) {
            date = start_date->GetValue().FormatISODate();
        }

        rep_params["begin_date"] = date;

        while (pos != wxNOT_FOUND)
        {
            sql.replace(pos, len, date);
            pos = sql.Lower().Find("&begin_date");
        }
    }

    pos = sql.Lower().Find("&single_date");
    len = wxString("&single_date").size();
    if (pos != wxNOT_FOUND)
    {
        wxDatePickerCtrl* start_date = (wxDatePickerCtrl*)
            wxWindow::FindWindowById(mmReportsPanel::RepPanel::ID_CHOICE_START_DATE);
        wxString date = wxDateTime::Today().FormatISODate();
        if (start_date) {
            date = start_date->GetValue().FormatISODate();
        }
        rep_params["single_date"] = date;

        while (pos != wxNOT_FOUND)
        {
            sql.replace(pos, len, date);
            pos = sql.Lower().Find("&single_date");
        }
    }

    pos = sql.Lower().Find("&end_date");
    len = wxString("&end_date").size();
    
    if (pos != wxNOT_FOUND)
    {
        wxDatePickerCtrl* end_date = (wxDatePickerCtrl*)
            wxWindow::FindWindowById(mmReportsPanel::RepPanel::ID_CHOICE_END_DATE);
        wxString date = wxDateTime::Today().FormatISODate();
        if (end_date) {
            date = end_date->GetValue().FormatISODate();
        }
        rep_params["end_date"] = date;

        while (pos != wxNOT_FOUND)
        {
            sql.replace(pos, len, date);
            pos = sql.Lower().Find("&end_date");
        }
    }

    pos = sql.Lower().Find("&budget_years");
    len = wxString("&budget_years").size();
    if (pos != wxNOT_FOUND)
    {
        wxChoice* years = (wxChoice*)
            wxWindow::FindWindowById(mmReportsPanel::RepPanel::ID_CHOICE_DATE_RANGE);
        wxString date = wxString::Format("%i", wxDate::Today().GetYear());
        if (years) {
            date = years->GetStringSelection();
        }
        rep_params["budget_years"] = date;

        while (pos != wxNOT_FOUND)
        {
            sql.replace(pos, len, date);
            pos = sql.Lower().Find("&budget_years");
        }
    }

    //TODO: other parameters
    return true;
}

wxString Model_Report::get_html(const Data* r)
{
    mm_html_template report(r->TEMPLATECONTENT);
    r->to_template(report);

    loop_t contents;

    loop_t errors;
    row_t error;

    wxSQLite3ResultSet q;
    int columnCount = 0;
    wxString sql = r->SQLCONTENT;
    std::map <wxString, wxString> rep_params;
    try
    {
        PrepareSQL(sql, rep_params);

        wxSQLite3Statement stmt = this->db_->PrepareStatement(sql);
        if (!stmt.IsReadOnly())
        {
            return wxString::Format(_("The SQL script:\n%s \nwill modify database! aborted!"), r->SQLCONTENT);
        }
        else
        {
            q = stmt.ExecuteQuery();
            columnCount = q.GetColumnCount();
        }
    }
    catch (const wxSQLite3Exception& e)
    {
        return e.GetMessage();
    }

    std::map <std::wstring, int> colHeaders;

    loop_t columns;
    for (int i = 0; i < columnCount; ++i)
    {
        int col_type = q.GetColumnType(i);
        const std::wstring col_name = q.GetColumnName(i).ToStdWstring();
        colHeaders[col_name] = col_type;
        row_t row;
        row(L"COLUMN") = col_name;
        columns += row;
    }
    report(L"COLUMNS") = columns;

    LuaGlue state;
    state.
        Class<Record>("Record").
        ctor("new").
        method("get", &Record::get).
        method("set", &Record::set).
        end().open().glue();

    bool skip_lua = r->LUACONTENT.IsEmpty();
    bool lua_status = state.doString(std::string(r->LUACONTENT.ToUTF8()));
    if (!skip_lua && !lua_status)
    {
        error(L"ERROR") = wxString("failed to doString : ") + r->LUACONTENT + wxString(" err: ") + wxString(state.lastError());
        errors += error;
    }

    while (q.NextRow())
    {
        Record rec;
        for (int i = 0; i < columnCount; ++i)
        {
            const wxString column_name = q.GetColumnName(i);
            rec[column_name.ToStdWstring()] = q.GetAsString(i);
        }

        if (lua_status && !skip_lua)
        {
            try
            {
                state.invokeVoidFunction("handle_record", &rec);
            }
            catch (const std::runtime_error& e)
            {
                error(L"ERROR") = wxString("failed to call handle_record : ") + wxString(e.what());
                errors += error;
            }
            catch (const std::exception& e)
            {
                error(L"ERROR") = wxString("failed to call handle_record : ") + wxString(e.what());
                errors += error;
            }
            catch (...)
            {
                error(L"ERROR") = L"failed to call handle_record ";
                errors += error;
            }
        }
        row_t row;
        for (const auto& item : rec)
        {
            row(item.first) = item.second;
        }
        contents += row;

    }
    q.Finalize();

    Record result;
    if (lua_status && !skip_lua)
    {
        try
        {
            state.invokeVoidFunction("complete", &result);
        }
        catch (const std::runtime_error& e)
        {
            error(L"ERROR") = wxString("failed to call complete: ") + wxString(e.what());
            errors += error;
        }
        catch (const std::exception& e)
        {
            error(L"ERROR") = wxString("failed to call complete: ") + wxString(e.what());
            errors += error;
        }
        catch (...)
        {
            error(L"ERROR") = L"failed to call complete";
            errors += error;
        }
    }

    for (const auto& item : result)
        report(item.first) = item.second;

    report(L"CONTENTS") = contents;
    {
        for (const auto& item : rep_params)
        {
            report(item.first.Upper().ToStdWstring()) = item.second;
        }
        auto p = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting());
        //javascript does not handle backslashs
        p.Replace("\\", "\\\\");
        report(L"ATTACHMENTSFOLDER") = p;
        auto s = wxString(wxFileName::GetPathSeparator());
        s.Replace("\\", "\\\\");
        report(L"FILESEPARATOR") = s;
        report(L"LANGUAGE") = Option::instance().LanguageISO6391();
        report(L"HTMLSCALE") = wxString::Format("%d", Option::instance().HtmlFontSize());
    }
    report(L"ERRORS") = errors;

    wxString out = wxEmptyString;
    try 
    {
        out = report.Process();
    }
    catch (const syntax_ex& e)
    {
        return e.what();
    }
    catch (...)
    {
        return _("Caught exception");
    }

    return out;
}

void Model_Report::prepareTempFolder()
{
    const wxString resDir = mmex::GetResourceDir().GetPathWithSep();
    const wxString tempDir = mmex::getTempFolder();
    wxFileName::Mkdir(tempDir, 511, wxPATH_MKDIR_FULL);
    wxArrayString filesArray;
    wxDir::GetAllFiles(resDir, &filesArray);
    for (const auto& sourceFile : filesArray)
    {
        const wxString repFile = tempDir + wxFileName(sourceFile).GetFullName();
        if (::wxFileExists(sourceFile))
        {
            if (!::wxFileExists(repFile)
                || wxFileName(sourceFile).GetModificationTime() > wxFileName(repFile).GetModificationTime())
            {
                if (!::wxCopyFile(sourceFile, repFile))
                    wxLogError("Could not copy %s !", sourceFile);
                else
                    wxLogDebug("Coping file:\n %s \nto\n %s", sourceFile, repFile);
            }
        }
    }
}

bool Model_Report::WindowsUpdateRegistry()
{
#if defined (__WXMSW__)
    // https://msdn.microsoft.com/en-us/library/ee330730(v=vs.85).aspx
    // https://kevinragsdale.net/windows-10-and-the-web-browser-control/
    wxRegKey Key(wxRegKey::HKCU, "Software\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_BROWSER_EMULATION");
    if (Key.Create(true) && Key.SetValue(wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetFullName(), 11001))
        return true;
    else
        return false;
#else
    return true;
#endif
}

bool Model_Report::outputReportFile(const wxString& str, const wxString& name)
{
    bool ok = true;
    wxFileOutputStream index_output(mmex::getReportFullFileName(name));
    if (index_output.IsOk())
    {
        wxTextOutputStream index_file(index_output);
        index_file << str;
        index_output.Close();
    }
    else
    {
        ok = false;
    }
    return ok;
}

wxString Model_Report::get_html(const Data& r) 
{ 
    return get_html(&r); 
}

Model_Report::Data* Model_Report::get(const wxString& name)
{
    Data* report = this->get_one(REPORTNAME(name));
    if (report) return report;

    Data_Set items = this->find(REPORTNAME(name));
    if (!items.empty()) report = this->get(items[0].id(), this->db_);
    return report;
}

