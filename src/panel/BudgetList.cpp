/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2012 Stefano Giorgio
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

#include "base/constants.h"
#include "base/images_list.h"
#include "BudgetList.h"
#include "BudgetPanel.h"

wxBEGIN_EVENT_TABLE(BudgetList, ListBase)
    EVT_LIST_ITEM_SELECTED(wxID_ANY,  BudgetList::onListItemSelected)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, BudgetList::onListItemActivated)
    EVT_MOTION(                       BudgetList::onMouseMove)
wxEND_EVENT_TABLE()

const std::vector<ListColumnInfo> BudgetList::LIST_INFO = {
    { LIST_ID_ICON,      true, _n("Icon"),      _WH, _FL, false },
    { LIST_ID_CATEGORY,  true, _n("Category"),  _WH, _FL, false },
    { LIST_ID_FREQUENCY, true, _n("Frequency"), _WH, _FL, false },
    { LIST_ID_AMOUNT,    true, _n("Amount"),    _WH, _FR, false },
    { LIST_ID_ESTIMATED, true, _n("Estimated"), _WH, _FR, false },
    { LIST_ID_ACTUAL,    true, _n("Actual"),    _WH, _FR, false },
    { LIST_ID_NOTES,     true, _n("Notes"),     _WH, _FL, false },
};

BudgetList::BudgetList(
    BudgetPanel* panel,
    wxWindow* parent_win,
    const wxWindowID win_id
) :
    ListBase(parent_win, win_id),
    w_attr3(new wxListItemAttr(
        wxNullColour, mmThemeMetaColour(meta::COLOR_LISTTOTAL), wxNullFont
    )),
    m_panel(panel)
{
    mmThemeMetaColour(this, meta::COLOR_LISTPANEL);

    m_setting_name = "BUDGET";
    m_col_info_id = LIST_INFO;
    o_col_width_prefix = "BUDGET_COL";
}

int BudgetList::OnGetItemImage(long item) const
{
    return m_panel->getItemImage(item);
}

wxString BudgetList::OnGetItemText(long item, long col_nr) const
{
    return m_panel->getItem(item, getColId_Nr(static_cast<int>(col_nr)));
}

wxListItemAttr* BudgetList::OnGetItemAttr(long item) const
{
    if ((m_panel->getCatId(item) < 0) &&
        (m_panel->getCurrentView() != BudgetPanel::VIEW_SUMM)
    ) {
        return w_attr3.get();
    }

    // Returns the alternating background pattern
    return (item % 2 == 0) ? w_attr1.get() : w_attr2.get();
}

void BudgetList::onListItemActivated(wxListEvent& event)
{
    m_select = event.GetIndex();
    m_panel->onListItemActivated(m_select);
}

void BudgetList::onMouseMove(wxMouseEvent& event)
{
    long row = -1;
    long column = -1;
    int flags = 0;

    row = HitTest(event.GetPosition(), flags, &column);

    if (LIST_ID_ICON == column && row >= 0) {
        wxString tooltip;
        int icon = m_panel->getItemImage(row);

        if (-1 == icon)
            tooltip = _("No budget defined");
        else if (BudgetPanel::ICON_VOID == icon)
            tooltip = _("Critical: budget exceeded! Stop spending");
        else if (BudgetPanel::ICON_RECONCILLED == icon)
            tooltip = _("Within budget limits");
        else if (BudgetPanel::ICON_FOLLOWUP == icon)
            tooltip = _("Alert: budget close to or over limit");

        mmToolTip(this, tooltip);
    }
    else {
        UnsetToolTip();
    }

    event.Skip();
}

void BudgetList::onListItemSelected(wxListEvent& event)
{
    m_select = event.GetIndex();
}
