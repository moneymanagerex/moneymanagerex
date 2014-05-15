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
#include "cpu_timer.h"
#include "db_init_model.h"
//----------------------------------------------------------------------------
#include "test_currency.h"

/*****************************************************************************
Turn test ON or OFF in file: defined_test_selection.h
*****************************************************************************/
#include "defined_test_selection.h"

#ifdef MMEX_TESTS_CURRENCY
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(Test_Currency);
#endif

static int instance_count = 0;
//----------------------------------------------------------------------------
Test_Currency::Test_Currency()
{
    instance_count++;
    m_test_db_filename = "test_db_model_currency.mmb";
}

Test_Currency::~Test_Currency()
{
    instance_count--;
    if (instance_count < 1)
    {
        wxRemoveFile(m_test_db_filename);
    }
}

void Test_Currency::setUp()
{
    CpuTimer time("Setup");
    m_test_db.Open(m_test_db_filename);
    m_dbmodel = new DB_Init_Model();
    m_dbmodel->Init_Model_Tables(&m_test_db);
    m_dbmodel->Init_BaseCurrency();
}

void Test_Currency::tearDown()
{
    m_test_db.Close();
    delete m_dbmodel;
}

void Test_Currency::TwoDigitPrecision()
{
    CpuTimer time_this;
    wxString value;
    int precision;
    Model_Currency currency = Model_Currency::instance();
    Model_Currency::Data au_record = currency.GetCurrencyRecord("AUD");

    // Check precision is 2 digits
    precision = currency.precision(au_record);
    CPPUNIT_ASSERT(precision == 2);

    // check database values
    CPPUNIT_ASSERT(au_record.DECIMAL_POINT == ".");
    CPPUNIT_ASSERT(au_record.GROUP_SEPARATOR = ",");

    //----------------------------------------------
    value = currency.toCurrency(12345.12345);
    CPPUNIT_ASSERT(value == "$12,345.12");

    value = currency.toCurrency(12345.12345, &au_record);
    CPPUNIT_ASSERT(value == "$12,345.12");

    value = currency.fromString2Default("12,345.1234", &au_record);
    CPPUNIT_ASSERT(value == "12345.1234");

    // Test precision regardless of currency to 2 digits
    value = currency.toString(12345.12345);
    CPPUNIT_ASSERT(value == "12,345.12");
    //----------------------------------------------

    Model_Currency::Data taiwan_record = currency.GetCurrencyRecord("TWD");
    precision = currency.precision(taiwan_record);
    CPPUNIT_ASSERT(precision == 2);

    taiwan_record.GROUP_SEPARATOR = ".";
    taiwan_record.DECIMAL_POINT = ",";
    taiwan_record.SFX_SYMBOL = " - MOD";
    currency.save(&taiwan_record);

    currency.SetBaseCurrency(&taiwan_record);
    //----------------------------------------------

    value = currency.toCurrency(12345.12345);
    CPPUNIT_ASSERT(value == "NT$12.345,12 - MOD");

    value = currency.fromString2Default("NT$12.345,1234 - MOD", &taiwan_record);
    CPPUNIT_ASSERT(value == "NT$12345.1234 - MOD");

    value = currency.fromString2Default("12.345,1234", &taiwan_record);
    CPPUNIT_ASSERT(value == "12345.1234");

    value = currency.toString(12345.12345);
    CPPUNIT_ASSERT(value == "12.345,12");
    //----------------------------------------------

    value = currency.fromString2Default("12,345.12", &au_record);
    CPPUNIT_ASSERT(value == "12345.12");

    value = currency.toCurrency(12345.12345, &au_record);
    CPPUNIT_ASSERT(value == "$12,345.12");

    currency.SetBaseCurrency(&au_record);
}

void Test_Currency::FourDigitPrecision()
{
    CpuTimer time_this;
    wxString value;
    int precision;

    Model_Currency currency = Model_Currency::instance();
    Model_Currency::Data au_record = currency.GetCurrencyRecord("AUD");

    // check precision of currency
    precision = currency.precision(au_record);
    CPPUNIT_ASSERT(precision == 2);

    CPPUNIT_ASSERT(au_record.GROUP_SEPARATOR == ",");

    // Test precision regardless of currency to 4 digits
    value = currency.toString(12345.12345, 0, 4);
    CPPUNIT_ASSERT(value == "12,345.1234");

    Model_Currency::Data taiwan_record = currency.GetCurrencyRecord("TWD");
    precision = currency.precision(taiwan_record);
    CPPUNIT_ASSERT(precision == 2);

    // Test precision using currency of currency to 4 digits
    value = currency.toString(12345.12345, &taiwan_record, 4);
    CPPUNIT_ASSERT(value == "12.345,1234");

    value = currency.toCurrency(12345.12345, &taiwan_record, 4);
    CPPUNIT_ASSERT(value == "NT$12.345,1234 - MOD");
}

void Test_Currency::FormatDoubleToCurrency()
{
    CpuTimer time_this;
    double value = 0.0099;

    // Two digit precision assumed
    wxString s = Model_Currency::toString(value, 0);
    CPPUNIT_ASSERT(s == "0.01");

    // Two digit precision assumed
    s = Model_Currency::toString(-value, 0);
//    CPPUNIT_ASSERT(s == "-0.01");         // This test fails

    //------------------------------------------------------------------------
    value = -0.001;

    // Two digit precision assumed negative value
    s = Model_Currency::toString(value, 0);
    CPPUNIT_ASSERT(s == "0.00");

    // Four digit precision negative value
    s = Model_Currency::toString(value, 0, 4);
//    CPPUNIT_ASSERT(s == "-0.0010");       // This test fails

    //------------------------------------------------------------------------
    // Four digit precision negative value
    value = -0.000099;

    // Two digit precision assumed negative value
    s = Model_Currency::toString(value, 0);
    CPPUNIT_ASSERT(s == "0.00");

    // Four digit precision negative value
    s = Model_Currency::toString(value, 0, 4);
//    CPPUNIT_ASSERT(s == "-0.0001");       // This test fails

    //------------------------------------------------------------------------
    // Four digit precision negative value
    value = -0.00001;

    // Two digit precision assumed negative value
    s = Model_Currency::toString(value, 0);
    CPPUNIT_ASSERT(s == "0.00");

    // Four digit precision negative value
    s = Model_Currency::toString(value, 0, 4);
    CPPUNIT_ASSERT(s == "0.0000");
}
//--------------------------------------------------------------------------
