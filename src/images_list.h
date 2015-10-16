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
    SAVINGS_ACC_NORMAL_XPM,
    SAVINGS_ACC_FAVORITE_XPM, //10
    SAVINGS_ACC_CLOSED_XPM,
    CARD_ACC_XPM,
    CARD_ACC_FAVORITE_XPM,
    CARD_ACC_CLOSED_XPM,
    TERMACCOUNT_XPM,
    TERM_ACC_FAVORITE_XPM,
    TERM_ACC_CLOSED_XPM,
    STOCK_ACC_XPM,
    STOCK_ACC_FAVORITE_XPM,
    STOCK_ACC_CLOSED_XPM, //20
    MAX_XPM,
};

enum png {
    NEW_DB,
    OPEN,
    SAVE,
    SAVEAS,
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
    UPARROW,
    DOWNARROW,
    IMPORT,
    LED_OFF,
    LED_RED,
    LED_YELLOW,
    LED_GREEN,
    RUN,
    AUTO,
    EXIT,
    EMPTY,
};

wxImageList* navtree_images_list();
const wxBitmap mmBitmap(int ref);