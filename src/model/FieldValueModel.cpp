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

const FieldValueData* FieldValueModel::get_key(int64 field_id, int64 ref_id)
{
    FieldValueModel::DataA fv_a = this->find(
        FieldValueCol::FIELDID(field_id),
        FieldValueCol::REFID(ref_id)
    );
    if (!fv_a.empty())
        return get_id_data_n(fv_a[0].FIELDATADID);
    return nullptr;
}

std::map<int64, FieldValueModel::DataA> FieldValueModel::get_all_id(const wxString& reftype)
{
    std::map<int64, FieldValueModel::DataA> id_data_a_m;
    for (const auto& field_d : FieldModel::instance().find(
        FieldCol::REFTYPE(reftype)
    )) {
        for (const auto& fv_d : find(
            FieldValueCol::FIELDID(field_d.m_id))
        ) {
            id_data_a_m[fv_d.REFID].push_back(fv_d);
        }
    }
    return id_data_a_m;
}

// Return all CustomFieldData value
wxArrayString FieldValueModel::allValue(const int64 field_id)
{
    wxArrayString values;
    wxString PreviousValue;

    FieldValueModel::DataA fv_a = find(
        FieldValueCol::FIELDID(field_id)
    );
    std::sort(fv_a.begin(), fv_a.end(), FieldValueData::SorterByCONTENT());

    for (const auto& fv_d : fv_a) {
        if (fv_d.CONTENT != PreviousValue) {
            values.Add(fv_d.CONTENT);
            PreviousValue = fv_d.CONTENT;
        }
    }
    return values;
}

bool FieldValueModel::DeleteAllData(const wxString& RefType, int64 ref_id)
{
    for (const auto& field_d : FieldModel::instance().find(
        FieldCol::REFTYPE(RefType)
    )) {
        const Data* fv_n = FieldValueModel::instance().get_key(field_d.m_id, ref_id);
        if (fv_n)
            FieldValueModel::instance().purge_id(fv_n->FIELDATADID);
    }
    return true;
}
