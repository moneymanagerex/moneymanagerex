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

#ifndef MODEL_CUSTOMFIELDDATA_H
#define MODEL_CUSTOMFIELDDATA_H

#include "Model.h"
#include "Model_Attachment.h" 
#include "Table_Customfielddata.h"

class Model_CustomFieldData : public Model<DB_Table_CUSTOMFIELDDATA>
{
public:
    using Model<DB_Table_CUSTOMFIELDDATA>::get;

public:
    Model_CustomFieldData();
    ~Model_CustomFieldData();

public:
    /**
    Initialize the global Model_CustomFieldData table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for Model_CustomFieldData table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_CustomFieldData& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for Model_CustomFieldData table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_CustomFieldData& instance();

public:
    std::map<int, Data_Set> get_all(Model_Attachment::REFTYPE reftype);
    Data* get(int FieldID, int RefID);
    wxArrayString allValue(const int FieldID);
    bool DeleteAllData(const wxString& RefType, int RefID);
};

#endif // 
