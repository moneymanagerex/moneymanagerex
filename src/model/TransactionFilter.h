/*******************************************************
Copyright (C) 2021-2022 Mark Whalley (mark@ipx.co.uk)

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

#include "util/mmDateRange2.h"
#include "_all.h"

class TransactionFilter
{
public:
    TransactionFilter();

    // Filter setup methods
    void clear();
    void setDateRange(const mmDateRange2& date_range);
    void setDateRange(wxDateTime startDate, wxDateTime endDate);
    void setAccountList(wxSharedPtr<wxArrayString> accountList);
    void setPayeeList(const wxArrayInt64& payeeList);
    void setCategoryList(const wxArrayInt64 &categoryList);

    // Apply Filter methods
    template<class MODEL, class DATA = typename MODEL::Data>
    bool checkCategory(
        const DATA& tran,
        const std::map<int64, typename MODEL::Split_Data_Set> & splits
    );
    bool mmIsRecordMatches(
        const TransactionModel::Data &tran,
        const std::map<int64, TransactionSplitModel::Data_Set>& split
    );

    wxString getHTML();

private:
    bool m_filter_date;
    bool m_filter_account;
    bool m_filter_payee;
    bool m_filter_category;
    wxString m_start_date, m_end_date;
    wxArrayInt64 m_account_a;
    wxArrayInt64 m_payee_a;
    wxArrayInt64 m_category_a;
    TransactionModel::Full_Data_Set m_trans;
};

