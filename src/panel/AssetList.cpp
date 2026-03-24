/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2015 -2021 Nikolay Akimov
 Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2026 Klaus Wich

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

#include "base/defs.h"
#include <wx/srchctrl.h>

#include "base/constants.h"
#include "base/images_list.h"
#include "util/_simple.h"

#include "model/_all.h"

#include "AssetPanel.h"

#include "dialog/AssetDialog.h"
#include "dialog/AttachmentDialog.h"

wxBEGIN_EVENT_TABLE(AssetList, ListBase)
    EVT_RIGHT_DOWN(                               AssetList::onMouseRightClick)
    EVT_LEFT_DOWN(                                AssetList::onListLeftClick)

    EVT_LIST_ITEM_ACTIVATED(wxID_ANY,             AssetList::onListItemActivated)
    EVT_LIST_ITEM_SELECTED(wxID_ANY,              AssetList::onListItemSelected)
    EVT_LIST_END_LABEL_EDIT(wxID_ANY,             AssetList::onEndLabelEdit)
    EVT_LIST_KEY_DOWN(wxID_ANY,                   AssetList::onListKeyDown)

    EVT_MENU(MENU_TREEPOPUP_NEW,                  AssetList::onNewAsset)
    EVT_MENU(MENU_TREEPOPUP_EDIT,                 AssetList::onEditAsset)
    EVT_MENU(MENU_TREEPOPUP_ADDTRANS,             AssetList::onAddAssetTrans)
    EVT_MENU(MENU_TREEPOPUP_VIEWTRANS,            AssetList::onViewAssetTrans)
    EVT_MENU(MENU_TREEPOPUP_GOTOACCOUNT,          AssetList::onGotoAssetAccount)
    EVT_MENU(MENU_TREEPOPUP_DELETE,               AssetList::onDeleteAsset)
    EVT_MENU(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, AssetList::onOrganizeAttachments)
    EVT_MENU(MENU_ON_DUPLICATE_TRANSACTION,       AssetList::onDuplicateAsset)
wxEND_EVENT_TABLE()

const std::vector<ListColumnInfo> AssetList::LIST_INFO = {
    { LIST_ID_ICON,          true, _n("Icon"),          25,  _FL, false },
    { LIST_ID_ID,            true, _n("ID"),            _WA, _FR, true },
    { LIST_ID_NAME,          true, _n("Name"),          150, _FL, true },
    { LIST_ID_DATE,          true, _n("Date"),          _WH, _FL, true },
    { LIST_ID_TYPE,          true, _n("Type"),          _WH, _FL, true },
    { LIST_ID_VALUE_INITIAL, true, _n("Initial Value"), _WH, _FR, true },
    { LIST_ID_VALUE_CURRENT, true, _n("Current Value"), _WH, _FR, true },
    { LIST_ID_NOTES,         true, _n("Notes"),         450, _FL, true },
};

AssetList::AssetList(
    AssetPanel* panel,
    wxWindow* parent_win,
    wxWindowID win_id
) :
    ListBase(parent_win, win_id),
    w_panel(panel)
{
    mmThemeMetaColour(this, meta::COLOR_LISTPANEL);

    m_setting_name = "ASSETS";
    o_col_order_prefix = "ASSETS";
    o_col_width_prefix = "ASSETS_COL";
    o_sort_prefix = "ASSETS";
    m_col_info_id = LIST_INFO;
    m_col_id_nr = ListColumnInfo::getListId(LIST_INFO);
    m_sort_col_id = { LIST_ID_DATE };
    createColumns();
}

int AssetList::getSortIcon(bool asc) const
{
    return asc
        ? AssetPanel::ICON_UPARROW
        : AssetPanel::ICON_DOWNARROW;
}

void AssetList::onMouseRightClick(wxMouseEvent& event)
{
    if (m_selected_row > -1)
        SetItemState(m_selected_row, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
    int Flags = wxLIST_HITTEST_ONITEM;
    m_selected_row = HitTest(wxPoint(event.m_x, event.m_y), Flags);

    if (m_selected_row >= 0) {
        SetItemState(m_selected_row, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(m_selected_row, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
    }
    w_panel->updateExtraAssetData(m_selected_row);
    wxMenu menu;
    menu.Append(MENU_TREEPOPUP_NEW, _tu("&New Asset…"));
    menu.AppendSeparator();
    menu.Append(MENU_ON_DUPLICATE_TRANSACTION, _tu("D&uplicate Asset…"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_ADDTRANS, _tu("&Add Asset Transaction…"));
    menu.Append(MENU_TREEPOPUP_VIEWTRANS, _t("&View Asset Transactions"));
    menu.Append(MENU_TREEPOPUP_GOTOACCOUNT, _tu("&Open Asset Account…"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_EDIT, _tu("&Edit Asset…"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_DELETE, _tu("&Delete Asset…"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, _tu("&Organize Attachments…"));
    if (m_selected_row < 0) {
        menu.Enable(MENU_ON_DUPLICATE_TRANSACTION, false);
        menu.Enable(MENU_TREEPOPUP_ADDTRANS, false);
        menu.Enable(MENU_TREEPOPUP_VIEWTRANS, false);
        menu.Enable(MENU_TREEPOPUP_EDIT, false);
        menu.Enable(MENU_TREEPOPUP_DELETE, false);
        menu.Enable(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, false);
    }
    else {
        // ASSETNAME <=> ACCOUNTNAME
        auto asset_account = AccountModel::instance().get_name_data_n(
            w_panel->m_asset_a[m_selected_row].m_name
        );
        if (!asset_account)
            // ASSETTYPE <=> ACCOUNTNAME
            asset_account = AccountModel::instance().get_name_data_n(
                w_panel->m_asset_a[m_selected_row].m_type.name()
            );
        menu.Enable(MENU_TREEPOPUP_GOTOACCOUNT, asset_account);
        menu.Enable(MENU_TREEPOPUP_VIEWTRANS, asset_account);
    }

    PopupMenu(&menu, event.GetPosition());
}

void AssetList::onListLeftClick(wxMouseEvent& event)
{
    int Flags = wxLIST_HITTEST_ONITEM;
    long index = HitTest(wxPoint(event.m_x, event.m_y), Flags);
    if (index == -1) {
        m_selected_row = -1;
        w_panel->updateExtraAssetData(m_selected_row);
    }
    event.Skip();
}

wxString AssetList::OnGetItemText(long item, long col_nr) const
{
    return w_panel->getItem(item, getColId_Nr(col_nr));
}

void AssetList::onListItemSelected(wxListEvent& event)
{
    m_selected_row = event.GetIndex();
    w_panel->updateExtraAssetData(m_selected_row);
}

int AssetList::OnGetItemImage(long item) const
{
    return w_panel->m_asset_a[item].m_type.id();
}

void AssetList::onListKeyDown(wxListEvent& event)
{
    if (event.GetKeyCode() == WXK_DELETE) {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_DELETE);
        onDeleteAsset(evt);
    }
    else {
        event.Skip();
    }
}

void AssetList::onNewAsset(wxCommandEvent& /*event*/)
{
    AssetDialog dlg(this, static_cast<AssetData*>(nullptr));
    if (dlg.ShowModal() == wxID_OK) {
        doRefreshItems(dlg.asset_id());
        w_panel->w_frame->RefreshNavigationTree();
    }
}

void AssetList::doRefreshItems(int64 trx_id)
{
    int selectedIndex = w_panel->initVirtualListControl(trx_id);

    long cnt = static_cast<long>(w_panel->m_asset_a.size());

    if (selectedIndex >= cnt || selectedIndex < 0)
        selectedIndex = getSortAsc() ? cnt - 1 : 0;

    if (cnt>0)
        RefreshItems(0, cnt > 0 ? --cnt : 0);
    else
        selectedIndex = -1;

    if (selectedIndex >= 0 && cnt>0) {
        SetItemState(selectedIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(selectedIndex, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
        EnsureVisible(selectedIndex);
    }
    m_selected_row = selectedIndex;
}

void AssetList::onDeleteAsset(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0)
        return;

    wxMessageDialog msgDlg(this,
        _t("Do you want to delete the asset?"),
        _t("Confirm Asset Deletion"),
        wxYES_NO | wxNO_DEFAULT | wxICON_ERROR
    );

    if (msgDlg.ShowModal() == wxID_YES) {
        const AssetData& asset = w_panel->m_asset_a[m_selected_row];
        AssetModel::instance().purge_id(asset.m_id);
        mmAttachmentManage::DeleteAllAttachments(AssetModel::s_ref_type, asset.m_id);
        TrxLinkModel::instance().purge_ref(AssetModel::s_ref_type, asset.m_id);

        w_panel->initVirtualListControl();
        m_selected_row = -1;
        w_panel->updateExtraAssetData(m_selected_row);
    }
}

void AssetList::onEditAsset(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0)
        return;

    wxListEvent evt(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxID_ANY);
    AddPendingEvent(evt);
}

void AssetList::onDuplicateAsset(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0)
        return;

    const AssetData& asset = w_panel->m_asset_a[m_selected_row];
    AssetData duplicate_asset;
    duplicate_asset.clone_from(asset);

    if (editAsset(&duplicate_asset)) {
        w_panel->initVirtualListControl();
        doRefreshItems(duplicate_asset.m_id);
    }
}

void AssetList::onAddAssetTrans(wxCommandEvent& WXUNUSED(event))
{
    if (m_selected_row < 0)
        return;

    w_panel->addAssetTrans(m_selected_row);
}

void AssetList::onViewAssetTrans(wxCommandEvent& WXUNUSED(event))
{
    if (m_selected_row < 0) return;

    w_panel->viewAssetTrans(m_selected_row);
}

void AssetList::onGotoAssetAccount(wxCommandEvent& WXUNUSED(event))
{
    if (m_selected_row < 0) return;

    w_panel->gotoAssetAccount(m_selected_row);
}

void AssetList::onOrganizeAttachments(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0) return;

    int64 ref_id = w_panel->m_asset_a[m_selected_row].m_id;

    AttachmentDialog dlg(this, AssetModel::s_ref_type, ref_id);
    dlg.ShowModal();

    doRefreshItems(ref_id);
}

void AssetList::onOpenAttachment(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0)
        return;

    int64 ref_id = w_panel->m_asset_a[m_selected_row].m_id;
    mmAttachmentManage::OpenAttachmentFromPanelIcon(this, AssetModel::s_ref_type, ref_id);
    doRefreshItems(ref_id);
}

void AssetList::onListItemActivated(wxListEvent& event)
{
    if (m_selected_row < 0) {
        m_selected_row = event.GetIndex();
    }
    editAsset(&(w_panel->m_asset_a[m_selected_row]));
}

bool AssetList::editAsset(AssetData* asset_n)
{
    AssetDialog dlg(this, asset_n);
    if (dlg.ShowModal() != wxID_OK)
        return false;

    doRefreshItems(dlg.asset_id());
    w_panel->updateExtraAssetData(m_selected_row);
    return true;
}

void AssetList::onColClick(wxListEvent& event)
{
    int col_nr = (event.GetId() == MENU_HEADER_SORT) ? m_sel_col_nr : event.GetColumn();
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

    int64 trx_id = (m_selected_row >= 0)
        ? w_panel->m_asset_a[m_selected_row].m_id
        : -1;

    doRefreshItems(trx_id);
}

void AssetList::onEndLabelEdit(wxListEvent& event)
{
    if (event.IsEditCancelled())
        return;

    AssetData* asset_n = &w_panel->m_asset_a[event.GetIndex()];
    asset_n->m_name = event.m_item.m_text;
    AssetModel::instance().unsafe_save_data_n(asset_n);
    RefreshItems(event.GetIndex(), event.GetIndex());
}
