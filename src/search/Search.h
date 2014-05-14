/*******************************************************
 Copyright (C) 2014 Guan Lisheng (guanlisheng@gmail.com)

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
#pragma once
#include "db/DB_Table.h"

class SearchBase
{
public:
    SearchBase() {};
    SearchBase(wxSQLite3Database* db, const wxString& table): db_(db), table_(table) {};
    ~SearchBase() {};

protected:
    wxSQLite3Database* db_;
    wxString table_;

public:
    virtual wxString name() const { return this->table_; };
    // http://www.sqlite.org/fts3.html#section_3
    virtual json::Array search(const wxString& query)
    {
        wxString sql = wxString::Format("SELECT * FROM %s WHERE %s MATCH ?", this->name(), this->name());;
        json::Array a;
        try
        {
            wxSQLite3Statement stmt = db_->PrepareStatement(sql);
            stmt.Bind(1, query);

            wxSQLite3ResultSet q = stmt.ExecuteQuery();
            while (q.NextRow())
            {
                json::Object o;
                for (int i = 0; i < q.GetColumnCount(); ++i)
                {
                    int column_type = q.GetColumnType(i);
                    std::string column_name = q.GetColumnName(i).ToStdString();
                    switch (column_type)
                    {
                    case WXSQLITE_INTEGER:
                        o[column_name] = json::Number(q.GetInt(i));
                        break;
                    case WXSQLITE_FLOAT:
                        o[column_name] = json::Number(q.GetDouble(i));
                        break;
                    case WXSQLITE_TEXT:
                        o[column_name] = json::String(q.GetString(i).ToStdString());
                        break;
                    case WXSQLITE_BLOB:
                        o[column_name] = json::String(q.GetString(i).ToStdString());
                        break;
                    case WXSQLITE_NULL:
                    defaut:
                        break;
                    }
                }
                a.Insert(o);
            }
            q.Finalize();
        }
        catch (const wxSQLite3Exception& e)
        {
            wxLogError("%s: Exception %s", this->name(), e.GetMessage());
        }

        return a;
    }
};
