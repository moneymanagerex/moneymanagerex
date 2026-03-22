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
#include "_ListBase.h"
#include "dialog/TrxFilterDialog.h"

class wxListEvent;
class SchedPanel;

/* Custom ListCtrl class that implements virtual LC style */
class SchedList: public ListBase
{
    friend class SchedPanel;

    DECLARE_NO_COPY_CLASS(SchedList)
    wxDECLARE_EVENT_TABLE();

public:
    enum LIST_ID
    {
        LIST_ID_ICON = 0,
        LIST_ID_ID,
        LIST_ID_PAYMENT_DATE,
        LIST_ID_DUE_DATE,
        LIST_ID_ACCOUNT,
        LIST_ID_PAYEE,
        LIST_ID_STATUS,
        LIST_ID_CATEGORY,
        LIST_ID_TAGS,
        LIST_ID_WITHDRAWAL,
        LIST_ID_DEPOSIT,
        LIST_ID_FREQUENCY,
        LIST_ID_REPEATS,
        LIST_ID_AUTO,
        LIST_ID_REMAINING,
        LIST_ID_NUMBER,
        LIST_ID_NOTES,
        LIST_ID_size, // number of columns
    };

private:
    static const std::vector<ListColumnInfo> LIST_INFO;
    long m_select_n = -1;

    SchedPanel* w_panel;

public:
    SchedList(
        SchedPanel* panel,
        wxWindow* parent_win,
        wxWindowID win_id = wxID_ANY
    );
    ~SchedList();

    // override ListBase
    virtual void onColClick(wxListEvent& event) override;
    virtual int  getSortIcon(bool asc) const override;

    // override wxListCtrl
    virtual auto OnGetItemAttr(long item) const -> wxListItemAttr* override;
    virtual auto OnGetItemText(long item, long col_nr) const -> wxString override;
    virtual int  OnGetItemImage(long item) const override;

private:
    static int col_sort() { return LIST_ID_PAYMENT_DATE; }

    void refreshList();
    void refreshVisualList(int selected_index = -1);

    void onNewBDSeries(wxCommandEvent& event);
    void onEditBDSeries(wxCommandEvent& event);
    void onDuplicateBDSeries(wxCommandEvent& event);
    void onDeleteBDSeries(wxCommandEvent& event);
    void onEnterBDTransaction(wxCommandEvent& event);
    void onSkipBDTransaction(wxCommandEvent& event);
    void onOpenAttachment(wxCommandEvent& event);
    void OnOrganizeAttachments(wxCommandEvent& event);
    void OnItemRightClick(wxMouseEvent& event);
    void OnListLeftClick(wxMouseEvent& event);
    void OnListItemActivated(wxListEvent& event);
    void OnMarkTransaction(wxCommandEvent& event);
    void OnMarkAllTransactions(wxCommandEvent& event);
    void OnListKeyDown(wxListEvent& event);
    void OnListItemSelected(wxListEvent& event);
    void OnSetUserColour(wxCommandEvent& event);
};
