/*******************************************************
Copyright (C) 2006 Madhan Kanagavel
Copyright (C) 2011, 2012 Stefano Giorgio
Copyright (C) 2013, 2014 Nikolay Akimov
Copyright (C) 2021-2025 Mark Whalley (mark@ipx.co.uk)
Copyright (C) 2025 Klaus Wich

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

#ifndef MM_EX_CHECKINGPANEL_H_
#define MM_EX_CHECKINGPANEL_H_
//----------------------------------------------------------------------------
#include <wx/tglbtn.h>
#include "mmpanelbase.h"
#include "constants.h"
#include "journal.h"
#include "model/Model_Account.h"
#include <map>
//----------------------------------------------------------------------------
class mmCheckingPanel;
class mmFilterTransactionsDialog;
class mmGUIFrame;
class TransactionListCtrl;
//----------------------------------------------------------------------------

class mmCheckingPanel : public mmPanelBase
{
public:
    enum EIcons
    {
        ICON_UNRECONCILED,
        ICON_RECONCILED,
        ICON_VOID,
        ICON_FOLLOWUP,
        ICON_DUPLICATE,
        ICON_DESC,
        ICON_ASC,
    };

    enum FILTER_ID
    {
        FILTER_ID_DATE = 0,     // Not currently used
        FILTER_ID_DATE_RANGE,
        FILTER_ID_DATE_PICKER
    };

public:
    mmCheckingPanel(
        mmGUIFrame* frame,
        wxWindow* parent,
        int64 checking_id,
        const std::vector<int64> &group_ids = std::vector<int64>{}
    );

    ~mmCheckingPanel();

    bool isAllTrans() const;
    bool isDeletedTrans() const;
    bool isGroup() const;
    bool isAccount() const;

    void loadAccount(int64 account_id = -1);
    void refreshList();
    wxString BuildPage() const;
    void resetColumnView();
    void setSelectedTransaction(Journal::IdRepeat journal_id);
    void displaySplitCategories(Journal::IdB journal_id);

    //static support function
    static wxString getFilterName(FILTER_ID id);
    static void loadDateRanges(std::vector<DateRange2::Range>* date_range_a, int* date_range_m, bool all_ranges = false);
    double GetReconciledBalance() const;

private:
    friend class TransactionListCtrl;


    wxDECLARE_EVENT_TABLE();
    enum
    {
        ID_FILTER = wxID_HIGHEST + 50,
        ID_DATE_RANGE_MIN,
        ID_DATE_RANGE_MAX = ID_DATE_RANGE_MIN + 99,
        ID_FILTER_ADVANCED,
        ID_FILTER_TRANS,
        ID_DATE_PICKER_LOW,
        ID_DATE_PICKER_HIGH,
        ID_DATE_RANGE_EDIT,
        ID_SCHEDULED,
    };

    static const std::vector<std::pair<FILTER_ID, wxString> > FILTER_NAME;

private:
    // set by constructor or loadAccount()
    int64 m_checking_id = -1;
        //  1..   : single account with id m_checking_id
        // -1     : all transactions
        // -2     : deleted transactions
        // -3     : favorite accounts
        // -(4+X) : accounts of type X
    int64 m_account_id = -1;                    // applicable if m_checking_id >= 1
    std::set<int64> m_group_ids = {};           // applicable if m_checking_id <= -3
    Model_Account::Data* m_account = nullptr;   // non-null if m_checking_id >= 1
    Model_Currency::Data* m_currency = nullptr; // currency of m_account, or base currency
    std::vector<DateRange2::Range> m_date_range_a = {};
    int m_date_range_m = -1;

    // set by gui
    FILTER_ID m_filter_id;
    bool m_filter_advanced;
    DateRange2 m_current_date_range = DateRange2();
    bool m_scheduled_enable;
    bool m_scheduled_selected;

    // calculated by filterList(); applicable if isAccount()
    double m_flow = 0.0;
    double m_balance = 0.0;
    double m_reconciled_balance = 0.0;
    bool m_show_reconciled;

    // set by showTips()
    bool m_show_tips = false;

    bool m_use_account_specific_filter;

    mmGUIFrame* m_frame = nullptr;
    wxButton* m_bitmapTransFilter = nullptr;
    wxDatePickerCtrl* fromDateCtrl = nullptr;
    wxDatePickerCtrl* toDateCtrl = nullptr;
    wxButton* m_btnTransDetailFilter = nullptr;
    wxButton* m_btnNew = nullptr;
    wxButton* m_btnEdit = nullptr;
    wxButton* m_btnDuplicate = nullptr;
    wxButton* m_btnDelete = nullptr;
    wxButton* m_btnEnter = nullptr;
    wxButton* m_btnSkip = nullptr;
    wxButton* m_btnRestore = nullptr;
    wxBitmapButton* m_btnAttachment = nullptr;
    wxStaticText* m_header_text = nullptr;
    wxBitmapToggleButton* m_header_scheduled = nullptr;
    wxStaticText* m_header_sortOrder = nullptr;
    wxGauge* m_header_credit = nullptr;
    wxStaticText* m_header_balance = nullptr;
    wxStaticText* m_info_panel = nullptr;
    wxStaticText* m_info_panel_mini = nullptr;
    wxString m_info_panel_selectedbal;
    wxVector<wxBitmapBundle> m_images;
    TransactionListCtrl* m_lc = nullptr;
    wxSharedPtr<mmFilterTransactionsDialog> m_trans_filter_dlg;


private:
    bool create(
        wxWindow* parent,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = "mmCheckingPanel"
    );
    void createControls();
    void updateHeader();
    void updateFilter(bool firstinit = false);
    void updateFilterTooltip();
    void setFilterDate(DateRange2::Range& range);
    void setFilterAdvanced();
    void loadFilterSettings();
    void saveFilterSettings();
    void filterList();
    void sortList();
    void updateExtraTransactionData(bool single, int repeat_num, bool foreign);
    void enableButtons(bool edit, bool dup, bool del, bool enter, bool skip, bool attach);
    void showTips();
    void showTips(const wxString& tip);
    void updateScheduledEnable();
    void updateScheduledToolTip();
    void datePickProceed();

    void onFilterPopup(wxCommandEvent& event);
    void onFilterDate(wxCommandEvent& event);
    void onDatePickLow(wxDateEvent& event);
    void onDatePickHigh(wxDateEvent& event);
    void onFilterAdvanced(wxCommandEvent& event);
    void onEditDateRanges(wxCommandEvent& event);
    void onScheduled(wxCommandEvent& event);
    void onNewTransaction(wxCommandEvent& event);
    void onEditTransaction(wxCommandEvent& event);
    void onDeleteTransaction(wxCommandEvent& event);
    void onRestoreTransaction(wxCommandEvent& event);
    void onDuplicateTransaction(wxCommandEvent& event);
    void onMoveTransaction(wxCommandEvent& event);
    void onEnterScheduled(wxCommandEvent& event);
    void onSkipScheduled(wxCommandEvent& event);
    void onOpenAttachment(wxCommandEvent& event);
    void onSearchTxtEntered(wxCommandEvent& event);
    void onButtonRightDown(wxMouseEvent& event);
    void onInfoPanelClick(wxMouseEvent& event, wxStaticText* infoPanel);
    void onReconcile(wxCommandEvent& event);

    wxString getPanelTitle() const;
    static void mmPlayTransactionSound();
};

inline bool mmCheckingPanel::isAllTrans() const { return m_checking_id == -1; }
inline bool mmCheckingPanel::isDeletedTrans() const { return m_checking_id == -2; }
inline bool mmCheckingPanel::isGroup() const { return m_checking_id <= -3; }
inline bool mmCheckingPanel::isAccount() const { return m_checking_id >= 1; }
inline double mmCheckingPanel::GetReconciledBalance() const { return m_reconciled_balance;}

#endif // MM_EX_CHECKINGPANEL_H_
