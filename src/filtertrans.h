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

#ifndef FILTERTRANS_H_
#define FILTERTRANS_H_

#include "model/allmodel.h"

class mmFilterTransactions
{

public:
    // Constructors
    mmFilterTransactions();

    void clear();

    // Filter setup methods
    void setDateRange(wxDateTime startDate, wxDateTime endDate);
    void setAccountList(wxSharedPtr<wxArrayString> accountList);
    void setPayeeList(const wxArrayInt payeeList);
    void setCategoryList(const std::vector<std::pair<int, int>> &categoryList);
 
    // Apply Filter methods
    template<class MODEL, class DATA = typename MODEL::Data>
    bool checkCategory(const DATA& tran, const std::map<int, typename MODEL::Split_Data_Set> & splits);
    bool mmIsRecordMatches(const Model_Checking::Data &tran
        , const std::map<int, Model_Splittransaction::Data_Set>& split);

    wxString getHTML();

private:
    // date range
    bool _dateFilter;
    wxString _startDate, _endDate;
    // account
    bool _accountFilter;
    wxArrayInt _accountList; 
    // payee
    bool _payeeFilter;
    wxArrayInt _payeeList; 
    // category
    bool _categoryFilter;
    std::vector<std::pair<int, int>> _categoryList;
 
    Model_Checking::Full_Data_Set _trans;

};

#endif
// FILTERTRANS_H_
