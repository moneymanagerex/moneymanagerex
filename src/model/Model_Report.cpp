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

#include "paths.h"
#include "platfdep.h"
#include "Model_Report.h"
#include "reports/htmlbuilder.h"
#include "LuaGlue/LuaGlue.h"
#include "sqlite3.h"

static const wxString HTT_CONTEINER =
"<!DOCTYPE html>\n"
"<html>\n"
"<head>\n"
"    <meta charset=\"UTF-8\" />\n"
"    <meta http - equiv = \"Content-Type\" content = \"text/html\" />\n"
"    <title><TMPL_VAR REPORTNAME></title>\n"
"    <script src = \"Chart.js\"></script>\n"
"    <link href = \"master.css\" rel = \"stylesheet\" />\n"
"</head>\n"
"<body>\n"
"<div class = \"container\">\n"
"<h3><TMPL_VAR REPORTNAME></h3>\n"
"<TMPL_VAR TODAY><hr>\n"
"<div class = \"row\">\n"
"<div class = \"col-xs-2\"></div>\n"
"<div class = \"col-xs-8\">\n"
"<table class = \"table\">\n"
"<thead>\n"
"    <tr>\n"
"%s"
"    </tr>\n"
"</thead>\n"
"<tbody>\n"
"    <TMPL_LOOP NAME=CONTENTS>\n"
"        <tr>\n"
"%s"
"        </tr>\n"
"    </TMPL_LOOP>\n"
"</tbody>\n"
"</table>\n"
"</div>\n"
"<TMPL_LOOP ERRORS>\n"
"    <hr>\n"
"    <TMPL_VAR ERROR>\n"
"</TMPL_LOOP>\n"
"</div>\n"
"</div>\n"
"</body>\n"
"<script>\n"
"<!--Format numbers-->\n"
"    function currency(n) { n = parseFloat(n); return isNaN(n) ? 0 : n.toFixed(2); }\n"
"    var elements = document.getElementsByClassName(\"money, text-right\");\n"
"    for (var i = 0; i < elements.length; i++)\n"
"        { elements[i].innerHTML = \"<TMPL_VAR PFX_SYMBOL>\" + currency(elements[i].innerHTML) + \"<TMPL_VAR SFX_SYMBOL>\"; }\n"
"</script>\n"
"</html>\n";

class Record : public std::map<std::wstring, std::wstring>
{
public:
    Record(){}
    ~Record(){}
    /* Access functions for LuaGlue (The required conversion between char and wchar_t is done through wxString.) */
    std::string get(const char* index) { return wxString((*this)[wxString(index).ToStdWstring()]).ToStdString(); }
    void set(const char* index, const char * val)
    {
        (*this)[wxString(index).ToStdWstring()] = wxString(val).ToStdWstring();
    }
    std::string GetDir(const char * val)
    {
        return mmex::getPathAttachment(wxString(val).ToStdWstring()).ToStdString();
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
        wxSQLite3Statement stmt;
        stmt = this->db_->PrepareStatement(r->SQLCONTENT);
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
        method("GetDir", &Record::GetDir).
        end().open().glue();

    bool skip_lua = r->LUACONTENT.IsEmpty();
    bool lua_status = state.doString(r->LUACONTENT.ToStdString());
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
                o[wxString(item.first).ToStdString()] = json::Number(v);
            }
            else
                o[wxString(item.first).ToStdString()] = json::String(wxString(item.second).ToStdString());
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
        std::stringstream ss;
        json::Writer::Write(jsoncontents, ss);
        report(L"JSONCONTENTS") = wxString(ss.str());
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
    const wxString tempDir = wxFileName(mmex::getReportIndex()).GetPathWithSep();
    const wxString resDir = mmex::GetResourceDir().GetPathWithSep();
    wxFileName::Mkdir(tempDir, 511, wxPATH_MKDIR_FULL);
    wxArrayString filesArray;
    wxDir::GetAllFiles(resDir, &filesArray);
    for (const auto& f : filesArray) {
        if (::wxFileExists(f)) {
            if (!::wxCopyFile(f, tempDir + wxFileName(f).GetFullName())) {
                wxLogError("Could not copy %s !", f);
            }
        }
        wxLogDebug("Coping file: %s to %s", f, tempDir + wxFileName(f).GetFullName());
    }
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
        wxString sql = wxString::Format(sqlColumns, table_name);
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
            body += wxString::Format("        <td class = \"money, text-right\"><TMPL_VAR \"%s\"></td>\n", col.first);
        else if (col.second == WXSQLITE_INTEGER)
            body += wxString::Format("        <td class = \"text-right\"><TMPL_VAR \"%s\"></td>\n", col.first);
        else
            body += wxString::Format("        <td><TMPL_VAR \"%s\"></td>\n", col.first);
    }
    return wxString::Format(HTT_CONTEINER, header, body);
}
