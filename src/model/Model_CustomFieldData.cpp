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
: Model<DB_Table_CUSTOMFIELDDATA>()
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
    return (Model_CustomFieldData::Data*)nullptr;
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

bool Model_CustomFieldData::DeleteAllData(const wxString& RefType, int RefID)
{
    const auto& fields = Model_CustomField::instance().find(Model_CustomField::DB_Table_CUSTOMFIELD::REFTYPE(RefType));

    this->Savepoint();
    for (const auto& field : fields)
    {
        Data* data = Model_CustomFieldData::instance().get(field.FIELDID, RefID);
        if (data)
            Model_CustomFieldData::instance().remove(data->FIELDATADID);
    }
    this->ReleaseSavepoint();
    return true;
}

/** Return a dataset **/
std::map<int, Model_CustomFieldData::Data_Set> Model_CustomFieldData::get_all(Model_Attachment::REFTYPE reftype)
{
    const wxString& reftype_desc = Model_Attachment::reftype_desc(reftype);
    Model_CustomField::Data_Set data_set = Model_CustomField::instance()
        .find(Model_CustomField::DB_Table_CUSTOMFIELD::REFTYPE(reftype_desc)
        , Model_CustomField::DB_Table_CUSTOMFIELD::TYPE("String"));
    std::map<int, Model_CustomFieldData::Data_Set> data;
    for (const auto & custom_field : this->find(Model_CustomFieldData::FIELDID(data_set.begin()->FIELDID)))
    {
        data[custom_field.REFID].push_back(custom_field);
    }

    return data;
}
