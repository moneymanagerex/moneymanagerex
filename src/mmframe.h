/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2012 Stefano Giorgio
 Copyright (C) 2013 Nikolay
 Copyright (C) 2014, 2017 James Higley
 Copyright (C) 2014 Guan Lisheng (guanlisheng@gmail.com)

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
#ifndef MM_FRAME_H_
#define MM_FRAME_H_
//----------------------------------------------------------------------------
#include <wx/aui/aui.h>
#include <vector>
#include "option.h"
#include "constants.h"
#include "util.h"
//----------------------------------------------------------------------------
class wxSQLite3Database;
class mmPrintableBase;
class mmPanelBase;
class mmHomePagePanel;
class mmTreeItemData;
class mmCheckingPanel;
class mmBudgetingPanel;
class mmBillsDepositsPanel;
class mmFileHistory;
class CommitCallbackHook;
class UpdateCallbackHook;
class ModelBase;
class mmGUIApp;
class wxToolBar;
//----------------------------------------------------------------------------

class mmGUIFrame : public wxFrame
{
public:
    mmGUIFrame(mmGUIApp* m_app, const wxString& title, const wxPoint& pos, const wxSize& size);
    ~mmGUIFrame();
public:
    mmGUIApp *m_app;
public:
    void setGotoAccountID(int account_id, long transID = -1);
    bool financialYearIsDifferent()
    {
        return (Option::instance().FinancialYearStartDay() != "1" ||
                Option::instance().FinancialYearStartMonth() != "1");
    }
    /// return the index (mmex::EDocFile) to return the correct file.
    int getHelpFileIndex() const
    {
        return helpFileIndex_;
    }

    void setAccountNavTreeSection(const wxString& accountName);
    bool setNavTreeSection(const wxString &sectionName);
    void menuPrintingEnable(bool enable);
    void OnToggleFullScreen(wxCommandEvent& WXUNUSED(event));
    void OnClose(wxCloseEvent&);
    std::vector<WebsiteNews> g_WebsiteNewsList;

    void RefreshNavigationTree();

private:
    std::vector<const ModelBase*> m_all_models;
private:
    /* handles to SQLite Database */
    wxSharedPtr<wxSQLite3Database> m_db;

    /* Currently open file name */
    wxString m_filename;

    int gotoAccountID_;
    int gotoTransID_;

    /* There are 2 kinds of reports */
    bool activeReport_;

    /* Repeat Transactions automatic processing delay */
    wxTimer autoRepeatTransactionsTimer_;
    void OnAutoRepeatTransactionsTimer(wxTimerEvent& event);

    /* controls */
    mmPanelBase* panelCurrent_;
    wxPanel* homePanel_;
    wxTreeCtrl* m_nav_tree_ctrl;
    wxMenuBar *menuBar_;
    wxToolBar* toolBar_;

    mmTreeItemData* selectedItemData_;

    wxTreeItemId getTreeItemfor(const wxTreeItemId& itemID, const wxString& accountName) const;
    bool setAccountInSection(const wxString& sectionName, const wxString& accountName);

    /* printing */
    int helpFileIndex_;

    /* wxAUI */
    wxAuiManager m_mgr;

    /* Homepage panel logic */

    void cleanup();
    void resetNavTreeControl();
    void cleanupNavTreeControl(wxTreeItemId& item);
    wxSizer* cleanupHomePanel(bool new_sizer = true);
    bool openFile(const wxString& fileName, const bool openingNew, const bool encrypt, const wxString &password = wxEmptyString);
    bool createDataStore(const wxString& fileName, const bool openingNew, const bool encrypt, const wxString &passwd);
    void InitializeModelTables();
    void createMenu();
    void createToolBar();
    void createReportsPage(mmPrintableBase* rb, bool cleanup);
    void createHelpPage();
    void refreshPanelData();

    mmHomePagePanel* homePage_;
    void createHomePage();
    mmCheckingPanel* checkingAccountPage_;
    void createCheckingAccountPage(int accountID);
    void createStocksAccountPage(int accountID);

    mmBillsDepositsPanel* billsDepositsPanel_;
    void createBillsDeposits();

    mmBudgetingPanel* budgetingPage_;
    void createBudgetingPage(int budgetYearID);

    void createControls();
    /*Set nav tree items status from JSON data with stored in DB*/
    void loadNavTreeItemsStatus();
    /*save Settings LASTFILENAME AUIPERSPECTIVE SIZES*/
    void saveSettings();
    void menuEnableItems(bool enable);
    void updateNavTreeControl();
    void updateReportNavigation(wxTreeItemId& reports, bool budget);
    void showTreePopupMenu(const wxTreeItemId& id, const wxPoint& pt);
    void showBeginAppDialog(bool fromScratch = false);
    void SetDataBaseParameters(const wxString& fileName);
    void OnLaunchAccountWebsite(wxCommandEvent& event);
    void OnAccountAttachments(wxCommandEvent& event);

    void OnNew(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnSetPassword(wxCommandEvent& event);
    void OnRemovePassword(wxCommandEvent& event);
    void OnVacuumDB(wxCommandEvent& event);
    void OnDebugDB(wxCommandEvent& event);
    void OnSaveAs(wxCommandEvent& event);
    void OnExportToCSV(wxCommandEvent& event);
    void OnExportToXML(wxCommandEvent& event);
    void OnExportToQIF(wxCommandEvent& event);
    void OnExportToHtml(wxCommandEvent& event);
    void OnExportToWebApp(wxCommandEvent& event);
    void OnImportUniversalCSV(wxCommandEvent& event);
    void OnImportXML(wxCommandEvent& event);
    void OnImportQIF(wxCommandEvent& event);
    void OnImportWebApp(wxCommandEvent& event);
    void OnPrintPage(wxCommandEvent& WXUNUSED(event));
    void OnQuit(wxCommandEvent& event);
    void OnBillsDeposits(wxCommandEvent& event);
    void OnAssets(wxCommandEvent& event);
    void OnGotoAccount(wxCommandEvent& WXUNUSED(event));
    void OnGotoStocksAccount(wxCommandEvent& WXUNUSED(event));

    bool m_hide_share_accounts;
    void OnHideShareAccounts(wxCommandEvent &event);
    void OnChangeGUILanguage(wxCommandEvent &event);

    void OnViewToolbar(wxCommandEvent &event);
    void OnViewLinks(wxCommandEvent &event);
    void OnViewBudgetFinancialYears(wxCommandEvent &event);
    void OnViewBudgetTransferTotal(wxCommandEvent &event);
    void OnViewBudgetSetupSummary(wxCommandEvent &event);
    void OnViewBudgetCategorySummary(wxCommandEvent &event);
    void OnViewIgnoreFutureTransactions(wxCommandEvent &event);
    void OnViewToolbarUpdateUI(wxUpdateUIEvent &event);
    void OnViewLinksUpdateUI(wxUpdateUIEvent &event);

    void OnNewAccount(wxCommandEvent& event);
    void OnAccountList(wxCommandEvent& event);
    void OnEditAccount(wxCommandEvent& event);
    void OnDeleteAccount(wxCommandEvent& event);
    void OnReallocateAccount(wxCommandEvent& event);

    void OnOrgCategories(wxCommandEvent& event);
    void OnOrgPayees(wxCommandEvent& event);
    void OnCategoryRelocation(wxCommandEvent& event);
    void OnPayeeRelocation(wxCommandEvent& event);
    void OnNewTransaction(wxCommandEvent& event);
    void refreshPanelData(wxCommandEvent& WXUNUSED(event));

    void OnOptions(wxCommandEvent& event);
    void OnBudgetSetupDialog(wxCommandEvent& event);
    void OnCurrency(wxCommandEvent& event);
    void OnTransactionReport(wxCommandEvent& event);
    void OnGeneralReportManager(wxCommandEvent& event);
    void OnCustomFieldsManager(wxCommandEvent& event);

    void OnHelp(wxCommandEvent& event);
    void OnShowAppStartDialog(wxCommandEvent& WXUNUSED(event));
    void OnCheckUpdate(wxCommandEvent& event);
    void OnBeNotified(wxCommandEvent& event);
    void OnSimpleURLOpen(wxCommandEvent& event);
    void OnReportBug(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    void OnItemRightClick(wxTreeEvent& event);
    void OnItemMenu(wxTreeEvent& event);
    void OnSelChanged(wxTreeEvent& event);
    void OnPopupDeleteAccount(wxCommandEvent& event);
    void OnPopupEditAccount(wxCommandEvent& event);
    void OnPopupReallocateAccount(wxCommandEvent& event);
    void OnPopupAccountBaseBalance(wxCommandEvent& event);

    void OnViewAccountsTemporaryChange(wxCommandEvent& event);

    void OnTreeItemExpanded(wxTreeEvent& event);
    void OnTreeItemCollapsed(wxTreeEvent& event);
    void navTreeStateToJson();

    void processPendingEvents();
    void ReallocateAccount(int accountID);

    /* Recent Files */
    mmFileHistory* m_recentFiles;
    wxMenu* m_menuRecentFiles;

    void OnRecentFiles(wxCommandEvent& event);
    void OnClearRecentFiles(wxCommandEvent& WXUNUSED(event));

    void OnHideShowReport(wxCommandEvent& event);

    /** Sets the database to the new database selected by the user */
    void SetDatabaseFile(const wxString& dbFileName, bool newDatabase = false, bool eencrypt = false, const wxString& password = wxEmptyString);
    
    // Required to prevent memory leaks.
    CommitCallbackHook* m_commit_callback_hook;
    UpdateCallbackHook* m_update_callback_hook;
    void ShutdownDatabase();

    // any class wishing to process wxWindows events must use this macro
    wxDECLARE_EVENT_TABLE();
    enum
    {
        /* Main Menu  */
        MENU_NEW = ID_MMEX_MAX + 1,
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
        MENU_CHECKUPDATE,
        MENU_IMPORT,
        MENU_IMPORT_UNIVCSV,
        MENU_IMPORT_XML,
        MENU_IMPORT_WEBAPP,
        MENU_ANNOUNCEMENTMAILING,
        MENU_FACEBOOK, // start range for OnSimpleURLOpen
        MENU_COMMUNITY,
        MENU_WEBSITE,
        MENU_WIKI,
        MENU_YOUTUBE,
        MENU_GITHUB,
        MENU_RSS,
        MENU_SLACK,
        MENU_DONATE,
        MENU_REPORTISSUES,
        MENU_BUY_COFFEE,
        MENU_GOOGLEPLAY,
        MENU_TWITTER, // end range for OnSimpleURLOpen
        MENU_REPORT_BUG,
        MENU_EXPORT_CSV,
        MENU_EXPORT_XML,
        MENU_EXPORT_QIF,
        MENU_EXPORT_WEBAPP,
        MENU_SHOW_APPSTART,
        MENU_EXPORT_HTML,
        MENU_CURRENCY,
        MENU_LANG,
        MENU_LANG_MAX = MENU_LANG + wxLANGUAGE_USER_DEFINED,

        MENU_IMPORT_MMNETCSV,
        MENU_IMPORT_QIF,
        MENU_ACCTEDIT,
        MENU_ACCTDELETE,
        MENU_TRANSACTIONREPORT,
        MENU_VIEW_TOOLBAR,
        MENU_VIEW_LINKS,
        MENU_VIEW_HIDE_SHARE_ACCOUNTS,
        MENU_CATEGORY_RELOCATION,
        MENU_PAYEE_RELOCATION,
        MENU_SET_PASSWORD,
        MENU_REMOVE_PASSWORD,
        MENU_DB_VACUUM,
        MENU_DB_DEBUG,
        MENU_ONLINE_UPD_CURRENCY_RATE,
        MENU_ACCOUNT_REALLOCATE,

        //magic munber needed to fix [bugs:#408]
        ID_NAVTREECTRL = MENU_ONLINE_UPD_CURRENCY_RATE + 233,

        MENU_TREEPOPUP_LAUNCHWEBSITE,
        MENU_TREEPOPUP_ACCOUNTATTACHMENTS,
        MENU_TREEPOPUP_NEW,
        MENU_TREEPOPUP_EDIT,
        MENU_TREEPOPUP_MOVE,
        MENU_TREEPOPUP_DELETE,
        MENU_TREEPOPUP_REALLOCATE,
        MENU_TREEPOPUP_ACCOUNT_BASE_BALANCE,

        //
        MENU_TREEPOPUP_ACCOUNT_NEW,
        MENU_TREEPOPUP_ACCOUNT_DELETE,
        MENU_TREEPOPUP_ACCOUNT_EDIT,
        MENU_TREEPOPUP_ACCOUNT_LIST,
        MENU_TREEPOPUP_ACCOUNT_EXPORT2CSV,
        MENU_TREEPOPUP_ACCOUNT_EXPORT2XML,
        MENU_TREEPOPUP_ACCOUNT_EXPORT2QIF,
        MENU_TREEPOPUP_ACCOUNT_IMPORTCSV,
        MENU_TREEPOPUP_ACCOUNT_IMPORTQIF,
        MENU_TREEPOPUP_ACCOUNT_IMPORTUNIVCSV,
        MENU_TREEPOPUP_ACCOUNT_IMPORTXML,
        MENU_TREEPOPUP_ACCOUNT_VIEWALL,
        MENU_TREEPOPUP_ACCOUNT_VIEWFAVORITE,
        MENU_TREEPOPUP_ACCOUNT_VIEWOPEN,
        MENU_TREEPOPUP_ACCOUNT_VIEWCLOSED,
        AUTO_REPEAT_TRANSACTIONS_TIMER_ID,

        MENU_TREEPOPUP_HIDE_SHOW_REPORT,
        MENU_TREEPOPUP_HIDE_SHOW_REPORT32 = MENU_TREEPOPUP_HIDE_SHOW_REPORT + 32, // Reserved space
    };
};
//----------------------------------------------------------------------------
#endif // MM_FRAME_H_
//----------------------------------------------------------------------------
