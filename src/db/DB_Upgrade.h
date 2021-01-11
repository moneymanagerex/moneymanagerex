//=============================================================================
/**
 *      Copyright (c) 2016 - 2016 Gabriele-V
 *
 *      @author [sqliteupgrade2cpp.py]
 *
 *      @brief
 *
 *      Revision History:
 *          AUTO GENERATED at 2016-07-16 22:51:32.920000.
 *          DO NOT EDIT!
 */
//=============================================================================

#ifndef DB_UPGRADE_H_
#define DB_UPGRADE_H_

#include <vector>
#include <wx/string.h>

const int dbLatestVersion = 7;

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

    // Upgrade to version 5
    R"(
        -- CustomField
        CREATE TABLE IF NOT EXISTS CUSTOMFIELD_V1 (
        FIELDID INTEGER NOT NULL PRIMARY KEY
        , REFTYPE TEXT NOT NULL /* Transaction, Stock, Asset, BankAccount, RepeatingTransaction, Payee */
        , DESCRIPTION TEXT COLLATE NOCASE
        , TYPE TEXT NOT NULL /* String, Integer, Decimal, Boolean, Date, Time, SingleChoice, MultiChoice */
        , PROPERTIES TEXT NOT NULL
        );
        CREATE INDEX IF NOT EXISTS IDX_CUSTOMFIELD_REF ON CUSTOMFIELD_V1 (REFTYPE);
        
        -- CustomFieldData
        CREATE TABLE IF NOT EXISTS CUSTOMFIELDDATA_V1 (
        FIELDATADID INTEGER NOT NULL PRIMARY KEY
        , FIELDID INTEGER NOT NULL
        , REFID INTEGER NOT NULL
        , CONTENT TEXT
        , UNIQUE(FIELDID, REFID)
        );
        CREATE INDEX IF NOT EXISTS IDX_CUSTOMFIELDDATA_REF ON CUSTOMFIELDDATA_V1 (FIELDID, REFID);
    )",

    // Upgrade to version 6
    R"(
        -- describe TRANSACTIONLINK_V1
        CREATE TABLE IF NOT EXISTS TRANSLINK_V1 (
        TRANSLINKID  integer NOT NULL primary key
        , CHECKINGACCOUNTID integer NOT NULL
        , LINKTYPE TEXT NOT NULL /* Asset, Stock */
        , LINKRECORDID integer NOT NULL
        );
        CREATE INDEX IF NOT EXISTS IDX_LINKRECORD ON TRANSLINK_V1 (LINKTYPE, LINKRECORDID);
        CREATE INDEX IF NOT EXISTS IDX_CHECKINGACCOUNT ON TRANSLINK_V1 (CHECKINGACCOUNTID);
        
        -- describe SHAREINFO_V1
        CREATE TABLE IF NOT EXISTS SHAREINFO_V1 (
        SHAREINFOID  integer NOT NULL primary key
        , CHECKINGACCOUNTID integer NOT NULL
        , SHARENUMBER numeric
        , SHAREPRICE numeric
        , SHARECOMMISSION numeric
        , SHARELOT TEXT
        );
        CREATE INDEX IF NOT EXISTS IDX_SHAREINFO ON SHAREINFO_V1 (CHECKINGACCOUNTID);
    )",

    // Upgrade to version 7
    R"(
        alter table ACCOUNTLIST_V1 add column STATEMENTLOCKED integer;
        alter table ACCOUNTLIST_V1 add column STATEMENTDATE TEXT;
        alter table ACCOUNTLIST_V1 add column MINIMUMBALANCE numeric;
        alter table ACCOUNTLIST_V1 add column CREDITLIMIT numeric;
        alter table ACCOUNTLIST_V1 add column INTERESTRATE numeric;
        alter table ACCOUNTLIST_V1 add column PAYMENTDUEDATE text;
        alter table ACCOUNTLIST_V1 add column MINIMUMPAYMENT numeric;
        
    )",

};

#endif // DB_UPGRADE_H_
