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
#include "model/Model_Setting.h"
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



#include "../resources/about.xpm"
#include "../resources/accounttree.xpm"
#include "../resources/calendar.xpm"
#include "../resources/car.xpm"
#include "../resources/card_acc.xpm"
#include "../resources/card_acc_closed.xpm"
#include "../resources/card_acc_favorite.xpm"
#include "../resources/cat.xpm"
#include "../resources/chartpiereport.xpm"
#include "../resources/clock.xpm"
#include "../resources/coin.xpm"
#include "../resources/customsql.xpm"
#include "../resources/dog.xpm"
#include "../resources/filter.xpm"
#include "../resources/flag.xpm"
#include "../resources/help.xpm"
#include "../resources/hourglass.xpm"
#include "../resources/house.xpm"
#include "../resources/money.xpm"
#include "../resources/money_dollar.xpm"
#include "../resources/money_euro.xpm"
#include "../resources/moneyaccount.xpm"
#include "../resources/rubik_cube.xpm"
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

static const std::map<int, wxImage> images_list()
{
    return {
        { HOUSE_XPM, wxImage(house_xpm) }
        , { SCHEDULE_XPM, wxImage(schedule_xpm) }
        , { CALENDAR_XPM, wxImage(calendar_xpm) }
        , { PIECHART_XPM, wxImage(chartpiereport_xpm) }
        , { HELP_XPM, wxImage(help_xpm) }
        , { FILTER_XPM, wxImage(filter_xpm) }
        , { CAR_XPM, wxImage(car_xpm) }
        , { CUSTOMSQL_XPM, wxImage(customsql_xpm) }
        , { CUSTOMSQL_GRP_XPM, wxImage(customsql_xpm) } //TODO: GRM rep group ico
        , { MONEYACCOUNT_XPM, wxImage(moneyaccount_xpm) }
        , { SAVINGS_ACC_FAVORITE_XPM, wxImage(savings_acc_favorite_xpm) }
        , { SAVINGS_ACC_CLOSED_XPM, wxImage(savings_acc_closed_xpm) }
        , { TERMACCOUNT_XPM, wxImage(termaccount_xpm) }
        , { TERM_ACC_FAVORITE_XPM, wxImage(term_acc_favorite_xpm) }
        , { TERM_ACC_CLOSED_XPM, wxImage(term_acc_closed_xpm) }
        , { STOCK_ACC_XPM, wxImage(stock_acc_xpm) }
        , { STOCK_ACC_FAVORITE_XPM, wxImage(stock_acc_favorite_xpm) }
        , { STOCK_ACC_CLOSED_XPM, wxImage(stock_acc_closed_xpm) }
        , { CARD_ACC_XPM, wxImage(card_acc_xpm) }
        , { CARD_ACC_FAVORITE_XPM, wxImage(card_acc_favorite_xpm) }
        , { CARD_ACC_CLOSED_XPM, wxImage(card_acc_closed_xpm) }
        // Custom icons for accounts
        , { MONEY_DOLLAR_XPM, wxImage(money_dollar_xpm) }
        , { MONEY_EURO_XPM, wxImage(money_euro_xpm) }
        , { FLAG_XPM, wxImage(flag_xpm) }
        , { ACCOUNTTREE_XPM, wxImage(accounttree_xpm) }
        , { ABOUT_XPM, wxImage(about_xpm) }
        , { CLOCK_XPM, wxImage(clock_xpm) }
        , { CAT_XPM, wxImage(cat_xpm) }
        , { DOG_XPM, wxImage(dog_xpm) }
        , { TREES_XPM, wxImage(trees_xpm) }
        , { HOURGLASS_XPM, wxImage(hourglass_xpm) }
        , { WORK_XPM, wxImage(work_xpm) }
        , { YANDEX_MONEY_XPM, wxImage(yandex_money_xpm) }
        , { WEB_MONEY_XPM, wxImage(web_money_xpm) }
        , { RUBIK_CUBE_XPM, wxImage(rubik_cube_xpm) }
    };
};

wxImageList* navtree_images_list()
{
    int vFontSize = Model_Setting::instance().GetHtmlScaleFactor();
    int x = 16;
    if (vFontSize >= 300) x = 48;
    else if (vFontSize >= 200) x = 32;

    wxImageList* imageList = new wxImageList(x, x);
    for (const auto& img : images_list())
    {
        imageList->Add(wxBitmap(img.second.Scale(x, x)));
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
    };
}

wxBitmap mmBitmap(int ref)
{
    int vFontSize = Model_Setting::instance().GetHtmlScaleFactor();
    int x = 16;
    if (vFontSize >= 300) x = 48;
    else if (vFontSize >= 200) x = 32;
    else if (vFontSize >= 150) x = 24;

    wxBitmap b;
    try {
        // throw, even if capacity allowed to access element
        const auto m = images_png().at(ref);
        b = m.at(x);
    }
    catch (std::out_of_range const& exc) {
        wxLogError("Exception in function 'mmBitmap': %s", exc.what());
        b = wxBitmap(about_xpm);
    }
    return b;
}