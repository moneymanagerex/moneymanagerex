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

#pragma once

#include "model/TransactionModel.h"
#include "_ReportBase.h"
#include "dialog/TransactionFilterDialog.h"

class mmBankTransaction;

class TransactionsReport : public ReportBase
{
public:
    ~TransactionsReport();
    TransactionsReport(wxSharedPtr<TransactionFilterDialog>& transDialog);

    wxString getHTMLText();

private:
    void Run(wxSharedPtr<TransactionFilterDialog>& transDialog);
    TransactionModel::Full_Data_Set trans_;
    wxSharedPtr<TransactionFilterDialog> m_transDialog;
    bool showColumnById(int num);
    void displayTotals(const std::map<int64, double>& total, std::map<int64, double>& total_in_base_curr, int noOfCols);
    void UDFCFormatHelper(FieldModel::TYPE_ID type, int64 ref, wxString data, double val, int scale);

    mmHTMLBuilder hb;
    int m_noOfCols;
};

