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

#include "toolbartypes.h"
#include "mmframe.h"
#include "constants.h"
#include "Model_Setting.h"


static const wxString TOOLBAR_INFO_KEY = "TOOLBAR_SETTINGS";

ToolBarEntries::ToolBarEntries()
{
    m_lastIdx = 0;
    m_previous = nullptr;
    m_toolbarParent = nullptr;
    Load();
}

ToolBarEntries::~ToolBarEntries()
{
    for (auto ref : m_toolbar_entries) {
        delete ref;
    }
}


ToolBarEntries& ToolBarEntries::instance()
{
    return Singleton<ToolBarEntries>::instance();
}

void ToolBarEntries::SetToDefault()
{
    // Delete old entries
    for (auto ref : m_toolbar_entries) {
        delete ref;
    }
    m_toolbar_entries.clear();

    // init with default entries
    m_toolbar_entries.push_back(new  ToolBarEntry{mmGUIFrame::MENU_NEW,                    "New",                    "New Database",                       0, -1, png::NEW_DB,          true,  TOOLBAR_BTN});
    m_toolbar_entries.push_back(new  ToolBarEntry{mmGUIFrame::MENU_OPEN,                   "Open",                   "Open Database",                      1, -1, png::OPEN,            true,  TOOLBAR_BTN});
    m_toolbar_entries.push_back(new  ToolBarEntry{0,                                       "",                       "",                                   2, -1, -1,                   true,  TOOLBAR_SEPARATOR});
    m_toolbar_entries.push_back(new  ToolBarEntry{mmGUIFrame::MENU_NEWACCT,                "New Account",            "New Account",                        4, -1, png::NEW_ACC,         true,  TOOLBAR_BTN});
    m_toolbar_entries.push_back(new  ToolBarEntry{mmGUIFrame::MENU_HOMEPAGE,               "Dashboard",              "Open Dashboard",                     5, -1, png::HOME,            true,  TOOLBAR_BTN});
    m_toolbar_entries.push_back(new  ToolBarEntry{0,                                       "",                       "",                                   6, -1, -1,                   true,  TOOLBAR_SEPARATOR});
    m_toolbar_entries.push_back(new  ToolBarEntry{wxID_NEW,                                "New",                    "New Transaction",                    7, -1, png::NEW_TRX,         true,  TOOLBAR_BTN});
    m_toolbar_entries.push_back(new  ToolBarEntry{0,                                       "",                       "",                                   8, -1, -1,                   true,  TOOLBAR_SEPARATOR});
    m_toolbar_entries.push_back(new  ToolBarEntry{mmGUIFrame::MENU_ORGPAYEE,               "Payee Manager",          "Payee Manager",                      9, -1, png::PAYEE,           true,  TOOLBAR_BTN});
    m_toolbar_entries.push_back(new  ToolBarEntry{mmGUIFrame::MENU_ORGCATEGS,              "Category Manager",       "Category Manager",                  10, -1, png::CATEGORY,        true,  TOOLBAR_BTN});
    m_toolbar_entries.push_back(new  ToolBarEntry{mmGUIFrame::MENU_ORGTAGS,                "Tag Manager",            "Tag Manager",                       11, -1, png::TAG,             true,  TOOLBAR_BTN});
    m_toolbar_entries.push_back(new  ToolBarEntry{mmGUIFrame::MENU_CURRENCY,               "Currency Manager",       "Currency Manager",                  12, -1, png::CURR,            true,  TOOLBAR_BTN});
    m_toolbar_entries.push_back(new  ToolBarEntry{0,                                       "",                       "",                                  13, -1, -1,                   true,  TOOLBAR_SEPARATOR});
    m_toolbar_entries.push_back(new  ToolBarEntry{mmGUIFrame::MENU_TRANSACTIONREPORT,      "Transaction Report",     "Transaction Report",                14, -1, png::FILTER,          true,  TOOLBAR_BTN});
    m_toolbar_entries.push_back(new  ToolBarEntry{0,                                       "",                       "",                                  15, -1, -1,                   true,  TOOLBAR_SEPARATOR});
    m_toolbar_entries.push_back(new  ToolBarEntry{wxID_VIEW_LIST,                          "General Report Manager", "General Report Manager",            16, -1, png::GRM,             true,  TOOLBAR_BTN});
    m_toolbar_entries.push_back(new  ToolBarEntry{mmGUIFrame::MENU_RATES,                  "Download Rates",         "Download currency and stock rates", 17, -1, png::CURRATES,        true,  TOOLBAR_BTN});
    m_toolbar_entries.push_back(new  ToolBarEntry{0,                                       "",                       "",                                  18, -1, -1,                   true,  TOOLBAR_SEPARATOR});
    m_toolbar_entries.push_back(new  ToolBarEntry{wxID_PRINT,                              "&Print",                 "Print",                             19, -1, png::PRINT,           true,  TOOLBAR_BTN});
    m_toolbar_entries.push_back(new  ToolBarEntry{0,                                       "",                       "",                                  20, -1, -1,                   true,  TOOLBAR_SEPARATOR});
    m_toolbar_entries.push_back(new  ToolBarEntry{0,                                       "",                       "",                                  21, -1, -1,                   true,  TOOLBAR_STRETCH});
    m_toolbar_entries.push_back(new  ToolBarEntry{0,                                       "",                       "",                                  22, -1, -1,                   true,  TOOLBAR_SEPARATOR});
    m_toolbar_entries.push_back(new  ToolBarEntry{MENU_VIEW_TOGGLE_FULLSCREEN,             "Full Screen\tF11",       "Toggle full screen",                23, -1, png::FULLSCREEN,      true,  TOOLBAR_BTN});
    m_toolbar_entries.push_back(new  ToolBarEntry{wxID_PREFERENCES,                        "&Settings",              "Settings",                          24, -1, png::OPTIONS,         true,  TOOLBAR_BTN});
    m_toolbar_entries.push_back(new  ToolBarEntry{0,                                       "",                       "",                                  25, -1, -1,                   true,  TOOLBAR_SEPARATOR});
    m_toolbar_entries.push_back(new  ToolBarEntry{mmGUIFrame::MENU_ANNOUNCEMENTMAILING,    "News",                   "News",                              26, -1, png::NEW_NEWS,        true,  TOOLBAR_BTN});
    m_toolbar_entries.push_back(new  ToolBarEntry{wxID_ABOUT,                              "&About",                 "About",                             27, -1, png::ABOUT,           true,  TOOLBAR_BTN});
    m_toolbar_entries.push_back(new  ToolBarEntry{wxID_HELP,                               "&Help\tF1",              "Help",                              28, -1, png::HELP,            true,  TOOLBAR_BTN});
    m_toolbar_entries.push_back(new  ToolBarEntry{MENU_BILLSDEPOSITS,                      "Scheduled Transactions", "Scheduled Transactions",            29, -1, png::RECURRING,       false, TOOLBAR_BTN});
    m_toolbar_entries.push_back(new  ToolBarEntry{MENU_BUDGETSETUPDIALOG,                  "Budget Planner",         "Budget Planner",                    30, -1, png::BUDGET,          false, TOOLBAR_BTN});
    m_toolbar_entries.push_back(new  ToolBarEntry{MENU_TRANSACTIONS_ALL,                   "All Transactions",       "All Transactions",                  31, -1, png::ALLTRANSACTIONS, false, TOOLBAR_BTN});
    m_toolbar_entries.push_back(new  ToolBarEntry{MENU_TRANSACTIONS_DEL,                   "Deleted Transactions",   "Deleted Transactions",              32, -1, png::TRASH,           false, TOOLBAR_BTN});
}

void ToolBarEntries::SortEntriesBySeq()
{
    std::stable_sort(m_toolbar_entries.begin(), m_toolbar_entries.end(), [](ToolBarEntry* a, ToolBarEntry* b) {
        return (a->seq_no < b->seq_no);
    });
}

wxImageList* ToolBarEntries::getImageList(){
    const int toolbar_icon_size = Option::instance().getToolbarIconSize();
    wxImageList* imageList = new wxImageList(toolbar_icon_size, toolbar_icon_size);
    int iidx = 0;
    ToolBarEntries::ToolBarEntry* ainfo = getFirstEntry();
    while (ainfo != nullptr) {
        if (ainfo->type == TOOLBAR_BTN) {
            wxBitmapBundle bundle = mmBitmapBundle(ainfo->imageId, toolbar_icon_size);
            imageList->Add(bundle.GetBitmap(wxSize(toolbar_icon_size, toolbar_icon_size)));
            ainfo->imageListID = iidx++;
        }
        ainfo = ToolBarEntries::instance().getNextEntry(ainfo);
    }
    return imageList;
}

ToolBarEntries::ToolBarEntry* ToolBarEntries::getFirstEntry()
{
    m_lastIdx = 0;
    m_previous = m_toolbar_entries[0];
    return m_previous;
}

ToolBarEntries::ToolBarEntry* ToolBarEntries::getNextEntry(ToolBarEntry* previous)
{
    if (previous == m_previous) {
        m_lastIdx++;
        m_previous = m_lastIdx < size(m_toolbar_entries) ? m_toolbar_entries[m_lastIdx] : nullptr;
    }
    else {
        wxLogError("Invalid use of ToolBarEntries::getNextEntry");
        m_previous = nullptr;
    }
    return m_previous;
}

ToolBarEntries::ToolBarEntry* ToolBarEntries::newEntry(TYPE_ID type, ToolBarEntry* previous)
{
    ToolBarEntry* entry = new ToolBarEntry;
    entry->type = type;
    entry->imageId = -1;
    entry->active = true;
    if (previous) {
        int seqBase = previous->seq_no;
        for (ToolBarEntry* tentry : m_toolbar_entries) {
            if (tentry->seq_no >= seqBase) {
                tentry->seq_no += 1;
            }
        }
        entry->seq_no = seqBase;
    }
    else {
        entry->seq_no = m_toolbar_entries.size();
    }
    m_toolbar_entries.push_back(entry);
    SortEntriesBySeq();
    return entry;
}

bool ToolBarEntries::DeleteEntry(ToolBarEntry* info)
{
    bool result = false;
    for (int i = 0; i < static_cast<int>(m_toolbar_entries.size()); i++) {
        if (m_toolbar_entries[i] == info) {
            m_toolbar_entries.erase(m_toolbar_entries.begin() + i);
            result = true;
            break;
        }
    }
    return result;
}

void ToolBarEntries::Save() {
    Document j_doc = Model_Setting::instance().getJdoc(TOOLBAR_INFO_KEY, "{}");
    j_doc.SetObject();
    rapidjson::Value array(rapidjson::kArrayType);

    for (ToolBarEntry* entry : m_toolbar_entries) {
        if (entry->seq_no > -1) {
            rapidjson::Value label(entry->label.ToUTF8().data(), j_doc.GetAllocator());
            rapidjson::Value helpstring(entry->helpstring.ToUTF8().data(), j_doc.GetAllocator());

            rapidjson::Value obj(rapidjson::kObjectType);
            obj.AddMember("toolid",      entry->toolId,      j_doc.GetAllocator());
            obj.AddMember("label",       label,             j_doc.GetAllocator());
            obj.AddMember("helpstring",  helpstring,        j_doc.GetAllocator());
            obj.AddMember("seq_no",      entry->seq_no,      j_doc.GetAllocator());
            obj.AddMember("imageId",     entry->imageId,     j_doc.GetAllocator());
            obj.AddMember("active",      entry->active,      j_doc.GetAllocator());
            obj.AddMember("type",        entry->type,        j_doc.GetAllocator());
            array.PushBack(obj, j_doc.GetAllocator());
        }
    }
    j_doc.AddMember("data", array, j_doc.GetAllocator());
    Model_Setting::instance().setJdoc(TOOLBAR_INFO_KEY, j_doc);

    if (m_toolbarParent) {  // update toolbar
        m_toolbarParent->PopulateToolBar();
    }
}

void ToolBarEntries::Load() {
    m_toolbar_entries.clear();
    bool doReset = false;
    Document doc = Model_Setting::instance().getJdoc(TOOLBAR_INFO_KEY, "{}");
    if (doc.HasMember("data") && doc["data"].IsArray()) {
        try {
            const rapidjson::Value& array = doc["data"];
            for (rapidjson::SizeType i = 0; i < array.Size(); ++i) {
                ToolBarEntry* entry = new ToolBarEntry;
                const rapidjson::Value& obj = array[i];
                entry->toolId = obj["toolid"].GetInt();
                const char* chars = obj["label"].GetString();
                entry->label = wxString::FromUTF8(chars);
                chars = obj["helpstring"].GetString();
                entry->helpstring = wxString::FromUTF8(chars);
                entry->seq_no = obj["seq_no"].GetInt();
                entry->imageId = obj["imageId"].GetInt();
                entry->active = obj["active"].GetBool();
                entry->type = static_cast<TYPE_ID>(obj["type"].GetInt());
                m_toolbar_entries.push_back(entry);
            }
        }
        catch (...) {
            doReset = true;
        }
    }
    else {
        doReset = true;
    }
    if (doReset) {
        wxLogDebug("Could not read Toolbar configuration from Info Table => reset to default");
        m_toolbar_entries.clear();
        SetToDefault();
    }
}

void ToolBarEntries::SetToolbarParent(mmGUIFrame* parent)
{
    m_toolbarParent = parent;
}
