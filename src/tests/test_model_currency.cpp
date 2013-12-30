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

#include "test_model_currency.h"
#include "dbwrapper.h"
#include "model/Model_Currency.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(Test_Model_Currency);

static int instance_count = 0;

Test_Model_Currency::Test_Model_Currency()
{
    instance_count++;
    test_db_filename = "cppunit_test_database.mmb";
}

Test_Model_Currency::~Test_Model_Currency()
{
}

void Test_Model_Currency::setUp()
{
    m_test_db = mmDBWrapper::Open(test_db_filename);

    // Using separate initialisation for debugging purposes.
    int shared_ptr_useage_count = m_test_db.use_count();
    if (shared_ptr_useage_count == 2)
    {
        // initialize the model. Once only
        Model_Currency currency = Model_Currency::instance(m_test_db.get());
    }
}

void Test_Model_Currency::tearDown()
{
//    m_test_db->Close();
}

void Test_Model_Currency::test_TwoDigitPrecision()
{
    Model_Currency currency = Model_Currency::instance();
    wxString value = currency.toString(3.1415926, 0, 2);
    CPPUNIT_ASSERT(value == "3.14");
}

void Test_Model_Currency::test_FourDigitPrecision()
{
    Model_Currency currency = Model_Currency::instance();
    wxString value = currency.toString(3.1415926, 0, 4);
    CPPUNIT_ASSERT(value == "3.1416");
}

void Test_Model_Currency::test_Currency_AUD()
{
    Model_Currency currency = Model_Currency::instance();

    // Locate the AU record
    Model_Currency::Data au_record;
    for (const auto& currency_record : currency.all())
    {
        if (currency_record.CURRENCY_SYMBOL == "AUD")
        {
            au_record = currency_record;
        }
    }

    wxString value = currency.toString(10003.1415926, &au_record, 4);
    CPPUNIT_ASSERT(value == "10,003.1416");
        
    value = currency.toString(10003.1415926, &au_record, 2);
    CPPUNIT_ASSERT(value == "10,003.14");

    au_record.PFX_SYMBOL = "$";
    au_record.save(m_test_db.get());

    value = currency.fromString("$1,003.1416", &au_record);
    CPPUNIT_ASSERT(value == "1,003.1416");

    au_record.PFX_SYMBOL = "AU $";
    au_record.save(m_test_db.get());

    value = currency.toCurrency(1003.14, &au_record);
    CPPUNIT_ASSERT(value == "AU $1,003.14");

    value = currency.toCurrency(1003.1416, &au_record);
    CPPUNIT_ASSERT(value == "AU $1,003.1416");
}
