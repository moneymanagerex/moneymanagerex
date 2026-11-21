#pragma once


class NetworkPref;

struct PocketBaseSyncResult {
    bool ok = false;
    wxString message;
};


// Coordinates collection synchronization independently from the preferences UI.
// NetworkPref is passed only as the current progress/status sink for this first
// extraction; request and record logic lives in this service.
class PocketBaseSyncService
{
public:
    static PocketBaseSyncResult syncCurrencies(const wxString& base, const wxString& token);
    static PocketBaseSyncResult syncAccounts(const wxString& base, const wxString& token);
    static PocketBaseSyncResult syncCategories(const wxString& base, const wxString& token);
};
