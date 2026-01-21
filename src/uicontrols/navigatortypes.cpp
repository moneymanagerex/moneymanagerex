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
    m_lastIdx = 0;
    m_previous = nullptr;
    SetToDefault();
    sortEntriesBySeq();
}

NavigatorTypes::~NavigatorTypes()
{
    for (auto ref : m_navigator_entries) {
        delete ref;
    }
}

NavigatorTypes& NavigatorTypes::instance()
{
    return Singleton<NavigatorTypes>::instance();
}

void NavigatorTypes::SetToDefault()
{
    // Delete old entries
    for (auto ref : m_navigator_entries) {
        delete ref;
    }
    m_navigator_entries.clear();

    // init with default entries
    m_navigator_entries.push_back(new  NavigatorTypesInfo{NAV_ENTRY_DASHBOARD,              "Dashboard",              "",            "",            -1, img::HOUSE_PNG,              NAV_TYP_PANEL,   true});
    m_navigator_entries.push_back(new  NavigatorTypesInfo{NAV_ENTRY_ALL_TRANSACTIONS,       "All Transactions",       "",            "",            -1, img::ALLTRANSACTIONS_PNG,    NAV_TYP_PANEL,   true});
    m_navigator_entries.push_back(new  NavigatorTypesInfo{NAV_ENTRY_SCHEDULED_TRANSACTIONS, "Scheduled Transactions", "",            "",            -1, img::SCHEDULE_PNG,           NAV_TYP_PANEL,   true});
    m_navigator_entries.push_back(new  NavigatorTypesInfo{NAV_ENTRY_FAVORITES,              "Favorites",              "",            "",            -1, img::FAVOURITE_PNG,          NAV_TYP_PANEL,   true});

    m_navigator_entries.push_back(new  NavigatorTypesInfo{TYPE_ID_CHECKING,                 "Bank Accounts",          "Checking",    "Checking",    -1, img::SAVINGS_ACC_NORMAL_PNG, NAV_TYP_ACCOUNT, true});
    m_navigator_entries.push_back(new  NavigatorTypesInfo{TYPE_ID_CREDIT_CARD,              "Credit Card Accounts",   "Credit Card", "Credit Card", -1, img::CARD_ACC_NORMAL_PNG,    NAV_TYP_ACCOUNT, true});
    m_navigator_entries.push_back(new  NavigatorTypesInfo{TYPE_ID_CASH,                     "Cash Accounts",          "Cash",        "Cash",        -1, img::CASH_ACC_NORMAL_PNG,    NAV_TYP_ACCOUNT, true});
    m_navigator_entries.push_back(new  NavigatorTypesInfo{TYPE_ID_LOAN,                     "Loan Accounts",          "Loan",        "Loan",        -1, img::LOAN_ACC_NORMAL_PNG,    NAV_TYP_ACCOUNT, true});
    m_navigator_entries.push_back(new  NavigatorTypesInfo{TYPE_ID_TERM,                     "Term Accounts",          "Term",        "Term",        -1, img::TERMACCOUNT_NORMAL_PNG, NAV_TYP_ACCOUNT, true});
    m_navigator_entries.push_back(new  NavigatorTypesInfo{TYPE_ID_INVESTMENT,               "Stock Portfolios",       "Investment",  "Investment",  -1, img::STOCK_ACC_NORMAL_PNG,   NAV_TYP_STOCK,   true});
    m_navigator_entries.push_back(new  NavigatorTypesInfo{TYPE_ID_SHARES,                   "Share Accounts",         "Shares",      "Shares",      -1, img::STOCK_ACC_NORMAL_PNG,   NAV_TYP_OTHER,   true});
    m_navigator_entries.push_back(new  NavigatorTypesInfo{TYPE_ID_ASSET,                    "Assets",                 "Asset",       "Asset",       -1, img::ASSET_NORMAL_PNG,       NAV_TYP_OTHER,   true});

    m_navigator_entries.push_back(new  NavigatorTypesInfo{NAV_ENTRY_BUDGET_PLANNER,         "Budget Planner",         "",            "",            -1, img::CALENDAR_PNG,           NAV_TYP_PANEL,   true});
    m_navigator_entries.push_back(new  NavigatorTypesInfo{NAV_ENTRY_TRANSACTION_REPORT,     "Transaction Report",     "",            "",            -1, img::FILTER_PNG,             NAV_TYP_PANEL,   true});
    m_navigator_entries.push_back(new  NavigatorTypesInfo{NAV_ENTRY_REPORTS,                "Reports",                "",            "",            -1, img::PIECHART_PNG,           NAV_TYP_PANEL,   true});
    m_navigator_entries.push_back(new  NavigatorTypesInfo{NAV_ENTRY_GRM,                    "General Report Manager", "",            "",            -1, img::CUSTOMSQL_GRP_PNG,      NAV_TYP_PANEL,   true});
    m_navigator_entries.push_back(new  NavigatorTypesInfo{NAV_ENTRY_DELETED_TRANSACTIONS,   "Deleted Transactions",   "",            "",            -1, img::TRASH_PNG,              NAV_TYP_PANEL,   true});
    m_navigator_entries.push_back(new  NavigatorTypesInfo{NAV_ENTRY_HELP,                   "Help",                   "",            "",            -1, img::HELP_PNG,               NAV_TYP_PANEL,   true});
}

bool NavigatorTypes::DeleteEntry(NavigatorTypesInfo* info)
{
    bool result = false;
    for (int i = 0; i < static_cast<int>(m_navigator_entries.size()); i++) {
        if  (m_navigator_entries[i] == info) {
            // change account type of all affected accounts to Banking
            Model_Account::instance().resetAccountType(info->dbaccid);
            m_navigator_entries.erase(m_navigator_entries.begin() + i);
            result = true;
            break;
        }
    }
    return result;
}

NavigatorTypesInfo* NavigatorTypes::getFirstAccount()
{
    m_lastIdx = 0;
    m_previous = m_navigator_entries[0];
    return m_previous;
}

NavigatorTypesInfo* NavigatorTypes::getNextAccount(NavigatorTypesInfo* previous)
{
    if (previous == m_previous) {
        m_lastIdx++;
        m_previous = m_lastIdx < size(m_navigator_entries) ? m_navigator_entries[m_lastIdx] : nullptr;
    }
    else {
        wxLogError("Invalid use of NavigatorTypes::getNextAccount");
        m_previous = nullptr;
    }
    return m_previous;
}

NavigatorTypesInfo* NavigatorTypes::getFirstActiveEntry()
{
    m_lastIdx = 0;
    bool found = false;
    while (m_lastIdx < size(m_navigator_entries)){
        if (m_navigator_entries[m_lastIdx]->active) {
            found = true;
            break;
        }
        m_lastIdx++;
    }
    m_previous = found ? m_navigator_entries[m_lastIdx]: nullptr;
    return m_previous;
}

NavigatorTypesInfo* NavigatorTypes::getNextActiveEntry(NavigatorTypesInfo* previous)
{
    if (previous == m_previous) {
        m_lastIdx++;
        bool found = false;
        while (m_lastIdx < size(m_navigator_entries)){
            if (m_navigator_entries[m_lastIdx]->active) {
                found = true;
                break;
            }
            m_lastIdx++;
        }
        m_previous = found ? m_navigator_entries[m_lastIdx] : nullptr;
    }
    else {
        wxLogError("Invalid use of NavigatorTypes::getNextAccount");
        m_previous = nullptr;
    }
    return m_previous;
}

void NavigatorTypes::SaveSequenceAndState()
{
    wxString key = "NAVIGATOR_SETTINGS";
    Document j_doc = Model_Infotable::instance().getJdoc(key, "{}");
    j_doc.SetObject();
    rapidjson::Value array(rapidjson::kArrayType);

    for (auto& entry : m_navigator_entries) {
        rapidjson::Value value(entry->name.ToUTF8().data(), j_doc.GetAllocator());
        rapidjson::Value cvalue(entry->choice.ToUTF8().data(), j_doc.GetAllocator());
        rapidjson::Value dbaccid(entry->dbaccid.ToUTF8().data(), j_doc.GetAllocator());
        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("id",      entry->type,    j_doc.GetAllocator());
        obj.AddMember("name",    value,         j_doc.GetAllocator());
        obj.AddMember("choice",  cvalue,        j_doc.GetAllocator());
        obj.AddMember("dbaccid", dbaccid,       j_doc.GetAllocator());
        obj.AddMember("seq_no",  entry->seq_no,  j_doc.GetAllocator());
        obj.AddMember("imageId", entry->imageId, j_doc.GetAllocator());
        obj.AddMember("navTyp",  entry->navTyp,  j_doc.GetAllocator());
        obj.AddMember("active",  entry->active,  j_doc.GetAllocator());
        array.PushBack(obj, j_doc.GetAllocator());
    }
    j_doc.AddMember("data", array, j_doc.GetAllocator());

    Model_Infotable::instance().setJdoc(key, j_doc);

    sortEntriesBySeq();
}

NavigatorTypesInfo* NavigatorTypes::FindEntry(int searchId)
{
    for (NavigatorTypesInfo* entry : m_navigator_entries) {
        if (entry->type == searchId) {
            return entry;
        }
    }
    return nullptr;
}

wxString NavigatorTypes::FindEntryName(int searchId)
{
    for (NavigatorTypesInfo* entry : m_navigator_entries) {
        if (entry->type == searchId) {
            return entry->name;
        }
    }
    return "";
}

NavigatorTypesInfo* NavigatorTypes::FindOrCreateEntry(int searchId)
{
   NavigatorTypesInfo* info = searchId > -1 ? FindEntry(searchId) : nullptr;
   if (!info) {
        info = new NavigatorTypesInfo;
        if (searchId == -1) {
            info->type = getMaxId() + 1;
        }
        else {
            info->type = searchId;
        }
        m_navigator_entries.push_back(info);
        info = m_navigator_entries.back();
   }
   return info;
}

void NavigatorTypes::LoadFromDB(bool keepnames)
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
            continue;
        }
        NavigatorTypesInfo* info = FindOrCreateEntry(id);
        if (!keepnames || id > NavigatorTypes::NAV_ENTRY_HELP) {
            if (obj.HasMember("name") && obj["name"].IsString()) {
                const char* chars = obj["name"].GetString();
                info->name = wxString::FromUTF8(chars);
            }
            if (obj.HasMember("choice") && obj["choice"].IsString()) {
                const char* chars = obj["choice"].GetString();
                info->choice = wxString::FromUTF8(chars);
            }
            if (obj.HasMember("dbaccid") && obj["dbaccid"].IsString()) {
                const char* chars = obj["dbaccid"].GetString();
                info->dbaccid = wxString::FromUTF8(chars);
            }
        }
        if (obj.HasMember("seq_no") && obj["seq_no"].IsInt()) {
            info->seq_no = obj["seq_no"].GetInt();
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
        if (info->dbaccid.IsEmpty()) {
            info->dbaccid = info->choice;
        }
    }
    sortEntriesBySeq();
}

void NavigatorTypes::sortEntriesBySeq()
{
    std::stable_sort(m_navigator_entries.begin(), m_navigator_entries.end(), [](NavigatorTypesInfo* a, NavigatorTypesInfo* b)
    {
        return (a->seq_no < b->seq_no);
    });

    m_account_type_entries = {};
    for (size_t i = 0; i < m_navigator_entries.size(); i++) {
        if (m_navigator_entries[i]->navTyp > NAV_TYP_PANEL) {
            m_account_type_entries.push_back(m_navigator_entries[i]);
        }
    }
}

wxString NavigatorTypes::getAccountSectionName(int account_type)
{
    NavigatorTypesInfo* info = FindEntry(account_type);
    return info ? info->name : "";
}

int NavigatorTypes::getNumberOfAccountTypes()
{
    return static_cast<int>(m_account_type_entries.size());
};

NavigatorTypesInfo* NavigatorTypes::getAccountTypeItem(int idx)
{
    return m_account_type_entries[idx];
}

int NavigatorTypes::getAccountTypeIdx(const wxString& atypename)
{
    for (int i = 0; i < static_cast<int>(m_account_type_entries.size()); i++) {
        if (atypename == m_account_type_entries[i]->dbaccid) {
            return i;
        }
    }
    return -1;
}

int NavigatorTypes::getAccountTypeIdx(int account_type)
{
    for (int i = 0; i < static_cast<int>(m_account_type_entries.size()); i++) {
        if (account_type == m_account_type_entries[i]->type) {
            return i;
        }
    }
    return -1;
}

wxString NavigatorTypes::getAccountDbTypeFromChoice(const wxString& choiceName)
{
    for (NavigatorTypesInfo* entry : m_navigator_entries) {
        if (entry->choice == choiceName) {
            return entry->dbaccid;
        }
    }
    return "Checking";
}

wxString NavigatorTypes::getAccountDbTypeFromName(const wxString& typeName)
{
    for (NavigatorTypesInfo* entry : m_navigator_entries) {
        if (entry->name == typeName) {
            return entry->dbaccid;
        }
    }
    return "Checking";
}

bool NavigatorTypes::isAccountTypeAsset(int idx)
{
    return m_account_type_entries[idx]->type == TYPE_ID_ASSET;
}

wxString NavigatorTypes::getAccountTypeName(int idx)
{
    return GetTranslatedName(m_account_type_entries[idx]);
}

wxString NavigatorTypes::GetTranslatedName(NavigatorTypesInfo* info)
{
    return info->navTyp < NavigatorTypes::NAV_ENTRY_size ? wxGetTranslation(info->name) : info->name;
}

wxString NavigatorTypes::GetTranslatedSelection(NavigatorTypesInfo* info)
{
    return info->navTyp < NavigatorTypes::NAV_ENTRY_size ? wxGetTranslation(info->choice) : info->choice;
}

wxArrayString NavigatorTypes::getAccountSelectionNames(wxString filter)
{
    wxArrayString names;
    for (NavigatorTypesInfo* entry : m_navigator_entries) {
        if (entry->navTyp > NAV_TYP_PANEL && entry->active && (filter.IsEmpty() || entry->choice != filter)) {
            names.Add(GetTranslatedSelection(entry));
        }
    }
    return names;
}

wxArrayString NavigatorTypes::getUsedAccountTypeNames()
{
    wxArrayString names;
    wxArrayString usedtypes = Model_Account::instance().getUsedAccountTypes();
    for (NavigatorTypesInfo* entry : m_navigator_entries) {
        if (entry->navTyp > NAV_TYP_PANEL && entry->active) {
            if (usedtypes.Index(entry->dbaccid) != wxNOT_FOUND) {
                names.Add(GetTranslatedName(entry));
            }
        }
    }
    return names;
}

// access to DB identifieres
const wxString NavigatorTypes::type_name(int id)
{
    for (NavigatorTypesInfo* entry : m_navigator_entries) {
        if (entry->type == id) {
            return entry->dbaccid;
        }
    }
    return "";
}

int NavigatorTypes::getTypeIdFromDBName(const wxString& dbname, int default_id)
{
    for (NavigatorTypesInfo* entry : m_navigator_entries) {
        if (entry->dbaccid == dbname) {
            return entry->type;
        }
    }
    return default_id;
}

int NavigatorTypes::getTypeIdFromChoice(const wxString& choice, int default_id)
{
    for (NavigatorTypesInfo* entry : m_navigator_entries) {
        if (entry->choice == choice) {
            return entry->type;
        }
    }
    return default_id;
}

// Other:
void NavigatorTypes::SetTrashStatus(bool state)
{
    NavigatorTypesInfo* info = FindEntry(NAV_ENTRY_DELETED_TRANSACTIONS);
    if (info) {
        info->active = state;
    }
}

void NavigatorTypes::SetShareAccountStatus(bool state)
{
    NavigatorTypesInfo* info = FindEntry(TYPE_ID_SHARES);
    if (info) {
        info->active = state;
    }
}

int NavigatorTypes::getMaxId()
{
    int max = 0;
    for (int i = 0; i < static_cast<int>(m_navigator_entries.size()); i++) {
        if (m_navigator_entries[i]->seq_no > max) {
            max = m_navigator_entries[i]->seq_no;
        }
    }
    return max;
}
