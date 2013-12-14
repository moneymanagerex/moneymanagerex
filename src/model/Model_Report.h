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

#ifndef MODEL_REPORT_H
#define MODEL_REPORT_H

#include "Model.h"
#include "db/DB_Table_Report_V1.h"
#include "reports/htmlbuilder.h"

class Model_Report : public Model_Mix<DB_Table_REPORT_V1>
{
public:
    Model_Report(): Model_Mix<DB_Table_REPORT_V1>()
    {
    };
    ~Model_Report() {};

public:
    /** Return the static instance of Model_Report table */
    static Model_Report& instance()
    {
        return Singleton<Model_Report>::instance();
    }

    /**
    * Initialize the global Model_Report table.
    * Reset the Model_Report table or create the table if it does not exist.
    */
    static Model_Report& instance(wxSQLite3Database* db)
    {
        Model_Report& ins = Singleton<Model_Report>::instance();
        ins.db_ = db;
        ins.destroy_cache();
        ins.ensure(db);

        return ins;
    }
public:
    wxString get_html(const Data* r)
    {
        mm_html_template report(r->TEMPLATEPATH);

        report("REPORTID") = r->REPORTID;
        report("REPORTNAME") = r->REPORTNAME;
        report("GROUPNAME") = r->GROUPNAME;
        report("CONTENTTYPE") = r->CONTENTTYPE;
        report("CONTENT") = r->CONTENT;
        report("TEMPLATEPATH") = r->TEMPLATEPATH;

        loop_t contents;

        if (r->CONTENTTYPE == "LUA")
        {
            // TODO
        }
        else // (r->CONTENTTYPE == "SQL")
        {
            wxSQLite3Statement stmt = this->db_->PrepareStatement(r->CONTENT);
            if (!stmt.IsReadOnly())
            {
                report("ERROR") = r->CONTENT + " will modify database! aborted!";
            }
            else
            {
                wxSQLite3ResultSet sqlQueryResult = stmt.ExecuteQuery();
                int columnCount = sqlQueryResult.GetColumnCount();

                loop_t columns;
                for (int i = 0; i < columnCount; ++ i)
                {
                    row_t row;
                    row("COLUMN") = sqlQueryResult.GetColumnName(i);

                    columns += row;
                }
                report("COLUMNS") = columns;

                while (sqlQueryResult.NextRow())
                {
                    row_t row;
                    for (int i = 0; i < columnCount; ++ i)
                    {
                        wxString column_name = sqlQueryResult.GetColumnName(i);
                        switch (sqlQueryResult.GetColumnType(i))
                        {
                        case WXSQLITE_INTEGER:
                            row(column_name.ToStdString()) = sqlQueryResult.GetInt(i);
                            break;
                        case WXSQLITE_FLOAT:
                            row(column_name.ToStdString()) = sqlQueryResult.GetDouble(i);
                            break;
                        default:
                            row(column_name.ToStdString()) = sqlQueryResult.GetAsString(i);
                            break;
                        }
                    }
                    contents += row;
                }
                sqlQueryResult.Finalize();
            }
        }

        report("CONTENTS") = contents;

        return wxString(report.Process());
    }
    wxString get_html(const Data& r) { return get_html(&r); }
};

#endif // 
