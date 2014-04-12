/*******************************************************
Copyright (C) 2013 Nikolay

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

#ifndef _MM_EX_EXPORT_H_
#define _MM_EX_EXPORT_H_

#include "mmOption.h"
#include "model/Model_Splittransaction.h"
#include "model/Model_Checking.h"

class mmExportBase
{

public:
    mmExportBase() : m_transaction_id(0) {}
    mmExportBase(int transactionID) : m_transaction_id(transactionID) {}

protected:
    int m_transaction_id;
};

class mmExportTransaction : public mmExportBase
{

public:
    ~mmExportTransaction();
    mmExportTransaction();
    mmExportTransaction(int accountID);
    mmExportTransaction(int transactionID, int accountID);

    static wxString getTransactionQIF(const Model_Checking::Full_Data & tran, int accountID);
    static wxString getTransactionCSV(const Model_Checking::Full_Data & tran, int accountID);
    wxString getAccountHeaderQIF();
    wxString getCategoriesQIF();
    wxString getCategoriesCSV();

protected:
    int m_account_id;
};


#endif 
