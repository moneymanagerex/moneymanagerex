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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ********************************************************/

#pragma once;
//----------------------------------------------------------------------------
#include <cppunit/extensions/HelperMacros.h>
//----------------------------------------------------------------------------
#include "mmOption.h"

class TestFrameBase;

class Test_DatabaseInitialisation : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(Test_DatabaseInitialisation);
    CPPUNIT_TEST(add_account_entries);
    CPPUNIT_TEST(add_payee_entries);
    CPPUNIT_TEST(add_category_entries);
    CPPUNIT_TEST(add_transaction_entries);
    CPPUNIT_TEST(add_repeat_transaction_entries);
    CPPUNIT_TEST_SUITE_END();

public:
    Test_DatabaseInitialisation();
    virtual ~Test_DatabaseInitialisation();

    void setUp();
    void tearDown();

private:
    wxString m_test_db_filename;
    wxSQLite3Database m_test_db;
    DB_Init_Model* m_dbmodel;
    TestFrameBase* m_base_frame;
    int m_this_instance;
    CommitCallbackHook* m_commit_hook;

private:
    // Test cases
    void add_account_entries();
    void add_payee_entries();
    void add_category_entries();
    void add_transaction_entries();
    void add_repeat_transaction_entries();

    void Test_Transaction_Dialog();
};
