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
    CASH_ACC_NORMAL_PNG,
    CASH_ACC_FAVORITE_PNG,
    CASH_ACC_CLOSED_PNG,
    LOAN_ACC_NORMAL_PNG,
    LOAN_ACC_FAVORITE_PNG,
    LOAN_ACC_CLOSED_PNG,
    LAST_NAVTREE_PNG
};

/* Images set for accounts to be user assigned */
enum acc_img {
    MONEY_DOLLAR_XPM = LAST_NAVTREE_PNG, //First user img
    MONEY_EURO_XPM,
    FLAG_XPM,
    COINS_XPM,
    ABOUT_XPM,
    CLOCK_XPM,
    CAT_XPM,
    DOG_XPM,
    TREES_XPM,
    HOURGLASS_XPM,
    WORK_XPM,
    PAYPAL_XPM,
    WALLET_XPM,
    RUBIK_CUBE_XPM,
    MAX_XPM,
};

enum png {
    ABOUT,
    APPSTART,
    ART,
    ASSET,
    AUTO,
    BUDGET,
    BUG,
    CAR,
    CASH,
    CASH_CLOSED,
    CASH_FAVORITE,
    CASH_NORMAL,
    CATEGORY,
    CATEGORY_RELOCATION,
    CC_CLOSED,
    CC_FAVORITE,
    CC_NORMAL,
    CLEARLIST,
    CLIP,
    COMMUNITY,
    CROWDIN,
    CURR,
    CURRATES,
    DELETE_ACC,
    DONATE,
    DOWNARROW,
    DUPLICATE_STAT,
    EDIT_ACC,
    EMPTY,
    ENCRYPT_DB,
    ENCRYPT_DB_EDIT,
    EXIT,
    FACEBOOK,
    FILTER,
    FOLLOW_UP,
    FORUM,
    FULLSCREEN,
    GITHUB,
    RIGHTARROWSIMPLE,
    LEFTARROWSIMPLE,
    GOOGLE_PLAY,
    GRM,
    CUSTOM,
    HELP,
    HOME,
    RELOCATION,
    HOUSEHOLD_OBJ,
    IMPORT,
    JEWELLERY,
    LANG,
    LED_GREEN,
    LED_OFF,
    LED_RED,
    LED_YELLOW,
    LOAN_ACC_CLOSED,
    LOAN_ACC_FAVORITE,
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
    PAYEE_RELOCATION,
    MENU_RELOCATION,
    PIE_CHART,
    PRINT,
    PROFIT,
    PROPERTY,
    REALLOCATE_ACC,
    RECONCILED,
    RECURRING,
    RIGHTARROW,
    RIGHTARROW_ACTIVE,
    RUN,
    SAVE,
    SAVEAS,
    SAVINGS_CLOSED,
    SAVINGS_FAVORITES,
    SAVINGS_NORMAL,
    SLACK,
    STOCKS_CLOSED,
    STOCKS_FAVORITE,
    STOCKS_NORMAL,
    TERM_CLOSED,
    TERM_FAVORITE,
    TERM_NORMAL,
    TRXNUM,
    UPARROW,
    UPDATE,
    VOID_STAT,
    WEB,
};

wxImageList* navtree_images_list();
const wxBitmap mmBitmap(int ref);
