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
#include "framebase_tests.h"
//----------------------------------------------------------------------------
#include "test_date_range.h"
#include <wx/frame.h>
#include "reports/mmDateRange.h"

/*****************************************************************************
Turn test ON or OFF in file: defined_test_selection.h
*****************************************************************************/
#include "defined_test_selection.h"

#ifdef __MMEX_TESTS__TEST_DATE_RANGE
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

    // Set date to the beginning of a financial year. Assume 1st July.
    int month = m_start_date->GetMonth();
    if (month > 6)
    {
        m_start_date->Subtract(wxDateSpan::Months(month - 6));
    }
    else
    {
        m_start_date->Subtract(wxDateSpan::Year()).Add(wxDateSpan::Months(6 - month));
    }

    // Correction to 1st July from start date: Today
    m_start_date->Subtract(wxDateSpan::Days(m_start_date->GetDay() - 1));
}

void Test_Date_Range::tearDown()
{
    delete m_start_date;
}

//--------------------------------------------------------------------------

void Test_Date_Range::Last_Year()
{
    mmLastYear last_year;
    m_start_date->Add(wxDateSpan::Months(6)).Subtract(wxDateSpan::Year());
    CPPUNIT_ASSERT_EQUAL(m_start_date->FormatISODate(), last_year.start_date().FormatISODate());

    m_start_date->Add(wxDateSpan::Year()).Subtract(wxDateSpan::Day());
    CPPUNIT_ASSERT_EQUAL(m_start_date->FormatISODate(), last_year.end_date().FormatISODate());
}

void Test_Date_Range::Current_Year()
{
    mmCurrentYear current_year;
    m_start_date->Add(wxDateSpan::Months(6));
    CPPUNIT_ASSERT_EQUAL(m_start_date->FormatISODate(), current_year.start_date().FormatISODate());

    m_start_date->Add(wxDateSpan::Year()).Subtract(wxDateSpan::Day());
    CPPUNIT_ASSERT_EQUAL(m_start_date->FormatISODate(), current_year.end_date().FormatISODate());
}

void Test_Date_Range::Current_Year_To_Date()
{
    mmCurrentYearToDate current_year_to_date;
    m_start_date->Add(wxDateSpan::Months(6));
    CPPUNIT_ASSERT_EQUAL(m_start_date->FormatISODate(), current_year_to_date.start_date().FormatISODate());

    wxDateTime today(wxDateTime::Today());
    CPPUNIT_ASSERT_EQUAL(today.FormatISODate(), current_year_to_date.end_date().FormatISODate());
}

void Test_Date_Range::Last_Financial_Year()
{
    mmLastFinancialYear last_financial_year(1, 6);
    m_start_date->Subtract(wxDateSpan::Year());
    CPPUNIT_ASSERT_EQUAL(m_start_date->FormatISODate(), last_financial_year.start_date().FormatISODate());

    m_start_date->Add(wxDateSpan::Year()).Subtract(wxDateSpan::Day());
    CPPUNIT_ASSERT_EQUAL(m_start_date->FormatISODate(), last_financial_year.end_date().FormatISODate());
}

void Test_Date_Range::Current_Financial_Year()
{
    mmCurrentFinancialYear current_financial_year(1, 6);
    CPPUNIT_ASSERT_EQUAL(m_start_date->FormatISODate(), current_financial_year.start_date().FormatISODate());

    m_start_date->Add(wxDateSpan::Year()).Subtract(wxDateSpan::Day());
    CPPUNIT_ASSERT_EQUAL(m_start_date->FormatISODate(), current_financial_year.end_date().FormatISODate());
}

void Test_Date_Range::Current_Financial_Year_To_Date()
{
    mmCurrentFinancialYear current_financial_year_to_date(1, 6);
    CPPUNIT_ASSERT_EQUAL(m_start_date->FormatISODate(), current_financial_year_to_date.start_date().FormatISODate());

    wxDateTime today(wxDateTime::Today());
    CPPUNIT_ASSERT_EQUAL(today.FormatISODate(), current_financial_year_to_date.end_date().FormatISODate());
}
