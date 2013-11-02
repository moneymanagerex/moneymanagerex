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
#include <wx/arrstr.h>
#include <wx/sharedptr.h>

class wxSQLite3Database;
class wxSQLite3ResultSet;

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

wxSharedPtr<wxSQLite3Database> Open(const wxString &dbpath, const wxString &key = "");

bool ViewExists(wxSQLite3Database* db, const char *viewName);
bool IsSelect(wxSQLite3Database* db, const wxString& sScript, int &rows);

/* Creating new DBs */
int createAllDataView(wxSQLite3Database* db);

} // namespace mmDBWrapper

//----------------------------------------------------------------------------

#endif // _MM_EX_DBWRAPPER_H_
//----------------------------------------------------------------------------
