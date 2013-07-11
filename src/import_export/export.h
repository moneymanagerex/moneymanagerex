/*******************************************************
  Copyright (C) 2013 Nikolay
 ********************************************************/

#ifndef _MM_EX_EXPORT_H_
#define _MM_EX_EXPORT_H_

#include "../reports/reportbase.h"

class mmExportTransaction : public mmPrintableBase
{

public:
    ~mmExportTransaction();
    mmExportTransaction(mmCoreDB* core
        , mmBankTransaction* pBankTransaction);

    wxString getTransactionQIF();

protected:
    wxString data_;
    mmBankTransaction* pBankTransaction_;
    mmCoreDB* core_;
};

class mmExportTransactionQIF: public mmExportTransaction
{
public:
    mmExportTransactionQIF(mmCoreDB* core, mmBankTransaction* pBankTransaction) 
        : mmExportTransaction(core, pBankTransaction)
    {}
};

#endif 
