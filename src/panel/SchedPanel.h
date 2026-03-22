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
    friend class SchedList;

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

private:
    SchedModel::DataExtA m_sched_xa;
    mmDate m_today;
    bool m_filter_active;
    wxArrayString m_tip_a;

    wxSharedPtr<TrxFilterDialog> w_filter_dlg;
    SchedList*    w_list       = nullptr;
    wxStaticText* w_info_text  = nullptr;
    wxStaticText* w_mini_text  = nullptr;
    wxButton*     w_filter_btn = nullptr;

public:
    SchedPanel(
        wxWindow* parent_win,
        wxWindowID win_id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = "SchedPanel" 
    );
    ~SchedPanel();

    // override PanelBase
    virtual auto buildPage() const -> wxString override {
        return w_list->buildPage(_t("Scheduled Transactions"));
    }
    virtual void sortList() override;

    void refreshList() { w_list->refreshList(); }

private:
    bool create(
        wxWindow* parent_win,
        wxWindowID win_id,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = "SchedPanel"
    );
    void createControls();
    auto getItem(long item, int col_id) -> wxString;
    int  initList(int64 sched_id_n = -1);
    auto getRemainingDays(const SchedData& sched_d) const -> const wxString;
    auto getRandomTip() -> wxString;
    void updateBottomPanelData(int selIndex);
    void enableEditDeleteButtons(bool en);

    void onNewBDSeries(wxCommandEvent& event) { w_list->onNewBDSeries(event); }
    void onEditBDSeries(wxCommandEvent& event) { w_list->onEditBDSeries(event); }
    void onDuplicateBDSeries(wxCommandEvent& event) { w_list->onDuplicateBDSeries(event); }
    void onDeleteBDSeries(wxCommandEvent& event) { w_list->onDeleteBDSeries(event); }
    void onEnterBDTransaction(wxCommandEvent& event) { w_list->onEnterBDTransaction(event); }
    void onSkipBDTransaction(wxCommandEvent& event) {
        w_list->onSkipBDTransaction(event);
        w_list->SetFocus();
    }
    void onOpenAttachment(wxCommandEvent& event) {
        w_list->onOpenAttachment(event);
        w_list->SetFocus();
    }
    void onFilterTransactions(wxCommandEvent& WXUNUSED(event));
};
