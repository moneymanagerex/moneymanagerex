#pragma once

#include <string>

namespace pocketbase_sync {

// The shared business key is an MMEX table primary key (SUID for modern rows).
struct CollectionSpec
{
    const char* collection;
    const char* primaryKey;
};

enum class ChangeSide { None, Local, Remote, Both };

// Three-way row comparison. The baseline is the last state acknowledged by
// both peers; it must be persisted per collection and business key.
inline ChangeSide classify(const std::string& baseline,
                           const std::string& local,
                           const std::string& remote)
{
    const bool localChanged = local != baseline;
    const bool remoteChanged = remote != baseline;
    if (localChanged && remoteChanged) return ChangeSide::Both;
    if (localChanged) return ChangeSide::Local;
    if (remoteChanged) return ChangeSide::Remote;
    return ChangeSide::None;
}

inline constexpr CollectionSpec currencies{
    "CURRENCYFORMATS_V1", "CURRENCYID"};
inline constexpr CollectionSpec accounts{
    "ACCOUNTLIST_V1", "ACCOUNTID"};

} // namespace pocketbase_sync
