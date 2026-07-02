/*******************************************************
 Copyright (C) 2025,2026 Klaus Wich

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

#include "mmNavigatorList.h"

#include "base/mmSingleton.h"
#include "util/mmImage.h"
#include "model/AccountModel.h"

mmNavigatorList::mmNavigatorList()
{
    m_lastIdx = 0;
    m_previous = nullptr;
    SetToDefault();
    sortEntriesBySeq();
}

mmNavigatorList::~mmNavigatorList()
{
    for (auto ref : m_navigator_entries) {
        delete ref;
    }
}

mmNavigatorList& mmNavigatorList::instance()
{
    return Singleton<mmNavigatorList>::instance();
}

void mmNavigatorList::SetToDefault()
{
    // Delete old entries
    for (auto ref : m_navigator_entries) {
        delete ref;
    }
    m_navigator_entries.clear();

    // init with default entries
    m_navigator_entries.push_back(new  mmNavigatorItem{mmNavigatorItem::NAV_ENTRY_DASHBOARD,              _n("Dashboard"),              "",                "",            -1, mmImage::img::HOUSE_PNG,              mmNavigatorItem::NAV_TYP_PANEL,   true});
    m_navigator_entries.push_back(new  mmNavigatorItem{mmNavigatorItem::NAV_ENTRY_ALL_TRANSACTIONS,       _n("All Transactions"),       "",                "",            -1, mmImage::img::ALLTRANSACTIONS_PNG,    mmNavigatorItem::NAV_TYP_PANEL,   true});
    m_navigator_entries.push_back(new  mmNavigatorItem{mmNavigatorItem::NAV_ENTRY_SCHEDULED_TRANSACTIONS, _n("Scheduled Transactions"), "",                "",            -1, mmImage::img::SCHEDULE_PNG,           mmNavigatorItem::NAV_TYP_PANEL,   true});
    m_navigator_entries.push_back(new  mmNavigatorItem{mmNavigatorItem::NAV_ENTRY_FAVORITES,              _n("Favorites"),              "",                "",            -1, mmImage::img::FAVOURITE_PNG,          mmNavigatorItem::NAV_TYP_PANEL,   true});

    m_navigator_entries.push_back(new  mmNavigatorItem{mmNavigatorItem::TYPE_ID_CHECKING,                 _n("Bank Accounts"),          _n("Checking"),    "Checking",    -1, mmImage::img::SAVINGS_ACC_NORMAL_PNG, mmNavigatorItem::NAV_TYP_ACCOUNT, true});
    m_navigator_entries.push_back(new  mmNavigatorItem{mmNavigatorItem::TYPE_ID_CREDIT_CARD,              _n("Credit Card Accounts"),   _n("Credit Card"), "Credit Card", -1, mmImage::img::CARD_ACC_NORMAL_PNG,    mmNavigatorItem::NAV_TYP_ACCOUNT, true});
    m_navigator_entries.push_back(new  mmNavigatorItem{mmNavigatorItem::TYPE_ID_CASH,                     _n("Cash Accounts"),          _n("Cash"),        "Cash",        -1, mmImage::img::CASH_ACC_NORMAL_PNG,    mmNavigatorItem::NAV_TYP_ACCOUNT, true});
    m_navigator_entries.push_back(new  mmNavigatorItem{mmNavigatorItem::TYPE_ID_LOAN,                     _n("Loan Accounts"),          _n("Loan"),        "Loan",        -1, mmImage::img::LOAN_ACC_NORMAL_PNG,    mmNavigatorItem::NAV_TYP_ACCOUNT, true});
    m_navigator_entries.push_back(new  mmNavigatorItem{mmNavigatorItem::TYPE_ID_TERM,                     _n("Term Accounts"),          _n("Term"),        "Term",        -1, mmImage::img::TERMACCOUNT_NORMAL_PNG, mmNavigatorItem::NAV_TYP_ACCOUNT, true});
    m_navigator_entries.push_back(new  mmNavigatorItem{mmNavigatorItem::TYPE_ID_INVESTMENT,               _n("Stock Portfolios"),       _n("Investment"),  "Investment",  -1, mmImage::img::STOCK_ACC_NORMAL_PNG,   mmNavigatorItem::NAV_TYP_STOCK,   true});
    m_navigator_entries.push_back(new  mmNavigatorItem{mmNavigatorItem::TYPE_ID_SHARES,                   _n("Share Accounts"),         _n("Shares"),      "Shares",      -1, mmImage::img::STOCK_ACC_NORMAL_PNG,   mmNavigatorItem::NAV_TYP_OTHER,   true});
    m_navigator_entries.push_back(new  mmNavigatorItem{mmNavigatorItem::TYPE_ID_ASSET,                    _n("Assets"),                 _n("Asset"),       "Asset",       -1, mmImage::img::ASSET_NORMAL_PNG,       mmNavigatorItem::NAV_TYP_OTHER,   true});

    m_navigator_entries.push_back(new  mmNavigatorItem{mmNavigatorItem::NAV_ENTRY_BUDGET_PLANNER,         _n("Budget Planner"),         "",                "",            -1, mmImage::img::CALENDAR_PNG,           mmNavigatorItem::NAV_TYP_PANEL,   true});
    m_navigator_entries.push_back(new  mmNavigatorItem{mmNavigatorItem::NAV_ENTRY_TRANSACTION_REPORT,     _n("Transaction Report"),     "",                "",            -1, mmImage::img::FILTER_PNG,             mmNavigatorItem::NAV_TYP_PANEL,   true});
    m_navigator_entries.push_back(new  mmNavigatorItem{mmNavigatorItem::NAV_ENTRY_REPORTS,                _n("Reports"),                "",                "",            -1, mmImage::img::PIECHART_PNG,           mmNavigatorItem::NAV_TYP_PANEL,   true});
    m_navigator_entries.push_back(new  mmNavigatorItem{mmNavigatorItem::NAV_ENTRY_GRM,                    _n("General Report Manager"), "",                "",            -1, mmImage::img::CUSTOMSQL_GRP_PNG,      mmNavigatorItem::NAV_TYP_PANEL,   true});
    m_navigator_entries.push_back(new  mmNavigatorItem{mmNavigatorItem::NAV_ENTRY_DELETED_TRANSACTIONS,   _n("Deleted Transactions"),   "",                "",            -1, mmImage::img::TRASH_PNG,              mmNavigatorItem::NAV_TYP_PANEL,   true});
    m_navigator_entries.push_back(new  mmNavigatorItem{mmNavigatorItem::NAV_ENTRY_HELP,                   _n("Help"),                   "",                "",            -1, mmImage::img::HELP_PNG,               mmNavigatorItem::NAV_TYP_PANEL,   true});
}

bool mmNavigatorList::DeleteEntry(mmNavigatorItem* info)
{
    bool result = false;
    for (int i = 0; i < static_cast<int>(m_navigator_entries.size()); i++) {
        if  (m_navigator_entries[i] == info) {
            AccountModel::instance().dangerous_reset_type(info->dbaccid);
            m_navigator_entries.erase(m_navigator_entries.begin() + i);
            result = true;
            break;
        }
    }
    return result;
}

mmNavigatorItem* mmNavigatorList::getFirstAccount()
{
    m_lastIdx = 0;
    m_previous = m_navigator_entries[0];
    return m_previous;
}

mmNavigatorItem* mmNavigatorList::getNextAccount(mmNavigatorItem* previous)
{
    if (previous == m_previous) {
        m_lastIdx++;
        m_previous = m_lastIdx < size(m_navigator_entries)
            ? m_navigator_entries[m_lastIdx]
            : nullptr;
    }
    else {
        wxLogError("Invalid use of mmNavigatorList::getNextAccount");
        m_previous = nullptr;
    }
    return m_previous;
}

mmNavigatorItem* mmNavigatorList::getFirstActiveEntry()
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

mmNavigatorItem* mmNavigatorList::getNextActiveEntry(mmNavigatorItem* previous)
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
        wxLogError("Invalid use of mmNavigatorList::getNextAccount");
        m_previous = nullptr;
    }
    return m_previous;
}

void mmNavigatorList::SaveSequenceAndState()
{
    wxString key = "NAVIGATOR_SETTINGS";
    Document j_doc = InfoModel::instance().getJdoc(key, "{}");
    j_doc.SetObject();
    rapidjson::Value array(rapidjson::kArrayType);

    for (auto& entry : m_navigator_entries) {
        rapidjson::Value value(entry->name.ToUTF8().data(), j_doc.GetAllocator());
        rapidjson::Value cvalue(entry->choice.ToUTF8().data(), j_doc.GetAllocator());
        rapidjson::Value dbaccid(entry->dbaccid.ToUTF8().data(), j_doc.GetAllocator());
        wxString imageValue;
        if (entry->imageId < mmImage::acc_img::MAX_ACC_ICON) {
            imageValue = wxString::Format(wxT("%i"), entry->imageId);
        }
        else {
            imageValue = "CI:" + NavTreeIconImages::instance().getIndexMap()[entry->imageId];
        }
        rapidjson::Value dbImageValue(imageValue.ToUTF8().data(), j_doc.GetAllocator());
        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("id",      entry->type,    j_doc.GetAllocator());
        obj.AddMember("name",    value,          j_doc.GetAllocator());
        obj.AddMember("choice",  cvalue,         j_doc.GetAllocator());
        obj.AddMember("dbaccid", dbaccid,        j_doc.GetAllocator());
        obj.AddMember("seq_no",  entry->seq_no,  j_doc.GetAllocator());
        obj.AddMember("imageIds", dbImageValue,  j_doc.GetAllocator());
        obj.AddMember("navTyp",  entry->navTyp,  j_doc.GetAllocator());
        obj.AddMember("active",  entry->active,  j_doc.GetAllocator());
        array.PushBack(obj, j_doc.GetAllocator());
    }
    j_doc.AddMember("data", array, j_doc.GetAllocator());

    InfoModel::instance().saveJdoc(key, j_doc);

    sortEntriesBySeq();
}

mmNavigatorItem* mmNavigatorList::FindEntry(int searchId)
{
    for (mmNavigatorItem* entry : m_navigator_entries) {
        if (entry->type == searchId) {
            return entry;
        }
    }
    return nullptr;
}

wxString mmNavigatorList::FindEntryName(int searchId)
{
    for (mmNavigatorItem* entry : m_navigator_entries) {
        if (entry->type == searchId) {
            return entry->name;
        }
    }
    return "";
}

mmNavigatorItem* mmNavigatorList::FindOrCreateEntry(int searchId)
{
   mmNavigatorItem* info = searchId > -1 ? FindEntry(searchId) : nullptr;
   if (!info) {
        info = new mmNavigatorItem;
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

void mmNavigatorList::LoadFromDB(bool keepnames)
{
    NavTreeIconImages::instance().initIndexMap();  // tmp enable maps, needed for reading.
    wxString key = "NAVIGATOR_SETTINGS";
    Document doc = InfoModel::instance().getJdoc(key, "{}");
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
        mmNavigatorItem* info = FindOrCreateEntry(id);
        if (!keepnames || id > mmNavigatorItem::NAV_ENTRY_HELP) {
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
        if (obj.HasMember("imageId") && obj["imageId"].IsInt()) {  //old for compatibility
            info->imageId = obj["imageId"].GetInt();
            if (info->imageId >= NavTreeIconImages::instance().getListSize()) {
                info->imageId = GetDefaultImage(id);
            }
        }
        if (obj.HasMember("imageIds") && obj["imageIds"].IsString()) {
            wxString timg = wxString::FromUTF8(obj["imageIds"].GetString());
            info->imageId = NavTreeIconImages::instance().getImgIndexFromStorageString(timg);
            if (info->imageId >= NavTreeIconImages::instance().getListSize() || info->imageId < 0) {
                info->imageId = GetDefaultImage(id);
            }
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

void mmNavigatorList::sortEntriesBySeq()
{
    std::stable_sort(m_navigator_entries.begin(), m_navigator_entries.end(), [](mmNavigatorItem* a, mmNavigatorItem* b)
    {
        return (a->seq_no < b->seq_no);
    });

    m_account_type_entries = {};
    for (size_t i = 0; i < m_navigator_entries.size(); i++) {
        if (m_navigator_entries[i]->navTyp > mmNavigatorItem::NAV_TYP_PANEL) {
            m_account_type_entries.push_back(m_navigator_entries[i]);
        }
    }
}

wxString mmNavigatorList::getAccountSectionName(int account_type)
{
    mmNavigatorItem* info = FindEntry(account_type);
    return info ? info->name : "";
}

int mmNavigatorList::getNumberOfAccountTypes()
{
    return static_cast<int>(m_account_type_entries.size());
};

mmNavigatorItem* mmNavigatorList::getAccountTypeItem(int idx)
{
    return m_account_type_entries[idx];
}

int mmNavigatorList::getAccountTypeIdx(const wxString& atypename)
{
    for (int i = 0; i < static_cast<int>(m_account_type_entries.size()); i++) {
        if (atypename == m_account_type_entries[i]->dbaccid) {
            return i;
        }
    }
    return -1;
}

int mmNavigatorList::getAccountTypeIdx(int account_type)
{
    for (int i = 0; i < static_cast<int>(m_account_type_entries.size()); i++) {
        if (account_type == m_account_type_entries[i]->type) {
            return i;
        }
    }
    return -1;
}

wxString mmNavigatorList::getAccountDbTypeFromChoice(const wxString& choiceName)
{
    for (mmNavigatorItem* entry : m_navigator_entries) {
        if (entry->choice == choiceName || GetTranslatedSelection(entry) == choiceName) {
            return entry->dbaccid;
        }
    }
    return "Checking";
}

wxString mmNavigatorList::getAccountDbTypeFromName(const wxString& typeName)
{
    for (mmNavigatorItem* entry : m_navigator_entries) {
        if (entry->name == typeName) {
            return entry->dbaccid;
        }
    }
    return "Checking";
}

bool mmNavigatorList::isAccountTypeAsset(int idx)
{
    return m_account_type_entries[idx]->type == mmNavigatorItem::TYPE_ID_ASSET;
}

wxString mmNavigatorList::getAccountTypeName(int idx)
{
    return GetTranslatedName(m_account_type_entries[idx]);
}

wxString mmNavigatorList::GetTranslatedName(mmNavigatorItem* info)
{
    return info->type < mmNavigatorItem::NAV_ENTRY_size
        ? wxGetTranslation(info->name)
        : info->name;
}

wxString mmNavigatorList::GetTranslatedSelection(mmNavigatorItem* info)
{
    return info->type < mmNavigatorItem::NAV_ENTRY_size
        ? wxGetTranslation(info->choice)
        : info->choice;
}

wxArrayString mmNavigatorList::getAccountSelectionNames(wxString filter)
{
    wxArrayString names;
    for (mmNavigatorItem* entry : m_navigator_entries) {
        if (entry->navTyp > mmNavigatorItem::NAV_TYP_PANEL && entry->active &&
            (filter.IsEmpty() || entry->choice != filter)
        ) {
            names.Add(GetTranslatedSelection(entry));
        }
    }
    return names;
}

wxArrayString mmNavigatorList::getUsedAccountTypeNames()
{
    wxArrayString names;
    wxArrayString usedtypes = AccountModel::instance().find_all_type_a(true);
    for (mmNavigatorItem* entry : m_navigator_entries) {
        if (entry->navTyp > mmNavigatorItem::NAV_TYP_PANEL && entry->active) {
            if (usedtypes.Index(entry->dbaccid) != wxNOT_FOUND) {
                names.Add(GetTranslatedName(entry));
            }
        }
    }
    return names;
}

// access to DB identifieres
const wxString mmNavigatorList::type_name(int id)
{
    for (mmNavigatorItem* entry : m_navigator_entries) {
        if (entry->type == id) {
            return entry->dbaccid;
        }
    }
    return "";
}

int mmNavigatorList::getTypeIdFromDBName(const wxString& dbname, int default_id)
{
    for (mmNavigatorItem* entry : m_navigator_entries) {
        if (entry->dbaccid == dbname) {
            return entry->type;
        }
    }
    return default_id;
}

int mmNavigatorList::getTypeIdFromChoice(const wxString& choice, int default_id)
{
    for (mmNavigatorItem* entry : m_navigator_entries) {
        if (entry->choice == choice || GetTranslatedSelection(entry) == choice) {
            return entry->type;
        }
    }
    return default_id;
}

// Other:
void mmNavigatorList::SetTrashStatus(bool state)
{
    mmNavigatorItem* info = FindEntry(mmNavigatorItem::NAV_ENTRY_DELETED_TRANSACTIONS);
    if (info) {
        info->active = state;
    }
}

void mmNavigatorList::SetShareAccountStatus(bool state)
{
    mmNavigatorItem* info = FindEntry(mmNavigatorItem::TYPE_ID_SHARES);
    if (info) {
        info->active = state;
    }
}

int mmNavigatorList::getMaxId()
{
    int max = 0;
    for (int i = 0; i < static_cast<int>(m_navigator_entries.size()); i++) {
        if (m_navigator_entries[i]->seq_no > max) {
            max = m_navigator_entries[i]->seq_no;
        }
    }
    return max;
}

int mmNavigatorList::GetDefaultImage(int navTyp)
{
    static std::map<int, int> defaultTyp = {
        { mmNavigatorItem::TYPE_ID_CASH,                     mmImage::img::CASH_ACC_NORMAL_PNG },
        { mmNavigatorItem::TYPE_ID_CHECKING,                 mmImage::img::SAVINGS_ACC_NORMAL_PNG },
        { mmNavigatorItem::TYPE_ID_CREDIT_CARD,              mmImage::img::CARD_ACC_NORMAL_PNG },
        { mmNavigatorItem::TYPE_ID_LOAN,                     mmImage::img::LOAN_ACC_NORMAL_PNG },
        { mmNavigatorItem::TYPE_ID_TERM,                     mmImage::img::TERMACCOUNT_NORMAL_PNG },
        { mmNavigatorItem::TYPE_ID_INVESTMENT,               mmImage::img::STOCK_ACC_NORMAL_PNG },
        { mmNavigatorItem::TYPE_ID_ASSET,                    mmImage::img::ASSET_NORMAL_PNG },
        { mmNavigatorItem::TYPE_ID_SHARES,                   mmImage::img::STOCK_ACC_NORMAL_PNG },
        { mmNavigatorItem::NAV_ENTRY_DASHBOARD,              mmImage::img::HOUSE_PNG },
        { mmNavigatorItem::NAV_ENTRY_ALL_TRANSACTIONS,       mmImage::img::ALLTRANSACTIONS_PNG },
        { mmNavigatorItem::NAV_ENTRY_SCHEDULED_TRANSACTIONS, mmImage::img::SCHEDULE_PNG },
        { mmNavigatorItem::NAV_ENTRY_FAVORITES,              mmImage::img::FAVOURITE_PNG },
        { mmNavigatorItem::NAV_ENTRY_BUDGET_PLANNER,         mmImage::img::CALENDAR_PNG },
        { mmNavigatorItem::NAV_ENTRY_TRANSACTION_REPORT,     mmImage::img::FILTER_PNG },
        { mmNavigatorItem::NAV_ENTRY_REPORTS,                mmImage::img::PIECHART_PNG },
        { mmNavigatorItem::NAV_ENTRY_GRM,                    mmImage::img::CUSTOMSQL_GRP_PNG },
        { mmNavigatorItem::NAV_ENTRY_DELETED_TRANSACTIONS,   mmImage::img::TRASH_PNG },
        { mmNavigatorItem::NAV_ENTRY_HELP,                   mmImage::img::HELP_PNG }
    };

    return navTyp < mmNavigatorItem::NAV_ENTRY_size ? defaultTyp[navTyp] : mmImage::img::SAVINGS_ACC_NORMAL_PNG;
}

mmNavigatorItem* mmNavigatorList::getAccountByNavType(int navTyp)
{
    for (int i = 0; i < static_cast<int>(m_account_type_entries.size()); i++) {
        if (m_account_type_entries[i]->type == navTyp) {
            return m_account_type_entries[i];
        }
    }
    return NULL;
}

bool mmNavigatorList::isAssetAccountActive()
{
    mmNavigatorItem* item = getAccountByNavType(mmNavigatorItem::TYPE_ID_ASSET);
    return item ? item->active : false;
}

void mmNavigatorList::setAssetAccountActive()
{
     mmNavigatorItem* item = getAccountByNavType(mmNavigatorItem::TYPE_ID_ASSET);
     if (item) {
        item->active = true;
     }
}
