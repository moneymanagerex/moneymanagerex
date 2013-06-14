/*************************************************************************
 Copyright (C) 2012 Stefano Giorgio & Nikolay

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

#ifndef _MM_EX_LUA_INTERFACE_H_
#define _MM_EX_LUA_INTERFACE_H_

#include "defs.h"
#include <iostream>
#include "mmcoredb.h"
#include "htmlbuilder.h"

extern "C" {
#include "lualib.h"
#include "lauxlib.h"
}

static mmCurrencyList* g_static_currency_list;

class TLuaInterface
{
public:
    struct Scriptable_HTMLBuilder
    {
       mmHTMLBuilder* hb_; 
       Scriptable_HTMLBuilder(lua_State *L): hb_(0)
       {}
       ~Scriptable_HTMLBuilder()
       {};
    };

public:
    TLuaInterface(mmHTMLBuilder* hb);
    ~TLuaInterface();
    
    bool RunLuaCode(const wxString& lua_code);
    bool RunLuaFile(const wxString& lua_filename);

public:
    static mmHTMLBuilder* html_builder_;

private:
    lua_State* lua_;
    int lua_result_;
    wxString LuaErrorResult();

    static int SetSqlScriptRowCount(const wxString& sScript, int& iRowsCount, int& iError, wxString& sOutput);
    static int SetSqlRestltSet(const wxString& sScript, wxSQLite3ResultSet& sqlQueryResult, int& iError, wxString& sOutput);

    static void ReportLuaError(lua_State* lua, wxString& error_str);
    static int GetLuaInteger(lua_State* lua);
    static double GetLuaDouble(lua_State* lua);
    static wxString GetLuaString(lua_State* lua);
    static bool OptionalParameter(lua_State* lua, int parameter_possition);
    static void SetCurrencyFormat(lua_State* lua, double number, bool for_edit);
    static void SetDirSetting(lua_State* lua, wxString dir_setting);

    /**************************************************************************
    Lua function extensions provided by c++ code:

    All functions are to be defined in this class, and
    added to the Open_MMEX_Library() function, defined in this class.
    **************************************************************************/
    // List of mmex_library functions
    void Open_MMEX_Library();
    // All Lua functions provided by C++ for MMEX
    static int cpp2lua_Bell(lua_State* lua);
    static int cpp2lua_MessageBox(lua_State* lua);
    static int cpp2lua_GetSQLResultSet(lua_State* lua);
    static int cpp2lua_GetTableColumns(lua_State* lua);
    static int cpp2lua_GetSingleChoice(lua_State* lua);
    static int cpp2lua_GetColumnChoice(lua_State* lua);
    static int cpp2lua_GetTextFromUser(lua_State* lua);
    static int cpp2lua_GetTranslation(lua_State* lua);
    static int cpp2lua_GetSiteContent(lua_State* lua);

    static int cpp2Lua_BaseCurrencyFormat(lua_State* lua);
    static int cpp2Lua_CurrencyFormat(lua_State* lua);
    static int cpp2Lua_DateFormat(lua_State* lua);
    static int cpp2lua_GetDocDir(lua_State* lua);
    static int cpp2lua_GetExeDir(lua_State* lua);
    static int cpp2lua_GetLuaDir(lua_State* lua);

    static int mmHTMLBuilderUni(lua_State* lua);
    static int mmHTMLReportHeader(lua_State* lua);
    static int mmHTMLStartTable(lua_State* lua);
    static int mmHTMLEndTable(lua_State* lua);
    static int mmHTMLStartTableRow(lua_State* lua);
    static int mmHTMLEndTableRow(lua_State* lua);
    static int mmHTMLTableCellMonth(lua_State* lua);
    static int mmHTMLTableCellInteger(lua_State* lua);
    static int mmHTMLTableCell(lua_State* lua);
    static int mmHTMLTableHeaderCell(lua_State* lua);
    static int mmHTMLaddText(lua_State* lua);
    static int mmHTMLhr(lua_State* lua);
};

#endif // _MM_EX_LUA_INTERFACE_H_
