/*******************************************************
  Copyright (C) 2013 Nikolay
 ********************************************************/

#ifndef _MM_EX_EXPORT_H_
#define _MM_EX_EXPORT_H_

#include "mmOption.h"

class mmExportBase
{

public:
    mmExportBase(){}

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

    wxString getTransactionQIF();
    wxString getTransactionCSV();
    wxString getAccountHeaderQIF();
    wxString getCategoriesQIF();
    wxString getCategoriesCSV();

protected:
    int m_account_id;
    int m_transaction_id;
};


#endif 
