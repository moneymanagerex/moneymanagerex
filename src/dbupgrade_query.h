/*******************************************************
Copyright (C) 2016 Gabriele-V

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
********************************************************/

#ifndef MM_EX_DBUPGRADE_QUERY_H_
#define MM_EX_DBUPGRADE_QUERY_H_

#include <vector>
#include <wx/string.h>

const int dbLatestVersion = 5;

const std::vector<wxString> dbUpgradeQuery =
{
    // Upgrade to version 0
    "",
    // Upgrade to version 1
    "",
    // Upgrade to version 2
    "",
    // Upgrade to version 3
    "",
    // Upgrade to version 4
    R"(
        --Asset Classes
        CREATE TABLE IF NOT EXISTS ASSETCLASS_V1(
            'ID' INTEGER primary key,
            'PARENTID' INTEGER,
            'NAME' TEXT COLLATE NOCASE NOT NULL,
            'ALLOCATION' REAL,
            'SORTORDER' INTEGER
        );

        --Asset Class / Stock link table
        CREATE TABLE IF NOT EXISTS ASSETCLASS_STOCK_V1(
            'ID' INTEGER primary key,
            'ASSETCLASSID' INTEGER NOT NULL,
            'STOCKSYMBOL' TEXT UNIQUE
        );
    )",
    // Upgrade to version 5
    R"(
        -- SPLITTRANSACTIONS_V1
        CREATE TEMPORARY TABLE SPLITTRANSACTIONS_V1_TEMP AS SELECT * FROM SPLITTRANSACTIONS_V1;

        DROP TABLE SPLITTRANSACTIONS_V1;

        CREATE TABLE SPLITTRANSACTIONS_V1(
        SPLITTRANSID integer primary key
        , ACCOUNTID integer NOT NULL
        , TRANSID integer NOT NULL
        , CATEGID integer
        , SUBCATEGID integer
        , SPLITTRANSAMOUNT numeric
        , NOTES TEXT
        );
        CREATE INDEX IDX_SPLITTRANSACTIONS_ACCOUNTID ON SPLITTRANSACTIONS_V1(ACCOUNTID);
        CREATE INDEX IDX_SPLITTRANSACTIONS_TRANSID ON SPLITTRANSACTIONS_V1(TRANSID);

        INSERT INTO SPLITTRANSACTIONS_V1
        SELECT SPLITTRANSID, ACCOUNTID, S.TRANSID, S.CATEGID, S.SUBCATEGID, SPLITTRANSAMOUNT, ''
        FROM SPLITTRANSACTIONS_V1_TEMP S INNER JOIN CHECKINGACCOUNT_V1 T ON S.TRANSID = T.TRANSID;

        DROP TABLE SPLITTRANSACTIONS_V1_TEMP;


        -- BUDGETSPLITTRANSACTIONS_V1 
        CREATE TEMPORARY TABLE BUDGETSPLITTRANSACTIONS_V1_TEMP AS SELECT * FROM BUDGETSPLITTRANSACTIONS_V1;

        DROP TABLE BUDGETSPLITTRANSACTIONS_V1;

        CREATE TABLE BUDGETSPLITTRANSACTIONS_V1(
        SPLITTRANSID integer primary key
        , ACCOUNTID integer NOT NULL
        , TRANSID integer NOT NULL
        , CATEGID integer
        , SUBCATEGID integer
        , SPLITTRANSAMOUNT numeric
        , NOTES TEXT
        );
        CREATE INDEX IDX_BUDGETSPLITTRANSACTIONS_ACCOUNTID ON BUDGETSPLITTRANSACTIONS_V1(ACCOUNTID);
        CREATE INDEX IDX_BUDGETSPLITTRANSACTIONS_TRANSID ON BUDGETSPLITTRANSACTIONS_V1(TRANSID);

        INSERT INTO BUDGETSPLITTRANSACTIONS_V1
        SELECT SPLITTRANSID, ACCOUNTID, S.TRANSID, S.CATEGID, S.SUBCATEGID, SPLITTRANSAMOUNT, ''
        FROM BUDGETSPLITTRANSACTIONS_V1_TEMP S INNER JOIN BILLSDEPOSITS_V1 BD ON S.TRANSID = BD.BDID;

        DROP TABLE BUDGETSPLITTRANSACTIONS_V1_TEMP;
    )",
};

#endif // MM_EX_DBUPGRADE_QUERY_H_