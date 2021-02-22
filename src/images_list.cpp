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
//#include "../resources/about_png.h"
//#include "../resources/alltransactions_png.h"
#include "../resources/appstart_png.h"
#include "../resources/art_png.h"
//#include "../resources/asset_png.h"
//#include "../resources/budget_png.h"
#include "../resources/bug_png.h"
#include "../resources/car_png.h"
//#include "../resources/cash_account_png.h"
//#include "../resources/category_png.h"
#include "../resources/category_relocation_png.h"
#include "../resources/change_password_png.h"
#include "../resources/clearlist_png.h"
#include "../resources/clear_png.h"
#include "../resources/clip_png.h"
#include "../resources/coin_png.h"
#include "../resources/community_png.h"
//#include "../resources/credit_card_png.h"
//#include "../resources/currates_png.h"
//#include "../resources/currency_png.h"
#include "../resources/delete_account_png.h"
#include "../resources/donate_png.h"
#include "../resources/downarrow_png.h"
#include "../resources/edit_account_png.h"
#include "../resources/empty_png.h"
#include "../resources/exit_png.h"
#include "../resources/facebook_png.h"
//#include "../resources/filter_png.h"
#include "../resources/forum_png.h"
//#include "../resources/fullscreen_png.h"
#include "../resources/github_png.h"
#include "../resources/globe_png.h"
#include "../resources/google_play_png.h"
#include "../resources/custom_fields_png.h"
//#include "../resources/grm_png.h"
//#include "../resources/help_png.h"
//#include "../resources/home_png.h"
#include "../resources/household_obj_png.h"
#include "../resources/import_png.h"
#include "../resources/jewellery_png.h"
#include "../resources/language_png.h"
#include "../resources/leds_png.h"
//#include "../resources/loan_account_png.h"
#include "../resources/loss_png.h"
//#include "../resources/new_acc_png.h"
//#include "../resources/new_db_png.h"
//#include "../resources/new_trx_png.h"
//#include "../resources/news_png.h"
//#include "../resources/nnews_png.h"
//#include "../resources/open_png.h"
//#include "../resources/options_png.h"
#include "../resources/padlock_png.h"
//#include "../resources/payee_png.h"
#include "../resources/payee_relocation_png.h"
//#include "../resources/pie_chart_png.h"
//#include "../resources/print_png.h"
#include "../resources/profit_png.h"
#include "../resources/property_png.h"
#include "../resources/puzzle_png.h"
#include "../resources/reallocate_acc_png.h"
#include "../resources/reloc_png.h"
//#include "../resources/recurring_png.h"
#include "../resources/rightarrow_png.h"
#include "../resources/run_png.h"
#include "../resources/save_png.h"
#include "../resources/saveas_png.h"
//#include "../resources/savings_acc_png.h"
#include "../resources/simplearrow_left_png.h"
#include "../resources/simplearrow_right_png.h"
#include "../resources/slack_png.h"
#include "../resources/status_d_png.h"
#include "../resources/status_f_png.h"
#include "../resources/status_r_png.h"
#include "../resources/status_v_png.h"
#include "../resources/stocks_png.h"
//#include "../resources/term_png.h"
#include "../resources/trxnum_png.h"
#include "../resources/uparrow_png.h"
#include "../resources/update_png.h"
#include "../resources/web_png.h"

//----------------------------------------------------------------------------
/* Include XPM Support */

#include "../resources/coins.xpm"
#include "../resources/clock.xpm"
#include "../resources/cat.xpm"
#include "../resources/dog.xpm"
#include "../resources/hourglass.xpm"
#include "../resources/dollar.xpm"
#include "../resources/euro.xpm"
#include "../resources/trees.xpm"
#include "../resources/wallet.xpm"
#include "../resources/work.xpm"
#include "../resources/paypal.xpm"

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
    int x = Option::instance().getIconSize();
    return
    {
        { MONEY_DOLLAR_XPM, wxBitmap(wxImage(dollar_xpm).Scale(x, x)) }
        , { MONEY_EURO_XPM, wxBitmap(wxImage(euro_xpm).Scale(x, x)) }
        , { FLAG_XPM, mmBitmap(png::FOLLOW_UP) }
        , { COINS_XPM, wxBitmap(wxImage(coins_xpm).Scale(x, x)) }
        , { ABOUT_XPM, mmBitmap(png::ABOUT) }
        , { CLOCK_XPM, wxBitmap(wxImage(clock_xpm).Scale(x, x)) }
        , { CAT_XPM, wxBitmap(wxImage(cat_xpm).Scale(x, x)) }
        , { DOG_XPM, wxBitmap(wxImage(dog_xpm).Scale(x, x)) }
        , { TREES_XPM, wxBitmap(wxImage(trees_xpm).Scale(x, x)) }
        , { HOURGLASS_XPM, wxBitmap(wxImage(hourglass_xpm).Scale(x, x)) }
        , { WORK_XPM, wxBitmap(wxImage(work_xpm).Scale(x, x)) }
        , { PAYPAL_XPM, wxBitmap(wxImage(paypal_xpm).Scale(x, x)) }
        , { WALLET_XPM, wxBitmap(wxImage(wallet_xpm).Scale(x, x)) }
        , { RUBIK_CUBE_XPM, mmBitmap(png::OTHER) }
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
    case ABOUT: return x == 16 ? wxBITMAP_PNG_FROM_DATA(ABOUT16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(ABOUT24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(ABOUT32) : wxBITMAP_PNG_FROM_DATA(ABOUT48);
    case ALLTRANSACTIONS: return x == 16 ? wxBITMAP_PNG_FROM_DATA(ALLTRANSACTIONS16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(ALLTRANSACTIONS24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(ALLTRANSACTIONS32) : wxBITMAP_PNG_FROM_DATA(ALLTRANSACTIONS48);
    case APPSTART: return x == 16 ? wxBITMAP_PNG_FROM_DATA(appstart) : x == 24 ? wxBITMAP_PNG_FROM_DATA(appstart24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(appstart32) : wxBITMAP_PNG_FROM_DATA(appstart48);
    case ART: return x == 16 ? wxBITMAP_PNG_FROM_DATA(ART16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(ART24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(ART32) : wxBITMAP_PNG_FROM_DATA(ART48);
    case ASSET_NORMAL: return x == 16 ? wxBITMAP_PNG_FROM_DATA(ASSET_NORMAL16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(ASSET_NORMAL24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(ASSET_NORMAL32) : wxBITMAP_PNG_FROM_DATA(ASSET_NORMAL48);
    case ASSET_CLOSED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(ASSET_CLOSED16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(ASSET_CLOSED24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(ASSET_CLOSED32) : wxBITMAP_PNG_FROM_DATA(ASSET_CLOSED48);
    case AUTO: return x == 16 ? wxBITMAP_PNG_FROM_DATA(run) : x == 24 ? wxBITMAP_PNG_FROM_DATA(run24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(run32) : wxBITMAP_PNG_FROM_DATA(run48);
    case BUDGET: return x == 16 ? wxBITMAP_PNG_FROM_DATA(BUDGET16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(BUDGET24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(BUDGET32) : wxBITMAP_PNG_FROM_DATA(BUDGET48);
    case BUG: return x == 16 ? wxBITMAP_PNG_FROM_DATA(bug) : x == 24 ? wxBITMAP_PNG_FROM_DATA(bug24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(bug32) : wxBITMAP_PNG_FROM_DATA(bug48);
    case CAR: return x == 16 ? wxBITMAP_PNG_FROM_DATA(CAR16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(CAR24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(CAR32) : wxBITMAP_PNG_FROM_DATA(CAR48);
    case CASH: return x == 16 ? wxBITMAP_PNG_FROM_DATA(CASH16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(CASH24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(CASH32) : wxBITMAP_PNG_FROM_DATA(CASH48);
    case CASH_CLOSED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(CASH_CLOSED16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(CASH_CLOSED24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(CASH_CLOSED32) : wxBITMAP_PNG_FROM_DATA(CASH_CLOSED48);
    case CASH_NORMAL: return x == 16 ? wxBITMAP_PNG_FROM_DATA(CASH_NORMAL16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(CASH_NORMAL24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(CASH_NORMAL32) : wxBITMAP_PNG_FROM_DATA(CASH_NORMAL48);
    case CATEGORY: return x == 16 ? wxBITMAP_PNG_FROM_DATA(CATEGORY16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(CATEGORY24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(CATEGORY32) : wxBITMAP_PNG_FROM_DATA(CATEGORY48);
    case CATEGORY_RELOCATION: return x == 16 ? wxBITMAP_PNG_FROM_DATA(category_relocation) : x == 24 ? wxBITMAP_PNG_FROM_DATA(category_relocation24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(category_relocation32) : wxBITMAP_PNG_FROM_DATA(category_relocation48);
    case CC_CLOSED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(CC_CLOSED16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(CC_CLOSED24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(CC_CLOSED32) : wxBITMAP_PNG_FROM_DATA(CC_CLOSED48);
    case CC_NORMAL: return x == 16 ? wxBITMAP_PNG_FROM_DATA(CC_NORMAL16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(CC_NORMAL24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(CC_NORMAL32) : wxBITMAP_PNG_FROM_DATA(CC_NORMAL48);
    case CLEAR: return x == 16 ? wxBITMAP_PNG_FROM_DATA(clear) : x == 24 ? wxBITMAP_PNG_FROM_DATA(clear24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(clear32) : wxBITMAP_PNG_FROM_DATA(clear48);
    case CLEARLIST: return x == 16 ? wxBITMAP_PNG_FROM_DATA(clearlist) : x == 24 ? wxBITMAP_PNG_FROM_DATA(clearlist24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(clearlist32) : wxBITMAP_PNG_FROM_DATA(clearlist48);
    case CLIP: return x == 16 ? wxBITMAP_PNG_FROM_DATA(clip) : x == 24 ? wxBITMAP_PNG_FROM_DATA(clip24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(clip32) : wxBITMAP_PNG_FROM_DATA(clip48);
    case COMMUNITY: return x == 16 ? wxBITMAP_PNG_FROM_DATA(community) : x == 24 ? wxBITMAP_PNG_FROM_DATA(community24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(community32) : wxBITMAP_PNG_FROM_DATA(community48);
    case CROWDIN: return x == 16 ? wxBITMAP_PNG_FROM_DATA(globe) : x == 24 ? wxBITMAP_PNG_FROM_DATA(globe24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(globe32) : wxBITMAP_PNG_FROM_DATA(globe48);
    case CURR: return x == 16 ? wxBITMAP_PNG_FROM_DATA(CURR16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(CURR24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(CURR32) : wxBITMAP_PNG_FROM_DATA(CURR48);
    case CURRATES: return x == 16 ? wxBITMAP_PNG_FROM_DATA(CURRATES16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(CURRATES24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(CURRATES32) : wxBITMAP_PNG_FROM_DATA(CURRATES48);
    case DELETE_ACC: return x == 16 ? wxBITMAP_PNG_FROM_DATA(delete_account) : x == 24 ? wxBITMAP_PNG_FROM_DATA(delete_account24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(delete_account32) : wxBITMAP_PNG_FROM_DATA(delete_account48);
    case DONATE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(donate) : x == 24 ? wxBITMAP_PNG_FROM_DATA(donate24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(donate32) : wxBITMAP_PNG_FROM_DATA(donate48);
    case DOWNARROW: return x == 16 ? wxBITMAP_PNG_FROM_DATA(DOWNARROW16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(DOWNARROW24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(DOWNARROW32) : wxBITMAP_PNG_FROM_DATA(DOWNARROW48);
    case DUPLICATE_STAT: return x == 16 ? wxBITMAP_PNG_FROM_DATA(DUPLICATE_STAT16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(DUPLICATE_STAT24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(DUPLICATE_STAT32) : wxBITMAP_PNG_FROM_DATA(DUPLICATE_STAT48);
    case EDIT_ACC: return x == 16 ? wxBITMAP_PNG_FROM_DATA(edit_account) : x == 24 ? wxBITMAP_PNG_FROM_DATA(edit_account24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(edit_account32) : wxBITMAP_PNG_FROM_DATA(edit_account48);
    case EMPTY: return x == 16 ? wxBITMAP_PNG_FROM_DATA(empty) : x == 24 ? wxBITMAP_PNG_FROM_DATA(empty24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(empty32) : wxBITMAP_PNG_FROM_DATA(empty48);
    case ENCRYPT_DB: return x == 16 ? wxBITMAP_PNG_FROM_DATA(padlock) : x == 24 ? wxBITMAP_PNG_FROM_DATA(padlock24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(padlock32) : wxBITMAP_PNG_FROM_DATA(padlock48);
    case ENCRYPT_DB_EDIT: return x == 16 ? wxBITMAP_PNG_FROM_DATA(change_password) : x == 24 ? wxBITMAP_PNG_FROM_DATA(change_password24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(change_password32) : wxBITMAP_PNG_FROM_DATA(change_password48);
    case EXIT: return x == 16 ? wxBITMAP_PNG_FROM_DATA(exit) : x == 24 ? wxBITMAP_PNG_FROM_DATA(exit24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(exit32) : wxBITMAP_PNG_FROM_DATA(exit48);
    case FACEBOOK: return x == 16 ? wxBITMAP_PNG_FROM_DATA(facebook) : x == 24 ? wxBITMAP_PNG_FROM_DATA(facebook24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(facebook32) : wxBITMAP_PNG_FROM_DATA(facebook48);
    case FAVOURITE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(FAVOURITE16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(FAVOURITE24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(FAVOURITE32) : wxBITMAP_PNG_FROM_DATA(FAVOURITE48);
    case FILTER: return x == 16 ? wxBITMAP_PNG_FROM_DATA(FILTER16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(FILTER24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(FILTER32) : wxBITMAP_PNG_FROM_DATA(FILTER48);
    case FOLLOW_UP: return x == 16 ? wxBITMAP_PNG_FROM_DATA(FOLLOW_UP16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(FOLLOW_UP24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(FOLLOW_UP32) : wxBITMAP_PNG_FROM_DATA(FOLLOW_UP48);
    case FORUM: return x == 16 ? wxBITMAP_PNG_FROM_DATA(forum) : x == 24 ? wxBITMAP_PNG_FROM_DATA(forum24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(forum32) : wxBITMAP_PNG_FROM_DATA(forum48);
    case FULLSCREEN: return x == 16 ? wxBITMAP_PNG_FROM_DATA(FULLSCREEN16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(FULLSCREEN24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(FULLSCREEN32) : wxBITMAP_PNG_FROM_DATA(FULLSCREEN48);
    case GITHUB: return x == 16 ? wxBITMAP_PNG_FROM_DATA(github) : x == 24 ? wxBITMAP_PNG_FROM_DATA(github24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(github32) : wxBITMAP_PNG_FROM_DATA(github48);
    case LEFTARROWSIMPLE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(simplearrow_left) : x == 24 ? wxBITMAP_PNG_FROM_DATA(simplearrow_left24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(simplearrow_left32) : wxBITMAP_PNG_FROM_DATA(simplearrow_left48);
    case RIGHTARROWSIMPLE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(simplearrow_right) : x == 24 ? wxBITMAP_PNG_FROM_DATA(simplearrow_right24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(simplearrow_right32) : wxBITMAP_PNG_FROM_DATA(simplearrow_right48);
    case GOOGLE_PLAY: return x == 16 ? wxBITMAP_PNG_FROM_DATA(google_play) : x == 24 ? wxBITMAP_PNG_FROM_DATA(google_play24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(google_play32) : wxBITMAP_PNG_FROM_DATA(google_play48);
    case CUSTOM: return x == 16 ? wxBITMAP_PNG_FROM_DATA(custom_fields) : x == 24 ? wxBITMAP_PNG_FROM_DATA(custom_fields24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(custom_fields32) : wxBITMAP_PNG_FROM_DATA(custom_fields48);
    case GRM: return x == 16 ? wxBITMAP_PNG_FROM_DATA(GRM16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(GRM24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(GRM32) : wxBITMAP_PNG_FROM_DATA(GRM48);
    case HELP: return x == 16 ? wxBITMAP_PNG_FROM_DATA(HELP16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(HELP24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(HELP32) : wxBITMAP_PNG_FROM_DATA(HELP48);
    case HOME: return x == 16 ? wxBITMAP_PNG_FROM_DATA(HOME16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(HOME24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(HOME32) : wxBITMAP_PNG_FROM_DATA(HOME48);
    case RELOCATION: return x == 16 ? wxBITMAP_PNG_FROM_DATA(reloc) : x == 24 ? wxBITMAP_PNG_FROM_DATA(reloc24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(reloc32) : wxBITMAP_PNG_FROM_DATA(reloc48);
    case HOUSEHOLD_OBJ: return x == 16 ? wxBITMAP_PNG_FROM_DATA(HOUSEHOLD_OBJ16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(HOUSEHOLD_OBJ24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(HOUSEHOLD_OBJ32) : wxBITMAP_PNG_FROM_DATA(HOUSEHOLD_OBJ48);
    case IMPORT: return x == 16 ? wxBITMAP_PNG_FROM_DATA(import) : x == 24 ? wxBITMAP_PNG_FROM_DATA(import24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(import32) : wxBITMAP_PNG_FROM_DATA(import48);
    case JEWELLERY: return x == 16 ? wxBITMAP_PNG_FROM_DATA(JEWELLERY16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(JEWELLERY24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(JEWELLERY32) : wxBITMAP_PNG_FROM_DATA(JEWELLERY48);
    case LANG: return x == 16 ? wxBITMAP_PNG_FROM_DATA(language) : x == 24 ? wxBITMAP_PNG_FROM_DATA(language24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(language32) : wxBITMAP_PNG_FROM_DATA(language48);
    case LED_GREEN: return x == 16 ? wxBITMAP_PNG_FROM_DATA(led_green) : x == 24 ? wxBITMAP_PNG_FROM_DATA(led_green24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(led_green32) : wxBITMAP_PNG_FROM_DATA(led_green48);
    case LED_OFF: return x == 16 ? wxBITMAP_PNG_FROM_DATA(led_off) : x == 24 ? wxBITMAP_PNG_FROM_DATA(led_off24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(led_off32) : wxBITMAP_PNG_FROM_DATA(led_off48);
    case LED_RED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(led_red) : x == 24 ? wxBITMAP_PNG_FROM_DATA(led_red24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(led_red32) : wxBITMAP_PNG_FROM_DATA(led_red48);
    case LED_YELLOW: return x == 16 ? wxBITMAP_PNG_FROM_DATA(led_yellow) : x == 24 ? wxBITMAP_PNG_FROM_DATA(led_yellow24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(led_yellow32) : wxBITMAP_PNG_FROM_DATA(led_yellow48);
    case LOAN_ACC_CLOSED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(LOAN_CLOSED16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(LOAN_CLOSED24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(LOAN_CLOSED32) : wxBITMAP_PNG_FROM_DATA(LOAN_CLOSED48);
    case LOAN_ACC_NORMAL: return x == 16 ? wxBITMAP_PNG_FROM_DATA(LOAN_NORMAL16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(LOAN_NORMAL24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(LOAN_NORMAL32) : wxBITMAP_PNG_FROM_DATA(LOAN_NORMAL48);
    case LOSS: return x == 16 ? wxBITMAP_PNG_FROM_DATA(loss) : x == 24 ? wxBITMAP_PNG_FROM_DATA(loss24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(loss32) : wxBITMAP_PNG_FROM_DATA(loss48);
    case NEW_ACC: return x == 16 ? wxBITMAP_PNG_FROM_DATA(NEW_ACC16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(NEW_ACC24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(NEW_ACC32) : wxBITMAP_PNG_FROM_DATA(NEW_ACC48);
    case NEW_DB: return x == 16 ? wxBITMAP_PNG_FROM_DATA(NEW_DB16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(NEW_DB24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(NEW_DB32) : wxBITMAP_PNG_FROM_DATA(NEW_DB48);
    case NEW_NEWS: return x == 16 ? wxBITMAP_PNG_FROM_DATA(NEW_NEWS16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(NEW_NEWS24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(NEW_NEWS32) : wxBITMAP_PNG_FROM_DATA(NEW_NEWS48);
    case NEW_TRX: return x == 16 ? wxBITMAP_PNG_FROM_DATA(NEW_TRX16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(NEW_TRX24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(NEW_TRX32) : wxBITMAP_PNG_FROM_DATA(NEW_TRX48);
    case NEWS: return x == 16 ? wxBITMAP_PNG_FROM_DATA(NEWS16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(NEWS24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(NEWS32) : wxBITMAP_PNG_FROM_DATA(NEWS48);
    case OPEN: return x == 16 ? wxBITMAP_PNG_FROM_DATA(OPEN16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(OPEN24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(OPEN32) : wxBITMAP_PNG_FROM_DATA(OPEN48);
    case OPTIONS: return x == 16 ? wxBITMAP_PNG_FROM_DATA(OPTIONS16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(OPTIONS24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(OPTIONS32) : wxBITMAP_PNG_FROM_DATA(OPTIONS48);
    case OTHER: return x == 16 ? wxBITMAP_PNG_FROM_DATA(OTHER16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(OTHER24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(OTHER32) : wxBITMAP_PNG_FROM_DATA(OTHER48);
    case PAYEE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(PAYEE16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(PAYEE24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(PAYEE32) : wxBITMAP_PNG_FROM_DATA(PAYEE48);
    case PAYEE_RELOCATION: return x == 16 ? wxBITMAP_PNG_FROM_DATA(payee_relocation) : x == 24 ? wxBITMAP_PNG_FROM_DATA(payee_relocation24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(payee_relocation32) : wxBITMAP_PNG_FROM_DATA(payee_relocation48);
    case PIE_CHART: return x == 16 ? wxBITMAP_PNG_FROM_DATA(PIE_CHART16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(PIE_CHART24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(PIE_CHART32) : wxBITMAP_PNG_FROM_DATA(PIE_CHART48);
    case PRINT: return x == 16 ? wxBITMAP_PNG_FROM_DATA(PRINT16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(PRINT24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(PRINT32) : wxBITMAP_PNG_FROM_DATA(PRINT48);
    case PROFIT: return x == 16 ? wxBITMAP_PNG_FROM_DATA(profit) : x == 24 ? wxBITMAP_PNG_FROM_DATA(profit24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(profit32) : wxBITMAP_PNG_FROM_DATA(profit48);
    case PROPERTY: return x == 16 ? wxBITMAP_PNG_FROM_DATA(PROPERTY16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(PROPERTY24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(PROPERTY32) : wxBITMAP_PNG_FROM_DATA(PROPERTY48);
    case REALLOCATE_ACC: return x == 16 ? wxBITMAP_PNG_FROM_DATA(reallocate_acc) : x == 24 ? wxBITMAP_PNG_FROM_DATA(reallocate_acc24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(reallocate_acc32) : wxBITMAP_PNG_FROM_DATA(reallocate_acc48);
    case RECONCILED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(RECONCILED16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(RECONCILED24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(RECONCILED32) : wxBITMAP_PNG_FROM_DATA(RECONCILED48);
    case RECURRING: return x == 16 ? wxBITMAP_PNG_FROM_DATA(RECURRING16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(RECURRING24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(RECURRING32) : wxBITMAP_PNG_FROM_DATA(RECURRING48);
    case RIGHTARROW: return x == 16 ? wxBITMAP_PNG_FROM_DATA(rightarrow) : x == 24 ? wxBITMAP_PNG_FROM_DATA(rightarrow24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(rightarrow32) : wxBITMAP_PNG_FROM_DATA(rightarrow48);
    case RIGHTARROW_ACTIVE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(rightarrow_active) : x == 24 ? wxBITMAP_PNG_FROM_DATA(rightarrow_active24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(rightarrow_active32) : wxBITMAP_PNG_FROM_DATA(rightarrow_active48);
    case RUN: return x == 16 ? wxBITMAP_PNG_FROM_DATA(run) : x == 24 ? wxBITMAP_PNG_FROM_DATA(run24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(run32) : wxBITMAP_PNG_FROM_DATA(run48);
    case SAVE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(save) : x == 24 ? wxBITMAP_PNG_FROM_DATA(save24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(save32) : wxBITMAP_PNG_FROM_DATA(save48);
    case SAVEAS: return x == 16 ? wxBITMAP_PNG_FROM_DATA(saveas) : x == 24 ? wxBITMAP_PNG_FROM_DATA(saveas24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(saveas32) : wxBITMAP_PNG_FROM_DATA(saveas48);
    case SAVINGS_CLOSED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(SAVINGS_CLOSED16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(SAVINGS_CLOSED24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(SAVINGS_CLOSED32) : wxBITMAP_PNG_FROM_DATA(SAVINGS_CLOSED48);
    case SAVINGS_NORMAL: return x == 16 ? wxBITMAP_PNG_FROM_DATA(SAVINGS_NORMAL16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(SAVINGS_NORMAL24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(SAVINGS_NORMAL32) : wxBITMAP_PNG_FROM_DATA(SAVINGS_NORMAL48);
    case SLACK: return x == 16 ? wxBITMAP_PNG_FROM_DATA(slack) : x == 24 ? wxBITMAP_PNG_FROM_DATA(slack24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(slack32) : wxBITMAP_PNG_FROM_DATA(slack48);
    case STOCKS_CLOSED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(STOCKS_CLOSED16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(STOCKS_CLOSED24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(STOCKS_CLOSED32) : wxBITMAP_PNG_FROM_DATA(STOCKS_CLOSED48);
    case STOCKS_NORMAL: return x == 16 ? wxBITMAP_PNG_FROM_DATA(STOCKS_NORMAL16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(STOCKS_NORMAL24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(STOCKS_NORMAL32) : wxBITMAP_PNG_FROM_DATA(STOCKS_NORMAL48);
    case TERM_CLOSED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(TERM_CLOSED16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(TERM_CLOSED24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(TERM_CLOSED32) : wxBITMAP_PNG_FROM_DATA(TERM_CLOSED48);
    case TERM_NORMAL: return x == 16 ? wxBITMAP_PNG_FROM_DATA(TERM_NORMAL16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(TERM_NORMAL24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(TERM_NORMAL32) : wxBITMAP_PNG_FROM_DATA(TERM_NORMAL48);
    case TRXNUM: return x == 16 ? wxBITMAP_PNG_FROM_DATA(trxnum) : x == 24 ? wxBITMAP_PNG_FROM_DATA(trxnum24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(trxnum32) : wxBITMAP_PNG_FROM_DATA(trxnum48);
    case UPARROW: return x == 16 ? wxBITMAP_PNG_FROM_DATA(UPARROW16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(UPARROW24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(UPARROW32) : wxBITMAP_PNG_FROM_DATA(UPARROW48);
    case UPDATE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(update) : x == 24 ? wxBITMAP_PNG_FROM_DATA(update24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(update32) : wxBITMAP_PNG_FROM_DATA(update48);
    case VOID_STAT: return x == 16 ? wxBITMAP_PNG_FROM_DATA(VOID_STAT16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(VOID_STAT24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(VOID_STAT32) : wxBITMAP_PNG_FROM_DATA(VOID_STAT48);
    case WEB: return x == 16 ? wxBITMAP_PNG_FROM_DATA(web) : x == 24 ? wxBITMAP_PNG_FROM_DATA(web24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(web32) : wxBITMAP_PNG_FROM_DATA(web48);
    default: return x == 16 ? wxBITMAP_PNG_FROM_DATA(empty) : x == 24 ? wxBITMAP_PNG_FROM_DATA(empty24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(empty32) : wxBITMAP_PNG_FROM_DATA(empty48);
    }
}
