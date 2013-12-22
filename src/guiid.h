/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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
 ********************************************************/
//----------------------------------------------------------------------------
#ifndef _MM_EX_GUIID_H_
#define _MM_EX_GUIID_H_
//----------------------------------------------------------------------------
#include <wx/defs.h>
//----------------------------------------------------------------------------
/* Include XPM Support */
#include "../resources/about.xpm"
#include "../resources/accounttree.xpm"
#include "../resources/addacctwiz.xpm"
#include "../resources/appstart.xpm"
#include "../resources/art.xpm"
#include "../resources/assets.xpm"
#include "../resources/calendar.xpm"
#include "../resources/car.xpm"
#include "../resources/cat.xpm"
#include "../resources/categoryedit.xpm"
#include "../resources/chartpiereport.xpm"
#include "../resources/checkupdate.xpm"
#include "../resources/clearlist.xpm"
#include "../resources/clock.xpm"
#include "../resources/coin.xpm"
#include "../resources/currency.xpm"
#include "../resources/customsql.xpm"
#include "../resources/delete_account.xpm"
#include "../resources/delete_custom_sql.xpm"
#include "../resources/dog.xpm"
#include "../resources/downarrow_red.xpm"
#include "../resources/downarrow.xpm"
#include "../resources/edit_account.xpm"
#include "../resources/edit_custom_sql.xpm"
#include "../resources/encrypt_db.xpm"
#include "../resources/error.xpm"
#include "../resources/exit.xpm"
#include "../resources/export-import.xpm"
#include "../resources/facebook.xpm"
#include "../resources/filter.xpm"
#include "../resources/help.xpm"
#include "../resources/house.xpm"
#include "../resources/hourglass.xpm"
#include "../resources/issues.xpm"
#include "../resources/leds.xpm"
#include "../resources/main-setup.xpm"
#include "../resources/money_dollar.xpm"
#include "../resources/money_euro.xpm"
#include "../resources/money.xpm"
#include "../resources/moneyaccount.xpm"
#include "../resources/new_custom_sql.xpm"
#include "../resources/new.xpm"
#include "../resources/newacct.xpm"
#include "../resources/new_transaction.xpm"
#include "../resources/notify.xpm"
#include "../resources/open.xpm"
#include "../resources/preferences-color.xpm"
#include "../resources/preferences-other.xpm"
#include "../resources/print.xpm"
#include "../resources/printsetup.xpm"
#include "../resources/printpreview.xpm"
#include "../resources/relocate_categories.xpm"
#include "../resources/relocate_payees.xpm"
#include "../resources/rt_exec_auto.xpm"
#include "../resources/rt_exec_user.xpm"
#include "../resources/rubik_cube.xpm"
#include "../resources/save.xpm"
#include "../resources/saveas.xpm"
#include "../resources/savings_acc_closed.xpm"
#include "../resources/savings_acc_favorite.xpm"
#include "../resources/schedule.xpm"
#include "../resources/stock_acc_closed.xpm"
#include "../resources/stock_acc_favorite.xpm"
#include "../resources/stock_acc.xpm"
#include "../resources/stock_curve.xpm"
#include "../resources/term_acc_closed.xpm"
#include "../resources/term_acc_favorite.xpm"
#include "../resources/termaccount.xpm"
#include "../resources/tipicon.xpm"
#include "../resources/trash.xpm"
#include "../resources/trees.xpm"
#include "../resources/update_currency.xpm"
#include "../resources/user_edit.xpm"
#include "../resources/view.xpm"
#include "../resources/wrench.xpm"
#include "../resources/web_money.xpm"
#include "../resources/work.xpm"
#include "../resources/yandex_money.xpm"
//----------------------------------------------------------------------------

enum
{
    /* Main Menu  */
    MENU_NEW = wxID_HIGHEST + 1,
    MENU_OPEN,
    MENU_SAVE,
    MENU_SAVE_AS,
    MENU_RECENT_FILES,
    MENU_RECENT_FILES_0,
    MENU_RECENT_FILES_CLEAR,
    MENU_EXPORT,
    MENU_NEWACCT,
    MENU_ACCTLIST,
    MENU_ORGCATEGS,
    MENU_ORGPAYEE,
    MENU_BUDGETSETUPDIALOG,
    MENU_BILLSDEPOSITS,
    MENU_CHECKUPDATE,
    MENU_IMPORT,
    MENU_IMPORT_UNIVCSV,
    MENU_REPORTISSUES,
    MENU_ANNOUNCEMENTMAILING,
    MENU_FACEBOOK,
    MENU_EXPORT_CSV,
    MENU_EXPORT_QIF,
    MENU_PRINT_PAGE_SETUP,
    MENU_PRINT_REPORT,
    MENU_PRINT,
    MENU_PRINT_PREVIEW,
    MENU_PRINT_PREVIEW_REPORT,
    MENU_SHOW_APPSTART,
    MENU_EXPORT_HTML,
    MENU_STOCKS,
    MENU_CURRENCY,
    MENU_TREEPOPUP_LAUNCHWEBSITE,
    MENU_IMPORT_MMNETCSV,
    MENU_ASSETS,
    MENU_GOTOACCOUNT,
    MENU_IMPORT_QIF,
    MENU_ACCTEDIT,
    MENU_ACCTDELETE,
    MENU_TRANSACTIONREPORT,
    MENU_VIEW_TOOLBAR,
    MENU_VIEW_LINKS,
    MENU_CATEGORY_RELOCATION,
    MENU_PAYEE_RELOCATION,
    MENU_CONVERT_ENC_DB,
    MENU_ONLINE_UPD_CURRENCY_RATE,
    MENU_IGNORE_FUTURE_TRANSACTIONS,

    //
    MENU_TREEPOPUP_NEW,
    MENU_TREEPOPUP_EDIT,
    MENU_TREEPOPUP_MOVE,
    MENU_TREEPOPUP_DELETE,

    ID_NAVTREECTRL,
    ID_PANEL_HOME,
    ID_BUTTON_CURRENCY,
    ID_DIALOG_OPTIONS_BUTTON_CURRENCY,
    ID_DIALOG_OPTIONS_TEXTCTRL_USERNAME,


    //
    MENU_TREEPOPUP_ACCOUNT_NEW,
    MENU_TREEPOPUP_ACCOUNT_DELETE,
    MENU_TREEPOPUP_ACCOUNT_EDIT,
    MENU_TREEPOPUP_ACCOUNT_LIST,
    MENU_TREEPOPUP_ACCOUNT_EXPORT2CSV,
    MENU_TREEPOPUP_ACCOUNT_EXPORT2QIF,
    MENU_TREEPOPUP_ACCOUNT_IMPORTCSV,
    MENU_TREEPOPUP_ACCOUNT_IMPORTQIF,
    MENU_TREEPOPUP_ACCOUNT_IMPORTUNIVCSV,
    MENU_TREEPOPUP_ACCOUNT_IMPORTMMNET,
    MENU_TREEPOPUP_ACCOUNT_VIEWALL,
    MENU_TREEPOPUP_ACCOUNT_VIEWFAVORITE,
    MENU_TREEPOPUP_ACCOUNT_VIEWOPEN,
    AUTO_REPEAT_TRANSACTIONS_TIMER_ID,

};

//----------------------------------------------------------------------------
#endif // _MM_EX_GUIID_H_
//----------------------------------------------------------------------------

