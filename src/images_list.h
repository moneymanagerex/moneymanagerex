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
    HOUSE_PNG,
    SCHEDULE_PNG,
    CALENDAR_PNG,
    PIECHART_PNG,
    HELP_PNG,
    FILTER_PNG,
    ASSET_PNG,
    CUSTOMSQL_PNG,
    CUSTOMSQL_GRP_PNG,
    SAVINGS_ACC_NORMAL_PNG,
    SAVINGS_ACC_FAVORITE_PNG, //10
    SAVINGS_ACC_CLOSED_PNG,
    CARD_ACC_PNG,
    CARD_ACC_FAVORITE_PNG,
    CARD_ACC_CLOSED_PNG,
    TERMACCOUNT_PNG,
    TERM_ACC_FAVORITE_PNG,
    TERM_ACC_CLOSED_PNG,
    STOCK_ACC_PNG,
    STOCK_ACC_FAVORITE_PNG,
    STOCK_ACC_CLOSED_PNG, //20
    LAST_NAVTREE_PNG,
};

/* Images set for accounts to be user assigned */
enum acc_img {
    MONEY_DOLLAR_XPM = LAST_NAVTREE_PNG, //First user img
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
    SAVE,
    SAVEAS,
    APPSTART,
    CLEARLIST,
    DELETE_ACC,
    EDIT_ACC,
    REALLOCATE_ACC,
    ENCRYPT_DB,
    ENCRYPT_DB_EDIT,
    ACCOUNTTREE,
    NEW_ACC,
    HOME,
    CATEGORY,
    CATEGORY_RELOCATION,
    PAYEE,
    PAYEE_RELOCATION,
    CURR,
    FILTER,
    GRM,
    OPTIONS,
    NEW_TRX,
    ABOUT,
    HELP,
    NEWS,
    NEW_NEWS,
    FULLSCREEN,
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
    SAVINGS_NORMAL,
    SAVINGS_FAVORITES,
    SAVINGS_CLOSED,
    CC_NORMAL,
    CC_FAVORITE,
    CC_CLOSED,
    RECONCILED,
    VOID_STAT,
    FOLLOW_UP,
    DUPLICATE_STAT,
    RIGHTARROW,
    RIGHTARROW_ACTIVE,
    GOOGLE_PLAY,
    UPDATE,
    FORUM,
    CLIP,
    PROFIT,
    LOSS,
    WEB,
    UPARROW,
    DOWNARROW,
    IMPORT,
    LED_OFF,
    LED_RED,
    LED_YELLOW,
    LED_GREEN,
    RUN,
    AUTO,
    PRINT,
    EXIT,
    EMPTY,
};

wxImageList* navtree_images_list();
const wxBitmap mmBitmap(int ref);