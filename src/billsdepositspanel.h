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

#ifndef _MM_EX_BILLSDEPOSITSPANEL_H_
#define _MM_EX_BILLSDEPOSITSPANEL_H_

#include "guiid.h"
#include "filtertransdialog.h"
#include "mmpanelbase.h"
#include "util.h"
#include "model/Model_Billsdeposits.h"
#include <vector>

class wxListEvent;
class mmBillsDepositsPanel;

/* Custom ListCtrl class that implements virtual LC style */
class billsDepositsListCtrl: public mmListCtrl
{
    DECLARE_NO_COPY_CLASS(billsDepositsListCtrl)
    DECLARE_EVENT_TABLE()

public:
    billsDepositsListCtrl(mmBillsDepositsPanel* cp, wxWindow *parent, wxWindowID winid = wxID_ANY);
    ~billsDepositsListCtrl();

    void OnNewBDSeries(wxCommandEvent& event);
    void OnEditBDSeries(wxCommandEvent& event);
    void OnDeleteBDSeries(wxCommandEvent& event);
    void OnEnterBDTransaction(wxCommandEvent& event);
    void OnSkipBDTransaction(wxCommandEvent& event);
    void RefreshList();

private:
    /* required overrides for virtual style list control */
    virtual wxString OnGetItemText(long item, long column) const;
    virtual int OnGetItemImage(long item) const;

    void OnItemRightClick(wxListEvent& event);
    void OnListItemActivated(wxListEvent& event);
    void OnMarkTransaction(wxCommandEvent& event);
    void OnMarkAllTransactions(wxCommandEvent& event);
    void OnListKeyDown(wxListEvent& event);
    void OnListItemSelected(wxListEvent& event);
    void OnListItemDeselected(wxListEvent& event);
    void OnItemResize(wxListEvent& event);
    void OnColClick(wxListEvent& event);

    void refreshVisualList(int selected_index = -1);

    mmBillsDepositsPanel* cp_;
};

class mmBillsDepositsPanel : public mmPanelBase
{
    DECLARE_EVENT_TABLE()

public:
    mmBillsDepositsPanel(
        wxWindow *parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = wxPanelNameStr
    );
    ~mmBillsDepositsPanel();

    bool Create( wxWindow *parent, wxWindowID winid,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxTAB_TRAVERSAL | wxNO_BORDER,
                 const wxString& name = wxPanelNameStr);
    /* Helper Functions/data */
    Model_Billsdeposits::Full_Data_Set bills_;
    void updateBottomPanelData(int selIndex);
    /* updates the Repeating transactions panel data */
    int initVirtualListControl(int id = -1);
    /* Getter for Virtual List Control */
    wxString getItem(long item, long column);
    void RefreshList();
    int getColumnsNumber() { return ColName_.size(); }

    static wxString GetFrequency(const Model_Billsdeposits::Data* item);
    static wxString GetRemainingDays(const Model_Billsdeposits::Data* item);

private:
    void CreateControls();

    /* Event handlers for Buttons */
    void OnNewBDSeries(wxCommandEvent& event);
    void OnEditBDSeries(wxCommandEvent& event);
    void OnDeleteBDSeries(wxCommandEvent& event);

    void OnEnterBDTransaction(wxCommandEvent& event);
    void OnSkipBDTransaction(wxCommandEvent& event);

    void enableEditDeleteButtons(bool en);

    void OnViewPopupSelected(wxCommandEvent& event);

    void sortTable();
    wxString tips();

private:
    wxImageList* m_imageList;
    billsDepositsListCtrl* listCtrlAccount_;

    enum EColumn
    {
        COL_PAYEE = 0,
        COL_ACCOUNT,
        COL_TYPE,
        COL_AMOUNT,
        COL_DUE_DATE,
        COL_FREQUENCY,
        COL_DAYS,
        COL_NOTES,
        COL_MAX, // number of columns
    };
    std::map<int, wxString> ColName_;

    bool transFilterActive_;
    void OnFilterTransactions(wxMouseEvent& event);
    mmFilterTransactionsDialog* transFilterDlg_;
    wxStaticBitmap* bitmapTransFilter_;

    wxArrayString tips_;
};
#endif

