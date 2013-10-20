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

#ifndef _MM_EX_BUDGETINGPANEL_H_
#define _MM_EX_BUDGETINGPANEL_H_

#include "mmpanelbase.h"
#include "util.h"
#include "model/Model_Budget.h"

class wxListCtrl;
class wxListEvent;
class mmBudgetingPanel;

/* Custom ListCtrl class that implements virtual LC style */
class budgetingListCtrl: public wxListCtrl
{
    DECLARE_NO_COPY_CLASS(budgetingListCtrl)
    DECLARE_EVENT_TABLE()

public:
    budgetingListCtrl(mmBudgetingPanel* cp, wxWindow *parent,
                      const wxWindowID id, const wxPoint& pos,
                      const wxSize& size, long style)
        : wxListCtrl(parent, id, pos, size, style),
          attr1_(mmColors::listBorderColor, mmColors::listAlternativeColor0, wxNullFont),
          attr2_(mmColors::listBorderColor, mmColors::listAlternativeColor1, wxNullFont),
          attr3_(mmColors::listAlternativeColor1, mmColors::listFutureDateColor, wxNullFont),

          cp_(cp),
          selectedIndex_(-1)
    {}

public:
    /* required overrides for virtual style list control */
    virtual wxString OnGetItemText(long item, long column) const;
    virtual wxListItemAttr *OnGetItemAttr(long item) const;
    virtual int OnGetItemImage(long item) const;

    void OnListItemSelected(wxListEvent& event);
    void OnListItemActivated(wxListEvent& event);

private:
    void OnItemResize(wxListEvent& event);
    wxListItemAttr attr1_; // style1
    wxListItemAttr attr2_; // style2
    wxListItemAttr attr3_; // style3
    mmBudgetingPanel* cp_;
    long selectedIndex_;
};

// TODO
struct mmBudgetEntryHolder: public mmHolderBase
{
    int categID_;
    wxString catStr_;

    int subcategID_;
    wxString subCatStr_;

    wxString period_;

    wxString amtString_;
    double amt_;

    wxString estimatedStr_;
    double estimated_;

    wxString actualStr_;
    double actual_;
};

class mmBudgetingPanel : public mmPanelBase
{
    DECLARE_EVENT_TABLE()

public:
    mmBudgetingPanel( int budgetYearID, wxWindow *parent,
                      wxWindowID winid = wxID_ANY,
                      const wxPoint& pos = wxDefaultPosition,
                      const wxSize& size = wxDefaultSize,
                      long style = wxTAB_TRAVERSAL | wxNO_BORDER,
                      const wxString& name = wxPanelNameStr );
    ~mmBudgetingPanel();

    /* updates the checking panel data */
    void initVirtualListControl();
    void save_column_width(int width);

    /* Getter for Virtual List Control */
    wxString getItem(long item, long column);

    void DisplayBudgetingDetails(int budgetYearID);
    int GetBudgetYearID()
    {
        return budgetYearID_;
    }
    wxString GetCurrentView()
    {
        return currentView_;
    }
    int GetItemImage(long item) const;
    void OnListItemActivated(int selectedIndex);
    int GetTransID(long item)
    {
        return trans_[item].id_;
    }

    void RefreshList();

private:
    std::vector<mmBudgetEntryHolder> trans_;
    Model_Budget::Data_Set m_trans;
    budgetingListCtrl* listCtrlBudget_;
    wxString currentView_;
    int budgetYearID_;
    wxImageList* m_imageList;
    wxStaticText* budgetReportHeading_;
    wxStaticText* income_estimated_;
    wxStaticText* income_actual_;
    wxStaticText* income_diff_;
    wxStaticText* expences_estimated_;
    wxStaticText* expences_actual_;
    wxStaticText* expences_diff_;

    bool Create( wxWindow *parent, wxWindowID winid,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxTAB_TRAVERSAL | wxNO_BORDER,
                 const wxString& name = wxPanelNameStr);

    void CreateControls();
    void sortTable();
    bool DisplayEntryAllowed(mmBudgetEntryHolder& budgetEntry);
    void UpdateBudgetHeading();

    /* Event handlers for Buttons */
    void OnViewPopupSelected(wxCommandEvent& event);
    void OnMouseLeftDown( wxMouseEvent& event );
};

#endif

