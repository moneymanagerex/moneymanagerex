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
#include "mmOption.h"

class TestFrameBase;

class Test_Date_Range : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(Test_Date_Range);
    CPPUNIT_TEST(Last_Year);
    CPPUNIT_TEST(Current_Year);
    CPPUNIT_TEST(Current_Year_To_Date);
    CPPUNIT_TEST(Last_Financial_Year);
    CPPUNIT_TEST(Current_Financial_Year);
    CPPUNIT_TEST(Current_Financial_Year_To_Date);
    CPPUNIT_TEST_SUITE_END();

public:
    Test_Date_Range();
    virtual ~Test_Date_Range();

    void setUp();
    void tearDown();

private:
    TestFrameBase* m_base_frame;
    int m_this_instance;
    wxDateTime* m_start_date;

private:
    // Test cases
    void Last_Year();
    void Current_Year();
    void Current_Year_To_Date();

    void Last_Financial_Year();
    void Current_Financial_Year();
    void Current_Financial_Year_To_Date();
};
//----------------------------------------------------------------------------
