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

NavigatorTypes::NavigatorTypes()
{
    SetToDefault();
    loadFromInfoTable();
}

NavigatorTypes& NavigatorTypes::instance()
{
    return Singleton<NavigatorTypes>::instance();
}

void NavigatorTypes::SetToDefault() {
    m_entries = {  // init with default entries
        { NAV_ENTRY_DASHBOARD,                _nu("Dashboard"),              -1, img::HOUSE_PNG,              NAV_TYP_PANEL, true},
        { NAV_ENTRY_ALL_TRANSACTIONS,         _nu("All Transactions"),       -1, img::ALLTRANSACTIONS_PNG,    NAV_TYP_PANEL, true},
        { NAV_ENTRY_SCHEDULED_TRANSACTIONS,   _nu("Scheduled Transactions"), -1, img::SCHEDULE_PNG,           NAV_TYP_PANEL, true},
        { NAV_ENTRY_FAVORITES,                _nu("Favorites"),              -1, img::FAVOURITE_PNG,          NAV_TYP_PANEL, true},

        { Model_Account::TYPE_ID_CHECKING,    _nu("Bank Accounts"),          -1, img::SAVINGS_ACC_NORMAL_PNG, NAV_TYP_ACCOUNT, true},
        { Model_Account::TYPE_ID_CREDIT_CARD, _nu("Credit Card Accounts"),   -1, img::CARD_ACC_NORMAL_PNG,    NAV_TYP_ACCOUNT, true},
        { Model_Account::TYPE_ID_CASH,        _nu("Cash Accounts"),          -1, img::CASH_ACC_NORMAL_PNG,    NAV_TYP_ACCOUNT, true},
        { Model_Account::TYPE_ID_LOAN,        _nu("Loan Accounts"),          -1, img::LOAN_ACC_NORMAL_PNG,    NAV_TYP_ACCOUNT, true},
        { Model_Account::TYPE_ID_TERM,        _nu("Term Accounts"),          -1, img::TERMACCOUNT_NORMAL_PNG, NAV_TYP_ACCOUNT, true},
        { Model_Account::TYPE_ID_INVESTMENT,  _nu("Stock Portfolios"),       -1, img::STOCK_ACC_NORMAL_PNG,   NAV_TYP_OTHER, true},
        { Model_Account::TYPE_ID_SHARES,      _nu("Share Accounts"),         -1, img::STOCK_ACC_NORMAL_PNG,   NAV_TYP_OTHER, true},
        { Model_Account::TYPE_ID_ASSET,       _nu("Assets"),                 -1, img::ASSET_NORMAL_PNG,       NAV_TYP_OTHER, true},

        { NAV_ENTRY_BUDGET_PLANNER,           _nu("Budget Planner"),         -1, img::CALENDAR_PNG,           NAV_TYP_PANEL, true},
        { NAV_ENTRY_TRANSACTION_REPORT,       _nu("Transaction Report"),     -1, img::FILTER_PNG,             NAV_TYP_PANEL, true},
        { NAV_ENTRY_REPORTS,                  _nu("Reports"),                -1, img::PIECHART_PNG,           NAV_TYP_PANEL, true},
        { NAV_ENTRY_GRM,                      _nu("General Report Manager"), -1, img::CUSTOMSQL_GRP_PNG,      NAV_TYP_PANEL, true},
        { NAV_ENTRY_DELETED_TRANSACTIONS,     _nu("Deleted Transactions"),   -1, img::TRASH_PNG,              NAV_TYP_PANEL, true},
        { NAV_ENTRY_HELP,                     _nu("Help"),                   -1, img::HELP_PNG,               NAV_TYP_PANEL, true},
    };
    m_maxId = NAV_ENTRY_size -1;
}

bool NavigatorTypes::DeleteEntry(NavigatorTypesInfo* info) {
    bool result = false;
    for (int i = 0; i < static_cast<int>(m_entries.size()); i++) {
        if (&m_entries[i] == info) {
            m_entries.erase(m_entries.begin() + i);
            result = true;
            break;
        }
    }
    return result;
}

NavigatorTypesInfo* NavigatorTypes::getFirstAccount() {
    t_lastIdx = 0;
    t_previous = &m_entries[0];
    return t_previous;
}

NavigatorTypesInfo* NavigatorTypes::getNextAccount(NavigatorTypesInfo* previous) {
    if (previous == t_previous) {
        t_lastIdx++;
        t_previous = t_lastIdx < size(m_entries) ? &m_entries[t_lastIdx] : nullptr;
    }
    else {
        wxLogError("Invalid use of NavigatorTypes::getNextAccount");
        t_previous = nullptr;
    }
    return t_previous;
}

NavigatorTypesInfo* NavigatorTypes::getFirstActiveEntry() {
    t_lastIdx = 0;
    bool found = false;
    while (t_lastIdx < size(m_entries)){
        if (m_entries[t_lastIdx].active) {
            found = true;
            break;
        }
        t_lastIdx++;
    }
    t_previous = found ? &m_entries[t_lastIdx]: nullptr;
    return t_previous;
}

NavigatorTypesInfo* NavigatorTypes::getNextActiveEntry(NavigatorTypesInfo* previous) {
    if (previous == t_previous) {
        t_lastIdx++;
        bool found = false;
        while (t_lastIdx < size(m_entries)){
            if (m_entries[t_lastIdx].active) {
                found = true;
                break;
            }
            t_lastIdx++;
        }
        t_previous = found ? &m_entries[t_lastIdx] : nullptr;
    }
    else {
        wxLogError("Invalid use of NavigatorTypes::getNextAccount");
        t_previous = nullptr;
    }
    return t_previous;
}

void NavigatorTypes::SaveSequenceAndState() {
    wxString key = "NAVIGATOR_SETTINGS";
    Document j_doc = Model_Infotable::instance().getJdoc(key, "{}");
    j_doc.SetObject();
    rapidjson::Value array(rapidjson::kArrayType);

    for (int i = 0; i < static_cast<int>(m_entries.size()); i++) {
        rapidjson::Value value(m_entries[i].name.ToUTF8().data(), j_doc.GetAllocator());
        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("id",      m_entries[i].id,      j_doc.GetAllocator());
        obj.AddMember("name",    value,                j_doc.GetAllocator());
        obj.AddMember("seq_no",  m_entries[i].seq_no,  j_doc.GetAllocator());
        obj.AddMember("imageId", m_entries[i].imageId, j_doc.GetAllocator());
        obj.AddMember("navTyp",  m_entries[i].navTyp,  j_doc.GetAllocator());
        obj.AddMember("active",  m_entries[i].active,  j_doc.GetAllocator());
        array.PushBack(obj, j_doc.GetAllocator());
    }
    j_doc.AddMember("data", array, j_doc.GetAllocator());

    Model_Infotable::instance().setJdoc(key, j_doc);

    sortEntriesBySeq();
}

NavigatorTypesInfo& NavigatorTypes::FindOrCreateEntry(int searchId) {
    if (searchId > -1) {
        auto it = std::find_if(m_entries.begin(), m_entries.end(),
        [searchId](const NavigatorTypesInfo& entry) {
            return entry.id == searchId;
        });
        if (it != m_entries.end()) {
            return *it;
        }
    }
    NavigatorTypesInfo newEntry;
    newEntry.id = searchId > -1 ? searchId : ++m_maxId;
    if (newEntry.id > m_maxId) {
        m_maxId = newEntry.id;
    }
    m_entries.emplace_back(std::move(newEntry));
    return m_entries.back();
}

void NavigatorTypes::loadFromInfoTable() {
    wxString key = "NAVIGATOR_SETTINGS";
    Document doc = Model_Infotable::instance().getJdoc(key, "{}");
    if (!doc.HasMember("data") || !doc["data"].IsArray()) {
        wxLogDebug("Navigatortypes: No data found in settings => use default");
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
        NavigatorTypesInfo& info = FindOrCreateEntry(id);
        if (obj.HasMember("name") && obj["name"].IsString()) {
            const char* chars = obj["name"].GetString();
            wxString newName = wxString::FromUTF8(chars);
            if (info.name != newName) {
                info.name = newName;
            }
        }
        if (obj.HasMember("seq_no") && obj["seq_no"].IsInt()) {
            info.seq_no = obj["seq_no"].GetInt();
        }
        if (obj.HasMember("active") && obj["active"].IsBool()) {
            info.active = obj["active"].GetBool();
        }
        if (obj.HasMember("imageId") && obj["imageId"].IsInt()) {
            info.imageId = obj["imageId"].GetInt();
        }
        if (obj.HasMember("navTyp") && obj["navTyp"].IsInt()) {
            info.navTyp = obj["navTyp"].GetInt();
        }
    }
    sortEntriesBySeq();
}

void NavigatorTypes::sortEntriesBySeq() {
    std::stable_sort(m_entries.begin(), m_entries.end(), [](NavigatorTypesInfo a, NavigatorTypesInfo b)
    {
        return (a.seq_no < b.seq_no);
    });
}

wxArrayString* NavigatorTypes::GetCustomCheckingAccounts()
{
    return &m_customChecking;
}