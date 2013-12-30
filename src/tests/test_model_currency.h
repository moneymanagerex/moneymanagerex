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

class Test_Model_Currency : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(Test_Model_Currency);
    CPPUNIT_TEST(test_TwoDigitPrecision);
    CPPUNIT_TEST(test_FourDigitPrecision);
    CPPUNIT_TEST(test_Currency_AUD);
    CPPUNIT_TEST_SUITE_END();

public:
    Test_Model_Currency();

    virtual ~Test_Model_Currency();

    void setUp();
    void tearDown();

    void test_TwoDigitPrecision();
    void test_FourDigitPrecision();
    void test_Currency_AUD();

private:
    /// Prevents the use of the copy constructor.
    Test_Model_Currency(const Test_Model_Currency &copy);

    /// Prevents the use of the copy operator.
    void operator =(const Test_Model_Currency &copy);

    wxString test_db_filename;
    wxSharedPtr<wxSQLite3Database> m_test_db;
    wxLocale locale_;
};
