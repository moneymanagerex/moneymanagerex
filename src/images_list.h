/*******************************************************
Copyright (C) 2014, 2015 Nikolay Akimov
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

#include <wx/bmpbndl.h>
#include <wx/window.h>
#include <wx/imaglist.h>
#include <wx/arrstr.h>
#include <vector>

// Meta data items
enum meta {
    THEME_NAME,
    THEME_AUTHOR,
    THEME_DESCRIPTION,  
    THEME_URL,
    COLOR_NAVPANEL,
    COLOR_NAVPANEL_FONT,
    COLOR_LISTPANEL,
    COLOR_LIST,
    COLOR_LISTALT0,
    COLOR_LISTALT0A,
    COLOR_LISTTOTAL,
    COLOR_LISTBORDER,
    COLOR_LISTFUTURE,
    COLOR_HTMLPANEL_BACK,
    COLOR_HTMLPANEL_FORE,
    COLOR_REPORT_ALTROW,
    COLOR_REPORT_CREDIT,
    COLOR_REPORT_DEBIT,
    COLOR_REPORT_DELTA,
    COLOR_REPORT_PERF,
    COLOR_REPORT_FORECOLOR,
    COLOR_REPORT_PALETTE,
    // The end
    MAX_METADATA
};

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
    CUSTOMSQL_PNG,
    CUSTOMSQL_GRP_PNG,
    SAVINGS_ACC_NORMAL_PNG,
    CARD_ACC_NORMAL_PNG,
    TERMACCOUNT_NORMAL_PNG,
    STOCK_ACC_NORMAL_PNG,
    CASH_ACC_NORMAL_PNG,
    TRASH_PNG,
    LOAN_ACC_NORMAL_PNG,
    ACCOUNT_CLOSED_PNG,
    LAST_NAVTREE_PNG,
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
    TAG,
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
    CC_NORMAL,
    CASH_NORMAL, 
    LOAN_ACC_NORMAL,
    TERM_NORMAL,
    STOCKS_NORMAL, 
    ASSET_NORMAL,
    ACCOUNT_CLOSED,
    RECURRING,
    TRASH,
    BUDGET,
    PIE_CHART,
    
    // Status
    UNRECONCILED,
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
    CALCULATOR,
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

extern const int mmBitmapButtonSize;
//wxImageList* navtree_images_list(const int size = 0, const double dpiScale = 1.0);
wxVector<wxBitmapBundle> navtree_images_list(const int size = 0);
void LoadTheme();
void CloseTheme();
const wxString mmThemeMetaString(int ref);
long mmThemeMetaLong(int ref);
const wxColour mmThemeMetaColour(int ref);
void mmThemeMetaColour(wxWindow *object, int ref, bool foreground = false);
const std::vector<wxColour> mmThemeMetaColourArray(int ref);
const wxBitmapBundle mmBitmapBundle(const int ref, const int defSize = -1);
