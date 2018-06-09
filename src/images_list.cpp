/*******************************************************
Copyright (C) 2014, 2015 Nikolay Akimov

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
#include <wx/image.h>
#include <wx/bitmap.h>
#include <map>

#include "../resources/about_png.h"
#include "../resources/appstart_png.h"
#include "../resources/art_png.h"
#include "../resources/asset_png.h"
#include "../resources/budget_png.h"
#include "../resources/bug_png.h"
#include "../resources/car_png.h"
#include "../resources/cash_account_png.h"
#include "../resources/category_png.h"
#include "../resources/category_relocation_png.h"
#include "../resources/change_password_png.h"
#include "../resources/clearlist_png.h"
#include "../resources/clip_png.h"
#include "../resources/coin_png.h"
#include "../resources/community_png.h"
#include "../resources/credit_card_png.h"
#include "../resources/currency_png.h"
#include "../resources/delete_account_png.h"
#include "../resources/downarrow_png.h"
#include "../resources/edit_account_png.h"
#include "../resources/exit_png.h"
#include "../resources/facebook_png.h"
#include "../resources/filter_png.h"
#include "../resources/forum_png.h"
#include "../resources/fullscreen_png.h"
#include "../resources/github_png.h"
#include "../resources/google_play_png.h"
#include "../resources/grm_png.h"
#include "../resources/clear_png.h"
#include "../resources/custom_fields_png.h"
#include "../resources/help_png.h"
#include "../resources/home_png.h"
#include "../resources/household_obj_png.h"
#include "../resources/import_png.h"
#include "../resources/jewellery_png.h"
#include "../resources/language_png.h"
#include "../resources/leds_png.h"
#include "../resources/loan_account_png.h"
#include "../resources/loss_png.h"
#include "../resources/new_acc_png.h"
#include "../resources/new_db_png.h"
#include "../resources/new_trx_png.h"
#include "../resources/news_png.h"
#include "../resources/nnews_png.h"
#include "../resources/open_png.h"
#include "../resources/options_png.h"
#include "../resources/padlock_png.h"
#include "../resources/payee_png.h"
#include "../resources/payee_relocation_png.h"
#include "../resources/pie_chart_png.h"
#include "../resources/pp_png.h"
#include "../resources/print_png.h"
#include "../resources/profit_png.h"
#include "../resources/property_png.h"
#include "../resources/puzzle_png.h"
#include "../resources/currates_png.h"
#include "../resources/reallocate_acc_png.h"
#include "../resources/recurring_png.h"
#include "../resources/reloc_png.h"
#include "../resources/rightarrow_png.h"
#include "../resources/run_png.h"
#include "../resources/save_png.h"
#include "../resources/saveas_png.h"
#include "../resources/savings_acc_png.h"
#include "../resources/simplearrow_left_png.h"
#include "../resources/simplearrow_right_png.h"
#include "../resources/slack_png.h"
#include "../resources/status_d_png.h"
#include "../resources/status_f_png.h"
#include "../resources/status_r_png.h"
#include "../resources/status_v_png.h"
#include "../resources/stocks_png.h"
#include "../resources/bitcoin_png.h"
#include "../resources/term_png.h"
#include "../resources/trxnum_png.h"
#include "../resources/uparrow_png.h"
#include "../resources/update_png.h"
#include "../resources/web_png.h"

//----------------------------------------------------------------------------
/* Include XPM Support */
#include "../resources/empty.xpm"

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
        , { SCHEDULE_PNG, mmBitmap(png::RECURRING) }
        , { CALENDAR_PNG, mmBitmap(png::BUDGET) }
        , { PIECHART_PNG, mmBitmap(png::PIE_CHART) }
        , { HELP_PNG, mmBitmap(png::HELP) }
        , { FILTER_PNG, mmBitmap(png::FILTER) }
        , { ASSET_PNG, mmBitmap(png::ASSET) }
        , { CRYPTO_PNG, mmBitmap(png::CRYPTO) }
        , { CUSTOMSQL_PNG, mmBitmap(png::GRM) }
        , { CUSTOMSQL_GRP_PNG, mmBitmap(png::GRM) } //TODO: GRM rep group ico
        , { SAVINGS_ACC_NORMAL_PNG, mmBitmap(png::SAVINGS_NORMAL) }
        , { SAVINGS_ACC_FAVORITE_PNG, mmBitmap(png::SAVINGS_FAVORITES) }
        , { SAVINGS_ACC_CLOSED_PNG, mmBitmap(png::SAVINGS_CLOSED) }
        , { CARD_ACC_PNG, mmBitmap(png::CC_NORMAL) }
        , { CARD_ACC_FAVORITE_PNG, mmBitmap(png::CC_FAVORITE) }
        , { CARD_ACC_CLOSED_PNG, mmBitmap(png::CC_CLOSED) }
        , { TERMACCOUNT_PNG, mmBitmap(png::TERM_NORMAL) }
        , { TERM_ACC_FAVORITE_PNG, mmBitmap(png::TERM_FAVORITE) }
        , { TERM_ACC_CLOSED_PNG, mmBitmap(png::TERM_CLOSED) }
        , { STOCK_ACC_PNG, mmBitmap(png::STOCKS_NORMAL) }
        , { STOCK_ACC_FAVORITE_PNG, mmBitmap(png::STOCKS_FAVORITE) }
        , { STOCK_ACC_CLOSED_PNG, mmBitmap(png::STOCKS_CLOSED) }
        , { CASH_ACC_NORMAL_PNG, mmBitmap(png::CASH_NORMAL) }
        , { CASH_ACC_FAVORITE_PNG, mmBitmap(png::CASH_FAVORITE) }
        , { CASH_ACC_CLOSED_PNG, mmBitmap(png::CASH_CLOSED) }
        , { LOAN_ACC_NORMAL_PNG, mmBitmap(png::LOAN_ACC_NORMAL) }
        , { LOAN_ACC_FAVORITE_PNG, mmBitmap(png::LOAN_ACC_FAVORITE) }
        , { LOAN_ACC_CLOSED_PNG, mmBitmap(png::LOAN_ACC_CLOSED) }
        , { CRYPTO_ACC_NORMAL_PNG, mmBitmap(png::CRYPTO_ACC_NORMAL) }
        , { CRYPTO_ACC_FAVORITE_PNG, mmBitmap(png::CRYPTO_ACC_FAVORITE) }
        , { CRYPTO_ACC_CLOSED_PNG, mmBitmap(png::CRYPTO_ACC_CLOSED) }
    };
};

// Custom icons for accounts
static const std::map<int, wxBitmap> acc_images()
{
    int x = Option::instance().IconSize();
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
    int x = Option::instance().IconSize();

    wxImageList* imageList = new wxImageList(x, x);
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
    int x = Option::instance().IconSize();

    switch (ref)
    {
    case NEW_DB: return x == 16 ? wxBITMAP_PNG_FROM_DATA(new_db) : x == 24 ? wxBITMAP_PNG_FROM_DATA(new_db24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(new_db32) : wxBITMAP_PNG_FROM_DATA(new_db48);
    case OPEN: return x == 16 ? wxBITMAP_PNG_FROM_DATA(open) : x == 24 ? wxBITMAP_PNG_FROM_DATA(open24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(open32) : wxBITMAP_PNG_FROM_DATA(open48);
    case SAVE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(save) : x == 24 ? wxBITMAP_PNG_FROM_DATA(save24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(save32) : wxBITMAP_PNG_FROM_DATA(save48);
    case SAVEAS: return x == 16 ? wxBITMAP_PNG_FROM_DATA(saveas) : x == 24 ? wxBITMAP_PNG_FROM_DATA(saveas24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(saveas32) : wxBITMAP_PNG_FROM_DATA(saveas48);
    case NEW_ACC: return x == 16 ? wxBITMAP_PNG_FROM_DATA(new_acc) : x == 24 ? wxBITMAP_PNG_FROM_DATA(new_acc24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(new_acc32) : wxBITMAP_PNG_FROM_DATA(new_acc48);
    case HOME: return x == 16 ? wxBITMAP_PNG_FROM_DATA(home) : x == 24 ? wxBITMAP_PNG_FROM_DATA(home24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(home32) : wxBITMAP_PNG_FROM_DATA(home48);
    case RELOCATION: return x == 16 ? wxBITMAP_PNG_FROM_DATA(reloc) : x == 24 ? wxBITMAP_PNG_FROM_DATA(reloc24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(reloc32) : wxBITMAP_PNG_FROM_DATA(reloc48);
    case CATEGORY: return x == 16 ? wxBITMAP_PNG_FROM_DATA(category) : x == 24 ? wxBITMAP_PNG_FROM_DATA(category24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(category32) : wxBITMAP_PNG_FROM_DATA(category48);
    case CATEGORY_RELOCATION: return x == 16 ? wxBITMAP_PNG_FROM_DATA(category_relocation) : x == 24 ? wxBITMAP_PNG_FROM_DATA(category_relocation24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(category_relocation32) : wxBITMAP_PNG_FROM_DATA(category_relocation48);
    case PAYEE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(payee) : x == 24 ? wxBITMAP_PNG_FROM_DATA(payee24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(payee32) : wxBITMAP_PNG_FROM_DATA(payee48);
    case PAYEE_RELOCATION: return x == 16 ? wxBITMAP_PNG_FROM_DATA(payee_relocation) : x == 24 ? wxBITMAP_PNG_FROM_DATA(payee_relocation24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(payee_relocation32) : wxBITMAP_PNG_FROM_DATA(payee_relocation48);
    case CURR: return x == 16 ? wxBITMAP_PNG_FROM_DATA(currency) : x == 24 ? wxBITMAP_PNG_FROM_DATA(currency24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(currency32) : wxBITMAP_PNG_FROM_DATA(currency48);
    case FILTER: return x == 16 ? wxBITMAP_PNG_FROM_DATA(filter) : x == 24 ? wxBITMAP_PNG_FROM_DATA(filter24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(filter32) : wxBITMAP_PNG_FROM_DATA(filter48);
    case GRM: return x == 16 ? wxBITMAP_PNG_FROM_DATA(grm) : x == 24 ? wxBITMAP_PNG_FROM_DATA(grm24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(grm32) : wxBITMAP_PNG_FROM_DATA(grm48);
    case CUSTOM: return x == 16 ? wxBITMAP_PNG_FROM_DATA(custom_fields) : x == 24 ? wxBITMAP_PNG_FROM_DATA(custom_fields24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(custom_fields32) : wxBITMAP_PNG_FROM_DATA(custom_fields48);
    case CLEAR: return x == 16 ? wxBITMAP_PNG_FROM_DATA(clear) : x == 24 ? wxBITMAP_PNG_FROM_DATA(clear24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(clear32) : wxBITMAP_PNG_FROM_DATA(clear48);
    case OPTIONS: return x == 16 ? wxBITMAP_PNG_FROM_DATA(options) : x == 24 ? wxBITMAP_PNG_FROM_DATA(options24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(options32) : wxBITMAP_PNG_FROM_DATA(options48);
    case NEW_TRX: return x == 16 ? wxBITMAP_PNG_FROM_DATA(new_trx) : x == 24 ? wxBITMAP_PNG_FROM_DATA(new_trx24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(new_trx32) : wxBITMAP_PNG_FROM_DATA(new_trx48);
    case LANG: return x == 16 ? wxBITMAP_PNG_FROM_DATA(language) : x == 24 ? wxBITMAP_PNG_FROM_DATA(language24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(language32) : wxBITMAP_PNG_FROM_DATA(language48);
    case ABOUT: return x == 16 ? wxBITMAP_PNG_FROM_DATA(about) : x == 24 ? wxBITMAP_PNG_FROM_DATA(about24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(about32) : wxBITMAP_PNG_FROM_DATA(about48);
    case HELP: return x == 16 ? wxBITMAP_PNG_FROM_DATA(help) : x == 24 ? wxBITMAP_PNG_FROM_DATA(help24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(help32) : wxBITMAP_PNG_FROM_DATA(help48);
    case COMMUNITY: return x == 16 ? wxBITMAP_PNG_FROM_DATA(community) : x == 24 ? wxBITMAP_PNG_FROM_DATA(community24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(community32) : wxBITMAP_PNG_FROM_DATA(community48);
    case GITHUB: return x == 16 ? wxBITMAP_PNG_FROM_DATA(github) : x == 24 ? wxBITMAP_PNG_FROM_DATA(github24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(github32) : wxBITMAP_PNG_FROM_DATA(github48);
    case SLACK: return x == 16 ? wxBITMAP_PNG_FROM_DATA(slack) : x == 24 ? wxBITMAP_PNG_FROM_DATA(slack24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(slack32) : wxBITMAP_PNG_FROM_DATA(slack48);
    case BUG: return x == 16 ? wxBITMAP_PNG_FROM_DATA(bug) : x == 24 ? wxBITMAP_PNG_FROM_DATA(bug24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(bug32) : wxBITMAP_PNG_FROM_DATA(bug48);
    case NEWS: return x == 16 ? wxBITMAP_PNG_FROM_DATA(news) : x == 24 ? wxBITMAP_PNG_FROM_DATA(news24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(news32) : wxBITMAP_PNG_FROM_DATA(news48);
    case NEW_NEWS: return x == 16 ? wxBITMAP_PNG_FROM_DATA(nnews) : x == 24 ? wxBITMAP_PNG_FROM_DATA(nnews24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(nnews32) : wxBITMAP_PNG_FROM_DATA(nnews48);
    case PP: return x == 16 ? wxBITMAP_PNG_FROM_DATA(pp) : x == 24 ? wxBITMAP_PNG_FROM_DATA(pp24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(pp32) : wxBITMAP_PNG_FROM_DATA(pp48);
    case FULLSCREEN: return x == 16 ? wxBITMAP_PNG_FROM_DATA(fullscreen) : x == 24 ? wxBITMAP_PNG_FROM_DATA(fullscreen24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(fullscreen32) : wxBITMAP_PNG_FROM_DATA(fullscreen48);
    case PIE_CHART: return x == 16 ? wxBITMAP_PNG_FROM_DATA(pie_chart) : x == 24 ? wxBITMAP_PNG_FROM_DATA(pie_chart24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(pie_chart32) : wxBITMAP_PNG_FROM_DATA(pie_chart48);
    case BUDGET: return x == 16 ? wxBITMAP_PNG_FROM_DATA(budget) : x == 24 ? wxBITMAP_PNG_FROM_DATA(budget24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(budget32) : wxBITMAP_PNG_FROM_DATA(budget48);
    case RECURRING: return x == 16 ? wxBITMAP_PNG_FROM_DATA(recurring) : x == 24 ? wxBITMAP_PNG_FROM_DATA(recurring24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(recurring32) : wxBITMAP_PNG_FROM_DATA(recurring48);
    case ASSET: return x == 16 ? wxBITMAP_PNG_FROM_DATA(asset) : x == 24 ? wxBITMAP_PNG_FROM_DATA(asset24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(asset32) : wxBITMAP_PNG_FROM_DATA(asset48);
    case PROPERTY: return x == 16 ? wxBITMAP_PNG_FROM_DATA(property) : x == 24 ? wxBITMAP_PNG_FROM_DATA(property24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(property32) : wxBITMAP_PNG_FROM_DATA(property48);
    case CAR: return x == 16 ? wxBITMAP_PNG_FROM_DATA(car) : x == 24 ? wxBITMAP_PNG_FROM_DATA(car24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(car32) : wxBITMAP_PNG_FROM_DATA(car48);
    case HOUSEHOLD_OBJ: return x == 16 ? wxBITMAP_PNG_FROM_DATA(household_obj) : x == 24 ? wxBITMAP_PNG_FROM_DATA(household_obj24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(household_obj32) : wxBITMAP_PNG_FROM_DATA(household_obj48);
    case ART: return x == 16 ? wxBITMAP_PNG_FROM_DATA(art) : x == 24 ? wxBITMAP_PNG_FROM_DATA(art24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(art32) : wxBITMAP_PNG_FROM_DATA(art48);
    case JEWELLERY: return x == 16 ? wxBITMAP_PNG_FROM_DATA(jewellery) : x == 24 ? wxBITMAP_PNG_FROM_DATA(jewellery24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(jewellery32) : wxBITMAP_PNG_FROM_DATA(jewellery48);
    case CASH: return x == 16 ? wxBITMAP_PNG_FROM_DATA(coin) : x == 24 ? wxBITMAP_PNG_FROM_DATA(coin24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(coin32) : wxBITMAP_PNG_FROM_DATA(coin48);
    case OTHER: return x == 16 ? wxBITMAP_PNG_FROM_DATA(puzzle) : x == 24 ? wxBITMAP_PNG_FROM_DATA(puzzle24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(puzzle32) : wxBITMAP_PNG_FROM_DATA(puzzle48);
    case FACEBOOK: return x == 16 ? wxBITMAP_PNG_FROM_DATA(facebook) : x == 24 ? wxBITMAP_PNG_FROM_DATA(facebook24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(facebook32) : wxBITMAP_PNG_FROM_DATA(facebook48);
    case SAVINGS_NORMAL: return x == 16 ? wxBITMAP_PNG_FROM_DATA(savings_acc_normal) : x == 24 ? wxBITMAP_PNG_FROM_DATA(savings_acc_normal24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(savings_acc_normal32) : wxBITMAP_PNG_FROM_DATA(savings_acc_normal48);
    case SAVINGS_FAVORITES: return x == 16 ? wxBITMAP_PNG_FROM_DATA(savings_acc_favorite) : x == 24 ? wxBITMAP_PNG_FROM_DATA(savings_acc_favorite24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(savings_acc_favorite32) : wxBITMAP_PNG_FROM_DATA(savings_acc_favorite48);
    case SAVINGS_CLOSED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(savings_acc_closed) : x == 24 ? wxBITMAP_PNG_FROM_DATA(savings_acc_closed24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(savings_acc_closed32) : wxBITMAP_PNG_FROM_DATA(savings_acc_closed48);
    case CASH_NORMAL: return x == 16 ? wxBITMAP_PNG_FROM_DATA(cash_acc_16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(cash_acc_24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(cash_acc_32) : wxBITMAP_PNG_FROM_DATA(cash_acc_48);
    case CASH_FAVORITE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(cash_fav_16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(cash_fav_24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(cash_fav_32) : wxBITMAP_PNG_FROM_DATA(cash_fav_48);
    case CASH_CLOSED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(cash_closed_16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(cash_closed_24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(cash_closed_32) : wxBITMAP_PNG_FROM_DATA(cash_closed_48);
    case LOAN_ACC_NORMAL: return x == 16 ? wxBITMAP_PNG_FROM_DATA(loan_normal_16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(loan_normal_24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(loan_normal_32) : wxBITMAP_PNG_FROM_DATA(loan_normal_48);
    case LOAN_ACC_FAVORITE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(loan_favorite_16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(loan_favorite_24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(loan_favorite_32) : wxBITMAP_PNG_FROM_DATA(loan_favorite_48);
    case LOAN_ACC_CLOSED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(loan_closed_16) : x == 24 ? wxBITMAP_PNG_FROM_DATA(loan_closed_24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(loan_closed_32) : wxBITMAP_PNG_FROM_DATA(loan_closed_48);
    case TERM_NORMAL: return x == 16 ? wxBITMAP_PNG_FROM_DATA(term) : x == 24 ? wxBITMAP_PNG_FROM_DATA(term24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(term32) : wxBITMAP_PNG_FROM_DATA(term48);
    case TERM_FAVORITE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(term_favorite) : x == 24 ? wxBITMAP_PNG_FROM_DATA(term_favorite24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(term_favorite32) : wxBITMAP_PNG_FROM_DATA(term_favorite48);
    case TERM_CLOSED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(term_closed) : x == 24 ? wxBITMAP_PNG_FROM_DATA(term_closed24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(term_closed32) : wxBITMAP_PNG_FROM_DATA(term_closed48);
    case CC_NORMAL: return x == 16 ? wxBITMAP_PNG_FROM_DATA(cc_normal) : x == 24 ? wxBITMAP_PNG_FROM_DATA(cc_normal24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(cc_normal32) : wxBITMAP_PNG_FROM_DATA(cc_normal48);
    case CC_FAVORITE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(cc_favorite) : x == 24 ? wxBITMAP_PNG_FROM_DATA(cc_favorite24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(cc_favorite32) : wxBITMAP_PNG_FROM_DATA(cc_favorite48);
    case CC_CLOSED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(cc_closed) : x == 24 ? wxBITMAP_PNG_FROM_DATA(cc_closed24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(cc_closed32) : wxBITMAP_PNG_FROM_DATA(cc_closed48);
    case STOCKS_NORMAL: return x == 16 ? wxBITMAP_PNG_FROM_DATA(stocks) : x == 24 ? wxBITMAP_PNG_FROM_DATA(stocks24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(stocks32) : wxBITMAP_PNG_FROM_DATA(stocks48);
    case STOCKS_FAVORITE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(stocks_favorite) : x == 24 ? wxBITMAP_PNG_FROM_DATA(stocks_favorite24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(stocks_favorite32) : wxBITMAP_PNG_FROM_DATA(stocks_favorite48);
    case STOCKS_CLOSED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(stocks_closed) : x == 24 ? wxBITMAP_PNG_FROM_DATA(stocks_closed24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(stocks_closed32) : wxBITMAP_PNG_FROM_DATA(stocks_closed48);
    case CRYPTO: return x == 16 ? wxBITMAP_PNG_FROM_DATA(bitcoin) : x == 24 ? wxBITMAP_PNG_FROM_DATA(bitcoin24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(bitcoin32) : wxBITMAP_PNG_FROM_DATA(bitcoin48);
    case CRYPTO_ACC_NORMAL: return x == 16 ? wxBITMAP_PNG_FROM_DATA(bitcoin) : x == 24 ? wxBITMAP_PNG_FROM_DATA(bitcoin24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(bitcoin32) : wxBITMAP_PNG_FROM_DATA(bitcoin48);
    case CRYPTO_ACC_FAVORITE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(bitcoin) : x == 24 ? wxBITMAP_PNG_FROM_DATA(bitcoin24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(bitcoin32) : wxBITMAP_PNG_FROM_DATA(bitcoin48);
    case CRYPTO_ACC_CLOSED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(bitcoin) : x == 24 ? wxBITMAP_PNG_FROM_DATA(bitcoin24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(bitcoin32) : wxBITMAP_PNG_FROM_DATA(bitcoin48);
    case RECONCILED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(status_r) : x == 24 ? wxBITMAP_PNG_FROM_DATA(status_r24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(status_r32) : wxBITMAP_PNG_FROM_DATA(status_r48);
    case VOID_STAT: return x == 16 ? wxBITMAP_PNG_FROM_DATA(status_v) : x == 24 ? wxBITMAP_PNG_FROM_DATA(status_v24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(status_v32) : wxBITMAP_PNG_FROM_DATA(status_v48);
    case FOLLOW_UP: return x == 16 ? wxBITMAP_PNG_FROM_DATA(status_f) : x == 24 ? wxBITMAP_PNG_FROM_DATA(status_f24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(status_f32) : wxBITMAP_PNG_FROM_DATA(status_f48);
    case DUPLICATE_STAT: return x == 16 ? wxBITMAP_PNG_FROM_DATA(status_d) : x == 24 ? wxBITMAP_PNG_FROM_DATA(status_d24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(status_d32) : wxBITMAP_PNG_FROM_DATA(status_d48);
	case TRXNUM: return x == 16 ? wxBITMAP_PNG_FROM_DATA(trxnum) : x == 24 ? wxBITMAP_PNG_FROM_DATA(trxnum24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(trxnum32) : wxBITMAP_PNG_FROM_DATA(trxnum48);
	case RIGHTARROW: return x == 16 ? wxBITMAP_PNG_FROM_DATA(rightarrow) : x == 24 ? wxBITMAP_PNG_FROM_DATA(rightarrow24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(rightarrow32) : wxBITMAP_PNG_FROM_DATA(rightarrow48);
    case RIGHTARROW_ACTIVE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(rightarrow_active) : x == 24 ? wxBITMAP_PNG_FROM_DATA(rightarrow_active24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(rightarrow_active32) : wxBITMAP_PNG_FROM_DATA(rightarrow_active48);
    case LEFTARROWSIMPLE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(simplearrow_left) : x == 24 ? wxBITMAP_PNG_FROM_DATA(simplearrow_left24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(simplearrow_left32) : wxBITMAP_PNG_FROM_DATA(simplearrow_left48);
    case RIGHTARROWSIMPLE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(simplearrow_right) : x == 24 ? wxBITMAP_PNG_FROM_DATA(simplearrow_right24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(simplearrow_right32) : wxBITMAP_PNG_FROM_DATA(simplearrow_right48);
    case GOOGLE_PLAY: return x == 16 ? wxBITMAP_PNG_FROM_DATA(google_play) : x == 24 ? wxBITMAP_PNG_FROM_DATA(google_play24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(google_play32) : wxBITMAP_PNG_FROM_DATA(google_play48);
    case UPDATE: return x == 16 ? wxBITMAP_PNG_FROM_DATA(update) : x == 24 ? wxBITMAP_PNG_FROM_DATA(update24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(update32) : wxBITMAP_PNG_FROM_DATA(update48);
	case CURRATES: return x == 16 ? wxBITMAP_PNG_FROM_DATA(currates) : x == 24 ? wxBITMAP_PNG_FROM_DATA(currates24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(currates32) : wxBITMAP_PNG_FROM_DATA(currates48);
	case FORUM: return x == 16 ? wxBITMAP_PNG_FROM_DATA(forum) : x == 24 ? wxBITMAP_PNG_FROM_DATA(forum24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(forum32) : wxBITMAP_PNG_FROM_DATA(forum48);
    case CLIP: return x == 16 ? wxBITMAP_PNG_FROM_DATA(clip) : x == 24 ? wxBITMAP_PNG_FROM_DATA(clip24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(clip32) : wxBITMAP_PNG_FROM_DATA(clip48);
    case PROFIT: return x == 16 ? wxBITMAP_PNG_FROM_DATA(profit) : x == 24 ? wxBITMAP_PNG_FROM_DATA(profit24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(profit32) : wxBITMAP_PNG_FROM_DATA(profit48);
    case LOSS: return x == 16 ? wxBITMAP_PNG_FROM_DATA(loss) : x == 24 ? wxBITMAP_PNG_FROM_DATA(loss24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(loss32) : wxBITMAP_PNG_FROM_DATA(loss48);
    case WEB: return x == 16 ? wxBITMAP_PNG_FROM_DATA(web) : x == 24 ? wxBITMAP_PNG_FROM_DATA(web24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(web32) : wxBITMAP_PNG_FROM_DATA(web48);
    case UPARROW: return x == 16 ? wxBITMAP_PNG_FROM_DATA(uparrow) : x == 24 ? wxBITMAP_PNG_FROM_DATA(uparrow24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(uparrow32) : wxBITMAP_PNG_FROM_DATA(uparrow48);
    case DOWNARROW: return x == 16 ? wxBITMAP_PNG_FROM_DATA(downarrow) : x == 24 ? wxBITMAP_PNG_FROM_DATA(downarrow24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(downarrow32) : wxBITMAP_PNG_FROM_DATA(downarrow48);
    case IMPORT: return x == 16 ? wxBITMAP_PNG_FROM_DATA(import) : x == 24 ? wxBITMAP_PNG_FROM_DATA(import24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(import32) : wxBITMAP_PNG_FROM_DATA(import48);
    // case LED_OFF: return x == 16 ? wxBITMAP_PNG_FROM_DATA(led_off) : x == 24 ? wxBITMAP_PNG_FROM_DATA(led_off24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(led_off32) : wxBITMAP_PNG_FROM_DATA(led_off48);
    case LED_RED: return x == 16 ? wxBITMAP_PNG_FROM_DATA(led_red) : x == 24 ? wxBITMAP_PNG_FROM_DATA(led_red24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(led_red32) : wxBITMAP_PNG_FROM_DATA(led_red48);
    case LED_YELLOW: return x == 16 ? wxBITMAP_PNG_FROM_DATA(led_yellow) : x == 24 ? wxBITMAP_PNG_FROM_DATA(led_yellow24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(led_yellow32) : wxBITMAP_PNG_FROM_DATA(led_yellow48);
    case LED_GREEN: return x == 16 ? wxBITMAP_PNG_FROM_DATA(led_green) : x == 24 ? wxBITMAP_PNG_FROM_DATA(led_green24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(led_green32) : wxBITMAP_PNG_FROM_DATA(led_green48);
    case RUN: return x == 16 ? wxBITMAP_PNG_FROM_DATA(run) : x == 24 ? wxBITMAP_PNG_FROM_DATA(run24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(run32) : wxBITMAP_PNG_FROM_DATA(run48);
    case AUTO: return x == 16 ? wxBITMAP_PNG_FROM_DATA(run) : x == 24 ? wxBITMAP_PNG_FROM_DATA(run24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(run32) : wxBITMAP_PNG_FROM_DATA(run48);
    case CLEARLIST: return x == 16 ? wxBITMAP_PNG_FROM_DATA(clearlist) : x == 24 ? wxBITMAP_PNG_FROM_DATA(clearlist24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(clearlist32) : wxBITMAP_PNG_FROM_DATA(clearlist48);
    case DELETE_ACC: return x == 16 ? wxBITMAP_PNG_FROM_DATA(delete_account) : x == 24 ? wxBITMAP_PNG_FROM_DATA(delete_account24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(delete_account32) : wxBITMAP_PNG_FROM_DATA(delete_account48);
    case EDIT_ACC: return x == 16 ? wxBITMAP_PNG_FROM_DATA(edit_account) : x == 24 ? wxBITMAP_PNG_FROM_DATA(edit_account24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(edit_account32) : wxBITMAP_PNG_FROM_DATA(edit_account48);
    case REALLOCATE_ACC: return x == 16 ? wxBITMAP_PNG_FROM_DATA(reallocate_acc) : x == 24 ? wxBITMAP_PNG_FROM_DATA(reallocate_acc24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(reallocate_acc32) : wxBITMAP_PNG_FROM_DATA(reallocate_acc48);
    case PRINT: return x == 16 ? wxBITMAP_PNG_FROM_DATA(print) : x == 24 ? wxBITMAP_PNG_FROM_DATA(print24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(print32) : wxBITMAP_PNG_FROM_DATA(print48);
    case APPSTART: return x == 16 ? wxBITMAP_PNG_FROM_DATA(appstart) : x == 24 ? wxBITMAP_PNG_FROM_DATA(appstart24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(appstart32) : wxBITMAP_PNG_FROM_DATA(appstart48);
    case ENCRYPT_DB: return x == 16 ? wxBITMAP_PNG_FROM_DATA(padlock) : x == 24 ? wxBITMAP_PNG_FROM_DATA(padlock24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(padlock32) : wxBITMAP_PNG_FROM_DATA(padlock48);
    case ENCRYPT_DB_EDIT: return x == 16 ? wxBITMAP_PNG_FROM_DATA(change_password) : x == 24 ? wxBITMAP_PNG_FROM_DATA(change_password24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(change_password32) : wxBITMAP_PNG_FROM_DATA(change_password48);
    case EXIT: return x == 16 ? wxBITMAP_PNG_FROM_DATA(exit) : x == 24 ? wxBITMAP_PNG_FROM_DATA(exit24) : x == 32 ? wxBITMAP_PNG_FROM_DATA(exit32) : wxBITMAP_PNG_FROM_DATA(exit48);
    case EMPTY: return x == 16 ? wxBitmap(wxImage(empty_xpm).Scale(16, 16)) : x == 24 ? wxBitmap(wxImage(empty_xpm).Scale(24, 24)) : x == 32 ? wxBitmap(wxImage(empty_xpm).Scale(32, 32)) : wxBitmap(wxImage(empty_xpm).Scale(48, 48));
    default: return wxBitmap(wxImage(empty_xpm).Scale(x, x));
    }
}