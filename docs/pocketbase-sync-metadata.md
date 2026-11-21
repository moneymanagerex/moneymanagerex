# PocketBase sync metadata (planned)

The row-level sync metadata belongs to each MMEX database file, not to the
reference schema repository and not to ordinary `SETTING_V1` preferences. Four
small internal tables are planned:

- `MMEX_SYNC_CHANGE`: one pending local insert/update/delete per collection and
  MMEX business key. SQLite triggers record these changes in the same
  transaction as the source row.
- `MMEX_SYNC_ROW_STATE`: last acknowledged PocketBase `updated` value and local
  sync confirmation for each collection/business key.
- `MMEX_SYNC_CURSOR`: per-collection incremental pull cursor, ordered by
  PocketBase `updated` plus PocketBase record ID; pull pages overlap slightly
  and deduplicate by MMEX business key.
- `MMEX_SYNC_CONFLICT`: unresolved row-level conflicts, with local and remote
  versions retained until the user chooses a winner.

PocketBase `updated` is remote version information, not a replacement for local
dirty tracking. SUID remains the shared business key. Deletes require durable
tombstones. Full initial pull establishes the first row states; later pulls use
the collection cursor. These tables are planned and have not been added yet.
