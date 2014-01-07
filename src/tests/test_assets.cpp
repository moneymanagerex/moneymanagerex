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
#include "test_assets.h"
#include "model/Model_Asset.h"

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

    Model_Asset::instance(&m_test_db);
}

void Test_Asset::tearDown()
{
    m_test_db.Close();
}

void Test_Asset::test_add()
{
    Model_Asset asset = Model_Asset::instance();

    Model_Asset::Data* entry = asset.create();
    CPPUNIT_ASSERT(entry->id() == -1);

    entry->ASSETNAME = "Stop watch";
    entry->ASSETTYPE = Model_Asset::all_type()[Model_Asset::TYPE_JEWELLERY];
    wxDateTime date = wxDateTime::Today().Subtract(wxDateSpan::Years(2));
    entry->STARTDATE = date.FormatISODate();
    entry->VALUE = 2000;
    entry->VALUECHANGE = Model_Asset::all_rate()[Model_Asset::RATE_NONE];
    entry->VALUECHANGERATE = 20.00;
    entry->NOTES = "One of 3";
    int asset_id = asset.save(entry);
    CPPUNIT_ASSERT(asset_id == 1);

    entry = asset.clone(entry);
    CPPUNIT_ASSERT(entry->id() == -1);
    entry->VALUECHANGE = Model_Asset::all_rate()[Model_Asset::RATE_APPRECIATE];
    entry->NOTES = "two of 3";
    asset_id = asset.save(entry);
    CPPUNIT_ASSERT(asset_id == 2);

    entry = asset.clone(entry);
    CPPUNIT_ASSERT(entry->id() == -1);
    entry->VALUECHANGE = Model_Asset::all_rate()[Model_Asset::RATE_DEPRECIATE];
    entry->NOTES = "Three of 3";
    asset_id = asset.save(entry);
    CPPUNIT_ASSERT(asset_id == 3);
}

void Test_Asset::test_appreciate()
{
    Model_Asset asset = Model_Asset::instance();
    //TODO Finalise the test

    CPPUNIT_ASSERT_EQUAL("Test", "TODO");
}

void Test_Asset::test_depreciate()
{
    Model_Asset asset = Model_Asset::instance();
    //TODO Finalise the test

    CPPUNIT_ASSERT_EQUAL("Test", "TODO");
}

void Test_Asset::test_remove()
{
    Model_Asset asset = Model_Asset::instance();
    //TODO Finalise the test

    CPPUNIT_ASSERT_EQUAL("Test", "TODO");
}
//--------------------------------------------------------------------------
