/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2015, 2017 Nikolay Akimov

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

#include "assetspanel.h"
#include "assetdialog.h"
#include "attachmentdialog.h"
#include "constants.h"
#include "images_list.h"
#include "mmframe.h"

#include "Model_Setting.h"
#include "Model_Usage.h"
#include <wx/srchctrl.h>

/*******************************************************/

wxBEGIN_EVENT_TABLE(mmAssetsListCtrl, mmListCtrl)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY,   mmAssetsListCtrl::OnListItemActivated)
    EVT_LIST_ITEM_SELECTED(wxID_ANY,    mmAssetsListCtrl::OnListItemSelected)
    EVT_LIST_END_LABEL_EDIT(wxID_ANY,   mmAssetsListCtrl::OnEndLabelEdit)
    EVT_RIGHT_DOWN(mmAssetsListCtrl::OnMouseRightClick)
    EVT_LEFT_DOWN(mmAssetsListCtrl::OnListLeftClick)

    EVT_MENU(MENU_TREEPOPUP_NEW,    mmAssetsListCtrl::OnNewAsset)
    EVT_MENU(MENU_TREEPOPUP_EDIT,   mmAssetsListCtrl::OnEditAsset)
    EVT_MENU(MENU_TREEPOPUP_ADDTRANS, mmAssetsListCtrl::OnAddAssetTrans)
    EVT_MENU(MENU_TREEPOPUP_VIEWTRANS, mmAssetsListCtrl::OnViewAssetTrans)
    EVT_MENU(MENU_TREEPOPUP_GOTOACCOUNT, mmAssetsListCtrl::OnGotoAssetAccount)
    EVT_MENU(MENU_TREEPOPUP_DELETE, mmAssetsListCtrl::OnDeleteAsset)
    EVT_MENU(MENU_ON_DUPLICATE_TRANSACTION, mmAssetsListCtrl::OnDuplicateAsset)
    EVT_MENU(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, mmAssetsListCtrl::OnOrganizeAttachments)

    EVT_LIST_KEY_DOWN(wxID_ANY, mmAssetsListCtrl::OnListKeyDown)
wxEND_EVENT_TABLE()
/*******************************************************/

mmAssetsListCtrl::mmAssetsListCtrl(mmAssetsPanel* cp, wxWindow *parent, wxWindowID winid)
: mmListCtrl(parent, winid)
, m_panel(cp)
{
    ToggleWindowStyle(wxLC_EDIT_LABELS);

    m_columns.push_back(PANEL_COLUMN(" ", 25, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("ID"), wxLIST_AUTOSIZE, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(PANEL_COLUMN(_("Name"), 150, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Date"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Type"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Initial Value"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(PANEL_COLUMN(_("Current Value"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(PANEL_COLUMN(_("Notes"), 450, wxLIST_FORMAT_LEFT));

    m_col_width = "ASSETS_COL%d_WIDTH";
    m_default_sort_column = m_panel->col_sort();

    for (const auto& entry : m_columns)
    {
        int count = GetColumnCount();
        InsertColumn(count
            , entry.HEADER
            , entry.FORMAT
            , Model_Setting::instance().GetIntSetting(wxString::Format(m_col_width, count), entry.WIDTH));
    }

    // load the global variables
    m_selected_col = Model_Setting::instance().GetIntSetting("ASSETS_SORT_COL", m_default_sort_column);
    m_asc = Model_Setting::instance().GetBoolSetting("ASSETS_ASC", true);

}

void mmAssetsListCtrl::OnMouseRightClick(wxMouseEvent& event)
{
    if (m_selected_row > -1)
        SetItemState(m_selected_row, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
    int Flags = wxLIST_HITTEST_ONITEM;
    m_selected_row = HitTest(wxPoint(event.m_x, event.m_y), Flags);

    if (m_selected_row >= 0)
    {
        SetItemState(m_selected_row, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(m_selected_row, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
    }
    m_panel->updateExtraAssetData(m_selected_row);
    wxMenu menu;
    menu.Append(MENU_TREEPOPUP_NEW, _("&New Asset"));
    menu.AppendSeparator();
    menu.Append(MENU_ON_DUPLICATE_TRANSACTION, _("D&uplicate Asset"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_ADDTRANS, _("&Add Asset Transaction"));
    menu.Append(MENU_TREEPOPUP_VIEWTRANS, _("&View Asset Transactions"));
    menu.Append(MENU_TREEPOPUP_GOTOACCOUNT, _("&Open Asset Account"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_EDIT, _("&Edit Asset"));
    menu.Append(MENU_TREEPOPUP_DELETE, _("&Delete Asset"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, _("&Organize Attachments"));
    if (m_selected_row < 0)
    {
        menu.Enable(MENU_ON_DUPLICATE_TRANSACTION, false);
        menu.Enable(MENU_TREEPOPUP_ADDTRANS, false);
        menu.Enable(MENU_TREEPOPUP_VIEWTRANS, false);
        menu.Enable(MENU_TREEPOPUP_EDIT, false);
        menu.Enable(MENU_TREEPOPUP_DELETE, false);
        menu.Enable(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, false);
    }
    PopupMenu(&menu, event.GetPosition());
}

void mmAssetsListCtrl::OnListLeftClick(wxMouseEvent& event)
{
    int Flags = wxLIST_HITTEST_ONITEM;
    long index = HitTest(wxPoint(event.m_x, event.m_y), Flags);
    if (index == -1)
    {
        m_selected_row = -1;
        m_panel->updateExtraAssetData(m_selected_row);
    }
    event.Skip();
}

wxString mmAssetsListCtrl::OnGetItemText(long item, long column) const
{
    return m_panel->getItem(item, column);
}

void mmAssetsListCtrl::OnListItemSelected(wxListEvent& event)
{
    m_selected_row = event.GetIndex();
    m_panel->updateExtraAssetData(m_selected_row);
}

int mmAssetsListCtrl::OnGetItemImage(long item) const
{
    return Model_Asset::type(m_panel->m_assets[item]);
}

void mmAssetsListCtrl::OnListKeyDown(wxListEvent& event)
{
    if (event.GetKeyCode() == WXK_DELETE)
    {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_DELETE);
        OnDeleteAsset(evt);
    }
    else
    {
        event.Skip();
    }
}

void mmAssetsListCtrl::OnNewAsset(wxCommandEvent& WXUNUSED(event))
{
    mmAssetDialog dlg(this, m_panel->m_frame, (Model_Asset::Data*)nullptr);
    if (dlg.ShowModal() == wxID_OK)
    {
        doRefreshItems(dlg.m_asset->ASSETID);
        m_panel->m_frame->RefreshNavigationTree();
    }
}

void mmAssetsListCtrl::doRefreshItems(int trx_id)
{
    int selectedIndex = initVirtualListControl(trx_id, m_selected_col, m_asc);

    long cnt = static_cast<long>(m_panel->m_assets.size());

    if (selectedIndex >= cnt || selectedIndex < 0)
        selectedIndex = m_asc ? cnt - 1 : 0;

    if (cnt>0)
    {
        RefreshItems(0, cnt - 1);
        if (selectedIndex >= 0)
        {
            SetItemState(selectedIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            SetItemState(selectedIndex, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
            EnsureVisible(selectedIndex);
        }
    }
    else
        selectedIndex = -1;

    m_selected_row = selectedIndex;
}

void mmAssetsListCtrl::OnDeleteAsset(wxCommandEvent& WXUNUSED(event))
{
    if (m_selected_row < 0)    return;

    wxMessageDialog msgDlg(this
        , _("Do you really want to delete the Asset?")
        , _("Confirm Asset Deletion")
        , wxYES_NO | wxNO_DEFAULT | wxICON_ERROR);

    if (msgDlg.ShowModal() == wxID_YES)
    {
        const Model_Asset::Data& asset = m_panel->m_assets[m_selected_row];
        Model_Asset::instance().remove(asset.ASSETID);
        mmAttachmentManage::DeleteAllAttachments(Model_Attachment::reftype_desc(Model_Attachment::ASSET), asset.ASSETID);
        Model_Translink::RemoveTransLinkRecords(Model_Attachment::ASSET, asset.ASSETID);

        initVirtualListControl(m_selected_row, m_selected_col, m_asc);
        m_selected_row = -1;
        m_panel->updateExtraAssetData(m_selected_row);
    }
}

void mmAssetsListCtrl::OnEditAsset(wxCommandEvent& WXUNUSED(event))
{
    if (m_selected_row < 0)     return;

    wxListEvent evt(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxID_ANY);
    AddPendingEvent(evt);
}

void mmAssetsListCtrl::OnDuplicateAsset(wxCommandEvent& WXUNUSED(event))
{
    if (m_selected_row < 0)     return;

    const Model_Asset::Data& asset = m_panel->m_assets[m_selected_row];
    Model_Asset::Data* duplicate_asset = Model_Asset::instance().clone(&asset);

    if (EditAsset(duplicate_asset))
    {
        initVirtualListControl();
        doRefreshItems(duplicate_asset->ASSETID);
    }
}

void mmAssetsListCtrl::OnAddAssetTrans(wxCommandEvent& WXUNUSED(event))
{
    if (m_selected_row < 0) return;

    m_panel->AddAssetTrans(m_selected_row);
}

void mmAssetsListCtrl::OnViewAssetTrans(wxCommandEvent& WXUNUSED(event))
{
    if (m_selected_row < 0) return;

    m_panel->ViewAssetTrans(m_selected_row);
}

void mmAssetsListCtrl::OnGotoAssetAccount(wxCommandEvent& WXUNUSED(event))
{
    if (m_selected_row < 0) return;

    m_panel->GotoAssetAccount(m_selected_row);
}

void mmAssetsListCtrl::OnOrganizeAttachments(wxCommandEvent& WXUNUSED(event))
{
    if (m_selected_row < 0) return;

    wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::ASSET);
    int RefId = m_panel->m_assets[m_selected_row].ASSETID;

    mmAttachmentDialog dlg(this, RefType, RefId);
    dlg.ShowModal();

    doRefreshItems(RefId);
}

void mmAssetsListCtrl::OnOpenAttachment(wxCommandEvent& WXUNUSED(event))
{
    if (m_selected_row < 0) return;

    wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::ASSET);
    int RefId = m_panel->m_assets[m_selected_row].ASSETID;

    mmAttachmentManage::OpenAttachmentFromPanelIcon(this, RefType, RefId);
    doRefreshItems(RefId);
}

void mmAssetsListCtrl::OnListItemActivated(wxListEvent& event)
{
    if (m_selected_row < 0)
    {
        m_selected_row = event.GetIndex();
    }
    EditAsset(&(m_panel->m_assets[m_selected_row]));
}

bool mmAssetsListCtrl::EditAsset(Model_Asset::Data* pEntry)
{
    mmAssetDialog dlg(this, m_panel->m_frame, pEntry);
    bool edit = true;
    if (dlg.ShowModal() == wxID_OK)
    {
        doRefreshItems(dlg.m_asset->ASSETID);
        m_panel->updateExtraAssetData(m_selected_row);
    }
    else edit = false;

    return edit;
}

void mmAssetsListCtrl::OnColClick(wxListEvent& event)
{
    int ColumnNr;
    if (event.GetId() != MENU_HEADER_SORT)
         ColumnNr = event.GetColumn();
    else
         ColumnNr = m_ColumnHeaderNbr;
    if (0 > ColumnNr || ColumnNr >= m_panel->col_max() || ColumnNr == 0) return;

    if (m_selected_col == ColumnNr && event.GetId() != MENU_HEADER_SORT) m_asc = !m_asc;

    wxListItem item;
    item.SetMask(wxLIST_MASK_IMAGE);
    item.SetImage(-1);
    SetColumn(m_selected_col, item);

    m_selected_col = ColumnNr;

    item.SetImage(m_asc ? ICON_DOWNARROW : ICON_UPARROW);
    SetColumn(m_selected_col, item);

    Model_Setting::instance().Set("ASSETS_ASC", m_asc);
    Model_Setting::instance().Set("ASSETS_SORT_COL", m_selected_col);

    int trx_id = -1;
    if (m_selected_row>=0) trx_id = m_panel->m_assets[m_selected_row].ASSETID;

    doRefreshItems(trx_id);
}

void mmAssetsListCtrl::OnEndLabelEdit(wxListEvent& event)
{
    if (event.IsEditCancelled()) return;
    Model_Asset::Data* asset = &m_panel->m_assets[event.GetIndex()];
    asset->ASSETNAME = event.m_item.m_text;
    Model_Asset::instance().save(asset);
    RefreshItems(event.GetIndex(), event.GetIndex());
}

int mmAssetsListCtrl::initVirtualListControl(int id, int col, bool asc)
{
    /* Clear all the records */
    DeleteAllItems();

    wxListItem item;
    item.SetMask(wxLIST_MASK_IMAGE);
    item.SetImage(asc ? ICON_DOWNARROW : ICON_UPARROW);
    SetColumn(col, item);

    if (m_panel->m_filter_type == Model_Asset::TYPE(-1)) // ALL
        m_panel->m_assets = Model_Asset::instance().all();
    else
        m_panel->m_assets = Model_Asset::instance().find(Model_Asset::ASSETTYPE(m_panel->m_filter_type));
    m_panel->sortTable();

    SetItemCount(m_panel->m_assets.size());

    double balance = 0.0;
    for (const auto& asset : m_panel->m_assets) balance += Model_Asset::value(asset);
    m_panel->m_header_text->SetLabelText(wxString::Format(_("Total: %s"), Model_Currency::toCurrency(balance))); // balance

    int selected_item = 0;
    for (const auto& asset : m_panel->m_assets)
    {
        if (asset.ASSETID == id) return selected_item;
        ++selected_item;
    }
    return -1;
}

/*******************************************************/
BEGIN_EVENT_TABLE(mmAssetsPanel, wxPanel)
    EVT_BUTTON(wxID_NEW, mmAssetsPanel::OnNewAsset)
    EVT_BUTTON(wxID_EDIT, mmAssetsPanel::OnEditAsset)
    EVT_BUTTON(wxID_ADD, mmAssetsPanel::OnAddAssetTrans)
    EVT_BUTTON(wxID_VIEW_DETAILS , mmAssetsPanel::OnViewAssetTrans)
    EVT_BUTTON(wxID_DELETE, mmAssetsPanel::OnDeleteAsset)
    EVT_BUTTON(wxID_FILE, mmAssetsPanel::OnOpenAttachment)
    EVT_MENU(wxID_ANY, mmAssetsPanel::OnViewPopupSelected)
    EVT_SEARCHCTRL_SEARCH_BTN(wxID_FIND, mmAssetsPanel::OnSearchTxtEntered)
    EVT_TEXT_ENTER(wxID_FIND, mmAssetsPanel::OnSearchTxtEntered)
END_EVENT_TABLE()
/*******************************************************/

mmAssetsPanel::mmAssetsPanel(mmGUIFrame* frame, wxWindow *parent, wxWindowID winid, const wxString& name)
    : m_frame(frame)
    , m_filter_type(Model_Asset::TYPE(-1))
    , m_header_text(nullptr)
    , m_listCtrlAssets(nullptr)
    , m_itemStaticTextMainFilter(nullptr)
    , m_tips()
{
    Create(parent, winid, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, name);
}

bool mmAssetsPanel::Create(wxWindow *parent
    , wxWindowID winid
    , const wxPoint &pos
    , const wxSize &size
    , long style
    , const wxString &name)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    if (!wxPanel::Create(parent, winid, pos, size, style, name)) return false;

    this->windowsFreezeThaw();
    wxDateTime start = wxDateTime::UNow();

    m_tips = _("MMEX allows you to track fixed assets like cars,"
        " houses, land and others. Each asset can have its value"
        " appreciate by a certain rate per year, depreciate by a"
        " certain rate per year, or not change in value."
        " The total assets are added to your total financial worth.");
    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    m_listCtrlAssets->initVirtualListControl(-1
        , m_listCtrlAssets->m_selected_col, m_listCtrlAssets->m_asc);
    if (!this->m_assets.empty())
        m_listCtrlAssets->EnsureVisible(this->m_assets.size() - 1);

    this->windowsFreezeThaw();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    Model_Usage::instance().pageview(this, (wxDateTime::UNow() - start).GetMilliseconds().ToLong());

    return true;
}

void mmAssetsPanel::CreateControls()
{
    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer9);

    wxPanel* headerPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition
        , wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
    itemBoxSizer9->Add(headerPanel, g_flagsBorder1V);

    wxBoxSizer* itemBoxSizerVHeader = new wxBoxSizer(wxVERTICAL);
    headerPanel->SetSizer(itemBoxSizerVHeader);

    wxStaticText* itemStaticText9 = new wxStaticText( headerPanel, wxID_STATIC, _("Assets"));
    itemStaticText9->SetFont(this->GetFont().Larger().Bold());
    itemBoxSizerVHeader->Add(itemStaticText9, g_flagsBorder1V);

    wxBoxSizer* itemBoxSizerHHeader2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizerVHeader->Add(itemBoxSizerHHeader2);

    wxStaticBitmap* itemStaticBitmap3 = new wxStaticBitmap(headerPanel, wxID_STATIC, mmBitmap(png::RIGHTARROW));
    itemBoxSizerHHeader2->Add(itemStaticBitmap3, g_flagsBorder1H);
    //itemStaticBitmap3->Connect(ID_PANEL_CHECKING_STATIC_BITMAP_VIEW, wxEVT_RIGHT_DOWN, wxMouseEventHandler(mmAssetsPanel::OnFilterResetToViewAll), nullptr, this);
    itemStaticBitmap3->Connect(wxID_STATIC, wxEVT_LEFT_DOWN, wxMouseEventHandler(mmAssetsPanel::OnMouseLeftDown), nullptr, this);

    m_itemStaticTextMainFilter = new wxStaticText(headerPanel, wxID_STATIC, _("All"));
    itemBoxSizerHHeader2->Add(m_itemStaticTextMainFilter, 0, wxALIGN_CENTER_VERTICAL | wxALL, 1);

    m_header_text = new wxStaticText(headerPanel, wxID_STATIC, "");
    itemBoxSizerVHeader->Add(m_header_text, g_flagsBorder1V);

    /* ---------------------- */

    wxSplitterWindow* itemSplitterWindow10 = new wxSplitterWindow( this, wxID_STATIC,
        wxDefaultPosition, wxSize(200, 200), wxSP_3DBORDER|wxSP_3DSASH|wxNO_BORDER);

    m_listCtrlAssets = new mmAssetsListCtrl(this, itemSplitterWindow10, wxID_ANY);

    int x = Option::instance().IconSize();
    m_imageList.reset(new wxImageList(x, x));
    m_imageList->Add(mmBitmap(png::PROPERTY));
    m_imageList->Add(mmBitmap(png::CAR));
    m_imageList->Add(mmBitmap(png::HOUSEHOLD_OBJ));
    m_imageList->Add(mmBitmap(png::ART));
    m_imageList->Add(mmBitmap(png::JEWELLERY));
    m_imageList->Add(mmBitmap(png::CASH));
    m_imageList->Add(mmBitmap(png::OTHER));
    m_imageList->Add(mmBitmap(png::UPARROW));
    m_imageList->Add(mmBitmap(png::DOWNARROW));

    m_listCtrlAssets->SetImageList(m_imageList.get(), wxIMAGE_LIST_SMALL);

    wxPanel* assets_panel = new wxPanel(itemSplitterWindow10, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);

    itemSplitterWindow10->SplitHorizontally(m_listCtrlAssets, assets_panel);
    itemSplitterWindow10->SetMinimumPaneSize(100);
    itemSplitterWindow10->SetSashGravity(1.0);
    itemBoxSizer9->Add(itemSplitterWindow10, g_flagsExpandBorder1);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    assets_panel->SetSizer(itemBoxSizer4);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(itemBoxSizer5, g_flagsBorder1V);

    wxButton* itemButton6 = new wxButton( assets_panel, wxID_NEW, _("&New "));
    itemButton6->SetToolTip(_("New Asset"));
    itemBoxSizer5->Add(itemButton6, 0, wxRIGHT, 5);

    wxButton* add_trans_btn = new wxButton(assets_panel, wxID_ADD, _("&Add Trans "));
    add_trans_btn->SetToolTip(_("Add Asset Transaction"));
    itemBoxSizer5->Add(add_trans_btn, 0, wxRIGHT, 5);
    add_trans_btn->Enable(false);

    wxButton* view_trans_btn = new wxButton(assets_panel, wxID_VIEW_DETAILS, _("&View Trans "));
    view_trans_btn->SetToolTip(_("View Asset Transactions"));
    itemBoxSizer5->Add(view_trans_btn, 0, wxRIGHT, 5);
    view_trans_btn->Enable(false);

    wxButton* itemButton81 = new wxButton( assets_panel, wxID_EDIT, _("&Edit "));
    itemButton81->SetToolTip(_("Edit Asset"));
    itemBoxSizer5->Add(itemButton81, 0, wxRIGHT, 5);
    itemButton81->Enable(false);

    wxButton* itemButton7 = new wxButton( assets_panel, wxID_DELETE, _("&Delete "));
    itemButton7->SetToolTip(_("Delete Asset"));
    itemBoxSizer5->Add(itemButton7, 0, wxRIGHT, 5);
    itemButton7->Enable(false);

    wxBitmapButton* attachment_button_ = new wxBitmapButton(assets_panel
        , wxID_FILE, mmBitmap(png::CLIP), wxDefaultPosition
        , wxSize(30, itemButton7->GetSize().GetY()));
    attachment_button_->SetToolTip(_("Open attachments"));
    itemBoxSizer5->Add(attachment_button_, 0, wxRIGHT, 5);
    attachment_button_->Enable(false);

    wxSearchCtrl* searchCtrl = new wxSearchCtrl(assets_panel
        , wxID_FIND, wxEmptyString, wxDefaultPosition
        , wxSize(100, itemButton7->GetSize().GetHeight())
        , wxTE_PROCESS_ENTER, wxDefaultValidator, _("Search"));
    searchCtrl->SetHint(_("Search"));
    itemBoxSizer5->Add(searchCtrl, 0, wxCENTER, 1);
    searchCtrl->SetToolTip(_("Enter any string to find related assets"));

    //Infobar-mini
    wxStaticText* itemStaticText44 = new wxStaticText(assets_panel, IDC_PANEL_ASSET_STATIC_DETAILS_MINI, "");
    itemBoxSizer5->Add(itemStaticText44, 1, wxGROW | wxTOP | wxLEFT, 5);

    //Infobar
    wxStaticText* itemStaticText33 = new wxStaticText(assets_panel
        , IDC_PANEL_ASSET_STATIC_DETAILS, "", wxDefaultPosition, wxSize(200, -1), wxTE_MULTILINE | wxTE_WORDWRAP);
    itemBoxSizer4->Add(itemStaticText33, g_flagsExpandBorder1);

    updateExtraAssetData(-1);
}

void mmAssetsPanel::sortTable()
{
    std::sort(this->m_assets.begin(), this->m_assets.end());
    std::stable_sort(this->m_assets.begin(), this->m_assets.end(), SorterBySTARTDATE());
    switch (this->m_listCtrlAssets->m_selected_col)
    {
    case COL_ID:
        std::stable_sort(this->m_assets.begin(), this->m_assets.end(), SorterByASSETID());
        break;
    case COL_NAME:
        std::stable_sort(this->m_assets.begin(), this->m_assets.end(), SorterByASSETNAME());
        break;
    case COL_TYPE:
        std::stable_sort(this->m_assets.begin(), this->m_assets.end(), SorterByASSETTYPE());
        break;
    case COL_VALUE_INITIAL:
        std::stable_sort(this->m_assets.begin(), this->m_assets.end(), SorterByVALUE());
        break;
    case COL_VALUE_CURRENT:
        std::stable_sort(this->m_assets.begin(), this->m_assets.end()
            , [](const Model_Asset::Data& x, const Model_Asset::Data& y)
            {
                return Model_Asset::value(x) < Model_Asset::value(y);
            });
        break;
    case COL_DATE:
        break;
    case COL_NOTES:
        std::stable_sort(this->m_assets.begin(), this->m_assets.end(), SorterByNOTES());
    default:
        break;
    }

    if (!this->m_listCtrlAssets->m_asc) std::reverse(this->m_assets.begin(), this->m_assets.end());
}

void mmAssetsPanel::OnDeleteAsset(wxCommandEvent& event)
{
    m_listCtrlAssets->OnDeleteAsset(event);
}

void mmAssetsPanel::OnNewAsset(wxCommandEvent& event)
{
    m_listCtrlAssets->OnNewAsset(event);
}

void mmAssetsPanel::OnEditAsset(wxCommandEvent& event)
{
    m_listCtrlAssets->OnEditAsset(event);
}

void mmAssetsPanel::OnAddAssetTrans(wxCommandEvent& event)
{
    m_listCtrlAssets->OnAddAssetTrans(event);
}

void mmAssetsPanel::OnViewAssetTrans(wxCommandEvent& event)
{
    m_listCtrlAssets->OnViewAssetTrans(event);
}

void mmAssetsPanel::OnOpenAttachment(wxCommandEvent& event)
{
    m_listCtrlAssets->OnOpenAttachment(event);
}

wxString mmAssetsPanel::getItem(long item, long column)
{
    const Model_Asset::Data& asset = this->m_assets[item];
    switch (column)
    {
    case COL_ICON:
        return " ";
    case COL_ID:
        return wxString::Format("%i", asset.ASSETID).Trim();
    case COL_NAME:
        return asset.ASSETNAME;
    case COL_TYPE:
        return wxGetTranslation(asset.ASSETTYPE);
    case COL_VALUE_INITIAL:
        return Model_Currency::toCurrency(asset.VALUE);
    case COL_VALUE_CURRENT:
        return Model_Currency::toCurrency(Model_Asset::value(asset));
    case COL_DATE:
        return mmGetDateForDisplay(asset.STARTDATE);
    case COL_NOTES:
    {
        wxString full_notes = asset.NOTES;
        if (Model_Attachment::NrAttachments(Model_Attachment::reftype_desc(Model_Attachment::ASSET), asset.ASSETID))
            full_notes = full_notes.Prepend(mmAttachmentManage::GetAttachmentNoteSign());
        return full_notes;
    }
    default:
        return "";
    }
}

void mmAssetsPanel::updateExtraAssetData(int selIndex)
{
    wxStaticText* st = (wxStaticText*)FindWindow(IDC_PANEL_ASSET_STATIC_DETAILS);
    wxStaticText* stm = (wxStaticText*)FindWindow(IDC_PANEL_ASSET_STATIC_DETAILS_MINI);
    if (selIndex > -1)
    {
        const Model_Asset::Data& asset = this->m_assets[selIndex];
        enableEditDeleteButtons(true);
        const auto& change_rate = (Model_Asset::rate(asset) != Model_Asset::RATE_NONE)
            ? wxString::Format("%.2f %%", asset.VALUECHANGERATE) : "";
        const wxString& miniInfo = " " + wxString::Format(_("Change in Value: %s %s")
            , wxGetTranslation(asset.VALUECHANGE), change_rate);

        st->SetLabelText(asset.NOTES);
        stm->SetLabelText(miniInfo);
    }
    else
    {
        stm->SetLabelText("");
        st->SetLabelText(this->m_tips);
        enableEditDeleteButtons(false);
    }
}

void mmAssetsPanel::enableEditDeleteButtons(bool enable)
{
    wxButton* btn = static_cast<wxButton*>(FindWindow(wxID_EDIT));
    if (btn) btn->Enable(enable);

    btn = static_cast<wxButton*>(FindWindow(wxID_NEW));
    if (btn) btn->Enable(!enable);

    btn = static_cast<wxButton*>(FindWindow(wxID_ADD));
    if (btn) btn->Enable(enable);

    btn = static_cast<wxButton*>(FindWindow(wxID_VIEW_DETAILS));
    if (btn) btn->Enable(enable);

    btn = static_cast<wxButton*>(FindWindow(wxID_DELETE));
    if (btn) btn->Enable(enable);

    btn = static_cast<wxButton*>(FindWindow(wxID_FILE));
    if (btn) btn->Enable(enable);
}

void mmAssetsPanel::OnMouseLeftDown ( wxMouseEvent& event )
{
    int i = 0;
    wxMenu menu;
    menu.Append(++i, wxGetTranslation(wxTRANSLATE("All")));

    for (const auto& type: Model_Asset::all_type())
    {
        menu.Append(++i, wxGetTranslation(type));
    }
    PopupMenu(&menu);

    event.Skip();
}

void mmAssetsPanel::OnViewPopupSelected(wxCommandEvent& event)
{
    int evt = std::max(event.GetId() - 1, 0);

    if (evt == 0)
    {
        m_itemStaticTextMainFilter->SetLabelText(_("All"));
        this->m_filter_type = Model_Asset::TYPE(-1);
    }
    else
    {
        this->m_filter_type = Model_Asset::TYPE(evt - 1);
        m_itemStaticTextMainFilter->SetLabelText(wxGetTranslation(Model_Asset::all_type()[evt - 1]));
    }

    int trx_id = -1;
    m_listCtrlAssets->doRefreshItems(trx_id);
    updateExtraAssetData(trx_id);
}

void mmAssetsPanel::OnSearchTxtEntered(wxCommandEvent& event)
{
    const wxString search_string = event.GetString().Lower();
    if (search_string.IsEmpty()) return;

    long last = m_listCtrlAssets->GetItemCount();
    long selectedItem = m_listCtrlAssets->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selectedItem < 0) //nothing selected
        selectedItem = m_listCtrlAssets->m_asc ? last - 1 : 0;

    while (selectedItem > 0 && selectedItem <= last)
    {
        m_listCtrlAssets->m_asc ? selectedItem-- : selectedItem++;
        const wxString t = getItem(selectedItem, COL_NOTES).Lower();
        if (t.Matches(search_string + "*"))
        {
            //First of all any items should be unselected
            long cursel = m_listCtrlAssets->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            if (cursel != wxNOT_FOUND)
                m_listCtrlAssets->SetItemState(cursel, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);

            //Then finded item will be selected
            m_listCtrlAssets->SetItemState(selectedItem, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            m_listCtrlAssets->EnsureVisible(selectedItem);
            break;
        }
    }
}

void mmAssetsPanel::AddAssetTrans(const int selected_index)
{
    Model_Asset::Data* asset = &m_assets[selected_index];
    mmAssetDialog asset_dialog(this, m_frame, asset, true);
    Model_Account::Data* account = Model_Account::instance().get(asset->ASSETNAME);
    if (account)
    {
        asset_dialog.SetTransactionAccountName(asset->ASSETNAME);
    }
    else
    {
        Model_Translink::Data_Set translist = Model_Translink::TranslinkList(Model_Attachment::ASSET, asset->ASSETID);
        if (!translist.empty())
        {
            wxMessageBox(_(
                "This asset does not have its own account\n\n"
                "Multiple transactions for this asset are not recommended.\n"
                "Please use the popup: 'Open Asset Account' to track the transaction.")
                , _("Asset Management"), wxOK | wxICON_INFORMATION);

            return; // abort process
        }
    }

    if (asset_dialog.ShowModal() == wxID_OK)
    {
        m_listCtrlAssets->doRefreshItems(selected_index);
        updateExtraAssetData(selected_index);
    }
}

void mmAssetsPanel::ViewAssetTrans(const int selected_index)
{
    Model_Asset::Data* asset = &m_assets[selected_index];
    Model_Translink::Data_Set asset_list = Model_Translink::TranslinkList(Model_Attachment::ASSET, asset->ASSETID);

    // TODO create a panel to display all the information on one screen
    wxString msg = _("Account \t Date\t   Value\n\n");
    for (const auto asset_entry : asset_list)
    {
        Model_Checking::Data* asset_trans = Model_Checking::instance().get(asset_entry.CHECKINGACCOUNTID);
        if (asset_trans)
        {
            const auto aa = Model_Account::get_account_name(asset_trans->ACCOUNTID);
            const auto ad = mmGetDateForDisplay(asset_trans->TRANSDATE);
            const auto av = Model_Currency::toString(asset_trans->TRANSAMOUNT); //TODO: check if currency needed
            msg << wxString::Format("%s \t%s   \t%s \n", aa, ad, av);
        }
    }
    wxMessageBox(msg, "Viewing Asset Transactions");
}

void mmAssetsPanel::GotoAssetAccount(const int selected_index)
{
    Model_Asset::Data* asset = &m_assets[selected_index];
    const Model_Account::Data* account = Model_Account::instance().get(asset->ASSETNAME);
    if (account)
    {
        SetAccountParameters(account);
    }
    else
    {
        Model_Translink::Data_Set asset_list = Model_Translink::TranslinkList(Model_Attachment::ASSET, asset->ASSETID);
        for (const auto asset_entry : asset_list)
        {
            Model_Checking::Data* asset_trans = Model_Checking::instance().get(asset_entry.CHECKINGACCOUNTID);
            if (asset_trans)
            {
                account = Model_Account::instance().get(asset_trans->ACCOUNTID);
                SetAccountParameters(account);
            }
        }
    }
}

void mmAssetsPanel::SetAccountParameters(const Model_Account::Data* account)
{
    m_frame->setAccountNavTreeSection(account->ACCOUNTNAME);
    m_frame->setGotoAccountID(account->ACCOUNTID, -1);
    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_GOTOACCOUNT);
    m_frame->GetEventHandler()->AddPendingEvent(evt);
}
