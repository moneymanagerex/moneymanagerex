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
#include "platfdep.h"
#include "attachmentdialog.h"
#include "reports/htmlbuilder.h"
#include "model/Model_Setting.h"
#include "LuaGlue/LuaGlue.h"
#include "sqlite3.h"

#if defined (__WXMSW__)
    #include <wx/msw/registry.h>
#endif

static const char *HTT_CONTEINER = R"(<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta http - equiv = "Content-Type" content = "text/html">
    <title><TMPL_VAR REPORTNAME></title>
    <script src = "ChartNew.js"></script>
    <script src = "sorttable.js"></script>
    <link href = "master.css" rel = "stylesheet">
</head>
<body>
<div class = "container">
<h3><TMPL_VAR REPORTNAME></h3>
<TMPL_VAR TODAY><hr>
<div class = "row">
<div class = "col-xs-2"></div>
<div class = "col-xs-8">
<table class = "table">
<thead>
    <tr>
%s
    </tr>
</thead>
<tbody>
    <TMPL_LOOP NAME=CONTENTS>
        <tr>
%s
        </tr>
    </TMPL_LOOP>
</tbody>

</table>
</div>
<TMPL_LOOP ERRORS>
    <TMPL_VAR ERROR>
</TMPL_LOOP>"
</div>
</div>
</body>
<script>
    <!-- Format double to base currency -->
    function currency(n) {
        n = parseFloat(n);
        n =  isNaN(n) ? 0 : n.toFixed(2);
        var out = n.toString().replace(".", "|");
        out = out.toString().replace(/\B(?=(\d{3})+(?!\d))/g, "<TMPL_VAR GROUP_SEPARATOR>");
        out = out.replace("|", "<TMPL_VAR DECIMAL_POINT>");
        return out;
    }
    var elements= document.getElementsByClassName("money");
    for (var i = 0; i < elements.length; i++) {
        var element = elements[i];
        element.style.textAlign='right';
        if (element.innerHTML.indexOf("-") > -1) {
            element.style.color="#ff0000";
        } 
        element.innerHTML = '<TMPL_VAR PFX_SYMBOL>' + currency(element.innerHTML) +'<TMPL_VAR SFX_SYMBOL>';
    }
</script>
</html>
)";

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

Model_Report::Model_Report(): Model<DB_Table_REPORT_V1>()
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

wxString Model_Report::get_html(const Data* r)
{
    mm_html_template report(r->TEMPLATECONTENT);
    r->to_template(report);

    loop_t contents;
    json::Array jsoncontents;

    loop_t errors;
    row_t error;

    wxSQLite3ResultSet q;
    int columnCount = 0;
    try
    {
        wxString sql = r->SQLCONTENT;
        sql.Trim();
        if (!sql.empty() && sql.Last() != ';') sql += ';';
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
        Record r;
        for (int i = 0; i < columnCount; ++i)
        {
            const wxString column_name = q.GetColumnName(i);
            r[column_name.ToStdWstring()] = q.GetAsString(i);
        }

        if (lua_status && !skip_lua)
        {
            try
            {
                state.invokeVoidFunction("handle_record", &r);
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
        json::Object o;
        for (const auto& item : r)
        {
            row(item.first) = item.second;

            double v;
            if ((colHeaders[item.first] == WXSQLITE_INTEGER || colHeaders[item.first] == WXSQLITE_FLOAT)
                && wxString(item.second).ToDouble(&v))
            {
                o[wxString(item.first).ToStdWstring()] = json::Number(v);
            }
            else
                o[wxString(item.first).ToStdWstring()] = json::String(wxString(item.second).ToStdWstring());
        }
        contents += row;
        jsoncontents.Insert(o);

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
        std::wstringstream ss;
        json::Writer::Write(jsoncontents, ss);
        report(L"JSONCONTENTS") = wxString(ss.str());
        auto p = mmex::getPathAttachment(mmAttachmentManage::InfotablePathSetting());
        //javascript does not handle backslashs
        p.Replace("\\", "\\\\");
        report(L"ATTACHMENTSFOLDER") = p;
        auto s = wxString(wxFileName::GetPathSeparator());
        s.Replace("\\", "\\\\");
        report(L"FILESEPARATOR") = s;
        report(L"LANGUAGE") = Model_Setting::instance().GetStringSetting(LANGUAGE_PARAMETER, "english");
    }
    report(L"ERRORS") = errors;

    wxString out = wxEmptyString;
    try 
    {
        out = report.Process();
    }
    catch (const syntax_ex& e)
    {
        out = e.what();
    }
    catch (...)
    {
        // TODO
    }

    outputReportFile(out);
    return out;
}

void Model_Report::prepareTempFolder()
{
    const wxString& repDir = wxFileName(mmex::getReportIndex()).GetPathWithSep();
    const wxString& resDir = mmex::GetResourceDir().GetPathWithSep();
    const wxString& tempDir = mmex::getTempFolder();
    wxString repFile;
    wxFileName::Mkdir(repDir, 511, wxPATH_MKDIR_FULL);
    wxFileName::Mkdir(tempDir, 511, wxPATH_MKDIR_FULL);
    wxArrayString filesArray;
    wxDir::GetAllFiles(resDir, &filesArray);
    for (const auto& sourceFile : filesArray)
    {
        repFile = repDir + wxFileName(sourceFile).GetFullName();
        if (::wxFileExists(sourceFile))
        {
            if (!::wxFileExists(repFile)
                || wxFileName(sourceFile).GetModificationTime() > wxFileName(repFile).GetModificationTime())
            {
                if (!::wxCopyFile(sourceFile, repFile))
                    wxLogError("Could not copy %s !", sourceFile);
            }
        }
        wxLogDebug("Coping file: %s to %s", sourceFile, repFile);
    }
}

bool Model_Report::WindowsUpdateRegistry()
{
#if defined (__WXMSW__)
    wxRegKey Key(wxRegKey::HKCU, "Software\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_BROWSER_EMULATION");
    if (Key.Create(true) && !Key.HasValue("mmex.exe") && Key.SetValue("mmex.exe", 9000))
        return true;
    else
        return false;
#else
    return true;
#endif 
}

void Model_Report::outputReportFile(const wxString& str)
{
    wxFileOutputStream index_output(mmex::getReportIndex());
    wxTextOutputStream index_file(index_output);
    index_file << str;
    index_output.Close();
}

wxString Model_Report::get_html(const Data& r) 
{ 
    return get_html(&r); 
}

bool Model_Report::getColumns(const wxString& sql, std::vector<std::pair<wxString, int> > &colHeaders)
{
    wxSQLite3Statement stmt;
    wxSQLite3ResultSet q;
    int columnCount = 0;
    try
    {
        stmt = this->db_->PrepareStatement(sql);
        if (!stmt.IsReadOnly())
            return false;
        q = stmt.ExecuteQuery();
        columnCount = q.GetColumnCount();
    }
    catch (const wxSQLite3Exception& /*e*/)
    {
        return false;
    }

    for (int i = 0; i < columnCount; ++i)
    {
        std::pair<wxString, int> col_and_type;
        col_and_type.second = q.GetColumnType(i);
        col_and_type.first = q.GetColumnName(i);

        colHeaders.push_back(col_and_type);
    }
    return true;
}

bool Model_Report::getSqlQuery(/*in*/ const wxString& sql, /*out*/ std::vector <std::vector <wxString> > &sqlQueryData)
{
    wxSQLite3ResultSet q;
    int columnCount = 0;
    try
    {
        wxString temp = sql;
        temp.Trim();
        if (temp.Last() != ';') temp += ';';
        wxSQLite3Statement stmt = this->db_->PrepareStatement(temp);
        if (!stmt.IsReadOnly())
            return false;
        q = stmt.ExecuteQuery();
        columnCount = q.GetColumnCount();
    }
    catch (const wxSQLite3Exception& /*e*/)
    {
        return false;
    }

    sqlQueryData.clear();
    while (q.NextRow())
    {
        std::vector<wxString> row;
        for (int i = 0; i < columnCount; ++i)
            row.push_back(q.GetAsString(i));
        sqlQueryData.push_back(row);
    }
    return true;
}

void Model_Report::getSqlTableInfo(std::vector<std::pair<wxString, wxArrayString>> &sqlTableInfo)
{
    const wxString sqlTables = "SELECT type, name FROM sqlite_master WHERE type = 'table' or type = 'view' ORDER BY type, name";
    const wxString sqlColumns = "PRAGMA table_info(%s);";
    sqlTableInfo.clear();

    // Get a list of the database tables
    wxSQLite3Statement stmtTables = this->db_->PrepareStatement(sqlTables);
    wxSQLite3ResultSet qTables = stmtTables.ExecuteQuery();
    while (qTables.NextRow())
    {
        const wxString table_name = qTables.GetAsString(1);

        // Get a list of the table columns
        const wxString& sql = wxString::Format(sqlColumns, table_name);
        wxSQLite3Statement stmtColumns = this->db_->PrepareStatement(sql);
        wxSQLite3ResultSet qColumns = stmtColumns.ExecuteQuery();
        wxArrayString column_names;
        while (qColumns.NextRow())
            column_names.push_back(qColumns.GetAsString(1));

        sqlTableInfo.push_back(std::make_pair(table_name, column_names));
    }
}

wxString Model_Report::getTemplate(const wxString& sql)
{
    wxString body, header;
    std::vector<std::pair<wxString, int> > colHeaders;
    this->getColumns(sql, colHeaders);
    for (const auto& col : colHeaders)
    {
        header += wxString::Format("        <th>%s</th>\n", col.first);
        if (col.second == WXSQLITE_FLOAT)
            body += wxString::Format("        <td class = \"money\"><TMPL_VAR \"%s\"></td>\n", col.first);
        else if (col.second == WXSQLITE_INTEGER)
            body += wxString::Format("        <td class = \"text-right\"><TMPL_VAR \"%s\"></td>\n", col.first);
        else
            body += wxString::Format("        <td><TMPL_VAR \"%s\"></td>\n", col.first);
    }
    return wxString::Format(HTT_CONTEINER, header, body);
}

Model_Report::Data* Model_Report::get(const wxString& name)
{
    Data* report = this->get_one(REPORTNAME(name));
    if (report) return report;

    Data_Set items = this->find(REPORTNAME(name));
    if (!items.empty()) report = this->get(items[0].id(), this->db_);
    return report;
}

