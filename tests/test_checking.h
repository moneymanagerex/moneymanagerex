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

#pragma once
//----------------------------------------------------------------------------
#include <cppunit/extensions/HelperMacros.h>
//----------------------------------------------------------------------------

class TestFrameBase;

class Test_Checking : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(Test_Checking);
    CPPUNIT_TEST(Set_UP_Database_conditions);
    CPPUNIT_TEST(Add_Transactions);
    CPPUNIT_TEST(Transaction_Dialog_Messages);
    CPPUNIT_TEST(Transaction_New_Edit);
    CPPUNIT_TEST(Account_View_Savings);
    CPPUNIT_TEST(Account_View_Mary);
    CPPUNIT_TEST_SUITE_END();

public:
    Test_Checking();
    virtual ~Test_Checking();

    void setUp();
    void tearDown();

private:
    wxString m_test_db_filename;
    wxSQLite3Database m_test_db;
    DB_Model* m_dbmodel;
    TestFrameBase* m_base_frame;
    int m_this_instance;
    CommitCallbackHook* m_commit_hook;

private:
    // Test cases
    void Set_UP_Database_conditions();
    void Add_Transactions();
    void Transaction_Dialog_Messages();
    void Transaction_New_Edit();
    void Account_View_Savings();
    void Account_View_Mary();
};
