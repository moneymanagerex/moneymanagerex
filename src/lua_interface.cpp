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

#include "lua_interface.h"
#include "util.h"
#include "mmCurrencyFormatter.h"
#include "mmOption.h"
#include <wx/stdpaths.h>
#include "mmex_settings.h"

// Constructor: Initialises Lua when an instant is created.
mmHTMLBuilder* TLuaInterface::html_builder_ = 0;
TLuaInterface::TLuaInterface(mmHTMLBuilder* hb)
{
    this->html_builder_ = hb;
    MMEX_IniSettings* info_table = new MMEX_IniSettings(static_db_ptr(), true);

    g_static_currency_list = new mmCurrencyList(static_db_ptr());
    g_static_currency_list->SetInfoTable(info_table);
    g_static_currency_list->LoadCurrencies();

    lua_ = luaL_newstate();
    wxASSERT(lua_);

    luaopen_string(lua_);
    luaL_openlibs(lua_);
    Open_MMEX_Library();
}

// Destructor: Shuts down Lua on completion
TLuaInterface::~TLuaInterface()
{
    delete g_static_currency_list;
    lua_close(lua_);
}

// Passes Lua code in a string, to be run by Lua.
bool TLuaInterface::RunLuaCode(const wxString& lua_code)
{
    lua_result_ = luaL_loadstring(lua_, lua_code.ToUTF8());
    if (lua_result_)
    {
        this->html_builder_->addParaText(LuaErrorResult());
        return false;
    }

    lua_result_ = lua_pcall(lua_, 0, LUA_MULTRET, 0);
    if (lua_result_)
    {
        this->html_builder_->addParaText(LuaErrorResult());
        return false;
    }

    //Output Lua stack as html if not empty
    if (lua_gettop(lua_) > 1)
        this->html_builder_->addText(GetLuaString(lua_));

    return true;
}

// Passes a filename containing Lua code, to be run by Lua.
bool TLuaInterface::RunLuaFile(const wxString& lua_filename)
{
    lua_result_ = luaL_loadfile(lua_, lua_filename.ToUTF8());
    if (lua_result_)
    {
        this->html_builder_->addParaText(LuaErrorResult());
        return false;
    }

    lua_result_ = lua_pcall(lua_, 0, LUA_MULTRET, 0);
    if (lua_result_)
    {
        this->html_builder_->addParaText(LuaErrorResult());
        return false;
    }
    return true;
}

// Decode the error into a literal string
wxString TLuaInterface::LuaErrorResult()
{
    wxString error_result;
    if (lua_result_ != 0)
    {
        error_result = wxString::Format(_("Error: %s\n"), wxString::FromUTF8(lua_tostring(lua_, -1)));
        lua_pop(lua_, 1); // remove error message from stack
    }
    return error_result;
}

void TLuaInterface::ReportLuaError(lua_State* lua, wxString& error_str)
{
    lua_pushstring(lua, error_str.ToUTF8());
    lua_error(lua);
}

int TLuaInterface::GetLuaInteger(lua_State* lua)
{
    int iValue = lua_tointeger(lua, -1);
    lua_pop(lua, 1);  // remove the value from the stack

    return iValue;
}

double TLuaInterface::GetLuaDouble(lua_State* lua)
{
    double number = lua_tonumber(lua, -1);
    lua_pop(lua, 1);  // remove the value from the stack

    return number;
}

wxString TLuaInterface::GetLuaString(lua_State* lua)
{
    wxString sValue = wxString::FromUTF8(lua_tostring(lua, -1));
    lua_pop(lua, 1);  // remove the value from the stack

    return sValue;
}

bool TLuaInterface::OptionalParameter(lua_State* lua, int parameter_possition)
{
    bool has_option = lua_gettop(lua) > parameter_possition - 1;
    if (has_option)
    {
        lua_pop(lua, 1);    // remove arbitary value from the stack.
    }

    return has_option;
}

/******************************************************************************
 Lua extended functions, provided by c++ code:

 New functions needed to be registered with Lua using following values:
 (lua_, "lua_function_name", static_method_name_defined_in TLuaInterface);
 *****************************************************************************/
void TLuaInterface::Open_MMEX_Library()
{
    lua_register(lua_, "_",                 cpp2lua_GetTranslation);
    lua_register(lua_, "mmBell",            cpp2lua_Bell);
    lua_register(lua_, "mmMessageBox",      cpp2lua_MessageBox);
    lua_register(lua_, "mmGetSQLResultSet", cpp2lua_GetSQLResultSet);
    lua_register(lua_, "mmGetTableColumns", cpp2lua_GetTableColumns);
    lua_register(lua_, "mmGetSingleChoice", cpp2lua_GetSingleChoice);
    lua_register(lua_, "mmGetColumnChoice", cpp2lua_GetColumnChoice);
    lua_register(lua_, "mmGetTextFromUser", cpp2lua_GetTextFromUser);
    lua_register(lua_, "mmGetSiteContent",  cpp2lua_GetSiteContent);

    lua_register(lua_, "mmCurrencyFormat",     cpp2Lua_CurrencyFormat);
    lua_register(lua_, "mmBaseCurrencyFormat", cpp2Lua_BaseCurrencyFormat);
    lua_register(lua_, "mmDateFormat",         cpp2Lua_DateFormat);
    lua_register(lua_, "mmGetDocDir",          cpp2lua_GetDocDir);
    lua_register(lua_, "mmGetExeDir",          cpp2lua_GetExeDir);
    lua_register(lua_, "mmGetLuaDir",          cpp2lua_GetLuaDir);

    //html builder functions
    lua_register(lua_, "mmHTMLBuilder",        mmHTMLBuilderUni);
    lua_register(lua_, "mmHTMLTableCellMonth", mmHTMLTableCellMonth);
    lua_register(lua_, "mmHTMLTableCellInteger", mmHTMLTableCellInteger);
    lua_register(lua_, "mmHTMLTableCell", mmHTMLTableCell);
    lua_register(lua_, "mmHTMLReportHeader", mmHTMLReportHeader);
    lua_register(lua_, "mmHTMLStartTable", mmHTMLStartTable);
    lua_register(lua_, "mmHTMLEndTable", mmHTMLEndTable);
    lua_register(lua_, "mmHTMLStartTableRow", mmHTMLStartTableRow);
    lua_register(lua_, "mmHTMLEndTableRow", mmHTMLEndTableRow);
    lua_register(lua_, "mmHTMLTableHeaderCell", mmHTMLTableHeaderCell);
    lua_register(lua_, "mmHTMLaddText", mmHTMLaddText);
    lua_register(lua_, "mmHTMLhr", mmHTMLhr);
}

/******************************************************************************
 Lua Syntax: See comment above the defined function.

 Note: Generally all functions require a variable to accept a return value.
       Some functions can return multiple values, shown as xx[, yy]
       [, yy] represents an optional value.
 *****************************************************************************/

/*****************************************************************************
 _"translation_string" or _("translation_string")
 returns the translation string similar to wxWidgets
 *****************************************************************************/
int TLuaInterface::cpp2lua_GetTranslation(lua_State *lua)
{
    wxString sData = wxGetTranslation(GetLuaString(lua));
    lua_pushstring(lua, sData.ToUTF8());

    return 1;
}

/******************************************************************************
 mmBell() - For Windows, ring the system bell when available. No return value.
 *****************************************************************************/
int TLuaInterface::cpp2lua_Bell(lua_State* lua)
{
    int stack_pos;                  // prevent compiler
    stack_pos = lua_gettop(lua);    // warnings
    wxBell();

    return 0;
}

/******************************************************************************
 value = mmMessageBox(_("Message"), _("Caption"), Style)
 *****************************************************************************/
int TLuaInterface::cpp2lua_MessageBox(lua_State* lua)
{
    int mm_style = GetLuaInteger(lua);
    wxString caption = GetLuaString(lua);
    wxString message = GetLuaString(lua);

    double win_style;

    if      (mm_style == 1) win_style = wxOK;
    else if (mm_style == 2) win_style = wxYES_NO;
    else if (mm_style == 3) win_style = wxCANCEL;
    else if (mm_style == 13 || mm_style == 31) win_style = wxCANCEL + wxOK;
    else if (mm_style == 23 || mm_style == 32) win_style = wxCANCEL + wxYES_NO;
    else if (mm_style == 4) win_style = wxICON_EXCLAMATION;
    else if (mm_style == 5) win_style = wxICON_HAND;
    else if (mm_style == 6) win_style = wxICON_ERROR;
    else if (mm_style == 7) win_style = wxICON_QUESTION;
    else win_style = wxICON_INFORMATION;

    int ans = wxMessageBox(message, caption, win_style);

    if (ans == 2 || ans == 4) mm_style = 1; // OK or Yes
    else if (ans == 8)        mm_style = 2; // No
    else if (ans == 16)       mm_style = 3; // Cancel
    else mm_style = ans;

    lua_pushinteger(lua, mm_style);
    return 1;
}

/******************************************************************************
 Private Helper functions to access the database
 *****************************************************************************/
int TLuaInterface::SetSqlScriptRowCount(const wxString& sScript, int& iRowsCount, int& iError, wxString& sOutput)
{
    if (!mmDBWrapper::IsSelect(static_db_ptr().get(), sScript, iRowsCount))
    {
        iError = 3; //SQLITE_PERM;
        sOutput = _("Please, use read only access to DB in this function");
    }

    return iError;
}

int TLuaInterface::SetSqlRestltSet(const wxString& sScript, wxSQLite3ResultSet& sqlQueryResult, int& iError, wxString& sOutput)
{
    try
    {
        sqlQueryResult = static_db_ptr().get()->ExecuteQuery(sScript);
    }
    catch (const wxSQLite3Exception& e)
    {
        iError = e.GetErrorCode();
        sOutput = wxString::Format(_("Error: %s"), e.GetMessage());
    }
    return iError;
}

/******************************************************************************
 output[, error] = mmGetSQLResultSet("sql_script")
 global SQLResultSet = { { x[1][1] ... x[1][n] }, { x[2][1] ... x[2][n] } }
 global SQLResultSetX , SQLResultSetY ... spefify size of array
 *****************************************************************************/
int TLuaInterface::cpp2lua_GetSQLResultSet(lua_State *lua)
{
    wxString sScript = GetLuaString(lua);
    int iError  = 0; //SQLITE_OK;
    wxString sOutput = "";

    int iRowsCount = 0;

    if (SetSqlScriptRowCount(sScript, iRowsCount, iError, sOutput) == 0) //SQLITE_OK
    {
        wxSQLite3ResultSet sqlQueryResult;
        if (SetSqlRestltSet(sScript, sqlQueryResult, iError, sOutput) == 0)
        {
            lua_createtable(lua , iRowsCount, 0);  // push the main table T onto the stack

            int iColumnsCount = sqlQueryResult.GetColumnCount();
            for (int key = 1; key <= iRowsCount; key ++)
            {
                sqlQueryResult.NextRow();
                lua_createtable(lua , iColumnsCount, 0); // push a Row Table R onto the stack

                for (int i = 0; i < iColumnsCount; i++)
                {
                    wxString sData = sqlQueryResult.GetAsString(i);
                    // value is at -1, R is at -2
                    lua_pushstring(lua, sData.ToUTF8());
                    lua_rawseti(lua, -2, i+1);   // R[i] = Data

                    sOutput << sData << "\t|\t";
                }
                // R is at -1, T is at -2
                lua_rawseti(lua, -2, key); // T[key] = R

                sOutput.RemoveLast(3);
                sOutput << "<br>\n";
            }
            lua_setglobal(lua,"SQLResultSet");
            lua_pushinteger(lua, iRowsCount);
            lua_setglobal(lua, "SQLResultSetX");
            lua_pushinteger(lua, iColumnsCount);
            lua_setglobal(lua, "SQLResultSetY");
        }
    }

    lua_pushstring(lua, sOutput.ToUTF8());
    lua_pushinteger(lua, iError);

    return 2;
}

/******************************************************************************
 col_table, size = mmGetTableColumns("sql_script")
 col_table = { x[1][1] ... x[1][n] }
 *****************************************************************************/
int TLuaInterface::cpp2lua_GetTableColumns(lua_State* lua)
{
    int iError = 0;
    wxString sError = "";
    int iColumCount = -1;

    // if all OK, put array of columns on the stack from given SQL statement.
    wxSQLite3ResultSet sqlQueryResult;
    if (SetSqlRestltSet(GetLuaString(lua), sqlQueryResult, iError, sError) == 0)
    {
        iColumCount = sqlQueryResult.GetColumnCount();

        lua_createtable(lua, iColumCount, 0);
        for (int key = 0; key < iColumCount; ++key)
        {
            // create a table entry and place it on the stack
            lua_pushstring(lua, sqlQueryResult.GetColumnName(key).ToUTF8());
            lua_rawseti(lua, -2, key + 1);  // set the table Key
        }
    }
    else // incase of an error, ensure stack has 2 values.
    {
        lua_pushnil(lua);
    }

    lua_pushinteger(lua, iColumCount);    // put table size on stack. -1 if error

    return 2;
}

/******************************************************************************
 value = mmGetSingleChoice("message", "heading", choice_array)
 *****************************************************************************/
int TLuaInterface::cpp2lua_GetSingleChoice(lua_State* lua)
{
    wxArrayString data;
    if (lua_istable(lua, -1))
    {
        size_t len = lua_rawlen(lua , -1);  // get length of array
        size_t index = 1;
        while (index <= len)
        {
            lua_rawgeti(lua, -1, index);    // put value on the stack
            data.Add(wxString::FromUTF8(luaL_checkstring(lua, -1)));
            lua_pop(lua, 1);                // remove value from the stack
            index ++;
        }
        lua_pop(lua, 1);   // remove table from the stack
    }
    else
    {
        wxString error = _("mmGetTableChoice: Array not recognised");
        ReportLuaError(lua, error);
    }

    wxString heading = GetLuaString(lua);
    wxString message = GetLuaString(lua);

    wxASSERT (lua_gettop(lua) == 0);    // stack should be neutral

    wxString sValue = wxGetSingleChoice(message, heading, data);
    lua_pushstring(lua, sValue.ToUTF8());

    return 1;
}

/******************************************************************************
 value[, row_id] = mmColumnChoice("message", "heading", 2D_Array, column)
 *****************************************************************************/
int TLuaInterface::cpp2lua_GetColumnChoice(lua_State* lua)
{
    int column = GetLuaInteger(lua);

    size_t row_len = 0;
    wxArrayString data;
    if (lua_istable(lua, -1))
    {
        row_len = lua_rawlen(lua , -1);  // get length of row array
        size_t index = 1;
        while (index <= row_len)
        {
            lua_rawgeti(lua, -1, index);    // put col array on the stack
            if (lua_istable(lua, -1))
            {
                lua_rawgeti(lua, -1, column);    // put value on the stack
                wxString value = wxString::FromUTF8(luaL_checkstring(lua, -1));
                data.Add(value);
                lua_pop(lua, 1);        // remove value from the stack
            }
            else
            {
                wxString err_msg = _("mmGetColumnChoice: This is not a 2 dimentional array");
                ReportLuaError(lua, err_msg);   // This will not return
            }

            lua_pop(lua, 1);            // remove 2nd array from the stack
            index ++;
        }
        lua_pop(lua, 1);   // remove table from the stack
    }
    else
    {
        wxString error = _("mmGetTableChoice: Array not recognised");
        ReportLuaError(lua, error);     // This will not return
    }

    wxString heading = GetLuaString(lua);
    wxString message = GetLuaString(lua);

    int row_id = wxGetSingleChoiceIndex(message, heading, data);

    lua_pushstring(lua, row_id > -1 ? data[row_id].ToUTF8(): wxString("").ToUTF8());
    lua_pushinteger(lua, row_id > -1 ? row_id + 1: -1);

    return 2;
}

/******************************************************************************
 value = mmGetTextFromUser("field_name", "dialog_heading", "default_value")
 *****************************************************************************/
int TLuaInterface::cpp2lua_GetTextFromUser(lua_State* lua)
{
    wxString value = GetLuaString(lua);
    wxString header = GetLuaString(lua);
    wxString message = GetLuaString(lua);

    value = wxGetTextFromUser(message, header, value);
    lua_pushstring(lua, value.ToUTF8());

    return 1;
}

/******************************************************************************
 output[, error] = mmGetSiteContent("url_address")
 *****************************************************************************/
int TLuaInterface::cpp2lua_GetSiteContent(lua_State* lua)
{
    wxString sSite_address = GetLuaString(lua);

    wxString sOutput;
    int error = site_content(sSite_address, sOutput);

    lua_pushstring(lua, sOutput.ToUTF8());
    lua_pushinteger(lua, error);

    return 2;
}

/******************************************************************************
 Private Helper functions set the currency options
 *****************************************************************************/
void TLuaInterface::SetCurrencyFormat(lua_State* lua, double number, bool for_edit)
{
    wxString number_string;

    if (for_edit)
    {
         number_string = CurrencyFormatter::float2String(number);
    }
    else
    {
        number_string = CurrencyFormatter::float2Money(number);
    }

    lua_pushstring(lua, number_string.ToUTF8());
}

/******************************************************************************
 formatted_currency = mmBaseCurrencyFormat(value[, for_edit])
 *****************************************************************************/
int TLuaInterface::cpp2Lua_BaseCurrencyFormat(lua_State* lua)
{
    bool for_edit = OptionalParameter(lua, 2);
    double number = GetLuaDouble(lua);

    g_static_currency_list->LoadBaseCurrencySettings();
    SetCurrencyFormat(lua, number, for_edit);

    return 1;
}

/******************************************************************************
 formatted_currency = mmCurrencyFormat(currency_symbol, value[, for_edit])
 *****************************************************************************/
int TLuaInterface::cpp2Lua_CurrencyFormat(lua_State* lua)
{
    bool for_edit = OptionalParameter(lua, 3);
    double number = GetLuaDouble(lua);

    wxString currency_symbol = GetLuaString(lua).MakeUpper();
    g_static_currency_list->LoadCurrencySetting(currency_symbol);

    mmCurrency* pCurrency = g_static_currency_list->getCurrencySharedPtr(currency_symbol, true);
    if (pCurrency)
    {
        number = number * pCurrency->baseConv_;
    }
    else
    {
        wxString lua_error = wxString::Format(_("Base conversion for currency symbol: %s has not been set."), currency_symbol);
        ReportLuaError(lua, lua_error);
    }

    SetCurrencyFormat(lua, number, for_edit);

    return 1;
}

/******************************************************************************
 formatted_date = mmDateFormat(date_string)
 *****************************************************************************/
int TLuaInterface::cpp2Lua_DateFormat(lua_State* lua)
{
    wxString sDate = GetLuaString(lua);
    wxDateTime dt = wxDateTime::Now();
    if (!sDate.IsEmpty()) dt.ParseDate(sDate.GetData());
    sDate = dt.Format(mmOptions::instance().dateFormat_);
    lua_pushstring(lua, sDate.ToUTF8());
    return 1;
}

/******************************************************************************
 Private Helper function to set the directory settings on the return stack
 *****************************************************************************/
void TLuaInterface::SetDirSetting(lua_State* lua, wxString dir_setting)
{
    bool add_slash = ! OptionalParameter(lua, 1);
    dir_setting.Replace("\\", "/");
    if (add_slash)
    {
        dir_setting << "/";
    }
    lua_pushstring(lua, dir_setting.ToUTF8());
}

/******************************************************************************
 current_working_dir = mmGetDocDir([without_end_slash]])
 *****************************************************************************/
int TLuaInterface::cpp2lua_GetDocDir(lua_State* lua)
{
    static wxStandardPaths stdpath = wxStandardPaths();

    SetDirSetting(lua, stdpath.GetDocumentsDir());
    return 1;
}

/******************************************************************************
 exe_dir = mmGetExeDir([without_end_slash])
 *****************************************************************************/
int TLuaInterface::cpp2lua_GetExeDir(lua_State* lua)
{
    static wxStandardPaths stdpath = wxStandardPaths();
    wxFileName fn(stdpath.GetExecutablePath());

    SetDirSetting(lua, fn.GetPath());
    return 1;
}

/******************************************************************************
 lua_dir = mmGetLuaDir([without_end_slash])
 *****************************************************************************/
int TLuaInterface::cpp2lua_GetLuaDir(lua_State* lua)
{
    static wxStandardPaths stdpath = wxStandardPaths();
    wxFileName fn(stdpath.GetExecutablePath());

    const wxArrayString &dirs = fn.GetDirs();
    if (dirs.Last().Upper() == "BIN") // bin\mmex.exe
        fn.RemoveLastDir();

    SetDirSetting(lua, fn.GetPath() + "/lua");
    return 1;
}

/******************************************************************************
 html = mmHTMLBuilder("function[, value_1][, value_2][, value_3][, value_4]")
 *****************************************************************************/
int TLuaInterface::mmHTMLBuilderUni(lua_State* lua)
{
    wxString value_6;
    if (lua_gettop(lua) > 6 ) value_6 = GetLuaString(lua);

    wxString value_5;
    if (lua_gettop(lua) > 5 ) value_5 = GetLuaString(lua);

    wxString value_4;
    if (lua_gettop(lua) > 4 ) value_4 = GetLuaString(lua);

    wxString value_3;
    if (lua_gettop(lua) > 3 ) value_3 = GetLuaString(lua);

    wxString value_2;
    if (lua_gettop(lua) > 2 ) value_2 = GetLuaString(lua);

    wxString value_1;
    if (lua_gettop(lua) > 1 ) value_1 = GetLuaString(lua);

    wxString fn_name = GetLuaString(lua);
    wxString html_error = _("HTML_BUILDER: Syntax error function: %s");

    try
    {
        if (fn_name == "Init") html_builder_->init();
        else if (fn_name == "StartTable") html_builder_->startTable(value_1, value_2, value_3);
        else if (fn_name == "EndTable") html_builder_->endTable();
        else if (fn_name == "StartTableRow") html_builder_->startTableRow(value_1);
        else if (fn_name == "EndTableRow") html_builder_->endTableRow();
        else if (fn_name == "StartTableCell") html_builder_->startTableCell(value_1);
        else if (fn_name == "EndTableCell") html_builder_->endTableCell();
        else if (fn_name == "AddTableHeaderCell") html_builder_->addTableHeaderCell(value_1, !value_2.IsEmpty());
        else if (fn_name == "AddTableHeaderRow") html_builder_->addTableHeaderRow(value_1, wxAtoi(value_2));
        else if (fn_name == "AddTableCell") html_builder_->addTableCell(value_1, !value_2.IsEmpty()
            , !value_3.IsEmpty(), !value_4.IsEmpty(), value_5);
        else if (fn_name == "AddTableCellLink") html_builder_->addTableCellLink(value_1, value_2
            , !value_3.IsEmpty(), !value_4.IsEmpty()
            , !value_5.IsEmpty(), value_6);
        else if (fn_name == "AddRowSeparator") html_builder_->addRowSeparator(wxAtoi(value_1));
        else if (fn_name == "AddTotalRow") html_builder_->addTotalRow(value_1, wxAtoi(value_2), value_3);
        else if (fn_name == "AddDateNow") html_builder_->addDateNow();
        else if (fn_name == "AddParaText") html_builder_->addParaText(value_1);
        else if (fn_name == "AddLineBreak") html_builder_->addLineBreak();
        else if (fn_name == "AddHorizontalLine") html_builder_->addHorizontalLine(wxAtoi(value_1));
        else if (fn_name == "StartCenter") html_builder_->startCenter();
        else if (fn_name == "EndCenter") html_builder_->endCenter();
        else if (fn_name == "End") html_builder_->end();
        else
        {
            html_builder_->addParaText(wxString::Format(html_error, fn_name));
        }
    }
    catch (...)
    {
        html_builder_->addParaText(wxString::Format(html_error, fn_name));
    }

    return 0;
}

int TLuaInterface::mmHTMLReportHeader(lua_State* lua)
{
    wxString report_name = GetLuaString(lua);
    html_builder_->addHeader(1, report_name);
    return 0;
}

int TLuaInterface::mmHTMLTableCellMonth(lua_State* lua)
{
    html_builder_->addTableCellMonth(GetLuaInteger(lua)-1);
    return 0;
}

int TLuaInterface::mmHTMLTableCell(lua_State* lua)
{
    html_builder_->addTableCell(GetLuaString(lua));
    return 0;
}

int TLuaInterface::mmHTMLTableCellInteger(lua_State* lua)
{
    html_builder_->addTableCell(wxString()<<GetLuaInteger(lua), true);
    return 0;
}

int TLuaInterface::mmHTMLTableHeaderCell(lua_State* lua)
{
    html_builder_->addTableHeaderCell(GetLuaString(lua));
    return 0;
}

int TLuaInterface::mmHTMLStartTable(lua_State* lua)
{
    html_builder_->startTable(GetLuaString(lua));
    return 0;
}

int TLuaInterface::mmHTMLEndTable(lua_State* /*lua*/)
{
    html_builder_->endTable();
    return 0;
}

int TLuaInterface::mmHTMLStartTableRow(lua_State* /*lua*/)
{
    html_builder_->startTableRow();
    return 0;
}

int TLuaInterface::mmHTMLEndTableRow(lua_State* /*lua*/)
{
    html_builder_->endTableRow();
    return 0;
}

int TLuaInterface::mmHTMLaddText(lua_State* lua)
{
    html_builder_->addText(GetLuaString(lua));
    return 0;
}

int TLuaInterface::mmHTMLhr(lua_State* /*lua*/)
{
    html_builder_->addHorizontalLine(1);
    return 0;
}
