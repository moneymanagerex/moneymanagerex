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
#include "db_init_model.h"
//----------------------------------------------------------------------------
#include "test_assets.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(Test_Asset);

static int instance_count = 0;
//----------------------------------------------------------------------------
Test_Asset::Test_Asset()
{
    instance_count++;
    m_test_db_filename = "test_db_model_asset.mmb";
}

Test_Asset::~Test_Asset()
{
    instance_count--;
    if (instance_count < 1)
    {
        wxRemoveFile(m_test_db_filename);
    }
}

void Test_Asset::setUp()
{
    m_test_db.Open(m_test_db_filename);
    m_dbmodel = new DB_Init_Model();
    m_dbmodel->Init_Model_Assets(&m_test_db);
}

void Test_Asset::tearDown()
{
    m_test_db.Close();
    delete m_dbmodel;
}

void Test_Asset::test_add()
{
    wxDateTime asset_date = wxDateTime::Today().Subtract(wxDateSpan::Years(5));
    int asset_id_1 = m_dbmodel->Add_Asset("Stop Watch", asset_date, 1000, Model_Asset::TYPE_JEWELLERY, Model_Asset::RATE_APPRECIATE, 20.0, "One of 3");
    CPPUNIT_ASSERT(asset_id_1 == 1);

    int asset_id_2 = m_dbmodel->Add_Asset("Stop Watch", asset_date, 1000, Model_Asset::TYPE_JEWELLERY, Model_Asset::RATE_DEPRECIATE, 20.0, "Two of 3");
    CPPUNIT_ASSERT(asset_id_2 == 2);

    int asset_id_3 = m_dbmodel->Add_Asset("Stop Watch", asset_date, 1000, Model_Asset::TYPE_JEWELLERY, Model_Asset::RATE_NONE, 20.0, "Three of 3");
    CPPUNIT_ASSERT(asset_id_3 == 3);

    Model_Asset::Data_Set assets = Model_Asset::instance().all();
    CPPUNIT_ASSERT(assets.size() == 3);
}

void Test_Asset::test_appreciate()
{
    //TODO Finalise the test

    CPPUNIT_ASSERT_EQUAL("Test", "TODO");
}

void Test_Asset::test_depreciate()
{
    //TODO Finalise the test

    CPPUNIT_ASSERT_EQUAL("Test", "TODO");
}

void Test_Asset::test_remove()
{
    //TODO Finalise the test

    Model_Asset::instance().remove(3);
    Model_Asset::Data_Set assets = Model_Asset::instance().all();
    CPPUNIT_ASSERT(assets.size() == 2);
}
//--------------------------------------------------------------------------
