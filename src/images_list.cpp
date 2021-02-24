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

#include "images_list.h"
#include "option.h"
#include "util.h"
#include <wx/image.h>
#include <wx/bitmap.h>
#include <map>
//#include <wx/artprov.h>//

#include "../resources/toolbar/icons.h"
#include "../resources/navigation/icons.h"
#include "../resources/assets/icons.h"
#include "../resources/controls/icons.h"
#include "../resources/status/icons.h"
#include "../resources/settings/icons.h"
#include "../resources/accounticons/icons.h"

#include "../resources/category_relocation_png.h"
#include "../resources/clear_png.h"
#include "../resources/empty_png.h"
#include "../resources/import_png.h"
#include "../resources/leds_png.h"
#include "../resources/payee_relocation_png.h"
#include "../resources/rightarrow_png.h"
#include "../resources/run_png.h"
#include "../resources/save_png.h"
#include "../resources/saveas_png.h"
#include "../resources/simplearrow_left_png.h"
#include "../resources/simplearrow_right_png.h"
#include "../resources/trxnum_png.h"
#include "../resources/update_png.h"

static const std::map<int, wxBitmap> navtree_images()
{
    return{
        { HOUSE_PNG, mmBitmap(png::HOME) }
        , { ALLTRANSACTIONS_PNG, mmBitmap(png::ALLTRANSACTIONS) }
        , { SCHEDULE_PNG, mmBitmap(png::RECURRING) }
        , { CALENDAR_PNG, mmBitmap(png::BUDGET) }
        , { PIECHART_PNG, mmBitmap(png::PIE_CHART) }
        , { HELP_PNG, mmBitmap(png::HELP) }
        , { FAVOURITE_PNG, mmBitmap(png::FAVOURITE) }
        , { FILTER_PNG, mmBitmap(png::FILTER) }
        , { ASSET_NORMAL_PNG, mmBitmap(png::ASSET_NORMAL) }
        , { ASSET_CLOSED_PNG, mmBitmap(png::ASSET_CLOSED) } 
        , { CUSTOMSQL_PNG, mmBitmap(png::GRM) }
        , { CUSTOMSQL_GRP_PNG, mmBitmap(png::GRM) } //TODO: GRM rep group ico
        , { SAVINGS_ACC_NORMAL_PNG, mmBitmap(png::SAVINGS_NORMAL) }
        , { SAVINGS_ACC_CLOSED_PNG, mmBitmap(png::SAVINGS_CLOSED) }
        , { CARD_ACC_NORMAL_PNG, mmBitmap(png::CC_NORMAL) }
        , { CARD_ACC_CLOSED_PNG, mmBitmap(png::CC_CLOSED) }
        , { TERMACCOUNT_NORMAL_PNG, mmBitmap(png::TERM_NORMAL) }
        , { TERM_ACC_CLOSED_PNG, mmBitmap(png::TERM_CLOSED) }
        , { STOCK_ACC_NORMAL_PNG, mmBitmap(png::STOCKS_NORMAL) }
        , { STOCK_ACC_CLOSED_PNG, mmBitmap(png::STOCKS_CLOSED) }
        , { CASH_ACC_NORMAL_PNG, mmBitmap(png::CASH_NORMAL) }
        , { CASH_ACC_CLOSED_PNG, mmBitmap(png::CASH_CLOSED) }
        , { LOAN_ACC_NORMAL_PNG, mmBitmap(png::LOAN_ACC_NORMAL) }
        , { LOAN_ACC_CLOSED_PNG, mmBitmap(png::LOAN_ACC_CLOSED) }
    };
};

// Custom icons for accounts
static const std::map<int, wxBitmap> acc_images()
{
    return
    {
        { ACC_ICON_MONEY, mmBitmap(png::ACC_MONEY) }
        , { ACC_ICON_EURO, mmBitmap(png::ACC_EURO) }
        , { ACC_ICON_FLAG, mmBitmap(png::ACC_FLAG) }
        , { ACC_ICON_COINS, mmBitmap(png::ACC_COINS) }
        , { ACC_ICON_ABOUT, mmBitmap(png::ACC_ABOUT) }
        , { ACC_ICON_CLOCK, mmBitmap(png::ACC_CLOCK) }
        , { ACC_ICON_CAT, mmBitmap(png::ACC_CAT) }
        , { ACC_ICON_DOG, mmBitmap(png::ACC_DOG) }
        , { ACC_ICON_TREES, mmBitmap(png::ACC_TREES) }
        , { ACC_ICON_HOURGLASS, mmBitmap(png::ACC_HOURGLASS) }
        , { ACC_ICON_WORK, mmBitmap(png::ACC_WORK) }
        , { ACC_ICON_PAYPAL, mmBitmap(png::ACC_PAYPAL)}
        , { ACC_ICON_WALLET, mmBitmap(png::ACC_WALLET) }
        , { ACC_ICON_RUBIK, mmBitmap(png::ACC_RUBIK) }
    };
}
wxImageList* navtree_images_list()
{
    int x = Option::instance().getIconSize();
    wxImageList* imageList = createImageList();
    for (const auto& img : navtree_images())
    {
        wxASSERT(img.second.GetHeight() == x && img.second.GetWidth() == x);
        imageList->Add(img.second);
    }
    for (const auto& img : acc_images())
    {
        wxASSERT(img.second.GetHeight() == x && img.second.GetWidth() == x);
        imageList->Add(img.second);
    }

    return imageList;
}

const wxBitmap mmBitmap(int ref)
{
    int x = Option::instance().getIconSize();

    switch (ref)
    {
    // Toolbar
    case NEW_DB: return x == 16 ? wxBITMAP_PNG_FROM_DATA(NEW_DB16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(NEW_DB24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(NEW_DB32) : wxBITMAP_PNG_FROM_DATA(NEW_DB48);
    case OPEN: return x == 16 ? wxBITMAP_PNG_FROM_DATA(OPEN16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(OPEN24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(OPEN32) : wxBITMAP_PNG_FROM_DATA(OPEN48);
    case NEW_ACC: return x == 16 ? wxBITMAP_PNG_FROM_DATA(NEW_ACC16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(NEW_ACC24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(NEW_ACC32) : wxBITMAP_PNG_FROM_DATA(NEW_ACC48);
    case HOME: return x == 16 ? wxBITMAP_PNG_FROM_DATA(HOME16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(HOME24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(HOME32) : wxBITMAP_PNG_FROM_DATA(HOME48);
    case CATEGORY: return x == 16 ? wxBITMAP_PNG_FROM_DATA(CATEGORY16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(CATEGORY24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(CATEGORY32) : wxBITMAP_PNG_FROM_DATA(CATEGORY48);
    case PAYEE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(PAYEE16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(PAYEE24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(PAYEE32) : wxBITMAP_PNG_FROM_DATA(PAYEE48);
    case CURR: return x == 16 ? wxBITMAP_PNG_FROM_DATA(CURR16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(CURR24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(CURR32) : wxBITMAP_PNG_FROM_DATA(CURR48);
    case FILTER: return x == 16 ? wxBITMAP_PNG_FROM_DATA(FILTER16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(FILTER24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(FILTER32) : wxBITMAP_PNG_FROM_DATA(FILTER48);
    case GRM: return x == 16 ? wxBITMAP_PNG_FROM_DATA(GRM16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(GRM24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(GRM32) : wxBITMAP_PNG_FROM_DATA(GRM48);
    case OPTIONS: return x == 16 ? wxBITMAP_PNG_FROM_DATA(OPTIONS16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(OPTIONS24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(OPTIONS32) : wxBITMAP_PNG_FROM_DATA(OPTIONS48);
    case NEW_TRX: return x == 16 ? wxBITMAP_PNG_FROM_DATA(NEW_TRX16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(NEW_TRX24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(NEW_TRX32) : wxBITMAP_PNG_FROM_DATA(NEW_TRX48);
    case NEW_NEWS: return x == 16 ? wxBITMAP_PNG_FROM_DATA(NEW_NEWS16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(NEW_NEWS24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(NEW_NEWS32) : wxBITMAP_PNG_FROM_DATA(NEW_NEWS48);
    case NEWS: return x == 16 ? wxBITMAP_PNG_FROM_DATA(NEWS16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(NEWS24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(NEWS32) : wxBITMAP_PNG_FROM_DATA(NEWS48);
    case CURRATES: return x == 16 ? wxBITMAP_PNG_FROM_DATA(CURRATES16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(CURRATES24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(CURRATES32) : wxBITMAP_PNG_FROM_DATA(CURRATES48);
    case FULLSCREEN: return x == 16 ? wxBITMAP_PNG_FROM_DATA(FULLSCREEN16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(FULLSCREEN24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(FULLSCREEN32) : wxBITMAP_PNG_FROM_DATA(FULLSCREEN48);
    case PRINT: return x == 16 ? wxBITMAP_PNG_FROM_DATA(PRINT16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(PRINT24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(PRINT32) : wxBITMAP_PNG_FROM_DATA(PRINT48);
    case ABOUT: return x == 16 ? wxBITMAP_PNG_FROM_DATA(ABOUT16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(ABOUT24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(ABOUT32) : wxBITMAP_PNG_FROM_DATA(ABOUT48);
    case HELP: return x == 16 ? wxBITMAP_PNG_FROM_DATA(HELP16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(HELP24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(HELP32) : wxBITMAP_PNG_FROM_DATA(HELP48);

    // Navigation
    case ALLTRANSACTIONS: return x == 16 ? wxBITMAP_PNG_FROM_DATA(ALLTRANSACTIONS16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(ALLTRANSACTIONS24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(ALLTRANSACTIONS32) : wxBITMAP_PNG_FROM_DATA(ALLTRANSACTIONS48);
    case FAVOURITE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(FAVOURITE16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(FAVOURITE24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(FAVOURITE32) : wxBITMAP_PNG_FROM_DATA(FAVOURITE48);
    case SAVINGS_NORMAL: return x == 16 ? wxBITMAP_PNG_FROM_DATA(SAVINGS_NORMAL16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(SAVINGS_NORMAL24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(SAVINGS_NORMAL32) : wxBITMAP_PNG_FROM_DATA(SAVINGS_NORMAL48);
    case SAVINGS_CLOSED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(SAVINGS_CLOSED16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(SAVINGS_CLOSED24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(SAVINGS_CLOSED32) : wxBITMAP_PNG_FROM_DATA(SAVINGS_CLOSED48);
    case CC_NORMAL: return x == 16 ? wxBITMAP_PNG_FROM_DATA(CC_NORMAL16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(CC_NORMAL24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(CC_NORMAL32) : wxBITMAP_PNG_FROM_DATA(CC_NORMAL48);
    case CC_CLOSED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(CC_CLOSED16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(CC_CLOSED24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(CC_CLOSED32) : wxBITMAP_PNG_FROM_DATA(CC_CLOSED48);
    case CASH_NORMAL: return x == 16 ? wxBITMAP_PNG_FROM_DATA(CASH_NORMAL16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(CASH_NORMAL24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(CASH_NORMAL32) : wxBITMAP_PNG_FROM_DATA(CASH_NORMAL48);
    case CASH_CLOSED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(CASH_CLOSED16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(CASH_CLOSED24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(CASH_CLOSED32) : wxBITMAP_PNG_FROM_DATA(CASH_CLOSED48);
    case LOAN_ACC_NORMAL: return x == 16 ? wxBITMAP_PNG_FROM_DATA(LOAN_NORMAL16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(LOAN_NORMAL24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(LOAN_NORMAL32) : wxBITMAP_PNG_FROM_DATA(LOAN_NORMAL48);
    case LOAN_ACC_CLOSED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(LOAN_CLOSED16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(LOAN_CLOSED24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(LOAN_CLOSED32) : wxBITMAP_PNG_FROM_DATA(LOAN_CLOSED48);
    case TERM_NORMAL: return x == 16 ? wxBITMAP_PNG_FROM_DATA(TERM_NORMAL16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(TERM_NORMAL24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(TERM_NORMAL32) : wxBITMAP_PNG_FROM_DATA(TERM_NORMAL48);
    case TERM_CLOSED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(TERM_CLOSED16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(TERM_CLOSED24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(TERM_CLOSED32) : wxBITMAP_PNG_FROM_DATA(TERM_CLOSED48);
    case STOCKS_NORMAL: return x == 16 ? wxBITMAP_PNG_FROM_DATA(STOCKS_NORMAL16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(STOCKS_NORMAL24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(STOCKS_NORMAL32) : wxBITMAP_PNG_FROM_DATA(STOCKS_NORMAL48);
    case STOCKS_CLOSED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(STOCKS_CLOSED16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(STOCKS_CLOSED24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(STOCKS_CLOSED32) : wxBITMAP_PNG_FROM_DATA(STOCKS_CLOSED48);
    case ASSET_NORMAL: return x == 16 ? wxBITMAP_PNG_FROM_DATA(ASSET_NORMAL16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(ASSET_NORMAL24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(ASSET_NORMAL32) : wxBITMAP_PNG_FROM_DATA(ASSET_NORMAL48);
    case ASSET_CLOSED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(ASSET_CLOSED16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(ASSET_CLOSED24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(ASSET_CLOSED32) : wxBITMAP_PNG_FROM_DATA(ASSET_CLOSED48);
    case RECURRING: return x == 16 ? wxBITMAP_PNG_FROM_DATA(RECURRING16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(RECURRING24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(RECURRING32) : wxBITMAP_PNG_FROM_DATA(RECURRING48);
    case BUDGET: return x == 16 ? wxBITMAP_PNG_FROM_DATA(BUDGET16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(BUDGET24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(BUDGET32) : wxBITMAP_PNG_FROM_DATA(BUDGET48);   
    case PIE_CHART: return x == 16 ? wxBITMAP_PNG_FROM_DATA(PIE_CHART16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(PIE_CHART24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(PIE_CHART32) : wxBITMAP_PNG_FROM_DATA(PIE_CHART48);

    // Status
    case RECONCILED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(RECONCILED16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(RECONCILED24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(RECONCILED32) : wxBITMAP_PNG_FROM_DATA(RECONCILED48);
    case DUPLICATE_STAT: return x == 16 ? wxBITMAP_PNG_FROM_DATA(DUPLICATE_STAT16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(DUPLICATE_STAT24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(DUPLICATE_STAT32) : wxBITMAP_PNG_FROM_DATA(DUPLICATE_STAT48);
    case FOLLOW_UP: return x == 16 ? wxBITMAP_PNG_FROM_DATA(FOLLOW_UP16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(FOLLOW_UP24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(FOLLOW_UP32) : wxBITMAP_PNG_FROM_DATA(FOLLOW_UP48);
    case VOID_STAT: return x == 16 ? wxBITMAP_PNG_FROM_DATA(VOID_STAT16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(VOID_STAT24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(VOID_STAT32) : wxBITMAP_PNG_FROM_DATA(VOID_STAT48);
    case PROFIT: return x == 16 ? wxBITMAP_PNG_FROM_DATA(PROFIT16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(PROFIT24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(PROFIT32) : wxBITMAP_PNG_FROM_DATA(PROFIT48);
    case LOSS: return x == 16 ? wxBITMAP_PNG_FROM_DATA(LOSS16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(LOSS24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(LOSS32) : wxBITMAP_PNG_FROM_DATA(LOSS48);

    // Assets
    case PROPERTY: return x == 16 ? wxBITMAP_PNG_FROM_DATA(PROPERTY16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(PROPERTY24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(PROPERTY32) : wxBITMAP_PNG_FROM_DATA(PROPERTY48);
    case CAR: return x == 16 ? wxBITMAP_PNG_FROM_DATA(CAR16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(CAR24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(CAR32) : wxBITMAP_PNG_FROM_DATA(CAR48);
    case HOUSEHOLD_OBJ: return x == 16 ? wxBITMAP_PNG_FROM_DATA(HOUSEHOLD_OBJ16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(HOUSEHOLD_OBJ24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(HOUSEHOLD_OBJ32) : wxBITMAP_PNG_FROM_DATA(HOUSEHOLD_OBJ48);
    case ART: return x == 16 ? wxBITMAP_PNG_FROM_DATA(ART16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(ART24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(ART32) : wxBITMAP_PNG_FROM_DATA(ART48);
    case JEWELLERY: return x == 16 ? wxBITMAP_PNG_FROM_DATA(JEWELLERY16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(JEWELLERY24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(JEWELLERY32) : wxBITMAP_PNG_FROM_DATA(JEWELLERY48);
    case CASH: return x == 16 ? wxBITMAP_PNG_FROM_DATA(CASH16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(CASH24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(CASH32) : wxBITMAP_PNG_FROM_DATA(CASH48);
    case OTHER: return x == 16 ? wxBITMAP_PNG_FROM_DATA(OTHER16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(OTHER24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(OTHER32) : wxBITMAP_PNG_FROM_DATA(OTHER48);
     
    // Controls
    case DOWNARROW: return x == 16 ? wxBITMAP_PNG_FROM_DATA(DOWNARROW16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(DOWNARROW24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(DOWNARROW32) : wxBITMAP_PNG_FROM_DATA(DOWNARROW48);
    case UPARROW: return x == 16 ? wxBITMAP_PNG_FROM_DATA(UPARROW16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(UPARROW24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(UPARROW32) : wxBITMAP_PNG_FROM_DATA(UPARROW48);
    case CLIP: return x == 16 ? wxBITMAP_PNG_FROM_DATA(CLIP16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(CLIP24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(CLIP32) : wxBITMAP_PNG_FROM_DATA(CLIP48);
    case WEB: return x == 16 ? wxBITMAP_PNG_FROM_DATA(WEB16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(WEB24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(WEB32) : wxBITMAP_PNG_FROM_DATA(WEB48);

    // Settings
    case GENERAL: return x == 16 ? wxBITMAP_PNG_FROM_DATA(GENERAL16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(GENERAL24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(GENERAL32) : wxBITMAP_PNG_FROM_DATA(GENERAL48);
    case VIEW: return x == 16 ? wxBITMAP_PNG_FROM_DATA(VIEW16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(VIEW24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(VIEW32) : wxBITMAP_PNG_FROM_DATA(VIEW48);
    case ATTACHMENTS: return x == 16 ? wxBITMAP_PNG_FROM_DATA(ATTACHMENTS16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(ATTACHMENTS24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(ATTACHMENTS32) : wxBITMAP_PNG_FROM_DATA(ATTACHMENTS48);
    case NETWORK: return x == 16 ? wxBITMAP_PNG_FROM_DATA(NETWORK16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(NETWORK24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(NETWORK32) : wxBITMAP_PNG_FROM_DATA(NETWORK48);
    case OTHERS: return x == 16 ? wxBITMAP_PNG_FROM_DATA(OTHERS16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(OTHERS24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(OTHERS32) : wxBITMAP_PNG_FROM_DATA(OTHERS48);
  
    //Extra Account Icons
    case ACC_ABOUT: return x == 16 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_ABOUT16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_ABOUT24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_ABOUT32) : wxBITMAP_PNG_FROM_DATA(ACC_ICON_ABOUT48);
    case ACC_CAT: return x == 16 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_CAT16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_CAT24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_CAT32) : wxBITMAP_PNG_FROM_DATA(ACC_ICON_CAT48);
    case ACC_CLOCK: return x == 16 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_CLOCK16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_CLOCK24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_CLOCK32) : wxBITMAP_PNG_FROM_DATA(ACC_ICON_CLOCK48);
    case ACC_COINS: return x == 16 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_COINS16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_COINS24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_COINS32) : wxBITMAP_PNG_FROM_DATA(ACC_ICON_COINS48);
    case ACC_DOG: return x == 16 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_DOG16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_DOG24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_DOG32) : wxBITMAP_PNG_FROM_DATA(ACC_ICON_DOG48);
    case ACC_MONEY: return x == 16 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_MONEY16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_MONEY24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_MONEY32) : wxBITMAP_PNG_FROM_DATA(ACC_ICON_MONEY48);
    case ACC_EURO: return x == 16 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_EURO16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_EURO24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_EURO32) : wxBITMAP_PNG_FROM_DATA(ACC_ICON_EURO48);
    case ACC_FLAG: return x == 16 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_FLAG16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_FLAG24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_FLAG32) : wxBITMAP_PNG_FROM_DATA(ACC_ICON_FLAG48);
    case ACC_HOURGLASS: return x == 16 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_HOURGLASS16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_HOURGLASS24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_HOURGLASS32) : wxBITMAP_PNG_FROM_DATA(ACC_ICON_HOURGLASS48);
    case ACC_PAYPAL: return x == 16 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_PAYPAL16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_PAYPAL24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_PAYPAL32) : wxBITMAP_PNG_FROM_DATA(ACC_ICON_PAYPAL48);
    case ACC_RUBIK: return x == 16 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_RUBIK16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_RUBIK24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_RUBIK32) : wxBITMAP_PNG_FROM_DATA(ACC_ICON_RUBIK48);
    case ACC_TREES: return x == 16 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_TREES16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_TREES24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_TREES32) : wxBITMAP_PNG_FROM_DATA(ACC_ICON_TREES48);
    case ACC_WALLET: return x == 16 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_WALLET16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_WALLET24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_WALLET32) : wxBITMAP_PNG_FROM_DATA(ACC_ICON_WALLET48);
    case ACC_WORK: return x == 16 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_WORK16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_WORK24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(ACC_ICON_WORK32) : wxBITMAP_PNG_FROM_DATA(ACC_ICON_WORK48);
 

    // To Do
    case CATEGORY_RELOCATION: return x == 16 ? wxBITMAP_PNG_FROM_DATA(category_relocation) : x == 24 ? wxBITMAP_PNG_FROM_DATA(category_relocation24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(category_relocation32) : wxBITMAP_PNG_FROM_DATA(category_relocation48);
    case CLEAR: return x == 16 ? wxBITMAP_PNG_FROM_DATA(clear) : x == 24 ? wxBITMAP_PNG_FROM_DATA(clear24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(clear32) : wxBITMAP_PNG_FROM_DATA(clear48);
    case EMPTY: return x == 16 ? wxBITMAP_PNG_FROM_DATA(empty) : x == 24 ? wxBITMAP_PNG_FROM_DATA(empty24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(empty32) : wxBITMAP_PNG_FROM_DATA(empty48);
    case LEFTARROWSIMPLE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(simplearrow_left) : x == 24 ? wxBITMAP_PNG_FROM_DATA(simplearrow_left24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(simplearrow_left32) : wxBITMAP_PNG_FROM_DATA(simplearrow_left48);
    case RIGHTARROWSIMPLE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(simplearrow_right) : x == 24 ? wxBITMAP_PNG_FROM_DATA(simplearrow_right24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(simplearrow_right32) : wxBITMAP_PNG_FROM_DATA(simplearrow_right48);
    case IMPORT: return x == 16 ? wxBITMAP_PNG_FROM_DATA(import) : x == 24 ? wxBITMAP_PNG_FROM_DATA(import24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(import32) : wxBITMAP_PNG_FROM_DATA(import48);
    case LED_GREEN: return x == 16 ? wxBITMAP_PNG_FROM_DATA(led_green) : x == 24 ? wxBITMAP_PNG_FROM_DATA(led_green24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(led_green32) : wxBITMAP_PNG_FROM_DATA(led_green48);
    case LED_OFF: return x == 16 ? wxBITMAP_PNG_FROM_DATA(led_off) : x == 24 ? wxBITMAP_PNG_FROM_DATA(led_off24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(led_off32) : wxBITMAP_PNG_FROM_DATA(led_off48);
    case LED_RED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(led_red) : x == 24 ? wxBITMAP_PNG_FROM_DATA(led_red24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(led_red32) : wxBITMAP_PNG_FROM_DATA(led_red48);
    case LED_YELLOW: return x == 16 ? wxBITMAP_PNG_FROM_DATA(led_yellow) : x == 24 ? wxBITMAP_PNG_FROM_DATA(led_yellow24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(led_yellow32) : wxBITMAP_PNG_FROM_DATA(led_yellow48);
    case RIGHTARROW: return x == 16 ? wxBITMAP_PNG_FROM_DATA(rightarrow) : x == 24 ? wxBITMAP_PNG_FROM_DATA(rightarrow24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(rightarrow32) : wxBITMAP_PNG_FROM_DATA(rightarrow48);
    case RIGHTARROW_ACTIVE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(rightarrow_active) : x == 24 ? wxBITMAP_PNG_FROM_DATA(rightarrow_active24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(rightarrow_active32) : wxBITMAP_PNG_FROM_DATA(rightarrow_active48);
    case RUN: return x == 16 ? wxBITMAP_PNG_FROM_DATA(run) : x == 24 ? wxBITMAP_PNG_FROM_DATA(run24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(run32) : wxBITMAP_PNG_FROM_DATA(run48);
    case SAVE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(save) : x == 24 ? wxBITMAP_PNG_FROM_DATA(save24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(save32) : wxBITMAP_PNG_FROM_DATA(save48);
    case SAVEAS: return x == 16 ? wxBITMAP_PNG_FROM_DATA(saveas) : x == 24 ? wxBITMAP_PNG_FROM_DATA(saveas24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(saveas32) : wxBITMAP_PNG_FROM_DATA(saveas48);
    case TRXNUM: return x == 16 ? wxBITMAP_PNG_FROM_DATA(trxnum) : x == 24 ? wxBITMAP_PNG_FROM_DATA(trxnum24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(trxnum32) : wxBITMAP_PNG_FROM_DATA(trxnum48);
    case UPDATE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(update) : x == 24 ? wxBITMAP_PNG_FROM_DATA(update24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(update32) : wxBITMAP_PNG_FROM_DATA(update48);
     default: return x == 16 ? wxBITMAP_PNG_FROM_DATA(empty) : x == 24 ? wxBITMAP_PNG_FROM_DATA(empty24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(empty32) : wxBITMAP_PNG_FROM_DATA(empty48);
    }
}
