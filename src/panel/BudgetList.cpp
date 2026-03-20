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
#include "mmex.h"
#include "util/mmDateRange.h"

#include "model/_all.h"
#include "model/PrefModel.h"

#include "BudgetPanel.h"
#include "dialog/BudgetEntryDialog.h"
#include "report/budget.h"

wxBEGIN_EVENT_TABLE(BudgetList, ListBase)
    EVT_LIST_ITEM_SELECTED(wxID_ANY,  BudgetList::OnListItemSelected)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, BudgetList::OnListItemActivated)
    EVT_MOTION(BudgetList::OnMouseMove)
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
    BudgetPanel* cp, wxWindow *parent, const wxWindowID id
) :
    ListBase(parent, id),
    attr3_(new wxListItemAttr(
        wxNullColour, mmThemeMetaColour(meta::COLOR_LISTTOTAL), wxNullFont
    )),
    cp_(cp)
{
    mmThemeMetaColour(this, meta::COLOR_LISTPANEL);

    m_setting_name = "BUDGET";
    m_col_info_id = LIST_INFO;
    o_col_width_prefix = "BUDGET_COL";
}

int BudgetList::OnGetItemImage(long item) const
{
    return cp_->GetItemImage(item);
}

wxString BudgetList::OnGetItemText(long item, long col_nr) const
{
    return cp_->getItem(item, getColId_Nr(static_cast<int>(col_nr)));
}

wxListItemAttr* BudgetList::OnGetItemAttr(long item) const
{
    if ((cp_->GetTransID(item) < 0) &&
        (cp_->GetCurrentView() != BudgetPanel::VIEW_SUMM)
    ) {
        return attr3_.get();
    }

    /* Returns the alternating background pattern */
    return (item % 2) ? attr2_.get() : attr1_.get();
}

void BudgetList::OnListItemActivated(wxListEvent& event)
{
    selectedIndex_ = event.GetIndex();
    cp_->OnListItemActivated(selectedIndex_);
}

void BudgetList::OnMouseMove(wxMouseEvent& event)
{
    long row = -1;
    long column = -1;
    int flags = 0;

    row = HitTest(event.GetPosition(), flags, &column);

    if (LIST_ID_ICON == column && row >= 0) {
        wxString tooltip;
        int icon = cp_->GetItemImage(row);

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
