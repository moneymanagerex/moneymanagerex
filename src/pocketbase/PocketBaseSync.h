#pragma once

#include <wx/string.h>

struct PocketBaseSyncResult {
    bool ok = false;
    wxString message;
};


// PocketBase synchronization service. This interface is independent of UI.
class PocketBaseSyncService
{
public:
    static PocketBaseSyncResult syncCurrencies(const wxString& base, const wxString& token);
    static PocketBaseSyncResult syncAccounts(const wxString& base, const wxString& token);
    static PocketBaseSyncResult syncCategories(const wxString& base, const wxString& token);
};
