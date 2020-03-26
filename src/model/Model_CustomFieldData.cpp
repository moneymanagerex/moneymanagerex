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

#include "Model_CustomFieldData.h"
#include "Model_CustomField.h"
#include <wx/string.h>

Model_CustomFieldData::Model_CustomFieldData()
: Model<DB_Table_CUSTOMFIELDDATA_V1>()
{
}

Model_CustomFieldData::~Model_CustomFieldData()
{
}

/**
* Initialize the global Model_CustomFieldData table.
* Reset the Model_CustomFieldData table or create the table if it does not exist.
*/
Model_CustomFieldData& Model_CustomFieldData::instance(wxSQLite3Database* db)
{
    Model_CustomFieldData& ins = Singleton<Model_CustomFieldData>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);

    return ins;
}

/** Return the static instance of Model_CustomFieldData table */
Model_CustomFieldData& Model_CustomFieldData::instance()
{
    return Singleton<Model_CustomFieldData>::instance();
}

Model_CustomFieldData::Data* Model_CustomFieldData::get(int FieldID, int RefID)
{
    Model_CustomFieldData::Data_Set items = this->find(FIELDID(FieldID), REFID(RefID));
    if (!items.empty())
        return this->get(items[0].FIELDATADID, this->db_);
    return nullptr;
}

/** Return all CustomFieldData value*/
wxArrayString Model_CustomFieldData::allValue(const int FieldID)
{
    wxArrayString values;
    wxString PreviousValue;

    Model_CustomFieldData::Data_Set items = this->find(FIELDID(FieldID));
    std::sort(items.begin(), items.end(), SorterByCONTENT());

    for (const auto &item : items)
    {
        if (item.CONTENT != PreviousValue)
        {
            values.Add(item.CONTENT);
            PreviousValue = item.CONTENT;
        }
    }
    return values;
}

bool Model_CustomFieldData::RelocateAllData(const wxString& RefType, int OldRefId, int NewRefId)
{
    auto fields = Model_CustomField::instance().find(Model_CustomField::REFTYPE(RefType));

    this->Savepoint();
    for (auto &field : fields)
    {
        Data* data = Model_CustomFieldData::instance().get(field.FIELDID, OldRefId);
        if (data)
        {
            data->REFID = NewRefId;
            Model_CustomFieldData::instance().save(data);
        }
    }
    this->ReleaseSavepoint();
    return true;
}

bool Model_CustomFieldData::DeleteAllData(const wxString& RefType, int RefID)
{
    auto fields = Model_CustomField::instance().find(Model_CustomField::REFTYPE(RefType));

    this->Savepoint();
    for (auto &field : fields)
    {
        Data* data = Model_CustomFieldData::instance().get(field.FIELDID, RefID);
        if (data)
            Model_CustomFieldData::instance().remove(data->FIELDATADID);
    }
    this->ReleaseSavepoint();
    return true;
}
