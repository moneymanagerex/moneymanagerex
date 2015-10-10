/*******************************************************
Copyright (C) 2014, 2015 Nikolay

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
*******************************************************/

#include <wx/imaglist.h>

enum img {
    HOUSE_XPM,
    SCHEDULE_XPM,
    CALENDAR_XPM,
    PIECHART_XPM,
    HELP_XPM,
    FILTER_XPM,
    ASSET_XPM,
    CUSTOMSQL_XPM,
    CUSTOMSQL_GRP_XPM,
    MONEYACCOUNT_XPM,
    SAVINGS_ACC_FAVORITE_XPM, //10
    SAVINGS_ACC_CLOSED_XPM,
    TERMACCOUNT_XPM,
    TERM_ACC_FAVORITE_XPM,
    TERM_ACC_CLOSED_XPM,
    STOCK_ACC_XPM,
    STOCK_ACC_FAVORITE_XPM,
    STOCK_ACC_CLOSED_XPM,
    CARD_ACC_XPM,
    CARD_ACC_FAVORITE_XPM,
    CARD_ACC_CLOSED_XPM, //20
    MONEY_DOLLAR_XPM, //First user img
    MONEY_EURO_XPM,
    FLAG_XPM,
    ACCOUNTTREE_XPM,
    ABOUT_XPM,
    CLOCK_XPM,
    CAT_XPM,
    DOG_XPM,
    TREES_XPM,
    HOURGLASS_XPM,
    WORK_XPM,
    YANDEX_MONEY_XPM,
    WEB_MONEY_XPM,
    RUBIK_CUBE_XPM, 
    MAX_XPM,
};

enum png {
    NEW_DB,
    OPEN,
    NEW_ACC,
    HOME,
    CATEGORY,
    PAYEE,
    CURR,
    FILTER,
    GRM,
    OPTIONS,
    NEW_TRX,
    ABOUT,
    HELP,
    NEWS,
    NEW_NEWS,
    PIE_CHART,
    BUDGET,
    RECURRING,
    ASSET,
    PROPERTY,
    CAR,
    HOUSEHOLD_OBJ,
    ART,
    JEWELLERY,
    CASH,
    OTHER,
    FACEBOOK,
    CC_NORMAL,
    CC_FAVORITE,
    CC_CLOSED,
    EXIT,
};

wxImageList* navtree_images_list();
const wxBitmap mmBitmap(int ref);