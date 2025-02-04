/*******************************************************
 Copyright (C) 2024 George Ef (george.a.ef@gmail.com)

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

#ifndef MM_EX_FUSED_TRANSACTION_H_
#define MM_EX_FUSED_TRANSACTION_H_

#include "Model.h"
#include "Model_Checking.h"
#include "Model_Billsdeposits.h"
#include "Model_Splittransaction.h"
#include "Model_Budgetsplittransaction.h"
#include "Model_Taglink.h"

class Fused_Transaction
{
public:
    // id represents TRANSID if !is_bill, or BDID otherwise
    typedef std::pair<int64 /* id */, bool /* is_bill */> IdB;

    // id represents TRANSID if repeat_num == 0, or BDID otherwise
    typedef std::pair<int64 /* id */, int /* repeat_num */> IdRepeat;

    typedef Model_Splittransaction::Data_Set Split_Data_Set;
    typedef Model_Budgetsplittransaction::Data_Set Budgetsplit_Data_Set;
    typedef Model_Taglink::Data_Set Taglink_Data_Set;

    static Model_Checking::Data execute_bill(const Model_Billsdeposits::Data& r, wxString date);
    static Model_Checking::Full_Data execute_bill_full(const Model_Billsdeposits::Data& r, wxString date);
    static Split_Data_Set execute_splits(const Budgetsplit_Data_Set& rs);

    struct Data: public Model_Checking::Data
    {
        Data();
        explicit Data(const Model_Checking::Data& t);
        explicit Data(const Model_Billsdeposits::Data& r);
        Data(const Model_Billsdeposits::Data& r, wxString date, int repeat_num);
        ~Data();

        int64 m_bdid;
        int m_repeat_num;
    };
    typedef std::vector<Data> Data_Set;

    struct Full_Data: public Model_Checking::Full_Data
    {
        explicit Full_Data(const Model_Checking::Data& t);
        Full_Data(const Model_Checking::Data& t,
            const std::map<int64 /* TRANSID */, Split_Data_Set>& splits,
            const std::map<int64 /* TRANSID */, Taglink_Data_Set>& tags
        );
        Full_Data(const Model_Billsdeposits::Data& r);
        Full_Data(const Model_Billsdeposits::Data& r, wxString date, int repeat_num);
        Full_Data(const Model_Billsdeposits::Data& r, wxString date, int repeat_num,
            const std::map<int64 /* BDID */, Budgetsplit_Data_Set>& budgetsplits,
            const std::map<int64 /* BDID */, Taglink_Data_Set>& tags
        );
        ~Full_Data();

        int64 m_bdid;
        int m_repeat_num;
    };
    typedef std::vector<Full_Data> Full_Data_Set;

    struct SorterByFUSEDTRANSID
    { 
        template<class DATA>
        bool operator()(const DATA& x, const DATA& y)
        {
            return (!x.m_repeat_num && y.m_repeat_num) ||
                (!x.m_repeat_num && !y.m_repeat_num && x.TRANSID < y.TRANSID) ||
                (x.m_repeat_num && y.m_repeat_num && x.m_bdid < y.m_bdid);
        }
    };

    struct SorterByFUSEDTRANSSN
    { 
        template<class DATA>
        bool operator()(const DATA& x, const DATA& y)
        {
            return (x.SN < y.SN);
        }
    };

    static void getEmptyData(Data &data, int64 account_id);
    static bool getFusedData(Data &data, IdB fused_id);
    static const Model_Splittransaction::Data_Set split(Data &r);
};

#endif
