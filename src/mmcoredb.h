/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

//----------------------------------------------------------------------------
#ifndef _MM_EX_COREDB_H_
#define _MM_EX_COREDB_H_
//----------------------------------------------------------------------------
#include "mmtransaction.h"

//----------------------------------------------------------------------------
#include <wx/sharedptr.h>
//----------------------------------------------------------------------------
class wxSQLite3Database;
//----------------------------------------------------------------------------

/**
   mmCoreDB encapsulates most of the work in translating between
   the SQLite DB and the C++ datastructures used by MMEX
*/

class mmCoreDB
{
public:
    mmCoreDB(wxSharedPtr<wxSQLite3Database> db);
    ~mmCoreDB();

    // Global access point to the database.
    wxSharedPtr<wxSQLite3Database> db_;

    mmBankTransactionList bTransactionList_;
    bool displayDatabaseError_;
};

//----------------------------------------------------------------------------
#endif // _MM_EX_COREDB_H_
//----------------------------------------------------------------------------
