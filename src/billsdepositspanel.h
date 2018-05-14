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

#ifndef MM_EX_BILLSDEPOSITSPANEL_H_
#define MM_EX_BILLSDEPOSITSPANEL_H_

#include "mmpanelbase.h"
#include "Model_Billsdeposits.h"
class wxListEvent;
class mmBillsDepositsPanel;
class mmFilterTransactionsDialog;
class wxStaticText;
class wxStaticBitmap;

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
    void OnDeleteBDSeries(wxCommandEvent& event);
    void OnEnterBDTransaction(wxCommandEvent& event);
    void OnSkipBDTransaction(wxCommandEvent& event);
    void OnOpenAttachment(wxCommandEvent& event);
    void OnOrganizeAttachments(wxCommandEvent& event);
    void RefreshList();

protected:
    virtual void OnColClick(wxListEvent& event);

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

    void refreshVisualList(int selected_index = -1);

    mmBillsDepositsPanel* m_bdp;
};

class mmBillsDepositsPanel : public mmPanelBase
{
    wxDECLARE_EVENT_TABLE();

public:
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
    int getColumnsNumber() { return COL_MAX; }
    int col_sort() { return COL_PAYMENT_DATE; }

    static wxString GetFrequency(const Model_Billsdeposits::Data* item);
    static wxString GetRemainingDays(const Model_Billsdeposits::Data* item);

    wxString BuildPage() const;

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
    void OnDeleteBDSeries(wxCommandEvent& event);

    void OnEnterBDTransaction(wxCommandEvent& event);
    void OnSkipBDTransaction(wxCommandEvent& event);
    void OnOpenAttachment(wxCommandEvent& event);

    void OnViewPopupSelected(wxCommandEvent& event);

    void sortTable();
    wxString tips();

private:
    wxImageList* m_imageList;
    billsDepositsListCtrl* listCtrlAccount_;
    wxStaticText* m_infoText;
    wxStaticText* m_infoTextMini;

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

    bool transFilterActive_;
    void OnFilterTransactions(wxMouseEvent& event);
    mmFilterTransactionsDialog* transFilterDlg_;
    wxStaticBitmap* bitmapTransFilter_;

    wxArrayString tips_;
};
#endif

