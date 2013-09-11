/*************************************************************************
 Copyright (C) 2009 VaDiM
 Copyright (C) 2013 Stefano Giorgio

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

#pragma once
//#define UTIL_TEST_INCLUDED_IN_BUILD

//#define DBWRAPPER_TEST_INCLUDED_IN_BUILD
//#define MMEX_LUA_TEST_INCLUDED_IN_BUILD

#define NEW_CLASSES_TEST_INCLUDED_IN_BUILD

#include <wx/filename.h>
//----------------------------------------------------------------------------
#include "dbwrapper.h"
#include "mmcoredb.h"
#include <iostream>
//----------------------------------------------------------------------------

const wxString g_BudgetYear   = "2009";
const wxString g_CategName    = "new category";
const wxString g_SubCategName = "new subcategory";
const wxString g_CurrencyName = "US Dollar";
const wxString g_PayeeName    = "Payee #1";

const wxString g_TransType_deposit    = "Deposit";
const wxString g_TransType_withdrawal = "Withdrawal";
const wxString g_status_reconciled    = "R";
const wxString g_status_void          = "V";
//----------------------------------------------------------------------------

/*****************************************************************************************
 The test platform will create and test a new database.

 This class is used to remove the temporary database on completion.
 *****************************************************************************************/
class Cleanup
{
public:
    Cleanup(wxString filename, bool pause = false);
    ~Cleanup();
private:
    wxString dbFileName_;
    bool pause_;
};


/*****************************************************************************************
 Create a single access point for the ini_settings database
 *****************************************************************************************/
// Returns the user's current working directory for the new inidb database.
// wxString getIniDbPpath();

std::shared_ptr<wxSQLite3Database> get_pInidb();

/*****************************************************************************************
 Create a single access point for the database, Remove database on completion.
 *****************************************************************************************/
// Returns the user's current working directory for the new database.
// wxString getDbPath();
std::shared_ptr<wxSQLite3Database> get_pDb();

// Create a single access point for the main database, stored in memory.
std::shared_ptr<mmCoreDB> pDb_core();

// Deprecated: Use displayTimeTaken(...) using wxStopWatch
void const displayTimeTaken(const wxString& msg, const wxDateTime start_time);
// Display a message string displaying time taken on a console
void const displayTimeTaken(const wxString& msg, const wxStopWatch& start_time);

void display_STD_IO_separation_line();
void display_STD_IO_line();

// End of file
