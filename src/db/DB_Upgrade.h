//=============================================================================
/**
 *      Copyright (c) 2016 - 2025 Gabriele-V
 *
 *      @author [sqliteupgrade2cpp.py]
 *
 *      @brief
 *
 *      Revision History:
 *          AUTO GENERATED at 2025-02-04 16:22:20.096031.
 *          DO NOT EDIT!
 */
//=============================================================================

#ifndef DB_UPGRADE_H_
#define DB_UPGRADE_H_

#include <vector>
#include <wx/string.h>

const int dbLatestVersion = 20;

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

    // Upgrade to version 8
    R"(
        -- Upgrade not needed (we will report error if user is on this version)
    )",

    // Upgrade to version 9
    R"(
        -- Upgrade not needed (we will report error if user is on this version)
    )",

    // Upgrade to version 10
    R"(
        -- Upgrade not needed (we will report error if user is on this version)
    )",

    // Upgrade to version 11
    R"(
        -- Upgrade not needed (we will report error if user is on this version)
    )",

    // Upgrade to version 12
    R"(
        -- Upgrade not needed (we will report error if user is on this version)
    )",

    // Upgrade to version 13
    R"(
        -- Upgrade not needed (we will report error if user is on this version)
    )",

    // Upgrade to version 14
    R"(
        -- Upgrade not needed (we will report error if user is on this version)
    )",

    // Upgrade to version 15
    R"(
        -- db tidy, fix corrupt indices
        REINDEX;
        
        -- Will fail if v8-14 by recreating a column that exists in those versions
        -- so only upgrade from v1-v7 possible. Need to downgrade other versions prior to upgrade 
        alter table CURRENCYFORMATS_V1 add column CURRENCY_TYPE TEXT;
        update CURRENCYFORMATS_V1 set CURRENCY_TYPE = 'Fiat';
        update CURRENCYFORMATS_V1 set CURRENCY_TYPE = 'Crypto' where CURRENCY_SYMBOL NOT IN
        ('ADP', 'AED', 'AFA', 'AFN', 'ALK', 'ALL', 'AMD', 'ANG', 'AOA', 'AOK', 'AON', 'AOR', 'ARA', 
        'ARP', 'ARS', 'ARY', 'ATS', 'AUD', 'AWG', 'AYM', 'AZM', 'AZN', 'BAD', ' BAM', 'BBD', 'BDT', 
        'BEC', 'BEF', 'BEL', 'BGJ', 'BGK', 'BGL', 'BGN', 'BHD', 'BIF', 'BMD', 'BND', 'BOB', 'BOP', 
        'BRB', 'BRC', 'BRE', 'BRL', 'BRN', 'BRR', 'BSD', ' BTN', 'BUK', 'BWP', 'BYB', 'BYN', 'BYR', 
        'BZD', 'CAD', 'CDF', 'CHC', 'CHF', 'CLP', 'CNY', 'COP', 'CRC', 'CSD', 'CSJ', 'CSK', 'CUC', 
        'CUP', 'CVE', 'CYP', 'CZK', ' DDM', 'DEM', 'DJF', 'DKK', 'DOP', 'DZD', 'ECS', 'ECV', 'EEK', 
        'EGP', 'ERN', 'ESA', 'ESB', 'ESP', 'ETB', 'EUR', 'FIM', 'FJD', 'FKP', 'FRF', 'GBP', 'GEK', 
        'GEL', ' GHC', 'GHP', 'GHS', 'GIP', 'GMD', 'GNE', 'GNF', 'GNS', 'GQE', 'GRD', 'GTQ', 'GWE', 
        'GWP', 'GYD', 'HKD', 'HNL', 'HRD', 'HRK', 'HTG', 'HUF', 'IDR', 'IEP', 'ILP', ' ILR', 'ILS', 
        'INR', 'IQD', 'IRR', 'ISJ', 'ISK', 'ITL', 'JMD', 'JOD', 'JPY', 'KES', 'KGS', 'KHR', 'KMF', 
        'KPW', 'KRW', 'KWD', 'KYD', 'KZT', 'LAJ', 'LAK', 'LBP', ' LKR', 'LRD', 'LSL', 'LSM', 'LTL', 
        'LTT', 'LUC', 'LUF', 'LUL', 'LVL', 'LVR', 'LYD', 'MAD', 'MDL', 'MGA', 'MGF', 'MKD', 'MLF', 
        'MMK', 'MNT', 'MOP', 'MRO', 'MRU', ' MTL', 'MTP', 'MUR', 'MVQ', 'MVR', 'MWK', 'MXN', 'MXP', 
        'MYR', 'MZE', 'MZM', 'MZN', 'NAD', 'NGN', 'NIC', 'NIO', 'NLG', 'NOK', 'NPR', 'NZD', 'OMR', 
        'PAB', 'PEH', ' PEI', 'PEN', 'PES', 'PGK', 'PHP', 'PKR', 'PLN', 'PLZ', 'PTE', 'PYG', 'QAR', 
        'RHD', 'ROK', 'ROL', 'RON', 'RSD', 'RUB', 'RUR', 'RWF', 'SAR', 'SBD', 'SCR', 'SDD', ' SDG', 
        'SDP', 'SEK', 'SGD', 'SHP', 'SIT', 'SKK', 'SLL', 'SOS', 'SRD', 'SRG', 'SSP', 'STD', 'STN', 
        'SUR', 'SVC', 'SYP', 'SZL', 'THB', 'TJR', 'TJS', 'TMM', 'TMT', ' TND', 'TOP', 'TPE', 'TRL', 
        'TRY', 'TTD', 'TWD', 'TZS', 'UAH', 'UAK', 'UGS', 'UGW', 'UGX', 'USD', 'USS', 'UYN', 'UYP', 
        'UYU', 'UZS', 'VEB', 'VEF', 'VNC', 'VND', ' VUV', 'WST', 'XAF', 'XCD', 'XDR', 'XEU', 'XFO', 
        'XOF', 'XPF', 'YDD', 'YER', 'YUD', 'YUM', 'YUN', 'ZAL', 'ZAR', 'ZMK', 'ZMW', 'ZRN', 'ZRZ', 
        'ZWC', 'ZWD', 'ZWL', 'ZWN', 'ZWR', 'VUV', 'TND', 'SDG', 'LKR', 'BAM', 'BTN');
        
        -- Setup date of account initial balance
        -- https://github.com/moneymanagerex/moneymanagerex/issues/3554
        alter table ACCOUNTLIST_V1 add column INITIALDATE text;
        update ACCOUNTLIST_V1 SET INITIALDATE = ( select TRANSDATE from CHECKINGACCOUNT_V1 where 
            (ACCOUNTLIST_V1.ACCOUNTID = CHECKINGACCOUNT_V1.ACCOUNTID OR
            ACCOUNTLIST_V1.ACCOUNTID = CHECKINGACCOUNT_V1.TOACCOUNTID )
            order by TRANSDATE asc limit 1 );
        update ACCOUNTLIST_V1 SET INITIALDATE = (select PURCHASEDATE from STOCK_V1 where
            (ACCOUNTLIST_V1.ACCOUNTID = STOCK_V1.HELDAT)
            order by PURCHASEDATE asc limit 1 )  where INITIALDATE is null;
        update ACCOUNTLIST_V1 set INITIALDATE = date() where INITIALDATE is null;
        
        alter table ASSETS_V1 add column ASSETSTATUS TEXT;
        alter table ASSETS_V1 add column CURRENCYID integer;
        alter table ASSETS_V1 add column VALUECHANGEMODE TEXT;
        update ASSETS_V1 set ASSETSTATUS = 'Open', CURRENCYID = -1, VALUECHANGEMODE = 'Percentage';
        
        alter table BUDGETSPLITTRANSACTIONS_V1 add column NOTES TEXT;
        alter table SPLITTRANSACTIONS_V1 add column NOTES TEXT;
        
        alter table BUDGETTABLE_V1 add column NOTES TEXT;
        alter table BUDGETTABLE_V1 add column ACTIVE integer;
        update BUDGETTABLE_V1 set ACTIVE = 1;
        
        alter table CATEGORY_V1 add column ACTIVE integer;
        alter table SUBCATEGORY_V1 add column ACTIVE integer;
        update CATEGORY_V1 set ACTIVE = 1;
        update SUBCATEGORY_V1 set ACTIVE = 1;
        
        alter table PAYEE_V1 add column NUMBER TEXT;
        alter table PAYEE_V1 add column WEBSITE TEXT;
        alter table PAYEE_V1 add column NOTES TEXT;
        alter table PAYEE_V1 add column ACTIVE integer;
        update PAYEE_V1 set ACTIVE = 1;
        
        alter table REPORT_V1 add column ACTIVE integer;
        update REPORT_V1 set ACTIVE = 1;
        
        -- Tidy-up: This table was in the schema but has been removed and should not exist
        drop table if exists SPLITTRANSACTIONS_V2;
        
        
    )",

    // Upgrade to version 16
    R"(
        -- db tidy, fix corrupt indices
        REINDEX;
        
        alter table CHECKINGACCOUNT_V1 add column LASTUPDATEDTIME text;
        alter table CHECKINGACCOUNT_V1 add column DELETEDTIME text;
        
    )",

    // Upgrade to version 17
    R"(
        -- db tidy, fix corrupt indices
        REINDEX;
        
        -- DB Tidy, redundant views
        DROP VIEW IF EXISTS alldata;
        DROP VIEW IF EXISTS alldatax;
        DROP VIEW IF EXISTS budget;
        
        -- Nested Categories
        -- https://github.com/moneymanagerex/moneymanagerex/issues/1477
        CREATE TABLE TEMP_RENAME_CATEGORY(SOURCE integer primary key, TARGET integer not null);
        
        INSERT INTO TEMP_RENAME_CATEGORY(SOURCE, TARGET)
        SELECT SOURCE, TARGET
        FROM
            (
            SELECT S.CATEGID as SOURCE, T.CATEGID as TARGET
            FROM
                (
                SELECT C.CATEGID, D.CATEGNAME
                FROM 
                    (
                    SELECT CATEGNAME 
                    FROM CATEGORY_V1 
                    GROUP BY CATEGNAME 
                    HAVING COUNT(*) > 1
                    ) AS D 
                    INNER JOIN CATEGORY_V1 AS C
                ON C.CATEGNAME = D.CATEGNAME
                ) AS S
                INNER JOIN CATEGORY_V1 AS T
                ON S.CATEGNAME = T.CATEGNAME AND
                S.CATEGID > T.CATEGID
                GROUP BY SOURCE
                );
        
        UPDATE CHECKINGACCOUNT_V1 SET CATEGID = (SELECT TARGET FROM TEMP_RENAME_CATEGORY WHERE SOURCE = CATEGID) WHERE CATEGID IN (SELECT SOURCE FROM TEMP_RENAME_CATEGORY);
        UPDATE BILLSDEPOSITS_V1 SET CATEGID = (SELECT TARGET FROM TEMP_RENAME_CATEGORY WHERE SOURCE = CATEGID) WHERE CATEGID IN (SELECT SOURCE FROM TEMP_RENAME_CATEGORY);
        UPDATE BUDGETSPLITTRANSACTIONS_V1 SET CATEGID = (SELECT TARGET FROM TEMP_RENAME_CATEGORY WHERE SOURCE = CATEGID) WHERE CATEGID IN (SELECT SOURCE FROM TEMP_RENAME_CATEGORY);
        UPDATE SPLITTRANSACTIONS_V1 SET CATEGID = (SELECT TARGET FROM TEMP_RENAME_CATEGORY WHERE SOURCE = CATEGID) WHERE CATEGID IN (SELECT SOURCE FROM TEMP_RENAME_CATEGORY);
        UPDATE BUDGETTABLE_V1 SET CATEGID = (SELECT TARGET FROM TEMP_RENAME_CATEGORY WHERE SOURCE = CATEGID) WHERE CATEGID IN (SELECT SOURCE FROM TEMP_RENAME_CATEGORY);
        UPDATE PAYEE_V1 SET CATEGID = (SELECT TARGET FROM TEMP_RENAME_CATEGORY WHERE SOURCE = CATEGID) WHERE CATEGID IN (SELECT SOURCE FROM TEMP_RENAME_CATEGORY);
        UPDATE SUBCATEGORY_V1 SET CATEGID = (SELECT TARGET FROM TEMP_RENAME_CATEGORY WHERE SOURCE = CATEGID) WHERE CATEGID IN (SELECT SOURCE FROM TEMP_RENAME_CATEGORY);
        
        DELETE FROM CATEGORY_V1 WHERE CATEGID IN (SELECT SOURCE FROM TEMP_RENAME_CATEGORY);
        DROP TABLE TEMP_RENAME_CATEGORY;
        	
        CREATE TABLE TEMP_RENAME_SUBCATEGORY(SOURCE integer primary key, TARGET integer not null);
        
        INSERT INTO TEMP_RENAME_SUBCATEGORY(SOURCE, TARGET) 
        SELECT SOURCE, TARGET 
        FROM
            (
            SELECT S.SUBCATEGID as SOURCE, T.SUBCATEGID as TARGET
            FROM
            	(
            	SELECT S.SUBCATEGID, S.SUBCATEGNAME, S.CATEGID
                FROM SUBCATEGORY_V1 AS S 
                INNER JOIN 
                    (
                    SELECT SUBCATEGNAME, CATEGID 
                    FROM SUBCATEGORY_V1 
                    GROUP BY SUBCATEGNAME, CATEGID 
                    HAVING COUNT(*) > 1
                    ) AS D 
                ON S.SUBCATEGNAME = D.SUBCATEGNAME AND S.CATEGID = D.CATEGID
            	) as S
            	INNER JOIN SUBCATEGORY_V1 AS T
            	ON S.SUBCATEGNAME = T.SUBCATEGNAME AND
            	S.CATEGID = T.CATEGID AND
            	S.SUBCATEGID > T.SUBCATEGID
                GROUP BY SOURCE
                );
        
        UPDATE CHECKINGACCOUNT_V1 SET SUBCATEGID = (SELECT TARGET FROM TEMP_RENAME_SUBCATEGORY WHERE SOURCE = SUBCATEGID) WHERE SUBCATEGID IN (SELECT SOURCE FROM TEMP_RENAME_SUBCATEGORY);
        UPDATE BILLSDEPOSITS_V1 SET SUBCATEGID = (SELECT TARGET FROM TEMP_RENAME_SUBCATEGORY WHERE SOURCE = SUBCATEGID) WHERE SUBCATEGID IN (SELECT SOURCE FROM TEMP_RENAME_SUBCATEGORY);
        UPDATE BUDGETSPLITTRANSACTIONS_V1 SET SUBCATEGID = (SELECT TARGET FROM TEMP_RENAME_SUBCATEGORY WHERE SOURCE = SUBCATEGID) WHERE SUBCATEGID IN (SELECT SOURCE FROM TEMP_RENAME_SUBCATEGORY);
        UPDATE SPLITTRANSACTIONS_V1 SET SUBCATEGID = (SELECT TARGET FROM TEMP_RENAME_SUBCATEGORY WHERE SOURCE = SUBCATEGID) WHERE SUBCATEGID IN (SELECT SOURCE FROM TEMP_RENAME_SUBCATEGORY);
        UPDATE BUDGETTABLE_V1 SET SUBCATEGID = (SELECT TARGET FROM TEMP_RENAME_SUBCATEGORY WHERE SOURCE = SUBCATEGID) WHERE SUBCATEGID IN (SELECT SOURCE FROM TEMP_RENAME_SUBCATEGORY);
        UPDATE PAYEE_V1 SET SUBCATEGID = (SELECT TARGET FROM TEMP_RENAME_SUBCATEGORY WHERE SOURCE = SUBCATEGID) WHERE SUBCATEGID IN (SELECT SOURCE FROM TEMP_RENAME_SUBCATEGORY);
        
        DELETE FROM SUBCATEGORY_V1 WHERE SUBCATEGID IN (SELECT SOURCE FROM TEMP_RENAME_SUBCATEGORY);
        DROP TABLE TEMP_RENAME_SUBCATEGORY;
        
        CREATE TABLE CATEGORY_V1_TEMP
        ( CATEGID INTEGER PRIMARY KEY,
          CATEGNAME TEXT NOT NULL COLLATE NOCASE,
          ACTIVE INTEGER,
          PARENTID INTEGER,
          UNIQUE(CATEGNAME, PARENTID)
        );
        
        INSERT INTO CATEGORY_V1_TEMP (CATEGID, CATEGNAME, ACTIVE, PARENTID)
          SELECT CATEGID, CATEGNAME, ACTIVE, '-1'
          FROM CATEGORY_V1;
        
        INSERT INTO CATEGORY_V1_TEMP (CATEGID, CATEGNAME, ACTIVE, PARENTID)
        SELECT (SUBCATEGID + (SELECT MAX(CATEGID) FROM CATEGORY_V1_TEMP)), 
        	SUBCATEGNAME, ACTIVE, CATEGID
        FROM SUBCATEGORY_V1; 
        
        UPDATE CHECKINGACCOUNT_V1 SET CATEGID = SUBCATEGID + (SELECT MAX(CATEGID) FROM CATEGORY_V1) WHERE SUBCATEGID <> -1;
        UPDATE BILLSDEPOSITS_V1 SET CATEGID = SUBCATEGID + (SELECT MAX(CATEGID) FROM CATEGORY_V1) WHERE SUBCATEGID <> -1;
        UPDATE BUDGETSPLITTRANSACTIONS_V1 SET CATEGID = SUBCATEGID + (SELECT MAX(CATEGID) FROM CATEGORY_V1) WHERE SUBCATEGID <> -1;
        UPDATE SPLITTRANSACTIONS_V1 SET CATEGID = SUBCATEGID + (SELECT MAX(CATEGID) FROM CATEGORY_V1) WHERE SUBCATEGID <> -1;
        UPDATE BUDGETTABLE_V1 SET CATEGID = SUBCATEGID + (SELECT MAX(CATEGID) FROM CATEGORY_V1) WHERE SUBCATEGID <> -1;
        UPDATE PAYEE_V1 SET CATEGID = SUBCATEGID + (SELECT MAX(CATEGID) FROM CATEGORY_V1) WHERE SUBCATEGID <> -1;
        
        DROP TABLE CATEGORY_V1;					   
        ALTER TABLE CATEGORY_V1_TEMP RENAME TO CATEGORY_V1;
        CREATE INDEX IF NOT EXISTS IDX_CATEGORY_CATEGNAME ON CATEGORY_V1(CATEGNAME);
        CREATE INDEX IF NOT EXISTS IDX_CATEGORY_CATEGNAME_PARENTID ON CATEGORY_V1(CATEGNAME, PARENTID);																							   
        
        ALTER TABLE BILLSDEPOSITS_V1 DROP COLUMN SUBCATEGID; 
        ALTER TABLE BUDGETSPLITTRANSACTIONS_V1 DROP COLUMN SUBCATEGID; 
        ALTER TABLE BUDGETTABLE_V1 DROP COLUMN SUBCATEGID; 
        ALTER TABLE CHECKINGACCOUNT_V1 DROP COLUMN SUBCATEGID; 
        ALTER TABLE PAYEE_V1 DROP COLUMN SUBCATEGID; 
        ALTER TABLE SPLITTRANSACTIONS_V1 DROP COLUMN SUBCATEGID; 
        
        -- DB Tidy, redundant tables
        DROP TABLE IF EXISTS ASSETCLASS_V1;
        DROP TABLE IF EXISTS ASSETCLASS_STOCK_V1;
        DROP TABLE IF EXISTS SUBCATEGORY_V1;
        
    )",

    // Upgrade to version 18
    R"(
        -- db tidy, fix corrupt indices
        REINDEX;
        
        -- db tidy -- remove blank records caused by https://github.com/moneymanagerex/moneymanagerex/issues/5630
        DELETE FROM CHECKINGACCOUNT_V1 WHERE ACCOUNTID = '-1';
        
        -- Payee Matching
        -- https://github.com/moneymanagerex/moneymanagerex/issues/3148
        ALTER TABLE PAYEE_V1 ADD COLUMN 'PATTERN' TEXT DEFAULT '';
    )",

    // Upgrade to version 19
    R"(
        -- db tidy, fix corrupt indices
        REINDEX;
        
        -- Tags
        -- https://github.com/moneymanagerex/moneymanagerex/issues/5439
        -- Describe TAG_V1
        CREATE TABLE IF NOT EXISTS TAG_V1(
        TAGID INTEGER PRIMARY KEY
        , TAGNAME TEXT COLLATE NOCASE NOT NULL UNIQUE
        , ACTIVE INTEGER
        );
        CREATE INDEX IF NOT EXISTS IDX_TAGNAME ON TAG_V1 (TAGNAME);
        
        -- Describe TAGLINK_V1
        CREATE TABLE IF NOT EXISTS TAGLINK_V1(
        TAGLINKID INTEGER PRIMARY KEY
        , REFTYPE TEXT NOT NULL
        , REFID INTEGER NOT NULL
        , TAGID INTEGER NOT NULL
        , UNIQUE(REFTYPE, REFID, TAGID)
        );
        CREATE INDEX IF NOT EXISTS IDX_TAGLINK ON TAGLINK_V1 (REFTYPE, REFID, TAGID);
        
        -- Migrate FOLLOWUPID to COLOR
        ALTER TABLE CHECKINGACCOUNT_V1 ADD COLUMN COLOR INTEGER DEFAULT -1;
        ALTER TABLE BILLSDEPOSITS_V1 ADD COLUMN COLOR INTEGER DEFAULT -1;
        
        UPDATE CHECKINGACCOUNT_V1 SET COLOR = FOLLOWUPID;
        UPDATE BILLSDEPOSITS_v1 SET COLOR = FOLLOWUPID;
    )",

    // Upgrade to version 20
    R"(
        -- db tidy, fix corrupt indices
        REINDEX;
        
        -- realign missing category #7175
        -- set as inactive with <recovery> info
        UPDATE CATEGORY_V1
          set active = 0,
              PARENTID = -1,
              CATEGNAME = CATEGNAME || " [recovered]"
          where CATEGID = PARENTID;
        
        -- To alleviate future issues we are normalizing the TRANSDATE column
        UPDATE CHECKINGACCOUNT_V1 SET TRANSDATE = TRANSDATE || 'T00:00:00' WHERE LENGTH(TRANSDATE)=10;
    )",

};

#endif // DB_UPGRADE_H_
