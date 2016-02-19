//=============================================================================
/**
 *      Copyright (c) 2016 - 2016 Gabriele-V
 *
 *      @author [sqliteupgrade2cpp.py]
 *
 *      @brief
 *
 *      Revision History:
 *          AUTO GENERATED at 2016-02-19 21:55:44.628000.
 *          DO NOT EDIT!
 */
//=============================================================================

#ifndef MM_EX_DBUPGRADE_QUERY_H_
#define MM_EX_DBUPGRADE_QUERY_H_

#include <vector>
#include <wx/string.h>

const int dbLatestVersion = 4;

const std::vector<wxString> dbUpgradeQuery =
{
    // Upgrade to version 0
    R"(
        -- First version
    )",

    // Upgrade to version 1
    R"(
        -- Upgrade not needed, skip version to keep compatibility 1
    )",

    // Upgrade to version 2
    R"(
        -- Upgrade not needed, skip version to keep compatibility 2
    )",

    // Upgrade to version 3
    R"(
        -- Upgrade not needed, skip version to keep compatibility
    )",

    // Upgrade to version 4
    R"(
        -- Asset Classes
        CREATE TABLE IF NOT EXISTS ASSETCLASS_V1 (
            'ID' INTEGER primary key,
            'PARENTID' INTEGER,
            'NAME' TEXT COLLATE NOCASE NOT NULL,
            'ALLOCATION' REAL,
            'SORTORDER' INTEGER
        );
        
        -- Asset Class / Stock link table
        CREATE TABLE IF NOT EXISTS ASSETCLASS_STOCK_V1 (
            'ID' INTEGER primary key,
            'ASSETCLASSID' INTEGER NOT NULL,
            'STOCKSYMBOL' TEXT UNIQUE
        );
    )",

};

#endif // MM_EX_DBUPGRADE_QUERY_H_
