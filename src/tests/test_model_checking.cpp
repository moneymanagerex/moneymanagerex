/*******************************************************
Copyright (C) 2013 James Higley
Copyright (C) 2013 Stefano Giorgio

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
#include  "cpu_timer.h"
//----------------------------------------------------------------------------
#include "test_model_checking.h"
#include "db_init_model.h"

// Registers the fixture into the 'registry'
//CPPUNIT_TEST_SUITE_REGISTRATION(Test_Model_Checking);

static int instance_count = 0;
//----------------------------------------------------------------------------
Test_Model_Checking::Test_Model_Checking()
{
    instance_count++;
    m_test_db_filename = "test_db_model_checking.mmb";
}

Test_Model_Checking::~Test_Model_Checking()
{
    instance_count--;
    if (instance_count < 1)
    {
        wxRemoveFile(m_test_db_filename);
    }
}

void Test_Model_Checking::setUp()
{
    m_commit_hook = new CommitCallbackHook();
    m_test_db.Open(m_test_db_filename);
    m_test_db.SetCommitHook(m_commit_hook);

    m_dbmodel = new DB_Init_Model();
    m_dbmodel->Init_Model_Tables(&m_test_db);
    m_dbmodel->Init_BaseCurrency();
}

void Test_Model_Checking::tearDown()
{
    m_test_db.SetCommitHook(0);
    m_test_db.Close();
    delete m_commit_hook;
    delete m_dbmodel;
}

void Test_Model_Checking::add_entries()
{
    CpuTimer Start("Entries");

    m_dbmodel->Add_Payee("Supermarket");
    m_dbmodel->Add_Payee("Aldi");
    m_dbmodel->Add_Payee("Coles");
    m_dbmodel->Add_Payee("Woolworths");

    m_dbmodel->Add_Account("Savings", Model_Account::TYPE::CHECKING);
    m_dbmodel->Add_Account("Cheque", Model_Account::TYPE::CHECKING);
    m_dbmodel->Add_Account("Mastercard", Model_Account::TYPE::CHECKING);


    m_dbmodel->Set_AccountName("Cheque");

    m_dbmodel->Add_Trans_Deposit(wxDateTime::Today(), "Aldi", 100.0, "Income", "Salary");
    m_dbmodel->Add_Trans_Withdrawal(wxDateTime::Today(), "Coles", 20.0, "Food", "Groceries");
    m_dbmodel->Add_Trans_Transfer(wxDateTime::Today(), "Savings", 30.0, "Gifts", "", true, 40.0);
}

void Test_Model_Checking::add_entries_savings()
{
    CpuTimer Start("Entries_Savings");

    m_dbmodel->Set_AccountName("Savings");

    m_dbmodel->Add_Trans_Deposit(wxDateTime::Today(), "Aldi", 200.0, "Income", "Salary");
    m_dbmodel->Add_Trans_Withdrawal(wxDateTime::Today(), "Coles", 20.0, "Food", "Groceries");
    m_dbmodel->Add_Trans_Transfer(wxDateTime::Today(), "Savings", 30.0, "Gifts", "", true, 40.0);
}

void Test_Model_Checking::add_entries_mc()
{
    CpuTimer Start("Entries_mc");

    m_dbmodel->Set_AccountName("Mastercard");

    m_dbmodel->Add_Trans_Deposit(wxDateTime::Today(), "Aldi", 300.0, "Income", "Salary");
    m_dbmodel->Add_Trans_Withdrawal(wxDateTime::Today(), "Coles", 20.0, "Food", "Groceries");
    m_dbmodel->Add_Trans_Transfer(wxDateTime::Today(), "Savings", 30.0, "Gifts", "", true, 40.0);
}
//--------------------------------------------------------------------------
