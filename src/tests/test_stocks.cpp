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
//----------------------------------------------------------------------------
#include "test_stocks.h"
#include "model/Model_Stock.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(Test_Stock);

static int instance_count = 0;
//----------------------------------------------------------------------------
Test_Stock::Test_Stock()
{
    instance_count++;
    m_test_db_filename = "test_db_model_stock.mmb";
}

Test_Stock::~Test_Stock()
{
    instance_count--;
    if (instance_count < 1)
    {
        wxRemoveFile(m_test_db_filename);
    }
}

void Test_Stock::setUp()
{
    m_test_db.Open(m_test_db_filename);

    Model_Stock::instance(&m_test_db);
}

void Test_Stock::tearDown()
{
    m_test_db.Close();
}

void Test_Stock::test_add()
{
    Model_Stock* stock = &Model_Stock::instance();
    CPPUNIT_ASSERT(stock);
    //TODO Finalise the test

    CPPUNIT_ASSERT_EQUAL("Test", "TODO");
}
//--------------------------------------------------------------------------
