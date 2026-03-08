/*******************************************************
 Copyright (C) 2016 Gabriele-V

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

#include "base/defs.h"
#include <wx/string.h>

#include "FieldModel.h"
#include "FieldValueModel.h"

FieldValueModel::FieldValueModel() :
    TableFactory<FieldValueTable, FieldValueData>()
{
}

FieldValueModel::~FieldValueModel()
{
}

// Initialize the global FieldValueModel table.
// Reset the FieldValueModel table or create the table if it does not exist.
FieldValueModel& FieldValueModel::instance(wxSQLite3Database* db)
{
    FieldValueModel& ins = Singleton<FieldValueModel>::instance();
    ins.reset_cache();
    ins.m_db = db;
    ins.ensure_table();

    return ins;
}

// Return the static instance of FieldValueModel table
FieldValueModel& FieldValueModel::instance()
{
    return Singleton<FieldValueModel>::instance();
}

FieldValueCol::REFID FieldValueModel::REFTYPEID(RefTypeN ref_type, int64 ref_id)
{
    return FieldValueCol::REFID(FieldValueData::encode_REFID(ref_type, ref_id));
}

bool FieldValueModel::purge_ref(RefTypeN ref_type, int64 ref_id)
{
    const DataA fv_a = find(
        FieldValueModel::REFTYPEID(ref_type, ref_id)
    );
    for (const Data& fv_d : fv_a) {
        purge_id(fv_d.m_id);
    }
    return true;
}

const FieldValueData* FieldValueModel::get_key_data_n(
    int64 field_id, RefTypeN ref_type, int64 ref_id
) {
    FieldValueModel::DataA fv_a = find(
        FieldValueCol::FIELDID(field_id),
        FieldValueModel::REFTYPEID(ref_type, ref_id)
    );
    if (!fv_a.empty())
        return get_id_data_n(fv_a[0].m_id);
    return nullptr;
}

std::map<int64, FieldValueModel::DataA> FieldValueModel::find_refType_mRefId(
    RefTypeN ref_type
) {
    std::map<int64, FieldValueModel::DataA> refId_dataA_m;
    for (const Data& fv_d : find_all()) {
        if (fv_d.m_ref_type.id_n() == ref_type.id_n())
            refId_dataA_m[fv_d.m_ref_id].push_back(fv_d);
    }
    return refId_dataA_m;
}

