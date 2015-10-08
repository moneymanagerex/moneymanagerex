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
#include "../resources/pie_chart_png.h"
#include "../resources/budget_png.h"
#include "../resources/asset_png.h"
#include "../resources/car_png.h"
#include "../resources/jewellery_png.h"
#include "../resources/household_obj_png.h"
#include "../resources/art_png.h"
#include "../resources/coin_png.h"
#include "../resources/puzzle_png.h"
#include "../resources/exit_png.h"

#include "../resources/accounttree.xpm"
#include "../resources/cat.xpm"
#include "../resources/card_acc.xpm"
#include "../resources/card_acc_closed.xpm"
#include "../resources/card_acc_favorite.xpm"
#include "../resources/chartpiereport.xpm"
#include "../resources/clock.xpm"
#include "../resources/customsql.xpm"
#include "../resources/dog.xpm"
#include "../resources/empty.xpm"
#include "../resources/flag.xpm"
#include "../resources/help.xpm"
#include "../resources/hourglass.xpm"
#include "../resources/house.xpm"
#include "../resources/money.xpm"
#include "../resources/money_dollar.xpm"
#include "../resources/money_euro.xpm"
#include "../resources/moneyaccount.xpm"
#include "../resources/savings_acc_closed.xpm"
#include "../resources/savings_acc_favorite.xpm"
#include "../resources/schedule.xpm"
#include "../resources/stock_acc.xpm"
#include "../resources/stock_acc_closed.xpm"
#include "../resources/stock_acc_favorite.xpm"
#include "../resources/term_acc_closed.xpm"
#include "../resources/term_acc_favorite.xpm"
#include "../resources/termaccount.xpm"
#include "../resources/trees.xpm"
#include "../resources/web_money.xpm"
#include "../resources/work.xpm"
#include "../resources/yandex_money.xpm"

static const std::map<int, wxBitmap> images_list()
{
    int x = mmIniOptions::instance().ico_size_;
    return{
        { HOUSE_XPM, mmBitmap(png::HOME) }
        , { SCHEDULE_XPM, wxBitmap(wxImage(schedule_xpm).Scale(x, x)) }
        , { CALENDAR_XPM, mmBitmap(png::BUDGET) }
        , { PIECHART_XPM, mmBitmap(png::PIE_CHART) }
        , { HELP_XPM, mmBitmap(png::HELP) }
        , { FILTER_XPM, mmBitmap(png::FILTER) }
        , { ASSET_XPM, mmBitmap(png::ASSET) }
        , { CUSTOMSQL_XPM, mmBitmap(png::GRM) }
        , { CUSTOMSQL_GRP_XPM, mmBitmap(png::GRM) } //TODO: GRM rep group ico
        , { MONEYACCOUNT_XPM, wxBitmap(wxImage(moneyaccount_xpm).Scale(x, x)) }
        , { SAVINGS_ACC_FAVORITE_XPM, wxBitmap(wxImage(savings_acc_favorite_xpm).Scale(x, x)) }
        , { SAVINGS_ACC_CLOSED_XPM, wxBitmap(wxImage(savings_acc_closed_xpm).Scale(x, x)) }
        , { TERMACCOUNT_XPM, wxBitmap(wxImage(termaccount_xpm).Scale(x, x)) }
        , { TERM_ACC_FAVORITE_XPM, wxBitmap(wxImage(term_acc_favorite_xpm).Scale(x, x)) }
        , { TERM_ACC_CLOSED_XPM, wxBitmap(wxImage(term_acc_closed_xpm).Scale(x, x)) }
        , { STOCK_ACC_XPM, wxBitmap(wxImage(stock_acc_xpm).Scale(x, x)) }
        , { STOCK_ACC_FAVORITE_XPM, wxBitmap(wxImage(stock_acc_favorite_xpm).Scale(x, x)) }
        , { STOCK_ACC_CLOSED_XPM, wxBitmap(wxImage(stock_acc_closed_xpm).Scale(x, x)) }
        , { CARD_ACC_XPM, wxBitmap(wxImage(card_acc_xpm).Scale(x, x)) }
        , { CARD_ACC_FAVORITE_XPM, wxBitmap(wxImage(card_acc_favorite_xpm).Scale(x, x)) }
        , { CARD_ACC_CLOSED_XPM, wxBitmap(wxImage(card_acc_closed_xpm).Scale(x, x)) }
        // Custom icons for accounts
        , { MONEY_DOLLAR_XPM, wxBitmap(wxImage(money_dollar_xpm).Scale(x, x)) }
        , { MONEY_EURO_XPM, wxBitmap(wxImage(money_euro_xpm).Scale(x, x)) }
        , { FLAG_XPM, wxBitmap(wxImage(flag_xpm).Scale(x, x)) }
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
};

wxImageList* navtree_images_list()
{
    int x = mmIniOptions::instance().ico_size_;

    wxImageList* imageList = new wxImageList(x, x);
    for (const auto& img : images_list())
    {
        imageList->Add(img.second);
    }
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
        , { NEW_NEWS, { { 16, wxBITMAP_PNG_FROM_DATA(nnews) }, { 24, wxBITMAP_PNG_FROM_DATA(nnews24) }, { 32, wxBITMAP_PNG_FROM_DATA(nnews32) }, { 48, wxBITMAP_PNG_FROM_DATA(nnews48) } } }
        , { PIE_CHART, { { 16, wxBITMAP_PNG_FROM_DATA(pie_chart) }, { 24, wxBITMAP_PNG_FROM_DATA(pie_chart24) }, { 32, wxBITMAP_PNG_FROM_DATA(pie_chart32) }, { 48, wxBITMAP_PNG_FROM_DATA(pie_chart48) } } }
        , { BUDGET, { { 16, wxBITMAP_PNG_FROM_DATA(budget) }, { 24, wxBITMAP_PNG_FROM_DATA(budget24) }, { 32, wxBITMAP_PNG_FROM_DATA(budget32) }, { 48, wxBITMAP_PNG_FROM_DATA(budget48) } } }
        , { ASSET, { { 16, wxBITMAP_PNG_FROM_DATA(asset) }, { 24, wxBITMAP_PNG_FROM_DATA(asset24) }, { 32, wxBITMAP_PNG_FROM_DATA(asset32) }, { 48, wxBITMAP_PNG_FROM_DATA(asset48) } } }
        , { CAR, { { 16, wxBITMAP_PNG_FROM_DATA(car) }, { 24, wxBITMAP_PNG_FROM_DATA(car24) }, { 32, wxBITMAP_PNG_FROM_DATA(car32) }, { 48, wxBITMAP_PNG_FROM_DATA(car48) } } }
        , { HOUSEHOLD_OBJ, { { 16, wxBITMAP_PNG_FROM_DATA(household_obj) }, { 24, wxBITMAP_PNG_FROM_DATA(household_obj24) }, { 32, wxBITMAP_PNG_FROM_DATA(household_obj32) }, { 48, wxBITMAP_PNG_FROM_DATA(household_obj48) } } }
        , { ART, { { 16, wxBITMAP_PNG_FROM_DATA(art) }, { 24, wxBITMAP_PNG_FROM_DATA(art24) }, { 32, wxBITMAP_PNG_FROM_DATA(art32) }, { 48, wxBITMAP_PNG_FROM_DATA(art48) } } }
        , { JEWELLERY, { { 16, wxBITMAP_PNG_FROM_DATA(jewellery) }, { 24, wxBITMAP_PNG_FROM_DATA(jewellery24) }, { 32, wxBITMAP_PNG_FROM_DATA(jewellery32) }, { 48, wxBITMAP_PNG_FROM_DATA(jewellery48) } } }
        , { CASH, { { 16, wxBITMAP_PNG_FROM_DATA(coin) }, { 24, wxBITMAP_PNG_FROM_DATA(coin24) }, { 32, wxBITMAP_PNG_FROM_DATA(coin32) }, { 48, wxBITMAP_PNG_FROM_DATA(coin48) } } }
        , { OTHER, { { 16, wxBITMAP_PNG_FROM_DATA(puzzle) }, { 24, wxBITMAP_PNG_FROM_DATA(puzzle24) }, { 32, wxBITMAP_PNG_FROM_DATA(puzzle32) }, { 48, wxBITMAP_PNG_FROM_DATA(puzzle48) } } }
        , { EXIT, { { 16, wxBITMAP_PNG_FROM_DATA(exit) }, { 24, wxBITMAP_PNG_FROM_DATA(exit24) }, { 32, wxBITMAP_PNG_FROM_DATA(exit32) }, { 48, wxBITMAP_PNG_FROM_DATA(exit48) } } }
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
        b = wxBitmap(empty_xpm);
    }
    return b;
}