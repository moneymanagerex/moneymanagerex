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

#pragma once

#include "defs.h"
#include "util/choices.h"
#include "_Model.h"
#include "db/DB_Table_Customfield_V1.h"

class FieldModel : public Model<DB_Table_CUSTOMFIELD_V1>
{
public:
    using Model<DB_Table_CUSTOMFIELD_V1>::get;

    enum TYPE_ID
    {
        TYPE_ID_UNKNOWN = -1,
        TYPE_ID_STRING = 0,
        TYPE_ID_INTEGER,
        TYPE_ID_DECIMAL,
        TYPE_ID_BOOLEAN,
        TYPE_ID_DATE,
        TYPE_ID_TIME,
        TYPE_ID_SINGLECHOICE,
        TYPE_ID_MULTICHOICE,
        TYPE_ID_size
    };

private:
    static ChoicesName TYPE_CHOICES;

public:
    FieldModel();
    ~FieldModel();

public:
    /**
    Initialize the global FieldModel table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for FieldModel table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static FieldModel& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for FieldModel table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static FieldModel& instance();

public:
    static const wxString type_name(int id);
    static int type_id(const wxString& name, int default_id = TYPE_ID_UNKNOWN);
    static TYPE_ID type_id(const Data* r);
    static TYPE_ID type_id(const Data& r);

    bool Delete(const int64& FieldID);
    static const wxString getRegEx(const wxString& properties);
    static const wxString getTooltip(const wxString& properties);
    static bool getAutocomplete(const wxString& properties);
    static const wxString getDefault(const wxString& properties);
    static const wxArrayString getChoices(const wxString& properties);
    static const wxArrayString getUDFCList(DB_Table_CUSTOMFIELD_V1::Data* r);
    static const wxString getUDFC(const wxString& properties);
    static const wxString getUDFCName(const wxString& ref_type, const wxString& name);
    static TYPE_ID getUDFCType(const wxString& ref_type, const wxString& name);
    static const wxString getUDFCProperties(const wxString& ref_type, const wxString& name);
    static int64 getUDFCID(const wxString& ref_type, const wxString& name);
    static const std::map<wxString, int64> getMatrix(const wxString& reftype);
    static int getDigitScale(const wxString& Properties);
    static const wxString formatProperties(const wxString& Tooltip, const wxString& RegEx
        , bool Autocomplete, const wxString& Default, const wxArrayString& Choices
        , const int DigitScale, const wxString& udfc_str);
    static const wxArrayString UDFC_FIELDS();
};

//----------------------------------------------------------------------------

inline const wxString FieldModel::type_name(int id)
{
    return TYPE_CHOICES.getName(id);
}

inline int FieldModel::type_id(const wxString& name, int default_id)
{
    return TYPE_CHOICES.findName(name, default_id);
}

inline FieldModel::TYPE_ID FieldModel::type_id(const Data* r)
{
    return static_cast<TYPE_ID>(type_id(r->TYPE));
}

inline FieldModel::TYPE_ID FieldModel::type_id(const Data& r)
{
    return type_id(&r);
}

