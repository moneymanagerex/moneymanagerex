/*******************************************************
  Copyright (C) 2013 Nikolay
 ********************************************************/

#include "export.h"

mmExportTransaction::mmExportTransaction(mmCoreDB* core, mmBankTransaction* pBankTransaction)
    : mmPrintableBase(core)
    , pBankTransaction_(pBankTransaction)
{}

mmExportTransaction::~mmExportTransaction()
{};

wxString mmExportTransaction::getTransactionQIF()
{
    core_->currencyList_.LoadBaseCurrencySettings();
    return "test";
}
