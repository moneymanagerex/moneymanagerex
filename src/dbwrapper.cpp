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

namespace
{

const double g_defBASECONVRATE = 1.0;

//----------------------------------------------------------------------------

void insertCategoryTree(wxSQLite3Database* db,
                   wxSQLite3Statement &cat,
                   wxSQLite3Statement &subcat,
                   const wxString &categoryName,
                   const char* subcats[] // must be NULL or ends with NULL
                  )
{
    wxASSERT(db);

    bool ok = cat.GetParamCount() == 1;
    wxASSERT(ok);

    cat.Bind(1, wxGetTranslation(categoryName));
    cat.ExecuteUpdate();
    cat.Reset();

    if (!subcats)
        return;

    wxLongLong catId = db->GetLastRowId(); // id of newly inserted categoryName

    ok = subcat.GetParamCount() == 2;
    wxASSERT(ok);

    int name_idx = subcat.GetParamIndex(":name");
    wxASSERT(name_idx);

    int id_idx = subcat.GetParamIndex(":id");
    wxASSERT(id_idx);

    subcat.Bind(id_idx, catId);

    for (size_t i = 0; subcats[i]; ++i)
    {
        wxString subcateg = wxString::Format("%s", (subcats[i]));
        subcat.Bind(name_idx, wxGetTranslation(subcateg));
        subcat.ExecuteUpdate();
        subcat.Reset();

        wxASSERT(i < 50); // subcats must ends with 0
    }
}
//----------------------------------------------------------------------------

void createDefaultCategories(wxSQLite3Database* db)
{
    try
    {
        wxASSERT(db);

        wxSQLite3Statement st_cat = db->PrepareStatement(INSERT_INTO_CATEGORY_V1);

        wxSQLite3Statement st_subcat = db->PrepareStatement(INSERT_ROW_INTO_SUBCATEGORY_V1);

        const char* BillsCategories[] = {
            (char *)wxTRANSLATE("Telephone"), (char *)wxTRANSLATE("Electricity"),
            (char *)wxTRANSLATE("Gas"), (char *)wxTRANSLATE("Internet"),
            (char *)wxTRANSLATE("Rent"), (char *)wxTRANSLATE("Cable TV"),
            (char *)wxTRANSLATE("Water"), 0
        };
        insertCategoryTree(db, st_cat, st_subcat, wxTRANSLATE("Bills"), BillsCategories);

        const char* FoodCategories[] = {
        (char *)wxTRANSLATE("Groceries"), (char *)wxTRANSLATE("Dining out"), 0};
        insertCategoryTree(db, st_cat, st_subcat, wxTRANSLATE("Food"), FoodCategories);

        const char* LeisureCategories[] = {
            (char *)wxTRANSLATE("Movies"), (char *)wxTRANSLATE("Video Rental"),
            (char *)wxTRANSLATE("Magazines"), 0};
        insertCategoryTree(db, st_cat, st_subcat, wxTRANSLATE("Leisure"), LeisureCategories);

        const char* AutomobileCategories[] = {
            (char *)wxTRANSLATE("Maintenance"), (char *)wxTRANSLATE("Gas"),
            (char *)wxTRANSLATE("Parking"), (char *)wxTRANSLATE("Registration"), 0};
        insertCategoryTree(db, st_cat, st_subcat, wxTRANSLATE("Automobile"), AutomobileCategories);

        const char* EducationCategories[] = {
            (char *)wxTRANSLATE("Books"),
            (char *)wxTRANSLATE("Tuition"), (char *)wxTRANSLATE("Others"), 0};
        insertCategoryTree(db, st_cat, st_subcat, wxTRANSLATE("Education"), EducationCategories);

        const char* HomeneedsCategories[] = {
            (char *)wxTRANSLATE("Clothing"),
            (char *)wxTRANSLATE("Furnishing"), (char *)wxTRANSLATE("Others"), 0};
        insertCategoryTree(db, st_cat, st_subcat, wxTRANSLATE("Homeneeds"), HomeneedsCategories);

        const char* HealthcareCategories[] = {
            (char *)wxTRANSLATE("Health"),
            (char *)wxTRANSLATE("Dental"), (char *)wxTRANSLATE("Eyecare"),
            (char *)wxTRANSLATE("Physician"), (char *)wxTRANSLATE("Prescriptions"), 0};
        insertCategoryTree(db, st_cat, st_subcat, wxTRANSLATE("Healthcare"), HealthcareCategories);

        const char* InsuranceCategories[] = {
            (char *)wxTRANSLATE("Auto"), (char *)wxTRANSLATE("Life"),
            (char *)wxTRANSLATE("Home"), (char *)wxTRANSLATE("Health"), 0};
        insertCategoryTree(db, st_cat, st_subcat, wxTRANSLATE("Insurance"), InsuranceCategories);

        const char* VacationCategories[] = {
            (char *)wxTRANSLATE("Travel"), (char *)wxTRANSLATE("Lodging"),
            (char *)wxTRANSLATE("Sightseeing"), 0};
        insertCategoryTree(db, st_cat, st_subcat, wxTRANSLATE("Vacation"), VacationCategories);

        const char* TaxesCategories[] = {
            (char *)wxTRANSLATE("Income Tax"), (char *)wxTRANSLATE("House Tax"),
            (char *)wxTRANSLATE("Water Tax"), (char *)wxTRANSLATE("Others"), 0};
        insertCategoryTree(db, st_cat, st_subcat, wxTRANSLATE("Taxes"), TaxesCategories);

        insertCategoryTree(db, st_cat, st_subcat, wxTRANSLATE("Miscellaneous"), 0);
        insertCategoryTree(db, st_cat, st_subcat, wxTRANSLATE("Gifts"), 0);

        const char* IncomeCategories[] = {
            (char *)wxTRANSLATE("Salary"), (char *)wxTRANSLATE("Reimbursement/Refunds"),
            (char *)wxTRANSLATE("Investment Income"), 0};
        insertCategoryTree(db, st_cat, st_subcat, wxTRANSLATE("Income"), IncomeCategories);

        insertCategoryTree(db, st_cat, st_subcat, wxTRANSLATE("Other Income"), 0);
        insertCategoryTree(db, st_cat, st_subcat, wxTRANSLATE("Other Expenses"), 0);
        insertCategoryTree(db, st_cat, st_subcat, wxTRANSLATE("Transfer"), 0);

        // cleanup
        st_subcat.Finalize();
        st_cat.Finalize();
    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogDebug("Database::createDefaultCategories: %s", e.GetMessage());
        wxLogError("insert into [SUB]CATEGORY_V1. " + wxString::Format(_("Error: %s"), e.GetMessage()));
    }
}
//----------------------------------------------------------------------------

} // namespace

//----------------------------------------------------------------------------

int mmDBWrapper::createTable(wxSQLite3Database* db, const wxString &sTableName, const wxString &sql)
{
    int iError = 0;
    try
    {
        bool valid = db->TableExists(sTableName);
        if (!valid)
        {
            db->ExecuteUpdate(sql);
            valid = db->TableExists(sTableName);
            wxASSERT(valid);
            if (sTableName == "SUBCATEGORY_V1") createDefaultCategories(db);
        }
    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogDebug("Create table %s Error: %s", sTableName, e.GetMessage());
        iError = e.GetErrorCode();
    }
    return iError;
}

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

void removeCruft(wxSQLite3Database* db)
{
    try
    {
        if (db->TableExists("SPLITTRANSACTIONS_V1"))
        {
            if (db->ExecuteScalar("SELECT COUNT(*) FROM SPLITTRANSACTIONS_V1 WHERE SPLITTRANSACTIONS_V1.TRANSID NOT IN (SELECT TRANSID FROM CHECKINGACCOUNT_V1)") > 0)
                db->ExecuteUpdate("DELETE FROM SPLITTRANSACTIONS_V1 WHERE SPLITTRANSACTIONS_V1.TRANSID NOT IN (SELECT TRANSID FROM CHECKINGACCOUNT_V1)");
        }

        if (db->TableExists("BUDGETSPLITTRANSACTIONS_V1"))
        {
            if (db->ExecuteScalar("SELECT COUNT(*) FROM BUDGETSPLITTRANSACTIONS_V1 WHERE BUDGETSPLITTRANSACTIONS_V1.TRANSID NOT IN (SELECT BDID FROM BILLSDEPOSITS_V1)") > 0)
                db->ExecuteUpdate("DELETE FROM BUDGETSPLITTRANSACTIONS_V1 WHERE BUDGETSPLITTRANSACTIONS_V1.TRANSID NOT IN (SELECT BDID FROM BILLSDEPOSITS_V1)");
        }

#if 0
        {
            wxSQLite3StatementBuffer bufSQL3;
            bufSQL3.Format("UPDATE CHECKINGACCOUNT_V1 SET CATEGID = -1, SUBCATEGID=-1  WHERE CHECKINGACCOUNT_V1.TRANSID IN (SELECT SPLITTRANSACTIONS_V1.TRANSID FROM SPLITTRANSACTIONS_V1);");
            db->ExecuteUpdate(bufSQL3);
        }

        {
            wxSQLite3StatementBuffer bufSQL3;
            bufSQL3.Format("UPDATE BUDGETSPLITTRANSACTIONS_V1 SET CATEGID = -1, SUBCATEGID=-1  WHERE BUDGETSPLITTRANSACTIONS_V1.TRANSID IN (SELECT BUDGETSPLITTRANSACTIONS_V1.TRANSID FROM BUDGETSPLITTRANSACTIONS_V1);");
            db->ExecuteUpdate(bufSQL3);
        }
#endif
    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogDebug("Database::removeCruft: %s", e.GetMessage());
        wxLogError("Remove Cruft. " + wxString::Format(_("Error: %s"), e.GetMessage()));
    }
}

/*
    This routine sets up a new DB as well as update an old one
*/

void mmDBWrapper::initDB(wxSQLite3Database* db)
{
    db->Begin();

    for (size_t i = 0; i < sizeof(TABLE_NAMES)/sizeof(wxString); ++i)
    {
        createTable(db, TABLE_NAMES[i], CREATE_TABLE_SQL()[i]);
    }

    /* Create AllData view */
    createAllDataView(db);

    /* Remove Any cruft */
    removeCruft(db);
    db->Commit();
}

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

bool mmDBWrapper::deleteCategoryWithConstraints(wxSQLite3Database* db, int categID)
{
    try
    {
        static const char sql[] =
        "select 1 "
        "from ( select CATEGID from CHECKINGACCOUNT_V1 "
               "union all "
               "select CATEGID from SPLITTRANSACTIONS_V1 "
               "union all "
               "select CATEGID from BILLSDEPOSITS_V1 "
               "union all "
               "select CATEGID from BUDGETSPLITTRANSACTIONS_V1 "
             ") "
        "where CATEGID = ? "
        "limit 1"; // return only one row

        {
            wxSQLite3Statement st = db->PrepareStatement(sql);
            st.Bind(1, categID);

            wxSQLite3ResultSet q1 = st.ExecuteQuery();
            bool found = q1.NextRow();
            st.Finalize();

            if (found)
            {
                return false;
            }
        }

        // --

        static const char* sql_del[] =
        {
            "DELETE FROM SUBCATEGORY_V1     WHERE CATEGID = ?",
            "DELETE FROM CATEGORY_V1        WHERE CATEGID = ?",
            "DELETE FROM BUDGETTABLE_V1     WHERE CATEGID = ?",
            "UPDATE PAYEE_V1 SET CATEGID=-1 WHERE CATEGID = ?",
            0
        };

        db->Begin();
        for (int i = 0; sql_del[i]; ++i)
        {
            wxSQLite3Statement st = db->PrepareStatement(sql_del[i]);
            st.Bind(1, categID);
            st.ExecuteUpdate();
            st.Finalize();
        }
        db->Commit();

        mmOptions::instance().databaseUpdated_ = true;
        return true;
    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogDebug("Database::deleteCategoryWithConstraints: %s", e.GetMessage());
        wxLogError("Delete Category with Constraints. " + wxString::Format(_("Error: %s"), e.GetMessage()));
        return false;
    }
}

bool mmDBWrapper::deleteSubCategoryWithConstraints(wxSQLite3Database* db, int categID, int subcategID)
{
    try
    {
        static const char sql[] =
        "select 1 "
        "from ( select CATEGID, SUBCATEGID from CHECKINGACCOUNT_V1 "
               "union all "
               "select CATEGID, SUBCATEGID from SPLITTRANSACTIONS_V1 "
               "union all "
               "select CATEGID, SUBCATEGID from BILLSDEPOSITS_V1 "
               "union all "
               "select CATEGID, SUBCATEGID from BUDGETSPLITTRANSACTIONS_V1 "
             ") "
        "where CATEGID = ? and SUBCATEGID = ? "
        "limit 1"; // return only one row

        wxSQLite3Statement st = db->PrepareStatement(sql);
        st.Bind(1, categID);
        st.Bind(2, subcategID);

        wxSQLite3ResultSet q1 = st.ExecuteQuery();
        bool found = q1.NextRow();
        st.Finalize();

        if (found)
        {
            return false;
        }

        // --

        static const char* sql_del[] =
        {
            "DELETE FROM SUBCATEGORY_V1 WHERE CATEGID=? AND SUBCATEGID=?",
            "DELETE FROM BUDGETTABLE_V1 WHERE CATEGID=? AND SUBCATEGID=?",

            "UPDATE PAYEE_V1 SET CATEGID = -1, SUBCATEGID = -1 "
                                       "WHERE CATEGID=? AND SUBCATEGID=?",
            0
        };

        db->Begin();
        for (int i = 0; sql_del[i]; ++i)
        {
            wxSQLite3Statement st = db->PrepareStatement(sql_del[i]);
            st.Bind(1, categID);
            st.Bind(2, subcategID);

            st.ExecuteUpdate();
            st.Finalize();
        }
        db->Commit();

        mmOptions::instance().databaseUpdated_ = true;
        return true;
    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogDebug("Database::deleteSubCategoryWithConstraints: %s", e.GetMessage());
        wxLogError("Delete SubCategory with Constraints. " + wxString::Format(_("Error: %s"), e.GetMessage()));
        return false;
    }
}

bool mmDBWrapper::updateCategory(wxSQLite3Database* db, int categID,
                                 int subcategID, const wxString &newName)
{
    try
    {
        if (subcategID == -1)
        {
            wxSQLite3Statement st = db->PrepareStatement(UPDATE_CATEGORY_V1);

            st.Bind(1, newName);
            st.Bind(2, categID);

            st.ExecuteUpdate();
            st.Finalize();
        }
        else
        {
            wxSQLite3Statement st = db->PrepareStatement(UPDATE_SUBCATEGORY_V1);

            st.Bind(1, newName);
            st.Bind(2, subcategID);

            st.ExecuteUpdate();
            st.Finalize();
        }

        mmOptions::instance().databaseUpdated_ = true;
    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogDebug("Database::updateCategory: %s", e.GetMessage());
        wxLogError("update [SUB]CATEGORY_V1. " + wxString::Format(_("Error: %s"), e.GetMessage()));
    }
    return true;

}

bool mmDBWrapper::addCategory(wxSQLite3Database* db, const wxString &newName)
{
    int rows_affected = 0;

    try
    {
        wxSQLite3Statement st = db->PrepareStatement(INSERT_INTO_CATEGORY_V1);
        st.Bind(1, newName);
        rows_affected = st.ExecuteUpdate();
        st.Finalize();
        mmOptions::instance().databaseUpdated_ = true;

    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogDebug("Database::addCategory: %s", e.GetMessage());
        wxLogError("insert into CATEGORY_V1. " + wxString::Format(_("Error: %s"), e.GetMessage()));
    }
    return rows_affected == 1;
}

bool mmDBWrapper::addSubCategory(wxSQLite3Database* db, int categID, const wxString &newName)
{
    static const char sql[] =
    "insert into SUBCATEGORY_V1 (SUBCATEGNAME, CATEGID) values (?, ?)";

    int rows_affected = 0;

    try
    {
        wxSQLite3Statement st = db->PrepareStatement(sql);
        st.Bind(1, newName);
        st.Bind(2, categID);

        rows_affected = st.ExecuteUpdate();
        st.Finalize();

    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogDebug("Database::addSubCategory: %s", e.GetMessage());
        wxLogError("insert into SUBCATEGORY_V1. " + wxString::Format(_("Error: %s"), e.GetMessage()));
    }
    mmOptions::instance().databaseUpdated_ = true;
    return rows_affected == 1;
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

int mmDBWrapper::relocatePayee(wxSQLite3Database* db, int destPayeeID, int sourcePayeeID, int &changed)
{
    int err = SQLITE_OK;
    changed = 0;

    wxSQLite3Statement st = db->PrepareStatement(SET_PAYEEID_CHECKINGACCOUNT_V1);
    st.Bind(1, destPayeeID);
    st.Bind(2, sourcePayeeID);

    wxSQLite3Statement stBD = db->PrepareStatement(SET_PAYEEID_BILLSDEPOSITS_V1);
    stBD.Bind(1, destPayeeID);
    stBD.Bind(2, sourcePayeeID);

    try
    {
        changed = st.ExecuteUpdate();
        changed += stBD.ExecuteUpdate();

        st.Finalize();
        stBD.Finalize();
    }
    catch(const wxSQLite3Exception& e)
    {
        err = e.GetExtendedErrorCode();
        wxLogDebug("update checkingaccount_v1 : %s", e.GetMessage());
        wxLogError(wxString::Format(_("Error: %s"), e.GetMessage()));
    }
    return err;
}

int mmDBWrapper::relocateCategory(wxSQLite3Database* db,
    int destCatID, int destSubCatID, int sourceCatID, int sourceSubCatID,
    int &changedCat, int &changedSubCat)
{
    int err = SQLITE_OK;
    changedCat = 0;
    changedSubCat = 0;

    wxSQLite3Statement stCat = db->PrepareStatement(SET_CATEGID_CHECKINGACCOUNT_V1);
    stCat.Bind(1, destCatID);
    stCat.Bind(2, destSubCatID);
    stCat.Bind(3, sourceCatID);
    stCat.Bind(4, sourceSubCatID);

    wxSQLite3Statement stSubCat = db->PrepareStatement(SET_CATEGID_SPLITTRANSACTIONS_V1);
    stSubCat.Bind(1, destCatID);
    stSubCat.Bind(2, destSubCatID);
    stSubCat.Bind(3, sourceCatID);
    stSubCat.Bind(4, sourceSubCatID);

    wxSQLite3Statement stBDCat = db->PrepareStatement(SET_CATEGID_BILLSDEPOSITS_V1);
    stBDCat.Bind(1, destCatID);
    stBDCat.Bind(2, destSubCatID);
    stBDCat.Bind(3, sourceCatID);
    stBDCat.Bind(4, sourceSubCatID);

    wxSQLite3Statement stBDSubCat = db->PrepareStatement(SET_CATEGID_BUDGETSPLITTRANSACTIONS_V1);
    stBDSubCat.Bind(1, destCatID);
    stBDSubCat.Bind(2, destSubCatID);
    stBDSubCat.Bind(3, sourceCatID);
    stBDSubCat.Bind(4, sourceSubCatID);

    try
    {
        changedCat = stCat.ExecuteUpdate();
        changedSubCat = stSubCat.ExecuteUpdate();
        changedCat += stBDCat.ExecuteUpdate();
        changedSubCat += stBDSubCat.ExecuteUpdate();

        stCat.Finalize();
        stSubCat.Finalize();
        stBDCat.Finalize();
        stBDSubCat.Finalize();
    }
    catch(const wxSQLite3Exception& e)
    {
        err = e.GetExtendedErrorCode();
        wxLogDebug("update checkingaccount_v1 : %s", e.GetMessage());
        wxLogError(wxString::Format(_("Error: %s"), e.GetMessage()));
    }
    return err;
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

void mmDBWrapper::completeBDInSeries(wxSQLite3Database* db, int bdID)
{
    try
    {
        static const char sql[] =
        "SELECT NUMOCCURRENCES, "
               "NEXTOCCURRENCEDATE, "
               "REPEATS "
        "FROM BILLSDEPOSITS_V1 "
        "WHERE BDID = ?";
        // Removed "date(NEXTOCCURRENCEDATE, 'localtime') as NEXTOCCURRENCEDATE, "
        // because causing problems with some systems and in different time zones

        wxDateTime updateOccur = wxDateTime::Now();
        int numRepeats  = -1;

        wxSQLite3Statement st = db->PrepareStatement(sql);
        st.Bind(1, bdID);

        wxSQLite3ResultSet q1 = st.ExecuteQuery();

        if (q1.NextRow())
        {
            wxDateTime dtno = q1.GetDate("NEXTOCCURRENCEDATE");
            updateOccur = dtno;

            int repeats = q1.GetInt("REPEATS");

            // DeMultiplex the Auto Executable fields.
            if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute User Acknowlegement required
                repeats -= BD_REPEATS_MULTIPLEX_BASE;
            if (repeats >= BD_REPEATS_MULTIPLEX_BASE)    // Auto Execute Silent mode
                repeats -= BD_REPEATS_MULTIPLEX_BASE;

            numRepeats = q1.GetInt("NUMOCCURRENCES");
            if (numRepeats != -1)
            {
                if ((repeats < 11) || (repeats > 14)) --numRepeats;
            }

            if (repeats == 0)
            {
                numRepeats = 0;
            }
            else if (repeats == 1)
            {
                updateOccur = dtno.Add(wxTimeSpan::Week());
            }
            else if (repeats == 2)
            {
                updateOccur = dtno.Add(wxTimeSpan::Weeks(2));
            }
            else if (repeats == 3)
            {
                updateOccur = dtno.Add(wxDateSpan::Month());
            }
            else if (repeats == 4)
            {
                updateOccur = dtno.Add(wxDateSpan::Months(2));
            }
            else if (repeats == 5)
            {
                updateOccur = dtno.Add(wxDateSpan::Months(3));
            }
            else if (repeats == 6)
            {
                updateOccur = dtno.Add(wxDateSpan::Months(6));
            }
            else if (repeats == 7)
            {
                updateOccur = dtno.Add(wxDateSpan::Year());
            }
            else if (repeats == 8)
            {
                updateOccur = dtno.Add(wxDateSpan::Months(4));
            }
            else if (repeats == 9)
            {
                updateOccur = dtno.Add(wxDateSpan::Weeks(4));
            }
            else if (repeats == 10)
            {
                updateOccur = dtno.Add(wxDateSpan::Days(1));
            }
            else if ((repeats == 11) || (repeats == 12))
            {
                if (numRepeats != -1) numRepeats = -1;
            }
            else if (repeats == 13)
            {
                if (numRepeats > 0) updateOccur = dtno.Add(wxDateSpan::Days(numRepeats));
            }
            else if (repeats == 14)
            {
                if (numRepeats > 0) updateOccur = dtno.Add(wxDateSpan::Months(numRepeats));
            }
            else if ((repeats == 15) || (repeats == 16))
            {
                updateOccur = dtno.Add(wxDateSpan::Month());
                updateOccur = updateOccur.SetToLastMonthDay(updateOccur.GetMonth(),updateOccur.GetYear());
                if (repeats == 16) // last weekday of month
                {
                    if (updateOccur.GetWeekDay() == wxDateTime::Sun || updateOccur.GetWeekDay() == wxDateTime::Sat)
                        updateOccur.SetToPrevWeekDay(wxDateTime::Fri);
                }
            }
        }

        st.Finalize();

        {
            static const char UPDATE_BILLSDEPOSITS_V1[] =
            "UPDATE BILLSDEPOSITS_V1 "
            "set NEXTOCCURRENCEDATE = ?, "
            "NUMOCCURRENCES = ? "
            "where BDID = ?";

            wxSQLite3Statement st = db->PrepareStatement(UPDATE_BILLSDEPOSITS_V1);

            st.Bind(1, updateOccur.FormatISODate());
            st.Bind(2, numRepeats);
            st.Bind(3, bdID);

            st.ExecuteUpdate();
            st.Finalize();
        }
        db->ExecuteUpdate("DELETE FROM BILLSDEPOSITS_V1 where NUMOCCURRENCES = 0");
        mmOptions::instance().databaseUpdated_ = true;

    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogDebug("Function::completeBDInSeries: %s", e.GetMessage());
        wxLogError("Complete BD In Series. " + wxString::Format(_("Error: %s"), e.GetMessage()));
    }
}

void mmDBWrapper::deleteStockInvestment(wxSQLite3Database* db, int stockID)
{
    try
    {
        wxSQLite3Statement st = db->PrepareStatement("DELETE FROM STOCK_V1 where STOCKID = ?");
        st.Bind(1, stockID);
        st.ExecuteUpdate();
        st.Finalize();
        mmOptions::instance().databaseUpdated_ = true;

    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogDebug("Function::deleteStockInvestment: %s", e.GetMessage());
        wxLogError("Delete Stock Investment. " + wxString::Format(_("Error: %s"), e.GetMessage()));
    }
}

bool mmDBWrapper::moveStockInvestment(wxSQLite3Database* db, int stockID, int toAccountID)
{
    try
    {
        wxSQLite3Statement st = db->PrepareStatement("UPDATE STOCK_V1 SET HELDAT = ? where STOCKID = ?");
        st.Bind(1, toAccountID);
        st.Bind(2, stockID);
        st.ExecuteUpdate();
        st.Finalize();
        mmOptions::instance().databaseUpdated_ = true;

    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogDebug("Function::moveStockInvestment: %s", e.GetMessage());
        wxLogError("Delete Stock Investment. " + wxString::Format(_("Error: %s"), e.GetMessage()));
        return false;
    }
    return true;
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

bool mmDBWrapper::deleteCurrency(wxSQLite3Database* db, int currencyID)
{
    try
    {
        wxSQLite3Statement st = db->PrepareStatement(DELETE_CURRENCYID_FROM_CURRENCYFORMATS_V1);
        st.Bind(1, currencyID);
        st.ExecuteUpdate();
        st.Finalize();

        mmOptions::instance().databaseUpdated_ = true;
    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogError(wxString::Format(_("Error: %s"), e.GetMessage()));
        return false;
    }
    return true;
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

int mmDBWrapper::mmSQLiteExecuteUpdate(wxSQLite3Database* db, const std::vector<wxString>& data, const wxString& sql, long &lLastRowId)
{
    int iError = 0;
    try
    {
        wxSQLite3Statement st = db->PrepareStatement(sql);
        int i = 0;
        for (std::vector<wxString>::const_iterator d = data.begin(); d != data.end(); ++d)
            st.Bind(++i, *d);

        st.ExecuteUpdate();
        st.Finalize();
        lLastRowId = db->GetLastRowId().ToLong();

    }
    catch(const wxSQLite3Exception& e)
    {
        wxLogDebug("Function::mmSQLiteExecuteUpdate: %s", e.GetMessage());
        wxLogError("mmSQLiteExecuteUpdate. " + wxString::Format(_("Error: %s"), e.GetMessage()));
        iError = e.GetErrorCode();
    }

    return iError;
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

const wxArrayString CREATE_TABLE_SQL()
{
    wxArrayString q;
    q.Add(wxString::FromUTF8(CREATE_TABLE_ASSETS_V1));
    q.Add(wxString::FromUTF8(CREATE_TABLE_ACCOUNTLIST_V1));
    q.Add(wxString::FromUTF8(CREATE_TABLE_CHECKINGACCOUNT_V1));
    q.Add(wxString::FromUTF8(CREATE_TABLE_PAYEE_V1));
    q.Add(wxString::FromUTF8(CREATE_TABLE_BILLSDEPOSITS_V1));
    q.Add(wxString::FromUTF8(CREATE_TABLE_STOCK_V1));
    q.Add(wxString::FromUTF8(CREATE_TABLE_SPLITTRANSACTIONS_V1));
    q.Add(wxString::FromUTF8(CREATE_TABLE_BUDGETSPLITTRANSACTIONS_V1));
    q.Add(wxString::FromUTF8(CREATE_TABLE_CATEGORY_V1));
    q.Add(wxString::FromUTF8(CREATE_TABLE_SUBCATEGORY_V1));
    q.Add(wxString::FromUTF8(CREATE_TABLE_BUDGETYEAR_V1));
    q.Add(wxString::FromUTF8(CREATE_TABLE_BUDGETTABLE_V1));
    q.Add(wxString::FromUTF8(CREATE_TABLE_CURRENCYFORMATS_V1));
    return q;
};

