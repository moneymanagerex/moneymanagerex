/*******************************************************
 Copyright (C) 2026

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

#include <algorithm>
#include <unordered_set>
#include <wx/statline.h>
#include "base/_defs.h"
#include "base/_constants.h"
#include "util/mmPath.h"
#include "util/_util.h"
#include "util/mmNavigatorList.h"
#include "model/AccountModel.h"
#include "model/CurrencyModel.h"
#include "model/StockModel.h"
#include "model/PlanEngine.h"
#include "PlanAccountsDialog.h"

namespace
{
    // Only the types the plan can draw on are worth showing: a credit card is a
    // debt rather than an asset, and is handled elsewhere.
    bool type_counts(mmNavigatorItem::TYPE_ID type)
    {
        return type == mmNavigatorItem::TYPE_ID_CASH
            || type == mmNavigatorItem::TYPE_ID_CHECKING
            || type == mmNavigatorItem::TYPE_ID_TERM
            || type == mmNavigatorItem::TYPE_ID_INVESTMENT
            || type == mmNavigatorItem::TYPE_ID_SHARES;
    }

    // Item payload: an account id, or -1 for a type heading.
    class AccountItemData : public wxClientData
    {
    public:
        explicit AccountItemData(int64 id) : m_id(id) {}
        int64 id() const { return m_id; }
    private:
        int64 m_id;
    };
}

wxIMPLEMENT_DYNAMIC_CLASS(PlanAccountsDialog, wxDialog);

wxBEGIN_EVENT_TABLE(PlanAccountsDialog, wxDialog)
    EVT_BUTTON(wxID_OK, PlanAccountsDialog::OnOk)
    EVT_BUTTON(ID_ALL, PlanAccountsDialog::OnAll)
    EVT_BUTTON(ID_NONE, PlanAccountsDialog::OnNone)
    EVT_TREELIST_ITEM_CHECKED(wxID_ANY, PlanAccountsDialog::OnItemChecked)
wxEND_EVENT_TABLE()

PlanAccountsDialog::PlanAccountsDialog()
{
}

PlanAccountsDialog::PlanAccountsDialog(wxWindow* parent)
{
    long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX | wxRESIZE_BORDER;
    Create(parent, wxID_ANY, _t("Plan Accounts"), wxDefaultPosition, wxSize(620, 480), style);
    mmThemeAutoColour(this);
}

bool PlanAccountsDialog::Create(wxWindow* parent, wxWindowID id,
    const wxString& caption, const wxPoint& pos,
    const wxSize& size, long style)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style);

    CreateControls();
    fillControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    this->SetInitialSize();
    SetIcon(mmPath::getProgramIcon());
    Centre();
    return true;
}

void PlanAccountsDialog::CreateControls()
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(mainSizer);

    wxStaticText* hint = new wxStaticText(this, wxID_STATIC, _t(
        "Which accounts count towards the plan? Cleared accounts are left out "
        "of assets and free assets. Unchecking a portfolio also leaves out the "
        "share accounts beneath it."));
    hint->Wrap(560);
    mainSizer->Add(hint, 0, wxALL, 8);

    // A three-state tree: checking a heading takes its accounts with it, and a
    // partly selected heading is shown as such rather than as on or off.
    m_tree = new wxTreeListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(580, 300),
        wxTL_CHECKBOX | wxTL_3STATE | wxTL_NO_HEADER | wxTL_SINGLE);
    m_tree->AppendColumn(_t("Account"), 300);
    m_tree->AppendColumn(_t("Balance"), 140, wxALIGN_RIGHT);
    mainSizer->Add(m_tree, 1, wxGROW | wxALL, 5);

    m_summary = new wxStaticText(this, wxID_STATIC, "");
    mainSizer->Add(m_summary, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);

    wxBoxSizer* buttonRow = new wxBoxSizer(wxHORIZONTAL);
    mainSizer->Add(buttonRow, 0, wxGROW | wxALL, 5);

    wxButton* btnAll = new wxButton(this, ID_ALL, _t("Select &All"));
    buttonRow->Add(btnAll, 0, wxALIGN_CENTER_VERTICAL);
    wxButton* btnNone = new wxButton(this, ID_NONE, _t("Select &None"));
    buttonRow->Add(btnNone, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, 5);

    wxStaticLine* line = new wxStaticLine(this, wxID_STATIC,
        wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    mainSizer->Add(line, 0, wxGROW | wxALL, 5);

    wxPanel* btnPanel = new wxPanel(this, wxID_ANY);
    mainSizer->Add(btnPanel, 0, wxALIGN_RIGHT, 5);
    wxBoxSizer* okSizer = new wxBoxSizer(wxHORIZONTAL);
    btnPanel->SetSizer(okSizer);
    wxButton* btnOK = new wxButton(btnPanel, wxID_OK, _t("&OK "));
    okSizer->Add(btnOK, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxBOTTOM, 5);
    wxButton* btnCancel = new wxButton(btnPanel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    okSizer->Add(btnCancel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT | wxBOTTOM, 5);
}

void PlanAccountsDialog::fillControls()
{
    AccountModel& am = AccountModel::instance();
    CurrencyModel& cm = CurrencyModel::instance();
    StockModel& sm = StockModel::instance();

    const wxTreeListItem root = m_tree->GetRootItem();

    // Group by account type, in the same order the navigation rail uses, so the
    // two read the same way.
    std::map<int, wxTreeListItem> sections;

    AccountModel::DataA accounts = am.find_data_a();
    std::sort(accounts.begin(), accounts.end(),
        [](const AccountData& x, const AccountData& y) {
            return x.m_name.CmpNoCase(y.m_name) < 0;
        });

    // Share accounts appear beneath their portfolio, so they must not also be
    // listed as top-level entries.
    std::unordered_set<long long> nested_share_ids;
    for (const auto& acc_d : accounts) {
        if (AccountModel::type_id(acc_d) != mmNavigatorItem::TYPE_ID_INVESTMENT)
            continue;
        for (const auto& stock : sm.find_data_a(
            StockCol::WHERE_HELDAT(OP_EQ, acc_d.m_id)
        )) {
            const AccountData* share_n = am.get_name_data_n(stock.m_name);
            if (share_n)
                nested_share_ids.insert(share_n->m_id.GetValue());
        }
    }

    auto add_account = [&](const wxTreeListItem& parent, const AccountData& acc_d) {
        double bal = am.get_data_balance(acc_d);
        const CurrencyData* curr_n = cm.get_idN_data_n(acc_d.m_currency_id);
        if (curr_n && curr_n->m_base_conv_rate > 0.0)
            bal *= curr_n->m_base_conv_rate;

        wxTreeListItem item = m_tree->AppendItem(parent, acc_d.m_name,
            wxTreeListCtrl::NO_IMAGE, wxTreeListCtrl::NO_IMAGE,
            new AccountItemData(acc_d.m_id));
        m_tree->SetItemText(item, 1, cm.toCurrency(bal));
        m_tree->CheckItem(item, PlanEngine::account_is_included(acc_d.m_id)
            ? wxCHK_CHECKED : wxCHK_UNCHECKED);

        m_all_account_id_a.push_back(acc_d.m_id);
        return item;
    };

    for (const auto& acc_d : accounts) {
        if (acc_d.m_status.key() != "Open")
            continue;

        const mmNavigatorItem::TYPE_ID type = AccountModel::type_id(acc_d);
        if (!type_counts(type))
            continue;

        // Skip share accounts here; they are added under their portfolio below.
        if (type == mmNavigatorItem::TYPE_ID_SHARES &&
            nested_share_ids.count(acc_d.m_id.GetValue()))
            continue;

        if (sections.find(type) == sections.end()) {
            sections[type] = m_tree->AppendItem(root,
                mmNavigatorList::instance().type_name(type),
                wxTreeListCtrl::NO_IMAGE, wxTreeListCtrl::NO_IMAGE,
                new AccountItemData(-1));
        }

        wxTreeListItem item = add_account(sections[type], acc_d);

        if (type != mmNavigatorItem::TYPE_ID_INVESTMENT)
            continue;

        // A portfolio owns its share accounts, mirroring the navigation rail.
        StockModel::DataA stocks = sm.find_data_a(
            StockCol::WHERE_HELDAT(OP_EQ, acc_d.m_id)
        );
        std::sort(stocks.begin(), stocks.end(), StockData::SorterBySTOCKNAME());

        std::unordered_set<wxString> seen;
        for (const auto& stock : stocks) {
            if (!seen.insert(stock.m_name).second)
                continue;
            const AccountData* share_n = am.get_name_data_n(stock.m_name);
            if (!share_n || share_n->m_status.key() != "Open")
                continue;
            add_account(item, *share_n);
        }
    }

    // A heading follows its children, so a partly-selected type shows as such.
    for (const auto& entry : sections)
        m_tree->UpdateItemParentStateRecursively(m_tree->GetFirstChild(entry.second));

    // wxTreeListCtrl has no ExpandAll, so expand every node that has children:
    // the type headings and the portfolios holding share accounts.
    for (wxTreeListItem item = m_tree->GetFirstItem();
         item.IsOk();
         item = m_tree->GetNextItem(item))
    {
        if (m_tree->GetFirstChild(item).IsOk())
            m_tree->Expand(item);
    }

    updateSummary();
}

void PlanAccountsDialog::collectChecked(
    const wxTreeListItem& parent, std::vector<int64>& out
) const {
    for (wxTreeListItem item = m_tree->GetFirstChild(parent);
         item.IsOk();
         item = m_tree->GetNextSibling(item))
    {
        const AccountItemData* data =
            static_cast<AccountItemData*>(m_tree->GetItemData(item));
        if (data && data->id() > 0 && m_tree->GetCheckedState(item) == wxCHK_CHECKED)
            out.push_back(data->id());

        collectChecked(item, out);
    }
}

void PlanAccountsDialog::setAllChecked(bool checked)
{
    for (wxTreeListItem item = m_tree->GetFirstItem();
         item.IsOk();
         item = m_tree->GetNextItem(item))
    {
        m_tree->CheckItem(item, checked ? wxCHK_CHECKED : wxCHK_UNCHECKED);
    }
    updateSummary();
}

void PlanAccountsDialog::updateSummary()
{
    std::vector<int64> checked;
    collectChecked(m_tree->GetRootItem(), checked);

    // Say what the choice adds up to, since that is the number it feeds.
    AccountModel& am = AccountModel::instance();
    CurrencyModel& cm = CurrencyModel::instance();

    double total = 0.0;
    for (const int64 id : checked) {
        const AccountData* acc_n = am.get_idN_data_n(id);
        if (!acc_n)
            continue;
        double bal = am.get_data_balance(*acc_n);
        const CurrencyData* curr_n = cm.get_idN_data_n(acc_n->m_currency_id);
        if (curr_n && curr_n->m_base_conv_rate > 0.0)
            bal *= curr_n->m_base_conv_rate;
        total += bal;
    }

    m_summary->SetLabel(wxString::Format(
        wxPLURAL("%d account selected, totalling %s",
                 "%d accounts selected, totalling %s",
                 static_cast<int>(checked.size())),
        static_cast<int>(checked.size()), cm.toCurrency(total)));
    Layout();
}

void PlanAccountsDialog::OnItemChecked(wxTreeListEvent& event)
{
    // Checking a heading applies to everything under it, and a heading reflects
    // whatever its children now are.
    m_tree->CheckItemRecursively(event.GetItem(),
        m_tree->GetCheckedState(event.GetItem()));
    m_tree->UpdateItemParentStateRecursively(event.GetItem());
    updateSummary();
}

void PlanAccountsDialog::OnAll(wxCommandEvent& /*event*/)
{
    setAllChecked(true);
}

void PlanAccountsDialog::OnNone(wxCommandEvent& /*event*/)
{
    setAllChecked(false);
}

void PlanAccountsDialog::OnOk(wxCommandEvent& /*event*/)
{
    std::vector<int64> checked;
    collectChecked(m_tree->GetRootItem(), checked);

    // Stored as exclusions so an account created later counts by default rather
    // than going silently missing from the plan.
    std::vector<int64> excluded;
    for (const int64 id : m_all_account_id_a) {
        if (std::find(checked.begin(), checked.end(), id) == checked.end())
            excluded.push_back(id);
    }

    PlanEngine::set_excluded_account_id_a(excluded);
    EndModal(wxID_OK);
}
