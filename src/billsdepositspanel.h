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
    /* required overrides for virtual style list control */
    virtual wxString OnGetItemText(long item, long column) const;
    virtual int OnGetItemImage(long item) const;

    void OnItemRightClick(wxMouseEvent& event);
    void OnListLeftClick(wxMouseEvent& event);
    void OnListItemActivated(wxListEvent& event);
    void OnMarkTransaction(wxCommandEvent& event);
    void OnMarkAllTransactions(wxCommandEvent& event);
    void OnListKeyDown(wxListEvent& event);
    void OnListItemSelected(wxListEvent& event);
    void OnSetUserColour(wxCommandEvent& event);

    void refreshVisualList(int selected_index = -1);

    mmBillsDepositsPanel* m_bdp;
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

    enum EColumn
    {
        COL_ICON = 0,
        COL_ID,
        COL_PAYMENT_DATE,
        COL_DUE_DATE,
        COL_ACCOUNT,
        COL_PAYEE,
        COL_STATUS,
        COL_CATEGORY,
        COL_TAGS,
        COL_TYPE,
        COL_AMOUNT,
        COL_FREQUENCY,
        COL_REPEATS,
        COL_AUTO,
        COL_DAYS,
        COL_NUMBER,
        COL_NOTES,
        COL_MAX, // number of columns
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
    int initVirtualListControl(int id = -1);
    /* Getter for Virtual List Control */
    wxString getItem(long item, long column);
    void RefreshList();
    int getColumnsNumber();
    int col_sort();

    const wxString GetFrequency(const Model_Billsdeposits::Data* item) const;
    int GetNumRepeats(const Model_Billsdeposits::Data* item) const;
    const wxString GetRemainingDays(const Model_Billsdeposits::Data* item) const;

    wxString BuildPage() const;
    wxDate getToday() const;

    void do_delete_custom_values(int id);

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

    void sortTable();
    wxString tips();

private:
    wxSharedPtr<mmFilterTransactionsDialog> transFilterDlg_;
    billsDepositsListCtrl* listCtrlAccount_ = nullptr;
    wxStaticText* m_infoText = nullptr;
    wxStaticText* m_infoTextMini = nullptr;
    wxDate m_today;

    bool transFilterActive_;
    void OnFilterTransactions(wxCommandEvent& WXUNUSED(event));
    wxButton* m_bitmapTransFilter = nullptr;

    wxArrayString tips_;
};

inline wxDate mmBillsDepositsPanel::getToday() const { return m_today; }
inline int mmBillsDepositsPanel::getColumnsNumber() { return COL_MAX; }
inline int mmBillsDepositsPanel::col_sort() { return COL_PAYMENT_DATE; }
#endif

