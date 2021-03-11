/*******************************************************
Copyright (C) 2014, 2015 Nikolay
Copyright (C) 2021 Mark Whalley (mark@ipx.co.uk)

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
#include <wx/sharedptr.h>
#include <wx/arrstr.h>
#include <map>

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

    // Toolbar
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
    NEW_NEWS,
    NEWS,
    CURRATES, 
    FULLSCREEN, 
    PRINT,
    ABOUT, 
    HELP,

    // Navigation
    NAV_HOME,
    ALLTRANSACTIONS, 
    NAV_FILTER,
    NAV_GRM,
    NAV_HELP,
    FAVOURITE,
    SAVINGS_NORMAL, 
    SAVINGS_CLOSED,
    CC_NORMAL,
    CC_CLOSED,
    CASH_NORMAL, 
    CASH_CLOSED, 
    LOAN_ACC_NORMAL,
    LOAN_ACC_CLOSED, 
    TERM_NORMAL,
    TERM_CLOSED,
    STOCKS_NORMAL, 
    STOCKS_CLOSED, 
    ASSET_NORMAL,
    ASSET_CLOSED,
    RECURRING,
    BUDGET,
    PIE_CHART,
    
    // Status
    RECONCILED,
    DUPLICATE_STAT,
    FOLLOW_UP,
    VOID_STAT, 
    PROFIT,
    LOSS,
    LED_OFF,
    LED_GREEN, 
    LED_YELLOW, 
    LED_RED,
    RUN,
    RUN_AUTO, 

    // Assets
    PROPERTY,
    CAR,
    HOUSEHOLD_OBJ, 
    ART,
    JEWELLERY, 
    CASH,
    OTHER,

    //Controls
    CLEAR,
    CLIP,
    DOWNARROW,
    UPARROW,
    LEFTARROW,
    RIGHTARROW,
    IMPORT,
    MORE_OPTIONS,
    RELOCATION,
    SAVE,
    TRANSFILTER,
    TRANSFILTER_ACTIVE,
    TRXNUM,
    UPDATE,
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
    ACC_RUBIK,

    // Other
    EMPTY,

    // The end
    MAX_PNG
};

// SVG filename in Zip, the PNG enum to which it relates, the background color for the generated bitmap 0 = transparent
static const std::map<std::string, std::pair<int, std::uint32_t>> iconName2enum = {
    { "NEW_DB.svg", { NEW_DB, 0 } },
    { "OPEN.svg", { OPEN, 0 } },
    { "NEW_ACC.svg", { NEW_ACC, 0 } },
    { "HOME.svg", { HOME, 0 } },
    { "CATEGORY.svg", { CATEGORY, 0 } },
    { "PAYEE.svg", { PAYEE, 0 } },
    { "CURR.svg", { CURR, 0 } },
    { "FILTER.svg", { FILTER, 0 } }, 
    { "GRM.svg", { GRM, 0 } },
    { "OPTIONS.svg", { OPTIONS, 0 } },
    { "NEW_TRX.svg", { NEW_TRX, 0 } },
    { "NEW_NEWS.svg", { NEW_NEWS, 0 } },
    { "NEWS.svg", { NEWS, 0 } },
    { "CURRATES.svg", { CURRATES, 0 } },
    { "FULLSCREEN.svg", { FULLSCREEN, 0 } },
    { "PRINT.svg", { PRINT, 0 } },
    { "ABOUT.svg", { ABOUT, 0 } },
    { "HELP.svg", { HELP, 0 } },

    // Navigation
    { "NAV_HOME.svg", { NAV_HOME, 0xFFFFFFFF } },
    { "ALLTRANSACTIONS.svg", { ALLTRANSACTIONS, 0xFFFFFFFF } },
    { "NAV_FILTER.svg", { NAV_FILTER, 0xFFFFFFFF } },
    { "NAV_GRM.svg", { NAV_GRM, 0xFFFFFFFF } },
    { "NAV_HELP.svg", { NAV_HELP, 0xFFFFFFFF } },
    { "FAVOURITE.svg", { FAVOURITE, 0xFFFFFFFF } },
    { "SAVINGS_NORMAL.svg", { SAVINGS_NORMAL, 0xFFFFFFFF } },
    { "SAVINGS_CLOSED.svg", { SAVINGS_CLOSED, 0xFFFFFFFF } },
    { "CC_NORMAL.svg", { CC_NORMAL, 0xFFFFFFFF } },
    { "CC_CLOSED.svg", { CC_CLOSED, 0xFFFFFFFF } },
    { "CASH_NORMAL.svg", { CASH_NORMAL, 0xFFFFFFFF } },
    { "CASH_CLOSED.svg", { CASH_CLOSED, 0xFFFFFFFF } },
    { "LOAN_ACC_NORMAL.svg", { LOAN_ACC_NORMAL, 0xFFFFFFFF } },
    { "LOAN_ACC_CLOSED.svg", { LOAN_ACC_CLOSED, 0xFFFFFFFF } },
    { "TERM_NORMAL.svg", { TERM_NORMAL, 0xFFFFFFFF } },
    { "TERM_CLOSED.svg", { TERM_CLOSED, 0xFFFFFFFF } },
    { "STOCKS_NORMAL.svg", { STOCKS_NORMAL, 0xFFFFFFFF } },
    { "STOCKS_CLOSED.svg", { STOCKS_CLOSED, 0xFFFFFFFF } },
    { "ASSET_NORMAL.svg", { ASSET_NORMAL, 0xFFFFFFFF } },
    { "ASSET_CLOSED.svg", { ASSET_CLOSED, 0xFFFFFFFF } },
    { "RECURRING.svg", { RECURRING, 0xFFFFFFFF } },
    { "BUDGET.svg", { BUDGET, 0xFFFFFFFF } },
    { "PIE_CHART.svg", { PIE_CHART, 0xFFFFFFFF } },
     
    // Status
    { "RECONCILED.svg", { RECONCILED, 0 } },
    { "DUPLICATE_STAT.svg", { DUPLICATE_STAT, 0 } },
    { "FOLLOW_UP.svg", { FOLLOW_UP, 0 } },
    { "VOID_STAT.svg", { VOID_STAT, 0 } },
    { "PROFIT.svg", { PROFIT, 0 } },
    { "LOSS.svg", { LOSS, 0 } },
    { "LED_OFF.svg", { LED_OFF, 0 } },
    { "LED_GREEN.svg", { LED_GREEN, 0 } },
    { "LED_YELLOW.svg", { LED_YELLOW, 0 } },
    { "LED_RED.svg", { LED_RED, 0 } },
    { "RUN.svg", { RUN, 0 } },
    { "RUN_AUTO.svg", { RUN_AUTO, 0 } },

    // Assets
    { "PROPERTY.svg", { PROPERTY, 0 } },
    { "CAR.svg", { CAR, 0 } },
    { "HOUSEHOLD_OBJ.svg", { HOUSEHOLD_OBJ, 0 } },
    { "ART.svg", { ART, 0 } },
    { "JEWELLERY.svg", { JEWELLERY, 0 } },
    { "CASH.svg", { CASH, 0 } },
    { "OTHER.svg", { OTHER, 0 } },

    //Controls
    { "CLEAR.svg", { CLEAR, 0 } },
    { "CLIP.svg", { CLIP, 0 } },
    { "DOWNARROW.svg", { DOWNARROW, 0 } },
    { "UPARROW.svg", { UPARROW, 0 } },
    { "LEFTARROW.svg", { LEFTARROW, 0 } },
    { "RIGHTARROW.svg", { RIGHTARROW, 0 } },
    { "IMPORT.svg", { IMPORT, 0 } },
    { "MORE_OPTIONS.svg", { MORE_OPTIONS, 0 } },
    { "RELOCATION.svg", { RELOCATION, 0 } },
    { "SAVE.svg", { SAVE, 0 } },
    { "TRANSFILTER.svg", { TRANSFILTER, 0 } },
    { "TRANSFILTER_ACTIVE.svg", { TRANSFILTER_ACTIVE, 0 } },
    { "TRXNUM.svg", { TRXNUM, 0 } },
    { "UPDATE.svg", { UPDATE, 0 } },
    { "WEB.svg", { WEB, 0 } },

    // Settings
    { "GENERAL.svg", { GENERAL, 0 } },
    { "VIEW.svg", { VIEW, 0 } },
    { "ATTACHMENTS.svg", { ATTACHMENTS, 0 } },
    { "NETWORK.svg", { NETWORK, 0 } },
    { "OTHERS.svg", { OTHERS, 0 } },

    // Extra Account Icons
    { "ACC_MONEY.svg", { ACC_MONEY, 0xFFFFFFFF } },
    { "ACC_EURO.svg", { ACC_EURO, 0xFFFFFFFF } },
    { "ACC_FLAG.svg", { ACC_FLAG, 0xFFFFFFFF } },
    { "ACC_ABOUT.svg", { ACC_ABOUT, 0xFFFFFFFF } },
    { "ACC_COINS.svg", { ACC_COINS, 0xFFFFFFFF } },
    { "ACC_CLOCK.svg", { ACC_CLOCK, 0xFFFFFFFF } },
    { "ACC_CAT.svg", { ACC_CAT, 0xFFFFFFFF } },
    { "ACC_DOG.svg", { ACC_DOG, 0xFFFFFFFF } },
    { "ACC_TREES.svg", { ACC_TREES, 0xFFFFFFFF } },
    { "ACC_HOURGLASS.svg", { ACC_HOURGLASS, 0xFFFFFFFF } },
    { "ACC_WORK.svg", { ACC_WORK, 0xFFFFFFFF } },
    { "ACC_PAYPAL.svg", { ACC_PAYPAL, 0xFFFFFFFF } },
    { "ACC_WALLET.svg", { ACC_WALLET, 0xFFFFFFFF } },
    { "ACC_RUBIK.svg", { ACC_RUBIK, 0xFFFFFFFF } },

    // Other
    { "EMPTY.svg", { EMPTY, 0 } }
};

static bool iconsLoaded;
static wxSharedPtr<wxBitmap> programIcons16[MAX_PNG];
static wxSharedPtr<wxBitmap> programIcons24[MAX_PNG];
static wxSharedPtr<wxBitmap> programIcons32[MAX_PNG];
static wxSharedPtr<wxBitmap> programIcons48[MAX_PNG];

static wxSharedPtr<wxArrayString> themes;
wxArrayString getThemes();

wxImageList* navtree_images_list();
const wxBitmap mmBitmap(int ref);

wxBitmap* CreateBitmapFromRGBA(unsigned char *rgba, int size);
bool buildBitmapsFromSVG(void);
