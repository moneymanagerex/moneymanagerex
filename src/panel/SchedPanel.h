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

#pragma once

#include <vector>

#include "model/SchedModel.h"
#include "_PanelBase.h"
#include "SchedList.h"
#include "dialog/TrxFilterDialog.h"

class wxListEvent;

class SchedPanel : public PanelBase
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

public:
    SchedModel::DataExtA bills_;

private:
    wxDate m_today;
    bool transFilterActive_;
    wxArrayString tips_;

    wxSharedPtr<TrxFilterDialog> transFilterDlg_;
    SchedList* m_lc = nullptr;
    wxStaticText* m_infoText = nullptr;
    wxStaticText* m_infoTextMini = nullptr;
    wxButton* m_bitmapTransFilter = nullptr;

public:
    SchedPanel(
        wxWindow *parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = "SchedPanel" 
    );
    ~SchedPanel();

    /* Helper Functions */
    void updateBottomPanelData(int selIndex);
    void enableEditDeleteButtons(bool en);
    // updates the Repeating transactions panel data
    int initVirtualListControl(int64 id = -1);
    // Getter for Virtual List Control
    wxString getItem(long item, int col_id);
    void RefreshList();

    const wxString GetRemainingDays(const SchedData& item) const;

    wxString BuildPage() const;
    wxDate getToday() const { return m_today; }

private:
    void CreateControls();
    bool Create(
        wxWindow *parent, wxWindowID winid,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = "SchedPanel"
    );

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

    void OnFilterTransactions(wxCommandEvent& WXUNUSED(event));
};
