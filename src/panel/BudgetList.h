/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2013 - 2022 Nikolay Akimov
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

#include "model/BudgetModel.h"
#include "_ListBase.h"

class wxListCtrl;
class wxListEvent;
class BudgetPanel;

/* Custom ListCtrl class that implements virtual LC style */
class BudgetList : public ListBase
{
    DECLARE_NO_COPY_CLASS(BudgetList)
    wxDECLARE_EVENT_TABLE();

public:
    enum LIST_ID
    {
        LIST_ID_ICON = 0,
        LIST_ID_CATEGORY,
        LIST_ID_FREQUENCY,
        LIST_ID_AMOUNT,
        LIST_ID_ESTIMATED,
        LIST_ID_ACTUAL,
        LIST_ID_NOTES,
        LIST_ID_size, // number of columns
    };

private:
    static const std::vector<ListColumnInfo> LIST_INFO;
    long m_select = -1;

    BudgetPanel* m_panel;
    wxSharedPtr<wxListItemAttr> w_attr3; // style3

public:
    BudgetList(BudgetPanel* panel, wxWindow* parent_win, const wxWindowID win_id);

public:
    // override wxListCtrl
    virtual auto OnGetItemText(long item, long col_nr) const -> wxString override;
    virtual auto OnGetItemAttr(long item) const -> wxListItemAttr* override;
    virtual int  OnGetItemImage(long item) const override;

private:
    void onListItemSelected(wxListEvent& event);
    void onListItemActivated(wxListEvent& event);
    void onMouseMove(wxMouseEvent& event);
};
