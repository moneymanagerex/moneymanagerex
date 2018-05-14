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

#ifndef MM_EX_EXPORT_H_
#define MM_EX_EXPORT_H_

#include "Model_Checking.h"

class mmExportTransaction
{

public:
    virtual ~mmExportTransaction();
    mmExportTransaction();

    static const wxString getTransactionQIF(const Model_Checking::Full_Data & tran, const wxString& dateMask, bool reverce = false);
    static const wxString getAccountHeaderQIF(int accountID);
    static const wxString getCategoriesQIF();
    static const std::unordered_map <wxString, int> m_QIFaccountTypes;
    static const wxString qif_acc_type(const wxString& mmex_type);
    static const wxString mm_acc_type(const wxString& qif_type);
};

#endif 
