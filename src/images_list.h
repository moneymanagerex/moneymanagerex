/*******************************************************
Copyright (C) 2014, 2015 Nikolay
Copyright (C) 22021 Mark Whalley (mark@ipx.co.uk)

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
    ALLTRANSACTIONS_PNG,
    SCHEDULE_PNG,
    CALENDAR_PNG,
    PIECHART_PNG,
    HELP_PNG,
    FAVOURITE_PNG,
    FILTER_PNG,
    ASSET_NORMAL_PNG,
    ASSET_CLOSED_PNG,
    CUSTOMSQL_PNG,
    CUSTOMSQL_GRP_PNG,
    SAVINGS_ACC_NORMAL_PNG,
    SAVINGS_ACC_CLOSED_PNG,
    CARD_ACC_NORMAL_PNG,
    CARD_ACC_CLOSED_PNG,
    TERMACCOUNT_NORMAL_PNG,
    TERM_ACC_CLOSED_PNG,
    STOCK_ACC_NORMAL_PNG,
    STOCK_ACC_CLOSED_PNG, 
    CASH_ACC_NORMAL_PNG,
    CASH_ACC_CLOSED_PNG,
    LOAN_ACC_NORMAL_PNG,
    LOAN_ACC_CLOSED_PNG,
    LAST_NAVTREE_PNG
};

/* Images set for accounts to be user assigned */
enum acc_img {
    ACC_ICON_MONEY = LAST_NAVTREE_PNG, //First user img
    ACC_ICON_EURO,
    ACC_ICON_FLAG,
    ACC_ICON_COINS,
    ACC_ICON_ABOUT,
    ACC_ICON_CLOCK,
    ACC_ICON_CAT,
    ACC_ICON_DOG,
    ACC_ICON_TREES,
    ACC_ICON_HOURGLASS,
    ACC_ICON_WORK,
    ACC_ICON_PAYPAL,
    ACC_ICON_WALLET,
    ACC_ICON_RUBIK,
    MAX_ACC_ICON,
};

enum png {
    ABOUT,
    ALLTRANSACTIONS,
    APPSTART,
    ART,
    ASSET_NORMAL,
    ASSET_CLOSED,
    BUDGET,
    CAR,
    CASH,
    CASH_CLOSED,
    CASH_NORMAL,
    CATEGORY,
    CATEGORY_RELOCATION,
    CC_CLOSED,
    CC_NORMAL,
    CLEAR,
    CLIP,
    CURR,
    CURRATES,
    DOWNARROW,
    DUPLICATE_STAT,
    EMPTY,
    FAVOURITE,
    FILTER,
    FOLLOW_UP,
    FULLSCREEN,
    RIGHTARROWSIMPLE,
    LEFTARROWSIMPLE,
    GRM,
    HELP,
    HOME,
    HOUSEHOLD_OBJ,
    IMPORT,
    JEWELLERY,
    LANG,
    LED_GREEN,
    LED_OFF,
    LED_RED,
    LED_YELLOW,
    LOAN_ACC_CLOSED,
    LOAN_ACC_NORMAL,
    LOSS,
    NEW_ACC,
    NEW_DB,
    NEW_NEWS,
    NEW_TRX,
    NEWS,
    OPEN,
    OPTIONS,
    OTHER,
    PAYEE,
    PIE_CHART,
    PRINT,
    PROFIT,
    PROPERTY,
    RECONCILED,
    RECURRING,
    RIGHTARROW,
    RIGHTARROW_ACTIVE,
    RUN,
    SAVE,
    SAVEAS,
    SAVINGS_CLOSED,
    SAVINGS_NORMAL,
    STOCKS_CLOSED,
    STOCKS_NORMAL,
    TERM_CLOSED,
    TERM_NORMAL,
    TRXNUM,
    UPARROW,
    UPDATE,
    VOID_STAT,
    WEB,
    // Settings
    GENERAL,
    VIEW,
    ATTACHMENTS,
    NETWORK,
    OTHERS,
    // Extra Account Icons
    ACC_MONEY,
    ACC_EURO,
    ACC_FLAG,
    ACC_COINS,
    ACC_ABOUT,
    ACC_CLOCK,
    ACC_CAT,
    ACC_DOG,
    ACC_TREES,
    ACC_HOURGLASS,
    ACC_WORK,
    ACC_PAYPAL,
    ACC_WALLET,
    ACC_RUBIK
};

wxImageList* navtree_images_list();
const wxBitmap mmBitmap(int ref);
