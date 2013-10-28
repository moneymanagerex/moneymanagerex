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

#include "dbwrapper.h"
#include "util.h"
#include "mmCurrencyFormatter.h"
#include "mmOption.h"
#include "paths.h"
#include "constants.h"
#include "mmcurrency.h"
#include "model/Model_Budgetyear.h"
//----------------------------------------------------------------------------
#include "sqlite3.h"
//----------------------------------------------------------------------------

/*
    wxSQLite3Database::ViewExists was removed.
*/
bool mmDBWrapper::ViewExists(wxSQLite3Database* db, const char *viewName)
{
    static const char sql[] =
    "select 1 "
    "from sqlite_master "
    "where type = 'view' and name like ?";

    wxASSERT(viewName);

    bool exists = false;

    wxSQLite3Statement st = db->PrepareStatement(sql);
    st.Bind(1, viewName);

    wxSQLite3ResultSet rs = st.ExecuteQuery();
    exists = rs.NextRow();

    st.Finalize();

    return exists;
}

int mmDBWrapper::createAllDataView(wxSQLite3Database* db)
{
    int iError = 0;
    try
    {
        static const char view_name[] = "ALLDATA";
        bool exists = ViewExists(db, view_name);

        if (!exists) {
            db->ExecuteUpdate(CREATE_VIEW_ALLDATA);
            exists = ViewExists(db, view_name);
            wxASSERT(exists);
        }
    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogDebug("Database::createAllDataView: %s", e.GetMessage());
        wxLogError("create AllData view. " + wxString::Format(_("Error: %s"), e.GetMessage()));
        iError = e.GetErrorCode();
    }
    return iError;
}

/*
    This routine sets up a new DB as well as update an old one
*/

void mmDBWrapper::loadCurrencySettings(wxSQLite3Database* db, int currencyID)
{
    wxSQLite3Statement st = db->PrepareStatement(SELECT_ROW_FROM_CURRENCYFORMATS_V1);
    st.Bind(1, currencyID);

    wxSQLite3ResultSet q1 = st.ExecuteQuery();

    if (q1.NextRow()) {

        wxString pfxSymbol = q1.GetString("PFX_SYMBOL");
        wxString sfxSymbol = q1.GetString("SFX_SYMBOL");
        wxString dec = q1.GetString("DECIMAL_POINT");
        wxString grp = q1.GetString("GROUP_SEPARATOR");
        wxString unit = q1.GetString("UNIT_NAME");
        wxString cent = q1.GetString("CENT_NAME");
        double scaleDl = q1.GetDouble("SCALE");
        wxString currencySymbol = q1.GetString("CURRENCY_SYMBOL");

        wxChar decChar = 0;
        wxChar grpChar = 0;

        if (!dec.empty()) {
            decChar = dec.GetChar(0);
        }

        if (!grp.empty()) {
            grpChar = grp.GetChar(0);
        }

        CurrencyFormatter::instance().loadSettings(pfxSymbol, sfxSymbol, decChar, grpChar, unit, cent, scaleDl);

    } else {
        wxASSERT(true);
    }

    st.Finalize();
}

bool mmDBWrapper::deleteTransaction(wxSQLite3Database* db, int transID)
{
    try
    {
        static const char* sql[] =
        {
            "delete from CHECKINGACCOUNT_V1   where TRANSID = ?",
            "delete from SPLITTRANSACTIONS_V1 where TRANSID = ?",
            0
        };

        for (int i = 0; sql[i]; ++i)
        {
            wxSQLite3Statement st = db->PrepareStatement(sql[i]);
            st.Bind(1, transID);
            st.ExecuteUpdate();
            st.Finalize();
        }

        mmOptions::instance().databaseUpdated_ = true;
        return true;
    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogDebug("Database::deleteTransaction: %s", e.GetMessage());
        wxLogError("Delete Transaction. " + wxString::Format(_("Error: %s"), e.GetMessage()));
        return false;
    }
}

//--------------------------------------------------------------------

bool mmDBWrapper::updateTransactionWithStatus(wxSQLite3Database &db, int transID, const wxString& status)
{
    try
    {
        wxSQLite3Statement st = db.PrepareStatement(SET_STATUS_CHECKINGACCOUNT_V1);
        st.Bind(1, status);
        st.Bind(2, transID);

        st.ExecuteUpdate();
        st.Finalize();
        mmOptions::instance().databaseUpdated_ = true;

    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogDebug("Database::updateTransactionWithStatus: %s", e.GetMessage());
        wxLogError("Update Transaction With Status. " + wxString::Format(_("Error: %s"), e.GetMessage()));
        return false;
    }
    return true;
}

bool mmDBWrapper::copyBudgetYear(wxSQLite3Database* db, int newYear, int baseYear)
{
    static const char INSERT_INTO_BUDGETTABLE_V1[] =
    "INSERT INTO BUDGETTABLE_V1 ( "
      "BUDGETYEARID, "
      "CATEGID, "
      "SUBCATEGID, "
      "PERIOD, "
      "AMOUNT "
    ") "
    "select ?, "
           "CATEGID, "
           "SUBCATEGID, "
           "PERIOD, "
           "AMOUNT "
    "from BUDGETTABLE_V1 "
    "WHERE BUDGETYEARID = ?";

    try
    {
         if (newYear == baseYear)
             return false;

         wxSQLite3Statement st = db->PrepareStatement(INSERT_INTO_BUDGETTABLE_V1);
         st.Bind(1, newYear);
         st.Bind(2, baseYear);

         st.ExecuteUpdate();
         st.Finalize();
         mmOptions::instance().databaseUpdated_ = true;

    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogDebug("Function::copyBudgetYear: %s", e.GetMessage());
        wxLogError("Copy Budget Year. " + wxString::Format(_("Error: %s"), e.GetMessage()));
    }
    return true;
}

bool mmDBWrapper::deleteBudgetYear(wxSQLite3Database* db, const wxString& yearName)
 {
    int budgetYearID = Model_Budgetyear::instance().Get(yearName); // FIXME
    if (budgetYearID == -1)
        return false;

    static const char* sql[] =
    {
        "delete from BUDGETTABLE_V1 where BUDGETYEARID = ?",
        "delete from BUDGETYEAR_V1  where BUDGETYEARID = ?",
        0
    };

    try
    {
        for (int i = 0; sql[i]; ++i)
        {
            wxSQLite3Statement st = db->PrepareStatement(sql[i]);
            st.Bind(1, budgetYearID);
            st.ExecuteUpdate();
            st.Finalize();
        }
        mmOptions::instance().databaseUpdated_ = true;

    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogDebug("Function::deleteBudgetYear: %s", e.GetMessage());
        wxLogError("Delete Budget Year. " + wxString::Format(_("Error: %s"), e.GetMessage()));
    }
    return true;
 }

void mmDBWrapper::addBudgetEntry(wxSQLite3Database* db, int budgetYearID,
    int categID, int subCategID, const wxString& period, double amount)
{
    try
    {
        wxSQLite3Statement st = db->PrepareStatement(INSERT_INTO_BUDGETTABLE_V1);

        int i = 0;
        st.Bind(++i, budgetYearID);
        st.Bind(++i, categID);
        st.Bind(++i, subCategID);
        st.Bind(++i, period);
        st.Bind(++i, amount);

        wxASSERT(st.GetParamCount() == i);

        st.ExecuteUpdate();
        st.Finalize();
        mmOptions::instance().databaseUpdated_ = true;

    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogDebug("Function::addBudgetEntry: %s", e.GetMessage());
        wxLogError("Add Budget Entry. " + wxString::Format(_("Error: %s"), e.GetMessage()));
    }
}

void mmDBWrapper::deleteBudgetEntry(wxSQLite3Database* db, int budgetEntryID)
{
    try
    {
        wxSQLite3Statement st = db->PrepareStatement(DELETE_FROM_BUDGETTABLE_V1);
        st.Bind(1, budgetEntryID);

        st.ExecuteUpdate();
        st.Finalize();
        mmOptions::instance().databaseUpdated_ = true;

    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogDebug("Function::deleteBudgetEntry: %s", e.GetMessage());
        wxLogError("delete from BUDGETTABLE_V1. " + wxString::Format(_("Error: %s"), e.GetMessage()));
    }
}

void mmDBWrapper::updateBudgetEntry(wxSQLite3Database* db,
    int budgetYearID, int categID, int subCategID, const wxString& period, double amount)
{
    try {

        wxSQLite3Statement st = db->PrepareStatement(DELETE_BUDGETENTRYIDS_FROM_BUDGETTABLE_V1);
        st.Bind(1, budgetYearID);
        st.Bind(2, categID);
        st.Bind(3, subCategID);

        st.ExecuteUpdate();
        st.Finalize();

        addBudgetEntry(db, budgetYearID, categID, subCategID, period, amount);
    } catch(const wxSQLite3Exception& e)
    {
        wxLogDebug("Function::updateBudgetEntry: %s", e.GetMessage());
        wxLogError("delete from BUDGETTABLE_V1. " + wxString::Format(_("Error: %s"), e.GetMessage()));
    }
}

bool mmDBWrapper::getBudgetEntry(wxSQLite3Database* db, int budgetYearID,
    int categID, int subCategID, wxString& period, double& amount)
{
    bool found = false;

    wxSQLite3Statement st = db->PrepareStatement(SELECT_ROW_FROM_BUDGETTABLE_V1);
    st.Bind(1, budgetYearID);
    st.Bind(2, categID);
    st.Bind(3, subCategID);

    wxSQLite3ResultSet q1 = st.ExecuteQuery();

    if (q1.NextRow())
    {
        found = true;
        period = q1.GetString("PERIOD", "None");
        amount = q1.GetDouble("AMOUNT");
    }

    st.Finalize();

    return found;
}

void mmDBWrapper::deleteBDSeries(wxSQLite3Database* db, int bdID)
{
    try
    {
        static const char* sql[] =
        {
            "DELETE FROM BILLSDEPOSITS_V1 where BDID = ?",
            "DELETE FROM BUDGETSPLITTRANSACTIONS_V1 where TRANSID = ?",
            0
        };

        for (int i = 0; sql[i]; ++i)
        {
            wxSQLite3Statement st = db->PrepareStatement(sql[i]);
            st.Bind(1, bdID);
            st.ExecuteUpdate();
            st.Finalize();
        }
        mmOptions::instance().databaseUpdated_ = true;

    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogDebug("Function::deleteBDSeries: %s", e.GetMessage());
        wxLogError(wxString::Format(_("Error: %s"), e.GetMessage()));
    }
}

void mmDBWrapper::removeSplitsForAccount(wxSQLite3Database* db, int accountID)
{
    try
    {
        wxSQLite3Statement st = db->PrepareStatement(DELETE_FROM_SPLITTRANSACTIONS_V1);
        st.Bind(1, accountID);
        st.Bind(2, accountID);

        st.ExecuteUpdate();
        st.Finalize();

        st = db->PrepareStatement(DELETE_FROM_BUDGETSPLITTRANSACTIONS_V1);
        st.Bind(1, accountID);
        st.Bind(2, accountID);

        st.ExecuteUpdate();
        st.Finalize();

        mmOptions::instance().databaseUpdated_ = true;
    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogDebug("Function::removeSplitsForAccount: %s", e.GetMessage());
        wxLogError("Remove Splits For Account. " + wxString::Format(_("Error: %s"), e.GetMessage()));
    }
}

bool mmDBWrapper::IsSelect(wxSQLite3Database* db, const wxString& sScript, int &rows)
{
    wxString sql_script_exception;
    wxString sql_modify;
    try
    {
        rows =db->ExecuteScalar("select count (*) from (\n" + sScript + "\n)");
    }
    catch(const wxSQLite3Exception& e)
    {
        sql_script_exception = e.GetMessage();
    }

    sql_modify = sql_script_exception.Lower();
    if (sql_modify.Contains("update") ||
        sql_modify.Contains("delete") ||
        sql_modify.Contains("insert"))
    {
        return false;
    }
    return true;
}

//----------------------------------------------------------------------------

/*
    SQLITE_OPEN_READWRITE
    The database is opened for reading and writing if possible, or reading
    only if the file is write protected by the operating system.  In either
    case the database must already exist, otherwise an error is returned.
*/
wxSharedPtr<wxSQLite3Database> mmDBWrapper::Open(const wxString &dbpath, const wxString &password)
{
    wxSharedPtr<wxSQLite3Database> db = static_db_ptr();

    int err = SQLITE_OK;
    wxString errStr=wxEmptyString;
    try
    {
        db->Open(dbpath, password);
    }
    catch (const wxSQLite3Exception& e)
    {
        //wxLogError("Database::open: %s: %s", e.GetMessage());
        //wxLogDebug("Database::open: %s: %s", e.GetMessage());
        err = e.GetErrorCode();
        errStr << e.GetMessage();
    }

    if (err==SQLITE_OK)
    {
        //Try if db selectable
        try
        {
            db->ExecuteScalar("select count(*) from  sqlite_master where type='table'");
        }
        catch (const wxSQLite3Exception &e)
        {
            err = e.GetExtendedErrorCode();
            errStr = e.GetMessage();
        }

        if (err==SQLITE_OK)
        {
            //timeout 2 sec
            db->SetBusyTimeout(2000);

            //TODO oblolete code
            if (err!=SQLITE_OK)
            {
                wxLogError(wxString::Format(_("Write error: %s"), errStr));
            }
            return (db);
        }
    }
    db->Close();
    db.reset();

    wxString s = _("When database file opening:");
    s << "\n" << wxString::Format("\n%s\n\n", dbpath);
    if (err == SQLITE_CANTOPEN)
    {
        s << _("Can't open file") <<"\n" << _("You must specify path to another database file") << "\n";
    }
    else if (err == SQLITE_NOTADB)
    {
        s << _("An incorrect password given for an encrypted file")
        << "\n\n" << _("or") << "\n\n"
        << _("Attempt to open a File that is not a database file") << "\n";
    }
    else
    {
        s << "\n" << _("Error") << err << "\n";
    }

    wxSafeShowMessage(_("Database::open: %s"), s);

    s << errStr << "\n\n" << _("Continue ?");

    wxMessageDialog msgDlg(NULL, s, _("Error"), wxYES_NO|wxICON_ERROR);
    if (msgDlg.ShowModal() == wxID_NO)
    {
        exit(err);
    }
    return db; // return a NULL database pointer
}

//----------------------------------------------------------------------------

