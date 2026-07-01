/*******************************************************
 Copyright (C) 2025, 2026 Klaus Wich

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

#pragma once

#include "base/_defs.h"

struct mmNavigatorItem
{
public:
    enum NAV_TYP
    {
        NAV_TYP_PANEL_STATIC = 0,
        NAV_TYP_PANEL,
        NAV_TYP_STOCK,
        NAV_TYP_ACCOUNT,
        NAV_TYP_OTHER
    };

    enum TYPE_ID
    {
        TYPE_ID_CASH = 0,                    // NAV_TYP_ACCOUNT
        TYPE_ID_CHECKING,                    // NAV_TYP_ACCOUNT
        TYPE_ID_CREDIT_CARD,                 // NAV_TYP_ACCOUNT
        TYPE_ID_LOAN,                        // NAV_TYP_ACCOUNT
        TYPE_ID_TERM,                        // NAV_TYP_ACCOUNT
        TYPE_ID_INVESTMENT,                  // NAV_TYP_STOCK
        TYPE_ID_ASSET,                       // NAV_TYP_OTHER
        TYPE_ID_SHARES,                      // NAV_TYP_OTHER
        TYPE_ID_size
    };

    enum NAV_ENTRY
    {
        NAV_ENTRY_DASHBOARD = TYPE_ID_size,  // NAV_TYP_PANEL
        NAV_ENTRY_ALL_TRANSACTIONS,          // NAV_TYP_PANEL
        NAV_ENTRY_SCHEDULED_TRANSACTIONS,    // NAV_TYP_PANEL
        NAV_ENTRY_FAVORITES,                 // NAV_TYP_PANEL
        NAV_ENTRY_BUDGET_PLANNER,            // NAV_TYP_PANEL
        NAV_ENTRY_TRANSACTION_REPORT,        // NAV_TYP_PANEL
        NAV_ENTRY_REPORTS,                   // NAV_TYP_PANEL
        NAV_ENTRY_GRM,                       // NAV_TYP_PANEL
        NAV_ENTRY_DELETED_TRANSACTIONS,      // NAV_TYP_PANEL
        NAV_ENTRY_HELP,                      // NAV_TYP_PANEL
        NAV_ENTRY_size
    };

    static const int NAV_IDXDIFF = mmNavigatorItem::NAV_ENTRY_size - mmNavigatorItem::TYPE_ID_size;

public:
    int type;           // enum TYPE_ID or NAV_ENTRY
    wxString name;      // Visible name in navigator
    wxString choice;    // Name used in selection lists
    wxString dbaccid;   // DB Account identifier (not to be translated!)
    int seq_no;
    int imageId;
    int navTyp;         // enum NAV_TYP
    bool active;
};

class mmNavigatorList
{
// -- state

private:
    std::vector<mmNavigatorItem*> m_navigator_entries;
    std::vector<mmNavigatorItem*> m_account_type_entries;
    long unsigned int   m_lastIdx;
    mmNavigatorItem* m_previous;

// -- constructor

public:
    mmNavigatorList();
    ~mmNavigatorList();

    static mmNavigatorList& instance();

// -- methods

public:
    void LoadFromDB(bool keepnames = false);
    mmNavigatorItem* getFirstAccount();
    mmNavigatorItem* getNextAccount(mmNavigatorItem* previous);
    mmNavigatorItem* getFirstActiveEntry();
    mmNavigatorItem* getNextActiveEntry(mmNavigatorItem* previous);

    void SaveSequenceAndState();
    void SetToDefault();
    bool DeleteEntry(mmNavigatorItem* info);
    mmNavigatorItem* FindOrCreateEntry(int searchId);
    mmNavigatorItem* FindEntry(int searchId);
    wxString FindEntryName(int searchId);

    wxString getAccountSectionName(int account_type);
    const wxString type_name(int id);
    int getTypeIdFromDBName(
        const wxString& dbname,
        int default_id = mmNavigatorItem::TYPE_ID_CHECKING
    );
    int getTypeIdFromChoice(
        const wxString& choice,
        int default_id = mmNavigatorItem::TYPE_ID_CHECKING
    );
    int getNumberOfAccountTypes();

    int getAccountTypeIdx(const wxString& atypename);
    int getAccountTypeIdx(int account_type);
    wxString getAccountTypeName(int idx);
    bool isAccountTypeAsset(int idx);
    bool isAssetAccountActive();
    void setAssetAccountActive();
    mmNavigatorItem* getAccountTypeItem(int idx);
    wxArrayString getAccountSelectionNames(wxString filter = "");
    wxString getAccountDbTypeFromChoice(const wxString& choiceName);
    wxArrayString getUsedAccountTypeNames();
    wxString getAccountDbTypeFromName(const wxString& typeName);

    wxString getInvestmentAccountStr() {
        return type_name(mmNavigatorItem::TYPE_ID_INVESTMENT);
    };
    bool isInvestmentAccount(wxString accountType) {
        return accountType == type_name(mmNavigatorItem::TYPE_ID_INVESTMENT);
    };
    bool isShareAccount(wxString accountType) {
        return accountType == type_name(mmNavigatorItem::TYPE_ID_SHARES);
    };
    wxString getShareAccountStr() {
        return type_name(mmNavigatorItem::TYPE_ID_SHARES);
    };
    bool isAssetAccount(wxString accountType) {
        return accountType == type_name(mmNavigatorItem::TYPE_ID_ASSET);
    };
    wxString getAssetAccountStr() {
        return type_name(mmNavigatorItem::TYPE_ID_ASSET);
    };
    wxString getCashAccountStr() {
        return type_name(mmNavigatorItem::TYPE_ID_CASH);
    };

    void SetTrashStatus(bool state);
    void SetShareAccountStatus(bool state);
    static wxString GetTranslatedName(mmNavigatorItem*);
    static wxString GetTranslatedSelection(mmNavigatorItem*);
    static int GetDefaultImage(int navTyp);

private:
    void sortEntriesBySeq();
    int getMaxId();
    mmNavigatorItem* getAccountByNavType(int navTyp);
};
