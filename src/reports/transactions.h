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

#ifndef MM_EX_REPORTTRANSACT_H_
#define MM_EX_REPORTTRANSACT_H_

#include "reportbase.h"
#include "filtertransdialog.h"
#include "model/Model_Checking.h"

class mmBankTransaction;

class mmReportTransactions : public mmPrintableBase
{
public:
    ~mmReportTransactions();
    mmReportTransactions(wxSharedPtr<mmFilterTransactionsDialog>& transDialog);

    wxString getHTMLText();

private:
    void Run(wxSharedPtr<mmFilterTransactionsDialog>& transDialog);
    Model_Checking::Full_Data_Set trans_;
    wxSharedPtr<mmFilterTransactionsDialog> m_transDialog;
    bool showColumnById(int num);
    void displayTotals(std::map<int, double> total, std::map<int, double> total_in_base_curr, int noOfCols);
    void UDFCFormatHelper(Model_CustomField::FIELDTYPE type, int ref, wxString data, double val, int scale);

    mmHTMLBuilder hb;
    int m_noOfCols;
};

#endif // MM_EX_REPORTTRANSACT_H_
