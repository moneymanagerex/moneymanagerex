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
#include "cpu_timer.h"
//----------------------------------------------------------------------------
#include "test_model_currency.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(Test_Model_Currency);

static int instance_count = 0;
//----------------------------------------------------------------------------
Test_Model_Currency::Test_Model_Currency()
{
    instance_count++;
    m_test_db_filename = "test_db_model_currency.mmb";
}

Test_Model_Currency::~Test_Model_Currency()
{
    instance_count--;
    if (instance_count < 1)
    {
        wxRemoveFile(m_test_db_filename);
    }
}

void Test_Model_Currency::setUp()
{
    CpuTimer time("Setup");
    m_test_db.Open(m_test_db_filename);

    Model_Currency::instance(&m_test_db);
    Model_Infotable::instance(&m_test_db);
}

void Test_Model_Currency::tearDown()
{
    m_test_db.Close();
}

void Test_Model_Currency::test_TwoDigitPrecision()
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
    CPPUNIT_ASSERT(au_record.GROUP_SEPARATOR != ",");   // Not set correctly in database

    // confirm that group separator should be a comma
    wxString os_gs = currency.os_group_separator();
    CPPUNIT_ASSERT(os_gs == ",");

    // ensure that group separator is a comma in database.
    au_record.GROUP_SEPARATOR = os_gs;
    currency.save(&au_record);

    // check database values
    au_record = currency.GetCurrencyRecord("AUD");
    CPPUNIT_ASSERT(au_record.DECIMAL_POINT == ".");
    CPPUNIT_ASSERT(au_record.GROUP_SEPARATOR == ",");

    currency.SetBaseCurrency(&au_record);

    //----------------------------------------------

    value = currency.toCurrency(12345.12345);
    CPPUNIT_ASSERT(value == "$12,345.12");

    value = currency.toCurrency(12345.12345, &au_record);
    CPPUNIT_ASSERT(value == "$12,345.12");

    value = currency.fromString2Default("12,345.1234", &au_record);
    CPPUNIT_ASSERT(value == "12,345.1234");

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
    CPPUNIT_ASSERT(value == "NT$12,345.1234 - MOD");

    value = currency.fromString2Default("12.345,1234", &taiwan_record);
    CPPUNIT_ASSERT(value == "12,345.1234");

    value = currency.toString(12345.12345);
    CPPUNIT_ASSERT(value == "12.345,12");
    //----------------------------------------------

    value = currency.fromString2Default("12,345.12", &au_record);
    CPPUNIT_ASSERT(value == "12,345.12");

    value = currency.toCurrency(12345.12345, &au_record);
    CPPUNIT_ASSERT(value == "$12,345.12");

    currency.SetBaseCurrency(&au_record);
}

void Test_Model_Currency::test_FourDigitPrecision()
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

void Test_Model_Currency::test_FormatDoubleToCurrency()
{
    CpuTimer time_this;
    double value = 0.0099;

    wxString s = Model_Currency::toString(value, 0);
    CPPUNIT_ASSERT(s == "0.01");

    s = Model_Currency::toString(-value, 0);
    CPPUNIT_ASSERT(s == "-0.01");

    //------------------------------------------------------------------------
    value = -0.001;

    s = Model_Currency::toString(value, 0);
    CPPUNIT_ASSERT(s == "0.00");

    s = Model_Currency::toString(value, 0, 4);
    CPPUNIT_ASSERT(s == "-0.0010");

    //------------------------------------------------------------------------
    value = -0.000099;
    s = Model_Currency::toString(value, 0);
    CPPUNIT_ASSERT(s == "0.00");

    s = Model_Currency::toString(value, 0, 4);
    CPPUNIT_ASSERT(s == "-0.0001");

    //------------------------------------------------------------------------
    value = -0.00001;
    s = Model_Currency::toString(value, 0);
    CPPUNIT_ASSERT(s == "0.00");

    s = Model_Currency::toString(value, 0, 4);
    CPPUNIT_ASSERT(s == "0.0000");
}
//--------------------------------------------------------------------------
