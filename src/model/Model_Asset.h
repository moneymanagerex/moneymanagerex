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

#ifndef MODEL_ASSET_H
#define MODEL_ASSET_H

#include "Model.h"
#include "db/DB_Table_Assets_V1.h"
#include "Model_Currency.h" // detect base currency

class Model_Asset : public Model<DB_Table_ASSETS_V1>
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
        TYPE_ID_OTHER
    };
    enum STATUS_ID {
        STATUS_ID_CLOSED = 0,
        STATUS_ID_OPEN
    };
    enum CHANGE_ID
    {
        CHANGE_ID_NONE = 0,
        CHANGE_ID_APPRECIATE,
        CHANGE_ID_DEPRECIATE
    };
    enum CHANGEMODE_ID {
        CHANGEMODE_ID_PERCENTAGE = 0,
        CHANGEMODE_ID_LINEAR
    };
    static wxArrayString TYPE_STR;
    static wxArrayString STATUS_STR;
    static wxArrayString CHANGE_STR;
    static wxArrayString CHANGEMODE_STR;

private:
    static const std::vector<std::pair<TYPE_ID, wxString> > TYPE_CHOICES;
    static const std::vector<std::pair<STATUS_ID, wxString> > STATUS_CHOICES;
    static const std::vector<std::pair<CHANGE_ID, wxString> > CHANGE_CHOICES;
    static const std::vector<std::pair<CHANGEMODE_ID, wxString> > CHANGEMODE_CHOICES;
    static wxArrayString type_str_all();
    static wxArrayString status_str_all();
    static wxArrayString change_str_all();
    static wxArrayString changemode_str_all();

public:
    Model_Asset();
    ~Model_Asset();

public:
    /**
    Initialize the global Model_Asset table on initial call.
    Resets the global table on subsequent calls.
    * Return the static instance address for Model_Asset table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Asset& instance(wxSQLite3Database* db);

    /**
    * Return the static instance address for Model_Asset table
    * Note: Assigning the address to a local variable can destroy the instance.
    */
    static Model_Asset& instance();

public:
    static DB_Table_ASSETS_V1::ASSETTYPE ASSETTYPE(TYPE_ID type, OP op = EQUAL);
    static DB_Table_ASSETS_V1::STARTDATE STARTDATE(const wxDate& date, OP op = EQUAL);
    
public:
    static wxString get_asset_name(int64 asset_id);
    double balance();
    static wxDate STARTDATE(const Data* r);
    static wxDate STARTDATE(const Data& r);

    static TYPE_ID type_id(const Data* r);
    static TYPE_ID type_id(const Data& r);
    static STATUS_ID status_id(const Data* r);
    static STATUS_ID status_id(const Data& r);
    static CHANGE_ID change_id(const Data* r);
    static CHANGE_ID change_id(const Data& r);
    static CHANGEMODE_ID changemode_id(const Data* r);
    static CHANGEMODE_ID changemode_id(const Data& r);

    /** Returns the base currency Data record pointer*/
    static Model_Currency::Data* currency(const Data* /* r */);
    /** Returns the calculated current value */
    static double value(const Data* r);
    /** Returns the calculated current value */
    static double value(const Data& r);
    /** Returns the calculated value at a given date */
    double valueAtDate(const Data* r, const wxDate date);
};

#endif // 
