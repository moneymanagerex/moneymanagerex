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

class TrxFilter
{
private:
    bool m_filter_date;
    bool m_filter_account;
    bool m_filter_payee;
    bool m_filter_category;
    mmDateN m_start_date_n, m_end_date_n;
    wxArrayInt64 m_account_id_a;
    wxArrayInt64 m_payee_id_a;
    wxArrayInt64 m_category_id_a;
    TrxModel::DataExtA m_trx_xa;

public:
    TrxFilter();

    // Filter setup methods
    void clear();
    void setDateRange(const mmDateRange2& date_range);
    void setDateRange(mmDateN start_date_n, mmDateN end_date_n);
    void setAccountList(wxSharedPtr<wxArrayString> accountList);
    void setPayeeList(const wxArrayInt64& payeeList);
    void setCategoryList(const wxArrayInt64 &categoryList);

    // Apply Filter methods
    template<class MODEL, class DATA = typename MODEL::Data>
    bool checkCategory(
        const DATA& d,
        const std::map<int64, typename MODEL::SplitDataA>& id_splitA_m
    );

    bool mmIsRecordMatches(
        const TrxData &tran,
        const std::map<int64, TrxSplitModel::DataA>& split
    );

    wxString getHTML();
};

