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

#include "model/Model_Account.h"

struct NavigatorTypesInfo {
    int id;
    wxString name;
    int seq_no;
    int imageId;
    int navTyp;
    bool active;
};

class NavigatorTypes
{
    public:
        enum {
            NAV_TYP_PANEL = 0,
            NAV_TYP_ACCOUNT,
            NAV_TYP_OTHER
        };

        enum {
            NAV_ENTRY_DASHBOARD = Model_Account::TYPE_ID_size,
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
        NavigatorTypesInfo* getFirstAccount();
        NavigatorTypesInfo* getNextAccount(NavigatorTypesInfo* previous);
        NavigatorTypesInfo* getFirstActiveEntry();
        NavigatorTypesInfo* getNextActiveEntry(NavigatorTypesInfo* previous);
        void SaveSequenceAndState();
        void SetToDefault();
        bool DeleteEntry(NavigatorTypesInfo* info);
        NavigatorTypesInfo& FindOrCreateEntry(int searchId);

        wxArrayString* GetCustomCheckingAccounts();

    private:
        std::vector<NavigatorTypesInfo> m_entries;
        long unsigned int t_lastIdx;
        NavigatorTypesInfo* t_previous;
        int m_maxId;

        wxArrayString m_customChecking;

        void sortEntriesBySeq();
        void loadFromInfoTable();
};
