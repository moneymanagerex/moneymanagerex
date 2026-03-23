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
#include "util/mmSingleton.h"
#include "table/_TableFactory.h"
#include "data/AssetData.h"

#include "CurrencyModel.h"

class AssetModel : public TableFactory<AssetTable, AssetData>
{
public:
    static const RefTypeN s_ref_type;

public:
    AssetModel();
    ~AssetModel();

public:
    static AssetModel& instance(wxSQLite3Database* db);
    static AssetModel& instance();

    static AssetCol::ASSETTYPE ASSETTYPE(OP op, AssetType type);
    static AssetCol::STARTDATE STARTDATE(OP op, const mmDate& date);

public:
    // FIXME: add purge_id() to remove AttachmentData owned by AssetData

    // lookup for given Data
    auto get_data_value_date(const Data& asset_d, const mmDate& date) -> const std::pair<double, double>;
    auto get_data_value(const Data& asset_d) -> const std::pair<double, double>;

    // lookup for given id
    auto get_id_name(int64 asset_id) -> const wxString;

    // lookup for all Data
    auto find_all_balance() -> double;
};
