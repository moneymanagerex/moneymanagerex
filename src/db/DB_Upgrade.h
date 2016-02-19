//=============================================================================
/**
 *      Copyright (c) 2016 - 2016 Gabriele-V
 *
 *      @author [sqliteupgrade2cpp.py]
 *
 *      @brief
 *
 *      Revision History:
 *          AUTO GENERATED at 2016-02-19 22:29:22.523000.
 *          DO NOT EDIT!
 */
//=============================================================================

#ifndef DB_UPGRADE_H_
#define DB_UPGRADE_H_

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
        -- Upgrade not needed, skip version to keep compatibility
    )",

    // Upgrade to version 2
    R"(
        -- Upgrade not needed, skip version to keep compatibility
    )",

    // Upgrade to version 3
    R"(
        -- Upgrade embedded in MMEX code before 1.3.0 version, moved here to keep compatibility with old DB
        UPDATE BILLSDEPOSITS_V1 SET TRANSDATE = NEXTOCCURRENCEDATE WHERE (SELECT INFOVALUE FROM INFOTABLE_V1 WHERE INFONAME = 'DATAVERSION')=2;
        UPDATE INFOTABLE_V1 SET INFOVALUE = 3 WHERE INFONAME = 'DATAVERSION';
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

#endif // DB_UPGRADE_H_
