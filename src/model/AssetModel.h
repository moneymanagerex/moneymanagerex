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

#include "defs.h"
#include "util/choices.h"
#include "db/DB_Table_Assets_V1.h"
#include "_Model.h"
#include "CurrencyModel.h"

class AssetModel : public Model<DB_Table_ASSETS_V1>
{
public:
    enum TYPE_ID
    {
        TYPE_ID_PROPERTY = 0,
        TYPE_ID_AUTO,
        TYPE_ID_HOUSE,
        TYPE_ID_ART,
        TYPE_ID_JEWELLERY,
        TYPE_ID_CASH,
        TYPE_ID_OTHER,
        TYPE_ID_size
    };
    enum STATUS_ID {
        STATUS_ID_CLOSED = 0,
        STATUS_ID_OPEN,
        STATUS_ID_size
    };
    enum CHANGE_ID
    {
        CHANGE_ID_NONE = 0,
        CHANGE_ID_APPRECIATE,
        CHANGE_ID_DEPRECIATE,
        CHANGE_ID_size
    };
    enum CHANGEMODE_ID {
        CHANGEMODE_ID_PERCENTAGE = 0,
        CHANGEMODE_ID_LINEAR,
        CHANGEMODE_ID_size
    };

private:
    static ChoicesName TYPE_CHOICES;
    static ChoicesName STATUS_CHOICES;
    static ChoicesName CHANGE_CHOICES;
    static ChoicesName CHANGEMODE_CHOICES;

public:
    AssetModel();
    ~AssetModel();

public:
    /**
    Initialize the global AssetModel table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for AssetModel table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static AssetModel& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for AssetModel table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static AssetModel& instance();

public:
    static DB_Table_ASSETS_V1::ASSETTYPE ASSETTYPE(TYPE_ID type, OP op = EQUAL);
    static DB_Table_ASSETS_V1::STARTDATE STARTDATE(const wxDate& date, OP op = EQUAL);
    
public:
    static wxString get_asset_name(int64 asset_id);
    double balance();
    static wxDate STARTDATE(const Data* r);
    static wxDate STARTDATE(const Data& r);

    static const wxString type_name(int id);
    static int type_id(const wxString& name, int default_id = -1);
    static TYPE_ID type_id(const Data* r);
    static TYPE_ID type_id(const Data& r);

    static const wxString status_name(int id);
    static int status_id(const wxString& name, int default_id = -1);
    static STATUS_ID status_id(const Data* r);
    static STATUS_ID status_id(const Data& r);

    static const wxString change_name(int id);
    static int change_id(const wxString& name, int default_id = -1);
    static CHANGE_ID change_id(const Data* r);
    static CHANGE_ID change_id(const Data& r);

    static const wxString changemode_name(int id);
    static int changemode_id(const wxString& name, int default_id = -1);
    static CHANGEMODE_ID changemode_id(const Data* r);
    static CHANGEMODE_ID changemode_id(const Data& r);

    /** Returns the base currency Data record pointer*/
    static CurrencyModel::Data* currency(const Data* /* r */);
    /** Returns the calculated current value */
    static std::pair<double, double> value(const Data* r);
    /** Returns the calculated current value */
    static std::pair<double, double> value(const Data& r);
    /** Returns the calculated value at a given date */
    std::pair<double, double> valueAtDate(const Data* r, const wxDate& date);

public:
    static const wxString refTypeName;
};

//----------------------------------------------------------------------------

inline const wxString AssetModel::type_name(int id)
{
    return TYPE_CHOICES.getName(id);
}

inline int AssetModel::type_id(const wxString& name, int default_id)
{
    return TYPE_CHOICES.findName(name, default_id);
}

inline AssetModel::TYPE_ID AssetModel::type_id(const Data* asset)
{
    return static_cast<TYPE_ID>(type_id(asset->ASSETTYPE));
}

inline AssetModel::TYPE_ID AssetModel::type_id(const Data& asset)
{
    return type_id(&asset);
}

inline const wxString AssetModel::status_name(int id)
{
    return STATUS_CHOICES.getName(id);
}

inline int AssetModel::status_id(const wxString& name, int default_id)
{
    return STATUS_CHOICES.findName(name, default_id);
}

inline AssetModel::STATUS_ID AssetModel::status_id(const Data* asset)
{
    return static_cast<STATUS_ID>(status_id(asset->ASSETSTATUS));
}

inline AssetModel::STATUS_ID AssetModel::status_id(const Data& asset)
{
    return status_id(&asset);
}

inline const wxString AssetModel::change_name(int id)
{
    return CHANGE_CHOICES.getName(id);
}

inline int AssetModel::change_id(const wxString& name, int default_id)
{
    return CHANGE_CHOICES.findName(name, default_id);
}

inline AssetModel::CHANGE_ID AssetModel::change_id(const Data* asset)
{
    return static_cast<CHANGE_ID>(change_id(asset->VALUECHANGE));
}

inline AssetModel::CHANGE_ID AssetModel::change_id(const Data& asset)
{
    return change_id(&asset);
}

inline const wxString AssetModel::changemode_name(int id)
{
    return CHANGEMODE_CHOICES.getName(id);
}

inline int AssetModel::changemode_id(const wxString& name, int default_id)
{
    return CHANGEMODE_CHOICES.findName(name, default_id);
}

inline AssetModel::CHANGEMODE_ID AssetModel::changemode_id(const Data* asset)
{
    return static_cast<CHANGEMODE_ID>(changemode_id(asset->VALUECHANGEMODE));
}

inline AssetModel::CHANGEMODE_ID AssetModel::changemode_id(const Data& asset)
{
    return changemode_id(&asset);
}

