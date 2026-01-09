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

#pragma once

#include "choices.h"
#include "defs.h"


struct NavigatorTypesInfo {
    int id;
    wxString name;
    wxString choice;
    int seq_no;
    int imageId;
    int navTyp;
    bool active;
};



class NavigatorTypes
{
    public:
        struct AccountItem {
            int id;
            int seq_no;
            wxString name;
            AccountItem(int i, int s, wxString n) : id(i), seq_no(s), name(n) {}
        };

    public:
        enum {
            NAV_TYP_PANEL_STATIC = 0,
            NAV_TYP_PANEL,
            NAV_TYP_ACCOUNT,
            NAV_TYP_STOCK,
            NAV_TYP_OTHER
        };

        enum TYPE_ID
        {
            TYPE_ID_CASH = 0,
            TYPE_ID_CHECKING,
            TYPE_ID_CREDIT_CARD,
            TYPE_ID_LOAN,
            TYPE_ID_TERM,
            TYPE_ID_INVESTMENT,
            TYPE_ID_ASSET,
            TYPE_ID_SHARES,
            TYPE_ID_size
        };

        enum {
            NAV_ENTRY_DASHBOARD = TYPE_ID_size,
            NAV_ENTRY_ALL_TRANSACTIONS,
            NAV_ENTRY_SCHEDULED_TRANSACTIONS,
            NAV_ENTRY_FAVORITES,
            NAV_ENTRY_BUDGET_PLANNER,
            NAV_ENTRY_TRANSACTION_REPORT,
            NAV_ENTRY_REPORTS,
            NAV_ENTRY_GRM,
            NAV_ENTRY_DELETED_TRANSACTIONS,
            NAV_ENTRY_HELP,
            NAV_ENTRY_size
        };

    public:
        NavigatorTypes();
        static NavigatorTypes& instance();
        void LoadFromInfoTable();
        NavigatorTypesInfo* getFirstAccount();
        NavigatorTypesInfo* getNextAccount(NavigatorTypesInfo* previous);
        NavigatorTypesInfo* getFirstActiveEntry();
        NavigatorTypesInfo* getNextActiveEntry(NavigatorTypesInfo* previous);
        void SaveSequenceAndState();
        void SetToDefault();
        bool DeleteEntry(NavigatorTypesInfo* info);
        NavigatorTypesInfo* FindOrCreateEntry(int searchId);
        NavigatorTypesInfo* FindEntry(int searchId);
        wxString FindEntryName(int searchId);

        wxString getAccountSectionName(int account_type);
        const wxString type_name(int id);
        int type_id(const wxString& name, int default_id = TYPE_ID_CHECKING);
        int getNumberOfAccountTypes();
        NavigatorTypes::AccountItem* getAccountTypeItem(int idx);

        wxString getInvestmentAccountStr() {return type_name(NavigatorTypes::TYPE_ID_INVESTMENT);};
        bool isInvestmentAccount(wxString accountType) {return accountType == type_name(NavigatorTypes::TYPE_ID_INVESTMENT);};
        bool isShareAccount(wxString accountType) {return accountType == type_name(NavigatorTypes::TYPE_ID_SHARES);};
        wxString getShareAccountStr() {return type_name(NavigatorTypes::TYPE_ID_SHARES);};
        bool isAssetAccount(wxString accountType) {return accountType == type_name(NavigatorTypes::TYPE_ID_ASSET);};
        wxString getAssetAccountStr() {return type_name(NavigatorTypes::TYPE_ID_ASSET);};
        wxString getCashAccountStr() {return type_name(NavigatorTypes::TYPE_ID_CASH);};

        void SetTrashStatus(bool state);


    private:
        std::vector<NavigatorTypesInfo> m_navigator_entries;
        std::vector<AccountItem> m_type_choices;

        long unsigned int t_lastIdx;
        NavigatorTypesInfo* t_previous;
        int m_maxId;

        wxArrayString m_customChecking;

        void sortEntriesBySeq();
        void updateTypeChoiceName(int id, int seq_no, wxString name);

};
