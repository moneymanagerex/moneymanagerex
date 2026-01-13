/*******************************************************
 Copyright (C) 2025 Klaus Wich

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

#include "navigatortypes.h"

#include "singleton.h"
#include "images_list.h"

#include "model/Model_Account.h"

NavigatorTypes::NavigatorTypes()
{
    SetToDefault();
}

NavigatorTypes& NavigatorTypes::instance()
{
    return Singleton<NavigatorTypes>::instance();
}

void NavigatorTypes::SetToDefault()
{
    m_navigator_entries = {  // init with default entries
        { NAV_ENTRY_DASHBOARD,              _nu("Dashboard"),              "",                -1, img::HOUSE_PNG,              NAV_TYP_PANEL_STATIC, true},
        { NAV_ENTRY_ALL_TRANSACTIONS,       _nu("All Transactions"),       "",                -1, img::ALLTRANSACTIONS_PNG,    NAV_TYP_PANEL,        true},
        { NAV_ENTRY_SCHEDULED_TRANSACTIONS, _nu("Scheduled Transactions"), "",                -1, img::SCHEDULE_PNG,           NAV_TYP_PANEL,        true},
        { NAV_ENTRY_FAVORITES,              _nu("Favorites"),              "",                -1, img::FAVOURITE_PNG,          NAV_TYP_PANEL,        true},

        { TYPE_ID_CHECKING,                 _nu("Bank Accounts"),          _n("Checking"),    -1, img::SAVINGS_ACC_NORMAL_PNG, NAV_TYP_ACCOUNT,      true},
        { TYPE_ID_CREDIT_CARD,              _nu("Credit Card Accounts"),   _n("Credit Card"), -1, img::CARD_ACC_NORMAL_PNG,    NAV_TYP_ACCOUNT,      true},
        { TYPE_ID_CASH,                     _nu("Cash Accounts"),          _n("Cash"),        -1, img::CASH_ACC_NORMAL_PNG,    NAV_TYP_ACCOUNT,      true},
        { TYPE_ID_LOAN,                     _nu("Loan Accounts"),          _n("Loan"),        -1, img::LOAN_ACC_NORMAL_PNG,    NAV_TYP_ACCOUNT,      true},
        { TYPE_ID_TERM,                     _nu("Term Accounts"),          _n("Term"),        -1, img::TERMACCOUNT_NORMAL_PNG, NAV_TYP_ACCOUNT,      true},
        { TYPE_ID_INVESTMENT,               _nu("Stock Portfolios"),       _n("Investment"),  -1, img::STOCK_ACC_NORMAL_PNG,   NAV_TYP_STOCK,        true},
        { TYPE_ID_SHARES,                   _nu("Share Accounts"),         _n("Shares"),      -1, img::STOCK_ACC_NORMAL_PNG,   NAV_TYP_OTHER,        true},
        { TYPE_ID_ASSET,                    _nu("Assets"),                 _n("Asset"),       -1, img::ASSET_NORMAL_PNG,       NAV_TYP_OTHER,        true},

        { NAV_ENTRY_BUDGET_PLANNER,         _nu("Budget Planner"),         "",                -1, img::CALENDAR_PNG,           NAV_TYP_PANEL,        true},
        { NAV_ENTRY_TRANSACTION_REPORT,     _nu("Transaction Report"),     "",                -1, img::FILTER_PNG,             NAV_TYP_PANEL,        true},
        { NAV_ENTRY_REPORTS,                _nu("Reports"),                "",                -1, img::PIECHART_PNG,           NAV_TYP_PANEL,        true},
        { NAV_ENTRY_GRM,                    _nu("General Report Manager"), "",                -1, img::CUSTOMSQL_GRP_PNG,      NAV_TYP_PANEL,        true},
        { NAV_ENTRY_DELETED_TRANSACTIONS,   _nu("Deleted Transactions"),   "",                -1, img::TRASH_PNG,              NAV_TYP_PANEL,        true},
        { NAV_ENTRY_HELP,                   _nu("Help"),                   "",                -1, img::HELP_PNG,               NAV_TYP_PANEL,        true},
    };
    m_maxId = NAV_ENTRY_size -1;

    m_type_choices = {
        { TYPE_ID_CASH,        0, _n("Cash") },
        { TYPE_ID_CHECKING,    1, _n("Checking") },
        { TYPE_ID_CREDIT_CARD, 2, _n("Credit Card") },
        { TYPE_ID_LOAN,        3, _n("Loan") },
        { TYPE_ID_TERM,        4, _n("Term") },
        { TYPE_ID_INVESTMENT,  5, _n("Investment") },
        { TYPE_ID_ASSET,       6, _n("Asset") },
        //{ TYPE_ID_SHARES,      7, _n("Shares") },
    };
}

bool NavigatorTypes::DeleteEntry(NavigatorTypesInfo* info)
{
    bool result = false;
    for (int i = 0; i < static_cast<int>(m_navigator_entries.size()); i++) {
        if (&m_navigator_entries[i] == info) {
            if (info->seq_no == m_maxId) {
                m_maxId--;
            }
            m_navigator_entries.erase(m_navigator_entries.begin() + i);
            result = true;
            break;
        }
    }
    return result;
}

NavigatorTypesInfo* NavigatorTypes::getFirstAccount()
{
    t_lastIdx = 0;
    t_previous = &m_navigator_entries[0];
    return t_previous;
}

NavigatorTypesInfo* NavigatorTypes::getNextAccount(NavigatorTypesInfo* previous)
{
    if (previous == t_previous) {
        t_lastIdx++;
        t_previous = t_lastIdx < size(m_navigator_entries) ? &m_navigator_entries[t_lastIdx] : nullptr;
    }
    else {
        wxLogError("Invalid use of NavigatorTypes::getNextAccount");
        t_previous = nullptr;
    }
    return t_previous;
}

NavigatorTypesInfo* NavigatorTypes::getFirstActiveEntry()
{
    t_lastIdx = 0;
    bool found = false;
    while (t_lastIdx < size(m_navigator_entries)){
        if (m_navigator_entries[t_lastIdx].active) {
            found = true;
            break;
        }
        t_lastIdx++;
    }
    t_previous = found ? &m_navigator_entries[t_lastIdx]: nullptr;
    return t_previous;
}

NavigatorTypesInfo* NavigatorTypes::getNextActiveEntry(NavigatorTypesInfo* previous)
{
    if (previous == t_previous) {
        t_lastIdx++;
        bool found = false;
        while (t_lastIdx < size(m_navigator_entries)){
            if (m_navigator_entries[t_lastIdx].active) {
                found = true;
                break;
            }
            t_lastIdx++;
        }
        t_previous = found ? &m_navigator_entries[t_lastIdx] : nullptr;
    }
    else {
        wxLogError("Invalid use of NavigatorTypes::getNextAccount");
        t_previous = nullptr;
    }
    return t_previous;
}

void NavigatorTypes::SaveSequenceAndState()
{
    m_maxId = 0;
    m_type_choices.clear();

    wxString key = "NAVIGATOR_SETTINGS";
    Document j_doc = Model_Infotable::instance().getJdoc(key, "{}");
    j_doc.SetObject();
    rapidjson::Value array(rapidjson::kArrayType);

    for (auto& entry : m_navigator_entries) {
        rapidjson::Value value(entry.name.ToUTF8().data(), j_doc.GetAllocator());
        rapidjson::Value cvalue(entry.choice.ToUTF8().data(), j_doc.GetAllocator());
        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("id",      entry.id,      j_doc.GetAllocator());
        obj.AddMember("name",    value,         j_doc.GetAllocator());
        obj.AddMember("choice",  cvalue,        j_doc.GetAllocator());
        obj.AddMember("seq_no",  entry.seq_no,  j_doc.GetAllocator());
        obj.AddMember("imageId", entry.imageId, j_doc.GetAllocator());
        obj.AddMember("navTyp",  entry.navTyp,  j_doc.GetAllocator());
        obj.AddMember("active",  entry.active,  j_doc.GetAllocator());
        array.PushBack(obj, j_doc.GetAllocator());
        if (entry.seq_no > m_maxId) {
            m_maxId = entry.seq_no;
        }
        if (entry.navTyp > NAV_TYP_PANEL) { // || entry.navTyp == NAV_TYP_OTHER) {
            updateTypeChoiceName(entry.id, entry.seq_no, entry.choice);
        }
    }
    j_doc.AddMember("data", array, j_doc.GetAllocator());

    Model_Infotable::instance().setJdoc(key, j_doc);

    sortEntriesBySeq();
}

NavigatorTypesInfo* NavigatorTypes::FindEntry(int searchId)
{
    for (auto& entry : m_navigator_entries) {
        if (entry.id == searchId) {
            return &entry;
        }
    }
    return nullptr;
}

wxString NavigatorTypes::FindEntryName(int searchId)
{
    for (auto& entry : m_navigator_entries) {
        if (entry.id == searchId) {
            return entry.name;
        }
    }
    return "";
}

NavigatorTypesInfo* NavigatorTypes::FindOrCreateEntry(int searchId)
{
   NavigatorTypesInfo* info = FindEntry(searchId);
   if (!info) {
        NavigatorTypesInfo newEntry;
        newEntry.id = ++m_maxId;
        m_navigator_entries.emplace_back(std::move(newEntry));
        info = &m_navigator_entries.back();
   }
   return info;
}

void NavigatorTypes::LoadFromInfoTable()
{
    wxString key = "NAVIGATOR_SETTINGS";
    Document doc = Model_Infotable::instance().getJdoc(key, "{}");
    if (!doc.HasMember("data") || !doc["data"].IsArray()) {
        return;
    }
    const rapidjson::Value& array = doc["data"];
    for (rapidjson::SizeType i = 0; i < array.Size(); ++i) {
        const rapidjson::Value& obj = array[i];
        int id = -1;
        if (obj.HasMember("id") && obj["id"].IsInt()) {
            id = obj["id"].GetInt();
        }
         if (id == -1) {
            wxLogError("NAVIGATOR_SETTINGS - invalid content!");
            return;
        }
        NavigatorTypesInfo* info = FindOrCreateEntry(id);
        if (obj.HasMember("name") && obj["name"].IsString()) {
            const char* chars = obj["name"].GetString();
            info->name = wxString::FromUTF8(chars);
        }
        if (obj.HasMember("choice") && obj["choice"].IsString()) {
            const char* chars = obj["choice"].GetString();
            info->choice = wxString::FromUTF8(chars);
        }
        if (obj.HasMember("seq_no") && obj["seq_no"].IsInt()) {
            info->seq_no = obj["seq_no"].GetInt();
            if (info->seq_no > m_maxId) {
                m_maxId = info->seq_no;
            }
        }
        if (obj.HasMember("active") && obj["active"].IsBool()) {
            info->active = obj["active"].GetBool();
        }
        if (obj.HasMember("imageId") && obj["imageId"].IsInt()) {
            info->imageId = obj["imageId"].GetInt();
        }
        if (obj.HasMember("navTyp") && obj["navTyp"].IsInt()) {
            info->navTyp = obj["navTyp"].GetInt();
        }
        // Clean up if necessary:
        if (info->name.IsEmpty()) {
            info->name = "UNKNOWN";
        }
        if (info->choice.IsEmpty()) {
            info->choice = info->name;
        }
        if (info->navTyp == NAV_TYP_ACCOUNT || info->navTyp == NAV_TYP_OTHER) {
            updateTypeChoiceName(info->id, info->seq_no, info->choice);
        }
    }
    sortEntriesBySeq();
}

void NavigatorTypes::sortEntriesBySeq()
{
    std::stable_sort(m_navigator_entries.begin(), m_navigator_entries.end(), [](NavigatorTypesInfo a, NavigatorTypesInfo b)
    {
        return (a.seq_no < b.seq_no);
    });

    std::stable_sort(m_type_choices.begin(), m_type_choices.end(), [](AccountItem a, AccountItem b)
    {
        return (a.seq_no < b.seq_no);
    });
}

wxString NavigatorTypes::getAccountSectionName(int account_type)
{
    NavigatorTypesInfo* info = FindEntry(account_type);
    return info ? info->name : "";
}

// Account choice names for selection
int NavigatorTypes::getNumberOfAccountTypes()
{
    return static_cast<int>(m_type_choices.size());
};

NavigatorTypes::AccountItem* NavigatorTypes::getAccountTypeItem(int idx)
{
    return &m_type_choices[idx];
}

const wxString NavigatorTypes::type_name(int id)
{
    for (AccountItem entry : m_type_choices) {
        if (entry.id == id) {
            return entry.name;
        }
    }
    return "";
}

int NavigatorTypes::type_id(const wxString& name, int default_id)
{
    for (AccountItem entry : m_type_choices) {
        if (entry.name == name) {
            return entry.id;
        }
    }
    return default_id;
}

void NavigatorTypes::updateTypeChoiceName(int id, int seq_no, wxString name)
{
    for (AccountItem entry : m_type_choices) {
        if (entry.id == id) {
            entry.name = name;
            entry.seq_no = seq_no;
            return;
        }
    }
    // Not found => create new entry
    m_type_choices.push_back(AccountItem(id, seq_no, name));
}

void NavigatorTypes::SetTrashStatus(bool state)
{
    NavigatorTypesInfo* info = FindEntry(NAV_ENTRY_DELETED_TRANSACTIONS);
    if (info) {
        info->active = state;
    }
}
