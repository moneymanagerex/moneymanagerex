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

BEGIN_EVENT_TABLE(AssetPanel, wxPanel)
    EVT_BUTTON(wxID_NEW,                 AssetPanel::onNewAsset)
    EVT_BUTTON(wxID_EDIT,                AssetPanel::onEditAsset)
    EVT_BUTTON(wxID_ADD,                 AssetPanel::onAddAssetTrans)
    EVT_BUTTON(wxID_VIEW_DETAILS ,       AssetPanel::onViewAssetTrans)
    EVT_BUTTON(wxID_DELETE,              AssetPanel::onDeleteAsset)
    EVT_BUTTON(wxID_FILE,                AssetPanel::onOpenAttachment)
    EVT_BUTTON(wxID_FILE2,               AssetPanel::onMouseLeftDown)
    EVT_MENU(wxID_ANY,                   AssetPanel::onViewPopupSelected)
    EVT_SEARCHCTRL_SEARCH_BTN(wxID_FIND, AssetPanel::onSearchTxtEntered)
END_EVENT_TABLE()

AssetPanel::AssetPanel(
    mmGUIFrame* frame, wxWindow* parent_win, wxWindowID win_id, const wxString& name
) :
    m_asset_type_id_n(-1),
    m_tip(),
    w_frame(frame)
{
    create(parent_win, win_id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, name);
    mmThemeAutoColour(this);
}

bool AssetPanel::create(
    wxWindow* parent_win,
    wxWindowID win_id,
    const wxPoint &pos,
    const wxSize &size,
    long style,
    const wxString &name
) {
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    if (!wxPanel::Create(parent_win, win_id, pos, size, style, name))
        return false;

    this->windowsFreezeThaw();

    auto start = wxDateTime::UNow();
    m_tip = _t("MMEX allows fixed assets like cars, houses, land, and others to be tracked. Each asset can have its value appreciate by a certain rate per year, depreciate by a certain rate per year, or not change in value. The total assets are added to your total financial worth.");
    createControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    initVirtualListControl();
    if (!this->m_asset_a.empty())
        w_list->EnsureVisible(this->m_asset_a.size() - 1);

    this->windowsFreezeThaw();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    UsageModel::instance().pageview(this,
        (wxDateTime::UNow() - start).GetMilliseconds().ToLong()
    );

    return true;
}

void AssetPanel::createControls()
{
    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer9);

    wxPanel* headerPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition
        , wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
    itemBoxSizer9->Add(headerPanel, g_flagsBorder1V);

    wxBoxSizer* itemBoxSizerVHeader = new wxBoxSizer(wxVERTICAL);
    headerPanel->SetSizer(itemBoxSizerVHeader);

    wxStaticText* itemStaticText9 = new wxStaticText(headerPanel, wxID_STATIC, _t("Assets"));
    itemStaticText9->SetFont(this->GetFont().Larger().Bold());
    itemBoxSizerVHeader->Add(itemStaticText9, g_flagsBorder1V);

    wxBoxSizer* itemBoxSizerHHeader2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizerVHeader->Add(itemBoxSizerHHeader2);

    w_filter_btn = new wxButton(headerPanel, wxID_FILE2);
    w_filter_btn->SetBitmap(mmBitmapBundle(png::TRANSFILTER, mmBitmapButtonSize));
    w_filter_btn->SetLabel(_t("All"));
    w_filter_btn->SetMinSize(wxSize(150, -1));
    itemBoxSizerHHeader2->Add(w_filter_btn, g_flagsBorder1H);

    w_header = new wxStaticText(headerPanel, wxID_STATIC, "");
    itemBoxSizerVHeader->Add(w_header, g_flagsBorder1V);

    /* ---------------------- */

    mmSplitterWindow* itemSplitterWindow10 = new mmSplitterWindow( this, wxID_STATIC,
        wxDefaultPosition, wxSize(200, 200),
        wxSP_3DBORDER | wxSP_3DSASH | wxNO_BORDER,
        mmThemeMetaColour(meta::COLOR_LISTPANEL)
    );

    w_list = new AssetList(this, itemSplitterWindow10, wxID_ANY);

    wxVector<wxBitmapBundle> images;
    images.push_back(mmBitmapBundle(png::PROPERTY));
    images.push_back(mmBitmapBundle(png::CAR));
    images.push_back(mmBitmapBundle(png::HOUSEHOLD_OBJ));
    images.push_back(mmBitmapBundle(png::ART));
    images.push_back(mmBitmapBundle(png::JEWELLERY));
    images.push_back(mmBitmapBundle(png::CASH));
    images.push_back(mmBitmapBundle(png::OTHER));
    images.push_back(mmBitmapBundle(png::UPARROW));
    images.push_back(mmBitmapBundle(png::DOWNARROW));

    w_list->SetSmallImages(images);

    wxPanel* assets_panel = new wxPanel(itemSplitterWindow10,
        wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxNO_BORDER | wxTAB_TRAVERSAL
    );
    itemSplitterWindow10->SplitHorizontally(w_list, assets_panel);
    itemSplitterWindow10->SetMinimumPaneSize(100);
    itemSplitterWindow10->SetSashGravity(1.0);
    itemBoxSizer9->Add(itemSplitterWindow10, g_flagsExpandBorder1);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    assets_panel->SetSizer(itemBoxSizer4);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizer4->Add(itemBoxSizer5, g_flagsBorder1V);

    wxButton* itemButton6 = new wxButton( assets_panel,
        wxID_NEW, _t("&New ")
    );
    mmToolTip(itemButton6, _t("New Asset"));
    itemBoxSizer5->Add(itemButton6, 0, wxRIGHT, 5);

    wxButton* add_trans_btn = new wxButton(assets_panel,
        wxID_ADD, _t("&Add Trans ")
    );
    mmToolTip(add_trans_btn, _t("Add Asset Transaction"));
    itemBoxSizer5->Add(add_trans_btn, 0, wxRIGHT, 5);
    add_trans_btn->Enable(false);

    wxButton* view_trans_btn = new wxButton(assets_panel,
        wxID_VIEW_DETAILS, _t("&View Trans ")
    );
    mmToolTip(view_trans_btn, _t("View Asset Transactions"));
    itemBoxSizer5->Add(view_trans_btn, 0, wxRIGHT, 5);
    view_trans_btn->Enable(false);

    wxButton* itemButton81 = new wxButton( assets_panel, wxID_EDIT, _t("&Edit "));
    mmToolTip(itemButton81, _t("Edit Asset"));
    itemBoxSizer5->Add(itemButton81, 0, wxRIGHT, 5);
    itemButton81->Enable(false);

    wxButton* itemButton7 = new wxButton( assets_panel, wxID_DELETE, _t("&Delete "));
    mmToolTip(itemButton7, _t("Delete Asset"));
    itemBoxSizer5->Add(itemButton7, 0, wxRIGHT, 5);
    itemButton7->Enable(false);

    wxBitmapButton* attachment_button_ = new wxBitmapButton(assets_panel,
        wxID_FILE, mmBitmapBundle(png::CLIP, mmBitmapButtonSize), wxDefaultPosition,
        wxSize(30, itemButton7->GetSize().GetY())
    );
    mmToolTip(attachment_button_, _t("Open attachments"));
    itemBoxSizer5->Add(attachment_button_, 0, wxRIGHT, 5);
    attachment_button_->Enable(false);

    wxSearchCtrl* searchCtrl = new wxSearchCtrl(assets_panel,
        wxID_FIND, wxEmptyString, wxDefaultPosition,
        wxSize(100, itemButton7->GetSize().GetHeight()),
        wxTE_PROCESS_ENTER, wxDefaultValidator, _t("Search")
    );
    searchCtrl->SetHint(_t("Search"));
    itemBoxSizer5->Add(searchCtrl, 0, wxCENTER, 1);
    mmToolTip(searchCtrl,
        _t("Enter any string to find related assets") + "\n\n" +
        _tu("Tip: Wildcard characters such as the question mark (?) and the asterisk (*) can be used in search criteria.") + "\n" +
        _tu("The question mark (?) is used to match a single character, for example \"s?t\" finds both \"sat\" and \"set\".") + "\n" +
        _tu("The asterisk (*) is used to match any number of characters, for example \"s*d\" finds both \"sad\" and \"started\".")  + "\n" +
        _tu("Use the asterisk (*) at the beginning to find any string in the middle of the sentence.")
    );

    //Infobar-mini
    wxStaticText* itemStaticText44 = new wxStaticText(assets_panel,
        IDC_PANEL_ASSET_STATIC_DETAILS_MINI, ""
    );
    itemBoxSizer5->Add(itemStaticText44, 1, wxGROW | wxTOP | wxLEFT, 5);

    //Infobar
    wxStaticText* itemStaticText33 = new wxStaticText(assets_panel,
        IDC_PANEL_ASSET_STATIC_DETAILS, "",
        wxDefaultPosition, wxSize(200, -1),
        wxTE_MULTILINE | wxTE_WORDWRAP
    );
    itemBoxSizer4->Add(itemStaticText33, g_flagsExpandBorder1);

    updateExtraAssetData(-1);
}

void AssetPanel::sortList()
{
    std::sort(m_asset_a.begin(), m_asset_a.end());
    std::stable_sort(m_asset_a.begin(), m_asset_a.end(),
        AssetData::SorterBySTARTDATE()
    );
    switch (this->w_list->getSortColId())
    {
    case AssetList::LIST_ID_ID:
        std::stable_sort(m_asset_a.begin(), m_asset_a.end(),
            AssetData::SorterByASSETID()
        );
        break;
    case AssetList::LIST_ID_NAME:
        std::stable_sort(m_asset_a.begin(), m_asset_a.end(),
            AssetData::SorterByASSETNAME()
        );
        break;
    case AssetList::LIST_ID_TYPE:
        std::stable_sort(m_asset_a.begin(), m_asset_a.end(),
            AssetData::SorterByASSETTYPE()
        );
        break;
    case AssetList::LIST_ID_VALUE_INITIAL:
        std::stable_sort(m_asset_a.begin(), m_asset_a.end(),
            [](const AssetData& x, const AssetData& y) {
                double x_value = AssetModel::instance().get_data_value(x).first;
                double y_value = AssetModel::instance().get_data_value(y).first;
                return x_value < y_value;
            }
        );
        break;
    case AssetList::LIST_ID_VALUE_CURRENT:
        std::stable_sort(m_asset_a.begin(), m_asset_a.end(),
            [](const AssetData& x, const AssetData& y) {
                double x_value = AssetModel::instance().get_data_value(x).second;
                double y_value = AssetModel::instance().get_data_value(y).second;
                return x_value < y_value;
            });
        break;
    case AssetList::LIST_ID_DATE:
        break;
    case AssetList::LIST_ID_NOTES:
        std::stable_sort(m_asset_a.begin(), m_asset_a.end(),
            AssetData::SorterByNOTES()
        );
    default:
        break;
    }

    if (!w_list->getSortAsc())
        std::reverse(this->m_asset_a.begin(), this->m_asset_a.end());
}

int AssetPanel::initVirtualListControl(int64 id)
{
    /* Clear all the records */
    w_list->DeleteAllItems();

    m_asset_a = (m_asset_type_id_n == -1)
        ? AssetModel::instance().find_all()
        : AssetModel::instance().find(
            AssetModel::ASSETTYPE(OP_EQ, AssetType(m_asset_type_id_n))
        );
    sortList();

    w_list->SetItemCount(this->m_asset_a.size());

    double initial = 0.0, balance = 0.0;
    for (const auto& asset_d: this->m_asset_a) {
        auto bal = AssetModel::instance().get_data_value(asset_d);
        initial += bal.first;
        balance += bal.second;
    }
    w_header->SetLabelText(wxString::Format("%s, %s",
        wxString::Format(_t("Total: %s"), CurrencyModel::instance().toCurrency(balance)),
        wxString::Format(_t("Initial: %s"), CurrencyModel::instance().toCurrency(initial))
    ));

    int selected_item = 0;
    for (const auto& asset_d: this->m_asset_a) {
        if (asset_d.m_id == id)
            return selected_item;
        ++ selected_item;
    }
    return -1;
}

wxString AssetPanel::getItem(long item, int col_id)
{
    const AssetData& asset = this->m_asset_a[item];
    switch (col_id) {
    case AssetList::LIST_ID_ICON:
        return "";
    case AssetList::LIST_ID_ID:
        return wxString::Format("%lld", asset.m_id).Trim();
    case AssetList::LIST_ID_NAME:
        return asset.m_name;
    case AssetList::LIST_ID_TYPE:
        return wxGetTranslation(asset.m_type.name());
    case AssetList::LIST_ID_VALUE_INITIAL:
        return CurrencyModel::instance().toCurrency(
            AssetModel::instance().get_data_value(asset).first
        );
    case AssetList::LIST_ID_VALUE_CURRENT:
        return CurrencyModel::instance().toCurrency(
            AssetModel::instance().get_data_value(asset).second
        );
    case AssetList::LIST_ID_DATE:
        return mmGetDateTimeForDisplay(asset.m_start_date.isoDate());
    case AssetList::LIST_ID_NOTES: {
        wxString full_notes = asset.m_notes;
        full_notes.Replace("\n", " ");
        if (AttachmentModel::instance().find_ref_c(AssetModel::s_ref_type, asset.m_id))
            full_notes = full_notes.Prepend(mmAttachmentManage::GetAttachmentNoteSign());
        return full_notes;
    }
    default:
        return "";
    }
}

void AssetPanel::updateExtraAssetData(int selIndex)
{
    wxStaticText* st = static_cast<wxStaticText*>(FindWindow(IDC_PANEL_ASSET_STATIC_DETAILS));
    wxStaticText* stm = static_cast<wxStaticText*>(FindWindow(IDC_PANEL_ASSET_STATIC_DETAILS_MINI));
    if (selIndex > -1) {
        const AssetData& asset = this->m_asset_a[selIndex];
        enableEditDeleteButtons(true);
        const auto& change_rate = (asset.m_change.id() != AssetChange::e_none)
            ? wxString::Format("%.2f %%", asset.m_change_rate) : "";
        const wxString& miniInfo = " " + wxString::Format(_t("Change in Value: %1$s %2$s"),
            wxGetTranslation(asset.m_change.name()), change_rate
        );

        st->SetLabelText(asset.m_notes);
        stm->SetLabelText(miniInfo);
    }
    else {
        stm->SetLabelText("");
        st->SetLabelText(this->m_tip);
        enableEditDeleteButtons(false);
    }
}

void AssetPanel::enableEditDeleteButtons(bool enable)
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

void AssetPanel::onMouseLeftDown (wxCommandEvent& event)
{
    int i = 0;
    wxMenu menu;
    menu.Append(++i, _t("All"));

    for (int typeId = 0; typeId < AssetType::size; ++typeId) {
        wxString type = AssetType(typeId).name();
        menu.Append(++i, wxGetTranslation(type));
    }
    PopupMenu(&menu);

    event.Skip();
}

void AssetPanel::onViewPopupSelected(wxCommandEvent& event)
{
    int evt = std::max(event.GetId() - 1, 0);
    m_asset_type_id_n = evt - 1;

    if (m_asset_type_id_n == -1) {
        w_filter_btn->SetLabel(_t("All"));
        w_filter_btn->SetBitmap(mmBitmapBundle(png::TRANSFILTER, mmBitmapButtonSize));
    }
    else {
        w_filter_btn->SetBitmap(mmBitmapBundle(png::TRANSFILTER_ACTIVE, mmBitmapButtonSize));
        w_filter_btn->SetLabel(wxGetTranslation(AssetType(m_asset_type_id_n).name()));
    }

    int64 trx_id = -1;
    w_list->doRefreshItems(trx_id);
    updateExtraAssetData(-1);
}

void AssetPanel::onSearchTxtEntered(wxCommandEvent& event)
{
    const wxString search_string = event.GetString().Lower();
    if (search_string.IsEmpty()) return;

    long last = w_list->GetItemCount();
    if (last == 0) return;

    long selectedItem = w_list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selectedItem == wxNOT_FOUND)
        selectedItem = 0;
    else
        selectedItem = (selectedItem + 1) % last;

    long startItem = selectedItem;

    SetEvtHandlerEnabled(false);

    do {
        const wxString t = getItem(selectedItem, AssetList::LIST_ID_NOTES).Lower();
        if (t.Contains(search_string))
        {
            long cursel = w_list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            if (cursel != wxNOT_FOUND)
                w_list->SetItemState(cursel, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);

            w_list->SetItemState(selectedItem, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED,
                                              wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
            w_list->EnsureVisible(selectedItem);
            break;
        }

        selectedItem = (selectedItem + 1) % last;

    } while (selectedItem != startItem);

    SetEvtHandlerEnabled(true);
}

void AssetPanel::addAssetTrans(const int selected_index)
{
    AssetData* asset = &m_asset_a[selected_index];
    AssetDialog asset_dialog(this, asset, true);
    const AccountData* account = AccountModel::instance().get_name_data_n(asset->m_name);
    const AccountData* account2 = AccountModel::instance().get_name_data_n(asset->m_type.name());
    if (account || account2) {
        asset_dialog.SetTransactionAccountName(account ? asset->m_name : asset->m_type.name());
    }
    else {
        TrxLinkModel::DataA tl_a = TrxLinkModel::instance().find_ref_data_a(
            AssetModel::s_ref_type, asset->m_id
        );
        if (tl_a.empty()) {
            wxMessageBox(
                _t("This asset does not have its own account\n\n"
                    "Multiple transactions for this asset are not recommended."
                ),
                _t("Asset Management"),
                wxOK | wxICON_INFORMATION
            );
            return; // abort process
        }
    }

    if (asset_dialog.ShowModal() == wxID_OK) {
        w_list->doRefreshItems(selected_index);
        updateExtraAssetData(selected_index);
    }
}

void AssetPanel::viewAssetTrans(int selectedIndex)
{
    AssetData* asset = &m_asset_a[selectedIndex];

    wxDialog dlg(this, wxID_ANY,
                 _t("View Asset Transactions") + ": " + asset->m_name,
                 wxDefaultPosition, wxDefaultSize,
                 wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

    dlg.SetIcon(mmex::getProgramIcon());
    wxWindow* parent = dlg.GetMainWindowOfCompositeControl();
    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);

    // Initialize list control
    wxListCtrl* assetTxnListCtrl = this->initAssetTxnListCtrl(parent);
    topsizer->Add(assetTxnListCtrl, wxSizerFlags(g_flagsExpand).TripleBorder());

    // Bind events here
    bindAssetListEvents(assetTxnListCtrl);

    // Load asset transactions
    loadAssetTransactions(assetTxnListCtrl, asset->m_id);

    // Add buttons
    wxSizer* buttonSizer = dlg.CreateSeparatedButtonSizer(wxOK);
    if (buttonSizer) {
        topsizer->Add(buttonSizer, wxSizerFlags().Expand().DoubleBorder(wxLEFT | wxRIGHT | wxBOTTOM));
    }

    dlg.SetSizerAndFit(topsizer);
    dlg.SetInitialSize(wxSize(600, 400)); // Set default size
    dlg.Center();
    dlg.ShowModal();
}

// Initialize the list control
wxListCtrl* AssetPanel::initAssetTxnListCtrl(wxWindow* parent)
{
    wxListCtrl* listCtrl = new wxListCtrl(
        parent, wxID_ANY,
        wxDefaultPosition, wxDefaultSize,
        wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_AUTOARRANGE
    );

    listCtrl->AppendColumn(_t("Account"),    wxLIST_FORMAT_LEFT,  120);
    listCtrl->AppendColumn(_t("Date"),       wxLIST_FORMAT_LEFT,  100);
    listCtrl->AppendColumn(_t("Trade Type"), wxLIST_FORMAT_LEFT,  100);
    listCtrl->AppendColumn(_t("Value"),      wxLIST_FORMAT_RIGHT, 120);

    return listCtrl;
}

// Load asset transactions into the list control
void AssetPanel::loadAssetTransactions(wxListCtrl* listCtrl, int64 asset_id)
{
    TrxLinkModel::DataA tl_a = TrxLinkModel::instance().find_ref_data_a(
        AssetModel::s_ref_type, asset_id
    );

    int row = 0;
    for (const auto& tl_d : tl_a) {
        const TrxData* trx_n = TrxModel::instance().get_id_data_n(tl_d.m_trx_id);
        if (!trx_n)
            continue;

        long index = listCtrl->InsertItem(row++, "");
        listCtrl->SetItemData(index, trx_n->m_id.GetValue());
        fillAssetListRow(listCtrl, index, *trx_n);
    }
}

void AssetPanel::fillAssetListRow(wxListCtrl* listCtrl, long index, const TrxData& trx_d)
{
    listCtrl->SetItem(index, 0, AccountModel::instance().get_id_name(trx_d.m_account_id));
    listCtrl->SetItem(index, 1, mmGetDateTimeForDisplay(trx_d.m_date_time.isoDateTime()));
    listCtrl->SetItem(index, 2, trx_d.m_type.trade_name());
    listCtrl->SetItem(index, 3, CurrencyModel::instance().toString(trx_d.m_amount));
//    listCtrl->SetItem(index, 3, CurrencyModel::instance().get_currency_symbol(trx_d.CURRENCYID));
}

void AssetPanel::bindAssetListEvents(wxListCtrl* listCtrl)
{
    listCtrl->Bind(wxEVT_LIST_ITEM_ACTIVATED, [listCtrl, this](wxListEvent& event) {
        long index = event.GetIndex();
        TrxData* trx_n = TrxModel::instance().unsafe_get_id_data_n(event.GetData());
        if (!trx_n)
            return;

        const TrxLinkData* tl_n = TrxLinkModel::instance().get_trx_data_n(trx_n->m_id);
        TrxLinkData tl_d = tl_n ? *tl_n : TrxLinkData();
        AssetDialog dlg(listCtrl, &tl_d, trx_n);
        dlg.ShowModal();

        this->fillAssetListRow(listCtrl, index, *trx_n);

        // FIXME: change type to int64
        listCtrl->SortItems([](wxIntPtr item1, wxIntPtr item2, wxIntPtr) -> int {
            auto date1 = TrxModel::instance().get_id_data_n(item1)->m_date_time.getDateTime();
            auto date2 = TrxModel::instance().get_id_data_n(item2)->m_date_time.getDateTime();
            return date1.IsEarlierThan(date2) ? -1 : (date1.IsLaterThan(date2) ? 1 : 0);
        }, 0);
    });

    listCtrl->Bind(wxEVT_CHAR, [listCtrl, this](wxKeyEvent& event) {
        if (event.GetKeyCode() == WXK_CONTROL_C) {
            copySelectedRowsToClipboard(listCtrl);
        } else if (event.GetKeyCode() == WXK_CONTROL_A) {
            for (int row = 0; row < listCtrl->GetItemCount(); row++)
                listCtrl->SetItemState(row, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        }
    });
}

void AssetPanel::copySelectedRowsToClipboard(wxListCtrl* listCtrl)
{
    if (!wxTheClipboard->Open()) return;

    wxString data;
    const wxString separator = "\t";

    for (int row = 0; row < listCtrl->GetItemCount(); row++) {
        if (listCtrl->GetItemState(row, wxLIST_STATE_SELECTED) != wxLIST_STATE_SELECTED)
            continue;

        for (int col = 0; col < listCtrl->GetColumnCount(); col++) {
            if (listCtrl->GetColumnWidth(col) > 0) {
                data += listCtrl->GetItemText(row, col) + separator;
            }
        }
        data += "\n";
    }

    wxTheClipboard->SetData(new wxTextDataObject(data));
    wxTheClipboard->Close();
}

void AssetPanel::gotoAssetAccount(const int selected_index)
{
    AssetData* asset_n = &m_asset_a[selected_index];
    const AccountData* account_n = AccountModel::instance().get_name_data_n(asset_n->m_name);
    if (account_n) {
        setAccountParameters(account_n);
    }
    else {
        TrxLinkModel::DataA tl_a = TrxLinkModel::instance().find_ref_data_a(
            AssetModel::s_ref_type, asset_n->m_id
        );
        for (const auto& _tl_a : tl_a) {
            const TrxData* trx_n = TrxModel::instance().get_id_data_n(_tl_a.m_trx_id);
            if (trx_n) {
                account_n = AccountModel::instance().get_id_data_n(trx_n->m_account_id);
                setAccountParameters(account_n);
            }
        }
    }
}

void AssetPanel::setAccountParameters(const AccountData* account_n)
{
    w_frame->selectNavTreeItem(account_n->m_name);
    w_frame->setGotoAccountID(account_n->m_id);
    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_GOTOACCOUNT);
    w_frame->GetEventHandler()->AddPendingEvent(evt);
}
