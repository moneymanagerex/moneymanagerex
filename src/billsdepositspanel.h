/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

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

#ifndef MM_EX_BILLSDEPOSITSPANEL_H_
#define MM_EX_BILLSDEPOSITSPANEL_H_

#include "filtertransdialog.h"
#include "mmpanelbase.h"
#include "model/Model_Billsdeposits.h"
#include <vector>

class wxListEvent;
class mmBillsDepositsPanel;

/* Custom ListCtrl class that implements virtual LC style */
class billsDepositsListCtrl: public mmListCtrl
{
    DECLARE_NO_COPY_CLASS(billsDepositsListCtrl)
    wxDECLARE_EVENT_TABLE();

public:
    enum LISTCOL_ID
    {
        LISTCOL_ID_ICON = 0,
        LISTCOL_ID_ID,
        LISTCOL_ID_PAYMENT_DATE,
        LISTCOL_ID_DUE_DATE,
        LISTCOL_ID_ACCOUNT,
        LISTCOL_ID_PAYEE,
        LISTCOL_ID_STATUS,
        LISTCOL_ID_CATEGORY,
        LISTCOL_ID_TAGS,
        LISTCOL_ID_TYPE,
        LISTCOL_ID_AMOUNT,
        LISTCOL_ID_FREQUENCY,
        LISTCOL_ID_REPEATS,
        LISTCOL_ID_AUTO,
        LISTCOL_ID_DAYS,
        LISTCOL_ID_NUMBER,
        LISTCOL_ID_NOTES,
        LISTCOL_ID_size, // number of columns
    };

private:
    static const std::vector<ListColumnInfo> LISTCOL_INFO;
    mmBillsDepositsPanel* m_bdp;
    long m_selected_row = -1;

public:
    billsDepositsListCtrl(mmBillsDepositsPanel* bdp, wxWindow *parent, wxWindowID winid = wxID_ANY);
    ~billsDepositsListCtrl();

    void OnNewBDSeries(wxCommandEvent& event);
    void OnEditBDSeries(wxCommandEvent& event);
    void OnDuplicateBDSeries(wxCommandEvent& event);
    void OnDeleteBDSeries(wxCommandEvent& event);
    void OnEnterBDTransaction(wxCommandEvent& event);
    void OnSkipBDTransaction(wxCommandEvent& event);
    void OnOpenAttachment(wxCommandEvent& event);
    void OnOrganizeAttachments(wxCommandEvent& event);
    void RefreshList();

protected:
    virtual void OnColClick(wxListEvent& event);
    virtual wxListItemAttr *OnGetItemAttr(long item) const;

private:
    static int col_sort();
    void refreshVisualList(int selected_index = -1);

    /* required overrides for virtual style list control */
    virtual wxString OnGetItemText(long item, long col_nr) const;
    virtual int OnGetItemImage(long item) const;

    void OnItemRightClick(wxMouseEvent& event);
    void OnListLeftClick(wxMouseEvent& event);
    void OnListItemActivated(wxListEvent& event);
    void OnMarkTransaction(wxCommandEvent& event);
    void OnMarkAllTransactions(wxCommandEvent& event);
    void OnListKeyDown(wxListEvent& event);
    void OnListItemSelected(wxListEvent& event);
    void OnSetUserColour(wxCommandEvent& event);
};

class mmBillsDepositsPanel : public mmPanelBase
{
    wxDECLARE_EVENT_TABLE();

public:

    enum EIcons
    {
        ICON_FOLLOWUP,
        ICON_RUN_AUTO,
        ICON_RUN,
        ICON_UPARROW,
        ICON_DOWNARROW
    };

    mmBillsDepositsPanel(wxWindow *parent
        , wxWindowID winid = wxID_ANY
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxDefaultSize
        , long style = wxTAB_TRAVERSAL | wxNO_BORDER
        , const wxString& name = "mmBillsDepositsPanel" 
    );
    ~mmBillsDepositsPanel();

    /* Helper Functions/data */
    Model_Billsdeposits::Full_Data_Set bills_;
    void updateBottomPanelData(int selIndex);
    void enableEditDeleteButtons(bool en);
    /* updates the Repeating transactions panel data */
    int initVirtualListControl(int64 id = -1);
    /* Getter for Virtual List Control */
    wxString getItem(long item, int col_id);
    void RefreshList();

    const wxString GetFrequency(const Model_Billsdeposits::Data* item) const;
    int GetNumRepeats(const Model_Billsdeposits::Data* item) const;
    const wxString GetRemainingDays(const Model_Billsdeposits::Data* item) const;

    wxString BuildPage() const;
    wxDate getToday() const;

    void do_delete_custom_values(int64 id);

private:
    void CreateControls();
    bool Create(wxWindow *parent, wxWindowID winid,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = "mmBillsDepositsPanel");

    /* Event handlers for Buttons */
    void OnNewBDSeries(wxCommandEvent& event);
    void OnEditBDSeries(wxCommandEvent& event);
    void OnDuplicateBDSeries(wxCommandEvent& event);
    void OnDeleteBDSeries(wxCommandEvent& event);

    void OnEnterBDTransaction(wxCommandEvent& event);
    void OnSkipBDTransaction(wxCommandEvent& event);
    void OnOpenAttachment(wxCommandEvent& event);

    //void OnViewPopupSelected(wxCommandEvent& event);

    void sortList();
    wxString tips();

private:
    wxSharedPtr<mmFilterTransactionsDialog> transFilterDlg_;
    billsDepositsListCtrl* m_lc = nullptr;
    wxStaticText* m_infoText = nullptr;
    wxStaticText* m_infoTextMini = nullptr;
    wxDate m_today;

    bool transFilterActive_;
    void OnFilterTransactions(wxCommandEvent& WXUNUSED(event));
    wxButton* m_bitmapTransFilter = nullptr;

    wxArrayString tips_;
};

inline int billsDepositsListCtrl::col_sort() { return LISTCOL_ID_PAYMENT_DATE; }

inline wxDate mmBillsDepositsPanel::getToday() const { return m_today; }

#endif
