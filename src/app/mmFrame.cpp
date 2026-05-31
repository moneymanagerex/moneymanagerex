/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2012 Stefano Giorgio
 Copyright (C) 2013 - 2022 Nikolay Akimov
 Copyright (C) 2014 James Higley
 Copyright (C) 2014 Guan Lisheng (guanlisheng@gmail.com)
 Copyright (C) 2021, 2022, 2024-2025 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2025, 2026 Klaus Wich

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

#include "mmFrame.h"

#include "base/_defs.h"
#include <stack>
#include <unordered_set>
#include <wx/fs_mem.h>
#include <wx/busyinfo.h>

#include "base/_constants.h"
#include "util/mmImage.h"
#include "util/mmSQLite3Hook.h"
#include "util/mmTreeItemData.h"
#include "util/mmFileHistory.h"
#include "util/mmSingleChoice.h"
#include "util/mmNavigatorList.h"
#include "util/mmToolbarList.h"
#include "util/mmMultiChoice.h"
#include "util/mmAttachment.h"
#include "util/_util.h"
#include "util/_simple.h"

#include "db/dbcheck.h"
#include "db/dbupgrade.h"
#include "db/dbwrapper.h"
#include "model/_all.h"

#include "panel/AssetPanel.h"
#include "panel/BudgetPanel.h"
#include "panel/DashboardPanel.h"
#include "panel/HelpPanel.h"
#include "panel/JournalList.h"
#include "panel/JournalPanel.h"
#include "panel/ReportPanel.h"
#include "panel/SchedPanel.h"
#include "panel/StockPanel.h"

#include "manager/CategoryManager.h"
#include "manager/DateRangeManager.h"
#include "manager/FieldManager.h"
#include "manager/GeneralReportManager.h"
#include "manager/PayeeManager.h"
#include "manager/PrefManager.h"
#include "manager/TagManager.h"
#include "manager/ThemeManager.h"

#include "dialog/AboutDialog.h"
#include "dialog/AccountDialog.h"
#include "dialog/AttachmentDialog.h"
#include "dialog/BudgetYearDialog.h"
#include "dialog/CurrencyChoiceDialog.h"
#include "dialog/DiagnosticsDialog.h"
#include "dialog/MergeCategoryDialog.h"
#include "dialog/MergePayeeDialog.h"
#include "dialog/MergeTagDialog.h"
#include "dialog/SchedDialog.h"
#include "dialog/StartupDialog.h"
#include "dialog/TrxDialog.h"
#include "dialog/TrxFilterDialog.h"
#include "dialog/NavigatorDialog.h"
#include "dialog/ToolbarDialog.h"
#include "dialog/ReconcileDialog.h"

#include "report/_all.h"
#include "report/GeneralGroupReport.h"
#include "report/bugreport.h"

#include "import_export/qif_export.h"
#include "import_export/qif_import_gui.h"
#include "import_export/univcsvdialog.h"
#include "import_export/ofx_import_gui.h"
#include "import_export/webapp.h"
#include "import_export/webappdialog.h"
#include "wizard/wizard_newaccount.h"
#include "wizard/wizard_newdb.h"
#include "wizard/wizard_update.h"
#include "mmApp.h"

//----------------------------------------------------------------------------

int REPEAT_FREQ_TRANS_DELAY_TIME = 3000; // 3 seconds

void mmToolbarArt::DrawPlainBackground(wxDC& dc, wxWindow* WXUNUSED(wnd), const wxRect& rect)
{
    dc.GradientFillLinear(rect,
        mmImage::themeMetaColour(mmImage::COLOR_TOOLBAR),
        mmImage::themeMetaColour(mmImage::COLOR_TOOLBAR)
    );
}

void mmToolbarArt::DrawButton(
    wxDC& dc,
    wxWindow* wnd,
    const wxAuiToolBarItem& item,
    const wxRect& rect
) {
    //wxColour clr = mmImage::themeMetaColour(mmImage::COLOR_TOOLBAR);
    bool dark = isDark(mmImage::themeMetaColour(mmImage::COLOR_TOOLBAR));
    int bmpX = 0, bmpY = 0;

    const wxBitmap& bmp = item.GetCurrentBitmapFor(wnd);
    const wxSize bmpSize = bmp.GetLogicalSize();

    bmpX = rect.x + (rect.width / 2) - (bmpSize.x / 2);
    bmpY = rect.y + (rect.height / 2) - (bmpSize.y / 2);

    if (!(item.GetState() & wxAUI_BUTTON_STATE_DISABLED)) {

        if (item.GetState() & wxAUI_BUTTON_STATE_PRESSED) {
            dc.SetPen(wxPen(m_highlightColour));
            dc.SetBrush(wxBrush(m_highlightColour.ChangeLightness(dark ? 10 : 140)));
            dc.DrawRectangle(rect);
        }
        else if (item.GetState() & wxAUI_BUTTON_STATE_HOVER) {
            dc.SetPen(wxPen(m_highlightColour));
            dc.SetBrush(wxBrush(m_highlightColour.ChangeLightness(dark ? 40 : 170)));
            dc.DrawRectangle(rect);
        }
    }
    dc.DrawBitmap(bmp, bmpX, bmpY, true);
}

//----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(mmFrame, wxFrame)

EVT_MENU(MENU_NEW,                             mmFrame::OnNew)
EVT_MENU(MENU_OPEN,                            mmFrame::OnOpen)
EVT_MENU(MENU_SAVE_AS,                         mmFrame::OnSaveAs)
EVT_MENU(MENU_EXPORT_CSV,                      mmFrame::OnExportToCSV)
EVT_MENU(MENU_EXPORT_XML,                      mmFrame::OnExportToXML)
EVT_MENU(MENU_EXPORT_QIF,                      mmFrame::OnExportToQIF)
EVT_MENU(MENU_EXPORT_JSON,                     mmFrame::OnExportToJSON)
EVT_MENU(MENU_EXPORT_MMEX,                     mmFrame::OnExportToMMEX)
EVT_MENU(MENU_IMPORT_QIF,                      mmFrame::OnImportQIF)
EVT_MENU(MENU_IMPORT_OFX,                      mmFrame::OnImportOFX)
EVT_MENU(MENU_IMPORT_UNIVCSV,                  mmFrame::OnImportUniversalCSV)
EVT_MENU(MENU_IMPORT_XML,                      mmFrame::OnImportXML)
EVT_MENU(MENU_IMPORT_WEBAPP,                   mmFrame::OnImportWebApp)
EVT_MENU(wxID_EXIT,                            mmFrame::OnQuit)
EVT_MENU(MENU_NEWACCT,                         mmFrame::OnNewAccount)
EVT_MENU(MENU_HOMEPAGE,                        mmFrame::OnAccountList)
EVT_MENU(MENU_ACCTEDIT,                        mmFrame::OnEditAccount)
EVT_MENU(MENU_ACCTDELETE,                      mmFrame::OnDeleteAccount)
EVT_MENU(MENU_ACCOUNT_REALLOCATE,              mmFrame::OnReallocateAccount)
EVT_MENU(MENU_ORGCATEGS,                       mmFrame::OnOrgCategories)
EVT_MENU(MENU_ORGPAYEE,                        mmFrame::OnOrgPayees)
EVT_MENU(MENU_ORGTAGS,                         mmFrame::OnOrgTags)
EVT_MENU(wxID_PREFERENCES,                     mmFrame::OnOptions)
EVT_MENU(wxID_NEW,                             mmFrame::OnNewTransaction)
EVT_MENU(wxID_REFRESH,                         mmFrame::refreshPanelData)
EVT_MENU(MENU_BUDGETSETUPDIALOG,               mmFrame::OnBudgetSetupDialog)

EVT_MENU(MENU_TRANSACTIONS_ALL,                mmFrame::OnTransactionsAll)
EVT_MENU(MENU_TRANSACTIONS_DEL,                mmFrame::OnTransactionsDel)

EVT_MENU(wxID_HELP,                            mmFrame::OnHelp)
EVT_MENU(MENU_CHECKUPDATE,                     mmFrame::OnCheckUpdate)
EVT_MENU(MENU_ANNOUNCEMENTMAILING,             mmFrame::OnBeNotified)
EVT_MENU_RANGE(MENU_FACEBOOK, MENU_TWITTER,    mmFrame::OnSimpleURLOpen)
EVT_MENU(MENU_REPORT_BUG,                      mmFrame::OnReportBug)
EVT_MENU(MENU_DIAGNOSTICS,                     mmFrame::OnDiagnostics)
EVT_MENU(wxID_ABOUT,                           mmFrame::OnAbout)
EVT_MENU(wxID_PRINT,                           mmFrame::OnPrintPage)
EVT_MENU(MENU_SHOW_APPSTART,                   mmFrame::OnShowAppStartDialog)
EVT_MENU(MENU_EXPORT_HTML,                     mmFrame::OnExportToHtml)
EVT_MENU(MENU_BILLSDEPOSITS,                   mmFrame::OnBillsDeposits)
EVT_MENU(MENU_CONVERT_ENC_DB,                  mmFrame::OnConvertEncryptedDB)
EVT_MENU(MENU_CHANGE_ENCRYPT_PASSWORD,         mmFrame::OnChangeEncryptPassword)
EVT_MENU(MENU_DB_VACUUM,                       mmFrame::OnVacuumDB)
EVT_MENU(MENU_DB_DEBUG,                        mmFrame::OnDebugDB)
EVT_MENU(MENU_DB_COOKIE_RESET,                 mmFrame::OnCookieReset)

EVT_MENU(MENU_ASSETS,                          mmFrame::OnAssets)
EVT_MENU(MENU_CURRENCY,                        mmFrame::OnCurrency)
EVT_MENU(MENU_RATES,                           mmFrame::OnRates)
EVT_MENU(MENU_TRANSACTIONREPORT,               mmFrame::OnTransactionReport)
EVT_MENU(MENU_REFRESH_WEBAPP,                  mmFrame::OnRefreshWebApp)
EVT_MENU(wxID_BROWSE,                          mmFrame::OnCustomFieldsManager)
EVT_MENU(wxID_VIEW_LIST,                       mmFrame::OnGeneralReportManager)
EVT_MENU(MENU_THEME_MANAGER,                   mmFrame::OnThemeManager)
EVT_MENU(MENU_DATE_RANGE_MANAGER,              mmFrame::OnDateRangeManager)
EVT_MENU(MENU_TREEPOPUP_LAUNCHWEBSITE,         mmFrame::OnLaunchAccountWebsite)
EVT_MENU(MENU_TREEPOPUP_ACCOUNTATTACHMENTS,    mmFrame::OnAccountAttachments)
EVT_MENU(MENU_TREEPOPUP_RECONCILE,             mmFrame::OnReconcileAccount)
EVT_MENU(MENU_VIEW_TOOLBAR,                    mmFrame::OnViewToolbar)
EVT_MENU(MENU_VIEW_LINKS,                      mmFrame::OnViewLinks)
EVT_MENU(MENU_VIEW_HIDE_SHARE_ACCOUNTS,        mmFrame::OnHideShareAccounts)
EVT_MENU(MENU_VIEW_HIDE_DELETED_TRANSACTIONS,  mmFrame::OnHideDeletedTransactions)
EVT_MENU(MENU_VIEW_BUDGET_FINANCIAL_YEARS,     mmFrame::OnViewBudgetFinancialYears)
EVT_MENU(MENU_VIEW_BUDGET_TRANSFER_TOTAL,      mmFrame::OnViewBudgetTransferTotal)
EVT_MENU(MENU_VIEW_BUDGET_CATEGORY_SUMMARY,    mmFrame::OnViewBudgetCategorySummary)
EVT_MENU(MENU_VIEW_IGNORE_FUTURE_TRANSACTIONS, mmFrame::OnViewIgnoreFutureTransactions)
EVT_MENU(MENU_VIEW_SHOW_TOOLTIPS,              mmFrame::OnViewShowToolTips)
EVT_MENU(MENU_VIEW_SHOW_MONEYTIPS,             mmFrame::OnViewShowMoneyTips)

EVT_MENU(MENU_CATEGORY_RELOCATION,             mmFrame::OnCategoryRelocation)
EVT_MENU(MENU_PAYEE_RELOCATION,                mmFrame::OnPayeeRelocation)
EVT_MENU(MENU_TAG_RELOCATION,                  mmFrame::OnTagRelocation)

EVT_UPDATE_UI(MENU_VIEW_TOOLBAR,               mmFrame::OnViewToolbarUpdateUI)
EVT_UPDATE_UI(MENU_VIEW_LINKS,                 mmFrame::OnViewLinksUpdateUI)
EVT_MENU(MENU_TREEPOPUP_NEW,                   mmFrame::OnNewTransaction)
EVT_MENU(MENU_TREEPOPUP_EDIT,                  mmFrame::OnPopupEditAccount)
EVT_MENU(MENU_TREEPOPUP_REALLOCATE,            mmFrame::OnPopupReallocateAccount)
EVT_MENU(MENU_TREEPOPUP_DELETE,                mmFrame::OnPopupDeleteAccount)

EVT_MENU(MENU_TREEPOPUP_EXPAND_ALL,            mmFrame::OnEmptyTreePopUp)
EVT_MENU(MENU_TREEPOPUP_COLLAPSE_ALL,          mmFrame::OnEmptyTreePopUp)
EVT_MENU(MENU_TREEPOPUP_CONFIG_NAV,            mmFrame::OnEmptyTreePopUp)
EVT_MENU(MENU_TREEPOPUP_THEME,                 mmFrame::OnEmptyTreePopUp)

EVT_MENU(MENU_TREEPOPUP_FILTER_DELETE,         mmFrame::OnPopupDeleteFilter)
EVT_MENU(MENU_TREEPOPUP_FILTER_RENAME,         mmFrame::OnPopupRenameFilter)
EVT_MENU(MENU_TREEPOPUP_FILTER_EDIT,           mmFrame::OnPopupEditFilter)

EVT_TREE_ITEM_EXPANDED(ID_NAVTREECTRL,         mmFrame::OnTreeItemExpanded)
EVT_TREE_ITEM_COLLAPSED(ID_NAVTREECTRL,        mmFrame::OnTreeItemCollapsed)

EVT_DROP_FILES(                                mmFrame::OnDropFiles)

EVT_MENU(MENU_GOTOACCOUNT,                     mmFrame::OnGotoAccount)

/* Navigation Panel */
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_NEW,           mmFrame::OnNewAccount)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_DELETE,        mmFrame::OnDeleteAccount)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_EDIT,          mmFrame::OnEditAccount)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_LIST,          mmFrame::OnAccountList)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_EXPORT2CSV,    mmFrame::OnExportToCSV)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_EXPORT2MMEX,   mmFrame::OnExportToMMEX)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_EXPORT2XML,    mmFrame::OnExportToXML)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_EXPORT2QIF,    mmFrame::OnExportToQIF)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_EXPORT2JSON,   mmFrame::OnExportToJSON)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_IMPORTUNIVCSV, mmFrame::OnImportUniversalCSV)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_IMPORTXML,     mmFrame::OnImportXML)
EVT_MENU(MENU_TREEPOPUP_ACCOUNT_IMPORTQIF,     mmFrame::OnImportQIF)

EVT_MENU_RANGE(
    MENU_TREEPOPUP_ACCOUNT_VIEWALL,
    MENU_TREEPOPUP_ACCOUNT_VIEWCLOSED,         mmFrame::OnViewAccountsTemporaryChange)
EVT_MENU_RANGE(MENU_LANG + 1, MENU_LANG_MAX,   mmFrame::OnChangeGUILanguage)

/*Automatic processing of repeat transactions*/
EVT_TIMER(AUTO_REPEAT_TRANSACTIONS_TIMER_ID,   mmFrame::OnAutoRepeatTransactionsTimer)

/* Recent Files */
EVT_MENU_RANGE(wxID_FILE1, wxID_FILE9,         mmFrame::OnRecentFiles)
EVT_MENU(MENU_RECENT_FILES_CLEAR,              mmFrame::OnClearRecentFiles)
EVT_MENU(MENU_VIEW_TOGGLE_FULLSCREEN,          mmFrame::OnToggleFullScreen)
EVT_MENU(MENU_VIEW_RESET,                      mmFrame::OnResetView)
EVT_CLOSE(                                     mmFrame::OnClose)

wxEND_EVENT_TABLE()

//----------------------------------------------------------------------------

mmFrame::mmFrame(
    mmApp* app,
    const wxString& title,
    const wxPoint& pos,
    const wxSize& size
) :
    wxFrame(nullptr, wxID_ANY, title, pos, size),
    m_app(app),
    autoRepeatTransactionsTimer_(this, AUTO_REPEAT_TRANSACTIONS_TIMER_ID)
{
    // tell wxAuiManager to manage this frame
    m_mgr.SetManagedWindow(this);
    SetIcon(mmPath::getProgramIcon());
    SetMinSize(wxSize(480, 275));

#if (wxMAJOR_VERSION == 3 && wxMINOR_VERSION >= 1)
    // Initialize code to turn on Mac OS X Fullscreen capabilities (Lion and up only)
    // code is a noop for all other systems
    EnableFullScreenView(true);
#endif
    // decide if we need to show app start dialog
    bool from_scratch = false;
    wxFileName dbpath = m_app->GetOptParam();
    if (!dbpath.IsOk()) {
        from_scratch = SettingModel::instance().getBool("SHOWBEGINAPP", true);
        if (from_scratch)
            dbpath = wxGetEmptyString();
        else
            dbpath = SettingModel::instance().getLastDbPath();
    }

    // Read news, if checking enabled
    if (PrefModel::instance().getCheckNews())
        getNewsRSS(websiteNewsArray_);

    /* Create the Controls for the frame */
    mmFontSize(this);
    mmImage::loadTheme();
    createMenu();
    createControls();
    createToolBar();

#if wxUSE_STATUSBAR
    CreateStatusBar();
    mmImage::themeMetaColour(GetStatusBar(), mmImage::COLOR_LISTPANEL);
#endif // wxUSE_STATUSBAR
    m_recentFiles = new mmFileHistory(); // TODO Max files
    m_recentFiles->SetMenuPathStyle(wxFH_PATH_SHOW_ALWAYS);
    m_recentFiles->UseMenu(m_menuRecentFiles);
    m_recentFiles->LoadHistory();

    // Load perspective
    const wxString auiPerspective = SettingModel::instance().getString("AUIPERSPECTIVE", wxEmptyString);
    m_mgr.LoadPerspective(auiPerspective);

    // add the toolbars to the manager
    m_mgr.AddPane(toolBar_, wxAuiPaneInfo()
        .Name("toolbar").ToolbarPane().Top()
        .LeftDockable(false).RightDockable(false)
        .Show(SettingModel::instance().getBool("SHOWTOOLBAR", true))
        .DockFixed(false)
        .Top()
        .MinSize(wxSize(-1, toolBar_->GetSize().GetHeight()))
        .MaxSize(wxSize(-1, toolBar_->GetSize().GetHeight()))
        .Position(0)
        .Row(0)
        .Layer(0)
        .Resizable(true)
        );

    // change look and feel of wxAuiManager
    toolBar_->SetArtProvider(new mmToolbarArt);
    m_mgr.GetArtProvider()->SetColour(wxAUI_DOCKART_BACKGROUND_COLOUR, mmImage::themeMetaColour(mmImage::COLOR_TOOLBAR));
    m_mgr.GetArtProvider()->SetColour(wxAUI_DOCKART_SASH_COLOUR, mmImage::themeMetaColour(mmImage::COLOR_LISTPANEL));
    m_mgr.GetArtProvider()->SetColour(wxAUI_DOCKART_BORDER_COLOUR, mmImage::themeMetaColour(mmImage::COLOR_LISTPANEL));
    m_mgr.GetArtProvider()->SetMetric(16, 0);
    m_mgr.GetArtProvider()->SetMetric(3, 1);

    // "commit" all changes made to wxAuiManager
    m_mgr.GetPane("Navigation").Caption(_t("Navigator"));
    m_mgr.GetPane("toolbar").Caption(_t("Toolbar"));
    m_mgr.GetPane("toolbar").PaneBorder(false);
    m_mgr.Update();

    // store reference for toolbar updates
    mmToolbarList::instance().SetToolbarParent(this);

    // Show license agreement at first open
    if (SettingModel::instance().getString(INIDB_SEND_USAGE_STATS, "") == "") {
        AboutDialog(this, 4).ShowModal();
    }

    //Check for new version at startup
    if (SettingModel::instance().getBool("UPDATECHECK", true))
        mmUpdate::checkUpdates(this, true);

    //Show appstart
    if (from_scratch || !dbpath.IsOk()) {
        menuEnableItems(false);
        showBeginAppDialog(dbpath.GetFullName().IsEmpty());
    }
    else {
        if (openFile(dbpath.GetFullPath(), false)) {
            DoRecreateNavTreeControl(true);
            //setHomePageActive(false);
            mmLoadColorsFromDatabase();
        }
        else {
            resetNavTreeControl();
            cleanupHomePanel();
            showBeginAppDialog(true);
        }
    }

    const wxAcceleratorEntry entries[] = {
        wxAcceleratorEntry(wxACCEL_NORMAL, WXK_F9, wxID_NEW),
        wxAcceleratorEntry(wxACCEL_NORMAL, WXK_F5, wxID_REFRESH),
        wxAcceleratorEntry(wxACCEL_NORMAL, WXK_F6, MENU_VIEW_LINKS),
    };

    wxAcceleratorTable tab(sizeof(entries) / sizeof(*entries), entries);
    SetAcceleratorTable(tab);

    if (m_db) {
        // Clean up deleted transactions
        autocleanDeletedTransactions();

        // Refresh stock quotes
        if (SettingModel::instance().getBool("REFRESH_STOCK_QUOTES_ON_OPEN", false) &&
            StockModel::instance().find_count() > 0
        ) {
            wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_RATES);
            this->GetEventHandler()->AddPendingEvent(evt);
        }
    }
    wxColour c = mmImage::themeMetaColour(mmImage::COLOR_LISTPANEL);
    mmImage::themeMetaColour(this, isDark(c) ? c.ChangeLightness(140) : c.ChangeLightness(70));
}
//----------------------------------------------------------------------------

mmFrame::~mmFrame()
{
    try {
        cleanup();
    }
    catch (...) {
        wxASSERT(false);
    }

    // Report database statistics
    for (const auto & model : this->m_all_models) {
        model->debug_stat();
        UsageModel::instance().append_cache(model->stat_json());
    }
}
//----------------------------------------------------------------------------

void mmFrame::cleanup()
{
    autoRepeatTransactionsTimer_.Stop();

    if (!m_filename.IsEmpty()) // Exiting before file is opened
        saveSettings();


    if (m_db) {
        navTreeStateToJson();
    }
    wxTreeItemId rootitem = m_nav_tree_ctrl->GetRootItem();
    cleanupNavTreeControl(rootitem);
    m_mgr.UnInit();

    /* Delete the GUI */
    cleanupHomePanel(false);
    ShutdownDatabase();
    mmImage::closeTheme();

    // Backup the database according to user requirements
    if (PrefModel::instance().getDatabaseUpdated() &&
        SettingModel::instance().getBool("BACKUPDB_UPDATE", false)
    ) {
        dbUpgrade::BackupDB(
            m_filename,
            dbUpgrade::BACKUPTYPE::CLOSE,
            SettingModel::instance().getInt("MAX_BACKUP_FILES", 4)
        );
    }
}

void mmFrame::ShutdownDatabase()
{
    if (!m_db)
        return;

    // Cache empty on InfoModel means instance never initialized
    if (!InfoModel::instance().cache_empty()) {
        if (!db_lockInPlace)
            InfoModel::instance().saveBool("ISUSED", false);
    }
    m_db->SetCommitHook(nullptr);
    m_db->Close();
    m_db.reset();
    for (auto& model : m_all_models)
        model->reset_cache();
}

void mmFrame::resetNavTreeControl()
{
    wxTreeItemId root = m_nav_tree_ctrl->GetRootItem();
    cleanupNavTreeControl(root);
    m_nav_tree_ctrl->DeleteAllItems();
    // -Check if image list was changed and update if necessary:
    if (NavTreeIconImages::instance().isListChanged()) {
        mmNavigatorList::instance().LoadFromDB();  // reinit index
        const auto navIconSize = PrefModel::instance().getNavigationIconSize();
        m_nav_tree_ctrl->SetImages(NavTreeIconImages::instance().getList(navIconSize));
    }
}

void mmFrame::cleanupNavTreeControl(wxTreeItemId& item)
{
    while (item.IsOk()) {
        if (m_nav_tree_ctrl->ItemHasChildren(item)) {
            wxTreeItemIdValue cookie;
            wxTreeItemId childitem = m_nav_tree_ctrl->GetFirstChild(item, cookie);
            cleanupNavTreeControl(childitem);
        }
        mmTreeItemData* iData = dynamic_cast<mmTreeItemData*>(m_nav_tree_ctrl->GetItemData(item));
        m_nav_tree_ctrl->SetItemData(item, nullptr);
        if (iData)
            delete iData;

        if (item != m_nav_tree_ctrl->GetRootItem())
            item = m_nav_tree_ctrl->GetNextSibling(item);
        else
            break;
    }
}

//----------------------------------------------------------------------------
// process all events waiting in the event queue if any.
void mmFrame::processPendingEvents()
{
    while (m_app->Pending())
        m_app->Dispatch();
}

//----------------------------------------------------------------------------
wxTreeItemId mmFrame::getNavTreeChild(const wxTreeItemId& section, const wxString& childName) const
{
    wxTreeItemId child;
    if (section) {
        wxTreeItemIdValue cookie;
        child = m_nav_tree_ctrl->GetFirstChild(section, cookie);
        while (child.IsOk()) {
            if (m_nav_tree_ctrl->GetItemText(child) == childName)
                break;
            child = m_nav_tree_ctrl->GetNextChild(section, cookie);
        }
    }
    return child;
}

void mmFrame::setNavTreeSection(const wxString &sectionName)
{
    // Set the NavTreeCtrl and prevent any event code being executed for now.
    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);
    wxTreeItemId section = getNavTreeChild(m_nav_tree_ctrl->GetRootItem(), sectionName);
    if (section.IsOk()) {
        m_nav_tree_ctrl->SelectItem(section);
    }
    else {
        m_nav_tree_ctrl->SelectItem(m_nav_tree_ctrl->GetRootItem());
    }
    m_nav_tree_ctrl->SetEvtHandlerEnabled(true);
}

void mmFrame::setNavTreeSectionById(int sectionid)
{
    wxString secname = mmNavigatorList::instance().getAccountSectionName(sectionid);
    selectNavTreeItem(secname);
}


void mmFrame::selectNavTreeItem(const wxString& account_name)
{
    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);
    if (!findAndSelectNavTreeItem(m_nav_tree_ctrl->GetRootItem(), account_name)) {
        m_nav_tree_ctrl->SelectItem(m_nav_tree_ctrl->GetRootItem());
    }
    m_nav_tree_ctrl->SetEvtHandlerEnabled(true);
}

bool mmFrame::findAndSelectNavTreeItem(const wxTreeItemId& treeitem, const wxString& itemName)  // recursive!
{
    if (!treeitem.IsOk()) {
        return false;
    }

    if (m_nav_tree_ctrl->GetItemText(treeitem) == itemName) {
        wxTreeItemId p = treeitem;
        while (p.IsOk() && p != m_nav_tree_ctrl->GetRootItem()) {
            m_nav_tree_ctrl->Expand(p);
            p = m_nav_tree_ctrl->GetItemParent(p);
        }

        m_nav_tree_ctrl->SelectItem(treeitem);
        m_nav_tree_ctrl->SetFocusedItem(treeitem);
        m_nav_tree_ctrl->EnsureVisible(treeitem);

        return true;
    }

    wxTreeItemIdValue cookie;
    wxTreeItemId child = m_nav_tree_ctrl->GetFirstChild(treeitem, cookie);
    while (child.IsOk()) {
        if (findAndSelectNavTreeItem(child, itemName)) {
            return true;
        }
       child = m_nav_tree_ctrl->GetNextChild(treeitem, cookie);
    }
    return false;
}

void mmFrame::OnAutoRepeatTransactionsTimer(wxTimerEvent& /*event*/)
{
    // WebApp check
    if (mmWebApp::isEnabled()) {
        if (OnRefreshWebApp(true)) {
            mmWebAppDialog dlg(this, true);
            dlg.ShowModal();
            if (dlg.getRefreshRequested())
                refreshPanelData();
        }
    }

    // Auto scheduled transaction
    bool continueExecution = false;

    for (SchedData& sched_d : SchedModel::instance().find_data_a(
        TableClause::ORDERBY(SchedCol::s_primary_name)
    )) {
        mmDate today = mmDate::today();
        if (sched_d.m_due_date.daysSince(today) > 0)
            continue;

        bool is_allowed = SchedModel::instance().is_data_allowed(sched_d);
        if (sched_d.m_repeat.m_mode.id() == RepeatMode::e_suggested) {
            if (is_allowed) {
                continueExecution = true;
                SchedDialog sched_dlg(this, SchedDialog::MODE_ENTER, sched_d.m_id);
                sched_dlg.setDialogHeader(_t("Auto Repeat Transactions"));
                if (sched_dlg.ShowModal() == wxID_OK) {
                    refreshPanelData();
                }
                else {
                    // stop repeat executions from occuring
                    continueExecution = false;
                }
            }
        }
        else if (sched_d.m_repeat.m_mode.id() == RepeatMode::e_automated) {
            if (is_allowed) {
                continueExecution = true;
                TrxData new_trx_d = TrxData();
                new_trx_d.m_datetime        = sched_d.m_datetime; // TODO: reset time
                new_trx_d.m_type            = sched_d.m_type;
                new_trx_d.m_status          = sched_d.m_status;
                new_trx_d.m_account_id      = sched_d.m_account_id;
                new_trx_d.m_to_account_id_n = sched_d.m_to_account_id_n;
                new_trx_d.m_payee_id_n      = sched_d.m_payee_id_n;
                new_trx_d.m_category_id_n   = sched_d.m_category_id_n;
                new_trx_d.m_amount          = sched_d.m_amount;
                new_trx_d.m_to_amount       = sched_d.m_to_amount;
                new_trx_d.m_number          = sched_d.m_number;
                new_trx_d.m_notes           = sched_d.m_notes;
                new_trx_d.m_followup_id     = sched_d.m_followup_id;
                new_trx_d.m_color           = sched_d.m_color;
                TrxModel::instance().save_trx_n(new_trx_d);
                int64 new_trx_id = new_trx_d.m_id;

                TrxSplitModel::DataA tp_a;
                std::vector<wxArrayInt64> splitTags;
                for (const auto& qp_d : SchedModel::instance().find_id_qp_a(sched_d.m_id)) {
                    TrxSplitData tp_d = TrxSplitData();
                    tp_d.m_trx_id      = new_trx_id;
                    tp_d.m_category_id = qp_d.m_category_id;
                    tp_d.m_amount      = qp_d.m_amount;
                    tp_d.m_notes       = qp_d.m_notes;
                    tp_a.push_back(tp_d);

                    wxArrayInt64 tags;
                    for (const auto& gl_d : TagLinkModel::instance().find_data_a(
                        TagLinkCol::WHERE_REFTYPE(OP_EQ, SchedSplitModel::s_ref_type.key_n()),
                        TagLinkCol::WHERE_REFID(OP_EQ, qp_d.m_id)
                    )) {
                        tags.push_back(gl_d.m_tag_id);
                    }
                    splitTags.push_back(tags);
                }
                TrxSplitModel::instance().save_data_a(tp_a);

                // Save split tags
                for (size_t i = 0; i < tp_a.size(); i++) {
                    TagLinkModel::DataA new_gl_a;
                    for (const auto& tagId : splitTags.at(i)) {
                        TagLinkData new_gl_d = TagLinkData();
                        new_gl_d.m_tag_id   = tagId;
                        new_gl_d.m_ref_type = TrxSplitModel::s_ref_type;
                        new_gl_d.m_ref_id   = tp_a[i].m_id;
                        new_gl_a.push_back(new_gl_d);
                    }
                    TagLinkModel::instance().update(
                        TrxSplitModel::s_ref_type, tp_a.at(i).m_id,
                        new_gl_a
                    );
                }

                // Copy the custom fields to the newly created transaction
                FieldValueModel::instance().db_savepoint();
                for (const auto& fv_d : FieldValueModel::instance().find_data_a(
                    FieldValueModel::WHERE_REFTYPEID(SchedModel::s_ref_type, sched_d.m_id)
                )) {
                    FieldValueData new_fv_d = FieldValueData();
                    new_fv_d.m_field_id = fv_d.m_field_id;
                    new_fv_d.m_ref_type = RefTypeN(RefTypeN::e_trx);
                    new_fv_d.m_ref_id   = new_trx_id;
                    new_fv_d.m_content = fv_d.m_content;
                    FieldValueModel::instance().add_data_n(new_fv_d);
                }
                FieldValueModel::instance().db_release_savepoint();

                // Save base transaction tags
                TagLinkModel::DataA new_gl_a;
                for (const auto& gl_d : TagLinkModel::instance().find_data_a(
                    TagLinkCol::WHERE_REFTYPE(OP_EQ, SchedModel::s_ref_type.key_n()),
                    TagLinkCol::WHERE_REFID(OP_EQ, sched_d.m_id)
                )) {
                    TagLinkData new_gl_d = TagLinkData();
                    new_gl_d.m_tag_id   = gl_d.m_tag_id;
                    new_gl_d.m_ref_type = TrxModel::s_ref_type;
                    new_gl_d.m_ref_id   = new_trx_id;
                    new_gl_a.push_back(new_gl_d);
                }
                TagLinkModel::instance().update(
                    TrxModel::s_ref_type, new_trx_id,
                    new_gl_a
                );
            }
            SchedModel::instance().reschedule_id(sched_d.m_id);
        }
    }

    if (continueExecution)
        autoRepeatTransactionsTimer_.Start(5, wxTIMER_ONE_SHOT);
    else
        refreshPanelData();
}
//----------------------------------------------------------------------------

// Save our settings to ini db.
void mmFrame::saveSettings()
{
    SettingModel::instance().db_savepoint();
    if (!m_filename.IsEmpty()) {
        wxFileName fname(m_filename);
        if (fname.GetExt().Upper() != "BAK") {
            SettingModel::instance().saveString("LASTFILENAME", fname.GetFullPath());
        }
    }
    /* Aui Settings */
    SettingModel::instance().saveString("AUIPERSPECTIVE", m_mgr.SavePerspective());

    // prevent values being saved while window is in an iconised state.
    if (this->IsIconized()) this->Restore();

    int value_x = 0, value_y = 0;
    this->GetPosition(&value_x, &value_y);
    SettingModel::instance().saveInt("ORIGINX", value_x);
    SettingModel::instance().saveInt("ORIGINY", value_y);

    int value_w = 0, value_h = 0;
    this->GetSize(&value_w, &value_h);
    SettingModel::instance().saveInt("SIZEW", value_w);
    SettingModel::instance().saveInt("SIZEH", value_h);
    SettingModel::instance().saveBool("ISMAXIMIZED", this->IsMaximized());
    SettingModel::instance().db_release_savepoint();
}
//----------------------------------------------------------------------------

void mmFrame::menuEnableItems(bool enable)
{
    menuBar_->FindItem(MENU_SAVE_AS)->Enable(enable);
    menuBar_->FindItem(MENU_EXPORT)->Enable(enable);
    menuBar_->FindItem(MENU_NEWACCT)->Enable(enable);
    menuBar_->FindItem(MENU_HOMEPAGE)->Enable(enable);
    menuBar_->FindItem(MENU_ACCTEDIT)->Enable(enable);
    menuBar_->FindItem(MENU_ACCOUNT_REALLOCATE)->Enable(enable);
    menuBar_->FindItem(MENU_ACCTDELETE)->Enable(enable);

    menuBar_->FindItem(MENU_ORGCATEGS)->Enable(enable);
    menuBar_->FindItem(MENU_ORGPAYEE)->Enable(enable);
    menuBar_->FindItem(MENU_CATEGORY_RELOCATION)->Enable(enable);
    menuBar_->FindItem(MENU_PAYEE_RELOCATION)->Enable(enable);
    menuBar_->FindItem(MENU_TAG_RELOCATION)->Enable(enable);
    menuBar_->FindItem(wxID_VIEW_LIST)->Enable(enable);
    menuBar_->FindItem(wxID_BROWSE)->Enable(enable);
    menuBar_->FindItem(MENU_CONVERT_ENC_DB)->Enable(enable);
    menuBar_->FindItem(MENU_RATES)->Enable(enable);
    menuBar_->FindItem(MENU_ORGTAGS)->Enable(enable);
    menuBar_->FindItem(MENU_THEME_MANAGER)->Enable(enable);
    menuBar_->FindItem(MENU_DATE_RANGE_MANAGER)->Enable(enable);

    menuBar_->FindItem(MENU_IMPORT)->Enable(enable);
    menuBar_->FindItem(wxID_PRINT)->Enable(enable);
    menuBar_->FindItem(wxID_PREFERENCES)->Enable(enable);
    menuBar_->FindItem(MENU_BILLSDEPOSITS)->Enable(enable);
    menuBar_->FindItem(MENU_CURRENCY)->Enable(enable);
    menuBar_->FindItem(MENU_ASSETS)->Enable(enable);
    menuBar_->FindItem(MENU_BUDGETSETUPDIALOG)->Enable(enable);
    menuBar_->FindItem(MENU_TRANSACTIONREPORT)->Enable(enable);
    menuBar_->FindItem(MENU_REFRESH_WEBAPP)->Enable(enable && mmWebApp::isEnabled());

    menuBar_->FindItem(MENU_VIEW_HIDE_SHARE_ACCOUNTS)->Enable(enable);
    menuBar_->FindItem(MENU_VIEW_HIDE_DELETED_TRANSACTIONS)->Enable(enable);
    menuBar_->FindItem(MENU_VIEW_BUDGET_FINANCIAL_YEARS)->Enable(enable);
    menuBar_->FindItem(MENU_VIEW_BUDGET_TRANSFER_TOTAL)->Enable(enable);
    menuBar_->FindItem(MENU_VIEW_BUDGET_CATEGORY_SUMMARY)->Enable(enable);
    menuBar_->FindItem(MENU_VIEW_IGNORE_FUTURE_TRANSACTIONS)->Enable(enable);
    menuBar_->FindItem(MENU_VIEW_SHOW_TOOLTIPS)->Enable(enable);
    menuBar_->FindItem(MENU_VIEW_SHOW_MONEYTIPS)->Enable(enable);

    menuBar_->FindItem(MENU_DB_VACUUM)->Enable(enable);
    menuBar_->FindItem(MENU_DB_DEBUG)->Enable(enable);

    toolbarEnableItems(enable);
}

void mmFrame::toolbarEnableItems(bool enable)
{
    toolBar_->EnableTool(MENU_NEWACCT, enable);
    toolBar_->EnableTool(MENU_HOMEPAGE, enable);
    toolBar_->EnableTool(MENU_ORGPAYEE, enable);
    toolBar_->EnableTool(MENU_ORGCATEGS, enable);
    toolBar_->EnableTool(MENU_CURRENCY, enable);
    toolBar_->EnableTool(wxID_VIEW_LIST, enable);
    toolBar_->EnableTool(wxID_BROWSE, enable);
    toolBar_->EnableTool(MENU_TRANSACTIONREPORT, enable);
    toolBar_->EnableTool(wxID_PREFERENCES, enable);
    toolBar_->EnableTool(wxID_NEW, enable);
    toolBar_->EnableTool(wxID_PRINT, enable);
    toolBar_->EnableTool(MENU_ORGTAGS, enable);
    toolBar_->EnableTool(MENU_RATES, enable);

    toolBar_->EnableTool(MENU_BILLSDEPOSITS, enable);
    toolBar_->EnableTool(MENU_BUDGETSETUPDIALOG, enable);
    toolBar_->EnableTool(MENU_TRANSACTIONS_ALL, enable);
    toolBar_->EnableTool(MENU_TRANSACTIONS_DEL, enable);

    toolBar_->Refresh();
    toolBar_->Update();
}
//----------------------------------------------------------------------------

void mmFrame::menuPrintingEnable(bool enable)
{
    menuBar_->FindItem(wxID_PRINT)->Enable(enable);
    toolBar_->EnableTool(wxID_PRINT, enable);
    menuBar_->FindItem(MENU_EXPORT_HTML)->Enable(enable);
}
//----------------------------------------------------------------------------

void mmFrame::createControls()
{
    m_nav_tree_ctrl = new wxTreeCtrl(
        this, ID_NAVTREECTRL, wxDefaultPosition, wxDefaultSize,
        wxTR_SINGLE | wxTR_HAS_BUTTONS | wxTR_NO_LINES | wxTR_TWIST_BUTTONS | wxTR_HIDE_ROOT | wxTR_LINES_AT_ROOT
    );

    m_nav_tree_ctrl->SetMinSize(wxSize(100, 100));
    mmImage::themeMetaColour(m_nav_tree_ctrl, mmImage::COLOR_NAVPANEL);
    mmImage::themeMetaColour(m_nav_tree_ctrl, mmImage::COLOR_NAVPANEL_FONT, true);

    const auto navIconSize = PrefModel::instance().getNavigationIconSize();
    //m_nav_tree_ctrl->SetImages(mmImage::navtree_bitmapBundle_a(navIconSize));
    m_nav_tree_ctrl->SetImages(NavTreeIconImages::instance().getList(navIconSize));
    m_nav_tree_ctrl->SetIndent(10);

    m_nav_tree_ctrl->Connect(
        ID_NAVTREECTRL, wxEVT_TREE_SEL_CHANGED,
        wxTreeEventHandler(mmFrame::OnSelChanged),
        nullptr, this
    );
    m_nav_tree_ctrl->Bind(wxEVT_RIGHT_DOWN, &mmFrame::OnTreeRightClick, this);

    homePanel_ = new wxPanel(
        this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxTAB_TRAVERSAL | wxTR_SINGLE | wxNO_BORDER
    );

    m_mgr.AddPane(m_nav_tree_ctrl, wxAuiPaneInfo()
        .Name("Navigation")
        .BestSize(wxSize(200, 100)).MinSize(wxSize(100, 100))
        .Left()
    );

    m_mgr.AddPane(homePanel_, wxAuiPaneInfo()
        .Name("Home").Caption("Home")
        .CenterPane().PaneBorder(false)
    );
}
//----------------------------------------------------------------------------

wxTreeItemId mmFrame::addNavTreeSection(
    const wxTreeItemId& root, const wxString& sectionName, int sectionImg,
    int dataType, int64 dataId)
{
    wxTreeItemId section = m_nav_tree_ctrl->AppendItem(root, sectionName, sectionImg, sectionImg);
    m_nav_tree_ctrl->SetItemData(section, new mmTreeItemData(dataType, dataId, sectionName));
    m_nav_tree_ctrl->SetItemBold(section, true);
    return section;
}

wxTreeItemId mmFrame::addNavTreeItem(
    const wxTreeItemId& root, const wxString& itemName, int itemImg,
    int dataType, int64 dataId)
{
    wxTreeItemId item = m_nav_tree_ctrl->AppendItem(root, itemName, itemImg, itemImg);
    m_nav_tree_ctrl->SetItemData(item, new mmTreeItemData(dataType, dataId));
    return item;
}

void mmFrame::DoRecreateNavTreeControl(bool home_page)
{
    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);

    int acc_size = mmNavigatorList::instance().getNumberOfAccountTypes(); // Call first to guarantee proper init!
    std::vector<wxTreeItemId> accountSection(acc_size);

    if (home_page) {
        createHomePage();
    }

    DoWindowsFreezeThaw(m_nav_tree_ctrl);
    resetNavTreeControl();

    wxTreeItemId root = m_nav_tree_ctrl->AddRoot("Root");
    wxTreeItemId dashboard;
    wxTreeItemId favorites;
    wxTreeItemId budgeting;
    wxTreeItemId transactionFilter;
    wxTreeItemId reports;
    wxTreeItemId grm;
    wxTreeItemId trash;

    mmNavigatorItem* navinfo = mmNavigatorList::instance().getFirstActiveEntry();
    while (navinfo) {
        wxString tname = navinfo->navTyp < mmNavigatorItem::NAV_ENTRY_size ? wxGetTranslation(navinfo->name) : navinfo->name;
        switch (navinfo->type) {
            case mmNavigatorItem::NAV_ENTRY_DASHBOARD:
                dashboard = addNavTreeSection(root, tname, navinfo->imageId, mmTreeItemData::HOME_PAGE);
                break;

            case mmNavigatorItem::NAV_ENTRY_ALL_TRANSACTIONS:
                addNavTreeSection(root, tname, navinfo->imageId, mmTreeItemData::CHECKING);
                break;

            case mmNavigatorItem::NAV_ENTRY_SCHEDULED_TRANSACTIONS:
                addNavTreeSection(root, tname, navinfo->imageId, mmTreeItemData::BILLS);
                break;

            case mmNavigatorItem::NAV_ENTRY_FAVORITES:
                favorites = addNavTreeSection(root, tname, navinfo->imageId, mmTreeItemData::CHECKING, -3);
                break;

            case mmNavigatorItem::NAV_ENTRY_BUDGET_PLANNER:
                budgeting = m_nav_tree_ctrl->AppendItem(root, tname, navinfo->imageId, navinfo->imageId);
                m_nav_tree_ctrl->SetItemData(budgeting, new mmTreeItemData(mmTreeItemData::HELP_BUDGET, "Budget Setup"));
                m_nav_tree_ctrl->SetItemBold(budgeting, true);
                this->DoUpdateBudgetNavigation(budgeting);
                break;

            case mmNavigatorItem::NAV_ENTRY_TRANSACTION_REPORT:
                transactionFilter = addNavTreeSection(root, tname, navinfo->imageId, mmTreeItemData::FILTER);
                this->DoUpdateFilterNavigation(transactionFilter);
                break;

            case mmNavigatorItem::NAV_ENTRY_REPORTS:
                reports = addNavTreeSection(root, tname, navinfo->imageId, mmTreeItemData::HELP_REPORT);
                this->DoUpdateReportNavigation(reports);
                break;

            case mmNavigatorItem::NAV_ENTRY_GRM:
                grm = addNavTreeSection(
                    root, tname, navinfo->imageId, mmTreeItemData::HELP_PAGE_GRM);
                this->DoUpdateGRMNavigation(grm);
                break;

            case mmNavigatorItem::NAV_ENTRY_DELETED_TRANSACTIONS:
                trash = addNavTreeSection(root, tname, navinfo->imageId, mmTreeItemData::CHECKING, -2);
                break;

            case mmNavigatorItem::NAV_ENTRY_HELP:
                addNavTreeSection(root, tname, navinfo->imageId, mmTreeItemData::HELP_PAGE_MAIN);
                break;

            case mmNavigatorItem::TYPE_ID_CHECKING:
            case mmNavigatorItem::TYPE_ID_CREDIT_CARD:
            case mmNavigatorItem::TYPE_ID_CASH:
            case mmNavigatorItem::TYPE_ID_LOAN:
            case mmNavigatorItem::TYPE_ID_TERM:
                accountSection[navinfo->type] = addNavTreeSection(root, tname, navinfo->imageId,
                    mmTreeItemData::CHECKING, - (4 + navinfo->type));
                break;

            case mmNavigatorItem::TYPE_ID_INVESTMENT:
                accountSection[navinfo->type] = addNavTreeSection(root, tname, navinfo->imageId, mmTreeItemData::HELP_PAGE_STOCKS, -1);
                break;

            case mmNavigatorItem::TYPE_ID_ASSET:
                accountSection[navinfo->type] = addNavTreeSection(root, tname, navinfo->imageId, mmTreeItemData::ASSETS, -1);
                break;

            default:
                if (navinfo->type > mmNavigatorItem::NAV_ENTRY_HELP) { // Treat all custom accounts as checking accounts:
                    int id = (navinfo->type < acc_size + mmNavigatorItem::NAV_IDXDIFF)
                        ? navinfo->type - mmNavigatorItem::NAV_IDXDIFF
                        : 0;
                    accountSection[id] = addNavTreeSection(root, tname, navinfo->imageId,
                        mmTreeItemData::CHECKING, - (4 + id));
                }
                break;
        }
        // get next:
        navinfo = mmNavigatorList::instance().getNextActiveEntry(navinfo);
    }

    ///////////////////////////////////////////////////////////////////

    bool hideShareAccounts = PrefModel::instance().getHideShareAccounts();

    if (m_db) {
        /* Start Populating the dynamic data */
        m_temp_view = SettingModel::instance().getViewAccounts();
        wxASSERT(
            m_temp_view == VIEW_ACCOUNTS_ALL_STR ||
            m_temp_view == VIEW_ACCOUNTS_FAVORITES_STR ||
            m_temp_view == VIEW_ACCOUNTS_OPEN_STR ||
            m_temp_view == VIEW_ACCOUNTS_CLOSED_STR
        );

        int account_type;
        int accountImg;
        wxTreeItemId accountItem;
        wxTreeItemId sectionid;

        for (const auto& account_d : AccountModel::instance().find_data_a(
            TableClause::ORDERBY(AccountCol::NAME_ACCOUNTNAME)
        )) {
            if (
                (m_temp_view == VIEW_ACCOUNTS_OPEN_STR      && !account_d.is_open()) ||
                (m_temp_view == VIEW_ACCOUNTS_CLOSED_STR    && !account_d.is_closed()) ||
                (m_temp_view == VIEW_ACCOUNTS_FAVORITES_STR && !account_d.is_favorite())
            ) {
                continue;
            }
            account_type = mmNavigatorList::instance().getTypeIdFromDBName(account_d.m_type_);
            if (account_type == mmNavigatorItem::TYPE_ID_SHARES && hideShareAccounts)
                continue;

            accountImg = PrefModel::instance().AccountImageId(account_d.m_id, false);

            if (favorites &&
                account_type != mmNavigatorItem::TYPE_ID_INVESTMENT &&
                account_d.is_favorite() &&
                account_d.is_open()
            ) {
                accountItem = addNavTreeItem(favorites, account_d.m_name, accountImg, mmTreeItemData::CHECKING, account_d.m_id);
            }

            if (account_type > mmNavigatorItem::NAV_ENTRY_HELP){
                if (account_type < acc_size + mmNavigatorItem::NAV_IDXDIFF) {
                    account_type += -mmNavigatorItem::NAV_IDXDIFF;
                }
                else {
                    account_type = mmNavigatorItem::TYPE_ID_CHECKING; //Custom account does not exist anymore
                }
            }

            sectionid = accountSection[account_type];
            if (!sectionid) {
                continue;
            }

            switch (account_type) {
                case mmNavigatorItem::TYPE_ID_INVESTMENT: {
                    accountItem = addNavTreeItem(
                        accountSection[account_type],
                        account_d.m_name,
                        accountImg,
                        mmTreeItemData::INVESTMENT,
                        account_d.m_id
                    );

                    // Cash Ledger
                    if (PrefModel::instance().getShowNavigatorCashLedger()) {
                        wxTreeItemId stockItem = m_nav_tree_ctrl->AppendItem(
                            accountItem,
                            _t("Cash Ledger"),
                            accountImg, accountImg
                        );
                        m_nav_tree_ctrl->SetItemData(
                            stockItem,
                            new mmTreeItemData(mmTreeItemData::CHECKING, account_d.m_id)
                        );
                        // find all the accounts associated with this stock portfolio
                        // just to keep compatibility for legacy Shares account data
                        StockModel::DataA stocks = StockModel::instance().find_data_a(
                            StockCol::WHERE_HELDAT(OP_EQ, account_d.m_id)
                        );
                        std::sort(stocks.begin(), stocks.end(), StockData::SorterBySTOCKNAME());

                        // Put the names of the Stock_entry names as children of the stock account.
                        std::unordered_set<wxString> processedStockNames;
                        for (const auto& stock : stocks) {
                            if (!processedStockNames.insert(stock.m_name).second)
                                continue;
                            const AccountData* share_account = AccountModel::instance().get_name_data_n(stock.m_name);
                            if (!share_account)
                                continue;
                            stockItem = m_nav_tree_ctrl->AppendItem(accountItem, stock.m_name, accountImg, accountImg);
                            m_nav_tree_ctrl->SetItemData(stockItem, new mmTreeItemData(mmTreeItemData::CHECKING, share_account->m_id));
                        }
                    }
                    break;
                }

                case mmNavigatorItem::TYPE_ID_CHECKING:
                case mmNavigatorItem::TYPE_ID_CREDIT_CARD:
                case mmNavigatorItem::TYPE_ID_CASH:
                case mmNavigatorItem::TYPE_ID_LOAN:
                case mmNavigatorItem::TYPE_ID_TERM:
                case mmNavigatorItem::TYPE_ID_SHARES:
                case mmNavigatorItem::TYPE_ID_ASSET:
                    accountItem = addNavTreeItem(accountSection[account_type], account_d.m_name, accountImg, mmTreeItemData::CHECKING, account_d.m_id);
                    break;

                default:
                    if (account_type < acc_size) {
                        accountItem = addNavTreeItem(accountSection[account_type], account_d.m_name, accountImg, mmTreeItemData::CHECKING, account_d.m_id);
                    }
                    break;
            }
        }

        loadNavigationTreeItemsStatusFromJson();

        if (favorites && !m_nav_tree_ctrl->ItemHasChildren(favorites)) {
            m_nav_tree_ctrl->Delete(favorites);
        }

        for (int i = 0; i < acc_size; i++) {
            if (i != mmNavigatorItem::TYPE_ID_ASSET) {
                wxTreeItemId treeID = accountSection[i];
                if (treeID && (!m_nav_tree_ctrl->ItemHasChildren(treeID) ||
                    (i == mmNavigatorItem::TYPE_ID_SHARES && hideShareAccounts))) {
                    m_nav_tree_ctrl->Delete(accountSection[i]);
                }
            }
        }

        if (PrefModel::instance().getHideDeletedTransactions() ||
            TrxModel::instance().find_count(
                TrxModel::WHERE_IS_DELETED(true)
            ) == 0
        ) {
            if (trash) {
                m_nav_tree_ctrl->Delete(trash);
            }
            if (panelCurrent_ && panelCurrent_->GetId() == mmID_CHECKING) {
                JournalPanel* cp = wxDynamicCast(panelCurrent_, JournalPanel);
                if (cp->isDeletedTrans()) {
                    wxCommandEvent event(wxEVT_MENU, MENU_HOMEPAGE);
                    GetEventHandler()->AddPendingEvent(event);
                }
            }
        }
    }
    if (dashboard) {
        m_nav_tree_ctrl->EnsureVisible(dashboard);
        if (home_page) m_nav_tree_ctrl->SelectItem(dashboard);
    }
    m_nav_tree_ctrl->SetEvtHandlerEnabled(true);
    m_nav_tree_ctrl->Refresh();
    m_nav_tree_ctrl->Update();

    /* issue #4778 */
#if !defined(__WXMSW__)
    m_nav_tree_ctrl->SetFocus();
#endif

    DoWindowsFreezeThaw(m_nav_tree_ctrl);
}

void mmFrame::loadNavigationTreeItemsStatusFromJson()
{
    SetEvtHandlerEnabled(false);

    const wxString& json = InfoModel::instance().getString("NAV_TREE_STATUS", "");
    rapidjson::Document doc;
    doc.Parse(json.c_str());

    if (doc.IsArray()) {
        for (auto& v : doc.GetArray()) {
            if (!v.IsString()) continue;
            std::string path = v.GetString();
            wxTreeItemId item = FindItemByPath(m_nav_tree_ctrl, path);
            if (item.IsOk()) {
                m_nav_tree_ctrl->Expand(item);
            }
        }
    }

    SetEvtHandlerEnabled(true);
}

wxTreeItemId mmFrame::FindItemByPath(wxTreeCtrl* tree, const std::string& path)
{
    std::stringstream ss(path);
    std::string segment;

    wxTreeItemId current = tree->GetRootItem();

    if (!std::getline(ss, segment, '/')) return wxTreeItemId();
    if (tree->GetItemText(current).ToStdString() != segment)
        return wxTreeItemId();

    while (std::getline(ss, segment, '/')) {
        bool found = false;
        wxTreeItemIdValue cookie;
        wxTreeItemId child = tree->GetFirstChild(current, cookie);

        while (child.IsOk())
        {
            if (tree->GetItemText(child).ToStdString() == segment)
            {
                current = child;
                found = true;
                break;
            }
            child = tree->GetNextChild(current, cookie);
        }

        if (!found) return wxTreeItemId();
    }

    return current;
}

void mmFrame::OnTreeItemExpanded(wxTreeEvent& WXUNUSED(event))
{
    navTreeStateToJson();
}

void mmFrame::OnTreeItemCollapsed(wxTreeEvent& WXUNUSED(event))
{
    navTreeStateToJson();
}
//----------------------------------------------------------------------------

void mmFrame::OnDropFiles(wxDropFilesEvent& event)
{
    int id = panelCurrent_->GetId();
    if (id != mmID_CHECKING)
        return;
    JournalPanel* cp = wxDynamicCast(panelCurrent_, JournalPanel);
    if (!cp->isAllTrans() && !cp->isAccount())
        return;
    if (event.GetNumberOfFiles() <= 0)
        return;

    wxString* dropped = event.GetFiles();
    wxASSERT(dropped);
    wxArrayString files;
    for (int i = 0; i < event.GetNumberOfFiles(); i++) {
        wxString name = dropped[i];
        if (wxFileExists(name))
            files.push_back(name);
        else if (wxDirExists(name))
            wxDir::GetAllFiles(name, &files);
    }

    for (size_t i = 0; i < files.size(); i++) {
        wxString ext = wxFileName(files[i]).GetExt().MakeLower();
        if (ext == "csv" || ext == "xml") {
            mmUnivCSVDialog(
                this, (ext == "csv" ?
                    mmUnivCSVDialog::DIALOG_TYPE_IMPORT_CSV :
                    mmUnivCSVDialog::DIALOG_TYPE_IMPORT_XML
                ),
                gotoAccountID_, files[i]
            ).ShowModal();
        }
        else if (ext == "qif") {
            mmQIFImportDialog(this, gotoAccountID_, files[i]).ShowModal();
        }
    }
    refreshPanelData();
}

void mmFrame::navTreeStateToJson()
{
    rapidjson::Document doc;
    doc.SetArray();
    auto& alloc = doc.GetAllocator();

    wxTreeItemId root = m_nav_tree_ctrl->GetRootItem();
    collectNavTreeExpanded(m_nav_tree_ctrl, root, doc, alloc);

    rapidjson::StringBuffer nav_tree_status;
    rapidjson::Writer<rapidjson::StringBuffer> writer(nav_tree_status);
    doc.Accept(writer);
    InfoModel::instance().saveString("NAV_TREE_STATUS", nav_tree_status.GetString());
    //wxLogDebug("=========== navTreeStateToJson =============================");
    //wxLogDebug(nav_tree_status.GetString());
}

void mmFrame::collectNavTreeExpanded(wxTreeCtrl* tree, wxTreeItemId item, rapidjson::Value& array, rapidjson::Document::AllocatorType& alloc)
{
    if (!item.IsOk()) return;

    if (item != tree->GetRootItem() && tree->IsExpanded(item)) {
        std::string path = getNavTreeItemPath(tree, item);
        rapidjson::Value str;
        str.SetString(path.c_str(), alloc);
        array.PushBack(str, alloc);
    }

    wxTreeItemIdValue cookie;
    wxTreeItemId child = tree->GetFirstChild(item, cookie);

    while (child.IsOk()) {
        collectNavTreeExpanded(tree, child, array, alloc);
        child = tree->GetNextChild(item, cookie);
    }
}


std::string mmFrame::getNavTreeItemPath(wxTreeCtrl* tree, wxTreeItemId item)
{
    std::vector<std::string> parts;
    while (item.IsOk()) {
        parts.push_back(tree->GetItemText(item).ToStdString());
        item = tree->GetItemParent(item);
    }

    std::reverse(parts.begin(), parts.end());
    std::string path;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) path += "/";
        path += parts[i];
    }
    return path;
}

//----------------------------------------------------------------------------
void mmFrame::OnSelChanged(wxTreeEvent& event)
{
    if (m_db) {
        wxTreeItemId item = event.GetItem();
        if (item) {
            navTreeSelection(item);
        }
    }
}

void mmFrame::navTreeSelection(wxTreeItemId selectedItem)
{
    mmTreeItemData* iData = dynamic_cast<mmTreeItemData*>(m_nav_tree_ctrl->GetItemData(selectedItem));
    if (!iData)
        return;

    menuPrintingEnable(false);
    m_nav_tree_ctrl->Update();
    selectedItemData_ = iData;
    activeReport_ = false;
    gotoAccountID_ = -1;
    DragAcceptFiles(false);
    wxCommandEvent e;

    switch (iData->getType()) {
    case mmTreeItemData::DO_NOTHING:
        return;

    case mmTreeItemData::HOME_PAGE:
        return createHomePage();
    case mmTreeItemData::HELP_PAGE_MAIN:
        return createHelpPage();
    case mmTreeItemData::HELP_PAGE_STOCKS:
        return createStocksAccountPage(-1);
        //return createHelpPage(mmPath::HTML_INVESTMENT);
    case mmTreeItemData::HELP_PAGE_GRM:
        return createHelpPage(mmPath::HTML_CUSTOM_SQL);
    case mmTreeItemData::HELP_BUDGET:
        return createHelpPage(mmPath::HTML_BUDGET);
    case mmTreeItemData::HELP_REPORT:
        //createHelpPage(mmPath::HTML_REPORTS);
        return;

    case  mmTreeItemData::CHECKING: {
        int64 id = iData->getId();
        std::vector<int64> group_ids = {};
        if (id <= -3) { // isGroup
            wxTreeItemIdValue cookie;
            wxTreeItemId child = m_nav_tree_ctrl->GetFirstChild(selectedItem, cookie);
            while (child.IsOk()) {
                mmTreeItemData* data = dynamic_cast<mmTreeItemData*>(
                    m_nav_tree_ctrl->GetItemData(child)
                );
                if (data->getType() == mmTreeItemData::CHECKING) {
                    int64 dataId = data->getId();
                    if (dataId >= 1) // isAccount
                        group_ids.push_back(dataId);
                }
                child = m_nav_tree_ctrl->GetNextChild(selectedItem, cookie);
            }
        }
        if (id == -1 || id >= 1) // isAllTrans, isAccount
            DragAcceptFiles(true);
        if (id >= 1) // isAccount
            gotoAccountID_ = id;
        return createCheckingPage(id, group_ids);
    }

    case mmTreeItemData::INVESTMENT: {
        const AccountData* account = AccountModel::instance().get_idN_data_n(iData->getId());
        gotoAccountID_ = account->m_id;
        return createStocksAccountPage(gotoAccountID_);
    }
    case mmTreeItemData::ASSETS:
        return OnAssets(e);

    case mmTreeItemData::BILLS:
        return OnBillsDeposits(e);
    case mmTreeItemData::FILTER:
        return OnTransactionReport(e);
    case mmTreeItemData::FILTER_REPORT: {
        activeReport_ = true;
        wxSharedPtr<TrxFilterDialog> dlg(
            new TrxFilterDialog(this, iData->getString())
        );
        /// FIXME memory leak
        TrxReport* rs = new TrxReport(dlg);
        return createReportsPage(rs, true);
    }
    case mmTreeItemData::BUDGET:
        return createBudgetingPage(iData->getId());
    case mmTreeItemData::REPORT:
        activeReport_ = true;
        return createReportsPage(iData->getReport(), false);
    case mmTreeItemData::GRM:
        activeReport_ = true;
        return createReportsPage(iData->getReport(), false);
    }
    wxLogDebug("");
}
//----------------------------------------------------------------------------

void mmFrame::OnTreeRightClick(wxMouseEvent& event)
{
    wxPoint pos = event.GetPosition();
    int flags = 0;
    wxTreeItemId item = m_nav_tree_ctrl->HitTest(pos, flags);
    if (item.IsOk()) {
        m_nav_tree_ctrl->SelectItem(item);
        showTreePopupMenu(item, ScreenToClient(wxGetMousePosition()));
    }
    else {
        showEmptyTreePopupMenu(ScreenToClient(wxGetMousePosition()));
    }
}
//----------------------------------------------------------------------------

void mmFrame::OnLaunchAccountWebsite(wxCommandEvent& /*event*/)
{
    if (!selectedItemData_)
        return;

    int64 id = selectedItemData_->getId();
    const AccountData* acccount_n = AccountModel::instance().get_idN_data_n(id);
    if (!acccount_n)
        return;

    wxString website = acccount_n->m_website;
    if (!website.IsEmpty())
        wxLaunchDefaultBrowser(website);
}
//----------------------------------------------------------------------------

void mmFrame::OnAccountAttachments(wxCommandEvent& /*event*/)
{
    if (!selectedItemData_)
        return;

    int64 ref_id = selectedItemData_->getId();
    AttachmentDialog dlg(this, AccountModel::s_ref_type, ref_id);
    dlg.ShowModal();
}
//----------------------------------------------------------------------------

void mmFrame::OnReconcileAccount(wxCommandEvent& WXUNUSED(event))
{
    const AccountData* account = AccountModel::instance().get_idN_data_n(selectedItemData_->getId());
    if (!account)
        return;

    JournalPanel* cp = wxDynamicCast(panelCurrent_, JournalPanel);
    ReconcileDialog dlg(wxGetTopLevelParent(this), account, cp);
    if (dlg.ShowModal() == wxID_OK) {
        cp->refreshList();
    }
}

//----------------------------------------------------------------------------
void mmFrame::OnPopupEditAccount(wxCommandEvent& WXUNUSED(event))
{
    if (!selectedItemData_)
        return;

    int64 id = selectedItemData_->getId();
    AccountData* account_n = AccountModel::instance().unsafe_get_idN_data_n(id);
    if (account_n) {
        AccountDialog dlg(account_n, this);
        if (dlg.ShowModal() == wxID_OK) {
            RefreshNavigationTree();
            refreshPanelData();
        }
    }
}
//----------------------------------------------------------------------------

void mmFrame::OnPopupReallocateAccount(wxCommandEvent& WXUNUSED(event))
{
    if (!selectedItemData_)
        return;

    int64 id = selectedItemData_->getId();
    ReallocateAccount(id);
}
//----------------------------------------------------------------------------

void mmFrame::OnPopupDeleteFilter(wxCommandEvent& /*event*/)
{
    if (!m_db)
        return;

    wxString data = selectedItemData_->getString();
    Document j_doc;
    if (j_doc.Parse(data.utf8_str()).HasParseError())
        j_doc.Parse("{}");
    Value& j_label = GetValueByPointerWithDefault(j_doc, "/LABEL", "");
    wxString selected = j_label.IsString() ? wxString::FromUTF8(j_label.GetString()) : "";

    if (wxMessageBox(
        _t("The selected item will be deleted.") + "\n\n" + _t("Do you want to continue?"),
        _t("Settings item deletion"),
        wxYES_NO | wxICON_WARNING
    ) == wxNO)
        return;

    int i = InfoModel::instance().findArrayItem("TRANSACTIONS_FILTER", selected);
    if (i != wxNOT_FOUND) {
        InfoModel::instance().eraseArrayItem("TRANSACTIONS_FILTER", i);
        RefreshNavigationTree();
    }
}
//--------------------------------------------------------------------------

void mmFrame::OnPopupRenameFilter(wxCommandEvent& /*event*/)
{
    if (!m_db)
        return;

    wxString data = selectedItemData_->getString();
    Document j_doc;
    if (j_doc.Parse(data.utf8_str()).HasParseError())
        j_doc.Parse("{}");
    Value& j_label = GetValueByPointerWithDefault(j_doc, "/LABEL", "");
    wxString selected = j_label.IsString() ? wxString::FromUTF8(j_label.GetString()) : "";

    wxString new_name;
    bool nameOK = false;
    while (!nameOK) {
        new_name = wxGetTextFromUser(_t("Setting Name"), _t("Please Enter"), selected);
        if (new_name.empty())
            return;
        if (wxNOT_FOUND == InfoModel::instance().findArrayItem("TRANSACTIONS_FILTER", new_name))
            nameOK = true;
        else {
            wxString msgStr = wxString() << _t("A setting with this name already exists") << "\n"
                << "\n"
                << _t("Please specify a new name for the setting") << "\n";
            wxMessageBox(msgStr, _t("Name in use"), wxICON_ERROR);
        }
    }

    int i = InfoModel::instance().findArrayItem("TRANSACTIONS_FILTER", selected);
    if (i != wxNOT_FOUND) {
        InfoModel::instance().eraseArrayItem("TRANSACTIONS_FILTER", i);

        // Change the name
        Value::MemberIterator v_name = j_doc.FindMember("LABEL");
        v_name->value.SetString(new_name.utf8_str(), j_doc.GetAllocator());
        // Serialize the new entry
        StringBuffer buffer;
        buffer.Clear();
        PrettyWriter<StringBuffer> writer(buffer);
        writer.SetFormatOptions(kFormatSingleLineArray);
        j_doc.Accept(writer);
        data = wxString::FromUTF8(buffer.GetString());
        InfoModel::instance().prependArrayItem("TRANSACTIONS_FILTER", data, -1);

        DoRecreateNavTreeControl();
        setNavTreeSection(_t("Transaction Report"));
    }
}
//--------------------------------------------------------------------------

void mmFrame::OnPopupEditFilter(wxCommandEvent& /*event*/)
{
    if (!m_db)
        return;
    if (AccountModel::instance().find_count() == 0)
        return;

    wxString data = selectedItemData_->getString();

    const auto filter_settings = InfoModel::instance().getArrayString("TRANSACTIONS_FILTER");

    wxSharedPtr<TrxFilterDialog> dlg(new TrxFilterDialog(this, -1, true, data));
    bool is_ok = (dlg->ShowModal() == wxID_OK);
    if (filter_settings != InfoModel::instance().getArrayString("TRANSACTIONS_FILTER")) {
        DoRecreateNavTreeControl();
        setNavTreeSection(_t("Transaction Report"));
    }

    if (is_ok) {
        TrxReport* rs = new TrxReport(dlg);
        createReportsPage(rs, true);
    }
}
//----------------------------------------------------------------------------

void mmFrame::OnPopupDeleteAccount(wxCommandEvent& /*event*/)
{
    if (!selectedItemData_)
        return;

    int64 id = selectedItemData_->getId();
    const AccountData* account_n = AccountModel::instance().get_idN_data_n(id);
    if (!account_n)
        return;

    wxString warning_msg = _t("Do you want to delete the account?");
    if (mmNavigatorList::instance().isInvestmentAccount(account_n->m_type_) ||
        mmNavigatorList::instance().isShareAccount(account_n->m_type_)
    ) {
        warning_msg += "\n\n" + _t("This will also delete any associated Shares.");
    }
    wxMessageDialog msgDlg(this,
        warning_msg,
        _t("Confirm Account Deletion"),
        wxYES_NO | wxNO_DEFAULT | wxICON_ERROR
    );
    if (msgDlg.ShowModal() != wxID_YES)
        return;

    AccountModel::instance().purge_id(account_n->m_id);
    DoRecreateNavTreeControl(true);
}
//----------------------------------------------------------------------------

void mmFrame::AppendImportMenu(wxMenu& menu)
{
    wxMenu* importFrom(new wxMenu);
    menu.AppendSubMenu(importFrom, _t("&Import from"));
    importFrom->Append(MENU_TREEPOPUP_ACCOUNT_IMPORTUNIVCSV, _tu("&CSV Files…"));
    importFrom->Append(MENU_TREEPOPUP_ACCOUNT_IMPORTXML, _tu("&XML Files…"), _t("Import from XML file (Excel format)"));
    importFrom->AppendSeparator();
    importFrom->Append(MENU_TREEPOPUP_ACCOUNT_IMPORTQIF, _tu("&QIF Files…"));

    wxMenu* exportTo(new wxMenu);
    menu.AppendSubMenu(exportTo, _t("&Export as"));
    exportTo->Append(MENU_TREEPOPUP_ACCOUNT_EXPORT2CSV, _tu("&CSV File…"));
    exportTo->Append(MENU_TREEPOPUP_ACCOUNT_EXPORT2XML, _tu("&XML File…"));
    exportTo->AppendSeparator();
    exportTo->Append(MENU_TREEPOPUP_ACCOUNT_EXPORT2MMEX, _tu("&MMEX CSV File…"));
    exportTo->Append(MENU_TREEPOPUP_ACCOUNT_EXPORT2JSON, _tu("&JSON File…"));
    exportTo->Append(MENU_TREEPOPUP_ACCOUNT_EXPORT2QIF, _tu("&QIF File…"));
}

void mmFrame::showTreePopupMenu(const wxTreeItemId& id, const wxPoint& pt)
{
    mmTreeItemData* iData = dynamic_cast<mmTreeItemData*>(m_nav_tree_ctrl->GetItemData(id));
    if (!iData)
        return;

    int64 acct_id = 0;
    selectedItemData_ = iData;
    int itemType = iData->getType();
    wxCommandEvent e;
    wxMenu menu;

    switch (itemType) {
    case mmTreeItemData::HOME_PAGE:
        return showEmptyTreePopupMenu(pt);
    case mmTreeItemData::HELP_BUDGET:
    case mmTreeItemData::BUDGET:
        return OnBudgetSetupDialog(e);
    case mmTreeItemData::FILTER:
        return OnTransactionReport(e);
    case mmTreeItemData::FILTER_REPORT: {
        const wxString data = iData->getString();
        wxLogDebug("MENU FILTER: %s", data);
        menu.Append(MENU_TREEPOPUP_FILTER_EDIT, _tu("&Customize Report…"));
        menu.Append(MENU_TREEPOPUP_FILTER_RENAME, _tu("&Rename Report…"));
        menu.Append(MENU_TREEPOPUP_FILTER_DELETE, _tu("&Delete Report…"));
        PopupMenu(&menu, pt);
        break;
    }
    case  mmTreeItemData::GRM:
    case  mmTreeItemData::HELP_PAGE_GRM:
        return OnGeneralReportManager(e);
    case mmTreeItemData::HELP_REPORT:
        return mmDoHideReportsDialog();
    case mmTreeItemData::HELP_PAGE_STOCKS:
        acct_id = -1;
        break;
    case mmTreeItemData::INVESTMENT: {
        acct_id = iData->getId();
        const AccountData* account_n = AccountModel::instance().get_idN_data_n(acct_id);
        if (account_n) {
            menu.Append(MENU_TREEPOPUP_RECONCILE, _t("&Reconcile Account"));
            menu.AppendSeparator();
            menu.Append(MENU_TREEPOPUP_LAUNCHWEBSITE, _t("&Launch Account Website"));
            menu.Append(MENU_TREEPOPUP_ACCOUNTATTACHMENTS, _tu("&Attachment Manager…"));
            menu.AppendSeparator();
            menu.Append(MENU_TREEPOPUP_EDIT, _tu("&Edit Account…"));
            menu.Append(MENU_TREEPOPUP_DELETE, _tu("&Delete Account…"));

            menu.Enable(MENU_TREEPOPUP_LAUNCHWEBSITE, !account_n->m_website.IsEmpty());
            PopupMenu(&menu, pt);
        }
        break;
    }
    case mmTreeItemData::CHECKING: {
        acct_id = iData->getId();
        if (acct_id >= 1) { // isAccount
            const AccountData* account_n = AccountModel::instance().get_idN_data_n(acct_id);
            if (!account_n)
                break;
            menu.Append(MENU_TREEPOPUP_RECONCILE, _t("&Reconcile Account"));;
            menu.AppendSeparator();
            menu.Append(MENU_TREEPOPUP_LAUNCHWEBSITE, _t("&Launch Account Website"));
            menu.Append(MENU_TREEPOPUP_ACCOUNTATTACHMENTS,_tu("&Attachment Manager…"));
            menu.AppendSeparator();
            AppendImportMenu(menu);
            menu.AppendSeparator();
            menu.Append(MENU_TREEPOPUP_EDIT, _tu("&Edit Account…"));
            menu.Append(MENU_TREEPOPUP_REALLOCATE, _tu("&Change Account Type…"));
            menu.AppendSeparator();
            menu.Append(MENU_TREEPOPUP_DELETE, _tu("&Delete Account…"));

            menu.Enable(MENU_TREEPOPUP_LAUNCHWEBSITE, !account_n->m_website.IsEmpty());
            menu.Enable(MENU_TREEPOPUP_REALLOCATE,
                !mmNavigatorList::instance().isShareAccount(account_n->m_type_) &&
                !mmNavigatorList::instance().isInvestmentAccount(account_n->m_type_) &&
                !mmNavigatorList::instance().isAssetAccount(account_n->m_type_)
            );
            PopupMenu(&menu, pt);
        }
        break;
    } }
    // isAllTrans/Stock Portfolios, isGroup
    if (acct_id == -1 || acct_id <= -3) {
        menu.Append(
            MENU_TREEPOPUP_ACCOUNT_NEW,
            _tu("&New Account…")
        );
        menu.Append(
            MENU_TREEPOPUP_ACCOUNT_EDIT,
            _tu("&Edit Account…")
        );
        menu.Append(MENU_TREEPOPUP_ACCOUNT_LIST, _t("Account &List"));
        menu.AppendSeparator();
        menu.Append(
            MENU_TREEPOPUP_ACCOUNT_DELETE,
            _tu("&Delete Account…")
        );
        menu.AppendSeparator();

        AppendImportMenu(menu);

        menu.AppendSeparator();
        wxMenu* viewAccounts(new wxMenu);
        viewAccounts->AppendRadioItem(
            MENU_TREEPOPUP_ACCOUNT_VIEWALL,
            _t("&All")
        )->Check(m_temp_view == VIEW_ACCOUNTS_ALL_STR);
        viewAccounts->AppendRadioItem(
            MENU_TREEPOPUP_ACCOUNT_VIEWFAVORITE,
            _t("&Favorites")
        )->Check(m_temp_view == VIEW_ACCOUNTS_FAVORITES_STR);
        viewAccounts->AppendRadioItem(
            MENU_TREEPOPUP_ACCOUNT_VIEWOPEN,
            _t("&Open")
        )->Check(m_temp_view == VIEW_ACCOUNTS_OPEN_STR);
        viewAccounts->AppendRadioItem(
            MENU_TREEPOPUP_ACCOUNT_VIEWCLOSED,
            _t("&Closed")
        )->Check(m_temp_view == VIEW_ACCOUNTS_CLOSED_STR);
        menu.AppendSubMenu(viewAccounts, _t("Accounts &Visible"));

        menu.AppendSeparator();
        menu.Append(MENU_TREEPOPUP_COLLAPSE_ALL, _tu("Co&llapse All"));

        PopupMenu(&menu, pt);
    }
}
//----------------------------------------------------------------------------

void mmFrame::showEmptyTreePopupMenu(const wxPoint& pt)
{
    wxMenu menu;
    menu.Append(MENU_TREEPOPUP_EXPAND_ALL, _tu("&Expand All"));
    menu.Append(MENU_TREEPOPUP_COLLAPSE_ALL, _tu("&Collapse All"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_CONFIG_NAV, _tu("&Navigator configuration…"));
    menu.Append(MENU_TREEPOPUP_THEME, _tu("T&heme Manager…"));
    PopupMenu(&menu, pt);
}

//----------------------------------------------------------------------------

void mmFrame::OnViewAccountsTemporaryChange(wxCommandEvent& e)
{
    int evt_id = e.GetId();
    //Get current settings for view accounts
    const wxString vAccts = SettingModel::instance().getViewAccounts();
    if (m_temp_view.empty())
        m_temp_view = vAccts;

    //Set view ALL & Refresh Navigation Panel
    switch (evt_id) {
    case MENU_TREEPOPUP_ACCOUNT_VIEWALL:
        m_temp_view = VIEW_ACCOUNTS_ALL_STR;
        break;
    case MENU_TREEPOPUP_ACCOUNT_VIEWFAVORITE:
        m_temp_view = VIEW_ACCOUNTS_FAVORITES_STR;
        break;
    case MENU_TREEPOPUP_ACCOUNT_VIEWOPEN:
        m_temp_view = VIEW_ACCOUNTS_OPEN_STR;
        break;
    case MENU_TREEPOPUP_ACCOUNT_VIEWCLOSED:
        m_temp_view = VIEW_ACCOUNTS_CLOSED_STR;
        break;
    }
    SettingModel::instance().saveViewAccounts(m_temp_view);
    RefreshNavigationTree();

    //Restore settings
    SettingModel::instance().saveViewAccounts(vAccts);
}

//----------------------------------------------------------------------------

void mmFrame::createMenu()
{
    wxMenu* menu_file = new wxMenu;

    wxMenuItem* menuItemNew = new wxMenuItem(menu_file, MENU_NEW, _tu("&New Database…"), _t("New Database"));
    wxMenuItem* menuItemOpen = new wxMenuItem(menu_file, MENU_OPEN, _tu("&Open Database…") + "\tCtrl-O", _t("Open Database"));
    wxMenuItem* menuItemSaveAs = new wxMenuItem(menu_file, MENU_SAVE_AS, _tu("Save Database &As…"), _t("Save Database As"));
    menu_file->Append(menuItemNew);
    menu_file->Append(menuItemOpen);
    menu_file->Append(menuItemSaveAs);
    menu_file->AppendSeparator();

    m_menuRecentFiles = new wxMenu;
    menu_file->Append(MENU_RECENT_FILES, _t("&Recent Files"), m_menuRecentFiles);
    wxMenuItem* menuClearRecentFiles = new wxMenuItem(menu_file, MENU_RECENT_FILES_CLEAR, _t("&Clear Recent Files"));
    menu_file->Append(menuClearRecentFiles);
    menu_file->AppendSeparator();

    wxMenu* importMenu = new wxMenu;
    menu_file->Append(MENU_IMPORT, _t("&Import from"), importMenu);
    importMenu->Append(MENU_IMPORT_UNIVCSV, _tu("&CSV File…"), _t("Import from CSV file"));
    importMenu->Append(MENU_IMPORT_XML, _tu("&XML File…"), _t("Import from XML file (Excel format)"));
    importMenu->AppendSeparator();
    importMenu->Append(MENU_IMPORT_QIF, _tu("&QIF File…"), _t("Import from QIF file"));
    importMenu->AppendSeparator();
    importMenu->Append(MENU_IMPORT_OFX, _tu("&OFX File…"), _t("Import from OFX file"));
    importMenu->AppendSeparator();
    importMenu->Append(MENU_IMPORT_WEBAPP, _tu("&WebApp…"), _t("Import from the WebApp"));

    wxMenu* exportMenu = new wxMenu;
    menu_file->Append(MENU_EXPORT, _t("&Export as"), exportMenu);
    exportMenu->Append(MENU_EXPORT_CSV, _tu("&CSV File…"), _t("Export as CSV file"));
    exportMenu->Append(MENU_EXPORT_XML, _tu("&XML File…"), _t("Export as XML file"));
    exportMenu->AppendSeparator();
    exportMenu->Append(MENU_EXPORT_MMEX, _tu("&MMEX CSV File…"), _t("Export as fixed CSV file"));
    exportMenu->Append(MENU_EXPORT_JSON, _tu("&JSON File…"), _t("Export as JSON file"));
    exportMenu->Append(MENU_EXPORT_QIF, _tu("&QIF File…"), _t("Export as QIF file"));
    exportMenu->AppendSeparator();
    exportMenu->Append(MENU_EXPORT_HTML, _tu("&HTML File…"), _t("Export as HTML file"));

    menu_file->AppendSeparator();

    wxMenuItem* menuItemPrint = new wxMenuItem(menu_file, wxID_PRINT,
        _tu("&Print…"), _t("Print current view"));
    menu_file->Append(menuItemPrint);

    menu_file->AppendSeparator();

    wxMenuItem* menuItemQuit = new wxMenuItem(menu_file, wxID_EXIT,
        _t("E&xit") + "\tCtrl-Q", _t("Quit this program"));
    menu_file->Append(menuItemQuit);

    // Create the required menu items
    wxMenu* menuView = new wxMenu;
    wxMenuItem* menuItemToolbar = new wxMenuItem(
        menuView,
        MENU_VIEW_TOOLBAR,
        _t("&Toolbar"),
        _t("Show/Hide the toolbar"),
        wxITEM_CHECK
    );
    wxMenuItem* menuItemLinks = new wxMenuItem(
        menuView,
        MENU_VIEW_LINKS,
        _t("&Navigator") + "\tF6",
        _t("Show/Hide Navigator"),
        wxITEM_CHECK
    );
    wxMenuItem* menuItemHideShareAccounts = new wxMenuItem(
        menuView,
        MENU_VIEW_HIDE_SHARE_ACCOUNTS,
        _t("&Display Share Accounts"),
        _t("Show/Hide Share Accounts in Navigator"),
        wxITEM_CHECK
    );
    wxMenuItem* menuItemHideDeletedTransactions = new wxMenuItem(
        menuView,
        MENU_VIEW_HIDE_DELETED_TRANSACTIONS,
        _t("&Display Deleted Transactions"),
        _t("Show/Hide Deleted Transactions in Navigator"),
        wxITEM_CHECK
    );
    wxMenuItem* menuItemBudgetFinancialYears = new wxMenuItem(
        menuView,
        MENU_VIEW_BUDGET_FINANCIAL_YEARS,
        _t("Budgets: As Financial &Years"),
        _t("Display Budgets in Financial Year Format"),
        wxITEM_CHECK
    );
    wxMenuItem* menuItemBudgetTransferTotal = new wxMenuItem(
        menuView,
        MENU_VIEW_BUDGET_TRANSFER_TOTAL,
        _t("Budgets: &Include Transfers in Totals"),
        _t("Include the transfer transactions in the Budget Totals"),
        wxITEM_CHECK
    );
    wxMenuItem* menuItemBudgetCategorySummary = new wxMenuItem(
        menuView,
        MENU_VIEW_BUDGET_CATEGORY_SUMMARY,
        _t("Budget Category Report: with &Summaries"),
        _t("Include the category summaries in the Budget Category Summary"),
        wxITEM_CHECK
    );
    wxMenuItem* menuItemIgnoreFutureTransactions = new wxMenuItem(
        menuView,
        MENU_VIEW_IGNORE_FUTURE_TRANSACTIONS,
        _t("Ignore F&uture Transactions"),
        _t("Ignore Future Transactions"),
        wxITEM_CHECK
    );
    wxMenuItem* menuItemShowToolTips = new wxMenuItem(
        menuView,
        MENU_VIEW_SHOW_TOOLTIPS,
        _t("Show T&ooltips"),
        _t("Show Tooltips"),
        wxITEM_CHECK
    );
    wxMenuItem* menuItemShowMoneyTips = new wxMenuItem(
        menuView,
        MENU_VIEW_SHOW_MONEYTIPS,
        _t("Show &Money Tips"),
        _t("Show Money Tips"),
        wxITEM_CHECK
    );

    //Add the menu items to the menu bar
    menuView->Append(menuItemToolbar);
    menuView->Append(menuItemLinks);
    menuView->Append(menuItemHideShareAccounts);
    menuView->Append(menuItemHideDeletedTransactions);
    menuView->Append(menuItemBudgetFinancialYears);
    menuView->Append(menuItemBudgetTransferTotal);
    menuView->AppendSeparator();
    menuView->Append(menuItemBudgetCategorySummary);
    menuView->AppendSeparator();
    menuView->Append(menuItemIgnoreFutureTransactions);
    menuView->AppendSeparator();
    menuView->Append(menuItemShowToolTips);
    menuView->Append(menuItemShowMoneyTips);
    menuView->AppendSeparator();

#if (wxMAJOR_VERSION >= 3 && wxMINOR_VERSION >= 0)
    wxMenuItem* menuItemToggleFullscreen = new wxMenuItem(
        menuView,
        MENU_VIEW_TOGGLE_FULLSCREEN,
        _t("&Full Screen") + "\tF11",
        _t("Toggle full screen"),
        wxITEM_CHECK
    );
    menuView->Append(menuItemToggleFullscreen);
#endif

    wxMenuItem* menuItemResetView = new wxMenuItem(
        menuView,
        MENU_VIEW_RESET,
        _t("&Reset View"),
        _t("Reset view and dock tools")
    );
    menuView->Append(menuItemResetView);

    menuView->AppendSeparator();
    wxMenuItem* menuItemLanguage = new wxMenuItem(
        menuView,
        MENU_LANG,
        _t("User Interface &Language"),
        _t("Change user interface language")
    );
    wxMenu* menuLang = new wxMenu;
    for (auto const& lang : g_translations()) {
        int            lang_id    = std::get<0>(lang);
        const wxString lang_label = std::get<1>(lang);
        const wxString lang_help  = std::get<2>(lang);
        menuLang->AppendRadioItem(
            MENU_LANG + 1 + lang_id, lang_label, lang_help
        )->Check(
            lang_id == m_app->getGUILanguage()
        );
    }
    menuItemLanguage->SetSubMenu(menuLang);
    menuView->Append(menuItemLanguage);

    wxMenu* menuAccounts = new wxMenu;

    wxMenuItem* menuItemAcctList = new wxMenuItem(
        menuAccounts,
        MENU_HOMEPAGE,
        _t("Dash&board"),
        _t("Open Dashboard")
    );

    wxMenuItem* menuItemNewAcct = new wxMenuItem(
        menuAccounts,
        MENU_NEWACCT,
        _tu("&New Account…"),
        _t("New Account")
    );

    wxMenuItem* menuItemAcctEdit = new wxMenuItem(
        menuAccounts,
        MENU_ACCTEDIT,
        _tu("&Edit Account…"),
        _t("Edit Account")
    );

    wxMenuItem* menuItemReallocateAcct = new wxMenuItem(
        menuAccounts,
        MENU_ACCOUNT_REALLOCATE,
        _tu("&Change Account Type…"),
        _t("Change the account type of an account")
    );

    wxMenuItem* menuItemAcctDelete = new wxMenuItem(
        menuAccounts,
        MENU_ACCTDELETE,
        _tu("&Delete Account…"),
        _t("Delete Account from database")
    );

    menuAccounts->Append(menuItemAcctList);
    menuAccounts->AppendSeparator();
    menuAccounts->Append(menuItemNewAcct);
    menuAccounts->Append(menuItemAcctEdit);
    menuAccounts->Append(menuItemReallocateAcct);
    menuAccounts->Append(menuItemAcctDelete);

    // Tools Menu
    wxMenu* menuTools = new wxMenu;

    wxMenuItem* menuItemRates = new wxMenuItem(menuTools
        , MENU_RATES, _t("Do&wnload Rates"), _t("Download Currency and Stock Rates"));
    menuTools->Append(menuItemRates);

    menuTools->AppendSeparator();

    wxMenuItem* menuItemPayee = new wxMenuItem(menuTools
        , MENU_ORGPAYEE, _tu("&Payee Manager…"), _t("Payee Manager"));
    menuTools->Append(menuItemPayee);

    wxMenuItem* menuItemCateg = new wxMenuItem(menuTools
        , MENU_ORGCATEGS, _tu("&Category Manager…"), _t("Category Manager"));
    menuTools->Append(menuItemCateg);

    wxMenuItem* menuItemTags = new wxMenuItem(menuTools
        , MENU_ORGTAGS, _tu("&Tag Manager…"), _t("Tag Manager"));
    menuTools->Append(menuItemTags);

    wxMenuItem* menuItemCurrency = new wxMenuItem(menuTools, MENU_CURRENCY
        , _tu("Curre&ncy Manager…"), _t("Currency Manager"));
    menuTools->Append(menuItemCurrency);

    wxMenuItem* menuItemCategoryRelocation = new wxMenuItem(menuTools
        , MENU_CATEGORY_RELOCATION, _tu("&Categories…")
        , _t("Merge categories"));
    wxMenuItem* menuItemPayeeRelocation = new wxMenuItem(menuTools
        , MENU_PAYEE_RELOCATION, _tu("&Payees…")
        , _t("Merge payees"));
    wxMenuItem* menuItemTagRelocation = new wxMenuItem(menuTools
        , MENU_TAG_RELOCATION, _tu("&Tags…")
        , _t("Merge tags"));
    wxMenuItem* menuItemRelocation = new wxMenuItem(menuTools
        , MENU_RELOCATION, _t("&Merge")
        , _t("Merge categories, payees, and tags"));
    wxMenu* menuRelocation = new wxMenu;
    menuRelocation->Append(menuItemPayeeRelocation);
    menuRelocation->Append(menuItemCategoryRelocation);
    menuRelocation->Append(menuItemTagRelocation);
    menuItemRelocation->SetSubMenu(menuRelocation);
    menuTools->Append(menuItemRelocation);

    menuTools->AppendSeparator();

    wxMenuItem* menuItemBudgeting = new wxMenuItem(menuTools, MENU_BUDGETSETUPDIALOG
        , _tu("&Budget Planner…"), _t("Budget Planner"));
    menuTools->Append(menuItemBudgeting);

    wxMenuItem* menuItemBillsDeposits = new wxMenuItem(menuTools, MENU_BILLSDEPOSITS
        , _t("&Scheduled Transactions"), _t("Bills and Deposits"));
    menuTools->Append(menuItemBillsDeposits);

    wxMenuItem* menuItemAssets = new wxMenuItem(menuTools, MENU_ASSETS
        , _t("&Assets"), _t("Assets"));
    menuTools->Append(menuItemAssets);

    menuTools->AppendSeparator();

    wxMenuItem* menuItemThemes = new wxMenuItem(menuTools, MENU_THEME_MANAGER
        , _tu("T&heme Manager…"), _t("Theme Manager"));
    menuTools->Append(menuItemThemes);

    menuTools->Append(new wxMenuItem(menuTools, MENU_DATE_RANGE_MANAGER, _tu("Date Range Manager…"), _t("Date Range Manager")));

    menuTools->AppendSeparator();

    wxMenuItem* menuItemTransactions = new wxMenuItem(menuTools, MENU_TRANSACTIONREPORT
        , _tu("Tra&nsaction Report…"), _t("Transaction Report"));
    menuTools->Append(menuItemTransactions);

    menuTools->AppendSeparator();

    wxMenuItem* menuItemGRM = new wxMenuItem(menuTools, wxID_VIEW_LIST
        , _tu("&General Report Manager…"), _t("General Report Manager"));
    menuTools->Append(menuItemGRM);

    wxMenuItem* menuItemCF = new wxMenuItem(menuTools, wxID_BROWSE
        , _tu("C&ustom Field Manager…"), _t("Custom Field Manager"));
    menuTools->Append(menuItemCF);

    menuTools->AppendSeparator();

    wxMenuItem* menuItemWA = new wxMenuItem(menuTools, MENU_REFRESH_WEBAPP
        , _tu("Refresh &WebApp…"), _t("Refresh WebApp"));
    menuTools->Append(menuItemWA);
    menuTools->AppendSeparator();

    wxMenuItem* menuItemOptions = new wxMenuItem(menuTools, wxID_PREFERENCES
        , _tu("&Settings…") + "\tAlt-F12", _t("Settings"));
    menuTools->Append(menuItemOptions);

    menuTools->AppendSeparator();

    wxMenu* menuDatabase = new wxMenu;
    wxMenuItem* menuItemConvertDB = new wxMenuItem(menuTools, MENU_CONVERT_ENC_DB
        , _tu("&Decrypt Encrypted Database…")
        , _t("Convert encrypted database to unencrypted database"));
    wxMenuItem* menuItemChangeEncryptPassword = new wxMenuItem(menuTools, MENU_CHANGE_ENCRYPT_PASSWORD
        , _tu("Change Encrypted &Password…")
        , _t("Change the password of an encrypted database"));
    wxMenuItem* menuItemVacuumDB = new wxMenuItem(menuTools, MENU_DB_VACUUM
        , _tu("&Optimize Database…")
        , _t("Optimize database for space and performance"));
    wxMenuItem* menuItemCheckDB = new wxMenuItem(menuTools, MENU_DB_DEBUG
        , _tu("Database Check and De&bug…")
        , _t("Generate database report or fix errors"));
    wxMenuItem* menuItemResetCookies = new wxMenuItem(menuTools, MENU_DB_COOKIE_RESET
        , _tu("Reset Web API Cookies…")
        , _t("Used to reset stored cookies for web API access"));
    menuDatabase->Append(menuItemConvertDB);
    menuDatabase->Append(menuItemChangeEncryptPassword);
    menuDatabase->Append(menuItemVacuumDB);
    menuDatabase->Append(menuItemCheckDB);
    menuDatabase->Append(menuItemResetCookies);
    menuTools->AppendSubMenu(menuDatabase, _t("&Database")
        , _t("Database management"));
    menuItemChangeEncryptPassword->Enable(false);

    // Help Menu
    wxMenu* menuHelp = new wxMenu;

    wxMenuItem* menuItemHelp = new wxMenuItem(menuTools, wxID_HELP,
        _t("&Help") + "\tF1", _t("Show Help"));
    menuHelp->Append(menuItemHelp);

    //Community Submenu
    wxMenuItem* menuItemWebsite = new wxMenuItem(menuHelp, MENU_WEBSITE
        , _t("&Website")
        , _t("Visit MMEX website for the latest news and updates"));
    wxMenuItem* menuItemFacebook = new wxMenuItem(menuHelp, MENU_FACEBOOK
        , _t("Face&book"), _t("Visit MMEX Facebook page"));
    wxMenuItem* menuItemCrowdin = new wxMenuItem(menuHelp, MENU_CROWDIN
        , _t("&Crowdin"), _t("Help translate MMEX on Crowdin"));
    wxMenuItem* menuItemTwitter = new wxMenuItem(menuHelp, MENU_TWITTER
        , _t("&X"), _t("Follow MMEX on X"));
    wxMenuItem* menuItemYouTube = new wxMenuItem(menuHelp, MENU_YOUTUBE
        , _t("&YouTube"), _t("Watch MMEX videos on YouTube"));
    wxMenuItem* menuItemSlack = new wxMenuItem(menuHelp, MENU_SLACK
        , _t("&Slack"), _t("Communicate with the MMEX team online"));
    wxMenuItem* menuItemGitHub = new wxMenuItem(menuHelp, MENU_GITHUB
        , _t("&GitHub"), _t("Access open source code repository and track reported bug statuses"));
    wxMenuItem* menuItemWiki = new wxMenuItem(menuHelp, MENU_WIKI
        , _t("Wi&ki"), _t("Read and update MMEX wiki"));
    wxMenuItem* menuItemReportIssues = new wxMenuItem(menuHelp, MENU_REPORTISSUES
        , _t("&Forum")
        , _t("Visit MMEX forum to read and post comments and for support"));
    wxMenuItem* menuItemAppleAppStore = new wxMenuItem(menuHelp, MENU_APPLE_APPSTORE
        , _t("MMEX for &iOS")
        , _t("Install MMEX on iPads and iPhones"));
    wxMenuItem* menuItemGooglePlay = new wxMenuItem(menuHelp, MENU_GOOGLEPLAY
        , _t("MMEX for &Android")
        , _t("Install MMEX for Android based smartphones and tablets"));
    wxMenuItem* menuItemNotify = new wxMenuItem(menuHelp, MENU_ANNOUNCEMENTMAILING
        , _t("&Newsletter")
        , _t("Visit the MMEX blog to view announcements"));
    wxMenuItem* menuItemRSS = new wxMenuItem(menuHelp, MENU_RSS
        , _t("&RSS Feed"), _t("Connect RSS web feed to news aggregator"));
    wxMenuItem* menuItemDonate = new wxMenuItem(menuHelp, MENU_DONATE
        , _t("Make a &Donation")
        , _t("Donate to the MMEX team to support infrastructure and development"));
    wxMenuItem* menuItemBuyCoffee = new wxMenuItem(menuHelp, MENU_BUY_COFFEE
        , _t("&Buy us a Coffee")
        , _t("Buy a key developer a coffee"));

    wxMenuItem* menuItemCommunity = new wxMenuItem(menuHelp, MENU_COMMUNITY
        , _t("&Community")
        , _t("Stay in touch with MMEX community"));

    wxMenu* menuCommunity = new wxMenu;
    menuCommunity->Append(menuItemWebsite);
    menuCommunity->Append(menuItemGitHub);
    menuCommunity->Append(menuItemCrowdin);
    menuCommunity->Append(menuItemWiki);
    menuCommunity->Append(menuItemNotify);
    menuCommunity->Append(menuItemReportIssues);
    menuCommunity->AppendSeparator();
    menuCommunity->Append(menuItemAppleAppStore);
    menuCommunity->Append(menuItemGooglePlay);
    menuCommunity->AppendSeparator();
    menuCommunity->Append(menuItemRSS);
    menuCommunity->Append(menuItemYouTube);
    menuCommunity->Append(menuItemFacebook);
    menuCommunity->Append(menuItemSlack);
    menuCommunity->Append(menuItemTwitter);
    menuCommunity->AppendSeparator();
    menuCommunity->Append(menuItemDonate);
    menuCommunity->Append(menuItemBuyCoffee);
    menuItemCommunity->SetSubMenu(menuCommunity);
    menuHelp->Append(menuItemCommunity);

    wxMenuItem* menuItemReportBug = new wxMenuItem(menuTools, MENU_REPORT_BUG
        , _t("Report a &Bug")
        , _t("Report an error in application to the developers"));
    menuHelp->Append(menuItemReportBug);

    wxMenuItem* menuItemDiagnostics = new wxMenuItem(menuTools, MENU_DIAGNOSTICS
        , _t("View &Diagnostics")
        , _t("Help provide information to the developers"));
    menuHelp->Append(menuItemDiagnostics);

    wxMenuItem* menuItemAppStart = new wxMenuItem(menuTools, MENU_SHOW_APPSTART
        , _t("&Show App Start Dialog"), _t("App Start Dialog"));
    menuHelp->Append(menuItemAppStart);

    wxMenuItem* menuItemCheck = new wxMenuItem(menuTools, MENU_CHECKUPDATE
        , _t("Check for &Updates"), _t("Check For Updates"));
    menuHelp->Append(menuItemCheck);

    wxMenuItem* menuItemAbout = new wxMenuItem(menuTools, wxID_ABOUT
        , _t("&About"), _t("About dialog"));
    menuHelp->Append(menuItemAbout);

    menuBar_ = new wxMenuBar;
    menuBar_->Append(menu_file, _t("&File"));
    menuBar_->Append(menuAccounts, _t("&Accounts"));
    menuBar_->Append(menuTools, _t("&Tools"));
    menuBar_->Append(menuView, _t("&View"));
    menuBar_->Append(menuHelp, _t("&Help"));
    SetMenuBar(menuBar_);

    menuBar_->Check(MENU_VIEW_HIDE_SHARE_ACCOUNTS, !PrefModel::instance().getHideShareAccounts());
    menuBar_->Check(MENU_VIEW_HIDE_DELETED_TRANSACTIONS, !PrefModel::instance().getHideDeletedTransactions());
    menuBar_->Check(MENU_VIEW_BUDGET_FINANCIAL_YEARS, PrefModel::instance().getBudgetFinancialYears());
    menuBar_->Check(MENU_VIEW_BUDGET_TRANSFER_TOTAL, PrefModel::instance().getBudgetIncludeTransfers());
    menuBar_->Check(MENU_VIEW_BUDGET_CATEGORY_SUMMARY, PrefModel::instance().getBudgetSummaryWithoutCategories());
    menuBar_->Check(MENU_VIEW_IGNORE_FUTURE_TRANSACTIONS, PrefModel::instance().getIgnoreFutureTransactions());
    menuBar_->Check(MENU_VIEW_SHOW_TOOLTIPS, PrefModel::instance().getShowToolTips());
    menuBar_->Check(MENU_VIEW_SHOW_MONEYTIPS, PrefModel::instance().getShowMoneyTips());
}
//----------------------------------------------------------------------------

void mmFrame::createToolBar()
{
    const long style = wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_HORIZONTAL | wxAUI_TB_PLAIN_BACKGROUND;

    toolBar_ = new wxAuiToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, style);
    toolBar_->SetToolBorderPadding(1);
    mmImage::themeMetaColour(toolBar_, mmImage::COLOR_LISTPANEL);
    toolBar_->Bind(wxEVT_RIGHT_DOWN, &mmFrame::OnToolbarRightClick, this);

    PopulateToolBar(false);
}

void  mmFrame::PopulateToolBar(bool update)
{
    const int toolbar_icon_size = PrefModel::instance().getToolbarIconSize();
    toolBar_->ClearTools();
    mmToolbarItem* ainfo = mmToolbarList::instance().getFirstEntry();
    while (ainfo != nullptr) {
        if (ainfo->active) {
            switch (ainfo->type) {
                case mmToolbarItem::TOOLBAR_BTN:
                    if (ainfo->toolId == MENU_ANNOUNCEMENTMAILING) {
                        wxString news_array;
                        for (const auto& entry : websiteNewsArray_) {
                            news_array += entry.m_titleN + "\n";
                        }
                        if (news_array.empty()) {
                            news_array = _t("News");
                        }
                        const auto news_ico = (websiteNewsArray_.size() > 0) ? mmImage::bitmapBundle(mmImage::png::NEW_NEWS, toolbar_icon_size) : mmImage::bitmapBundle(mmImage::png::NEWS, toolbar_icon_size);
                        toolBar_->AddTool(MENU_ANNOUNCEMENTMAILING, _t("News"), news_ico, news_array);
                    }
                    else {
                        toolBar_->AddTool(ainfo->toolId, ainfo->label, mmImage::bitmapBundle(ainfo->imageId, toolbar_icon_size), wxGetTranslation(ainfo->helpstring));
                    }
                    break;

                case mmToolbarItem::TOOLBAR_SEPARATOR:
                    toolBar_->AddSeparator();
                    break;

                case mmToolbarItem::TOOLBAR_STRETCH:
                    toolBar_->AddStretchSpacer();
                    break;

                case mmToolbarItem::TOOLBAR_SPACER:
                    toolBar_->AddSpacer(toolbar_icon_size);
                    break;
            }
        }
        ainfo = mmToolbarList::instance().getNextEntry(ainfo);
    }

    // after adding the buttons to the toolbar, must call Realize() to reflect changes
    toolBar_->Realize();

    if (update) {
        toolbarEnableItems(m_db);
        m_mgr.Update();
    }
}


void mmFrame::OnToolbarRightClick(wxMouseEvent& WXUNUSED(event))
{
    ToolbarDialog dlg(this);
    dlg.ShowModal();
}

//----------------------------------------------------------------------------

void mmFrame::InitializeModelTables()
{
    m_all_models.push_back(&InfoModel::instance(m_db.get()));
    m_all_models.push_back(&AssetModel::instance(m_db.get()));
    m_all_models.push_back(&StockModel::instance(m_db.get()));
    m_all_models.push_back(&StockHistoryModel::instance(m_db.get()));
    m_all_models.push_back(&AccountModel::instance(m_db.get()));
    m_all_models.push_back(&PayeeModel::instance(m_db.get()));
    m_all_models.push_back(&TrxModel::instance(m_db.get()));
    m_all_models.push_back(&CurrencyModel::instance(m_db.get()));
    m_all_models.push_back(&CurrencyHistoryModel::instance(m_db.get()));
    m_all_models.push_back(&BudgetPeriodModel::instance(m_db.get()));
    m_all_models.push_back(&CategoryModel::instance(m_db.get()));
    m_all_models.push_back(&SchedModel::instance(m_db.get()));
    m_all_models.push_back(&TrxSplitModel::instance(m_db.get()));
    m_all_models.push_back(&SchedSplitModel::instance(m_db.get()));
    m_all_models.push_back(&BudgetModel::instance(m_db.get()));
    m_all_models.push_back(&ReportModel::instance(m_db.get()));
    m_all_models.push_back(&AttachmentModel::instance(m_db.get()));
    m_all_models.push_back(&FieldValueModel::instance(m_db.get()));
    m_all_models.push_back(&FieldModel::instance(m_db.get()));
    m_all_models.push_back(&TagModel::instance(m_db.get()));
    m_all_models.push_back(&TagLinkModel::instance(m_db.get()));
    m_all_models.push_back(&TrxLinkModel::instance(m_db.get()));
    m_all_models.push_back(&TrxShareModel::instance(m_db.get()));

    ModelAll::instance(m_db.get());
}

bool mmFrame::createDataStore(const wxString& fileName, const wxString& pwd, bool openingNew)
{
    if (m_db) {
        ShutdownDatabase();
        // Backup the database according to user requirements
        if (PrefModel::instance().getDatabaseUpdated() &&
            SettingModel::instance().getBool("BACKUPDB_UPDATE", false)
        ) {
            dbUpgrade::BackupDB(
                m_filename,
                dbUpgrade::BACKUPTYPE::CLOSE,
                SettingModel::instance().getInt("MAX_BACKUP_FILES", 4)
            );
            PrefModel::instance().setDatabaseUpdated(false);
        }
    }

    wxFileName checkExt(fileName);
    wxString password;
    bool passwordCheckPassed = true;
    if ((checkExt.GetExt().Lower() == "emb" ||
        (checkExt.GetExt().Lower() == "bak" && (checkExt.GetName().Lower().Contains(".emb_update_") || checkExt.GetName().Lower().Contains(".emb_start_"))))
        && wxFileName::FileExists(fileName)) {
        wxString password_message = wxString::Format(
            _t("Please enter password for Database\n\n%s"),
            fileName
        );
        password = !pwd.empty() ? pwd :
            wxGetPasswordFromUser(password_message, _t("MMEX: Encrypted Database")
        );
        if (password.IsEmpty())
            passwordCheckPassed = false;
    }

    const wxString dialogErrorMessageHeading = _t("Opening MMEX Database – Error");

    // Existing Database
    if (!openingNew
        && !fileName.IsEmpty()
        && wxFileName::FileExists(fileName)
        && passwordCheckPassed
    ) {
        /* Do a backup before opening */
        if (SettingModel::instance().getBool("BACKUPDB", false)) {
            dbUpgrade::BackupDB(
                fileName,
                dbUpgrade::BACKUPTYPE::START,
                SettingModel::instance().getInt("MAX_BACKUP_FILES", 4)
            );
        }

        m_db = mmDBWrapper::Open(fileName, password);
        // if the database pointer has been reset, the password is possibly incorrect
        if (!m_db)
            return false;

        m_commit_callback_hook = new CommitCallbackHook();
        m_db->SetCommitHook(m_commit_callback_hook.get());
        m_update_callback_hook = new UpdateCallbackHook();
        m_db->SetUpdateHook(m_update_callback_hook.get());

        //Check if DB upgrade needed
        if (dbUpgrade::isUpgradeDBrequired(m_db.get())) {
            // close & reopen database in debug mode for upgrade (bypassing SQLITE_CorruptRdOnly flag)
            ShutdownDatabase();
            m_db = mmDBWrapper::Open(fileName, password, true);
            //DB backup is handled inside UpgradeDB
            if (!dbUpgrade::UpgradeDB(m_db.get(), fileName)) {
                int response = wxMessageBox(_t("Have MMEX support provided a debug/patch file?"), _t("MMEX upgrade"), wxYES_NO);
                if (response == wxYES) {
                    // upgrade failure turns CorruptRdOnly flag back on, so reopen again in debug mode
                    ShutdownDatabase();
                    m_db = mmDBWrapper::Open(fileName, password, true);
                    dbUpgrade::SqlFileDebug(m_db.get());
                }
                ShutdownDatabase();
                return false;
            }
        }

        InitializeModelTables();

        wxString UID = InfoModel::instance().getString("UID", wxEmptyString);
        if (UID.IsEmpty()) {
            UID = SettingModel::instance().getString("UUID", wxEmptyString);
            InfoModel::instance().saveString("UID", UID);
        }
        SettingModel::instance().saveString("UID", UID);

        // ** OBSOLETE **
        // Mantained only for really old compatibility reason and replaced by dbupgrade.cpp
        if (!InfoModel::instance().checkDBVersion()) {
            const wxString note = wxString::Format("%s - %s", mmex::getProgramName(), _t("No File opened"));
            this->SetTitle(note);
            wxMessageBox(_t("Sorry. The Database version is too old or Database password is incorrect")
                , dialogErrorMessageHeading
                , wxOK | wxICON_EXCLAMATION);

            ShutdownDatabase();
            return false;
        }

        m_password = password;

        // WE CAN EVENTUALLY DELETE THIS CODE
        // Get Hidden Categories id from stored INFO string and move to Category/Subcategory tables
        wxString sSettings = InfoModel::instance().getString("HIDDEN_CATEGS_ID", "");
        if (!sSettings.empty()) {
            wxStringTokenizer token(sSettings, ";");
            CategoryModel::instance().db_savepoint();
            while (token.HasMoreTokens()) {
                wxString catData = token.GetNextToken();
                wxLongLong_t catID = 0;
                wxLongLong_t subCatID = 0;
                if (2 == sscanf(catData.mb_str(),"*%lld:%lld*", &catID, &subCatID)) {
                    if (subCatID == -1) {
                        CategoryData* cat_n = CategoryModel::instance().unsafe_get_idN_data_n(catID);
                        if (cat_n && cat_n->m_id != -1) {
                            cat_n->m_active = false;
                            CategoryModel::instance().unsafe_update_data_n(cat_n);
                        }
                    }
                    else {
                        CategoryData* subcat_n = CategoryModel::instance().unsafe_get_idN_data_n(subCatID);
                        if (subcat_n && subcat_n->m_id != -1) {
                            subcat_n->m_active = false;
                            CategoryModel::instance().unsafe_update_data_n(subcat_n);
                        }
                    }
                }
            }
            CategoryModel::instance().db_release_savepoint();
            InfoModel::instance().saveString("HIDDEN_CATEGS_ID", "");
        }
    }
    else if (openingNew) { // New Database
        // Remove file so we can replace it instead of opening it
        if (wxFileName::FileExists(fileName))
            wxRemoveFile(fileName);

        m_db = mmDBWrapper::Open(fileName, password);
        m_commit_callback_hook = new CommitCallbackHook();
        m_db->SetCommitHook(m_commit_callback_hook.get());
        m_update_callback_hook = new UpdateCallbackHook();
        m_db->SetUpdateHook(m_update_callback_hook.get());

        m_password = password;
        dbUpgrade::InitializeVersion(m_db.get());
        InitializeModelTables();

        mmNewDatabaseWizard* wizard = new mmNewDatabaseWizard(this);
        wizard->CenterOnParent();
        if (!wizard->RunIt(true)) {
            ShutdownDatabase();
            wxRemoveFile(fileName);
            return false;
        }

        wxButton* next = static_cast<wxButton*>(wizard->FindWindow(wxID_FORWARD)); //FIXME:
        if (next) next->SetLabel(_tu("&Next →"));

        SetDataBaseParameters(fileName);
        /* Jump to new account creation screen */
        wxCommandEvent evt;
        OnNewAccount(evt);
        return true;
    }
    else { // open of existing database failed
        const wxString note = wxString::Format(
            "%s - %s",
            mmex::getProgramName(),
            _t("No File opened")
        );
        this->SetTitle(note);

        wxString msgStr = _t("Unable to find previously opened database.") + "\n";
        if (!passwordCheckPassed)
            msgStr = _t("Password not entered for encrypted database.") + "\n";

        msgStr << fileName;
        wxMessageBox(msgStr, dialogErrorMessageHeading, wxOK | wxICON_ERROR);
        menuEnableItems(false);
        return false;
    }

    SetDataBaseParameters(fileName);

    return true;
}
//----------------------------------------------------------------------------

void mmFrame::SetDataBaseParameters(const wxString& fileName)
{
    wxFileName fname(fileName);
    wxString title = wxString::Format("%s - %s (%s) %s",
                        fname.GetFullName(),
                        mmex::getProgramName(),
                        mmex::getTitleProgramVersion(),
                        wxGetOsDescription());
    if (mmPath::isPortableMode())
        title << " [" << _t("portable mode") << "]";
    SetTitle(title);

    if (m_db) {
        m_filename = fileName;
        /* Set Info Options into memory */
        PrefModel::instance().load();
    }
    else {
        m_filename.Clear();
        m_password.Clear();
    }
}
//----------------------------------------------------------------------------

bool mmFrame::openFile(const wxString& fileName, bool openingNew, const wxString &password)
{
    menuBar_->FindItem(MENU_CHANGE_ENCRYPT_PASSWORD)->Enable(false);
    if (createDataStore(fileName, password, openingNew)) {
        wxFileName fname(fileName);
        if (fname.GetExt().Upper() != "BAK") {
            m_recentFiles->AddFileToHistory(fileName);
        }
        menuEnableItems(true);
        menuPrintingEnable(false);

        if (m_db->IsEncrypted()) {
            menuBar_->FindItem(MENU_CHANGE_ENCRYPT_PASSWORD)->Enable(true);
        }

        if (!m_app->GetSilentParam()) {
            db_lockInPlace = InfoModel::instance().getBool("ISUSED", false);
            if (db_lockInPlace) {
                int response = wxMessageBox(_t(
                    "Attempting to open a database that has been marked as opened by another instance of MMEX.\n"
                    "To avoid data loss or conflict, it's strongly recommended that you close all other applications that may be using the database.\n\n"
                    "If nothing else is running, it's possible that the database was left open as a result of a crash during previous usage of MMEX.") +
                    "\n\n" +
                    _t("Do you want to open the database?")
                    , _t("MMEX Instance Check"), wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
                if (response == wxNO)
                    return false;
            }
        }

        InfoModel::instance().saveBool("ISUSED", true);
        db_lockInPlace = false;
        mmNavigatorList::instance().LoadFromDB();
        loadGrmIconMapping();
        autoRepeatTransactionsTimer_.Start(REPEAT_FREQ_TRANS_DELAY_TIME, wxTIMER_ONE_SHOT);
    }
    else
        return false;

    return true;
}
//----------------------------------------------------------------------------

void mmFrame::OnNew(wxCommandEvent& /*event*/)
{
    autoRepeatTransactionsTimer_.Stop();
    wxFileDialog dlg(this,
        _t("Choose database file to create"),
        wxEmptyString,
        wxEmptyString,
        _t("MMEX Database")+" (*.mmb)|*.mmb",
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT
    );

    if (dlg.ShowModal() != wxID_OK)
        return;

    wxString fileName = dlg.GetPath();

    if (!fileName.EndsWith(".mmb"))
        fileName += ".mmb";

    SetDatabaseFile(fileName, true);
    SettingModel::instance().saveString("LASTFILENAME", fileName);
    loadGrmIconMapping();
}
//----------------------------------------------------------------------------

void mmFrame::OnOpen(wxCommandEvent& WXUNUSED(event))
{
    autoRepeatTransactionsTimer_.Stop();
    wxString fileName = wxFileSelector(
        _t("Choose database file to open"),
        SettingModel::instance().getString("LAST_FILE_OPEN_PATH", wxEmptyString),
        wxEmptyString, wxEmptyString,
        _t("MMEX Database") + " (*.mmb)|*.mmb|" + _t("Encrypted MMEX Database") + " (*.emb)|*.emb|"
        + _t("MMEX Database Backup") + " (*.mmb_update_*.bak;*.mmb_start_*.bak)|*.mmb_update_*.bak;*.mmb_start_*.bak|"
        + _t("Encrypted MMEX Database Backup") + " (*.emb_update_*.bak;*.emb_start_*.bak)|*.emb_update_*.bak;*.emb_start_*.bak",
        wxFD_FILE_MUST_EXIST | wxFD_OPEN,
        this
    );

    if (!fileName.empty()) {
        SetDatabaseFile(fileName);
        saveSettings();
        SettingModel::instance().saveString("LAST_FILE_OPEN_PATH", wxFileName(fileName).GetPath());
        if (m_db) {
            autocleanDeletedTransactions();
            if (SettingModel::instance().getBool("REFRESH_STOCK_QUOTES_ON_OPEN", false) &&
                StockModel::instance().find_count() > 0
            ) {
                wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_RATES);
                this->GetEventHandler()->AddPendingEvent(evt);
            }
        }
    }
}
//----------------------------------------------------------------------------

void mmFrame::OnConvertEncryptedDB(wxCommandEvent& /*event*/)
{
    wxString encFileName = wxFileSelector(
        _t("Choose Encrypted database file to open"),
        wxEmptyString, wxEmptyString, wxEmptyString,
        _t("Encrypted MMEX Database") + " (*.emb)|*.emb",
        wxFD_FILE_MUST_EXIST,
        this
    );

    if (encFileName.empty())
        return;

    wxString password = wxGetPasswordFromUser(
        _t("Enter password for database"),
        _t("MMEX: Encrypted Database")
    );
    if (password.empty())
        return;

    wxFileDialog dlg(
        this,
        _t("Choose database file to Save As"),
        wxEmptyString,
        wxEmptyString,
        _t("MMEX Database") + " (*.mmb)|*.mmb",
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT
    );

    if (dlg.ShowModal() != wxID_OK)
        return;

    wxString fileName = dlg.GetPath();

    if (!dlg.GetPath().EndsWith(".mmb"))
        fileName += ".mmb";

    wxCopyFile(encFileName, fileName);

    wxSQLite3Database db;
    wxSQLite3CipherSQLCipher cipher;
    cipher.InitializeVersionDefault(4);
    cipher.SetLegacy(true);

    db.Open(fileName, cipher, password);
    db.ReKey(cipher, wxEmptyString);
    db.Close();

    mmErrorDialogs::MessageError(this, _t("Converted database!"), _t("MMEX message"));
}
//----------------------------------------------------------------------------

void mmFrame::OnChangeEncryptPassword(wxCommandEvent& /*event*/)
{
    wxString password_change_heading = _t("MMEX: Encryption Password Change");
    wxString password_message = wxString::Format(
        _t("New password for database:") + "\n\n%s",
        m_filename
    );

    wxPasswordEntryDialog dlg(this, password_message, password_change_heading);
    if (dlg.ShowModal() == wxID_OK) {
        wxString new_password = dlg.GetValue();
        if (new_password.IsEmpty()) {
            wxMessageBox(
                _t("New password must not be empty."),
                password_change_heading,
                wxOK | wxICON_WARNING
            );
        }
        else {
            wxPasswordEntryDialog confirm_dlg(
                this,
                _t("Please confirm new password"),
                password_change_heading
            );
            if (confirm_dlg.ShowModal() == wxID_OK) {
                wxString confirm_password = confirm_dlg.GetValue();
                if (!confirm_password.IsEmpty() && (new_password == confirm_password)) {
                    wxSQLite3CipherSQLCipher cipher;
                    cipher.InitializeVersionDefault(4);
                    cipher.SetLegacy(true);

                    m_db->ReKey(cipher, confirm_password);
                    wxMessageBox(_t("Password change completed"), password_change_heading);
                }
                else {
                    wxMessageBox(_t("Confirm password failed."), password_change_heading);
                }
            }
        }
    }
}
//----------------------------------------------------------------------------

void mmFrame::OnVacuumDB(wxCommandEvent& /*event*/)
{
    wxMessageDialog msgDlg(
        this,
        wxString::Format("%s\n\n%s", _t("Backup database before optimization."), _t("Do you want to continue?")),
        _t("DB Optimization"),
        wxYES_NO | wxNO_DEFAULT | wxICON_WARNING
    );
    if (msgDlg.ShowModal() == wxID_YES) {
        const wxString SizeBefore = wxFileName(m_filename).GetHumanReadableSize();
        m_db->Vacuum();
        const wxString SizeAfter = wxFileName(m_filename).GetHumanReadableSize();
        wxMessageBox(wxString::Format(
            _t("Database Optimization Completed!\n\n"
                "Size before: %1$s\n"
                "Size after: %2$s\n"
            ),
            SizeBefore, SizeAfter),
            _t("DB Optimization")
        );
    }
}
//----------------------------------------------------------------------------

void mmFrame::OnDebugDB(wxCommandEvent& /*event*/)
{
    wxASSERT(m_db);
    wxString resultMessage;
    wxSQLite3Statement stmt = m_db->PrepareStatement("PRAGMA integrity_check;");
    if (stmt.IsReadOnly()) {
        try {
            wxSQLite3ResultSet rs = stmt.ExecuteQuery();
            int columnCount = rs.GetColumnCount();
            while (rs.NextRow()) {
                wxString strRow = "";
                for (int i = 0; i < columnCount; ++i)
                    strRow << rs.GetAsString(i);
                resultMessage << strRow + wxTextFile::GetEOL();
            }
        }
        catch (const wxSQLite3Exception& e) {
            wxMessageBox(
                _t("Query error, please contact MMEX support!") + "\n\n" + e.GetMessage(),
                _t("MMEX debug error"),
                wxOK | wxICON_ERROR
            );
            return;
        }
    }

    if (!resultMessage.IsEmpty()) {
        wxTextEntryDialog checkDlg(this, _t("Result of database integrity check:"), _t("Database Check"), resultMessage.Trim(), wxOK | wxTE_MULTILINE);
        checkDlg.SetIcon(mmPath::getProgramIcon());
        wxTextCtrl* textCtrl = dynamic_cast<wxTextCtrl*>(checkDlg.FindWindow(3000));
        if (textCtrl) {
            textCtrl->SetEditable(false);
            textCtrl->SetInsertionPointEnd();
        }

        wxButton* ok = static_cast<wxButton*>(checkDlg.FindWindow(wxID_OK));
        if (ok)
            ok->SetLabel(wxGetTranslation(g_OkLabel));

        checkDlg.Centre();
        checkDlg.ShowModal();
    }

    wxMessageDialog msgDlg(
        this,
        wxString::Format("%s\n\n%s",
            _t("Please use this function only if requested by MMEX support and an .mmdbg debug file has been supplied."),
            _t("Do you want to continue?")
        ),
        _t("Database Debug"),
        wxYES_NO | wxNO_DEFAULT | wxICON_WARNING
    );

    msgDlg.SetYesNoLabels(_t("&Yes"), _t("&No"));

    if (msgDlg.ShowModal() == wxID_YES) {
        dbUpgrade::SqlFileDebug(m_db.get());
    }
}
//----------------------------------------------------------------------------

void mmFrame::OnCookieReset(wxCommandEvent& /*event*/)
{
     wxMessageDialog msgDlg(
        this,
        wxString::Format("%s\n\n%s", _t("Reset cookies used for Web API access."), _t("Do you want to continue?")),
        _t("Cookie Reset"),
        wxYES_NO | wxNO_DEFAULT | wxICON_WARNING
    );
    if (msgDlg.ShowModal() == wxID_YES) {
        SettingModel::instance().saveString("YAHOO_FINANCE_COOKIE", "");
        SettingModel::instance().saveString("YAHOO_FINANCE_CRUMB", "");
        wxMessageBox(_t("Cookies have been reset"), _t("Cookie Reset"));
    }
}
//----------------------------------------------------------------------------


void mmFrame::OnSaveAs(wxCommandEvent& /*event*/)
{
    wxASSERT(m_db);

    if (m_filename.empty()) {
        wxASSERT(false);
        return;
    }

    wxFileDialog dlg(this,
        _t("Save database file as"),
        SettingModel::instance().getString("LAST_FILE_OPEN_PATH", wxEmptyString),
        wxEmptyString,
        _t("MMEX Database")+" (*.mmb)|*.mmb|"+_t("Encrypted MMEX Database")+" (*.emb)|*.emb",
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT
    );

    if (dlg.ShowModal() != wxID_OK) return;

    // Ensure database is in a steady state first
    createHomePage();

    bool encrypt = dlg.GetFilterIndex() != 0; // emb -> Encrypted mMB
    wxFileName newFileName(dlg.GetPath());
    wxString ext = encrypt ? "emb" : "mmb";
    if (newFileName.GetExt().Lower() != ext) newFileName.SetExt(ext);

    wxFileName oldFileName(m_filename); // opened db's file

    if (newFileName == oldFileName) { // on case-sensitive FS uses case-sensitive comparison
        wxMessageDialog dlgMsg(
            this,
            _t("Unable to copy file to itself"),
            _t("Save database file as"),
            wxOK | wxICON_WARNING
        );
        dlgMsg.ShowModal();
        return;
    }

    // prepare to copy
    wxString new_password;
    bool rekey = encrypt ^ m_db->IsEncrypted();

    if (encrypt) {
        if (rekey) {
            new_password = wxGetPasswordFromUser(_t("Enter password for new database"), _t("MMEX: Encrypted Database"));
            if (new_password.empty())
                return;
        }
        else {
            new_password = m_password;
        }
    }

    // copying db
    ShutdownDatabase(); // database must be closed before copying its file

    if (!wxCopyFile(oldFileName.GetFullPath(), newFileName.GetFullPath(), true))  // true -> overwrite if file exists
        return;

    if (rekey) { // encrypt or reset encryption
        wxSQLite3Database dbx;

        wxSQLite3CipherSQLCipher cipher;
        cipher.InitializeVersionDefault(4);
        cipher.SetLegacy(true);

        dbx.Open(newFileName.GetFullPath(), cipher, m_password);
        dbx.ReKey(cipher, new_password); // empty password resets encryption
        dbx.Close();
    }

    m_password.clear();
    if (openFile(newFileName.GetFullPath(), false, new_password)) {
        DoRecreateNavTreeControl(true);
    }
}
//----------------------------------------------------------------------------

void mmFrame::OnExportToCSV(wxCommandEvent& /*event*/)
{
    mmUnivCSVDialog(this, mmUnivCSVDialog::DIALOG_TYPE_EXPORT_CSV, gotoAccountID_).ShowModal();
}
//----------------------------------------------------------------------------

void mmFrame::OnExportToXML(wxCommandEvent& /*event*/)
{
    mmUnivCSVDialog(this, mmUnivCSVDialog::DIALOG_TYPE_EXPORT_XML, gotoAccountID_).ShowModal();
}
//----------------------------------------------------------------------------

void mmFrame::OnExportToQIF(wxCommandEvent& /*event*/)
{
    mmQIFExportDialog dlg(this, mmQIFExportDialog::QIF, gotoAccountID_);
    dlg.ShowModal();
}
void mmFrame::OnExportToJSON(wxCommandEvent& /*event*/)
{
    mmQIFExportDialog dlg(this, mmQIFExportDialog::JSON, gotoAccountID_);
    dlg.ShowModal();
}
void mmFrame::OnExportToMMEX(wxCommandEvent& /*event*/)
{
    mmQIFExportDialog dlg(this, mmQIFExportDialog::CSV, gotoAccountID_);
    dlg.ShowModal();
}
//----------------------------------------------------------------------------

void mmFrame::OnImportQIF(wxCommandEvent& /*event*/)
{

    mmQIFImportDialog dlg(this, gotoAccountID_);
    dlg.ShowModal();
    int64 account_id = dlg.get_last_imported_acc();
    RefreshNavigationTree();
    if (account_id > 0) {
        setGotoAccountID(account_id);
        const AccountData* account = AccountModel::instance().get_idN_data_n(account_id);
        selectNavTreeItem(account->m_name);
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_GOTOACCOUNT);
        this->GetEventHandler()->AddPendingEvent(evt);
    }
    else {
        refreshPanelData();
    }

}
//----------------------------------------------------------------------------

void mmFrame::OnImportOFX(wxCommandEvent& /*event*/)
{
    mmOFXImportDialog dlg(this);
    dlg.ShowModal();
    refreshPanelData();
}

//----------------------------------------------------------------------------


void mmFrame::OnImportUniversalCSV(wxCommandEvent& /*event*/)
{
    if (AccountModel::instance().find_count() == 0) {
        wxMessageBox(
            _t("No account available to import"),
            _t("Universal CSV Import"),
            wxOK | wxICON_WARNING
        );
        return;
    }

    mmUnivCSVDialog univCSVDialog(this,
        mmUnivCSVDialog::DIALOG_TYPE_IMPORT_CSV,
        gotoAccountID_
    );
    univCSVDialog.ShowModal();
    if (univCSVDialog.isImportCompletedSuccessfully()) {
        const AccountData* account_n = AccountModel::instance().get_idN_data_n(
            univCSVDialog.ImportedAccountID()
        );
        if (account_n) {
            createCheckingPage(account_n->m_id);
            selectNavTreeItem(account_n->m_name);
        }
    }
}
//----------------------------------------------------------------------------

void mmFrame::OnImportXML(wxCommandEvent& /*event*/)
{
    if (AccountModel::instance().find_count() == 0) {
        wxMessageBox(
            _t("No account available to import"),
            _t("Universal CSV Import"),
            wxOK | wxICON_WARNING
        );
        return;
    }

    mmUnivCSVDialog univCSVDialog(this,
        mmUnivCSVDialog::DIALOG_TYPE_IMPORT_XML,
        gotoAccountID_
    );
    univCSVDialog.ShowModal();
    if (univCSVDialog.isImportCompletedSuccessfully()) {
        const AccountData* account_n = AccountModel::instance().get_idN_data_n(
            univCSVDialog.ImportedAccountID()
        );
        if (account_n) {
            createCheckingPage(account_n->m_id);
            selectNavTreeItem(account_n->m_name);
        }
    }
}

//----------------------------------------------------------------------------

void mmFrame::OnImportWebApp(wxCommandEvent& /*event*/)
{
    mmWebAppDialog dlg(this, false);
    if (dlg.ShowModal() == wxID_HELP) {
        helpFileIndex_ = mmPath::HTML_WEBAPP;
        createHelpPage(helpFileIndex_);
        setNavTreeSection(_t("Help"));
    }
    if (dlg.getRefreshRequested())
        refreshPanelData();
}
//----------------------------------------------------------------------------

void mmFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close(true);
}
//----------------------------------------------------------------------------

void mmFrame::OnNewAccount(wxCommandEvent& /*event*/)
{
    mmAddAccountWizard* wizard = new mmAddAccountWizard(this);
    wizard->CenterOnParent();
    wizard->RunIt();

    if (wizard->acctID_ != -1) {
        AccountData* account = AccountModel::instance().unsafe_get_idN_data_n(wizard->acctID_);
        AccountDialog dlg(account, this);
        dlg.ShowModal();
        if (mmNavigatorList::instance().isAssetAccount(account->m_type_)) {
            wxMessageBox(_t(
                "Asset Accounts hold Asset transactions\n\n"
                "Asset transactions are created within the Assets View\n"
                "after the selection of the Asset within that view.\n\n"
                "Asset Accounts can also hold normal transactions to regular accounts."
            ), _t("Asset Account Creation"));
        }

        else if (mmNavigatorList::instance().isShareAccount(account->m_type_)) {
            wxMessageBox(_tu(
                "Share Accounts hold Share transactions\n\n"
                "Share transactions are created within the Stock Portfolio View\n"
                "after the selection of the Company Stock within the associated view.\n\n"
                "These accounts only become visible after associating a Stock to the Share Account\n"
                "Or by using the Menu View → 'Display Share Accounts'\n"
                "Share Accounts can also hold normal transactions to regular account."
            ), _t("Share Account Creation"));
        }

        RefreshNavigationTree();
    }

    createHomePage();
}
//----------------------------------------------------------------------------

void mmFrame::OnAccountList(wxCommandEvent& /*event*/)
{
    createHomePage();
}
//----------------------------------------------------------------------------

void mmFrame::refreshPanelData(wxCommandEvent& /*event*/)
{
    refreshPanelData();
}
void mmFrame::refreshPanelData()
{
    int id = panelCurrent_ ? panelCurrent_->GetId() : mmID_HOMEPAGE;
    switch (id) {
    case mmID_HOMEPAGE:
        createHomePage();
        break;
    case mmID_CHECKING:
        wxDynamicCast(panelCurrent_, JournalPanel)->refreshList();
        break;
    case mmID_STOCKS:
        wxDynamicCast(panelCurrent_, StockPanel)->refreshList();
        break;
    case mmID_ASSETS:
        wxDynamicCast(panelCurrent_, AssetPanel)->refreshList();
        break;
    case mmID_BILLS:
        wxDynamicCast(panelCurrent_, SchedPanel)->refreshList();
        break;
    case mmID_BUDGET:
        wxDynamicCast(panelCurrent_, BudgetPanel)->refreshList();
        break;
    case mmID_REPORTS:
        if (activeReport_) {
            ReportPanel* reportsPanel = wxDynamicCast(panelCurrent_, ReportPanel);
            if (reportsPanel) createReportsPage(reportsPanel->getReportBase(), false);
        }
        break;
    default:
        createHelpPage();
        break;
    }
}

void mmFrame::SetNavTreeSelection(wxTreeItemId id) {
    if (id.IsOk()) {
        wxTreeEvent evt(wxEVT_TREE_SEL_CHANGED, m_nav_tree_ctrl, id);
        m_nav_tree_ctrl->GetEventHandler()->AddPendingEvent(evt);
    }
}

void mmFrame::OnOrgCategories(wxCommandEvent& /*event*/)
{
    CategoryManager dlg(this, false, -1);
    dlg.ShowModal();
    if (dlg.getRefreshRequested()) {
        activeReport_ = false;
        refreshPanelData();
        RefreshNavigationTree();
    }
}
//----------------------------------------------------------------------------

void mmFrame::OnOrgPayees(wxCommandEvent& /*event*/)
{
    mmPayeeDialog dlg(this, false);
    dlg.ShowModal();
    if (dlg.getAddActionRequested()) { // show transaction report
        std::list<int64> selections = dlg.getSelectedPayees();
        const PayeeData *payee_n = PayeeModel::instance().get_idN_data_n(selections.front());
        wxString filter = wxString::Format("{\"LABEL\":\"%s\",\"PAYEE\":\"%s\"}",
            _t("Transactions per payee"),
            payee_n->m_name
        );
        wxSharedPtr<TrxFilterDialog> pdlg(new TrxFilterDialog(this, filter));
        if (pdlg->ShowModal() == wxID_OK) {
            TrxReport* rs = new TrxReport(pdlg);
            createReportsPage(rs, true);
        }
    }
    else if (dlg.getRefreshRequested()) {
        activeReport_ = false;
        refreshPanelData();
        RefreshNavigationTree();
    }
}
//----------------------------------------------------------------------------

void mmFrame::OnOrgTags(wxCommandEvent& /*event*/)
{
    TagManager dlg(this);
    dlg.ShowModal();
    if (dlg.getRefreshRequested()) {
        activeReport_ = false;
        refreshPanelData();
        RefreshNavigationTree();
    }

}
//----------------------------------------------------------------------------

void mmFrame::OnNewTransaction(wxCommandEvent& event)
{
    if (!m_db)
        return;

    if (AccountModel::instance().find_all_name_a().empty())
        return;
    TrxDialog dlg(this, JournalKey(), false, gotoAccountID_);

    int i = dlg.ShowModal();
    if (i == wxID_CANCEL)
        return;

    gotoAccountID_ = dlg.trx_account_id();
    gotoTransID_ = JournalKey(-1, dlg.trx_id());
    const AccountData * account_n = AccountModel::instance().get_idN_data_n(
        gotoAccountID_
    );
    if (account_n) {
        createCheckingPage(gotoAccountID_);
        selectNavTreeItem(account_n->m_name);
    }

    if (i == wxID_NEW)
        OnNewTransaction(event);
}
//----------------------------------------------------------------------------

void mmFrame::OnTransactionReport(wxCommandEvent& WXUNUSED(event))
{
    if (!m_db) return;
    if (AccountModel::instance().find_count() == 0)
        return;

    const auto filter_settings = InfoModel::instance().getArrayString("TRANSACTIONS_FILTER");

    wxSharedPtr<TrxFilterDialog> dlg(new TrxFilterDialog(this, -1, true));
    bool is_ok = (dlg->ShowModal() == wxID_OK);
    if (filter_settings != InfoModel::instance().getArrayString("TRANSACTIONS_FILTER")) {
        DoRecreateNavTreeControl();
    }
    if (is_ok) {
        TrxReport* rs = new TrxReport(dlg);
        setNavTreeSection(_t("Transaction Report"));
        createReportsPage(rs, true);
    }
    else {
        m_nav_tree_ctrl->SelectItem(m_nav_tree_ctrl->GetRootItem());
    }
}

void mmFrame::OnBudgetSetupDialog(wxCommandEvent& WXUNUSED(event))
{
    if (!m_db)
        return;

    const auto a = BudgetPeriodModel::instance().find_data_a(
        TableClause::ORDERBY(BudgetPeriodCol::NAME_BUDGETYEARNAME)
    ).to_json();
    BudgetYearDialog(this).ShowModal();
    const auto b = BudgetPeriodModel::instance().find_data_a(
        TableClause::ORDERBY(BudgetPeriodCol::NAME_BUDGETYEARNAME)
    ).to_json();
    if (a != b)
        DoRecreateNavTreeControl(true);
    setNavTreeSection(_t("Budget Planner"));
}

void mmFrame::OnTransactionsAll(wxCommandEvent& WXUNUSED(event))
{
    createCheckingPage(-1);
}

void mmFrame::OnTransactionsDel(wxCommandEvent& WXUNUSED(event))
{
    createCheckingPage(-2);
}

void mmFrame::OnGeneralReportManager(wxCommandEvent& WXUNUSED(event))
{
    if (m_db) {
        wxTreeItemId selectedItem = m_nav_tree_ctrl->GetSelection();
        GeneralReportManager dlg(this, m_db.get(), selectedItem.IsOk() ? m_nav_tree_ctrl->GetItemText(selectedItem) : "");
        dlg.ShowModal();
        loadGrmIconMapping();
        RefreshNavigationTree();
    }
}

void mmFrame::OnOptions(wxCommandEvent& /*event*/)
{
    if (!m_db.get())
        return;

    PrefManager systemOptions(this, this->m_app);
    if (systemOptions.ShowModal() == wxID_OK) {
        //set the View Menu Option items the same as the options saved.
        menuBar_->FindItem(MENU_VIEW_BUDGET_FINANCIAL_YEARS)->Check(PrefModel::instance().getBudgetFinancialYears());
        menuBar_->FindItem(MENU_VIEW_BUDGET_TRANSFER_TOTAL)->Check(PrefModel::instance().getBudgetIncludeTransfers());
        menuBar_->FindItem(MENU_VIEW_BUDGET_CATEGORY_SUMMARY)->Check(PrefModel::instance().getBudgetSummaryWithoutCategories());
        menuBar_->FindItem(MENU_VIEW_IGNORE_FUTURE_TRANSACTIONS)->Check(PrefModel::instance().getIgnoreFutureTransactions());
        menuBar_->FindItem(MENU_VIEW_SHOW_TOOLTIPS)->Check(PrefModel::instance().getShowToolTips());
        menuBar_->FindItem(MENU_VIEW_SHOW_MONEYTIPS)->Check(PrefModel::instance().getShowMoneyTips());
        menuBar_->Refresh();
        menuBar_->Update();

        RefreshNavigationTree();
        refreshPanelData();

        // Reset columns of the checking panel in case the time columns was added/removed
        int id = panelCurrent_->GetId();
        if (id == mmID_CHECKING)
            wxDynamicCast(panelCurrent_, JournalPanel)->resetColumnView();

        const wxString& sysMsg = _t("Settings have been updated.") + "\n\n"
            + _t("Some settings take effect only after an application restart.");
        wxMessageBox(sysMsg, _t("Settings"), wxOK | wxICON_INFORMATION);
    }
}
//----------------------------------------------------------------------------

void mmFrame::OnCustomFieldsManager(wxCommandEvent& WXUNUSED(event))
{
    if (!m_db)
        return;

    FieldManager dlg(this);
    dlg.ShowModal();
    createHomePage();
}

void mmFrame::OnThemeManager(wxCommandEvent& WXUNUSED(event))
{
    ThemeManager tdlg(this);
    tdlg.ShowModal();
}

void mmFrame::OnEmptyTreePopUp(wxCommandEvent& event)
{
    int id = event.GetId();
    if (id == MENU_TREEPOPUP_EXPAND_ALL) {
        m_nav_tree_ctrl->ExpandAll();
    }
    else if (id == MENU_TREEPOPUP_COLLAPSE_ALL) {
        m_nav_tree_ctrl->CollapseAll();
    }
    else if (id == MENU_TREEPOPUP_CONFIG_NAV) {
        NavigatorDialog ndlg(this);
        ndlg.ShowModal();
        DoRecreateNavTreeControl(true);
    }
    else if (id == MENU_TREEPOPUP_THEME) {
        ThemeManager tdlg(this);
        tdlg.ShowModal();
    }
}

void mmFrame::OnDateRangeManager(wxCommandEvent& WXUNUSED(event))
{
    DateRangeManager dlg(this, DateRangeManager::TYPE_ID_CHECKING);
    if (dlg.ShowModal() == wxID_OK) {
        refreshPanelData();
    }
}

bool mmFrame::OnRefreshWebApp(bool is_silent)
{
    if (mmWebApp::uploadAccount()
        && mmWebApp::uploadCategory()
        && mmWebApp::uploadPayee()) {
        if (!is_silent) {
            wxMessageBox(_t("Accounts, Payees, and Categories Updated"), _t("Refresh WebApp"), wxOK | wxICON_INFORMATION);
        }
        return true;
    }
    else {
        if (!is_silent) {
            wxMessageBox(
                _t("Issue encountered updating WebApp, check Web server and WebApp settings"),
                _t("Refresh WebApp"),
                wxOK | wxICON_ERROR
            );
        }
    }
    return false;
}

void mmFrame::OnRefreshWebApp(wxCommandEvent&)
{
    OnRefreshWebApp(false);
}

//----------------------------------------------------------------------------

void mmFrame::OnHelp(wxCommandEvent& /*event*/)
{
    createHelpPage();
    setNavTreeSection(_t("Help"));
}
//----------------------------------------------------------------------------

void mmFrame::OnCheckUpdate(wxCommandEvent& /*event*/)
{
    mmUpdate::checkUpdates(this, false);
}
//----------------------------------------------------------------------------

void mmFrame::OnSimpleURLOpen(wxCommandEvent& event)
{
    wxString url;
    switch (event.GetId()) {
    case MENU_FACEBOOK:       url = mmex::weblink::Facebook;       break;
    case MENU_TWITTER:        url = mmex::weblink::Twitter;        break;
    case MENU_WEBSITE:        url = mmex::weblink::WebSite;        break;
    case MENU_WIKI:           url = mmex::weblink::Wiki;           break;
    case MENU_DONATE:         url = mmex::weblink::Donate;         break;
    case MENU_CROWDIN:        url = mmex::weblink::Crowdin;        break;
    case MENU_REPORTISSUES:   url = mmex::weblink::Forum;          break;
    case MENU_APPLE_APPSTORE: url = mmex::weblink::AppleAppStore;  break;
    case MENU_GOOGLEPLAY:     url = mmex::weblink::GooglePlay;     break;
    case MENU_BUY_COFFEE:     url = mmex::weblink::SquareCashGuan; break;
    case MENU_RSS:            url = mmex::weblink::NewsRSS;        break;
    case MENU_YOUTUBE:        url = mmex::weblink::YouTube;        break;
    case MENU_GITHUB:         url = mmex::weblink::GitHub;         break;
    case MENU_SLACK:          url = mmex::weblink::Slack;          break;
    }
    if (!url.IsEmpty())
        wxLaunchDefaultBrowser(url);
}
//----------------------------------------------------------------------------

void mmFrame::OnBeNotified(wxCommandEvent& /*event*/)
{
    SettingModel::instance().saveString(INIDB_NEWS_LAST_READ_DATE,
        mmDate::today().isoDate()
    );
    wxLaunchDefaultBrowser(mmex::weblink::News);

    int toolbar_icon_size = PrefModel::instance().getToolbarIconSize();
    //toolBar_->SetToolBitmapSize(wxSize(toolbar_icon_size, toolbar_icon_size));
    toolBar_->SetToolBitmap(MENU_ANNOUNCEMENTMAILING, mmImage::bitmapBundle(mmImage::png::NEWS, toolbar_icon_size));

    const auto b = toolBar_->FindTool(MENU_ANNOUNCEMENTMAILING);
    if (b) b->SetShortHelp(_t("News"));
}
//----------------------------------------------------------------------------

void mmFrame::OnReportBug(wxCommandEvent& WXUNUSED(event))
{
    ReportBase* br = new mmBugReport();
    setNavTreeSection(_t("Reports"));
    createReportsPage(br, true);
}

void mmFrame::OnDiagnostics(wxCommandEvent& /*event*/)
{
    DiagnosticsDialog dlg(this, this->IsMaximized());
    dlg.ShowModal();
}

void mmFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    AboutDialog(this, 0).ShowModal();
}
//----------------------------------------------------------------------------

void mmFrame::OnPrintPage(wxCommandEvent& WXUNUSED(event))
{
    if (panelCurrent_) {
        panelCurrent_->printPage();
    }
    else {
        wxLogDebug("Print: No panel selected");
    }
}

//----------------------------------------------------------------------------

void mmFrame::showBeginAppDialog(bool fromScratch)
{
    StartupDialog dlg(this, m_app);
    if (fromScratch) {
        dlg.SetCloseButtonToExit();
    }

    int end_mod = dlg.ShowModal();
    switch (end_mod) {
    case wxID_FILE1: {
        wxFileName fname(SettingModel::instance().getLastDbPath());
        if (fname.IsOk()) {
            SetDatabaseFile(fname.GetFullPath());
        }
        break;
    }
    case wxID_OPEN: {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_OPEN);
        AddPendingEvent(evt);
        break;
    }
    case wxID_NEW: {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_NEW);
        AddPendingEvent(evt);
        break;
    }
    case wxID_SETUP: {
        // auto language = PrefModel::instance().getLanguageID(true);
        break;
    }
    case wxID_EXIT:
        Close();
    }
}
//----------------------------------------------------------------------------

void mmFrame::OnShowAppStartDialog(wxCommandEvent& WXUNUSED(event))
{
    showBeginAppDialog();
}
//----------------------------------------------------------------------------

void mmFrame::OnExportToHtml(wxCommandEvent& WXUNUSED(event))
{
    wxString fileName = wxFileSelector(
        _t("Choose HTML file to export"),
        wxEmptyString, wxEmptyString, wxEmptyString,
        "*.html",
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT
    );

    if (!fileName.empty()) {
        wxString htmlText = panelCurrent_->buildPage();
        correctEmptyFileExt("html", fileName);
        wxFileOutputStream output(fileName);
        wxTextOutputStream text(output);
        text << htmlText;
        output.Close();
    }
}
//----------------------------------------------------------------------------

void mmFrame::OnBillsDeposits(wxCommandEvent& WXUNUSED(event))
{
    createBillsDeposits();
}
//----------------------------------------------------------------------------

void mmFrame::createHomePage()
{
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String("Home Page");

    const auto time = wxDateTime::UNow();

    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);
    int id = panelCurrent_ ? panelCurrent_->GetId() : -1;

    /* Update home page details only if it is being displayed */
    if (id == mmID_HOMEPAGE) {
        DashboardPanel* homePage = wxDynamicCast(panelCurrent_, DashboardPanel);
        homePage->createHtml();
    }
    else {
        DoWindowsFreezeThaw(homePanel_);
        wxSizer *sizer = cleanupHomePanel();
        panelCurrent_ = new DashboardPanel(homePanel_,
            this, mmID_HOMEPAGE,
            wxDefaultPosition, wxDefaultSize,
            wxNO_BORDER | wxTAB_TRAVERSAL
        );
        sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
        homePanel_->Layout();
        DoWindowsFreezeThaw(homePanel_);
    }

    wxTreeItemId root = m_nav_tree_ctrl->GetRootItem();
    if (root.IsOk() && m_nav_tree_ctrl->HasChildren(root)) {
        wxTreeItemIdValue cookie;
        wxTreeItemId firstChild = m_nav_tree_ctrl->GetFirstChild(root, cookie);
        if (firstChild.IsOk())
            m_nav_tree_ctrl->SelectItem(firstChild);
        m_nav_tree_ctrl->SetEvtHandlerEnabled(true);
    }

    json_writer.Key("seconds");
    json_writer.Double((wxDateTime::UNow() - time).GetMilliseconds().ToDouble() / 1000);
    json_writer.EndObject();

    UsageModel::instance().append_usage(wxString::FromUTF8(json_buffer.GetString()));
}
//----------------------------------------------------------------------------

void mmFrame::createReportsPage(ReportBase* rs, bool cleanup)
{
    if (!rs) return;
    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);
    DoWindowsFreezeThaw(homePanel_);
    wxSizer *sizer = cleanupHomePanel();
    panelCurrent_ = new ReportPanel(
        rs,
        cleanup, homePanel_, this, mmID_REPORTS,
        wxDefaultPosition, wxDefaultSize,
        wxNO_BORDER | wxTAB_TRAVERSAL
    );

    sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
    homePanel_->Layout();
    DoWindowsFreezeThaw(homePanel_);

    menuPrintingEnable(true);
    m_nav_tree_ctrl->SetEvtHandlerEnabled(true);
}
//----------------------------------------------------------------------------

void mmFrame::createHelpPage(int index)
{
    helpFileIndex_ = index;
    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);
    DoWindowsFreezeThaw(homePanel_);
    wxSizer *sizer = cleanupHomePanel();
    panelCurrent_ = new HelpPanel(homePanel_, this, wxID_HELP);
    sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
    homePanel_->Layout();
    DoWindowsFreezeThaw(homePanel_);
    menuPrintingEnable(true);
    m_nav_tree_ctrl->SetEvtHandlerEnabled(true);
}
//----------------------------------------------------------------------------

void mmFrame::createBillsDeposits()
{
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String("Bills & Deposits Panel");

    const auto time = wxDateTime::UNow();

    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);
    if (panelCurrent_ && panelCurrent_->GetId() == mmID_BILLS) {
        wxDynamicCast(panelCurrent_, SchedPanel)->refreshList();
    }
    else {
        DoWindowsFreezeThaw(homePanel_);
        wxSizer *sizer = cleanupHomePanel();
        panelCurrent_ = new SchedPanel(homePanel_, mmID_BILLS);

        sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);

        homePanel_->Layout();
        DoWindowsFreezeThaw(homePanel_);
    }
    menuPrintingEnable(true);
    m_nav_tree_ctrl->SetEvtHandlerEnabled(true);

    json_writer.Key("seconds");
    json_writer.Double((wxDateTime::UNow() - time).GetMilliseconds().ToDouble() / 1000);
    json_writer.EndObject();

    UsageModel::instance().append_usage(wxString::FromUTF8(json_buffer.GetString()));
    m_nav_tree_ctrl->SetFocus();
    setNavTreeSection(_t("Scheduled Transactions"));
}
//----------------------------------------------------------------------------

void mmFrame::createBudgetingPage(int64 budgetYearID)
{
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String("Budget Panel");

    const auto time = wxDateTime::UNow();

    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);
    if (panelCurrent_ && panelCurrent_->GetId() == mmID_BUDGET) {
        wxDynamicCast(panelCurrent_, BudgetPanel)->displayBudgetingDetails(budgetYearID);
    }
    else {
        DoWindowsFreezeThaw(homePanel_);
        wxSizer *sizer = cleanupHomePanel();
        panelCurrent_ = new BudgetPanel(budgetYearID, homePanel_, mmID_BUDGET);
        sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
        homePanel_->Layout();
        DoWindowsFreezeThaw(homePanel_);
    }

    json_writer.Key("seconds");
    json_writer.Double((wxDateTime::UNow() - time).GetMilliseconds().ToDouble() / 1000);
    json_writer.EndObject();

    UsageModel::instance().append_usage(wxString::FromUTF8(json_buffer.GetString()));

    menuPrintingEnable(true);
    m_nav_tree_ctrl->SetEvtHandlerEnabled(true);
}
//----------------------------------------------------------------------------

void mmFrame::createCheckingPage(int64 checking_id, const std::vector<int64> &group_ids) {
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String(
        (checking_id == -1) ? "All Transactions" :
        (checking_id == -2) ? "Deleted Transactions" :
        (checking_id <= -3) ? "Group Transactions" :
        "Checking Panel"
    );

    const auto time = wxDateTime::UNow();

    // Check if the credit balance needs to be displayed or not
    // If this differs from before then we need to rebuild
    bool newCreditDisplayed = false;
    if (checking_id >= 1) {
        const AccountData* account = AccountModel::instance().get_idN_data_n(checking_id);
        newCreditDisplayed = (account->m_credit_limit != 0);
    }

    m_nav_tree_ctrl->SetEvtHandlerEnabled(false);
    bool done = false;
    if (panelCurrent_ && panelCurrent_->GetId() == mmID_CHECKING) {
        JournalPanel* cp = wxDynamicCast(panelCurrent_, JournalPanel);
        if ((checking_id == -1 && cp->isAllTrans()) ||
            (checking_id == -2 && cp->isDeletedTrans()) ||
            (checking_id >= 1 && cp->isAccount() && newCreditDisplayed == creditDisplayed_)
        ) {
            if (cp->isAccount()) {
                cp->loadAccount(checking_id);
            }
            else {
                cp->refreshList();
            }
            done = true;
        }
    }
    if (!done) {
        DoWindowsFreezeThaw(homePanel_);
        if (checking_id >= 1)
            creditDisplayed_ = newCreditDisplayed;
        wxSizer *sizer = cleanupHomePanel();
        panelCurrent_ = new JournalPanel(this, homePanel_, checking_id, group_ids);
        sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
        homePanel_->Layout();
        DoWindowsFreezeThaw(homePanel_);
    }

    json_writer.Key("seconds");
    json_writer.Double((wxDateTime::UNow() - time).GetMilliseconds().ToDouble() / 1000);
    json_writer.EndObject();

    UsageModel::instance().append_usage(wxString::FromUTF8(json_buffer.GetString()));

    menuPrintingEnable(true);
    if (checking_id >= 1 && gotoTransID_.ref_id() > 0) {
        JournalPanel* cp = wxDynamicCast(panelCurrent_, JournalPanel);
        cp->setSelectedTransaction(gotoTransID_);
        gotoTransID_ = JournalKey();
    }
    m_nav_tree_ctrl->SetEvtHandlerEnabled(true);
    m_nav_tree_ctrl->SetFocus();
}

void mmFrame::createStocksAccountPage(int64 account_id)
{
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String("Stock Panel");

    const auto time = wxDateTime::UNow();

    if (panelCurrent_ && panelCurrent_->GetId() == mmID_STOCKS) {
        wxDynamicCast(panelCurrent_, StockPanel)->displayAccountDetails(account_id);
    }
    else {
        DoWindowsFreezeThaw(homePanel_);
        wxSizer *sizer = cleanupHomePanel();
        panelCurrent_ = new StockPanel(account_id, this, homePanel_);
        sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
        homePanel_->Layout();
        DoWindowsFreezeThaw(homePanel_);
    }

    json_writer.Key("seconds");
    json_writer.Double((wxDateTime::UNow() - time).GetMilliseconds().ToDouble() / 1000);
    json_writer.EndObject();

    UsageModel::instance().append_usage(wxString::FromUTF8(json_buffer.GetString()));
    menuPrintingEnable(true);
}

void mmFrame::OnGotoAccount(wxCommandEvent& event)
{
    int cmdInt = event.GetInt();
    switch(cmdInt)
    {
        case mmNavigatorItem::TYPE_ID_CHECKING:
            {
                wxString accid = event.GetString();
                wxLongLong_t id = -1;
                accid.ToLongLong(&id);
                setGotoAccountID(id);
                const AccountData* account_n = AccountModel::instance().get_idN_data_n(gotoAccountID_);
                if (account_n) {
                    if (AccountModel::type_id(*account_n) != mmNavigatorItem::TYPE_ID_INVESTMENT) {
                        createCheckingPage(gotoAccountID_);
                        selectNavTreeItem(account_n->m_name);
                    }
                }
            }
            break;

        case mmNavigatorItem::TYPE_ID_INVESTMENT:
            {
                wxString accid = event.GetString();
                wxLongLong_t id = -1;
                accid.ToLongLong(&id);
                setGotoAccountID(id);
                const AccountData* account_n = AccountModel::instance().get_idN_data_n(gotoAccountID_);
                if (account_n) {
                    if (AccountModel::type_id(*account_n) == mmNavigatorItem::TYPE_ID_INVESTMENT) {
                        createStocksAccountPage(gotoAccountID_);
                        selectNavTreeItem(account_n->m_name);
                    }
                }
            }
            break;

        case mmNavigatorItem::NAV_ENTRY_SCHEDULED_TRANSACTIONS:
            setNavTreeSectionById(cmdInt);
            createBillsDeposits();
            break;

        case mmNavigatorItem::TYPE_ID_ASSET:
            OnAssets(event);
            break;

        default:
            wxLogDebug("OnGotoAccount: unknown event %d", cmdInt);
    }
}

void mmFrame::OnAssets(wxCommandEvent& /*event*/)
{
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String("Asset Panel");

    const auto time = wxDateTime::UNow();

    if (panelCurrent_ && panelCurrent_->GetId() == mmID_ASSETS)
        refreshPanelData();
    else {
        DoWindowsFreezeThaw(homePanel_);
        wxSizer* sizer = cleanupHomePanel();
        panelCurrent_ = new AssetPanel(this, homePanel_, mmID_ASSETS);
        sizer->Add(panelCurrent_, 1, wxGROW | wxALL, 1);
        homePanel_->Layout();
        DoWindowsFreezeThaw(homePanel_);
        menuPrintingEnable(true);
        setNavTreeSection(_t("Assets"));
    }

    json_writer.Key("seconds");
    json_writer.Double((wxDateTime::UNow() - time).GetMilliseconds().ToDouble() / 1000);
    json_writer.EndObject();

    UsageModel::instance().append_usage(wxString::FromUTF8(json_buffer.GetString()));
}
//----------------------------------------------------------------------------

void mmFrame::OnCurrency(wxCommandEvent& /*event*/)
{
    CurrencyChoiceDialog(this, false, false).ShowModal();
    refreshPanelData();
}
//----------------------------------------------------------------------------

void mmFrame::OnRates(wxCommandEvent& WXUNUSED(event))
{
    wxBusyInfo info
#if (wxMAJOR_VERSION == 3 && wxMINOR_VERSION >= 1)
    (
        wxBusyInfoFlags()
        .Parent(this)
        .Title(_t("Downloading stock prices from Yahoo"))
        .Text(_tu("Please wait…"))
        .Foreground(*wxWHITE)
        .Background(wxColour(0, 102, 51))
        .Transparency(4 * wxALPHA_OPAQUE / 5)
    );
#else
    (_t("Downloading stock prices from Yahoo"), this);
#endif
    wxString msg;
    getOnlineCurrencyRates(msg);

    StockModel::DataA stock_a = StockModel::instance().find_data_a(
        TableClause::ORDERBY(StockCol::s_primary_name)
    );
    if (!stock_a.empty()) {

        std::map<wxString, double> symbols;
        for (const auto& stock_d : stock_a) {
            const wxString symbol = stock_d.m_symbol.Upper();
            if (symbol.IsEmpty()) continue;
            symbols[symbol] = stock_d.m_current_price;
        }

        std::map<wxString, double> stocks_data;
        if (get_yahoo_prices(symbols, stocks_data, "", msg, yahoo_price_type::SHARES)) {
            StockHistoryModel::instance().db_savepoint();
            for (auto& stock_d : stock_a) {
                std::map<wxString, double>::const_iterator it = stocks_data.find(
                    stock_d.m_symbol.Upper()
                );
                if (it == stocks_data.end())
                    continue;

                double dPrice = it->second;

                if (dPrice != 0) {
                    msg += wxString::Format("%s\t: %0.6f -> %0.6f\n",
                        stock_d.m_symbol, stock_d.m_current_price, dPrice
                    );
                    stock_d.m_current_price = dPrice;
                    if (stock_d.m_name.empty())
                        stock_d.m_name = stock_d.m_symbol;
                    StockModel::instance().save_data_n(stock_d);
                    StockHistoryModel::instance().save_record(
                        stock_d.m_symbol,
                        mmDate::today(),
                        dPrice,
                        UpdateType(UpdateType::e_online)
                    );
                }
            }
            StockHistoryModel::instance().db_release_savepoint();
            wxString strLastUpdate;
            strLastUpdate.Printf(_t("%1$s on %2$s"),
                wxDateTime::Now().FormatTime(),
                mmGetDateTimeForDisplay(wxDateTime::Now().FormatISODate())
            );
            InfoModel::instance().saveString("STOCKS_LAST_REFRESH_DATETIME", strLastUpdate);
        }

        wxLogDebug("%s", msg);
    }

    refreshPanelData();
}
//----------------------------------------------------------------------------

void mmFrame::OnEditAccount(wxCommandEvent& /*event*/)
{
    const auto& account_a = AccountModel::instance().find_data_a(
        TableClause::ORDERBY(AccountCol::NAME_ACCOUNTNAME)
    );
    if (account_a.empty()) {
        wxMessageBox(
            _t("No account available to edit!"),
            _t("Accounts"),
            wxOK | wxICON_WARNING
        );
        return;
    }

    mmSingleChoice scd(this,
        _t("Choose Account to Edit"),
        _t("Accounts"),
        account_a
    );
    if (scd.ShowModal() == wxID_OK) {
        const AccountData* account_n = AccountModel::instance().get_name_data_n(
            scd.GetStringSelection()
        );
        AccountData* edit_account_n = account_n
            ? AccountModel::instance().unsafe_get_idN_data_n(account_n->m_id)
            : nullptr;
        AccountDialog dlg(edit_account_n, this);
        if (dlg.ShowModal() == wxID_OK)
            RefreshNavigationTree();
    }
}
//----------------------------------------------------------------------------

void mmFrame::OnDeleteAccount(wxCommandEvent& /*event*/)
{
    const auto& account_a = AccountModel::instance().find_data_a(
        TableClause::ORDERBY(AccountCol::NAME_ACCOUNTNAME)
    );
    if (account_a.empty()) {
        wxMessageBox(
            _t("No account available to delete!"),
            _t("Accounts"),
            wxOK | wxICON_WARNING
        );
        return;
    }

    mmSingleChoice scd(this,
        _t("Choose Account to Delete"),
        _t("Accounts"),
        account_a
    );
    if (scd.ShowModal() == wxID_OK) {
        const AccountData* account_n = AccountModel::instance().get_name_data_n(
            scd.GetStringSelection()
        );
        wxString deletingAccountName = wxString::Format(
            _t("Do you you want to delete\n%1$s account: %2$s?"),
            wxGetTranslation(account_n->m_type_),
            account_n->m_name
        );
        wxMessageDialog msgDlg(this,
            deletingAccountName,
            _t("Confirm Account Deletion"),
            wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION
        );
        if (msgDlg.ShowModal() == wxID_YES) {
            AccountModel::instance().purge_id(account_n->m_id);
        }
    }
    DoRecreateNavTreeControl(true);
}
//----------------------------------------------------------------------------

void mmFrame::OnReallocateAccount(wxCommandEvent& WXUNUSED(event))
{
    mmSingleChoice account_choice(
        this,
        _t("Select account"),
        _t("Change Account Type"),
        AccountModel::instance().find_all_name_a()
    );

    if (account_choice.ShowModal() == wxID_OK) {
        const AccountData* account_n = AccountModel::instance().get_name_data_n(
            account_choice.GetStringSelection()
        );
        if (account_n)
            ReallocateAccount(account_n->m_id);
    }
}

void mmFrame::ReallocateAccount(int64 account_id)
{
    AccountData* account_n = AccountModel::instance().unsafe_get_idN_data_n(account_id);
    wxArrayString types = mmNavigatorList::instance().getAccountSelectionNames(
        account_n->m_type_
    );

    mmSingleChoice type_choice(
        this,
        wxString::Format(_t("Select new account type for %s"), account_n->m_name),
        _t("Change Account Type"),
        types
    );

    if (type_choice.ShowModal() == wxID_OK) {
        int sel = type_choice.GetSelection();
        account_n->m_type_ = mmNavigatorList::instance().getAccountDbTypeFromChoice(types[sel]);
        AccountModel::instance().unsafe_update_data_n(account_n);
        DoRecreateNavTreeControl(true);
    }
}

void mmFrame::OnViewToolbar(wxCommandEvent &event)
{
    m_mgr.GetPane("toolbar").Show(event.IsChecked());
    m_mgr.Update();
    SettingModel::instance().saveBool("SHOWTOOLBAR", event.IsChecked());
}

void mmFrame::OnViewLinks(wxCommandEvent& WXUNUSED(event))
{
    if (m_mgr.GetPane("Navigation").IsShown()) {
        SettingModel::instance().saveString("AUIPERSPECTIVE_NAV", m_mgr.SavePerspective());
        m_mgr.GetPane("Navigation").Hide();
    }
    else {
        m_mgr.GetPane("Navigation").Show();
        m_mgr.LoadPerspective(SettingModel::instance().getString("AUIPERSPECTIVE_NAV", wxEmptyString));
    }
    m_mgr.Update();
}

void mmFrame::OnViewToolbarUpdateUI(wxUpdateUIEvent &event)
{
    event.Check(m_mgr.GetPane("toolbar").IsShown());
}

void mmFrame::OnViewLinksUpdateUI(wxUpdateUIEvent &event)
{
    event.Check(m_mgr.GetPane("Navigation").IsShown());
}

void mmFrame::OnHideShareAccounts(wxCommandEvent &WXUNUSED(event))
{
    PrefModel::instance().saveHideShareAccounts(!PrefModel::instance().getHideShareAccounts());
    mmNavigatorList::instance().SetShareAccountStatus(!PrefModel::instance().getHideShareAccounts());
    RefreshNavigationTree();
}

void mmFrame::OnHideDeletedTransactions(wxCommandEvent& WXUNUSED(event))
{
    PrefModel::instance().saveHideDeletedTransactions(!PrefModel::instance().getHideDeletedTransactions());
    mmNavigatorList::instance().SetTrashStatus(!PrefModel::instance().getHideDeletedTransactions());
    RefreshNavigationTree();
}

void mmFrame::RefreshNavigationTree()
{
    // Save currently selected item data
    mmTreeItemData* iData = nullptr;
    wxString sectionName;
    wxTreeItemId selection = m_nav_tree_ctrl->GetSelection();
    if (selection.IsOk()) {
        selectedItemData_ = dynamic_cast<mmTreeItemData*>(m_nav_tree_ctrl->GetItemData(selection));
        if (selectedItemData_) {
            iData = new mmTreeItemData(*selectedItemData_);
            // also save current section
            wxTreeItemId parentID = m_nav_tree_ctrl->GetItemParent(selection);
            if (parentID.IsOk() && parentID != m_nav_tree_ctrl->GetRootItem())
                sectionName = m_nav_tree_ctrl->GetItemText(parentID);
        }
    }

    DoRecreateNavTreeControl();
    // Find and reselect the previously selected item
    if (iData) {
        // search for the item first under the selected section
        wxTreeItemId navTreeID = sectionName.empty() ? m_nav_tree_ctrl->GetRootItem() :
            getNavTreeChild(m_nav_tree_ctrl->GetRootItem(), sectionName);
        if (navTreeID.IsOk())
            navTreeID = findItemByData(navTreeID, *iData);
        // if we didn't find it search all nodes from root
        if (!navTreeID.IsOk())
            navTreeID = findItemByData(m_nav_tree_ctrl->GetRootItem(), *iData);
        if (navTreeID.IsOk()) {
            m_nav_tree_ctrl->EnsureVisible(navTreeID);
            m_nav_tree_ctrl->SelectItem(navTreeID);
        }
        else {
            wxLogDebug("Refresh Tree: Original selectedItem not found => Reset to Dashboard");
            panelCurrent_ = nullptr;
        }
        delete(iData);
    }
}

wxTreeItemId mmFrame::findItemByData(wxTreeItemId itemId, mmTreeItemData& searchData)
{
    // Check if the current item's data matches the search data
    if (itemId.IsOk())
        return wxTreeItemId();
    if (m_nav_tree_ctrl->GetItemData(itemId)) {
        if (*dynamic_cast<mmTreeItemData*>(m_nav_tree_ctrl->GetItemData(itemId)) == searchData)
            return itemId;
    }

    wxTreeItemIdValue searchCookie;
    wxTreeItemId childId = m_nav_tree_ctrl->GetFirstChild(itemId, searchCookie);
    while (childId.IsOk()) {
        // Recursively search for the item in the child subtree
        wxTreeItemId foundId = findItemByData(childId, searchData);
        if (foundId.IsOk()) {
            // Item found in the child subtree, return it
            return foundId;
        }

        // Not found yet, so move to the next child
        childId = m_nav_tree_ctrl->GetNextChild(itemId, searchCookie);
    }

    // Item not found
    return wxTreeItemId();
}

void mmFrame::OnViewBudgetFinancialYears(wxCommandEvent& WXUNUSED(event))
{
    PrefModel::instance().saveBudgetFinancialYears(!PrefModel::instance().getBudgetFinancialYears());
    refreshPanelData();
}

void mmFrame::OnViewBudgetTransferTotal(wxCommandEvent& WXUNUSED(event))
{
    PrefModel::instance().saveBudgetIncludeTransfers(!PrefModel::instance().getBudgetIncludeTransfers());
    refreshPanelData();
}

void mmFrame::OnViewBudgetCategorySummary(wxCommandEvent& WXUNUSED(event))
{
    PrefModel::instance().saveBudgetSummaryWithoutCategories(!PrefModel::instance().getBudgetSummaryWithoutCategories());
    refreshPanelData();
}

void mmFrame::OnViewIgnoreFutureTransactions(wxCommandEvent& WXUNUSED(event))
{
    PrefModel::instance().saveIgnoreFutureTransactions(!PrefModel::instance().getIgnoreFutureTransactions());
    RefreshNavigationTree();
}

void mmFrame::OnViewShowToolTips(wxCommandEvent& WXUNUSED(event))
{
    PrefModel::instance().saveShowToolTips(!PrefModel::instance().getShowToolTips());
    RefreshNavigationTree();
}

void mmFrame::OnViewShowMoneyTips(wxCommandEvent& WXUNUSED(event))
{
    PrefModel::instance().saveShowMoneyTips(!PrefModel::instance().getShowMoneyTips());
    RefreshNavigationTree();
}
//----------------------------------------------------------------------------

void mmFrame::OnCategoryRelocation(wxCommandEvent& /*event*/)
{
    MergeCategoryDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK) {
        wxString msgStr;
        msgStr << _t("Merge categories completed") << "\n\n"
            << wxString::Format(_t("Records have been updated in the database: %i"),
                dlg.updatedCategoriesCount());
        wxMessageBox(msgStr, _t("Merge categories result"));
        refreshPanelData();
    }
}
//----------------------------------------------------------------------------

void mmFrame::OnPayeeRelocation(wxCommandEvent& /*event*/)
{
    MergePayeeDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK) {
        wxString msgStr;
        msgStr << _t("Merge payees completed") << "\n\n"
            << wxString::Format(_t("Records have been updated in the database: %i"),
                dlg.updatedPayeesCount())
            << "\n\n";
        wxMessageBox(msgStr, _t("Merge payees result"));
        refreshPanelData();
    }
}
//----------------------------------------------------------------------------

void mmFrame::OnTagRelocation(wxCommandEvent& /*event*/)
{
    MergeTagDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK) {
        wxString msgStr;
        msgStr << _t("Merge tags completed") << "\n\n"
            << wxString::Format(_t("Records have been updated in the database: %i"),
                dlg.updatedTagsCount())
            << "\n\n";
        wxMessageBox(msgStr, _t("Merge tags result"));
        refreshPanelData();
    }
}
//----------------------------------------------------------------------------

wxSizer* mmFrame::cleanupHomePanel(bool new_sizer)
{
    wxASSERT(homePanel_);

    homePanel_->DestroyChildren();
    homePanel_->SetSizer(new_sizer ? new wxBoxSizer(wxHORIZONTAL) : nullptr);

    return homePanel_->GetSizer();
}
//----------------------------------------------------------------------------

void mmFrame::autocleanDeletedTransactions() {
    wxDateSpan days = wxDateSpan::Days(
        SettingModel::instance().getInt("DELETED_TRANS_RETAIN_DAYS", 30)
    );
    wxDateTime earliestDate = wxDateTime().Now().ToUTC().Subtract(days);
    std::vector<int64> trx_id_a = TrxModel::instance().find_id_a(
        TrxModel::WHERE_IS_DELETED(true),
        TrxCol::WHERE_DELETEDTIME(OP_LE, earliestDate.FormatISOCombined())
    );
    if (trx_id_a.empty())
        return;

    TrxModel::instance().db_savepoint();
    for (int64 trx_id : trx_id_a) {
        TrxModel::instance().purge_id(trx_id);
    }
    TrxModel::instance().db_release_savepoint();
}

void mmFrame::SetDatabaseFile(const wxString& dbFileName, bool newDatabase)
{
    autoRepeatTransactionsTimer_.Stop();

    if (openFile(dbFileName, newDatabase)) {
        DoRecreateNavTreeControl(true);
        mmLoadColorsFromDatabase();
    }
    else {
        mmNavigatorList::instance().SetToDefault();
        resetNavTreeControl();
        cleanupHomePanel();
        showBeginAppDialog(true);
    }
}
//----------------------------------------------------------------------------

void mmFrame::OnRecentFiles(wxCommandEvent& event)
{
    int fileNum = event.GetId() - m_recentFiles->GetBaseId();
    if (fileNum == 0)
        return;
    const wxString file_name = m_recentFiles->GetHistoryFile(fileNum);
    wxFileName file(file_name);
    if (file.FileExists()) {
        SetDatabaseFile(file_name);
        saveSettings();
    }
    else {
        wxMessageBox(
            wxString::Format(_t("File %s not found."), file_name),
            _t("Error"),
            wxOK | wxICON_ERROR
        );
        m_recentFiles->RemoveFileFromHistory(fileNum);
    }
}
//----------------------------------------------------------------------------

void mmFrame::OnClearRecentFiles(wxCommandEvent& /*event*/)
{
    m_recentFiles->ClearHistory();
    m_recentFiles->AddFileToHistory(m_filename);
}

void mmFrame::setGotoAccountID(int64 account_id, JournalKey journal_key)
{
    gotoAccountID_ = account_id;
    gotoTransID_ = journal_key;
}

void mmFrame::OnToggleFullScreen(wxCommandEvent& WXUNUSED(event))
{
#if (wxMAJOR_VERSION >= 3 && wxMINOR_VERSION >= 0)
    this->ShowFullScreen(!IsFullScreen());
#endif
}

void mmFrame::OnResetView(wxCommandEvent& WXUNUSED(event))
{
    SettingModel::instance().saveBool("SHOWTOOLBAR", true);
    m_mgr.GetPane("toolbar").Show(true).Dock().Top().Position(0);
    m_mgr.GetPane("Navigation").Show(true).Dock().Left();
    m_mgr.Update();
}

void mmFrame::OnClose(wxCloseEvent&)
{
    Destroy();
}

void mmFrame::OnChangeGUILanguage(wxCommandEvent& event)
{
    wxLanguage lang = static_cast<wxLanguage>(event.GetId() - MENU_LANG - 1);
    if (lang != m_app->getGUILanguage() && m_app->setGUILanguage(lang))
        mmErrorDialogs::MessageWarning(
            this,
            _t("The language for this application has been changed. "
                "The change will take effect the next time the application is started."),
            _t("Language change")
        );
}

void mmFrame::DoUpdateBudgetNavigation(wxTreeItemId& parent_item)
{
    BudgetPeriodModel::DataA bp_a = BudgetPeriodModel::instance().find_data_a(
        TableClause::ORDERBY(BudgetPeriodCol::NAME_BUDGETYEARNAME)
    );
    if (bp_a.empty())
        return;

    std::map <wxString, int64> years;

    wxRegEx pattern_year(R"(^([0-9]{4})$)");
    wxRegEx pattern_month(R"(^([0-9]{4})-([0-9]{2})$)");

    for (const auto& bp_d : bp_a) {
        const wxString& name = bp_d.m_name;
        if (pattern_year.Matches(name)) {
            years[name] = bp_d.m_id;
        }
        else if (pattern_month.Matches(name)) {
            wxString root_year = pattern_month.GetMatch(name, 1);
            if (years.find(root_year) == years.end())
                years[root_year] = bp_d.m_id;
        }
    }

    wxTreeItemId year_budget;
    for (const auto& entry : years) {
        for (const auto& bp_d : bp_a) {
            if (entry.second == bp_d.m_id) {
                year_budget = addNavTreeItem(
                    parent_item, bp_d.m_name,
                    mmImage::img::CALENDAR_PNG, mmTreeItemData::BUDGET,
                    bp_d.m_id
                );
            }
            else if (pattern_month.Matches(bp_d.m_name) &&
                pattern_month.GetMatch(bp_d.m_name, 1) == entry.first
            ) {
                addNavTreeItem(
                    year_budget, bp_d.m_name,
                    mmImage::img::CALENDAR_PNG, mmTreeItemData::BUDGET,
                    bp_d.m_id
                );

            }
        }
    }
}

void mmFrame::SetTrashState(bool state){
    menuBar_->FindItem(MENU_VIEW_HIDE_DELETED_TRANSACTIONS)->Check(state);
}

void mmFrame::SetShareAccountState(bool state){
    menuBar_->FindItem(MENU_VIEW_HIDE_SHARE_ACCOUNTS)->Check(state);
}

void mmFrame::DoUpdateReportNavigation(wxTreeItemId& parent_item)
{
    wxArrayString hidden_reports = InfoModel::instance().getArrayString("HIDDEN_REPORTS");

    if (hidden_reports.Index("Cash Flow") == wxNOT_FOUND)
    {
        wxTreeItemId cashFlow = m_nav_tree_ctrl->AppendItem(parent_item, _t("Cash Flow"), mmImage::img::PIECHART_PNG, mmImage::img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(cashFlow, new mmTreeItemData(mmTreeItemData::MENU_REPORT, "Cash Flow"));

        wxTreeItemId cashflowWithBankAccounts = m_nav_tree_ctrl->AppendItem(cashFlow, _t("Daily"), mmImage::img::PIECHART_PNG, mmImage::img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(cashflowWithBankAccounts, new mmTreeItemData("Cash Flow - Daily", new mmReportCashFlowDaily()));

        wxTreeItemId cashflowWithTermAccounts = m_nav_tree_ctrl->AppendItem(cashFlow, _t("Monthly"), mmImage::img::PIECHART_PNG, mmImage::img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(cashflowWithTermAccounts, new mmTreeItemData("Cash Flow - Monthly", new mmReportCashFlowMonthly()));

        wxTreeItemId cashflowWithTransactions = m_nav_tree_ctrl->AppendItem(cashFlow, _t("Transactions"), mmImage::img::PIECHART_PNG, mmImage::img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(cashflowWithTransactions, new mmTreeItemData("Cash Flow - Transactions", new mmReportCashFlowTransactions()));
    }

    ///////////////////////////////////////////////////////////////////

    if (hidden_reports.Index("Categories") == wxNOT_FOUND)
    {
        wxTreeItemId categs = m_nav_tree_ctrl->AppendItem(parent_item, _t("Categories"), mmImage::img::PIECHART_PNG, mmImage::img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(categs, new mmTreeItemData(mmTreeItemData::MENU_REPORT, "Categories"));

        wxTreeItemId categsMonthly = m_nav_tree_ctrl->AppendItem(categs, _t("Monthly"), mmImage::img::PIECHART_PNG, mmImage::img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(categsMonthly, new mmTreeItemData("Categories Monthly", new mmReportCategoryOverTimePerformance()));

        wxTreeItemId categsSummary = m_nav_tree_ctrl->AppendItem(categs, _t("Summary"), mmImage::img::PIECHART_PNG, mmImage::img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(categsSummary, new mmTreeItemData("Categories Summary", new  mmReportCategoryExpensesCategories()));

        wxTreeItemId categsGoes = m_nav_tree_ctrl->AppendItem(categs, _t("Where the Money Goes"), mmImage::img::PIECHART_PNG, mmImage::img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(categsGoes, new mmTreeItemData("Where the Money Goes", new mmReportCategoryExpensesGoes()));

        wxTreeItemId categsComes = m_nav_tree_ctrl->AppendItem(categs, _t("Where the Money Comes From"), mmImage::img::PIECHART_PNG, mmImage::img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(categsComes, new mmTreeItemData("Where the Money Comes From", new mmReportCategoryExpensesComes()));
    }

    //////////////////////////////////////////////////////////////////

    if (hidden_reports.Index("Forecast Report") == wxNOT_FOUND)
    {
        wxTreeItemId forecastReport = m_nav_tree_ctrl->AppendItem(parent_item, _t("Forecast Report"), mmImage::img::PIECHART_PNG, mmImage::img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(forecastReport, new mmTreeItemData("Forecast Report", new ForecastReport()));
    }

    ///////////////////////////////////////////////////////////////////

    if (hidden_reports.Index("Income vs Expenses") == wxNOT_FOUND)
    {
        wxTreeItemId incexpOverTime = m_nav_tree_ctrl->AppendItem(parent_item, _t("Income vs. Expenses"), mmImage::img::PIECHART_PNG, mmImage::img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(incexpOverTime, new mmTreeItemData("Income vs Expenses", new InExReport()));

        wxTreeItemId incexpMonthly = m_nav_tree_ctrl->AppendItem(incexpOverTime, _t("Monthly"), mmImage::img::PIECHART_PNG, mmImage::img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(incexpMonthly, new mmTreeItemData("Income vs Expenses - Monthly", new mmReportIncomeExpensesMonthly()));
    }

    ///////////////////////////////////////////////////////////////////

    if (hidden_reports.Index("My Usage") == wxNOT_FOUND)
    {
        wxTreeItemId myusage = m_nav_tree_ctrl->AppendItem(parent_item, _t("My Usage"), mmImage::img::PIECHART_PNG, mmImage::img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(myusage, new mmTreeItemData("My Usage", new UsageReport()));
    }

    //////////////////////////////////////////////////////////////////////

    if (hidden_reports.Index("Payees") == wxNOT_FOUND)
    {
        wxTreeItemId payeesOverTime = m_nav_tree_ctrl->AppendItem(parent_item, _t("Payees"), mmImage::img::PIECHART_PNG, mmImage::img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(payeesOverTime, new mmTreeItemData("Payee Report", new PayeeReport()));
    }

    //////////////////////////////////////////////////////////////////

    if (hidden_reports.Index("Summary of Accounts") == wxNOT_FOUND)
    {
        wxTreeItemId reportsSummary = m_nav_tree_ctrl->AppendItem(parent_item, _t("Summary of Accounts"), mmImage::img::PIECHART_PNG, mmImage::img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(reportsSummary, new mmTreeItemData(mmTreeItemData::MENU_REPORT, "Summary of Accounts"));

        wxTreeItemId accMonthly = m_nav_tree_ctrl->AppendItem(reportsSummary, _t("Monthly"), mmImage::img::PIECHART_PNG, mmImage::img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(
            accMonthly,
            new mmTreeItemData("Monthly Summary of Accounts",
                new BalanceReport(BalanceReport::PERIOD_ID::MONTH)
            )
        );

        wxTreeItemId accYearly = m_nav_tree_ctrl->AppendItem(reportsSummary, _t("Yearly"), mmImage::img::PIECHART_PNG, mmImage::img::PIECHART_PNG);
        m_nav_tree_ctrl->SetItemData(
            accYearly,
            new mmTreeItemData("Yearly Summary of Accounts",
                new BalanceReport(BalanceReport::PERIOD_ID::YEAR)
            )
        );
    }

    //////////////////////////////////////////////////////////////////

    if (BudgetPeriodModel::instance().find_count() > 0) {
        if (hidden_reports.Index("Budgets") == wxNOT_FOUND) {
            wxTreeItemId budgetReports = m_nav_tree_ctrl->AppendItem(parent_item, _t("Budgets"), mmImage::img::PIECHART_PNG, mmImage::img::PIECHART_PNG);
            m_nav_tree_ctrl->SetItemData(budgetReports, new mmTreeItemData(mmTreeItemData::MENU_REPORT, "Budgets"));

            wxTreeItemId budgetPerformance = m_nav_tree_ctrl->AppendItem(budgetReports, _t("Budget Performance"), mmImage::img::PIECHART_PNG, mmImage::img::PIECHART_PNG);
            m_nav_tree_ctrl->SetItemData(budgetPerformance, new mmTreeItemData("Budget Performance", new mmReportBudgetingPerformance()));

            wxTreeItemId budgetSetupPerformance = m_nav_tree_ctrl->AppendItem(budgetReports, _t("Budget Category Summary"), mmImage::img::PIECHART_PNG, mmImage::img::PIECHART_PNG);
            m_nav_tree_ctrl->SetItemData(budgetSetupPerformance, new mmTreeItemData("Budget Category Summary", new mmReportBudgetCategorySummary()));
        }
    }

    if (
        AccountModel::instance().find_count(
            AccountCol::WHERE_ACCOUNTTYPE(OP_EQ, mmNavigatorList::instance().getInvestmentAccountStr())
        ) > 0 &&
        hidden_reports.Index("Stocks Report") == wxNOT_FOUND
    ) {
        wxTreeItemId stocksReport = m_nav_tree_ctrl->AppendItem(
            parent_item,
            _t("Stocks Report"),
            mmImage::img::PIECHART_PNG, mmImage::img::PIECHART_PNG
        );
        m_nav_tree_ctrl->SetItemData(
            stocksReport,
            new mmTreeItemData("Stocks Report", new mmReportChartStocks())
        );

        wxTreeItemId stocksReportSummary = m_nav_tree_ctrl->AppendItem(
            stocksReport,
            _t("Summary"),
            mmImage::img::PIECHART_PNG, mmImage::img::PIECHART_PNG
        );
        m_nav_tree_ctrl->SetItemData(
            stocksReportSummary,
            new mmTreeItemData("Summary of Stocks", new StocksReport())
        );
    }
}

void mmFrame::DoUpdateGRMNavigation(wxTreeItemId& parent_item)
{
    // GRM Reports
    ReportModel::DataA report_a = ReportModel::instance().find_data_a(
        ReportCol::WHERE_ACTIVE(OP_EQ, 1)
    );
    // Sort by group name and report name
    std::sort(report_a.begin(), report_a.end(), ReportData::SorterByREPORTNAME());
    std::stable_sort(report_a.begin(), report_a.end(), ReportData::SorterByGROUPNAME());

    wxTreeItemId group;
    wxString group_name;
    for (const ReportData& report_d : report_a) {
        bool no_group = report_d.m_group_name.empty();
        if (group_name != report_d.m_group_name && !no_group) {
            group = m_nav_tree_ctrl->AppendItem(
                parent_item,
                wxGetTranslation(report_d.m_group_name),
                mmImage::img::CUSTOMSQL_GRP_PNG, mmImage::img::CUSTOMSQL_GRP_PNG
            );
            m_nav_tree_ctrl->SetItemBold(group, true);
            m_nav_tree_ctrl->SetItemData(group, new mmTreeItemData(
                new GeneralGroupReport(report_d.m_group_name),
                report_d.m_group_name
            ));
            group_name = report_d.m_group_name;
        }
        const ReportData* report_n = ReportModel::instance().get_idN_data_n(report_d.m_id);
        wxTreeItemId item = m_nav_tree_ctrl->AppendItem(
            no_group ? parent_item : group,
            wxGetTranslation(report_d.m_name),
            mmImage::img::CUSTOMSQL_PNG, mmImage::img::CUSTOMSQL_PNG
        );
        m_nav_tree_ctrl->SetItemData(item, new mmTreeItemData(
            new mmGeneralReport(report_n), report_n->m_name
        ));
    }

    // Update icons:
    applyGrmIconMapping(parent_item);
}

void mmFrame::DoUpdateFilterNavigation(wxTreeItemId& parent_item)
{

    wxArrayString filter_settings = InfoModel::instance().getArrayString("TRANSACTIONS_FILTER", true);
    for (const auto& data : filter_settings) {
        Document j_doc;
        if (j_doc.Parse(data.utf8_str()).HasParseError()) {
            j_doc.Parse("{}");
        }

        Value& j_label = GetValueByPointerWithDefault(j_doc, "/LABEL", "");
        const wxString& s_label = j_label.IsString() ? wxString::FromUTF8(j_label.GetString()) : "";

        wxTreeItemId item = m_nav_tree_ctrl->AppendItem(parent_item, s_label, mmImage::img::FILTER_PNG, mmImage::img::FILTER_PNG);
        m_nav_tree_ctrl->SetItemData(item, new mmTreeItemData(mmTreeItemData::FILTER_REPORT, data));
    }
}

void mmFrame::mmDoHideReportsDialog()
{
    wxString rep[] = {
        "Cash Flow",
        "Categories",
        "Forecast Report",
        "Income vs Expenses",
        "My Usage",
        "Payees",
        "Summary of Accounts",
        "Budgets",
        "Stocks Report",
    };

    wxArrayString stored_items = InfoModel::instance().getArrayString("HIDDEN_REPORTS");
    wxArrayInt hidden_reports;
    wxArrayString reports_name;
    wxArrayString reports_name_i10n;

    for (const auto& r : rep) {
        reports_name_i10n.Add(wxGetTranslation(r));
        reports_name.Add(r);
        if (stored_items.Index(r) != wxNOT_FOUND) {
            hidden_reports.Add(reports_name.Index(r));
        }
    }

    mmMultiChoice reports(this, _t("Hide"), _t("Reports"), reports_name_i10n);
    reports.SetSelections(hidden_reports);

    if (reports.ShowModal() == wxID_OK) {
        InfoModel::instance().saveString("HIDDEN_REPORTS", "[]");
        const auto sel = reports.GetSelections();
        for (const auto& i : sel) {
            const auto& report_name = reports_name[i];
            InfoModel::instance().prependArrayItem("HIDDEN_REPORTS", report_name, -1);
        }
    }
    DoRecreateNavTreeControl();
}

void mmFrame::loadGrmIconMapping()
{
    m_grm_icons_map.clear();

    rapidjson::Document doc;
    const wxString& json = InfoModel::instance().getString("GRM_REPORT_IMAGE_STATUS", "");
    doc.Parse(json.c_str());
    if (!doc.IsObject())
        return;

    const int prefix_len = static_cast<int>(_t("Reports").Len()) + 1;

    for (auto it = doc.MemberBegin(); it != doc.MemberEnd(); ++it) {
        std::string p = it->name.GetString();
        wxString path(p.substr(prefix_len).c_str(), wxConvUTF8);
        wxString timg = wxString::FromUTF8(it->value.GetString());
        m_grm_icons_map[path] = NavTreeIconImages::instance().getImgIndexFromStorageString(timg);
    }
}

void mmFrame::applyGrmIconMapping(wxTreeItemId& parent_item)
{
    for (auto const& x : m_grm_icons_map) {
        wxLogDebug ("Apply icon '%d' to path '%s'", x.second, x.first);

        std::vector<std::string> parts = GeneralReportManager::splitPath(x.first.ToStdString());
        wxTreeItemId current = parent_item;
        if (!parts.empty()) {
            for (size_t i = 0; i < parts.size(); ++i) {
                wxTreeItemId next = GeneralReportManager::findChild(m_nav_tree_ctrl, current, parts[i]);
                if (!next.IsOk()) {
                    current = wxTreeItemId();
                    break;
                }
                current = next;
            }
        }

        if (current.IsOk()) {
            m_nav_tree_ctrl->SetItemImage(current, x.second);
            m_nav_tree_ctrl->SetItemImage(current, x.second, wxTreeItemIcon_Selected);
        }
    }
}
