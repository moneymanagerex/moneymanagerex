/*******************************************************
  Copyright (C) 2013 Nikolay
 ********************************************************/

#ifndef _MM_EX_EXPORT_H_
#define _MM_EX_EXPORT_H_

#include "../mmcoredb.h"
#include "../mmOption.h"

class mmExportBase
{

public:
    mmExportBase(mmCoreDB* core): core_(core) {}

protected:
    const mmCoreDB* core_;
};

class mmExportTransaction : public mmExportBase
{

public:
    ~mmExportTransaction();
    mmExportTransaction(mmCoreDB* core);
	mmExportTransaction(mmCoreDB* core , int accountID);
    mmExportTransaction(mmCoreDB* core , mmBankTransaction* pBankTransaction);

    wxString getTransactionQIF();
    wxString getTransactionCSV();
    wxString getAccountHeaderQIF();
    wxString getCategoriesQIF();
    wxString getCategoriesCSV();

protected:
    mmBankTransaction* pBankTransaction_;
    int accountID_;
};


#endif 
