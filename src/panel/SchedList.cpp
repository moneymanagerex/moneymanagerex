/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2014 - 2022 Nikolay Akimov
 Copyright (C) 2021, 2022 Mark Whalley (mark@ipx.co.uk)

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
#include "model/_all.h"

#include "SchedPanel.h"

#include "dialog/AttachmentDialog.h"
#include "dialog/SchedDialog.h"

enum
{
    MENU_TREEPOPUP_NEW = wxID_HIGHEST + 1300,
    MENU_TREEPOPUP_EDIT,
    MENU_TREEPOPUP_DUPLICATE,
    MENU_TREEPOPUP_DELETE,
    MENU_POPUP_BD_ENTER_OCCUR,
    MENU_POPUP_BD_SKIP_OCCUR,
    MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS,
    MENU_ON_SET_UDC0,
    MENU_ON_SET_UDC1,
    MENU_ON_SET_UDC2,
    MENU_ON_SET_UDC3,
    MENU_ON_SET_UDC4,
    MENU_ON_SET_UDC5,
    MENU_ON_SET_UDC6,
    MENU_ON_SET_UDC7
};

wxBEGIN_EVENT_TABLE(SchedList, ListBase)
    EVT_LEFT_DOWN(                                SchedList::OnListLeftClick)
    EVT_RIGHT_DOWN(                               SchedList::OnItemRightClick)

    EVT_LIST_ITEM_ACTIVATED(wxID_ANY,             SchedList::OnListItemActivated)
    EVT_LIST_ITEM_SELECTED(wxID_ANY,              SchedList::OnListItemSelected)
    EVT_LIST_KEY_DOWN(wxID_ANY,                   SchedList::OnListKeyDown)

    EVT_MENU(MENU_TREEPOPUP_NEW,                  SchedList::onNewBDSeries)
    EVT_MENU(MENU_TREEPOPUP_EDIT,                 SchedList::onEditBDSeries)
    EVT_MENU(MENU_TREEPOPUP_DUPLICATE,            SchedList::onDuplicateBDSeries)
    EVT_MENU(MENU_TREEPOPUP_DELETE,               SchedList::onDeleteBDSeries)
    EVT_MENU(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, SchedList::OnOrganizeAttachments)
    EVT_MENU(MENU_POPUP_BD_ENTER_OCCUR,           SchedList::onEnterBDTransaction)
    EVT_MENU(MENU_POPUP_BD_SKIP_OCCUR,            SchedList::onSkipBDTransaction)
    EVT_MENU_RANGE(
        MENU_ON_SET_UDC0, MENU_ON_SET_UDC7,       SchedList::OnSetUserColour)
wxEND_EVENT_TABLE()

const std::vector<ListColumnInfo> SchedList::LIST_INFO = {
    { LIST_ID_ICON,         true, _n("Icon"),        25,  _FC, false },
    { LIST_ID_ID,           true, _n("ID"),          _WA, _FR, true },
    { LIST_ID_PAYMENT_DATE, true, _n("Date Paid"),   _WH, _FL, true },
    { LIST_ID_DUE_DATE,     true, _n("Date Due"),    _WH, _FL, true },
    { LIST_ID_ACCOUNT,      true, _n("Account"),     _WH, _FL, true },
    { LIST_ID_PAYEE,        true, _n("Payee"),       _WH, _FL, true },
    { LIST_ID_STATUS,       true, _n("Status"),      _WH, _FC, true },
    { LIST_ID_CATEGORY,     true, _n("Category"),    _WH, _FL, true },
    { LIST_ID_TAGS,         true, _n("Tags"),        200, _FL, true },
    { LIST_ID_WITHDRAWAL,   true, _n("Withdrawal"),  _WH, _FR, true },
    { LIST_ID_DEPOSIT,      true, _n("Deposit"),     _WH, _FR, true },
    { LIST_ID_FREQUENCY,    true, _n("Frequency"),   _WH, _FL, true },
    { LIST_ID_REPEATS,      true, _n("Repetitions"), _WH, _FR, true },
    { LIST_ID_AUTO,         true, _n("Autorepeat"),  _WH, _FL, true },
    { LIST_ID_REMAINING,    true, _n("Remaining"),   _WH, _FL, true },
    { LIST_ID_NUMBER,       true, _n("Number"),      _WH, _FL, true },
    { LIST_ID_NOTES,        true, _n("Notes"),       150, _FL, true },
};

SchedList::SchedList(
    SchedPanel* bdp, wxWindow *parent, wxWindowID winid
) :
    ListBase(parent, winid),
    w_panel(bdp)
{
    mmThemeMetaColour(this, meta::COLOR_LISTPANEL);

    const wxAcceleratorEntry entries[] = {
        wxAcceleratorEntry(wxACCEL_CTRL, 'N', MENU_TREEPOPUP_NEW),
        wxAcceleratorEntry(wxACCEL_CTRL, 'E', MENU_TREEPOPUP_EDIT),
        wxAcceleratorEntry(wxACCEL_CTRL, 'U', MENU_TREEPOPUP_DUPLICATE),
        wxAcceleratorEntry(wxACCEL_CTRL, 'D', MENU_TREEPOPUP_DELETE),
        wxAcceleratorEntry(wxACCEL_CTRL, '0', MENU_ON_SET_UDC0),
        wxAcceleratorEntry(wxACCEL_CTRL, '1', MENU_ON_SET_UDC1),
        wxAcceleratorEntry(wxACCEL_CTRL, '2', MENU_ON_SET_UDC2),
        wxAcceleratorEntry(wxACCEL_CTRL, '3', MENU_ON_SET_UDC3),
        wxAcceleratorEntry(wxACCEL_CTRL, '4', MENU_ON_SET_UDC4),
        wxAcceleratorEntry(wxACCEL_CTRL, '5', MENU_ON_SET_UDC5),
        wxAcceleratorEntry(wxACCEL_CTRL, '6', MENU_ON_SET_UDC6),
        wxAcceleratorEntry(wxACCEL_CTRL, '7', MENU_ON_SET_UDC7)
    };
    wxAcceleratorTable tab(sizeof(entries) / sizeof(*entries), entries);
    SetAcceleratorTable(tab);

    m_setting_name = "SCHEDULED";
    o_col_order_prefix = "BD";
    o_col_width_prefix = "BD_COL";
    o_sort_prefix = "BD";
    m_col_info_id = LIST_INFO;
    m_col_id_nr = ListColumnInfo::getListId(LIST_INFO);
    m_sort_col_id = { col_sort() };
    createColumns();
}

SchedList::~SchedList()
{
    wxLogDebug("Exit SchedList");
}

void SchedList::refreshList()
{
    if (w_panel->m_sched_xa.size() == 0)
        return;

    int64 sched_id_n = (m_select_n != -1)
        ? w_panel->m_sched_xa[m_select_n].m_id
        : -1;
    refreshVisualList(w_panel->initList(sched_id_n));
}

int SchedList::getSortIcon(bool asc) const
{
    return asc
        ? SchedPanel::ICON_UPARROW
        : SchedPanel::ICON_DOWNARROW;
}

void SchedList::onColClick(wxListEvent& event)
{
    int col_nr = (event.GetId() == MENU_HEADER_SORT)
        ? m_sel_col_nr
        : event.GetColumn();
    if (!isValidColNr(col_nr))
        return;

    int col_id = getColId_Nr(col_nr);
    if (!m_col_info_id[col_id].sortable)
        return;

    if (m_sort_col_id[0] != col_id)
        m_sort_col_id[0] = col_id;
    else if (event.GetId() != MENU_HEADER_SORT)
        m_sort_asc[0] = !m_sort_asc[0];

    updateSortIcon();
    savePref();

    int64 sched_id_n = (m_select_n != -1)
        ? w_panel->m_sched_xa[m_select_n].m_id
        : -1;
    refreshVisualList(w_panel->initList(sched_id_n));
}

void SchedList::OnItemRightClick(wxMouseEvent& event)
{
    if (m_select_n > -1)
        SetItemState(m_select_n, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
    int Flags = wxLIST_HITTEST_ONITEM;
    m_select_n = HitTest(wxPoint(event.m_x, event.m_y), Flags);

    if (m_select_n >= 0) {
        SetItemState(m_select_n, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(m_select_n, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
    }
    w_panel->updateBottomPanelData(m_select_n);
    bool item_active = (m_select_n >= 0);
    wxMenu menu;
    menu.Append(MENU_POPUP_BD_ENTER_OCCUR, _tu("Enter next Occurrence…"));
    menu.AppendSeparator();
    menu.Append(MENU_POPUP_BD_SKIP_OCCUR, _t("Skip next Occurrence"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_NEW, _tu("&New Scheduled Transaction…"));
    menu.Append(MENU_TREEPOPUP_EDIT, _tu("&Edit Scheduled Transaction…"));
    menu.Append(MENU_TREEPOPUP_DUPLICATE, _tu("D&uplicate Scheduled Transaction…"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_DELETE, _tu("&Delete Scheduled Transaction…"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, _tu("&Organize Attachments…"));

    menu.Enable(MENU_POPUP_BD_ENTER_OCCUR, item_active);
    menu.Enable(MENU_POPUP_BD_SKIP_OCCUR, item_active);
    menu.Enable(MENU_TREEPOPUP_EDIT, item_active);
    menu.Enable(MENU_TREEPOPUP_DUPLICATE, item_active);
    menu.Enable(MENU_TREEPOPUP_DELETE, item_active);
    menu.Enable(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, item_active);

    PopupMenu(&menu, event.GetPosition());
    this->SetFocus();
}

wxString SchedList::OnGetItemText(long item, long col_nr) const
{
    return w_panel->getItem(item, getColId_Nr(static_cast<int>(col_nr)));
}

void SchedList::OnListItemSelected(wxListEvent& event)
{
    m_select_n = event.GetIndex();
    w_panel->updateBottomPanelData(m_select_n);
}

void SchedList::OnListLeftClick(wxMouseEvent& event)
{
    int Flags = wxLIST_HITTEST_ONITEM;
    long index = HitTest(wxPoint(event.m_x, event.m_y), Flags);
    if (index == -1) {
        m_select_n = -1;
        w_panel->updateBottomPanelData(m_select_n);
    }
    event.Skip();
}

int SchedList::OnGetItemImage(long item) const
{
    const SchedData& sched_d = w_panel->m_sched_xa[item];
    int due_days = sched_d.m_due_date.daysSince(w_panel->m_today);

    // Returns the icon to be shown for each entry
    if (due_days < 0)
        return SchedPanel::ICON_FOLLOWUP;
    if (sched_d.m_repeat.m_mode.id() == RepeatMode::e_automated)
        return SchedPanel::ICON_RUN_AUTO;
    if (sched_d.m_repeat.m_mode.id() == RepeatMode::e_suggested)
        return SchedPanel::ICON_RUN;
    return -1;
}

void SchedList::OnListKeyDown(wxListEvent& event)
{
    switch ( event.GetKeyCode() ) {
    case WXK_DELETE: {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED,
            MENU_TREEPOPUP_DELETE);
        onDeleteBDSeries(evt);
        break;
    }
    default:
        event.Skip();
    }
}

void SchedList::onNewBDSeries(wxCommandEvent& /*event*/)
{
    SchedDialog dlg(this, 0, false, false);
    if (dlg.ShowModal() == wxID_OK)
        refreshVisualList(w_panel->initList(dlg.GetTransID()));
}

void SchedList::onEditBDSeries(wxCommandEvent& /*event*/)
{
    if (m_select_n == -1)
        return;

    SchedDialog dlg(this, w_panel->m_sched_xa[m_select_n].m_id, false, false);
    if (dlg.ShowModal() == wxID_OK)
        refreshVisualList(w_panel->initList(dlg.GetTransID()));
}

void SchedList::onDuplicateBDSeries(wxCommandEvent& /*event*/)
{
    if (m_select_n == -1) return;

    SchedDialog dlg(this, w_panel->m_sched_xa[m_select_n].m_id, true, false);
    if (dlg.ShowModal() == wxID_OK)
        refreshVisualList(w_panel->initList(dlg.GetTransID()));
}

void SchedList::onDeleteBDSeries(wxCommandEvent& WXUNUSED(event))
{
    if (w_panel->m_sched_xa.empty())
        return;
    if (m_select_n < 0)
        return;

    wxMessageDialog msgDlg(this,
        _t("Do you want to delete the scheduled transaction?"),
        _t("Confirm Deletion"),
        wxYES_NO | wxNO_DEFAULT | wxICON_ERROR
    );
    if (msgDlg.ShowModal() == wxID_YES) {
        int64 sched_id = w_panel->m_sched_xa[m_select_n].m_id;
        SchedModel::instance().purge_id(sched_id);
        mmAttachmentManage::DeleteAllAttachments(SchedModel::s_ref_type, sched_id);
        FieldValueModel::instance().purge_ref(SchedModel::s_ref_type, sched_id);
        w_panel->initList();
        refreshVisualList(m_select_n);
    }
}

void SchedList::onEnterBDTransaction(wxCommandEvent& /*event*/)
{
    if (m_select_n == -1)
        return;

    int64 id = w_panel->m_sched_xa[m_select_n].m_id;
    SchedDialog dlg(this, id, false, true);
    if ( dlg.ShowModal() == wxID_OK ) {
        if (++m_select_n < long(w_panel->m_sched_xa.size()))
            id = w_panel->m_sched_xa[m_select_n].m_id;
        refreshVisualList(w_panel->initList(id));
    }
}

void SchedList::onSkipBDTransaction(wxCommandEvent& /*event*/)
{
    if (m_select_n == -1)
        return;

    int64 sched_id = w_panel->m_sched_xa[m_select_n].m_id;
    SchedModel::instance().reschedule_id(sched_id);
    if (++m_select_n < long(w_panel->m_sched_xa.size()))
        sched_id = w_panel->m_sched_xa[m_select_n].m_id;
    refreshVisualList(w_panel->initList(sched_id));
}

void SchedList::OnOrganizeAttachments(wxCommandEvent& /*event*/)
{
    if (m_select_n == -1)
        return;

    int64 ref_id = w_panel->m_sched_xa[m_select_n].m_id;

    AttachmentDialog dlg(this, SchedModel::s_ref_type, ref_id);
    dlg.ShowModal();

    refreshVisualList(w_panel->initList(ref_id));
}

void SchedList::onOpenAttachment(wxCommandEvent& WXUNUSED(event))
{
    if (m_select_n == -1)
        return;

    int64 ref_id = w_panel->m_sched_xa[m_select_n].m_id;
    mmAttachmentManage::OpenAttachmentFromPanelIcon(this, SchedModel::s_ref_type, ref_id);
    refreshVisualList(w_panel->initList(ref_id));
}

void SchedList::OnListItemActivated(wxListEvent& WXUNUSED(event))
{
    if (m_select_n == -1)
        return;

    SchedDialog dlg(this, w_panel->m_sched_xa[m_select_n].m_id, false, false);
    if (dlg.ShowModal() == wxID_OK)
        refreshVisualList(w_panel->initList(dlg.GetTransID()));
}

wxListItemAttr* SchedList::OnGetItemAttr(long item) const
{
    if (item < 0 || item >= static_cast<int>(w_panel->m_sched_xa.size()))
        return 0;

    int color_id = w_panel->m_sched_xa[item].m_color.GetValue();

    static std::map<int, wxSharedPtr<wxListItemAttr> > cache;
    if (color_id > 0) {
        color_id = std::min(7, color_id);
        if (const auto it = cache.find(color_id); it != cache.end())
            return it->second.get();
        else {
            switch (color_id) {
            case 1: cache[color_id] = new wxListItemAttr(
                *bestFontColour(mmColors::userDefColor1), mmColors::userDefColor1, wxNullFont
            ); break;
            case 2: cache[color_id] = new wxListItemAttr(
                *bestFontColour(mmColors::userDefColor2), mmColors::userDefColor2, wxNullFont
            ); break;
            case 3: cache[color_id] = new wxListItemAttr(
                *bestFontColour(mmColors::userDefColor3), mmColors::userDefColor3, wxNullFont
            ); break;
            case 4: cache[color_id] = new wxListItemAttr(
                *bestFontColour(mmColors::userDefColor4), mmColors::userDefColor4, wxNullFont
            ); break;
            case 5: cache[color_id] = new wxListItemAttr(
                *bestFontColour(mmColors::userDefColor5), mmColors::userDefColor5, wxNullFont
            ); break;
            case 6: cache[color_id] = new wxListItemAttr(
                *bestFontColour(mmColors::userDefColor6), mmColors::userDefColor6, wxNullFont
            ); break;
            case 7: cache[color_id] = new wxListItemAttr(
                *bestFontColour(mmColors::userDefColor7), mmColors::userDefColor7, wxNullFont
            ); break;
            }
            return cache[color_id].get();
        }
    }

    /* Returns the alternating background pattern */
    return (item % 2) ? w_attr2.get() : w_attr1.get();
}

void SchedList::OnSetUserColour(wxCommandEvent& event)
{
    if (m_select_n == -1) return;
    int64 id = w_panel->m_sched_xa[m_select_n].m_id;

    int user_color_id = event.GetId();
    user_color_id -= MENU_ON_SET_UDC0;
    wxLogDebug("id: %i", user_color_id);

    SchedModel::instance().db_savepoint();

    SchedData* sched_n = SchedModel::instance().unsafe_get_id_data_n(id);
    if (sched_n) {
        sched_n->m_color = user_color_id;
        SchedModel::instance().unsafe_update_data_n(sched_n);
    }
    SchedModel::instance().db_release_savepoint();

    refreshList();
}
