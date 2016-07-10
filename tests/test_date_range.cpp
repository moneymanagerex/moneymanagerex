/*******************************************************
Copyright (C) 2013 James Higley
Copyright (C) 2014..2016 Stefano Giorgio

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
#include "framebase_tests.h"
//----------------------------------------------------------------------------
#include "test_date_range.h"
#include <wx/frame.h>
#include "reports/mmDateRange.h"

/*****************************************************************************
Turn test ON or OFF in file: defined_test_selection.h
*****************************************************************************/
#include "defined_test_selection.h"

#ifdef MMEX_TESTS_TEST_DATE_RANGE
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(Test_Date_Range);
#endif

static int s_instance_count = 0;
//----------------------------------------------------------------------------
Test_Date_Range::Test_Date_Range()
{
    s_instance_count++;
    m_this_instance = s_instance_count;
}

Test_Date_Range::~Test_Date_Range()
{
    s_instance_count--;
    if (s_instance_count < 1)
    {
    }
}

void Test_Date_Range::setUp()
{
    m_start_date = new wxDateTime(wxDateTime::Today());

    // Set date to the beginning of a calendar year.
    int month = m_start_date->GetMonth();
    m_start_date->Subtract(wxDateSpan::Months(month));

    // Correction to 1st day of month
    m_start_date->Subtract(wxDateSpan::Days(m_start_date->GetDay() - 1));
}

void Test_Date_Range::tearDown()
{
    delete m_start_date;
}

//--------------------------------------------------------------------------

void Test_Date_Range::Last_Year()
{
    wxDateTime* test_date = m_start_date;
    test_date->Subtract(wxDateSpan::Year());

    mmLastYear last_year;
    CPPUNIT_ASSERT_EQUAL(test_date->FormatISODate(), last_year.start_date().FormatISODate());
    test_date->Add(wxDateSpan::Year()).Subtract(wxDateSpan::Day());
    CPPUNIT_ASSERT_EQUAL(test_date->FormatISODate(), last_year.end_date().FormatISODate());
}

void Test_Date_Range::Current_Year()
{
    wxDateTime* test_date = m_start_date;

    mmCurrentYear current_year;
    CPPUNIT_ASSERT_EQUAL(test_date->FormatISODate(), current_year.start_date().FormatISODate());

    test_date->Add(wxDateSpan::Year()).Subtract(wxDateSpan::Day());
    CPPUNIT_ASSERT_EQUAL(test_date->FormatISODate(), current_year.end_date().FormatISODate());
}

void Test_Date_Range::Current_Year_To_Date()
{
    wxDateTime* test_date = m_start_date;

    mmCurrentYearToDate current_year_to_date;
    CPPUNIT_ASSERT_EQUAL(test_date->FormatISODate(), current_year_to_date.start_date().FormatISODate());

    wxDateTime today(wxDateTime::Today());
    CPPUNIT_ASSERT_EQUAL(today.FormatISODate(), current_year_to_date.end_date().FormatISODate());
}

void Test_Date_Range::Last_Financial_Year()
{
    wxDateTime* test_date = m_start_date;
    int month = wxDateTime::Today().GetMonth();
    if (month > wxDateTime::Jun)
    {
        test_date->Add(wxDateSpan::Months(6));
    }
    test_date->Subtract(wxDateSpan::Year());

    mmLastFinancialYear last_financial_year(1, 7);
    CPPUNIT_ASSERT_EQUAL(test_date->FormatISODate(), last_financial_year.start_date().FormatISODate());

    test_date->Add(wxDateSpan::Year()).Subtract(wxDateSpan::Day());
    CPPUNIT_ASSERT_EQUAL(test_date->FormatISODate(), last_financial_year.end_date().FormatISODate());
}

void Test_Date_Range::Current_Financial_Year()
{
    wxDateTime* test_date = m_start_date;
    int month = wxDateTime::Today().GetMonth();
    if (month > wxDateTime::Jun)
    {
        test_date->Add(wxDateSpan::Months(6));
    }

    mmCurrentFinancialYear current_financial_year(1, 7);
    CPPUNIT_ASSERT_EQUAL(test_date->FormatISODate(), current_financial_year.start_date().FormatISODate());

    test_date->Add(wxDateSpan::Year()).Subtract(wxDateSpan::Day());
    CPPUNIT_ASSERT_EQUAL(test_date->FormatISODate(), current_financial_year.end_date().FormatISODate());
}

void Test_Date_Range::Current_Financial_Year_To_Date()
{
    wxDateTime* test_date = m_start_date;
    int month = wxDateTime::Today().GetMonth();
    if (month > wxDateTime::Jun)
    {
        test_date->Add(wxDateSpan::Months(6));
    }

    mmCurrentFinancialYearToDate current_financial_year_to_date(1, 7);
    CPPUNIT_ASSERT_EQUAL(test_date->FormatISODate(), current_financial_year_to_date.start_date().FormatISODate());

    wxDateTime today(wxDateTime::Today());
    CPPUNIT_ASSERT_EQUAL(today.FormatISODate(), current_financial_year_to_date.end_date().FormatISODate());
}
