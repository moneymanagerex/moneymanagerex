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
//----------------------------------------------------------------------------
#include "test_model_checking.h"
#include "model/Model_Checking.h"

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

    Model_Checking::instance(&m_test_db);
}

void Test_Model_Checking::tearDown()
{
    m_test_db.SetCommitHook(0);
    m_test_db.Close();
    delete m_commit_hook;
}

void Test_Model_Checking::add_entries()
{
    Model_Checking tran = Model_Checking::instance();
    // Add a list of transactions in the database.

    Model_Checking::Data* entry;
    entry = tran.create();
    entry->TRANSCODE = tran.all_type()[Model_Checking::DEPOSIT];
    entry->TRANSAMOUNT = 1000;
    tran.save(entry);

    entry = tran.create();
    entry->TRANSCODE = tran.all_type()[Model_Checking::DEPOSIT];
    entry->TRANSAMOUNT = 2000;
    tran.save(entry);

    entry = tran.create();
    entry->TRANSCODE = tran.all_type()[Model_Checking::WITHDRAWAL];
    entry->TRANSAMOUNT = 500;
    tran.save(entry);
}

void Test_Model_Checking::test_balance()
{
    Model_Checking tran = Model_Checking::instance();
    // Add a list of transactions in the database.

    CPPUNIT_ASSERT_EQUAL("Test", "TODO");
}

void Test_Model_Checking::delete_entries()
{
    Model_Checking tran = Model_Checking::instance();
    // Add a list of transactions in the database.

    CPPUNIT_ASSERT_EQUAL("Test", "TODO");
}
//--------------------------------------------------------------------------
