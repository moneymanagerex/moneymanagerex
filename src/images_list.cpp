
#include "images_list.h"
#include <wx/image.h>
#include <wx/bitmap.h>

#include "../resources/flag.xpm"
#include "../resources/moneyaccount.xpm"
#include "../resources/schedule.xpm"
#include "../resources/yandex_money.xpm"
#include "../resources/calendar.xpm"
#include "../resources/help.xpm"
#include "../resources/stock_curve.xpm"
#include "../resources/savings_acc_favorite.xpm"
#include "../resources/savings_acc_closed.xpm"
#include "../resources/term_acc_favorite.xpm"
#include "../resources/term_acc_closed.xpm"
#include "../resources/stock_acc.xpm"
#include "../resources/stock_acc_closed.xpm"
#include "../resources/stock_acc_favorite.xpm"
#include "../resources/customsql.xpm"
#include "../resources/accounttree.xpm"
#include "../resources/termaccount.xpm"
#include "../resources/about.xpm"
#include "../resources/clock.xpm"
#include "../resources/filter.xpm"
#include "../resources/chartpiereport.xpm"
#include "../resources/work.xpm"
#include "../resources/money_dollar.xpm"
#include "../resources/money_euro.xpm"
#include "../resources/money.xpm"
#include "../resources/web_money.xpm"
#include "../resources/trees.xpm"
#include "../resources/rubik_cube.xpm"
#include "../resources/dog.xpm"
#include "../resources/coin.xpm"
#include "../resources/cat.xpm"
#include "../resources/car.xpm"
#include "../resources/house.xpm"
#include "../resources/hourglass.xpm"

wxImageList* navtree_images_list()
{
    wxImageList* imageList = new wxImageList(16, 16);

    imageList->Add(wxBitmap(wxImage(house_xpm).Scale(16, 16)));
    imageList->Add(wxBitmap(wxImage(moneyaccount_xpm).Scale(16, 16)));  //TODO: remove
    imageList->Add(wxBitmap(wxImage(schedule_xpm).Scale(16, 16)));
    imageList->Add(wxBitmap(wxImage(calendar_xpm).Scale(16, 16)));
    imageList->Add(wxBitmap(wxImage(chartpiereport_xpm).Scale(16, 16)));
    imageList->Add(wxBitmap(wxImage(help_xpm).Scale(16, 16)));
    imageList->Add(wxBitmap(wxImage(filter_xpm).Scale(16, 16)));
    imageList->Add(wxBitmap(wxImage(car_xpm).Scale(16, 16)));
    imageList->Add(wxBitmap(wxImage(customsql_xpm).Scale(16, 16)));
    imageList->Add(wxBitmap(wxImage(moneyaccount_xpm).Scale(16, 16))); // used for: savings_account
    imageList->Add(wxBitmap(wxImage(savings_acc_favorite_xpm).Scale(16, 16))); //10
    imageList->Add(wxBitmap(wxImage(savings_acc_closed_xpm).Scale(16, 16)));
    imageList->Add(wxBitmap(wxImage(termaccount_xpm).Scale(16, 16))); // used for: term_account
    imageList->Add(wxBitmap(wxImage(term_acc_favorite_xpm).Scale(16, 16)));
    imageList->Add(wxBitmap(wxImage(term_acc_closed_xpm).Scale(16, 16)));
    imageList->Add(wxBitmap(wxImage(stock_acc_xpm).Scale(16, 16))); // used for: invest_account
    imageList->Add(wxBitmap(wxImage(stock_acc_favorite_xpm).Scale(16, 16))); //TODO: more icons
    imageList->Add(wxBitmap(wxImage(stock_acc_closed_xpm).Scale(16, 16)));
    imageList->Add(wxBitmap(wxImage(money_dollar_xpm).Scale(16, 16)));
    imageList->Add(wxBitmap(wxImage(money_euro_xpm).Scale(16, 16))); //custom icons
    imageList->Add(wxBitmap(wxImage(flag_xpm).Scale(16, 16)));
    imageList->Add(wxBitmap(wxImage(accounttree_xpm).Scale(16, 16)));
    imageList->Add(wxBitmap(wxImage(about_xpm).Scale(16, 16)));
    imageList->Add(wxBitmap(wxImage(clock_xpm).Scale(16, 16)));
    imageList->Add(wxBitmap(wxImage(cat_xpm).Scale(16, 16)));
    imageList->Add(wxBitmap(wxImage(dog_xpm).Scale(16, 16)));
    imageList->Add(wxBitmap(wxImage(trees_xpm).Scale(16, 16)));
    imageList->Add(wxBitmap(wxImage(hourglass_xpm).Scale(16, 16)));
    imageList->Add(wxBitmap(wxImage(work_xpm).Scale(16, 16)));
    imageList->Add(wxBitmap(wxImage(yandex_money_xpm).Scale(16, 16)));
    imageList->Add(wxBitmap(wxImage(web_money_xpm).Scale(16, 16)));
    imageList->Add(wxBitmap(wxImage(rubik_cube_xpm).Scale(16, 16)));

    return imageList;
}
