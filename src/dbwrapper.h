/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

//----------------------------------------------------------------------------
#ifndef _MM_EX_DBWRAPPER_H_
#define _MM_EX_DBWRAPPER_H_
//----------------------------------------------------------------------------
#include "mmcurrency.h"
#include <wx/arrstr.h>
class MMEX_IniSettings;

//----------------------------------------------------------------------------
static const char SELECT_ALL_FROM_ASSETS_V1[] =
    "select a.ASSETNAME as ASSETNAME, "
    "a.STARTDATE as STARTDATE, "
    "a.ASSETTYPE as ASSETTYPE, "
    "a.value + (a.value/100*VALUECHANGERATE) "
    "* (case VALUECHANGE when 'Appreciates' then 1 when 'Depreciates' then -1 else 0 end ) "
    "* (julianday('now', 'localtime') - julianday(a.STARTDATE, 'localtime')) / 365 as TODAY_VALUE, "
    "a.NOTES as NOTES, "
    "a.STARTDATE as STARTDATE, "
    "a.VALUECHANGE as VALUECHANGE, "
    "a.ASSETID as ASSETID, "
    "a.VALUECHANGERATE as VALUECHANGERATE, "
    "a.value as VALUE "
    "from ASSETS_V1 a ";

static const char SELECT_ALL_FROM_ACCOUNTLIST_V1[] =
    "SELECT * "
    "FROM ACCOUNTLIST_V1 "
    "ORDER BY ACCOUNTNAME";

static const char SELECT_ALL_INVESTMENT_FROM_ACCOUNTLIST_V1[] =
    "SELECT ACCOUNTNAME, ACCOUNTID "
    "FROM ACCOUNTLIST_V1 "
    "WHERE ACCOUNTTYPE = 'Investment' "
    "ORDER BY ACCOUNTNAME";

static const char SELECT_ALL_FROM_BILLSDEPOSITS_V1[] =
    "SELECT BDID, "
    "ACCOUNTID, "
    "TOACCOUNTID, "
    "PAYEEID, "
    "TRANSCODE, "
    "TRANSAMOUNT, "
    "STATUS, "
    "TRANSACTIONNUMBER, "
    "NOTES, "
    "CATEGID, "
    "SUBCATEGID, "
    "TRANSDATE, "
    "FOLLOWUPID, "
    "TOTRANSAMOUNT, "
    "REPEATS, "
    "NEXTOCCURRENCEDATE, "
    "NUMOCCURRENCES "
    "FROM BILLSDEPOSITS_V1";

static const char SELECT_ROW_FROM_BUDGETSPLITTRANSACTIONS_V1[] =
    "select SPLITTRANSID, "
    "SPLITTRANSAMOUNT, "
    "CATEGID, "
    "SUBCATEGID "
    "from BUDGETSPLITTRANSACTIONS_V1 "
    "where TRANSID = ?";

static const char SELECT_ROW_FROM_BUDGETTABLE_V1[] =
    "SELECT PERIOD, "
    "AMOUNT "
    "FROM BUDGETTABLE_V1 "
    "WHERE BUDGETYEARID = ? AND "
    "CATEGID = ? AND "
    "SUBCATEGID = ?";

static const char SELECT_ALL_FROM_CATEGORY_V1[] =
    "SELECT CATEGID, CATEGNAME "
    "FROM CATEGORY_V1 "
    "ORDER BY CATEGNAME";

static const char SELECT_ALL_FROM_CHECKINGACCOUNT_V1[] =
    "SELECT * FROM CHECKINGACCOUNT_V1"
    " ORDER BY TRANSDATE, TRANSID ";

static const char IS_TRX_IN_CHECKINGACCOUNT_V1[] =
    "select TRANSID "
    "from CHECKINGACCOUNT_V1 "
    "where ACCOUNTID = ? and "
    "TOACCOUNTID = ? and "
    "PAYEEID = ? and "
    "TRANSCODE = ? and "
    "TRANSAMOUNT = ? and "
    "TRANSACTIONNUMBER = ? and "
    "NOTES = ? and "
    "CATEGID = ? and "
    "SUBCATEGID = ? and "
    "TRANSDATE = ? and "
    "TOTRANSAMOUNT = ? and "
    "TRANSID > 0"; // is not null

static const char SELECT_ROW_FROM_CURRENCYFORMATS_V1[] =
    "SELECT CURRENCYNAME, "
    "PFX_SYMBOL, "
    "SFX_SYMBOL, "
    "DECIMAL_POINT, "
    "GROUP_SEPARATOR, "
    "UNIT_NAME, "
    "CENT_NAME, "
    "SCALE, "
    "BASECONVRATE, "
    "CURRENCY_SYMBOL "
    "FROM CURRENCYFORMATS_V1 "
    "WHERE CURRENCYID = ?";

static const char SELECT_ALL_FROM_CURRENCYFORMATS_V1[] =
    "SELECT * "
    "FROM CURRENCYFORMATS_V1 "
    "ORDER BY CURRENCYNAME";

static const char DELETE_CURRENCYID_FROM_CURRENCYFORMATS_V1[] =
    "DELETE FROM CURRENCYFORMATS_V1 "
    "WHERE CURRENCYID = ?";

static const char INSERT_INTO_CURRENCYFORMATS_V1[] =
    "INSERT INTO CURRENCYFORMATS_V1 ( "
    "CURRENCYNAME, PFX_SYMBOL, SFX_SYMBOL, DECIMAL_POINT, "
    "GROUP_SEPARATOR, UNIT_NAME, CENT_NAME, SCALE, BASECONVRATE, CURRENCY_SYMBOL "
    " ) VALUES ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )";

static const char UPDATE_CURRENCYFORMATS_V1[] =
    "update CURRENCYFORMATS_V1 "
    "set PFX_SYMBOL=?, SFX_SYMBOL=?, DECIMAL_POINT=?,"
    "GROUP_SEPARATOR=?, UNIT_NAME=?, CENT_NAME=?, "
    "SCALE=?, BASECONVRATE=?, CURRENCY_SYMBOL=?, "
    "CURRENCYNAME=? "
    "where CURRENCYID = ?";

static const char SELECT_INFOVALUE_FROM_INFOTABLE_V1[] =
    "SELECT INFOVALUE FROM INFOTABLE_V1 "
    "where INFONAME = ?";

static const char SELECT_ALL_FROM_PAYEE_V1[] =
    "SELECT PAYEEID, PAYEENAME, CATEGID, SUBCATEGID "
    "FROM PAYEE_V1 "
    "ORDER BY PAYEENAME";

static const char SELECT_ROW_FROM_SETTING_V1[] =
    "SELECT SETTINGVALUE "
    "FROM SETTING_V1 "
    "WHERE SETTINGNAME = ?";

static const char SELECT_ROW_FROM_SPLITTRANSACTIONS_V1[] =
    "select SPLITTRANSID, "
    "SPLITTRANSAMOUNT, "
    "CATEGID, "
    "SUBCATEGID "
    "from SPLITTRANSACTIONS_V1 "
    "where TRANSID = ?";

static const char SELECT_SUBCATEGNAME_FROM_SUBCATEGORY_V1[] =
    "SELECT SUBCATEGNAME, "
    "SUBCATEGID "
    "FROM SUBCATEGORY_V1 "
    "WHERE CATEGID = ? AND "
    "SUBCATEGID = ?";

static const char SELECT_SUBCATEGID_FROM_SUBCATEGORY_V1[] =
    "SELECT SUBCATEGID "
    "FROM SUBCATEGORY_V1 "
    "WHERE SUBCATEGNAME = ? AND "
    "CATEGID = ?";

static const char SELECT_SUBCATEGS_FROM_SUBCATEGORY_V1[] =
    "SELECT SUBCATEGID, SUBCATEGNAME "
    "FROM SUBCATEGORY_V1 "
    "WHERE CATEGID = ? "
    "ORDER BY SUBCATEGNAME";

static const char SELECT_ALL_CATEGORIES[] =
    "SELECT C.CATEGID, C.CATEGNAME "
    "FROM CATEGORY_V1 C "
    "ORDER BY C.CATEGNAME ";

static const char SELECT_ALL_SUBCATEGORIES[] =
    "SELECT S.SUBCATEGID, S.SUBCATEGNAME, "
    "S.CATEGID "
    "FROM SUBCATEGORY_V1 S "
    "ORDER BY S.SUBCATEGNAME";

//DELETE
static const char DELETE_BUDGETENTRYIDS_FROM_BUDGETTABLE_V1[] =
    "DELETE FROM BUDGETTABLE_V1 "
    "where BUDGETENTRYID in (select BUDGETENTRYID "
    "from BUDGETTABLE_V1 "
    "WHERE BUDGETYEARID = ? AND "
    "CATEGID = ? AND "
    "SUBCATEGID = ? "
    ")";

static const char DELETE_FROM_BUDGETTABLE_V1[] =
    "DELETE FROM BUDGETTABLE_V1 where BUDGETENTRYID = ?";

static const char DELETE_FROM_PAYEE_V1[] =
    "DELETE FROM PAYEE_V1 WHERE PAYEEID = ?";

static const char DELETE_FROM_SPLITTRANSACTIONS_V1[] =
    "DELETE FROM SPLITTRANSACTIONS_V1 "
    "where TRANSID in( SELECT TRANSID "
    "from CHECKINGACCOUNT_V1 "
    "where ACCOUNTID = ? OR TOACCOUNTID = ? )";

static const char DELETE_TRANSID_SPLITTRANSACTIONS_V1[] =
    "DELETE FROM SPLITTRANSACTIONS_V1 where TRANSID = ?";

static const char DELETE_FROM_BUDGETSPLITTRANSACTIONS_V1[] =
    "DELETE FROM BUDGETSPLITTRANSACTIONS_V1 "
    "WHERE TRANSID IN ( SELECT BDID "
    "FROM BILLSDEPOSITS_V1 "
    "WHERE ACCOUNTID = ? OR TOACCOUNTID = ? )";

static const char SELECT_ALL_SYMBOLS_FROM_STOCK_V1[] =
    "SELECT STOCKID, "
    "SYMBOL, "
    "CURRENTPRICE, "
    "NUMSHARES, "
    "STOCKNAME "
    "FROM STOCK_V1 ";

static const char SELECT_ROW_HELDAT_FROM_STOCK_V1[] =
    "select "
    "S.PURCHASEDATE as PURCHASEDATE, "
    "S.STOCKNAME, "
    "S.NUMSHARES as NUMSHARES, "
    "S.VALUE as VALUE, "
    "(S.VALUE - (S.NUMSHARES*S.PURCHASEPRICE + S.COMMISSION)) as GAIN_LOSS, "
    "S.CURRENTPRICE as CURRENTPRICE, "
    "S.NOTES, "
    "T.AVG_PURCHASEPRICE as AVG_PURCHASEPRICE, "
    "S.PURCHASEPRICE as PURCHASEPRICE, "
    "S.STOCKID, S.HELDAT, UPPER(S.SYMBOL) SYMBOL, "
    "S.COMMISSION as COMMISSION, "
    "julianday('now', 'localtime')-julianday (S.PURCHASEDATE, 'localtime') as DAYSOWN, "
    "t.TOTAL_NUMSHARES, T.PURCHASEDTIME, "
    "ifnull (strftime(INFOVALUE, S.PURCHASEDATE),strftime(replace (i.infovalue, '%y', SubStr (strftime('%Y', S.PURCHASEDATE),3,2)),S.PURCHASEDATE)) as PURCHDATE "
    "from STOCK_V1 S "
    "left join infotable_v1 i on i.INFONAME='DATEFORMAT' "
    "left join ( "
    "select count (UPPER (SYMBOL)) as PURCHASEDTIME, "
    "HELDAT, UPPER (SYMBOL) as SYMBOL, "
    "total (NUMSHARES) as TOTAL_NUMSHARES, "
    "total(PURCHASEPRICE*NUMSHARES)/total(NUMSHARES) as AVG_PURCHASEPRICE "
    "from STOCK_V1 "
    "left join infotable_v1 i on i.INFONAME='DATEFORMAT' "
    "group by HELDAT, UPPER (SYMBOL) "
    "order by julianday(min (PURCHASEDATE),'localtime'), SYMBOL, STOCKNAME "
    ") T on UPPER (T.SYMBOL)=UPPER (S.SYMBOL) and T.HELDAT=S.HELDAT "
    "where S.HELDAT = ? ";

static const char SELECT_ROW_FROM_STOCK_V1[] =
    "SELECT HELDAT, STOCKNAME, SYMBOL, NOTES, "
    "PURCHASEDATE, NUMSHARES, VALUE, "
    "PURCHASEPRICE, CURRENTPRICE, COMMISSION "
    "FROM STOCK_V1 "
    "WHERE STOCKID = ?";

//UPDATE
static const char UPDATE_ACCOUNTLIST_V1[] =
    "UPDATE ACCOUNTLIST_V1 "
    "SET ACCOUNTNAME=?, ACCOUNTTYPE=?, ACCOUNTNUM=?"
    ",STATUS=?, NOTES=?, HELDAT=?, WEBSITE=?, CONTACTINFO=?,  ACCESSINFO=?"
    ",INITIALBAL=?, FAVORITEACCT=?, CURRENCYID=? "
    "where ACCOUNTID = ?";

static const char UPDATE_CATEGORY_V1[] =
    "UPDATE CATEGORY_V1 "
    "SET CATEGNAME = ? "
    "WHERE CATEGID = ?";

static const char SET_STATUS_CHECKINGACCOUNT_V1[] =
    "update CHECKINGACCOUNT_V1 set STATUS=? where TRANSID=?";

static const char SET_PAYEEID_CHECKINGACCOUNT_V1[] =
    "UPDATE CHECKINGACCOUNT_V1 SET PAYEEID = ? WHERE PAYEEID = ? ";

static const char UPDATE_CHECKINGACCOUNT_V1[] =
    "UPDATE CHECKINGACCOUNT_V1 "
    "SET ACCOUNTID=?, TOACCOUNTID=?, PAYEEID=?, TRANSCODE=? "
    ", TRANSAMOUNT=?, STATUS=?, TRANSACTIONNUMBER=?, NOTES=? "
    ", CATEGID=?, SUBCATEGID=?, TRANSDATE=?, FOLLOWUPID=? "
    ", TOTRANSAMOUNT=? "
    "WHERE TRANSID = ?";

static const char UPDATE_INFOTABLE_V1[] =
    "UPDATE INFOTABLE_V1 SET INFOVALUE=? WHERE INFONAME=?";

static const char UPDATE_PAYEE_V1[] =
    "UPDATE PAYEE_V1 "
    "SET PAYEENAME = ?, "
    "CATEGID = ?, "
    "SUBCATEGID = ? "
    "WHERE PAYEEID = ?";

static const char UPDATE_STOCK_V1[] =
    "UPDATE STOCK_V1 "
    "SET CURRENTPRICE = ?, VALUE = ?, STOCKNAME = ? "
    "WHERE STOCKID = ?";

static const char UPDATE_ROW_STOCK_V1[]  =
    "UPDATE STOCK_V1 "
    "SET HELDAT=?, PURCHASEDATE=?, STOCKNAME=?, SYMBOL=?, "
    "NUMSHARES=?, PURCHASEPRICE=?, NOTES=?, CURRENTPRICE=?, "
    "VALUE=?, COMMISSION=? "
    "WHERE STOCKID = ?";

static const char UPDATE_SUBCATEGORY_V1[] =
    "UPDATE SUBCATEGORY_V1 "
    "SET SUBCATEGNAME = ? "
    "WHERE SUBCATEGID = ?";

//INSERT
static const char INSERT_INTO_ACCOUNTLIST_V1[] =
    "INSERT INTO ACCOUNTLIST_V1 ( "
    "ACCOUNTNAME, ACCOUNTTYPE, ACCOUNTNUM, "
    "STATUS, NOTES, HELDAT, WEBSITE, CONTACTINFO, ACCESSINFO, "
    "INITIALBAL, FAVORITEACCT, CURRENCYID "
    " ) values (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )";

static const char INSERT_INTO_BUDGETTABLE_V1[] =
    "INSERT INTO BUDGETTABLE_V1 ( "
    "BUDGETYEARID, "
    "CATEGID, "
    "SUBCATEGID, "
    "PERIOD, "
    "AMOUNT "
    ") values ( "
    "?, ?, ?, ?, ? "
    ")";

static const char INSERT_INTO_CATEGORY_V1[] =
    "INSERT INTO CATEGORY_V1 (CATEGNAME) VALUES(?)";

static const char INSERT_INTO_CHECKINGACCOUNT_V1[] =
    "INSERT INTO CHECKINGACCOUNT_V1 ( "
    "ACCOUNTID, TOACCOUNTID, PAYEEID, TRANSCODE, "
    "TRANSAMOUNT, STATUS, TRANSACTIONNUMBER, NOTES, "
    "CATEGID, SUBCATEGID, TRANSDATE, FOLLOWUPID, TOTRANSAMOUNT "
    ") values ( ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )";

static const char INSERT_INTO_INFOTABLE_V1[] =
    "INSERT INTO INFOTABLE_V1 (INFONAME, INFOVALUE) VALUES (?, ?)";

static const char INSERT_INTO_PAYEE_V1[] =
    "INSERT INTO PAYEE_V1 (PAYEENAME, CATEGID, SUBCATEGID) VALUES (?, ?, ?)";

static const char INSERT_INTO_SPLITTRANSACTIONS_V1[] =
    "INSERT INTO SPLITTRANSACTIONS_V1 (TRANSID, CATEGID, SUBCATEGID, SPLITTRANSAMOUNT) "
    "VALUES (?, ?, ?, ?)";

static const char INSERT_ROW_INTO_STOCK_V1[]  =
    "INSERT INTO STOCK_V1 ( "
    "HELDAT, PURCHASEDATE, STOCKNAME, SYMBOL, "
    "NUMSHARES, PURCHASEPRICE, NOTES, CURRENTPRICE, "
    "VALUE, COMMISSION "
    " ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

static const char INSERT_ROW_INTO_SUBCATEGORY_V1[] =
    "INSERT INTO SUBCATEGORY_V1 "
    "( SUBCATEGNAME, CATEGID ) "
    "VALUES (:name, :id)";

//CREATE
static const char CREATE_TABLE_ACCOUNTLIST_V1[] =
    "CREATE TABLE ACCOUNTLIST_V1(ACCOUNTID integer primary key, "
    "ACCOUNTNAME TEXT NOT NULL UNIQUE, ACCOUNTTYPE TEXT NOT NULL, ACCOUNTNUM TEXT, "
    "STATUS TEXT NOT NULL, "
    "NOTES TEXT, HELDAT TEXT, WEBSITE TEXT, CONTACTINFO TEXT, "
    "ACCESSINFO TEXT, INITIALBAL numeric, FAVORITEACCT TEXT NOT NULL, "
    "CURRENCYID integer NOT NULL, "
    "CHECK (CURRENCYID > 0 and ACCOUNTID > 0))";

static const char CREATE_TABLE_ASSETS_V1[] =
    "CREATE TABLE ASSETS_V1 (ASSETID integer primary key, "
    "STARTDATE TEXT NOT NULL, ASSETNAME TEXT NOT NULL UNIQUE, "
    "VALUE numeric, VALUECHANGE TEXT, NOTES TEXT, VALUECHANGERATE numeric, "
    "ASSETTYPE TEXT)";

static const char CREATE_TABLE_BILLSDEPOSITS_V1[] =
    "CREATE TABLE BILLSDEPOSITS_V1 (BDID INTEGER PRIMARY KEY, "
    "ACCOUNTID INTEGER NOT NULL, TOACCOUNTID INTEGER, PAYEEID integer NOT NULL, TRANSCODE TEXT NOT NULL, "
    "TRANSAMOUNT numeric NOT NULL, STATUS TEXT, TRANSACTIONNUMBER TEXT, NOTES TEXT, "
    "CATEGID integer, SUBCATEGID integer, TRANSDATE TEXT, FOLLOWUPID integer, TOTRANSAMOUNT numeric, "
    "REPEATS numeric, NEXTOCCURRENCEDATE TEXT, NUMOCCURRENCES numeric)";

static const char CREATE_TABLE_BUDGETTABLE_V1[] =
    "CREATE TABLE BUDGETTABLE_V1(BUDGETENTRYID integer primary key, "
    "BUDGETYEARID integer, CATEGID integer, SUBCATEGID integer, "
    "PERIOD TEXT NOT NULL, AMOUNT numeric NOT NULL)";

static const char CREATE_TABLE_BUDGETSPLITTRANSACTIONS_V1[]=
    "CREATE TABLE BUDGETSPLITTRANSACTIONS_V1(SPLITTRANSID integer primary key, "
    "TRANSID integer NOT NULL, CATEGID integer, SUBCATEGID integer, SPLITTRANSAMOUNT numeric)";

static const char CREATE_TABLE_BUDGETYEAR_V1[] =
    "CREATE TABLE BUDGETYEAR_V1(BUDGETYEARID integer primary key, "
    "BUDGETYEARNAME TEXT NOT NULL UNIQUE)";

static const char CREATE_TABLE_CATEGORY_V1[]=
    "CREATE TABLE CATEGORY_V1(CATEGID integer primary key, "
    "CATEGNAME TEXT NOT NULL)";

static const char CREATE_TABLE_CHECKINGACCOUNT_V1[]=
    "CREATE TABLE CHECKINGACCOUNT_V1(TRANSID integer primary key, "
    "ACCOUNTID integer NOT NULL, TOACCOUNTID integer, PAYEEID integer NOT NULL, TRANSCODE TEXT NOT NULL, "
    "TRANSAMOUNT numeric NOT NULL, STATUS TEXT, TRANSACTIONNUMBER TEXT, NOTES TEXT, "
    "CATEGID integer, SUBCATEGID integer, TRANSDATE TEXT, FOLLOWUPID integer, TOTRANSAMOUNT numeric, "
    "CHECK (ACCOUNTID > 0))";

static const char CREATE_TABLE_CURRENCYFORMATS_V1[] =
    "CREATE TABLE CURRENCYFORMATS_V1 (CURRENCYID integer primary key, "
    "CURRENCYNAME TEXT NOT NULL UNIQUE, PFX_SYMBOL TEXT, SFX_SYMBOL TEXT, "
    "DECIMAL_POINT TEXT, GROUP_SEPARATOR TEXT, "
    "UNIT_NAME TEXT, CENT_NAME TEXT, SCALE numeric, BASECONVRATE numeric, CURRENCY_SYMBOL TEXT)";

static const char CREATE_TABLE_INFOTABLE_V1[] =
    "CREATE TABLE INFOTABLE_V1 "
    "( INFOID integer not null primary key, "
    "INFONAME TEXT NOT NULL UNIQUE, "
    "INFOVALUE TEXT NOT NULL )";

static const char CREATE_TABLE_PAYEE_V1[]=
    "CREATE TABLE PAYEE_V1(PAYEEID integer primary key, "
    "PAYEENAME TEXT NOT NULL UNIQUE, CATEGID integer, SUBCATEGID integer)";

static const char CREATE_TABLE_STOCK_V1[] =
    "CREATE TABLE STOCK_V1(STOCKID integer primary key, "
    "HELDAT numeric, PURCHASEDATE TEXT NOT NULL, STOCKNAME TEXT, SYMBOL TEXT, "
    "NUMSHARES numeric, PURCHASEPRICE numeric NOT NULL, NOTES TEXT, CURRENTPRICE numeric NOT NULL, "
    "VALUE numeric, COMMISSION numeric)";

static const char  CREATE_TABLE_SPLITTRANSACTIONS_V1[]=
    "CREATE TABLE SPLITTRANSACTIONS_V1(SPLITTRANSID integer primary key, "
    "TRANSID numeric NOT NULL, CATEGID integer, SUBCATEGID integer, SPLITTRANSAMOUNT numeric)";

static const char CREATE_TABLE_SUBCATEGORY_V1[]=
    "CREATE TABLE SUBCATEGORY_V1(SUBCATEGID integer primary key, "
    "SUBCATEGNAME TEXT NOT NULL, CATEGID integer NOT NULL)";

static const wxString TABLE_NAMES[] =
{
    "ASSETS_V1",
    "ACCOUNTLIST_V1",
    "CHECKINGACCOUNT_V1",
    "PAYEE_V1",
    "BILLSDEPOSITS_V1",
    "STOCK_V1",
    "SPLITTRANSACTIONS_V1",
    "BUDGETSPLITTRANSACTIONS_V1",
    "CATEGORY_V1",
    "SUBCATEGORY_V1",
    "BUDGETYEAR_V1",
    "BUDGETTABLE_V1",
    "CURRENCYFORMATS_V1",
};
const wxArrayString CREATE_TABLE_SQL();

static const char CREATE_VIEW_ALLDATA[] =
    "create  view alldata as "
    "select \n"
    "       CANS.TransID as ID, CANS.TransCode as TransactionType, \n"
    "       date(CANS.TransDate, 'localtime') as Date, \n"
    "       d.userdate as UserDate \n"
    "       ,coalesce(CAT.CategName, SCAT.CategName) as Category, \n"
    "       coalesce(SUBCAT.SUBCategName, SSCAT.SUBCategName, '') as Subcategory, \n"
    "       ROUND((case CANS.TRANSCODE when 'Withdrawal' then -1 else 1 end)*(case CANS.CATEGID when -1 then st.splittransamount else CANS.TRANSAMOUNT end),2) as Amount, \n"
    "       cf.currency_symbol as currency, \n"
    "       CANS.Status as Status, CANS.NOTES as Notes, \n"
    "       cf.BaseConvRate as BaseConvRate, \n"
    "       FROMACC.CurrencyID as CurrencyID, \n"
    "       FROMACC.AccountName as AccountName,  FROMACC.AccountID as AccountID, \n"
    "       ifnull (TOACC.AccountName,'') as ToAccountName, \n"
    "       ifnull (TOACC.ACCOUNTId,-1) as ToAccountID, CANS.ToTransAmount ToTransAmount, \n"
    "       ifnull (TOACC.CURRENCYID, -1) as ToCurrencyID, \n"
    "       (case ifnull(CANS.CATEGID,-1) when -1 then 1 else 0 end) as Splitted, \n"
    "       ifnull(CAT.CategId,st.CategId) as CategID, \n"
    "       ifnull (ifnull(SUBCAT.SubCategID,st.subCategId),-1) as SubCategID, \n"
    "       ifnull (PAYEE.PayeeName,'') as Payee, \n"
    "       ifnull (PAYEE.PayeeID,-1) as PayeeID, \n"
    "            CANS.TRANSACTIONNUMBER as TransactionNumber, \n"
    "            d.year as Year, \n"
    "            d.month as Month, \n"
    "            d.day as Day, \n"
    "            d.finyear as FinYear \n"
    "            from  CHECKINGACCOUNT_V1 CANS \n"
    "            left join CATEGORY_V1 CAT on CAT.CATEGID = CANS.CATEGID \n"
    "            left join SUBCATEGORY_V1 SUBCAT on SUBCAT.SUBCATEGID = CANS.SUBCATEGID and SUBCAT.CATEGID = CANS.CATEGID \n"
    "            left join PAYEE_V1 PAYEE on PAYEE.PAYEEID = CANS.PAYEEID \n"
    "            left join ACCOUNTLIST_V1 FROMACC on FROMACC.ACCOUNTID = CANS.ACCOUNTID \n"
    "            left join ACCOUNTLIST_V1 TOACC on TOACC.ACCOUNTID = CANS.TOACCOUNTID \n"
    "            left join splittransactions_v1 st on CANS.transid=st.transid \n"
    "            left join CATEGORY_V1 SCAT on SCAT.CATEGID = st.CATEGID and CANS.TransId=st.transid \n"
    "            left join SUBCATEGORY_V1 SSCAT on SSCAT.SUBCATEGID = st.SUBCATEGID and SSCAT.CATEGID = st.CATEGID and CANS.TransId=st.transid \n"
    "            left join currencyformats_v1 cf on cf.currencyid=FROMACC .currencyid \n"
    "           left join ( select transid as id , date(transdate, 'localtime') as transdate \n"
    ",round (strftime('%d', transdate, 'localtime'))  as day \n"
    ",round (strftime('%m', transdate, 'localtime'))  as month \n"
    ",round (strftime('%Y', transdate, 'localtime'))  as year \n"
    ",round(strftime('%Y', transdate, 'localtime' ,'start of month', ((case when fd.infovalue<=round(strftime('%d', transdate , 'localtime')) then 1  else 0 end)-fm.infovalue)||' month')) as finyear \n"
    ",ifnull (ifnull (strftime(df.infovalue, TransDate, 'localtime'), \n"
    "       (strftime(replace (df.infovalue, '%y', SubStr (strftime('%Y',TransDate, 'localtime'),3,2)),TransDate, 'localtime')) \n"
    "       ), date(TransDate, 'localtime')) as UserDate \n"
    "from  CHECKINGACCOUNT_V1 \n"
    "left join infotable_v1 df on df.infoname='DATEFORMAT' \n"
    "left join infotable_v1 fm on fm.infoname='FINANCIAL_YEAR_START_MONTH' \n"
    "left join infotable_v1 fd on fd.infoname='FINANCIAL_YEAR_START_DAY') d on d.id=CANS.TRANSID \n"
    "order by CANS.transid";

namespace mmDBWrapper
{

std::shared_ptr<wxSQLite3Database> Open(const wxString &dbpath, const wxString &key = "");

bool ViewExists(wxSQLite3Database* db, const char *viewName);
bool IsSelect(wxSQLite3Database* db, const wxString& sScript, int &rows);

/* Creating new DBs */
void initDB(wxSQLite3Database* db);
int createTable(wxSQLite3Database* db, const wxString &sTableName, const wxString &sql);
int createAllDataView(wxSQLite3Database* db);
bool initCurrencyV1Table(wxSQLite3Database* db);

bool copyBudgetYear(wxSQLite3Database* db, int newYear, int baseYear);
bool deleteBudgetYear(wxSQLite3Database* db, const wxString& yearName);

/* Budgeting BUDGETTABLE_V1 API */
void addBudgetEntry(wxSQLite3Database* db, int budgetYearID, int categID, int subCategID, const wxString& period, double amount);
void deleteBudgetEntry(wxSQLite3Database* db, int budgetEntryID);
void updateBudgetEntry(wxSQLite3Database* db, int budgetYearID, int categID, int subCategID, const wxString& period, double amout);
bool getBudgetEntry(wxSQLite3Database* db, int budgetYearID, int categID, int subCategID, wxString& period, double& amt);

/* Account API */
void removeSplitsForAccount(wxSQLite3Database* db, int accountID);

/* Category Table API */
bool deleteCategoryWithConstraints(wxSQLite3Database* db, int categID);
bool deleteSubCategoryWithConstraints(wxSQLite3Database* db, int categID, int subcategID);
bool updateCategory(wxSQLite3Database* db, int categID, int subcategID, const wxString &newName);
bool addCategory(wxSQLite3Database* db, const wxString &newName);
bool addSubCategory(wxSQLite3Database* db, int categID, const wxString &newName);

/* Transactions API */
bool updateTransactionWithStatus(wxSQLite3Database &db, int transID, const wxString& status);
bool deleteTransaction(wxSQLite3Database* db, int transID);
int relocatePayee(wxSQLite3Database* db, int destPayeeID, int sourcePayeeID);
int relocateCategory(wxSQLite3Database* db, int destCatID, int destSubCatID, int sourceCatID, int sourceSubCatID);

/* Bills & Deposits API */
void deleteBDSeries(wxSQLite3Database* db, int bdID);
void completeBDInSeries(wxSQLite3Database* db, int bdID);

/* Operations on the CURRENCY */
void loadCurrencySettings(wxSQLite3Database* db, int currencyID);
bool deleteCurrency(wxSQLite3Database* db, int currencyID);
int mmSQLiteExecuteUpdate(wxSQLite3Database* db, const std::vector<wxString>& data, const wxString& sql, long &lLastRowId);

wxString getLastDbPath(MMEX_IniSettings* iniSettings, const wxString &defaultVal = "");

/* Stocks API */
void deleteStockInvestment(wxSQLite3Database* db, int stockID);
bool moveStockInvestment(wxSQLite3Database* db, int stockID, int toAccountID);
double getStockInvestmentBalance(wxSQLite3Database* db, int accountID, double& originalVal);

//----------------------------------------------------------------------------

} // namespace mmDBWrapper

//----------------------------------------------------------------------------

#endif // _MM_EX_DBWRAPPER_H_
//----------------------------------------------------------------------------
