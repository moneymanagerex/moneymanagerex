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

class Record : public std::map<std::string, std::string>
{
public:
    Record(){}
    ~Record(){}
    std::string get(const char* index) { return (*this)[std::string(index)]; }
    void set(const char* index, const char * val) { (*this)[std::string(index)] = std::string(val); }
};

const std::vector<std::pair<Model_Report::STATUS, wxString> > Model_Report::STATUS_CHOICES = 
{
    std::make_pair(Model_Report::ACTIVE, wxTRANSLATE("ACTIVE"))
    , std::make_pair(Model_Report::INACTIVE, wxTRANSLATE("INACTIVE"))
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
    mm_html_template report(r->TEMPLATEPATH);

    report("REPORTID") = r->REPORTID;
    report("REPORTNAME") = r->REPORTNAME;
    report("GROUPNAME") = r->GROUPNAME;
    report("SQLCONTENTTYPE") = r->SQLCONTENT;
    report("LUACONTENT") = r->LUACONTENT;
    report("TEMPLATEPATH") = r->TEMPLATEPATH;

    loop_t contents;

    wxSQLite3Statement stmt = this->db_->PrepareStatement(r->SQLCONTENT);
    wxLogDebug(stmt.GetSQL());
    loop_t errors;
    row_t error;
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

        bool lua_status = !r->LUACONTENT.IsEmpty() && state.doString(r->LUACONTENT.ToStdString());
        if (!lua_status)
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

            if (lua_status) 
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
        if (lua_status) 
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

    report("CONTENTS") = contents;
    report("ERRORS") = errors;

    return wxString(report.Process());
}

wxString Model_Report::get_html(const Data& r) 
{ 
    return get_html(&r); 
}
