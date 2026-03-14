/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

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

#include "base/defs.h"

#include "table/TrxSplitTable.h"
#include "data/_DataEnum.h"
#include "data/TrxSplitData.h"

#include "_ModelBase.h"
#include "CurrencyModel.h"

struct Split
{
    int64        m_category_id;
    double       m_amount;
    wxString     m_notes;
    wxArrayInt64 m_tag_id_a;
};

class TrxSplitModel : public TableFactory<TrxSplitTable, TrxSplitData>
{
public:
    static const RefTypeN s_ref_type;

public:
    TrxSplitModel();
    ~TrxSplitModel();

public:
    static TrxSplitModel& instance(wxSQLite3Database* db);
    static TrxSplitModel& instance();

public:
    // override
    bool purge_id(int64 tp_id) override;

    auto find_all_mTrxId() -> std::map<int64, DataA>;

    auto get_total(const DataA& tp_a) -> double;
    auto get_total(const std::vector<Split>& split_a) -> double;
    auto get_tooltip(
        const std::vector<Split>& split_a, const CurrencyData* currency_n
    ) -> const wxString;

    int  update_trx(int64 trx_id, DataA& src_tp_a);
    int  update_trx(int64 trx_id, const std::vector<Split>& split_a);
};
