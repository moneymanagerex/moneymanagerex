/*******************************************************
Copyright (C) 2006 Madhan Kanagavel
Copyright (C) 2012 Stefano Giorgio
Copyright (C) 2013, 2022 Nikolay Akimov
Copyright (C) 2014 James Higley
Copyright (C) 2014 Guan Lisheng (guanlisheng@gmail.com)
Copyright (C) 2021, 2022 Mark Whalley (mark@ipx.co.uk)

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
#include <wx/toolbar.h>
#include <vector>
#include "option.h"
#include "constants.h"
#include "util.h"
#include "paths.h"

//----------------------------------------------------------------------------
class wxSQLite3Database;
class mmPrintableBase;
class mmPanelBase;
class mmHomePagePanel;
class mmTreeItemData;
class mmCheckingPanel;
class mmReportsPanel;
class mmStockPanel;
class mmBudgetingPanel;
class mmBillsDepositsPanel;
class mmFileHistory;
class CommitCallbackHook;
class UpdateCallbackHook;
class ModelBase;
class mmGUIApp;
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
    int getHelpFileIndex() const;
    void setHelpFileIndex();


    void setAccountNavTreeSection(const wxString& accountName);
    bool setNavTreeSection(const wxString &sectionName);
    void menuPrintingEnable(bool enable);
    void OnToggleFullScreen(wxCommandEvent& WXUNUSED(event));
    void OnClose(wxCloseEvent&);

    void RefreshNavigationTree();

private:
    std::vector<WebsiteNews> websiteNewsArray_;
    std::vector<const ModelBase*> m_all_models;

    /* handles to SQLite Database */
    wxSharedPtr<wxSQLite3Database> m_db;

    /* Currently open file name */
    wxString m_filename;
    wxString m_password;
    wxString m_temp_view;

    // Marker to indicate DB was inuse when opened and open cancelled
    bool db_lockInPlace;

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
    wxAuiToolBar* toolBar_;
private:
    mmTreeItemData* selectedItemData_;

    wxTreeItemId getTreeItemfor(const wxTreeItemId& itemID, const wxString& accountName) const;
    bool setAccountInSection(const wxString& sectionName, const wxString& accountName);

    /* printing */
    int helpFileIndex_;

    /* wxAUI */
    wxAuiManager m_mgr;

    /* Homepage panel logic */
    bool creditDisplayed_;

    void cleanup();
    void resetNavTreeControl();
    void cleanupNavTreeControl(wxTreeItemId& item);
    wxSizer* cleanupHomePanel(bool new_sizer = true);
    bool openFile(const wxString& fileName, bool openingNew, const wxString &password = "");
    void InitializeModelTables();
    bool createDataStore(const wxString& fileName, const wxString &passwd, bool openingNew);
    void createMenu();
    void CreateToolBar();
    void createReportsPage(mmPrintableBase* rb, bool cleanup);
    void createHelpPage(int index = mmex::HTML_INDEX);
    void refreshPanelData();

    void createHomePage();
    void createCheckingAccountPage(int accountID);
    void createAllTransactionsPage();
    void createStocksAccountPage(int accountID);
private:
    void createBillsDeposits();

    void createBudgetingPage(int budgetYearID);

    void createControls();
    /*Set nav tree items status from JSON data with stored in DB*/
    void loadNavigationTreeItemsStatusFromJson();
    /*save Settings LASTFILENAME AUIPERSPECTIVE SIZES*/
    void saveSettings();
    void menuEnableItems(bool enable);
    void DoRecreateNavTreeControl();
    void DoUpdateReportNavigation(wxTreeItemId& parent_item);
    void DoUpdateGRMNavigation(wxTreeItemId& parent_item);
    void DoUpdateFilterNavigation(wxTreeItemId& parent_item);
    void DoUpdateBudgetNavigation(wxTreeItemId& parent_item);
    void showTreePopupMenu(const wxTreeItemId& id, const wxPoint& pt);
    void AppendImportMenu(wxMenu& menu);
    void showBeginAppDialog(bool fromScratch = false);
    void SetDataBaseParameters(const wxString& fileName);
    void OnLaunchAccountWebsite(wxCommandEvent& event);
    void OnAccountAttachments(wxCommandEvent& event);
private:
    void OnNew(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnConvertEncryptedDB(wxCommandEvent& event);
    void OnChangeEncryptPassword(wxCommandEvent& event);
    void OnVacuumDB(wxCommandEvent& event);
    void OnDebugDB(wxCommandEvent& event);
    void OnSaveAs(wxCommandEvent& event);
    void OnExportToCSV(wxCommandEvent& event);
    void OnExportToXML(wxCommandEvent& event);
    void OnExportToQIF(wxCommandEvent& event);
    void OnExportToJSON(wxCommandEvent& event);
    void OnExportToMMEX(wxCommandEvent& event);
    void OnExportToHtml(wxCommandEvent& event);
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
private:
    void OnHideShareAccounts(wxCommandEvent &event);
    void OnChangeGUILanguage(wxCommandEvent &event);
    void OnKeyDown(wxTreeEvent& event);

    void OnViewToolbar(wxCommandEvent &event);
    void OnViewLinks(wxCommandEvent &event);
    void OnViewBudgetFinancialYears(wxCommandEvent& WXUNUSED(event));
    void OnViewBudgetTransferTotal(wxCommandEvent& WXUNUSED(event));
    void OnViewBudgetCategorySummary(wxCommandEvent& WXUNUSED(event));
    void OnViewIgnoreFutureTransactions(wxCommandEvent& WXUNUSED(event));
    void OnViewShowToolTips(wxCommandEvent& WXUNUSED(event));
    void OnViewShowMoneyTips(wxCommandEvent& WXUNUSED(event));
    void OnViewToolbarUpdateUI(wxUpdateUIEvent &event);
    void OnViewLinksUpdateUI(wxUpdateUIEvent &event);

    void OnNewAccount(wxCommandEvent& event);
    void OnAccountList(wxCommandEvent& event);
    void OnEditAccount(wxCommandEvent& event);
    void OnDeleteAccount(wxCommandEvent& event);
    void OnReallocateAccount(wxCommandEvent& event);

    void OnPopupEditFilter(wxCommandEvent& event);
    void OnPopupRenameFilter(wxCommandEvent& event);
    void OnPopupDeleteFilter(wxCommandEvent& event);   

private:
    void OnOrgCategories(wxCommandEvent& event);
    void OnOrgPayees(wxCommandEvent& event);
    void OnCategoryRelocation(wxCommandEvent& event);
    void OnPayeeRelocation(wxCommandEvent& event);
    void OnNewTransaction(wxCommandEvent& event);
    void refreshPanelData(wxCommandEvent& /*event*/);

    void OnOptions(wxCommandEvent& event);
    void OnBudgetSetupDialog(wxCommandEvent& event);
    void OnCurrency(wxCommandEvent& event);
    void OnRates(wxCommandEvent& event);
    void OnTransactionReport(wxCommandEvent& event);
    void OnCustomFieldsManager(wxCommandEvent& event);
    void OnGeneralReportManager(wxCommandEvent& event);
    void OnThemeManager(wxCommandEvent& event);
    void OnRefreshWebApp(wxCommandEvent& event);

    void OnHelp(wxCommandEvent& event);
    void OnShowAppStartDialog(wxCommandEvent& WXUNUSED(event));
    void OnCheckUpdate(wxCommandEvent& event);
    void OnBeNotified(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnSimpleURLOpen(wxCommandEvent& event);
    void OnReportBug(wxCommandEvent& event);
    void OnDiagnostics(wxCommandEvent& event);
private:
    void OnItemRightClick(wxTreeEvent& event);
    void OnItemMenu(wxTreeEvent& event);
    void OnSelChanged(wxTreeEvent& event);
    void OnPopupDeleteAccount(wxCommandEvent& event);
    void OnPopupEditAccount(wxCommandEvent& event);
    void OnPopupReallocateAccount(wxCommandEvent& event);

    void OnViewAccountsTemporaryChange(wxCommandEvent& event);

    void OnTreeItemExpanded(wxTreeEvent& event);
    void OnTreeItemCollapsing(wxTreeEvent& event);
    void OnTreeItemCollapsed(wxTreeEvent& event);

    void navTreeStateToJson();
    void processPendingEvents();
    void ReallocateAccount(int accountID);
    void mmDoHideReportsDialog();
private:
    /* Recent Files */
    wxSharedPtr<mmFileHistory> m_recentFiles;
    wxMenu* m_menuRecentFiles;

    void OnRecentFiles(wxCommandEvent& event);
    void OnClearRecentFiles(wxCommandEvent& /*event*/);

    /** Sets the database to the new database selected by the user */
    void SetDatabaseFile(const wxString& dbFileName, bool newDatabase = false);

    // Required to prevent memory leaks.
    CommitCallbackHook* m_commit_callback_hook;
    UpdateCallbackHook* m_update_callback_hook;
    void ShutdownDatabase();
private:
    // any class wishing to process wxWindows events must use this macro
    wxDECLARE_EVENT_TABLE();
    enum
    {
        /* Main Menu  */
        MENU_NEW = mmID_MAX + 1,
        MENU_OPEN,
        MENU_SAVE,
        MENU_SAVE_AS,
        MENU_RECENT_FILES,
        MENU_RECENT_FILES_0,
        MENU_RECENT_FILES_CLEAR,
        MENU_EXPORT,
        MENU_NEWACCT,
        MENU_HOMEPAGE,
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
        MENU_CROWDIN,
        MENU_REPORTISSUES,
        MENU_BUY_COFFEE,
        MENU_GOOGLEPLAY,
        MENU_TWITTER, // end range for OnSimpleURLOpen
        MENU_EXPORT_CSV,
        MENU_EXPORT_MMEX,
        MENU_EXPORT_XML,
        MENU_EXPORT_QIF,
        MENU_EXPORT_JSON,
        MENU_SHOW_APPSTART,
        MENU_EXPORT_HTML,
        MENU_CURRENCY,
        MENU_RATES,
        MENU_LANG,
        MENU_LANG_MAX = MENU_LANG + wxLANGUAGE_USER_DEFINED,

        MENU_IMPORT_MMNETCSV,
        MENU_IMPORT_QIF,
        MENU_ACCTEDIT,
        MENU_ACCTDELETE,
        MENU_TRANSACTIONREPORT,
        MENU_REFRESH_WEBAPP,
        MENU_VIEW_TOOLBAR,
        MENU_VIEW_LINKS,
        MENU_VIEW_HIDE_SHARE_ACCOUNTS,
        MENU_CATEGORY_RELOCATION,
        MENU_PAYEE_RELOCATION,
        MENU_RELOCATION,
        MENU_THEME_MANAGER,
        MENU_CONVERT_ENC_DB,
        MENU_CHANGE_ENCRYPT_PASSWORD,
        MENU_DB_VACUUM,
        MENU_DB_DEBUG,
        MENU_ONLINE_UPD_CURRENCY_RATE,
        MENU_ACCOUNT_REALLOCATE,
        MENU_DIAGNOSTICS,

        //magic munber needed to fix [bugs:#408]
        ID_NAVTREECTRL = MENU_ONLINE_UPD_CURRENCY_RATE + 233,

        MENU_TREEPOPUP_LAUNCHWEBSITE,
        MENU_TREEPOPUP_ACCOUNTATTACHMENTS,
        MENU_TREEPOPUP_NEW,
        MENU_TREEPOPUP_EDIT,
        MENU_TREEPOPUP_MOVE,
        MENU_TREEPOPUP_DELETE,
        MENU_TREEPOPUP_REALLOCATE,

        // Transaction Report Filter
        MENU_TREEPOPUP_FILTER_EDIT,
        MENU_TREEPOPUP_FILTER_RENAME,
        MENU_TREEPOPUP_FILTER_DELETE,

        //
        MENU_TREEPOPUP_ACCOUNT_NEW,
        MENU_TREEPOPUP_ACCOUNT_DELETE,
        MENU_TREEPOPUP_ACCOUNT_EDIT,
        MENU_TREEPOPUP_ACCOUNT_LIST,
        MENU_TREEPOPUP_ACCOUNT_EXPORT2CSV,
        MENU_TREEPOPUP_ACCOUNT_EXPORT2MMEX,
        MENU_TREEPOPUP_ACCOUNT_EXPORT2XML,
        MENU_TREEPOPUP_ACCOUNT_EXPORT2QIF,
        MENU_TREEPOPUP_ACCOUNT_EXPORT2JSON,
        MENU_TREEPOPUP_ACCOUNT_IMPORTCSV,
        MENU_TREEPOPUP_ACCOUNT_IMPORTQIF,
        MENU_TREEPOPUP_ACCOUNT_IMPORTUNIVCSV,
        MENU_TREEPOPUP_ACCOUNT_IMPORTXML,
        MENU_TREEPOPUP_ACCOUNT_VIEWALL,
        MENU_TREEPOPUP_ACCOUNT_VIEWFAVORITE,
        MENU_TREEPOPUP_ACCOUNT_VIEWOPEN,
        MENU_TREEPOPUP_ACCOUNT_VIEWCLOSED,
        AUTO_REPEAT_TRANSACTIONS_TIMER_ID,
    };
};


inline int mmGUIFrame::getHelpFileIndex() const { return helpFileIndex_; }
inline void mmGUIFrame::setHelpFileIndex() { helpFileIndex_ = mmex::EDocFile::HTML_INDEX; }
//----------------------------------------------------------------------------
#endif // MM_FRAME_H_
//----------------------------------------------------------------------------
