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

#include "paths.h"
#include "Model_Report.h"
#include "reports/htmlbuilder.h"
#include "LuaGlue/LuaGlue.h"

static const wxString HTT_CONTEINER =
"<!DOCTYPE html>"
"<h3><TMPL_VAR REPORTNAME></h3>\n"
"<TMPL_VAR TODAY><hr>\n"
"<table cellspacing='1' width='95%%'>\n"
"    <tr bgcolor='#D5D6DE'>\n"
"%s"
"    </tr>\n"
"    <TMPL_LOOP NAME=CONTENTS>\n"
"    <TMPL_IF __ODD__>\n"
"        <tr>\n"
"    <TMPL_ELSE>\n"
"        <tr bgcolor='#E1EDFB'>\n"
"    </TMPL_IF>\n"
"%s"
"        </tr>\n"
"    </TMPL_LOOP>\n"
"</table>\n"
"<TMPL_LOOP ERRORS>\n"
"    <hr>\n"
"    <TMPL_VAR ERROR>\n"
"</TMPL_LOOP>\n";

class Record : public std::map<std::string, std::string>
{
public:
    Record(){}
    ~Record(){}
    std::string get(const char* index) { return (*this)[std::string(index)]; }
    void set(const char* index, const char * val) { (*this)[std::string(index)] = std::string(val); }
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
    wxString fNameCss = wxFileName::CreateTempFileName(wxGetEmptyString());
    wxString fNameJs = wxFileName::CreateTempFileName(wxGetEmptyString());
    wxString fNameTemplate = wxFileName::CreateTempFileName(wxGetEmptyString());
    wxFileOutputStream output(fNameTemplate);
    wxTextOutputStream text(output);
    wxString template_text = r->TEMPLATECONTENT;
    if (template_text.Replace("<TMPL_INCLUDE NAME=\"master.css\">", wxString::Format("<TMPL_INCLUDE NAME=\"%s\">", fNameCss)) > 0)
    {
        wxFileName fn(mmex::getPathResource(mmex::EResFile::CSS_FILE));
        wxString filename = "memory:" + fn.GetFullName();
        wxFileSystem fs;
        wxFSFile *pFile = fs.OpenFile(filename);
        if (pFile)
        {
            wxFileOutputStream output(fNameCss);
            output.Write(*pFile->GetStream());
            output.Close();
            delete pFile;
        }
    }
    if (template_text.Replace("<TMPL_INCLUDE NAME=\"Chart.js\">", wxString::Format("<TMPL_INCLUDE NAME=\"%s\">", fNameJs)) > 0)
    {
        wxFileName fn(mmex::getPathResource(mmex::EResFile::JS_FILE));
        wxString filename = "memory:" + fn.GetFullName();
        wxFileSystem fs;
        wxFSFile *pFile = fs.OpenFile(filename);
        if (pFile)
        {
            wxFileOutputStream output(fNameJs);
            output.Write(*pFile->GetStream());
            output.Close();
            delete pFile;
        }
    }
    text << template_text;
    output.Close();

    mm_html_template report(fNameTemplate);
    report("REPORTID") = r->REPORTID;
    report("REPORTNAME") = r->REPORTNAME;
    report("GROUPNAME") = r->GROUPNAME;
    report("SQLCONTENT") = r->SQLCONTENT;
    report("LUACONTENT") = r->LUACONTENT;

    loop_t contents;

    loop_t errors;
    row_t error;

    if (!this->db_->CheckSyntax(r->SQLCONTENT))
    {
        error("ERROR") = wxString("Syntax Error : ") + r->SQLCONTENT;
        errors += error;
    }
    else
    {
        wxSQLite3Statement stmt = this->db_->PrepareStatement(r->SQLCONTENT);
        wxLogDebug("%s", stmt.GetSQL());

        if (!stmt.IsReadOnly())
        {
            error("ERROR") = r->SQLCONTENT + " will modify database! aborted!";
            errors += error;
        }
        else
        {
            wxSQLite3ResultSet q = stmt.ExecuteQuery();
            int columnCount = q.GetColumnCount();

            loop_t columns;
            for (int i = 0; i < columnCount; ++ i)
            {
                row_t row;
                row("COLUMN") = q.GetColumnName(i);

                columns += row;
            }
            report("COLUMNS") = columns;

            LuaGlue state;
            state.
                Class<Record>("Record").
                ctor("new").
                method("get", &Record::get).
                method("set", &Record::set).
                end().open().glue();

            bool skip_lua = r->LUACONTENT.IsEmpty();
            bool lua_status = state.doString(r->LUACONTENT.ToStdString());
            if (!skip_lua && !lua_status)
            {
                error("ERROR") = "failed to doString : " + r->LUACONTENT + " err: " + state.lastError();
                errors += error;
            }

            while (q.NextRow())
            {
                Record r;
                for (int i = 0; i < columnCount; ++ i)
                {
                    wxString column_name = q.GetColumnName(i);
                    r[column_name.ToStdString()] = q.GetAsString(i);
                }

                if (lua_status && !skip_lua) 
                {
                    try
                    {
                        state.invokeVoidFunction("handle_record", &r);
                    }
                    catch (const std::runtime_error& e)
                    {
                        error("ERROR") = std::string("failed to call handle_record : ") + e.what();
                        errors += error;
                    }
                    catch (const std::exception& e)
                    {
                        error("ERROR") = std::string("failed to call handle_record : ") + e.what();
                        errors += error;
                    }
                    catch (...)
                    {
                        error("ERROR") = "failed to call handle_record ";
                        errors += error;
                    }
                }
                row_t row;
                for (const auto& item : r) row(item.first) = item.second;
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
                    error("ERROR") = std::string("failed to call complete: ") + e.what();
                    errors += error;
                }
                catch (const std::exception& e)
                {
                    error("ERROR") = std::string("failed to call complete: ") + e.what();
                    errors += error;
                }
                catch (...)
                {
                    error("ERROR") = "failed to call complete";
                    errors += error;
                }
            }
            for (const auto& item : result) report(item.first) = item.second;
        }
    }

    report("CONTENTS") = contents;
    report("ERRORS") = errors;

    const wxString out = wxString(report.Process());
    wxRemoveFile(fNameTemplate);
    wxRemoveFile(fNameCss);
    wxRemoveFile(fNameJs);
    return out;
}

wxString Model_Report::get_html(const Data& r) 
{ 
    return get_html(&r); 
}

bool Model_Report::CheckSyntax(const wxString& sql)
{
    return this->db_->CheckSyntax(sql);
}

bool Model_Report::getColumns(const wxString& sql, std::vector<std::pair<wxString, int> > &colHeaders)
{
    if (!this->db_->CheckSyntax(sql)) return false;
    wxSQLite3Statement stmt = this->db_->PrepareStatement(sql);
    if (!stmt.IsReadOnly()) return false;

    wxSQLite3ResultSet q = stmt.ExecuteQuery();
    int columnCount = q.GetColumnCount();

    for (int i = 0; i < columnCount; ++i)
    {
        std::pair<wxString, int> col_and_type;
        col_and_type.second = q.GetColumnType(i);
        col_and_type.first = q.GetColumnName(i);

        colHeaders.push_back(col_and_type);
    }
    return true;
}

wxString Model_Report::getTemplate(const wxString& sql)
{
    wxString body, header;
    std::vector<std::pair<wxString, int> > colHeaders;
    this->getColumns(sql, colHeaders);
    for (const auto& col : colHeaders)
    {
        header += wxString::Format("        <th>%s</th>\n", col.first);
        if (col.second == 1)
            body += wxString::Format("        <td nowrap align='right'><TMPL_VAR \"%s\"></td>\n", col.first);
        else
            body += wxString::Format("        <td><TMPL_VAR \"%s\"></td>\n", col.first);
    }
    return wxString::Format(HTT_CONTEINER, header, body);
}

bool Model_Report::getSqlQuery(/*in*/ const wxString& sql, /*out*/ std::vector <std::vector <wxString> > &sqlQueryData)
{
    if (!this->db_->CheckSyntax(sql)) return false;
    wxSQLite3Statement stmt = this->db_->PrepareStatement(sql);
    if (!stmt.IsReadOnly()) return false;

    wxSQLite3ResultSet q = stmt.ExecuteQuery();
    int columnCount = q.GetColumnCount();

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
    const wxString sqlTables = "SELECT name FROM sqlite_master WHERE type = \"table\" ORDER BY name;";
    const wxString sqlColumns = "PRAGMA table_info(%s);";
    sqlTableInfo.clear();

    // Get a list of the database tables
    wxSQLite3Statement stmtTables = this->db_->PrepareStatement(sqlTables);
    wxSQLite3ResultSet qTables = stmtTables.ExecuteQuery();
    while (qTables.NextRow())
    {
        const wxString table_name = qTables.GetAsString(0);

        // Get a list of the table columns
        wxString sql = wxString::Format(sqlColumns, table_name);
        wxSQLite3Statement stmtColumns = this->db_->PrepareStatement(sql);
        wxSQLite3ResultSet qColumns = stmtColumns.ExecuteQuery();
        wxArrayString column_names;
        while (qColumns.NextRow())
            column_names.push_back(qColumns.GetAsString(1));

        std::pair<wxString, wxArrayString> table_columns;
        table_columns.first = table_name;
        table_columns.second = column_names;
        sqlTableInfo.push_back(table_columns);
    }
}
