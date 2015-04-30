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
#include <wx/image.h>
#include <wx/bitmap.h>

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

static const std::map<int, wxBitmap> images_list()
{
    return {
        { HOUSE_XPM, wxBitmap(wxImage(house_xpm).Scale(16, 16)) }
        , { MONEYACCOUNT2_XPM, wxBitmap(wxImage(moneyaccount_xpm).Scale(16, 16)) }
        , { SCHEDULE_XPM, wxBitmap(wxImage(schedule_xpm).Scale(16, 16)) }
        , { CALENDAR_XPM, wxBitmap(wxImage(calendar_xpm).Scale(16, 16)) }
        , { PIECHART_XPM, wxBitmap(wxImage(chartpiereport_xpm).Scale(16, 16)) }
        , { HELP_XPM, wxBitmap(wxImage(help_xpm).Scale(16, 16)) }
        , { FILTER_XPM, wxBitmap(wxImage(filter_xpm).Scale(16, 16)) }
        , { CAR_XPM, wxBitmap(wxImage(car_xpm).Scale(16, 16)) }
        , { CUSTOMSQL_XPM, wxBitmap(wxImage(customsql_xpm).Scale(16, 16)) }
        , { CUSTOMSQL_GRP_XPM, wxBitmap(wxImage(customsql_xpm).Scale(16, 16)) } //TODO: GRM rep group ico
        , { MONEYACCOUNT_XPM, wxBitmap(wxImage(moneyaccount_xpm).Scale(16, 16)) }
        , { SAVINGS_ACC_FAVORITE_XPM, wxBitmap(wxImage(savings_acc_favorite_xpm).Scale(16, 16)) }
        , { SAVINGS_ACC_CLOSED_XPM, wxBitmap(wxImage(savings_acc_closed_xpm).Scale(16, 16)) }
        , { TERMACCOUNT_XPM, wxBitmap(wxImage(termaccount_xpm).Scale(16, 16)) }
        , { TERM_ACC_FAVORITE_XPM, wxBitmap(wxImage(term_acc_favorite_xpm).Scale(16, 16)) }
        , { TERM_ACC_CLOSED_XPM, wxBitmap(wxImage(term_acc_closed_xpm).Scale(16, 16)) }
        , { STOCK_ACC_XPM, wxBitmap(wxImage(stock_acc_xpm).Scale(16, 16)) }
        , { STOCK_ACC_FAVORITE_XPM, wxBitmap(wxImage(stock_acc_favorite_xpm).Scale(16, 16)) }
        , { STOCK_ACC_CLOSED_XPM, wxBitmap(wxImage(stock_acc_closed_xpm).Scale(16, 16)) }
        , { CARD_ACC_XPM, wxBitmap(wxImage(card_acc_xpm).Scale(16, 16)) }
        , { CARD_ACC_FAVORITE_XPM, wxBitmap(wxImage(card_acc_favorite_xpm).Scale(16, 16)) }
        , { CARD_ACC_CLOSED_XPM, wxBitmap(wxImage(card_acc_closed_xpm).Scale(16, 16)) }
        // Custom icons for accounts
        , { MONEY_DOLLAR_XPM, wxBitmap(wxImage(money_dollar_xpm).Scale(16, 16)) }
        , { MONEY_EURO_XPM, wxBitmap(wxImage(money_euro_xpm).Scale(16, 16)) }
        , { FLAG_XPM, wxBitmap(wxImage(flag_xpm).Scale(16, 16)) }
        , { ACCOUNTTREE_XPM, wxBitmap(wxImage(accounttree_xpm).Scale(16, 16)) }
        , { ABOUT_XPM, wxBitmap(wxImage(about_xpm).Scale(16, 16)) }
        , { CLOCK_XPM, wxBitmap(wxImage(clock_xpm).Scale(16, 16)) }
        , { CAT_XPM, wxBitmap(wxImage(cat_xpm).Scale(16, 16)) }
        , { DOG_XPM, wxBitmap(wxImage(dog_xpm).Scale(16, 16)) }
        , { TREES_XPM, wxBitmap(wxImage(trees_xpm).Scale(16, 16)) }
        , { HOURGLASS_XPM, wxBitmap(wxImage(hourglass_xpm).Scale(16, 16)) }
        , { WORK_XPM, wxBitmap(wxImage(work_xpm).Scale(16, 16)) }
        , { YANDEX_MONEY_XPM, wxBitmap(wxImage(yandex_money_xpm).Scale(16, 16)) }
        , { WEB_MONEY_XPM, wxBitmap(wxImage(web_money_xpm).Scale(16, 16)) }
        , { RUBIK_CUBE_XPM, wxBitmap(wxImage(rubik_cube_xpm).Scale(16, 16)) }
    };
};

wxImageList* navtree_images_list()
{
    wxImageList* imageList = new wxImageList(16, 16);
    for (const auto& img : images_list())
    {
        imageList->Add(img.second);
    }
    return imageList;
}
