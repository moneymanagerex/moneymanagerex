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

#include "images_list.h"
#include "mmOption.h"
#include <wx/image.h>
#include <wx/bitmap.h>
#include <wx/artprov.h>
#include <map>

#include "../resources/new_db_png.h"
#include "../resources/open_png.h"
#include "../resources/new_acc_png.h"
#include "../resources/home_png.h"
#include "../resources/category_png.h"
#include "../resources/payee_png.h"
#include "../resources/currency_png.h"
#include "../resources/filter_png.h"
#include "../resources/grm_png.h"
#include "../resources/new_trx_png.h"
#include "../resources/options_png.h"
#include "../resources/about_png.h"
#include "../resources/help_png.h"
#include "../resources/news_png.h"
#include "../resources/nnews_png.h"
#include "../resources/fullscreen_png.h"
#include "../resources/pie_chart_png.h"
#include "../resources/budget_png.h"
#include "../resources/recurring_png.h"
#include "../resources/asset_png.h"
#include "../resources/car_png.h"
#include "../resources/jewellery_png.h"
#include "../resources/household_obj_png.h"
#include "../resources/art_png.h"
#include "../resources/coin_png.h"
#include "../resources/puzzle_png.h"
#include "../resources/facebook_png.h"
#include "../resources/credit_card_png.h"
#include "../resources/savings_acc_png.h"
#include "../resources/status_r_png.h"
#include "../resources/status_d_png.h"
#include "../resources/status_f_png.h"
#include "../resources/status_v_png.h"
#include "../resources/rightarrow_png.h"
#include "../resources/google_play_png.h"
#include "../resources/update_png.h"
#include "../resources/forum_png.h"
#include "../resources/profit_png.h"
#include "../resources/loss_png.h"
#include "../resources/uparrow_png.h"
#include "../resources/downarrow_png.h"
#include "../resources/import_png.h"
#include "../resources/leds_png.h"
#include "../resources/clip_png.h"
#include "../resources/exit_png.h"

#include "../resources/empty.xpm"
#include "../resources/stock_acc.xpm"
#include "../resources/stock_acc_closed.xpm"
#include "../resources/stock_acc_favorite.xpm"
#include "../resources/term_acc_closed.xpm"
#include "../resources/term_acc_favorite.xpm"
#include "../resources/termaccount.xpm"

#include "../resources/accounttree.xpm"
#include "../resources/cat.xpm"
#include "../resources/clock.xpm"
#include "../resources/dog.xpm"
#include "../resources/hourglass.xpm"
#include "../resources/money.xpm"
#include "../resources/money_dollar.xpm"
#include "../resources/money_euro.xpm"
#include "../resources/trees.xpm"
#include "../resources/web_money.xpm"
#include "../resources/work.xpm"
#include "../resources/yandex_money.xpm"

static const std::map<int, wxBitmap> navtree_images()
{
    int x = mmIniOptions::instance().ico_size_;
    return{
        { HOUSE_XPM, mmBitmap(png::HOME) }
        , { SCHEDULE_XPM, mmBitmap(png::RECURRING) }
        , { CALENDAR_XPM, mmBitmap(png::BUDGET) }
        , { PIECHART_XPM, mmBitmap(png::PIE_CHART) }
        , { HELP_XPM, mmBitmap(png::HELP) }
        , { FILTER_XPM, mmBitmap(png::FILTER) }
        , { ASSET_XPM, mmBitmap(png::ASSET) }
        , { CUSTOMSQL_XPM, mmBitmap(png::GRM) }
        , { CUSTOMSQL_GRP_XPM, mmBitmap(png::GRM) } //TODO: GRM rep group ico
        , { SAVINGS_ACC_NORMAL_XPM, mmBitmap(png::SAVINGS_NORMAL) }
        , { SAVINGS_ACC_FAVORITE_XPM, mmBitmap(png::SAVINGS_FAVORITES) }
        , { SAVINGS_ACC_CLOSED_XPM, mmBitmap(png::SAVINGS_CLOSED) }
        , { CARD_ACC_XPM, mmBitmap(png::CC_NORMAL) }
        , { CARD_ACC_FAVORITE_XPM, mmBitmap(png::CC_FAVORITE) }
        , { CARD_ACC_CLOSED_XPM, mmBitmap(png::CC_CLOSED) }
        , { TERMACCOUNT_XPM, wxBitmap(wxImage(termaccount_xpm).Scale(x, x)) }
        , { TERM_ACC_FAVORITE_XPM, wxBitmap(wxImage(term_acc_favorite_xpm).Scale(x, x)) }
        , { TERM_ACC_CLOSED_XPM, wxBitmap(wxImage(term_acc_closed_xpm).Scale(x, x)) }
        , { STOCK_ACC_XPM, wxBitmap(wxImage(stock_acc_xpm).Scale(x, x)) }
        , { STOCK_ACC_FAVORITE_XPM, wxBitmap(wxImage(stock_acc_favorite_xpm).Scale(x, x)) }
        , { STOCK_ACC_CLOSED_XPM, wxBitmap(wxImage(stock_acc_closed_xpm).Scale(x, x)) }
    };
};

static const std::map<int, wxBitmap> custom_images()
{
    int x = mmIniOptions::instance().ico_size_;
    return{
        // Custom icons for accounts
        { MONEY_DOLLAR_XPM, wxBitmap(wxImage(money_dollar_xpm).Scale(x, x)) }
        , { MONEY_EURO_XPM, wxBitmap(wxImage(money_euro_xpm).Scale(x, x)) }
        , { FLAG_XPM, mmBitmap(png::FOLLOW_UP) }
        , { ACCOUNTTREE_XPM, wxBitmap(wxImage(accounttree_xpm).Scale(x, x)) }
        , { ABOUT_XPM, mmBitmap(png::ABOUT) }
        , { CLOCK_XPM, wxBitmap(wxImage(clock_xpm).Scale(x, x)) }
        , { CAT_XPM, wxBitmap(wxImage(cat_xpm).Scale(x, x)) }
        , { DOG_XPM, wxBitmap(wxImage(dog_xpm).Scale(x, x)) }
        , { TREES_XPM, wxBitmap(wxImage(trees_xpm).Scale(x, x)) }
        , { HOURGLASS_XPM, wxBitmap(wxImage(hourglass_xpm).Scale(x, x)) }
        , { WORK_XPM, wxBitmap(wxImage(work_xpm).Scale(x, x)) }
        , { YANDEX_MONEY_XPM, wxBitmap(wxImage(yandex_money_xpm).Scale(x, x)) }
        , { WEB_MONEY_XPM, wxBitmap(wxImage(web_money_xpm).Scale(x, x)) }
        , { RUBIK_CUBE_XPM, mmBitmap(png::OTHER) }
    };
}
wxImageList* navtree_images_list()
{
    int x = mmIniOptions::instance().ico_size_;

    wxImageList* imageList = new wxImageList(x, x);
    for (const auto& img : navtree_images())
        imageList->Add(img.second);
    for (const auto& img : custom_images())
        imageList->Add(img.second);

    return imageList;
}

static const std::map<int, std::map<int, wxBitmap>> images_png()
{
    return{
        { NEWS, { { 16, wxBITMAP_PNG_FROM_DATA(news) }, { 24, wxBITMAP_PNG_FROM_DATA(news24) }, { 32, wxBITMAP_PNG_FROM_DATA(news32) }, { 48, wxBITMAP_PNG_FROM_DATA(news48) } } }
        , { NEW_DB, { { 16, wxBITMAP_PNG_FROM_DATA(new_db) }, { 24, wxBITMAP_PNG_FROM_DATA(new_db24) }, { 32, wxBITMAP_PNG_FROM_DATA(new_db32) }, { 48, wxBITMAP_PNG_FROM_DATA(new_db48) } } }
        , { OPEN, { { 16, wxBITMAP_PNG_FROM_DATA(open) }, { 24, wxBITMAP_PNG_FROM_DATA(open24) }, { 32, wxBITMAP_PNG_FROM_DATA(open32) }, { 48, wxBITMAP_PNG_FROM_DATA(open48) } } }
        , { NEW_ACC, { { 16, wxBITMAP_PNG_FROM_DATA(new_acc) }, { 24, wxBITMAP_PNG_FROM_DATA(new_acc24) }, { 32, wxBITMAP_PNG_FROM_DATA(new_acc32) }, { 48, wxBITMAP_PNG_FROM_DATA(new_acc48) } } }
        , { HOME, { { 16, wxBITMAP_PNG_FROM_DATA(home) }, { 24, wxBITMAP_PNG_FROM_DATA(home24) }, { 32, wxBITMAP_PNG_FROM_DATA(home32) }, { 48, wxBITMAP_PNG_FROM_DATA(home48) } } }
        , { CATEGORY, { { 16, wxBITMAP_PNG_FROM_DATA(category) }, { 24, wxBITMAP_PNG_FROM_DATA(category24) }, { 32, wxBITMAP_PNG_FROM_DATA(category32) }, { 48, wxBITMAP_PNG_FROM_DATA(category48) } } }
        , { PAYEE, { { 16, wxBITMAP_PNG_FROM_DATA(payee) }, { 24, wxBITMAP_PNG_FROM_DATA(payee24) }, { 32, wxBITMAP_PNG_FROM_DATA(payee32) }, { 48, wxBITMAP_PNG_FROM_DATA(payee48) } } }
        , { CURR, { { 16, wxBITMAP_PNG_FROM_DATA(currency) }, { 24, wxBITMAP_PNG_FROM_DATA(currency24) }, { 32, wxBITMAP_PNG_FROM_DATA(currency32) }, { 48, wxBITMAP_PNG_FROM_DATA(currency48) } } }
        , { FILTER, { { 16, wxBITMAP_PNG_FROM_DATA(filter) }, { 24, wxBITMAP_PNG_FROM_DATA(filter24) }, { 32, wxBITMAP_PNG_FROM_DATA(filter32) }, { 48, wxBITMAP_PNG_FROM_DATA(filter48) } } }
        , { GRM, { { 16, wxBITMAP_PNG_FROM_DATA(grm) }, { 24, wxBITMAP_PNG_FROM_DATA(grm24) }, { 32, wxBITMAP_PNG_FROM_DATA(grm32) }, { 48, wxBITMAP_PNG_FROM_DATA(grm48) } } }
        , { OPTIONS, { { 16, wxBITMAP_PNG_FROM_DATA(options) }, { 24, wxBITMAP_PNG_FROM_DATA(options24) }, { 32, wxBITMAP_PNG_FROM_DATA(options32) }, { 48, wxBITMAP_PNG_FROM_DATA(options48) } } }
        , { NEW_TRX, { { 16, wxBITMAP_PNG_FROM_DATA(new_trx) }, { 24, wxBITMAP_PNG_FROM_DATA(new_trx24) }, { 32, wxBITMAP_PNG_FROM_DATA(new_trx32) }, { 48, wxBITMAP_PNG_FROM_DATA(new_trx48) } } }
        , { ABOUT, { { 16, wxBITMAP_PNG_FROM_DATA(about) }, { 24, wxBITMAP_PNG_FROM_DATA(about24) }, { 32, wxBITMAP_PNG_FROM_DATA(about32) }, { 48, wxBITMAP_PNG_FROM_DATA(about48) } } }
        , { HELP, { { 16, wxBITMAP_PNG_FROM_DATA(help) }, { 24, wxBITMAP_PNG_FROM_DATA(help24) }, { 32, wxBITMAP_PNG_FROM_DATA(help32) }, { 48, wxBITMAP_PNG_FROM_DATA(help48) } } }
        , { NEWS, { { 16, wxBITMAP_PNG_FROM_DATA(news) }, { 24, wxBITMAP_PNG_FROM_DATA(news24) }, { 32, wxBITMAP_PNG_FROM_DATA(news32) }, { 48, wxBITMAP_PNG_FROM_DATA(news48) } } }
        , { FULLSCREEN, { { 16, wxBITMAP_PNG_FROM_DATA(fullscreen) }, { 24, wxBITMAP_PNG_FROM_DATA(fullscreen24) }, { 32, wxBITMAP_PNG_FROM_DATA(fullscreen32) }, { 48, wxBITMAP_PNG_FROM_DATA(fullscreen48) } } }
        , { NEW_NEWS, { { 16, wxBITMAP_PNG_FROM_DATA(nnews) }, { 24, wxBITMAP_PNG_FROM_DATA(nnews24) }, { 32, wxBITMAP_PNG_FROM_DATA(nnews32) }, { 48, wxBITMAP_PNG_FROM_DATA(nnews48) } } }
        , { PIE_CHART, { { 16, wxBITMAP_PNG_FROM_DATA(pie_chart) }, { 24, wxBITMAP_PNG_FROM_DATA(pie_chart24) }, { 32, wxBITMAP_PNG_FROM_DATA(pie_chart32) }, { 48, wxBITMAP_PNG_FROM_DATA(pie_chart48) } } }
        , { BUDGET, { { 16, wxBITMAP_PNG_FROM_DATA(budget) }, { 24, wxBITMAP_PNG_FROM_DATA(budget24) }, { 32, wxBITMAP_PNG_FROM_DATA(budget32) }, { 48, wxBITMAP_PNG_FROM_DATA(budget48) } } }
        , { RECURRING, { { 16, wxBITMAP_PNG_FROM_DATA(recurring) }, { 24, wxBITMAP_PNG_FROM_DATA(recurring24) }, { 32, wxBITMAP_PNG_FROM_DATA(recurring32) }, { 48, wxBITMAP_PNG_FROM_DATA(recurring48) } } }
        , { ASSET, { { 16, wxBITMAP_PNG_FROM_DATA(asset) }, { 24, wxBITMAP_PNG_FROM_DATA(asset24) }, { 32, wxBITMAP_PNG_FROM_DATA(asset32) }, { 48, wxBITMAP_PNG_FROM_DATA(asset48) } } }
        //, { PROPERTY, { { 16, wxBITMAP_PNG_FROM_DATA(home) }, { 24, wxBITMAP_PNG_FROM_DATA(home24) }, { 32, wxBITMAP_PNG_FROM_DATA(home32) }, { 48, wxBITMAP_PNG_FROM_DATA(home48) } } }
        , { CAR, { { 16, wxBITMAP_PNG_FROM_DATA(car) }, { 24, wxBITMAP_PNG_FROM_DATA(car24) }, { 32, wxBITMAP_PNG_FROM_DATA(car32) }, { 48, wxBITMAP_PNG_FROM_DATA(car48) } } }
        , { HOUSEHOLD_OBJ, { { 16, wxBITMAP_PNG_FROM_DATA(household_obj) }, { 24, wxBITMAP_PNG_FROM_DATA(household_obj24) }, { 32, wxBITMAP_PNG_FROM_DATA(household_obj32) }, { 48, wxBITMAP_PNG_FROM_DATA(household_obj48) } } }
        , { ART, { { 16, wxBITMAP_PNG_FROM_DATA(art) }, { 24, wxBITMAP_PNG_FROM_DATA(art24) }, { 32, wxBITMAP_PNG_FROM_DATA(art32) }, { 48, wxBITMAP_PNG_FROM_DATA(art48) } } }
        , { JEWELLERY, { { 16, wxBITMAP_PNG_FROM_DATA(jewellery) }, { 24, wxBITMAP_PNG_FROM_DATA(jewellery24) }, { 32, wxBITMAP_PNG_FROM_DATA(jewellery32) }, { 48, wxBITMAP_PNG_FROM_DATA(jewellery48) } } }
        , { CASH, { { 16, wxBITMAP_PNG_FROM_DATA(coin) }, { 24, wxBITMAP_PNG_FROM_DATA(coin24) }, { 32, wxBITMAP_PNG_FROM_DATA(coin32) }, { 48, wxBITMAP_PNG_FROM_DATA(coin48) } } }
        , { OTHER, { { 16, wxBITMAP_PNG_FROM_DATA(puzzle) }, { 24, wxBITMAP_PNG_FROM_DATA(puzzle24) }, { 32, wxBITMAP_PNG_FROM_DATA(puzzle32) }, { 48, wxBITMAP_PNG_FROM_DATA(puzzle48) } } }
        , { FACEBOOK, { { 16, wxBITMAP_PNG_FROM_DATA(facebook) }, { 24, wxBITMAP_PNG_FROM_DATA(facebook24) }, { 32, wxBITMAP_PNG_FROM_DATA(facebook32) }, { 48, wxBITMAP_PNG_FROM_DATA(facebook48) } } }
        , { SAVINGS_NORMAL, { { 16, wxBITMAP_PNG_FROM_DATA(savings_acc_normal) }, { 24, wxBITMAP_PNG_FROM_DATA(savings_acc_normal24) }, { 32, wxBITMAP_PNG_FROM_DATA(savings_acc_normal32) }, { 48, wxBITMAP_PNG_FROM_DATA(savings_acc_normal48) } } }
        , { SAVINGS_FAVORITES, { { 16, wxBITMAP_PNG_FROM_DATA(savings_acc_favorite) }, { 24, wxBITMAP_PNG_FROM_DATA(savings_acc_favorite24) }, { 32, wxBITMAP_PNG_FROM_DATA(savings_acc_favorite32) }, { 48, wxBITMAP_PNG_FROM_DATA(savings_acc_favorite48) } } }
        , { SAVINGS_CLOSED, { { 16, wxBITMAP_PNG_FROM_DATA(savings_acc_closed) }, { 24, wxBITMAP_PNG_FROM_DATA(savings_acc_closed24) }, { 32, wxBITMAP_PNG_FROM_DATA(savings_acc_closed32) }, { 48, wxBITMAP_PNG_FROM_DATA(savings_acc_closed48) } } }
        , { CC_NORMAL, { { 16, wxBITMAP_PNG_FROM_DATA(cc_normal) }, { 24, wxBITMAP_PNG_FROM_DATA(cc_normal24) }, { 32, wxBITMAP_PNG_FROM_DATA(cc_normal32) }, { 48, wxBITMAP_PNG_FROM_DATA(cc_normal48) } } }
        , { CC_FAVORITE, { { 16, wxBITMAP_PNG_FROM_DATA(cc_favorite) }, { 24, wxBITMAP_PNG_FROM_DATA(cc_favorite24) }, { 32, wxBITMAP_PNG_FROM_DATA(cc_favorite32) }, { 48, wxBITMAP_PNG_FROM_DATA(cc_favorite48) } } }
        , { CC_CLOSED, { { 16, wxBITMAP_PNG_FROM_DATA(cc_closed) }, { 24, wxBITMAP_PNG_FROM_DATA(cc_closed24) }, { 32, wxBITMAP_PNG_FROM_DATA(cc_closed32) }, { 48, wxBITMAP_PNG_FROM_DATA(cc_closed48) } } }
        , { RECONCILED, { { 16, wxBITMAP_PNG_FROM_DATA(status_r) }, { 24, wxBITMAP_PNG_FROM_DATA(status_r24) }, { 32, wxBITMAP_PNG_FROM_DATA(status_r32) }, { 48, wxBITMAP_PNG_FROM_DATA(status_r48) } } }
        , { VOID_STAT, { { 16, wxBITMAP_PNG_FROM_DATA(status_v) }, { 24, wxBITMAP_PNG_FROM_DATA(status_v24) }, { 32, wxBITMAP_PNG_FROM_DATA(status_v32) }, { 48, wxBITMAP_PNG_FROM_DATA(status_v48) } } }
        , { FOLLOW_UP, { { 16, wxBITMAP_PNG_FROM_DATA(status_f) }, { 24, wxBITMAP_PNG_FROM_DATA(status_f24) }, { 32, wxBITMAP_PNG_FROM_DATA(status_f32) }, { 48, wxBITMAP_PNG_FROM_DATA(status_f48) } } }
        , { DUPLICATE_STAT, { { 16, wxBITMAP_PNG_FROM_DATA(status_d) }, { 24, wxBITMAP_PNG_FROM_DATA(status_d24) }, { 32, wxBITMAP_PNG_FROM_DATA(status_d32) }, { 48, wxBITMAP_PNG_FROM_DATA(status_d48) } } }
        , { RIGHTARROW, { { 16, wxBITMAP_PNG_FROM_DATA(rightarrow) }, { 24, wxBITMAP_PNG_FROM_DATA(rightarrow24) }, { 32, wxBITMAP_PNG_FROM_DATA(rightarrow32) }, { 48, wxBITMAP_PNG_FROM_DATA(rightarrow48) } } }
        , { RIGHTARROW_ACTIVE, { { 16, wxBITMAP_PNG_FROM_DATA(rightarrow_active) }, { 24, wxBITMAP_PNG_FROM_DATA(rightarrow_active24) }, { 32, wxBITMAP_PNG_FROM_DATA(rightarrow_active32) }, { 48, wxBITMAP_PNG_FROM_DATA(rightarrow_active48) } } }
        , { GOOGLE_PLAY, { { 16, wxBITMAP_PNG_FROM_DATA(google_play) }, { 24, wxBITMAP_PNG_FROM_DATA(google_play24) }, { 32, wxBITMAP_PNG_FROM_DATA(google_play32) }, { 48, wxBITMAP_PNG_FROM_DATA(google_play48) } } }
        , { UPDATE, { { 16, wxBITMAP_PNG_FROM_DATA(update) }, { 24, wxBITMAP_PNG_FROM_DATA(update24) }, { 32, wxBITMAP_PNG_FROM_DATA(update32) }, { 48, wxBITMAP_PNG_FROM_DATA(update48) } } }
        , { FORUM, { { 16, wxBITMAP_PNG_FROM_DATA(forum) }, { 24, wxBITMAP_PNG_FROM_DATA(forum24) }, { 32, wxBITMAP_PNG_FROM_DATA(forum32) }, { 48, wxBITMAP_PNG_FROM_DATA(forum48) } } }
        , { CLIP, { { 16, wxBITMAP_PNG_FROM_DATA(clip) }, { 24, wxBITMAP_PNG_FROM_DATA(clip24) }, { 32, wxBITMAP_PNG_FROM_DATA(clip32) }, { 48, wxBITMAP_PNG_FROM_DATA(clip48) } } }
        , { PROFIT, { { 16, wxBITMAP_PNG_FROM_DATA(profit) }, { 24, wxBITMAP_PNG_FROM_DATA(profit24) }, { 32, wxBITMAP_PNG_FROM_DATA(profit32) }, { 48, wxBITMAP_PNG_FROM_DATA(profit48) } } }
        , { LOSS, { { 16, wxBITMAP_PNG_FROM_DATA(loss) }, { 24, wxBITMAP_PNG_FROM_DATA(loss24) }, { 32, wxBITMAP_PNG_FROM_DATA(loss32) }, { 48, wxBITMAP_PNG_FROM_DATA(loss48) } } }
        , { UPARROW, { { 16, wxBITMAP_PNG_FROM_DATA(uparrow) }, { 24, wxBITMAP_PNG_FROM_DATA(uparrow24) }, { 32, wxBITMAP_PNG_FROM_DATA(uparrow32) }, { 48, wxBITMAP_PNG_FROM_DATA(uparrow48) } } }
        , { DOWNARROW, { { 16, wxBITMAP_PNG_FROM_DATA(downarrow) }, { 24, wxBITMAP_PNG_FROM_DATA(downarrow24) }, { 32, wxBITMAP_PNG_FROM_DATA(downarrow32) }, { 48, wxBITMAP_PNG_FROM_DATA(downarrow48) } } }
        , { IMPORT, { { 16, wxBITMAP_PNG_FROM_DATA(import) }, { 24, wxBITMAP_PNG_FROM_DATA(import24) }, { 32, wxBITMAP_PNG_FROM_DATA(import32) }, { 48, wxBITMAP_PNG_FROM_DATA(import48) } } }
        , { LED_OFF, { { 16, wxBITMAP_PNG_FROM_DATA(led_off) }, { 24, wxBITMAP_PNG_FROM_DATA(led_off24) }, { 32, wxBITMAP_PNG_FROM_DATA(led_off32) }, { 48, wxBITMAP_PNG_FROM_DATA(led_off48) } } }
        , { LED_RED, { { 16, wxBITMAP_PNG_FROM_DATA(led_red) }, { 24, wxBITMAP_PNG_FROM_DATA(led_red24) }, { 32, wxBITMAP_PNG_FROM_DATA(led_red32) }, { 48, wxBITMAP_PNG_FROM_DATA(led_red48) } } }
        , { LED_YELLOW, { { 16, wxBITMAP_PNG_FROM_DATA(led_yellow) }, { 24, wxBITMAP_PNG_FROM_DATA(led_yellow24) }, { 32, wxBITMAP_PNG_FROM_DATA(led_yellow32) }, { 48, wxBITMAP_PNG_FROM_DATA(led_yellow48) } } }
        , { LED_GREEN, { { 16, wxBITMAP_PNG_FROM_DATA(led_green) }, { 24, wxBITMAP_PNG_FROM_DATA(led_green24) }, { 32, wxBITMAP_PNG_FROM_DATA(led_green32) }, { 48, wxBITMAP_PNG_FROM_DATA(led_green48) } } }
        , { EXIT, { { 16, wxBITMAP_PNG_FROM_DATA(exit) }, { 24, wxBITMAP_PNG_FROM_DATA(exit24) }, { 32, wxBITMAP_PNG_FROM_DATA(exit32) }, { 48, wxBITMAP_PNG_FROM_DATA(exit48) } } }
        , { EMPTY, { { 16, wxBitmap(wxImage(empty_xpm).Scale(16, 16)) }, { 24, wxBitmap(wxImage(empty_xpm).Scale(24, 24)) }, { 32, wxArtProvider::GetBitmap("wxART_MISSING_IMAGE") }, { 48, wxBitmap(wxImage(empty_xpm).Scale(48, 48)) } } }
    };
}

const wxBitmap mmBitmap(int ref)
{
    int x = mmIniOptions::instance().ico_size_;

    wxBitmap b;
    try {
        // throw, even if capacity allowed to access element
        const auto m = images_png().at(ref);
        b = m.at(x);
    }
    catch (std::out_of_range const& exc) {
        wxLogError("Exception in function 'mmBitmap': %s", exc.what());
        b = wxBitmap(wxImage(empty_xpm).Scale(x, x));
    }
    return b;
}