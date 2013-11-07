/*************************************************************************
 Copyright (C) 2011 Stefano Giorgio

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
 *************************************************************************/

#include "customreportdisplay.h"
#include "lua_interface.h"
#include "util.h"
#include "dbwrapper.h"
#include "model/Model_Currency.h"

mmCustomReport::mmCustomReport(wxWindow* parent, wxSQLite3Database* db
, const wxString& reportTitle, const wxString& sScript, const wxString& sScriptType)
: parent_(parent)
, db_(db)
, reportTitle_(reportTitle)
, sScript_(sScript)
, sScriptType_(sScriptType)
{}

bool mmCustomReport::DisplaySQL_Results(mmHTMLBuilder& hb)
{
    hb.startCenter();
    hb.startTable();

    int rows = 0;
    bool bSelect = mmDBWrapper::IsSelect(this->db_, sScript_, rows);
    if (!bSelect)
    {
        wxMessageDialog msgDlg(parent_, _("SQL Query will modify your Data. Proceed??"), _("Warning"), wxYES_NO|wxICON_WARNING);
        if (msgDlg.ShowModal() != wxID_YES)
        {
            hb.endCenter();
            hb.addParaText(_("SQL query discarded by user"));
            hb.endTable();
            return false;
        }
    }

    wxSQLite3ResultSet sqlQueryResult;
    try
    {
        sqlQueryResult = this->db_->ExecuteQuery(sScript_);
    }
    catch(const wxSQLite3Exception& e)
    {
        wxSafeShowMessage("Error",e.GetMessage());

        hb.endCenter();
        hb.addParaText(wxString::Format(_("Error: %s"), e.GetMessage()));
        hb.endTable();
        return false;
    }

    // Use column info to determine if data is to be right justified.
    int columnCount = sqlQueryResult.GetColumnCount();
    bool *alignRight = new bool[columnCount];

    hb.startTableRow();
    for (int index = 0; index < columnCount; index ++)
    {
        hb.addTableHeaderCell(sqlQueryResult.GetColumnName(index));
        alignRight[index] = (sqlQueryResult.GetColumnType(index) == WXSQLITE_INTEGER
                            || sqlQueryResult.GetColumnType(index) == WXSQLITE_FLOAT);
    }
    hb.endTableRow();

    while (sqlQueryResult.NextRow())
    {
        hb.startTableRow();
        for (int index = 0; index < columnCount; index ++)
        {
            wxString displayData = sqlQueryResult.GetAsString(index);
            if ( sqlQueryResult.GetColumnType(index) == WXSQLITE_FLOAT )
            {
                displayData = Model_Currency::toString(sqlQueryResult.GetDouble(index));
            }

            //Right justify numeric data.
            hb.addTableCell( displayData, alignRight[index]);
        }
        hb.endTableRow();
    }

    delete [] alignRight;
    sqlQueryResult.Finalize();


    if (!bSelect)
         hb.addHeader(2, _("Dababase updated succesfully"));

    hb.endCenter();
    hb.endTable();
    return true;
}

void mmCustomReport::DisplayLua_Results(mmHTMLBuilder& hb)
{
    TLuaInterface lua_core(&hb);
    lua_core.RunLuaCode(sScript_);
}

wxString mmCustomReport::getHTMLText()
{
    mmHTMLBuilder hb;
    hb.clear();

    if (sScriptType_ == "SQL")
    {
        DisplaySQL_Results(hb);
    }
    else if (sScriptType_ == "LUA")
    {
        DisplayLua_Results(hb);
    }
    else
    {
        hb.addParaText(_("Cannot execute unknown Script type"));
    }

    return hb.getHTMLText();
}
