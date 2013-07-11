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
    virtual wxString version() { return "$Rev: 4335 $"; }

protected:
    const mmCoreDB* core_;
};

class mmExportTransaction : public mmExportBase
{

public:
    ~mmExportTransaction();
    mmExportTransaction(mmCoreDB* core
        , mmBankTransaction* pBankTransaction);

    wxString getTransactionQIF();
    wxString getTransactionCSV();

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

class mmExportTransactionCSV: public mmExportTransaction
{
public:
    mmExportTransactionCSV(mmCoreDB* core, mmBankTransaction* pBankTransaction) 
        : mmExportTransaction(core, pBankTransaction)
    {}
};

#endif 
