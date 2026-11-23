#pragma once

#include <wx/string.h>
#include <cstddef>

struct PocketBaseSyncResult {
    bool ok = false;
    wxString message;
    size_t imported = 0;
    size_t created = 0;
    size_t unchanged = 0;
    size_t skipped = 0;
    size_t conflicts = 0;
};


// PocketBase synchronization service. This interface is independent of UI.
class PocketBaseSyncService
{
public:
    static PocketBaseSyncResult syncCurrencies(const wxString& base, const wxString& token);
    static PocketBaseSyncResult syncAccounts(const wxString& base, const wxString& token);
    static PocketBaseSyncResult syncCategories(const wxString& base, const wxString& token);
    static PocketBaseSyncResult syncPayees(const wxString& base, const wxString& token);
    static PocketBaseSyncResult syncTransactions(const wxString& base, const wxString& token);
    static PocketBaseSyncResult syncInvestments(const wxString& base, const wxString& token);
    static PocketBaseSyncResult syncAssets(const wxString& base, const wxString& token);
    static PocketBaseSyncResult syncStocks(const wxString& base, const wxString& token);
    static PocketBaseSyncResult syncStockHistory(const wxString& base, const wxString& token);
};
