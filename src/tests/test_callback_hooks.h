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
#include <wx/intl.h>
#include <cppunit/extensions/HelperMacros.h>
//----------------------------------------------------------------------------
class Test_Hooks;

class Test_Callback_Hooks : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(Test_Callback_Hooks);
    CPPUNIT_TEST(test_Init_Base_Currency);
    CPPUNIT_TEST(test_Init_Payee_Data);
    CPPUNIT_TEST_SUITE_END();

public:
    Test_Callback_Hooks();
    virtual ~Test_Callback_Hooks();

    void setUp();
    void tearDown();

private:
    wxString m_test_db_filename;
    wxSQLite3Database m_test_db;
    Test_Hooks* m_test_callback;

private:
    void test_Init_Base_Currency();
    void test_Init_Payee_Data();
    
    /// Prevents the use of the copy constructor.
    Test_Callback_Hooks(const Test_Callback_Hooks &copy);

    /// Prevents the use of the copy operator.
    void operator =(const Test_Callback_Hooks &copy);
};
//----------------------------------------------------------------------------

class Test_Hooks : public wxSQLite3Hook
{
private:
    wxString msg_header;

public:
    Test_Hooks();

    virtual bool CommitCallback();
    virtual void RollbackCallback();
    virtual void UpdateCallback(wxUpdateType type, const wxString& database, const wxString& table, wxLongLong rowid);
};
