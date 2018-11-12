/** @file
 * @brief     Compilation of SQL scripts to upgrade MMEX db version
 * @warning   Auto generated with sqliteupgrade2cpp.py script. DO NOT EDIT!
 * @copyright © 2016-2018 Gabriele-V
 * @date      2018-05-16 01:30:26.593396
 */

#ifndef DB_UPGRADE_H_
#define DB_UPGRADE_H_

#include <vector>
#include <wx/string.h>

const int dbLatestVersion = 13;

const std::vector<wxString> dbUpgradeQuery =
{
	// Upgrade to version 0
	R"(-- First version)",

	// Upgrade to version 1
	R"(-- Upgrade not needed, skip version to keep compatibility)",

	// Upgrade to version 2
	R"(-- Upgrade not needed, skip version to keep compatibility)",

	// Upgrade to version 3
	R"(-- Upgrade embedded in MMEX code before 1.3.0 version, moved here to keep compatibility with old DB
		UPDATE BILLSDEPOSITS_V1 SET TRANSDATE = NEXTOCCURRENCEDATE WHERE (SELECT INFOVALUE FROM INFOTABLE_V1 WHERE INFONAME = 'DATAVERSION')=2;
		UPDATE INFOTABLE_V1 SET INFOVALUE = 3 WHERE INFONAME = 'DATAVERSION';)",

	// Upgrade to version 4
	R"(-- Asset Classes
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
		);)",

	// Upgrade to version 5
	R"(-- CustomField
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
		CREATE INDEX IF NOT EXISTS IDX_CUSTOMFIELDDATA_REF ON CUSTOMFIELDDATA_V1 (FIELDID, REFID);)",

	// Upgrade to version 6
	R"(-- describe TRANSACTIONLINK_V1
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
		CREATE INDEX IF NOT EXISTS IDX_SHAREINFO ON SHAREINFO_V1 (CHECKINGACCOUNTID);)",

	// Upgrade to version 7
	R"(alter table ACCOUNTLIST_V1 add column STATEMENTLOCKED integer;
		alter table ACCOUNTLIST_V1 add column STATEMENTDATE TEXT;
		alter table ACCOUNTLIST_V1 add column MINIMUMBALANCE numeric;
		alter table ACCOUNTLIST_V1 add column CREDITLIMIT numeric;
		alter table ACCOUNTLIST_V1 add column INTERESTRATE numeric;
		alter table ACCOUNTLIST_V1 add column PAYMENTDUEDATE text;
		alter table ACCOUNTLIST_V1 add column MINIMUMPAYMENT numeric;
		)",

	// Upgrade to version 8
	R"(alter table CURRENCYFORMATS_V1 add column CURRENCY_TYPE TEXT;
		update CURRENCYFORMATS_V1 set CURRENCY_TYPE = 'Traditional';
		update CURRENCYFORMATS_V1 set CURRENCY_TYPE = 'Crypto' where CURRENCY_SYMBOL = 'BTC';)",

	// Upgrade to version 9
	R"(-- workaround for missing CURRENCYHISTORY_V1 from db v8 schema
		
		CREATE TABLE IF NOT EXISTS CURRENCYHISTORY_V1(
		CURRHISTID INTEGER PRIMARY KEY
		, CURRENCYID INTEGER NOT NULL
		, CURRDATE TEXT NOT NULL
		, CURRVALUE NUMERIC NOT NULL
		, CURRUPDTYPE INTEGER
		, UNIQUE(CURRENCYID, CURRDATE)
		);
		DROP INDEX IF EXISTS IDX_CURRENCYHISTORY_CURRENCYID_CURRDATE;
		CREATE INDEX IDX_CURRENCYHISTORY_CURRENCYID_CURRDATE ON CURRENCYHISTORY_V1(CURRENCYID, CURRDATE);
		
		-- remove all unused currencies first
		
		DELETE FROM CURRENCYHISTORY_V1
		WHERE CURRENCYID NOT IN (
		  SELECT DISTINCT CURRENCYID FROM ACCOUNTLIST_V1
		  WHERE CURRENCYID IS NOT NULL AND CURRENCYID <> ''
		);
		
		DELETE FROM CURRENCYFORMATS_V1
		WHERE CURRENCYID NOT IN (
		  SELECT DISTINCT CURRENCYID FROM ACCOUNTLIST_V1
		  WHERE CURRENCYID IS NOT NULL AND CURRENCYID <> ''
		);
		
		-- remove unused UNIT_NAME and CENT_NAME
		
		CREATE TABLE CURRENCYFORMATS_V1_NEW(
		CURRENCYID integer primary key
		, CURRENCYNAME TEXT COLLATE NOCASE NOT NULL UNIQUE
		, PFX_SYMBOL TEXT
		, SFX_SYMBOL TEXT
		, DECIMAL_POINT TEXT
		, GROUP_SEPARATOR TEXT
		, SCALE integer
		, BASECONVRATE numeric
		, CURRENCY_SYMBOL TEXT COLLATE NOCASE NOT NULL UNIQUE
		, CURRENCY_TYPE TEXT /* Traditional, Crypto */
		);
		
		INSERT INTO CURRENCYFORMATS_V1_NEW SELECT
		CURRENCYID
		, CURRENCYNAME
		, PFX_SYMBOL
		, SFX_SYMBOL
		, DECIMAL_POINT
		, GROUP_SEPARATOR
		, SCALE
		, BASECONVRATE
		, CURRENCY_SYMBOL
		, CURRENCY_TYPE
		FROM CURRENCYFORMATS_V1;
		
		DROP INDEX IF EXISTS IDX_CURRENCYFORMATS_SYMBOL;
		DROP TABLE CURRENCYFORMATS_V1;
		ALTER TABLE CURRENCYFORMATS_V1_NEW RENAME TO CURRENCYFORMATS_V1;
		CREATE INDEX IDX_CURRENCYFORMATS_SYMBOL ON CURRENCYFORMATS_V1(CURRENCY_SYMBOL);
		
		-- add missing currencies
		
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'US Dollar', '$', '', '.', ' ', '100', '1', 'USD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Euro', '€', '', '.', ' ', '100', '1', 'EUR', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Pound Sterling', '£', '', '.', ' ', '100', '1', 'GBP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Russian Ruble', '', 'р', ',', ' ', '100', '1', 'RUB', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Hryvnia', '₴', '', ',', ' ', '100', '1', 'UAH', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Afghani', '؋', '', '.', ' ', '100', '1', 'AFN', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Lek', '', 'L', '.', ' ', '100', '1', 'ALL', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Algerian Dinar', 'دج', '', '.', ' ', '100', '1', 'DZD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Kwanza', '', 'Kz', '.', ' ', '100', '1', 'AOA', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'East Caribbean Dollar', 'EC$', '', '.', ' ', '100', '1', 'XCD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Argentine Peso', 'AR$', '', ',', '.', '100', '1', 'ARS', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Armenian Dram', '', '', '.', ' ', '100', '1', 'AMD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Aruban Florin', 'ƒ', '', '.', ' ', '100', '1', 'AWG', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Australian Dollar', '$', '', '.', ',', '100', '1', 'AUD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Azerbaijan Manat', '', '', '.', ' ', '100', '1', 'AZN', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Bahamian Dollar', 'B$', '', '.', ' ', '100', '1', 'BSD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Bahraini Dinar', '', '', '.', ' ', '1000', '1', 'BHD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Taka', '', '', '.', ' ', '100', '1', 'BDT', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Barbados Dollar', 'Bds$', '', '.', ' ', '100', '1', 'BBD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Belarusian Ruble (before 2017-01)', 'Br', '', ',', ' ', '1', '1', 'BYR', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Belize Dollar', 'BZ$', '', '.', ' ', '100', '1', 'BZD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'CFA Franc BCEAO', 'CFA', '', '.', ' ', '1', '1', 'XOF', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Bermudian Dollar', 'BD$', '', '.', ' ', '100', '1', 'BMD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Ngultrum', 'Nu.', '', '.', ' ', '100', '1', 'BTN', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Boliviano', 'Bs.', '', '.', ' ', '100', '1', 'BOB', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Convertible Mark', 'KM', '', ',', '.', '100', '1', 'BAM', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Pula', 'P', '', '.', ' ', '100', '1', 'BWP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Brazilian Real', 'R$', '', '.', ' ', '100', '1', 'BRL', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Brunei Dollar', 'B$', '', '.', ' ', '100', '1', 'BND', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Bulgarian Lev', '', '', '.', ' ', '100', '1', 'BGN', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Burundi Franc', 'FBu', '', '.', ' ', '1', '1', 'BIF', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Riel', '', '', '.', ' ', '100', '1', 'KHR', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'CFA Franc BEAC', 'CFA', '', '.', ' ', '1', '1', 'XAF', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Canadian Dollar', '$', '', '.', ' ', '100', '1', 'CAD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Cabo Verde Escudo', 'Esc', '', '.', ' ', '100', '1', 'CVE', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Cayman Islands Dollar', 'KY$', '', '.', ' ', '100', '1', 'KYD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Chilean Peso', '$', '', '.', ' ', '1', '1', 'CLP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Yuan Renminbi', '¥', '', '.', ' ', '100', '1', 'CNY', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Colombian Peso', 'Col$', '', '.', ' ', '100', '1', 'COP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Comorian Franc ', '', '', '.', ' ', '1', '1', 'KMF', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Congolese Franc', 'F', '', '.', ' ', '100', '1', 'CDF', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Costa Rican Colon', '₡', '', '.', ' ', '100', '1', 'CRC', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Kuna', 'kn', '', '.', ' ', '100', '1', 'HRK', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Czech Koruna', 'Kč', '', '.', ' ', '100', '1', 'CZK', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Danish Krone', 'Kr', '', '.', ' ', '100', '1', 'DKK', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Djibouti Franc', 'Fdj', '', '.', ' ', '1', '1', 'DJF', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Dominican Peso', 'RD$', '', '.', ' ', '100', '1', 'DOP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Egyptian Pound', '£', '', '.', ' ', '100', '1', 'EGP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Nakfa', 'Nfa', '', '.', ' ', '100', '1', 'ERN', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Ethiopian Birr', 'Br', '', '.', ' ', '100', '1', 'ETB', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Falkland Islands Pound', '£', '', '.', ' ', '100', '1', 'FKP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Fiji Dollar', 'FJ$', '', '.', ' ', '100', '1', 'FJD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'CFP Franc', 'F', '', '.', ' ', '1', '1', 'XPF', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Dalasi', 'D', '', '.', ' ', '100', '1', 'GMD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Lari', '', '', '.', ' ', '100', '1', 'GEL', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Ghana Cedi', '', '', '.', ' ', '100', '1', 'GHS', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Gibraltar Pound', '£', '', '.', ' ', '100', '1', 'GIP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Quetzal', 'Q', '', '.', ' ', '100', '1', 'GTQ', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Guinean Franc', 'FG', '', '.', ' ', '1', '1', 'GNF', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Guyana Dollar', 'GY$', '', '.', ' ', '100', '1', 'GYD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Gourde', 'G', '', '.', ' ', '100', '1', 'HTG', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Lempira', 'L', '', '.', ' ', '100', '1', 'HNL', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Hong Kong Dollar', 'HK$', '', '.', ' ', '100', '1', 'HKD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Forint', 'Ft', '', '.', ' ', '100', '1', 'HUF', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Iceland Krona', 'kr', '', '.', ' ', '1', '1', 'ISK', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Indian Rupee', '₹', '', '.', ' ', '100', '1', 'INR', 'Traditional');
		INSERT OR IGNORE INTO CU)"
	R"(RRENCYFORMATS_V1 VALUES (NULL, 'Rupiah', 'Rp', '', '.', ' ', '100', '1', 'IDR', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Special Drawing Rights', 'SDR', '', '.', ' ', '100', '1', 'XDR', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Iranian Rial', '', '', '.', ' ', '100', '1', 'IRR', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Iraqi Dinar', '', '', '.', ' ', '1000', '1', 'IQD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'New Israeli Sheqel', '₪', '', '.', ' ', '100', '1', 'ILS', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Jamaican Dollar', 'J$', '', '.', ' ', '100', '1', 'JMD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Yen', '¥', '', '.', ' ', '1', '1', 'JPY', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Jordanian Dinar', '', '', '.', ' ', '1000', '1', 'JOD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Tenge', 'T', '', '.', ' ', '100', '1', 'KZT', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Kenyan Shilling', 'KSh', '', '.', ' ', '100', '1', 'KES', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'North Korean Won', 'W', '', '.', ' ', '100', '1', 'KPW', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Won', 'W', '', '.', ' ', '1', '1', 'KRW', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Kuwaiti Dinar', '', '', '.', ' ', '1000', '1', 'KWD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Som', '', '', '.', ' ', '100', '1', 'KGS', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Lao Kip', 'KN', '', '.', ' ', '100', '1', 'LAK', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Latvian Lats (before 2014-01)', 'Ls', '', '.', ' ', '100', '1', 'LVL', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Lebanese Pound', '', '', '.', ' ', '100', '1', 'LBP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Loti', 'M', '', '.', ' ', '100', '1', 'LSL', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Liberian Dollar', 'L$', '', '.', ' ', '100', '1', 'LRD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Libyan Dinar', 'LD', '', '.', ' ', '1000', '1', 'LYD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Lithuanian Litas (before 2014-12)', 'Lt', '', '.', ' ', '100', '1', 'LTL', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Pataca', 'P', '', '.', ' ', '100', '1', 'MOP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Denar', '', '', '.', ' ', '100', '1', 'MKD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Malagasy Ariary', 'FMG', '', '.', ' ', '100', '1', 'MGA', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Malawi Kwacha', 'MK', '', '.', ' ', '100', '1', 'MWK', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Malaysian Ringgit', 'RM', '', '.', ' ', '100', '1', 'MYR', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Rufiyaa', 'Rf', '', '.', ' ', '100', '1', 'MVR', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Ouguiya (before 2017-12)', 'UM', '', '.', ' ', '100', '1', 'MRO', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Mauritius Rupee', 'Rs', '', '.', ' ', '100', '1', 'MUR', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Mexican Peso', '$', '', '.', ' ', '100', '1', 'MXN', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Moldovan Leu', '', '', '.', ' ', '100', '1', 'MDL', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Tugrik', '₮', '', '.', ' ', '100', '1', 'MNT', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Moroccan Dirham', '', '', '.', ' ', '100', '1', 'MAD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Kyat', 'K', '', '.', ' ', '100', '1', 'MMK', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Namibia Dollar', 'N$', '', '.', ' ', '100', '1', 'NAD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Nepalese Rupee', 'NRs', '', '.', ' ', '100', '1', 'NPR', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Netherlands Antillean Guilder', 'NAƒ', '', '.', ' ', '100', '1', 'ANG', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'New Zealand Dollar', 'NZ$', '', '.', ' ', '100', '1', 'NZD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Cordoba Oro', 'C$', '', '.', ' ', '100', '1', 'NIO', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Naira', '₦', '', '.', ' ', '100', '1', 'NGN', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Norwegian Krone', 'kr', '', '.', ' ', '100', '1', 'NOK', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Rial Omani', '', '', '.', ' ', '1000', '1', 'OMR', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Pakistan Rupee', 'Rs.', '', '.', ' ', '100', '1', 'PKR', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Balboa', 'B./', '', '.', ' ', '100', '1', 'PAB', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Kina', 'K', '', '.', ' ', '100', '1', 'PGK', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Guarani', '', '', '.', ' ', '1', '1', 'PYG', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Sol', 'S/.', '', '.', ' ', '100', '1', 'PEN', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Philippine Piso', '₱', '', '.', ' ', '100', '1', 'PHP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Zloty', '', 'zł', ',', '.', '100', '1', 'PLN', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Qatari Rial', 'QR', '', '.', ' ', '100', '1', 'QAR', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Romanian Leu', 'L', '', '.', ' ', '100', '1', 'RON', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Rwanda Franc', 'RF', '', '.', ' ', '1', '1', 'RWF', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Dobra (before 2017-12)', 'Db', '', '.', ' ', '0', '1', 'STD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Saudi Riyal', 'SR', '', '.', ' ', '100', '1', 'SAR', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Serbian Dinar', 'din.', '', '.', ' ', '100', '1', 'RSD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Seychelles Rupee', 'SR', '', '.', ' ', '100', '1', 'SCR', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Leone', 'Le', '', '.', ' ', '100', '1', 'SLL', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Singapore Dollar', 'S$', '', '.', ' ', '100', '1', 'SGD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Solomon Islands Dollar', 'SI$', '', '.', ' ', '100', '1', 'SBD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Somali Shilling', 'Sh.', '', '.', ' ', '100', '1', 'SOS', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Rand', 'R', '', '.', ' ', '100', '1', 'ZAR', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Sri Lanka Rupee', 'Rs', '', '.', ' ', '100', '1', 'LKR', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Saint Helena Pound', '£', '', '.', ' ', '100', '1', 'SHP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Sudanese Pound', '', '', '.', ' ', '100', '1', 'SDG', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Surinam Dollar', '$', '', '.', ' ', '100', '1', 'SRD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Lilangeni', 'E', '', '.', ' ', '100', '1', 'SZL', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Swedish Krona', 'kr', '', '.', ' ', '100', '1', 'SEK', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Swiss Franc', 'Fr.', '', '.', ' ', '100', '1', 'CHF', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Syrian Pound', '', '', '.', ' ', '100', '1', 'SYP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'New Taiwan Dollar', 'NT$', '', '.', ' ', '100', '1', 'TWD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Somoni', '', '', '.', ' ', '100', '1', 'TJS', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Tanzanian Shilling', '', '', '.', ' ', '100', '1', 'TZS', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Baht', '฿', '', '.', ' ', '100', '1', 'THB', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Trinidad and Tobago Dollar', 'TT$', '', '.', ' ', '100', '1', 'TTD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Tunisian Dinar', 'DT', '', '.', ' ', '1000', '1', 'TND', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Turkish Lira', '₺', '', '.', ' ', '100', '1', 'TRY', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Turkmenistan New Manat', 'm', '', '.', ' ', '100', '1', 'TMT', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Uganda Shilling', 'USh', '', '.', ' ', '1', '1', 'UGX', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'UAE Dirham', '', '', '.', ' ', '100', '1', 'AED', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Peso Uruguayo', '$U', '', '.', ' ', '100', '1', 'UYU', 'Traditional'))"
	R"(;
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Uzbekistan Sum', '', '', '.', ' ', '100', '1', 'UZS', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Vatu', 'VT', '', '.', ' ', '1', '1', 'VUV', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Dong', '₫', '', '.', ' ', '1', '1', 'VND', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Tala', 'WS$', '', '.', ' ', '100', '1', 'WST', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Yemeni Rial', '', '', '.', ' ', '100', '1', 'YER', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Bolívar', 'Bs.', '', '.', ',', '100', '1', 'VEF', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Bitcoin', 'Ƀ', '', '.', ',', '100000000', '1', 'BTC', 'Crypto');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Andorran Peseta (before 2003-07)', '', '', '.', ',', '100', '1', 'ADP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Afghani (before 2003-01)', '', '', '.', ',', '100', '1', 'AFA', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Old Lek (before 1989-12)', '', '', '.', ',', '100', '1', 'ALK', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Kwanza (before 1991-03)', '', '', '.', ',', '100', '1', 'AOK', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'New Kwanza (before 2000-02)', '', '', '.', ',', '100', '1', 'AON', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Kwanza Reajustado (before 2000-02)', '', '', '.', ',', '100', '1', 'AOR', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Austral (before 1992-01)', '', '', '.', ',', '100', '1', 'ARA', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Peso Argentino (before 1985-07)', '', '', '.', ',', '100', '1', 'ARP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Peso (1989 to 1990)', '', '', '.', ',', '100', '1', 'ARY', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Schilling (before 2002-03)', '', '', '.', ',', '100', '1', 'ATS', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Azerbaijan Manat (before 2005-10)', '', '', '.', ',', '100', '1', 'AYM', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Azerbaijanian Manat (before 2005-12)', '', '', '.', ',', '100', '1', 'AZM', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Dinar (before 1998-07)', '', '', '.', ',', '100', '1', 'BAD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Convertible Franc (before 1990-03)', '', '', '.', ',', '100', '1', 'BEC', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Belgian Franc (before 2002-03)', '', '', '.', ',', '100', '1', 'BEF', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Financial Franc (before 1990-03)', '', '', '.', ',', '100', '1', 'BEL', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Lev A/52 (1989 to 1990)', '', '', '.', ',', '100', '1', 'BGJ', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Lev A/62 (1989 to 1990)', '', '', '.', ',', '100', '1', 'BGK', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Lev (before 2003-11)', '', '', '.', ',', '100', '1', 'BGL', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Peso boliviano (before 1987-02)', '', '', '.', ',', '100', '1', 'BOP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Cruzeiro (before 1986-03)', '', '', '.', ',', '100', '1', 'BRB', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Cruzado (before 1989-02)', '', '', '.', ',', '100', '1', 'BRC', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Cruzeiro (before 1993-03)', '', '', '.', ',', '100', '1', 'BRE', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'New Cruzado (before 1990-03)', '', '', '.', ',', '100', '1', 'BRN', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Cruzeiro Real (before 1994-07)', '', '', '.', ',', '100', '1', 'BRR', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Kyat (before 1990-02)', '', '', '.', ',', '100', '1', 'BUK', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Belarusian Ruble (before 2001-01)', '', '', '.', ',', '100', '1', 'BYB', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Belarusian Ruble', '', '', '.', ',', '100', '1', 'BYN', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'WIR Franc (for electronic) (before 2004-11)', '', '', '.', ',', '100', '1', 'CHC', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Serbian Dinar (before 2006-10)', '', '', '.', ',', '100', '1', 'CSD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Krona A/53 (1989 to 1990)', '', '', '.', ',', '100', '1', 'CSJ', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Koruna (before 1993-03)', '', '', '.', ',', '100', '1', 'CSK', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Peso Convertible', '', '', '.', ',', '100', '1', 'CUC', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Cuban Peso', '', '', '.', ',', '100', '1', 'CUP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Cyprus Pound (before 2008-01)', '', '', '.', ',', '100', '1', 'CYP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Mark der DDR (1990-07 to 1990-09)', '', '', '.', ',', '100', '1', 'DDM', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Deutsche Mark (before 2002-03)', '', '', '.', ',', '100', '1', 'DEM', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Sucre (before 2000-09)', '', '', '.', ',', '100', '1', 'ECS', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Unidad de Valor Constante (UVC) (before 2000-09)', '', '', '.', ',', '100', '1', 'ECV', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Kroon (before 2011-01)', '', '', '.', ',', '100', '1', 'EEK', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Spanish Peseta (1978 to 1981)', '', '', '.', ',', '100', '1', 'ESA', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'A Account (convertible Peseta Account) (before 1994-12)', '', '', '.', ',', '100', '1', 'ESB', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Spanish Peseta (before 2002-03)', '', '', '.', ',', '100', '1', 'ESP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Markka (before 2002-03)', '', '', '.', ',', '100', '1', 'FIM', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'French Franc (before 1999-01)', '', '', '.', ',', '100', '1', 'FRF', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Georgian Coupon (before 1995-10)', '', '', '.', ',', '100', '1', 'GEK', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Cedi (before 2008-01)', '', '', '.', ',', '100', '1', 'GHC', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Ghana Cedi (before 2007-06)', '', '', '.', ',', '100', '1', 'GHP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Syli (before 1989-12)', '', '', '.', ',', '100', '1', 'GNE', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Syli (before 1986-02)', '', '', '.', ',', '100', '1', 'GNS', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Ekwele (before 1986-06)', '', '', '.', ',', '100', '1', 'GQE', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Drachma (before 2002-03)', '', '', '.', ',', '100', '1', 'GRD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Guinea Escudo (1978 to 1981)', '', '', '.', ',', '100', '1', 'GWE', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Guinea-Bissau Peso (before 1997-05)', '', '', '.', ',', '100', '1', 'GWP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Croatian Dinar (before 1995-01)', '', '', '.', ',', '100', '1', 'HRD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Irish Pound (before 2002-03)', '', '', '.', ',', '100', '1', 'IEP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Pound (1978 to 1981)', '', '', '.', ',', '100', '1', 'ILP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Old Shekel (1989 to 1990)', '', '', '.', ',', '100', '1', 'ILR', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Old Krona (1989 to 1990)', '', '', '.', ',', '100', '1', 'ISJ', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Italian Lira (before 2002-03)', '', '', '.', ',', '100', '1', 'ITL', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Pathet Lao Kip (before 1979-12)', '', '', '.', ',', '100', '1', 'LAJ', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Loti (before 1985-05)', '', '', '.', ',', '100', '1', 'LSM', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Talonas (before 1993-07)', '', '', '.', ',', '100', '1', 'LTT', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Luxembourg Convertible Franc (before 1990-03)', '', '', '.', ',', '100', '1', 'LUC', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Luxembourg Franc (before 2002-03)', '', '', '.', ',', '100', '1', 'LUF', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Luxembourg Financial Franc (before 1990-03)', '', '', '.', ',', '100', '1', 'LUL', 'Traditional');
		INSERT OR IGNORE I)"
	R"(NTO CURRENCYFORMATS_V1 VALUES (NULL, 'Latvian Ruble (before 1994-12)', '', '', '.', ',', '100', '1', 'LVR', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Malagasy Franc (before 2004-12)', '', '', '.', ',', '100', '1', 'MGF', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Mali Franc (before 1984-11)', '', '', '.', ',', '100', '1', 'MLF', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Ouguiya', '', '', '.', ',', '100', '1', 'MRU', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Maltese Lira (before 2008-01)', '', '', '.', ',', '100', '1', 'MTL', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Maltese Pound (before 1983-06)', '', '', '.', ',', '100', '1', 'MTP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Maldive Rupee (before 1989-12)', '', '', '.', ',', '100', '1', 'MVQ', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Mexican Peso (before 1993-01)', '', '', '.', ',', '100', '1', 'MXP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Mozambique Escudo (1978 to 1981)', '', '', '.', ',', '100', '1', 'MZE', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Mozambique Metical (before 2006-06)', '', '', '.', ',', '100', '1', 'MZM', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Mozambique Metical', '', '', '.', ',', '100', '1', 'MZN', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Cordoba (before 1990-10)', '', '', '.', ',', '100', '1', 'NIC', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Netherlands Guilder (before 2002-03)', '', '', '.', ',', '100', '1', 'NLG', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Sol (1989 to 1990)', '', '', '.', ',', '100', '1', 'PEH', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Inti (before 1991-07)', '', '', '.', ',', '100', '1', 'PEI', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Sol (before 1986-02)', '', '', '.', ',', '100', '1', 'PES', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Zloty (before 1997-01)', '', '', '.', ',', '100', '1', 'PLZ', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Portuguese Escudo (before 2002-03)', '', '', '.', ',', '100', '1', 'PTE', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Rhodesian Dollar (1978 to 1981)', '', '', '.', ',', '100', '1', 'RHD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Leu A/52 (1989 to 1990)', '', '', '.', ',', '100', '1', 'ROK', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Old Leu (before 2005-06)', '', '', '.', ',', '100', '1', 'ROL', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Russian Ruble (before 1994-07)', '', '', '.', ',', '100', '1', 'RUR', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Sudanese Dinar (before 2007-07)', '', '', '.', ',', '100', '1', 'SDD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Sudanese Pound (before 1998-06)', '', '', '.', ',', '100', '1', 'SDP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Tolar (before 2007-01)', '', '', '.', ',', '100', '1', 'SIT', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Slovak Koruna (before 2009-01)', '', '', '.', ',', '100', '1', 'SKK', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Surinam Guilder (before 2003-12)', '', '', '.', ',', '100', '1', 'SRG', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'South Sudanese Pound', '', '', '.', ',', '100', '1', 'SSP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Dobra', '', '', '.', ',', '100', '1', 'STN', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Rouble (before 1990-12)', '', '', '.', ',', '100', '1', 'SUR', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'El Salvador Colon', '', '', '.', ',', '100', '1', 'SVC', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Tajik Ruble (before 2001-04)', '', '', '.', ',', '100', '1', 'TJR', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Turkmenistan Manat (before 2009-01)', '', '', '.', ',', '100', '1', 'TMM', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Pa’anga', '', '', '.', ',', '100', '1', 'TOP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Timor Escudo (before 2002-11)', '', '', '.', ',', '100', '1', 'TPE', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Old Turkish Lira (before 2005-12)', '', '', '.', ',', '100', '1', 'TRL', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Karbovanet (before 1996-09)', '', '', '.', ',', '100', '1', 'UAK', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Uganda Shilling (before 1987-05)', '', '', '.', ',', '100', '1', 'UGS', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Old Shilling (1989 to 1990)', '', '', '.', ',', '100', '1', 'UGW', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'US Dollar (Same day) (before 2014-03)', '', '', '.', ',', '100', '1', 'USS', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Old Uruguay Peso (before 1989-12)', '', '', '.', ',', '100', '1', 'UYN', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Uruguayan Peso (before 1993-03)', '', '', '.', ',', '100', '1', 'UYP', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Bolivar (before 2008-01)', '', '', '.', ',', '100', '1', 'VEB', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Old Dong (1989-1990)', '', '', '.', ',', '100', '1', 'VNC', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'European Currency Unit (E.C.U) (before 1999-01)', '', '', '.', ',', '100', '1', 'XEU', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Gold-Franc (before 2006-10)', '', '', '.', ',', '100', '1', 'XFO', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Yemeni Dinar (before 1991-09)', '', '', '.', ',', '100', '1', 'YDD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'New Yugoslavian Dinar (before 1990-01)', '', '', '.', ',', '100', '1', 'YUD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'New Dinar (before 2003-07)', '', '', '.', ',', '100', '1', 'YUM', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Yugoslavian Dinar (before 1995-11)', '', '', '.', ',', '100', '1', 'YUN', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Financial Rand (before 1995-03)', '', '', '.', ',', '100', '1', 'ZAL', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Zambian Kwacha (before 2012-12)', '', '', '.', ',', '100', '1', 'ZMK', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Zambian Kwacha', '', '', '.', ',', '100', '1', 'ZMW', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'New Zaire (before 1999-06)', '', '', '.', ',', '100', '1', 'ZRN', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Zaire (before 1994-02)', '', '', '.', ',', '100', '1', 'ZRZ', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Rhodesian Dollar (before 1989-12)', '', '', '.', ',', '100', '1', 'ZWC', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Zimbabwe Dollar (before 2008-08)', '', '', '.', ',', '100', '1', 'ZWD', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Zimbabwe Dollar', '', '', '.', ',', '100', '1', 'ZWL', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Zimbabwe Dollar (new) (before 2006-09)', '', '', '.', ',', '100', '1', 'ZWN', 'Traditional');
		INSERT OR IGNORE INTO CURRENCYFORMATS_V1 VALUES (NULL, 'Zimbabwe Dollar (before 2009-06)', '', '', '.', ',', '100', '1', 'ZWR', 'Traditional');
		
		PRAGMA user_version = 9;)",

	// Upgrade to version 10
	R"(CREATE TABLE CURRENCYFORMATS_V1_NEW(
		CURRENCYID integer primary key
		, CURRENCYNAME TEXT COLLATE NOCASE NOT NULL
		, PFX_SYMBOL TEXT
		, SFX_SYMBOL TEXT
		, DECIMAL_POINT TEXT
		, GROUP_SEPARATOR TEXT
		, SCALE integer
		, BASECONVRATE numeric
		, CURRENCY_SYMBOL TEXT COLLATE NOCASE NOT NULL UNIQUE
		, CURRENCY_TYPE TEXT /* Traditional, Crypto */
		, HISTORIC integer DEFAULT 0 /* 1 if no longer official */
		);
		
		INSERT INTO CURRENCYFORMATS_V1_NEW SELECT
		CURRENCYID
		, CURRENCYNAME
		, PFX_SYMBOL
		, SFX_SYMBOL
		, DECIMAL_POINT
		, GROUP_SEPARATOR
		, SCALE
		, BASECONVRATE
		, CURRENCY_SYMBOL
		, CURRENCY_TYPE
		, 0
		FROM CURRENCYFORMATS_V1;
		
		UPDATE CURRENCYFORMATS_V1_NEW SET HISTORIC = 1
		WHERE CURRENCY_SYMBOL IN (
		  'BYR', 'LVL', 'LTL', 'MRO', 'STD', 'ADP', 'AFA', 'ALK', 'AOK', 'AON', 'AOR',
		  'ARA', 'ARP', 'ARY', 'ATS', 'AYM', 'AZM', 'BAD', 'BEC', 'BEF', 'BEL', 'BGJ',
		  'BGK', 'BGL', 'BOP', 'BRB', 'BRC', 'BRE', 'BRN', 'BRR', 'BUK', 'BYB', 'CHC',
		  'CSD', 'CSJ', 'CSK', 'CYP', 'DDM', 'DEM', 'ECS', 'ECV', 'EEK', 'ESA', 'ESB',
		  'ESP', 'FIM', 'FRF', 'GEK', 'GHC', 'GHP', 'GNE', 'GNS', 'GQE', 'GRD', 'GWE',
		  'GWP', 'HRD', 'IEP', 'ILP', 'ILR', 'ISJ', 'ITL', 'LAJ', 'LSM', 'LTT', 'LUC',
		  'LUF', 'LUL', 'LVR', 'MGF', 'MLF', 'MTL', 'MTP', 'MVQ', 'MXP', 'MZE', 'MZM',
		  'NIC', 'NLG', 'PEH', 'PEI', 'PES', 'PLZ', 'PTE', 'RHD', 'ROK', 'ROL', 'RUR',
		  'SDD', 'SDP', 'SIT', 'SKK', 'SRG', 'SUR', 'TJR', 'TMM', 'TPE', 'TRL', 'UAK',
		  'UGS', 'UGW', 'USS', 'UYN', 'UYP', 'VEB', 'VNC', 'XEU', 'XFO', 'YDD', 'YUD',
		  'YUM', 'YUN', 'ZAL', 'ZMK', 'ZRN', 'ZRZ', 'ZWC', 'ZWD', 'ZWN', 'ZWR'
		);
		
		DROP INDEX IDX_CURRENCYFORMATS_SYMBOL;
		DROP TABLE CURRENCYFORMATS_V1;
		ALTER TABLE CURRENCYFORMATS_V1_NEW RENAME TO CURRENCYFORMATS_V1;
		CREATE INDEX IDX_CURRENCYFORMATS_SYMBOL ON CURRENCYFORMATS_V1(CURRENCY_SYMBOL);
		PRAGMA user_version = 10;)",

	// Upgrade to version 11
	R"(-- workaround for missing ttable
		CREATE TABLE IF NOT EXISTS ATTACHMENT_V1 (
		ATTACHMENTID INTEGER NOT NULL PRIMARY KEY
		, REFTYPE TEXT NOT NULL /* Transaction, Stock, Asset, BankAccount, RepeatingTransaction, Payee */
		, REFID INTEGER NOT NULL
		, DESCRIPTION TEXT COLLATE NOCASE
		, FILENAME TEXT NOT NULL COLLATE NOCASE
		);
		CREATE TABLE IF NOT EXISTS CUSTOMFIELD_V1 (
		FIELDID INTEGER NOT NULL PRIMARY KEY
		, REFTYPE TEXT NOT NULL /* Transaction, Stock, Asset, BankAccount, RepeatingTransaction, Payee */
		, DESCRIPTION TEXT COLLATE NOCASE
		, TYPE TEXT NOT NULL /* String, Integer, Decimal, Boolean, Date, Time, SingleChoice, MultiChoice */
		, PROPERTIES TEXT NOT NULL
		);
		
		UPDATE CURRENCYFORMATS_V1 SET CURRENCY_TYPE = 'Fiat' WHERE CURRENCY_TYPE <> 'Crypto';
		UPDATE ATTACHMENT_V1 SET REFTYPE = 'Bank Account' WHERE REFTYPE = 'BankAccount';
		UPDATE ATTACHMENT_V1 SET REFTYPE = 'Recurring Transaction' WHERE REFTYPE = 'RecurringTransaction';
		UPDATE CUSTOMFIELD_V1 SET REFTYPE = 'Bank Account' WHERE REFTYPE = 'BankAccount';
		UPDATE CUSTOMFIELD_V1 SET REFTYPE = 'Recurring Transaction' WHERE REFTYPE = 'RecurringTransaction';
		PRAGMA user_version = 11;)",

	// Upgrade to version 12
	R"(-- workaround for missing tables from v11 schema
		
		CREATE TABLE IF NOT EXISTS ACCOUNTLIST_V1(
		ACCOUNTID integer primary key
		, ACCOUNTNAME TEXT COLLATE NOCASE NOT NULL UNIQUE
		, ACCOUNTTYPE TEXT NOT NULL /* Checking, Term, Investment, Credit Card */
		, ACCOUNTNUM TEXT
		, STATUS TEXT NOT NULL /* Open, Closed */
		, NOTES TEXT
		, HELDAT TEXT
		, WEBSITE TEXT
		, CONTACTINFO TEXT
		, ACCESSINFO TEXT
		, INITIALBAL numeric
		, FAVORITEACCT TEXT NOT NULL
		, CURRENCYID integer NOT NULL
		, STATEMENTLOCKED integer
		, STATEMENTDATE TEXT
		, MINIMUMBALANCE numeric
		, CREDITLIMIT numeric
		, INTERESTRATE numeric
		, PAYMENTDUEDATE text
		, MINIMUMPAYMENT numeric
		);
		
		CREATE TABLE IF NOT EXISTS ASSETS_V1(
		ASSETID integer primary key
		, STARTDATE TEXT NOT NULL
		, ASSETNAME TEXT COLLATE NOCASE NOT NULL
		, VALUE numeric
		, VALUECHANGE TEXT /* None, Appreciates, Depreciates */
		, NOTES TEXT
		, VALUECHANGERATE numeric
		, ASSETTYPE TEXT /* Property, Automobile, Household Object, Art, Jewellery, Cash, Other */
		);
		
		CREATE TABLE IF NOT EXISTS BILLSDEPOSITS_V1(
		BDID integer primary key
		, ACCOUNTID integer NOT NULL
		, TOACCOUNTID integer
		, PAYEEID integer NOT NULL
		, TRANSCODE TEXT NOT NULL /* Withdrawal, Deposit, Transfer */
		, TRANSAMOUNT numeric NOT NULL
		, STATUS TEXT /* None, Reconciled, Void, Follow up, Duplicate */
		, TRANSACTIONNUMBER TEXT
		, NOTES TEXT
		, CATEGID integer
		, SUBCATEGID integer
		, TRANSDATE TEXT
		, FOLLOWUPID integer
		, TOTRANSAMOUNT numeric
		, REPEATS integer
		, NEXTOCCURRENCEDATE TEXT
		, NUMOCCURRENCES integer
		);
		
		CREATE TABLE IF NOT EXISTS BUDGETSPLITTRANSACTIONS_V1(
		SPLITTRANSID integer primary key
		, TRANSID integer NOT NULL
		, CATEGID integer
		, SUBCATEGID integer
		, SPLITTRANSAMOUNT numeric
		);
		
		CREATE TABLE IF NOT EXISTS BUDGETTABLE_V1(
		BUDGETENTRYID integer primary key
		, BUDGETYEARID integer
		, CATEGID integer
		, SUBCATEGID integer
		, PERIOD TEXT NOT NULL /* None, Weekly, Bi-Weekly, Monthly, Monthly, Bi-Monthly, Quarterly, Half-Yearly, Yearly, Daily*/
		, AMOUNT numeric NOT NULL
		);
		
		CREATE TABLE IF NOT EXISTS BUDGETYEAR_V1(
		BUDGETYEARID integer primary key
		, BUDGETYEARNAME TEXT NOT NULL UNIQUE
		);
		
		CREATE TABLE IF NOT EXISTS CATEGORY_V1(
		CATEGID integer primary key
		, CATEGNAME TEXT COLLATE NOCASE NOT NULL UNIQUE
		);
		
		CREATE TABLE IF NOT EXISTS CHECKINGACCOUNT_V1(
		TRANSID integer primary key
		, ACCOUNTID integer NOT NULL
		, TOACCOUNTID integer
		, PAYEEID integer NOT NULL
		, TRANSCODE TEXT NOT NULL /* Withdrawal, Deposit, Transfer */
		, TRANSAMOUNT numeric NOT NULL
		, STATUS TEXT /* None, Reconciled, Void, Follow up, Duplicate */
		, TRANSACTIONNUMBER TEXT
		, NOTES TEXT
		, CATEGID integer
		, SUBCATEGID integer
		, TRANSDATE TEXT
		, FOLLOWUPID integer
		, TOTRANSAMOUNT numeric
		);
		
		CREATE TABLE IF NOT EXISTS CURRENCYHISTORY_V1(
		CURRHISTID INTEGER PRIMARY KEY
		, CURRENCYID INTEGER NOT NULL
		, CURRDATE TEXT NOT NULL
		, CURRVALUE NUMERIC NOT NULL
		, CURRUPDTYPE INTEGER
		, UNIQUE(CURRENCYID, CURRDATE)
		);
		
		CREATE TABLE IF NOT EXISTS CURRENCYFORMATS_V1(
		CURRENCYID integer primary key
		, CURRENCYNAME TEXT COLLATE NOCASE NOT NULL
		, PFX_SYMBOL TEXT
		, SFX_SYMBOL TEXT
		, DECIMAL_POINT TEXT
		, GROUP_SEPARATOR TEXT
		, SCALE integer
		, BASECONVRATE numeric
		, CURRENCY_SYMBOL TEXT COLLATE NOCASE NOT NULL UNIQUE
		, CURRENCY_TYPE TEXT /* Traditional, Crypto */
		, HISTORIC integer DEFAULT 0 /* 1 if no longer official */
		);
		
		CREATE TABLE IF NOT EXISTS INFOTABLE_V1(
		INFOID integer not null primary key
		, INFONAME TEXT COLLATE NOCASE NOT NULL UNIQUE
		, INFOVALUE TEXT NOT NULL
		);
		
		CREATE TABLE IF NOT EXISTS PAYEE_V1(
		PAYEEID integer primary key
		, PAYEENAME TEXT COLLATE NOCASE NOT NULL UNIQUE
		, CATEGID integer
		, SUBCATEGID integer
		);
		
		CREATE TABLE IF NOT EXISTS SPLITTRANSACTIONS_V1(
		SPLITTRANSID integer primary key
		, TRANSID integer NOT NULL
		, CATEGID integer
		, SUBCATEGID integer
		, SPLITTRANSAMOUNT numeric
		);
		
		CREATE TABLE IF NOT EXISTS STOCK_V1(
		STOCKID integer primary key
		, HELDAT integer
		, PURCHASEDATE TEXT NOT NULL
		, STOCKNAME TEXT COLLATE NOCASE NOT NULL
		, SYMBOL TEXT
		, NUMSHARES numeric
		, PURCHASEPRICE numeric NOT NULL
		, NOTES TEXT
		, CURRENTPRICE numeric NOT NULL
		, VALUE numeric
		, COMMISSION numeric
		);
		
		CREATE TABLE IF NOT EXISTS STOCKHISTORY_V1(
		HISTID integer primary key
		, SYMBOL TEXT NOT NULL
		, DATE TEXT NOT NULL
		, VALUE numeric NOT NULL
		, UPDTYPE integer
		, UNIQUE(SYMBOL, DATE)
		);
		
		CREATE TABLE IF NOT EXISTS SUBCATEGORY_V1(
		SUBCATEGID integer primary key
		, SUBCATEGNAME TEXT COLLATE NOCASE NOT NULL
		, CATEGID integer NOT NULL
		, UNIQUE(CATEGID, SUBCATEGNAME)
		);
		
		create table IF NOT EXISTS REPORT_V1(
		REPORTID integer not null primary key
		, REPORTNAME TEXT COLLATE NOCASE NOT NULL UNIQUE
		, GROUPNAME TEXT COLLATE NOCASE
		, SQLCONTENT TEXT
		, LUACONTENT TEXT
		, TEMPLATECONTENT TEXT
		, DESCRIPTION TEXT
		);
		
		CREATE TABLE IF NOT EXISTS ATTACHMENT_V1 (
		ATTACHMENTID INTEGER NOT NULL PRIMARY KEY
		, REFTYPE TEXT NOT NULL /* Transaction, Stock, Asset, BankAccount, RepeatingTransaction, Payee */
		, REFID INTEGER NOT NULL
		, DESCRIPTION TEXT COLLATE NOCASE
		, FILENAME TEXT NOT NULL COLLATE NOCASE
		);
		
		CREATE TABLE IF NOT EXISTS ASSETCLASS_V1 (
		ID INTEGER primary key
		, PARENTID INTEGER
		, NAME TEXT COLLATE NOCASE NOT NULL
		, ALLOCATION REAL
		, SORTORDER INTEGER
		);
		
		CREATE TABLE IF NOT EXISTS ASSETCLASS_STOCK_V1 (
		ID INTEGER primary key
		, ASSETCLASSID INTEGER NOT NULL
		, STOCKSYMBOL TEXT UNIQUE
		);
		
		CREATE TABLE IF NOT EXISTS CUSTOMFIELD_V1 (
		FIELDID INTEGER NOT NULL PRIMARY KEY
		, REFTYPE TEXT NOT NULL /* Transaction, Stock, Asset, BankAccount, RepeatingTransaction, Payee */
		, DESCRIPTION TEXT COLLATE NOCASE
		, TYPE TEXT NOT NULL /* String, Integer, Decimal, Boolean, Date, Time, SingleChoice, MultiChoice */
		, PROPERTIES TEXT NOT NULL
		);
		
		CREATE TABLE IF NOT EXISTS CUSTOMFIELDDATA_V1 (
		FIELDATADID INTEGER NOT NULL PRIMARY KEY
		, FIELDID INTEGER NOT NULL
		, REFID INTEGER NOT NULL
		, CONTENT TEXT
		, UNIQUE(FIELDID, REFID)
		);
		
		CREATE TABLE IF NOT EXISTS TRANSLINK_V1 (
		TRANSLINKID  integer NOT NULL primary key
		, CHECKINGACCOUNTID integer NOT NULL
		, LINKTYPE TEXT NOT NULL /* Asset, Stock */
		, LINKRECORDID integer NOT NULL
		);
		
		CREATE TABLE IF NOT EXISTS SHAREINFO_V1 (
		SHAREINFOID integer NOT NULL primary key
		, CHECKINGACCOUNTID integer NOT NULL
		, SHARENUMBER numeric
		, SHAREPRICE numeric
		, SHARECOMMISSION numeric
		, SHARELOT TEXT
		);
		
		-- unused
		
		DROP TABLE IF EXISTS SPLITTRANSACTIONS_V2;
		DROP VIEW IF EXISTS alldata;
		
		-- recreate renamed tables with the same structure
		-- then copy existing data to validate with db schema
		
		CREATE TABLE ACCOUNTLIST(
		ACCOUNTID integer primary key
		, ACCOUNTNAME TEXT COLLATE NOCASE NOT NULL UNIQUE
		, ACCOUNTTYPE TEXT NOT NULL /* Checking, Term, Investment, Credit Card */
		, ACCOUNTNUM TEXT
		, STATUS TEXT NOT NULL /* Open, Closed */
		, NOTES TEXT
		, HELDAT TEXT
		, WEBSITE TEXT
		, CONTACTINFO TEXT
		, ACCESSINFO TEXT
		, INITIALBAL numeric
		, FAVORITEACCT TEXT NOT NULL
		, CURRENCYID integer NOT NULL
		, STATEMENTLOCKED integer
		, STATEMENTDATE TEXT
		, MINIMUMBALANCE numeric
		, CREDITLIMIT numeric
		, INTERESTRATE numeric
		, PAYMENTDUEDATE text
		, MINIMUMPAYMENT numeric
		);
		INSERT INTO ACCOUNTLIST SELECT
		ACCOUNTID
		, ACCOUNTNAME
		, ACCOUNTTYPE
		, ACCOUNTNUM
		, STATUS
		, NOTES
		, HELDAT
		, WEBSITE
		, CONTACTINFO
		, ACCESSINFO
		, INITIALBAL
		, FAVORITEACCT
		, CURRENCYID
		, STATEMENTLOCKED
		, STATEMENTDATE
		, MINIMUMBALANCE
		, CREDITLIMIT
		, INTERESTRATE
		, PAYMENTDUEDATE
		, MINIMUMPAYMENT
		FROM ACCOUNTLIST_V1;
		DROP INDEX IF EXISTS IDX_ACCOUNTLIST_ACCOUNTTYPE;
		DROP TABLE ACCOUNTLIST_V1;
		CREATE INDEX IDX_ACCOUNTLIST_ACCOUNTTYPE ON ACCOUNTLIST(ACCOUNTTYPE);
		
		CREATE TABLE ASSETS(
		ASSETID integer primary key
		, STARTDATE TEXT NOT NULL
		, ASSETNAME TEXT COLLATE NOCASE NOT NULL
		, VALUE numeric
		, VALUECHANGE TEXT /* None, Appreciates, Depreciates */
		, NOTES TEXT
		, VALUECHANGERATE numeric
		, ASSETTYPE TEXT /* Property, Automobile, Household Object, Art, Jewellery, Cash, Other */
		);
		INSERT INTO ASSETS SELECT
		ASSETID
		, STARTDATE
		, ASSETNAME
		, VALUE
		, VALUECHANGE
		, NOTES
		, VALUECHANGERATE
		, ASSETTYPE
		FROM ASSETS_V1;
		DROP INDEX IF EXISTS IDX_ASSETS_ASSETTYPE;
		DROP TABLE ASSETS_V1;
		CREATE INDEX IDX_ASSETS_ASSETTYPE ON ASSETS(ASSETTYPE);
		
		CREATE TABLE BILLSDEPOSITS(
		BDID integer primary key
		, ACCOUNTID integer NOT NULL
		, TOACCOUNTID integer
		, PAYEEID integer NOT NULL
		, TRANSCODE TEXT NOT NULL /* Withdrawal, Deposit, Transfer */
		, TRANSAMOUNT numeric NOT NULL
		, STATUS TEXT /* None, Reconciled, Void, Follow up, Duplicate */
		, TRANSACTIONNUMBER TEXT
		, NOTES TEXT
		, CATEGID integer
		, SUBCATEGID integer
		, TRANSDATE TEXT
		, FOLLOWUPID integer
		, TOTRANSAMOUNT numeric
		, REPEATS integer
		, NEXTOCCURRENCEDATE TEXT
		, NUMOCCURRENCES integer
		);
		INSERT INTO BILLSDEPOSITS SELECT
		BDID
		, ACCOUNTID
		, TOACCOUNTID
		, PAYEEID
		, TRANSCODE
		, TRANSAMOUNT
		, STATUS
		, TRANSACTIONNUMBER
		, NOTES
		, CATEGID
		, SUBCATEGID
		, TRANSDATE
		, FOLLOWUPID
		, TOTRANSAMOUNT
		, REPEATS
		, NEXTOCCURRENCEDATE
		, NUMOCCURRENCES
		FROM BILLSDEPOSITS_V1;
		DROP INDEX IF EXISTS IDX_BILLSDEPOSITS_ACCOUNT;
		DROP TABLE BILLSDEPOSITS_V1;
		CREATE INDEX IDX_BILLSDEPOSITS_ACCOUNT ON BILLSDEPOSITS (ACCOUNTID, TOACCOUNTID);
		
		CREATE TABLE BUDGETSPLITTRANSACTIONS(
		SPLITTRANSID integer primary key
		, TRANSID integer NOT NULL
		, CATEGID integer
		, SUBCATEGID integer
		, SPLITTRANSAMOUNT numeric
		);
		INSERT INTO BUDGETSPLITTRANSACTIONS SELECT
		SPLITTRANSID
		, TRANSID
		, CATEGID
		, SUBCATEGID
		, SPLITTRANSAMOUNT
		FROM BUDGETSPLITTRANSACTIONS_V1;
		DROP INDEX IF EXISTS IDX_BUDGETSPLITTRANSACTIONS_TRANSID;
		DROP TABLE BUDGETSPLITTRANSACTIONS_V1;
		CREATE INDEX IDX_BUDGETSPLITTRANSACTIONS_TRANSID ON BUDGETSPLITTRANSACTIONS(TRANSID);
		
		CREATE TABLE BUDGETTABLE(
		BUDGETENTRYID integer primar)"
	R"(y key
		, BUDGETYEARID integer
		, CATEGID integer
		, SUBCATEGID integer
		, PERIOD TEXT NOT NULL /* None, Weekly, Bi-Weekly, Monthly, Monthly, Bi-Monthly, Quarterly, Half-Yearly, Yearly, Daily*/
		, AMOUNT numeric NOT NULL
		);
		INSERT INTO BUDGETTABLE SELECT
		BUDGETENTRYID
		, BUDGETYEARID
		, CATEGID
		, SUBCATEGID
		, PERIOD
		, AMOUNT
		FROM BUDGETTABLE_V1;
		DROP INDEX IF EXISTS IDX_BUDGETTABLE_BUDGETYEARID;
		DROP TABLE BUDGETTABLE_V1;
		CREATE INDEX IDX_BUDGETTABLE_BUDGETYEARID ON BUDGETTABLE(BUDGETYEARID);
		
		CREATE TABLE BUDGETYEAR(
		BUDGETYEARID integer primary key
		, BUDGETYEARNAME TEXT NOT NULL UNIQUE
		);
		INSERT INTO BUDGETYEAR SELECT
		BUDGETYEARID
		, BUDGETYEARNAME
		FROM BUDGETYEAR_V1;
		DROP INDEX IF EXISTS IDX_BUDGETYEAR_BUDGETYEARNAME;
		DROP TABLE BUDGETYEAR_V1;
		CREATE INDEX IDX_BUDGETYEAR_BUDGETYEARNAME ON BUDGETYEAR(BUDGETYEARNAME);
		
		CREATE TABLE CATEGORY(
		CATEGID integer primary key
		, CATEGNAME TEXT COLLATE NOCASE NOT NULL UNIQUE
		);
		INSERT INTO CATEGORY SELECT
		CATEGID
		, CATEGNAME
		FROM CATEGORY_V1;
		DROP INDEX IF EXISTS IDX_CATEGORY_CATEGNAME;
		DROP TABLE CATEGORY_V1;
		CREATE INDEX IDX_CATEGORY_CATEGNAME ON CATEGORY(CATEGNAME);
		
		CREATE TABLE CHECKINGACCOUNT(
		TRANSID integer primary key
		, ACCOUNTID integer NOT NULL
		, TOACCOUNTID integer
		, PAYEEID integer NOT NULL
		, TRANSCODE TEXT NOT NULL /* Withdrawal, Deposit, Transfer */
		, TRANSAMOUNT numeric NOT NULL
		, STATUS TEXT /* None, Reconciled, Void, Follow up, Duplicate */
		, TRANSACTIONNUMBER TEXT
		, NOTES TEXT
		, CATEGID integer
		, SUBCATEGID integer
		, TRANSDATE TEXT
		, FOLLOWUPID integer
		, TOTRANSAMOUNT numeric
		);
		INSERT INTO CHECKINGACCOUNT SELECT
		TRANSID
		, ACCOUNTID
		, TOACCOUNTID
		, PAYEEID
		, TRANSCODE
		, TRANSAMOUNT
		, STATUS
		, TRANSACTIONNUMBER
		, NOTES
		, CATEGID
		, SUBCATEGID
		, TRANSDATE
		, FOLLOWUPID
		, TOTRANSAMOUNT
		FROM CHECKINGACCOUNT_V1;
		DROP INDEX IF EXISTS IDX_CHECKINGACCOUNT_ACCOUNT;
		DROP INDEX IF EXISTS IDX_CHECKINGACCOUNT_TRANSDATE;
		DROP TABLE CHECKINGACCOUNT_V1;
		CREATE INDEX IDX_CHECKINGACCOUNT_ACCOUNT ON CHECKINGACCOUNT (ACCOUNTID, TOACCOUNTID);
		CREATE INDEX IDX_CHECKINGACCOUNT_TRANSDATE ON CHECKINGACCOUNT (TRANSDATE);
		
		CREATE TABLE CURRENCYHISTORY(
		CURRHISTID INTEGER PRIMARY KEY
		, CURRENCYID INTEGER NOT NULL
		, CURRDATE TEXT NOT NULL
		, CURRVALUE NUMERIC NOT NULL
		, CURRUPDTYPE INTEGER
		, UNIQUE(CURRENCYID, CURRDATE)
		);
		INSERT INTO CURRENCYHISTORY SELECT
		CURRHISTID
		, CURRENCYID
		, CURRDATE
		, CURRVALUE
		, CURRUPDTYPE
		FROM CURRENCYHISTORY_V1;
		DROP INDEX IF EXISTS IDX_CURRENCYHISTORY_CURRENCYID_CURRDATE;
		DROP TABLE CURRENCYHISTORY_V1;
		CREATE INDEX IDX_CURRENCYHISTORY_CURRENCYID_CURRDATE ON CURRENCYHISTORY(CURRENCYID, CURRDATE);
		
		CREATE TABLE CURRENCYFORMATS(
		CURRENCYID integer primary key
		, CURRENCYNAME TEXT COLLATE NOCASE NOT NULL
		, PFX_SYMBOL TEXT
		, SFX_SYMBOL TEXT
		, DECIMAL_POINT TEXT
		, GROUP_SEPARATOR TEXT
		, SCALE integer
		, BASECONVRATE numeric
		, CURRENCY_SYMBOL TEXT COLLATE NOCASE NOT NULL UNIQUE
		, CURRENCY_TYPE TEXT /* Traditional, Crypto */
		, HISTORIC integer DEFAULT 0 /* 1 if no longer official */
		);
		INSERT INTO CURRENCYFORMATS SELECT
		CURRENCYID
		, CURRENCYNAME
		, PFX_SYMBOL
		, SFX_SYMBOL
		, DECIMAL_POINT
		, GROUP_SEPARATOR
		, SCALE
		, BASECONVRATE
		, CURRENCY_SYMBOL
		, CURRENCY_TYPE
		, HISTORIC
		FROM CURRENCYFORMATS_V1;
		DROP INDEX IF EXISTS IDX_CURRENCYFORMATS_SYMBOL;
		DROP TABLE CURRENCYFORMATS_V1;
		CREATE INDEX IDX_CURRENCYFORMATS_SYMBOL ON CURRENCYFORMATS(CURRENCY_SYMBOL);
		
		CREATE TABLE INFOTABLE(
		INFOID integer not null primary key
		, INFONAME TEXT COLLATE NOCASE NOT NULL UNIQUE
		, INFOVALUE TEXT NOT NULL
		);
		INSERT INTO INFOTABLE SELECT
		INFOID
		, INFONAME
		, INFOVALUE
		FROM INFOTABLE_V1;
		DROP INDEX IF EXISTS IDX_INFOTABLE_INFONAME;
		DROP TABLE INFOTABLE_V1;
		CREATE INDEX IDX_INFOTABLE_INFONAME ON INFOTABLE(INFONAME);
		
		CREATE TABLE PAYEE(
		PAYEEID integer primary key
		, PAYEENAME TEXT COLLATE NOCASE NOT NULL UNIQUE
		, CATEGID integer
		, SUBCATEGID integer
		);
		INSERT INTO PAYEE SELECT
		PAYEEID
		, PAYEENAME
		, CATEGID
		, SUBCATEGID
		FROM PAYEE_V1;
		DROP INDEX IF EXISTS IDX_PAYEE_INFONAME;
		DROP TABLE PAYEE_V1;
		CREATE INDEX IDX_PAYEE_INFONAME ON PAYEE(PAYEENAME);
		
		CREATE TABLE SPLITTRANSACTIONS(
		SPLITTRANSID integer primary key
		, TRANSID integer NOT NULL
		, CATEGID integer
		, SUBCATEGID integer
		, SPLITTRANSAMOUNT numeric
		);
		INSERT INTO SPLITTRANSACTIONS SELECT
		SPLITTRANSID
		, TRANSID
		, CATEGID
		, SUBCATEGID
		, SPLITTRANSAMOUNT
		FROM SPLITTRANSACTIONS_V1;
		DROP INDEX IF EXISTS IDX_SPLITTRANSACTIONS_TRANSID;
		DROP TABLE SPLITTRANSACTIONS_V1;
		CREATE INDEX IDX_SPLITTRANSACTIONS_TRANSID ON SPLITTRANSACTIONS(TRANSID);
		
		CREATE TABLE STOCK(
		STOCKID integer primary key
		, HELDAT integer
		, PURCHASEDATE TEXT NOT NULL
		, STOCKNAME TEXT COLLATE NOCASE NOT NULL
		, SYMBOL TEXT
		, NUMSHARES numeric
		, PURCHASEPRICE numeric NOT NULL
		, NOTES TEXT
		, CURRENTPRICE numeric NOT NULL
		, VALUE numeric
		, COMMISSION numeric
		);
		INSERT INTO STOCK SELECT
		STOCKID
		, HELDAT
		, PURCHASEDATE
		, STOCKNAME
		, SYMBOL
		, NUMSHARES
		, PURCHASEPRICE
		, NOTES
		, CURRENTPRICE
		, VALUE
		, COMMISSION
		FROM STOCK_V1;
		DROP INDEX IF EXISTS IDX_STOCK_HELDAT;
		DROP TABLE STOCK_V1;
		CREATE INDEX IDX_STOCK_HELDAT ON STOCK(HELDAT);
		
		CREATE TABLE STOCKHISTORY(
		HISTID integer primary key
		, SYMBOL TEXT NOT NULL
		, DATE TEXT NOT NULL
		, VALUE numeric NOT NULL
		, UPDTYPE integer
		, UNIQUE(SYMBOL, DATE)
		);
		INSERT INTO STOCKHISTORY SELECT
		HISTID
		, SYMBOL
		, DATE
		, VALUE
		, UPDTYPE
		FROM STOCKHISTORY_V1;
		DROP INDEX IF EXISTS IDX_STOCKHISTORY_SYMBOL;
		DROP TABLE STOCKHISTORY_V1;
		CREATE INDEX IDX_STOCKHISTORY_SYMBOL ON STOCKHISTORY(SYMBOL);
		
		CREATE TABLE SUBCATEGORY(
		SUBCATEGID integer primary key
		, SUBCATEGNAME TEXT COLLATE NOCASE NOT NULL
		, CATEGID integer NOT NULL
		, UNIQUE(CATEGID, SUBCATEGNAME)
		);
		INSERT INTO SUBCATEGORY SELECT
		SUBCATEGID
		, SUBCATEGNAME
		, CATEGID
		FROM SUBCATEGORY_V1;
		DROP INDEX IF EXISTS IDX_SUBCATEGORY_CATEGID;
		DROP TABLE SUBCATEGORY_V1;
		CREATE INDEX IDX_SUBCATEGORY_CATEGID ON SUBCATEGORY(CATEGID);
		
		create table REPORT(
		REPORTID integer not null primary key
		, REPORTNAME TEXT COLLATE NOCASE NOT NULL UNIQUE
		, GROUPNAME TEXT COLLATE NOCASE
		, SQLCONTENT TEXT
		, LUACONTENT TEXT
		, TEMPLATECONTENT TEXT
		, DESCRIPTION TEXT
		);
		INSERT INTO REPORT SELECT
		REPORTID
		, REPORTNAME
		, GROUPNAME
		, SQLCONTENT
		, LUACONTENT
		, TEMPLATECONTENT
		, DESCRIPTION
		FROM REPORT_V1;
		DROP INDEX IF EXISTS INDEX_REPORT_NAME;
		DROP TABLE REPORT_V1;
		CREATE INDEX INDEX_REPORT_NAME ON REPORT(REPORTNAME);
		
		CREATE TABLE ATTACHMENT (
		ATTACHMENTID INTEGER NOT NULL PRIMARY KEY
		, REFTYPE TEXT NOT NULL /* Transaction, Stock, Asset, BankAccount, RepeatingTransaction, Payee */
		, REFID INTEGER NOT NULL
		, DESCRIPTION TEXT COLLATE NOCASE
		, FILENAME TEXT NOT NULL COLLATE NOCASE
		);
		INSERT INTO ATTACHMENT SELECT
		ATTACHMENTID
		, REFTYPE
		, REFID
		, DESCRIPTION
		, FILENAME
		FROM ATTACHMENT_V1;
		DROP INDEX IF EXISTS IDX_ATTACHMENT_REF;
		DROP TABLE ATTACHMENT_V1;
		CREATE INDEX IDX_ATTACHMENT_REF ON ATTACHMENT (REFTYPE, REFID);
		
		CREATE TABLE ASSETCLASS (
		ID INTEGER primary key
		, PARENTID INTEGER
		, NAME TEXT COLLATE NOCASE NOT NULL
		, ALLOCATION REAL
		, SORTORDER INTEGER
		);
		INSERT INTO ASSETCLASS SELECT
		ID
		, PARENTID
		, NAME
		, ALLOCATION
		, SORTORDER
		FROM ASSETCLASS_V1;
		DROP TABLE ASSETCLASS_V1;
		
		CREATE TABLE ASSETCLASS_STOCK (
		ID INTEGER primary key
		, ASSETCLASSID INTEGER NOT NULL
		, STOCKSYMBOL TEXT UNIQUE
		);
		INSERT INTO ASSETCLASS_STOCK SELECT
		ID
		, ASSETCLASSID
		, STOCKSYMBOL
		FROM ASSETCLASS_STOCK_V1;
		DROP TABLE ASSETCLASS_STOCK_V1;
		
		CREATE TABLE CUSTOMFIELD (
		FIELDID INTEGER NOT NULL PRIMARY KEY
		, REFTYPE TEXT NOT NULL /* Transaction, Stock, Asset, BankAccount, RepeatingTransaction, Payee */
		, DESCRIPTION TEXT COLLATE NOCASE
		, TYPE TEXT NOT NULL /* String, Integer, Decimal, Boolean, Date, Time, SingleChoice, MultiChoice */
		, PROPERTIES TEXT NOT NULL
		);
		INSERT INTO CUSTOMFIELD SELECT
		FIELDID
		, REFTYPE
		, DESCRIPTION
		, TYPE
		, PROPERTIES
		FROM CUSTOMFIELD_V1;
		DROP INDEX IF EXISTS IDX_CUSTOMFIELD_REF;
		DROP TABLE CUSTOMFIELD_V1;
		CREATE INDEX IDX_CUSTOMFIELD_REF ON CUSTOMFIELD (REFTYPE);
		
		CREATE TABLE CUSTOMFIELDDATA (
		FIELDATADID INTEGER NOT NULL PRIMARY KEY
		, FIELDID INTEGER NOT NULL
		, REFID INTEGER NOT NULL
		, CONTENT TEXT
		, UNIQUE(FIELDID, REFID)
		);
		INSERT INTO CUSTOMFIELDDATA SELECT
		FIELDATADID
		, FIELDID
		, REFID
		, CONTENT
		FROM CUSTOMFIELDDATA_V1;
		DROP INDEX IF EXISTS IDX_CUSTOMFIELDDATA_REF;
		DROP TABLE CUSTOMFIELDDATA_V1;
		CREATE INDEX IDX_CUSTOMFIELDDATA_REF ON CUSTOMFIELDDATA (FIELDID, REFID);
		
		CREATE TABLE TRANSLINK (
		TRANSLINKID integer NOT NULL primary key
		, CHECKINGACCOUNTID integer NOT NULL
		, LINKTYPE TEXT NOT NULL /* Asset, Stock */
		, LINKRECORDID integer NOT NULL
		);
		INSERT INTO TRANSLINK SELECT
		TRANSLINKID
		, CHECKINGACCOUNTID
		, LINKTYPE
		, LINKRECORDID
		FROM TRANSLINK_V1;
		DROP INDEX IF EXISTS IDX_LINKRECORD;
		DROP INDEX IF EXISTS IDX_CHECKINGACCOUNT;
		DROP TABLE TRANSLINK_V1;
		CREATE INDEX IDX_LINKRECORD ON TRANSLINK (LINKTYPE, LINKRECORDID);
		CREATE INDEX IDX_CHECKINGACCOUNT ON TRANSLINK (CHECKINGACCOUNTID);
		
		CREATE TABLE SHAREINFO (
		SHAREINFOID integer NOT NULL primary key
		, CHECKINGACCOUNTID integer NOT NULL
		, SHARENUMBER numeric
		, SHAREPRICE numeric
		, SHARECOMMISSION numeric
		, SHARELOT TEXT
		);
		INSERT INTO SHAREINFO SELECT
		SHAREINFOID
		, CHECKINGACCOUNTID
		, SHARENUMBER
		, SHAREPRICE
		, SHARECOMMISSION
		, SHARELOT
		FROM SHAREINFO_V1;
		DROP INDEX IF EXISTS IDX_SHAREINFO;
		DROP TABLE SHAREINFO_V1;
		CREATE INDEX IDX_SHAREINFO ON SHAREINFO (CHECKINGACCOUNTID);
		
		-- rename table names used in GRM definitions
		
		UPDATE REPORT SET SQLCONTENT = REPLACE(SQLCONTENT, '_V1', '' ) WHERE SQLCONTENT LIKE '%_V1%';
		UPDATE REPORT SET LUACONTENT = REPLACE(LUACONTENT, )"
	R"('_V1', '' ) WHERE LUACONTENT LIKE '%_V1%';
		UPDATE REPORT SET TEMPLATECONTENT = REPLACE(TEMPLATECONTENT, '_V1', '' ) WHERE TEMPLATECONTENT LIKE '%_V1%';
		
		-- finish
		
		PRAGMA user_version = 12;)",

	// Upgrade to version 13
	R"(-- remove BASECONVRATE
		
		CREATE TABLE CURRENCYFORMATS_NEW(
		CURRENCYID integer primary key
		, CURRENCYNAME TEXT COLLATE NOCASE NOT NULL
		, PFX_SYMBOL TEXT
		, SFX_SYMBOL TEXT
		, DECIMAL_POINT TEXT
		, GROUP_SEPARATOR TEXT
		, SCALE integer
		, CURRENCY_SYMBOL TEXT COLLATE NOCASE NOT NULL UNIQUE
		, CURRENCY_TYPE TEXT /* Fiat, Crypto */
		, HISTORIC integer DEFAULT 0 /* 1 if no longer official */
		);
		
		INSERT INTO CURRENCYFORMATS_NEW SELECT
		CURRENCYID
		, CURRENCYNAME
		, PFX_SYMBOL
		, SFX_SYMBOL
		, DECIMAL_POINT
		, GROUP_SEPARATOR
		, SCALE
		, CURRENCY_SYMBOL
		, CURRENCY_TYPE
		, HISTORIC
		FROM CURRENCYFORMATS;
		
		DROP INDEX IDX_CURRENCYFORMATS_SYMBOL;
		DROP TABLE CURRENCYFORMATS;
		ALTER TABLE CURRENCYFORMATS_NEW RENAME TO CURRENCYFORMATS;
		CREATE INDEX IDX_CURRENCYFORMATS_SYMBOL ON CURRENCYFORMATS(CURRENCY_SYMBOL);
		
		PRAGMA user_version = 13;)",

};

#endif // DB_UPGRADE_H_
