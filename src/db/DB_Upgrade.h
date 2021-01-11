//=============================================================================
/**
 *      Copyright (c) 2016 - 2021 Gabriele-V
 *
 *      @author [sqliteupgrade2cpp.py]
 *
 *      @brief
 *
 *      Revision History:
 *          AUTO GENERATED at 2021-01-11 22:51:56.180291.
 *          DO NOT EDIT!
 */
//=============================================================================

#ifndef DB_UPGRADE_H_
#define DB_UPGRADE_H_

#include <vector>
#include <wx/string.h>

const int dbLatestVersion = 8;

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
        
        CREATE TABLE IF NOT EXISTS TICKER_V1 (
        TICKERID INTEGER PRIMARY KEY,
        SOURCE INTEGER, /* Yahoo, MorningStar, MOEX */
        SYMBOL TEXT COLLATE NOCASE NOT NULL,
        MARKET TEXT, 
        SOURCENAME TEXT,
        TYPE INTEGER DEFAULT 0, /* Share, Fund, Bond */
        COUNTRY TEXT, 
        SECTOR TEXT, /*Basic Materials
        , Consumer Cyclical
        , Financial Services
        , Real Estate
        , Consumer Defensive
        , Healthcare
        , Utilities
        , Communication Services
        , Energy
        , Industrials
        , Technology
        , Other */ 
        INDUSTRY TEXT,
        WEBPAGE TEXT,
        NOTES TEXT,
        PRECISION INTEGER,
        CURRENCYID INTEGER NOT NULL,
        FOREIGN KEY (CURRENCYID) REFERENCES CURRENCYFORMATS_V1(CURRENCYID) 
        );
        CREATE INDEX IF NOT EXISTS IDX_TICKER ON TICKER_V1 (SYMBOL, TICKERID);
        
        
        insert into TICKER_V1 (SOURCE, SYMBOL, MARKET, PRECISION, CURRENCYID)
        select distinct 0 SOURCE
          , case when INSTR(SYMBOL, '.') > 0 then substr(SYMBOL, 1, INSTR(SYMBOL, '.') -1) else SYMBOL end  SYMBOL 
          , case when INSTR(SYMBOL, '.') > 0 then substr(SYMBOL, INSTR(SYMBOL, '.')+ 1) else '' end  MARKET
          , IFNULL((select INFOVALUE from INFOTABLE_V1 where INFONAME = 'SHARE_PRECISION'), 2) PRECISION 
          , (select c.CURRENCYID from ACCOUNTLIST_V1 a
                 left join CURRENCYFORMATS_V1 c on c.CURRENCYID=a.CURRENCYID
        		 where a.ACCOUNTID = s.HELDAT) CURRENCYID
          from STOCK_V1 s;
        
        
        CREATE TABLE STOCK_NEW(
        STOCKID integer primary key
        , TICKERID integer NOT NULL
        , HELDAT integer
        , PURCHASEDATE TEXT NOT NULL
        , NUMSHARES numeric
        , PURCHASEPRICE numeric NOT NULL
        , NOTES TEXT
        , COMMISSION numeric
        , FOREIGN KEY (TICKERID) REFERENCES TICKER_V1(TICKERID) 
        );
        
        INSERT INTO STOCK_NEW 
        with t as (
        select TICKERID,
        (CASE WHEN T.MARKET ='' THEN T.SYMBOL ELSE T.SYMBOL||'.'||T.MARKET END) SYMBOL
        FROM TICKER_V1 T)
        SELECT
        STOCKID,
        t.TICKERID,
        HELDAT,
        PURCHASEDATE,
        NUMSHARES,
        PURCHASEPRICE,
        NOTES,
        COMMISSION
        FROM STOCK_V1 s, t
        where t.SYMBOL = s.SYMBOL;
        
        DROP INDEX IDX_STOCK_HELDAT;
        DROP TABLE STOCK_V1;
        ALTER TABLE STOCK_NEW RENAME TO STOCK_V1;
        CREATE INDEX IDX_STOCK_HELDAT ON STOCK_V1(HELDAT);
        
        
        CREATE TABLE STOCKHISTORY_NEW(
        HISTID integer primary key
        , TICKERID INTEGER NOT NULL
        , DATE TEXT NOT NULL
        , VALUE numeric NOT NULL
        , UPDTYPE integer
        , UNIQUE(TICKERID, DATE)
        , FOREIGN KEY (TICKERID) REFERENCES TICKER_V1(TICKERID)
        );
        
        INSERT INTO STOCKHISTORY_NEW (TICKERID, DATE, VALUE, UPDTYPE)
        with t as (
        select TICKERID,
        (CASE WHEN T.MARKET ='' THEN T.SYMBOL ELSE T.SYMBOL||'.'||T.MARKET END) SYMBOL
        FROM TICKER_V1 T)
        SELECT 
        t.TICKERID
        , DATE, VALUE, UPDTYPE
        FROM STOCKHISTORY_V1 S, t
        where S.SYMBOL = t.SYMBOL;
        
        
        DROP INDEX IDX_STOCKHISTORY_SYMBOL;
        DROP TABLE STOCKHISTORY_V1;
        ALTER TABLE STOCKHISTORY_NEW RENAME TO STOCKHISTORY_V1;
        CREATE INDEX IDX_STOCKHISTORY_SYMBOL ON STOCKHISTORY_V1(TICKERID);
        
        DROP TABLE IF EXISTS SHAREINFO_V1;
        DROP TABLE IF EXISTS TRANSLINK_V1;
        DROP TABLE IF EXISTS ASSETCLASS_V1;
        DROP TABLE IF EXISTS ASSETCLASS_STOCK_V1;
        
        insert into ACCOUNTLIST_V1(
          ACCOUNTNAME
        , ACCOUNTTYPE
        , STATUS
        , INITIALBAL
        , FAVORITEACCT
        , CURRENCYID)
        VALUES (
        'Asset_'||strftime('%Y-%m-%d', 'now')
        , 'Asset'
        , 'Open'
        , 0
        , 'TRUE'
        , (select INFOVALUE from INFOTABLE_V1 where INFONAME='BASECURRENCYID'));
        
        
        CREATE TABLE ASSETS_NEW(
          ASSETID integer primary key
        , STARTDATE TEXT NOT NULL
        , ENDDATE TEXT
        , ASSETNAME TEXT COLLATE NOCASE NOT NULL
        , VALUE numeric
        , VALUECHANGE TEXT /* Value, Percentage */
        , VALUECHANGERATE numeric default 0
        , ASSETTYPE TEXT /* Property, Automobile, Household Object, Art, Jewellery, Cash, Other */
        , NOTES TEXT
        , ACCOUNTID INTEGER NOT NULL
        , FOREIGN KEY (ACCOUNTID) REFERENCES ACCOUNTLIST_V1(ACCOUNTID) 
        );
        
        INSERT INTO ASSETS_NEW (
          ASSETID
        , STARTDATE
        , ASSETNAME
        , VALUE
        , VALUECHANGE
        , VALUECHANGERATE
        , ASSETTYPE
        , NOTES
        , ACCOUNTID)
        select 
          ASSETID
        , STARTDATE
        , ASSETNAME
        , VALUE
        , 'Percentage' as VALUECHANGE 
        , case when VALUECHANGE='Appreciates' then VALUECHANGERATE when VALUECHANGE='Depreciates' then -VALUECHANGERATE else 0 end as VALUECHANGERATE
        , ASSETTYPE
        , NOTES
        , (select ACCOUNTID from ACCOUNTLIST_V1 where ACCOUNTNAME='Asset_'||strftime('%Y-%m-%d', 'now')) as ACCOUNTID
        FROM ASSETS_V1;
        
        DROP INDEX IDX_ASSETS_ASSETTYPE;
        DROP TABLE ASSETS_V1;
        ALTER TABLE ASSETS_NEW RENAME TO ASSETS_V1;
        CREATE INDEX IDX_ASSETS_ASSETTYPE ON ASSETS_V1(ASSETTYPE);
        
        ALTER TABLE ACCOUNTLIST_V1 ADD COLUMN MULTICURRENCY integer DEFAULT 0;
        
        ALTER TABLE CHECKINGACCOUNT_V1 ADD COLUMN CURRENCYID integer;
        ALTER TABLE CHECKINGACCOUNT_V1 ADD COLUMN COLOURID integer;
        ALTER TABLE CHECKINGACCOUNT_V1 ADD COLUMN TRANSTIME TEXT;
        
        ALTER TABLE BILLSDEPOSITS_V1 ADD COLUMN CURRENCYID integer;
        ALTER TABLE BILLSDEPOSITS_V1 ADD COLUMN COLOURID integer;
        ALTER TABLE BILLSDEPOSITS_V1 ADD COLUMN TRANSTIME TEXT;
        
        update CHECKINGACCOUNT_V1 set COLOURID = case when FOLLOWUPID >=0 and FOLLOWUPID <= 7 then FOLLOWUPID end;
        update ACCOUNTLIST_V1 set MULTICURRENCY = 1 where ACCOUNTTYPE in ('Investment');
        
        PRAGMA user_version = 8;
        
    )",

};

#endif // DB_UPGRADE_H_
