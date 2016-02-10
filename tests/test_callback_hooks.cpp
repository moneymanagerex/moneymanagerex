/*******************************************************
Copyright (C) 2013 James Higley
Copyright (C) 2014 Stefano Giorgio

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
Foundation, Inc., 59 Temple Placeuite 330, Boston, MA  02111-1307  USA
 ********************************************************/

#include "defs.h"
#include <cppunit/config/SourcePrefix.h>
//----------------------------------------------------------------------------
#include "test_callback_hooks.h"
#include "model/Model_Currency.h"
#include "model/Model_Subcategory.h"
#include "model/Model_Category.h"
#include "model/Model_Payee.h"

/*****************************************************************************
Turn test ON or OFF in file: defined_test_selection.h
*****************************************************************************/
#include "defined_test_selection.h"

#ifdef MMEX_TESTS_CALLBACK_HOOKS
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(Test_Callback_Hooks);
#endif

static int instance_count = 0;
//----------------------------------------------------------------------------
Test_Callback_Hooks::Test_Callback_Hooks()
{
    instance_count++;
    m_test_db_filename = "test_db_callback_hooks.mmb";
}

Test_Callback_Hooks::~Test_Callback_Hooks()
{
    instance_count--;
    if (instance_count < 1)
    {
        wxRemoveFile(m_test_db_filename);
    }
}

// Common to all tests
void Test_Callback_Hooks::setUp()
{
    std::cout << "\n";
    m_test_db.Open(m_test_db_filename);

    // The test hooks area actually passed to SQLite3 by wxSQLite3
    m_test_callback = new Test_Hooks();

    m_test_db.SetCommitHook(m_test_callback);
    m_test_db.SetRollbackHook(m_test_callback);
    //m_test_db.SetUpdateHook(m_test_callback);
}

// Common to all tests
void Test_Callback_Hooks::tearDown()
{
    // need to reset the hooks before deleting them
    m_test_db.SetCommitHook(0);
    m_test_db.SetRollbackHook(0);
    //m_test_db.SetUpdateHook(0);

    m_test_db.Close();
    delete m_test_callback;
}

// Unique test
void Test_Callback_Hooks::test_Init_Base_Currency()
{
    Model_Infotable::instance(&m_test_db);
    Model_Currency& currency = Model_Currency::instance(&m_test_db);

    Model_Currency::Data* au_record = currency.GetCurrencyRecord("AUD");
    int precision = currency.precision(au_record);
    CPPUNIT_ASSERT(precision == 2);

    std::cout << "Set Base Currency: ";
    Model_Infotable::instance().SetBaseCurrency(au_record->CURRENCYID);
}

// Initialise some payees
void Test_Callback_Hooks::test_Init_Payee_Data()
{
    Model_Subcategory::instance(&m_test_db);
    Model_Category::instance(&m_test_db);
    Model_Payee::instance(&m_test_db);

    Model_Payee::Data* payee = Model_Payee::instance().create();
    payee->PAYEENAME = "Workshop";

    std::cout << "Save_Payee: ";
    Model_Payee::instance().save(payee);

    payee = Model_Payee::instance().clone(payee);
    payee->PAYEENAME = "Supermarket";
    payee->CATEGID = Model_Category::instance().get("Food")->id();
    payee->SUBCATEGID = Model_Subcategory::instance().get("Groceries", payee->CATEGID)->id();
    std::cout << "Save_Payee: ";
    Model_Payee::instance().save(payee);
}
//--------------------------------------------------------------------------

Test_Hooks::Test_Hooks()
{
    msg_header = "Test Result: Test_Model_Currency ";
    wxSQLite3Hook::wxSQLite3Hook();
}

bool Test_Hooks::CommitCallback()
{
    //wxMessageBox("COMMIT callback.", msg_header, wxOK, wxTheApp->GetTopWindow());
    std::cout << "    COMMIT callback activated.\n";

    return false;
}

void Test_Hooks::RollbackCallback()
{
    wxMessageBox("ROLLBACK callback", msg_header, wxOK, wxTheApp->GetTopWindow());
}

void Test_Hooks::UpdateCallback(wxUpdateType type, const wxString& database,
    const wxString& table, wxLongLong rowid)
{
    const char* strType;
    std::cout << "    UPDATE callback: ";

    switch (type)
    {
    case SQLITE_DELETE:
        strType = "DELETE row ";
        break;

    case SQLITE_INSERT:
        strType = "INSERT row ";
        break;

    case SQLITE_UPDATE:
        strType = "UPDATE row ";
        break;

    default:
        strType = "Unknown change row ";
        break;
    }

    std::cout << strType << (const char*) rowid.ToString().mb_str()
        << " in table " << (const char*) table.mb_str()
        << " of database " << (const char*) database.mb_str()
        << "\n";
}
//--------------------------------------------------------------------------
