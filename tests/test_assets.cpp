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
#include "framebase_tests.h"
//----------------------------------------------------------------------------
#include "test_assets.h"
#include "assetspanel.h"

/*****************************************************************************
Turn test ON or OFF in file: defined_test_selection.h
*****************************************************************************/
#include "defined_test_selection.h"

#ifdef MMEX_TESTS_ASSETS
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(Test_Asset);
#endif

static int s_instance_count = 0;
//----------------------------------------------------------------------------
Test_Asset::Test_Asset()
{
    s_instance_count++;
    m_this_instance = s_instance_count;
    m_test_db_filename = "test_db_model_asset.mmb";
}

Test_Asset::~Test_Asset()
{
    s_instance_count--;
    if (s_instance_count < 1)
    {
        wxRemoveFile(m_test_db_filename);
    }
}

void Test_Asset::setUp()
{
    CpuTimer time("Startup");
    m_test_db.Open(m_test_db_filename);

    if (m_this_instance == 5)
    {
        m_base_frame = new TestFrameBase(m_this_instance);
        m_base_frame->Show(true);
    }

    m_dbmodel = new DB_Init_Model();
    m_dbmodel->Init_Model_Assets(&m_test_db);
    m_dbmodel->Init_BaseCurrency();
}

void Test_Asset::tearDown()
{
    delete m_dbmodel;
    m_test_db.Close();

    if (m_this_instance == 5)
    {
        delete m_base_frame;
    }
}

void Test_Asset::test_add()
{
    CpuTimer time("add");
    wxDateTime asset_date = wxDateTime::Today().Subtract(wxDateSpan::Years(5));
    int asset_id = m_dbmodel->Add_Asset("Stop Watch", asset_date, 1000, Model_Asset::TYPE_JEWELLERY, Model_Asset::RATE_APPRECIATE, 20.0, "Appreciate 20% pa 5 years");
    CPPUNIT_ASSERT(asset_id == 1);

    asset_id = m_dbmodel->Add_Asset("Stop Watch", asset_date, 1000, Model_Asset::TYPE_JEWELLERY, Model_Asset::RATE_DEPRECIATE, 20.0, "Depreciates 20% pa 5 years");
    CPPUNIT_ASSERT(asset_id == 2);

    asset_id = m_dbmodel->Add_Asset("Stop Watch", asset_date, 1000, Model_Asset::TYPE_JEWELLERY, Model_Asset::RATE_NONE, 20.0, "No change over 5 years");
    CPPUNIT_ASSERT(asset_id == 3);

    asset_id = m_dbmodel->Add_Asset("To be deleted", asset_date, 1000, Model_Asset::TYPE_JEWELLERY, Model_Asset::RATE_APPRECIATE, 20.0, "Appreciate 20% pa 5 years");
    CPPUNIT_ASSERT(asset_id == 4);

    Model_Asset::Data_Set selected_asset = Model_Asset::instance().find(Model_Asset::ASSETID(2));
    if (selected_asset.size() == 1)
    {
        Model_Asset::Data entry_2 = selected_asset[0];
        CPPUNIT_ASSERT(entry_2.VALUECHANGE == "Depreciates");

        Model_Asset::Data* new_entry = Model_Asset::instance().clone(&entry_2);
        asset_date = asset_date.Subtract(wxDateSpan::Years(5));
        new_entry->STARTDATE = asset_date.FormatISODate();
        new_entry->NOTES = "10 Year depreciation at 20% pa";
        Model_Asset::instance().save(new_entry);
    }

    Model_Asset::Data_Set assets = Model_Asset::instance().all();
    CPPUNIT_ASSERT(assets.size() == 5);
}

void Test_Asset::test_appreciate()
{
    Model_Asset::Data_Set asset_list = Model_Asset::instance().all();
    Model_Asset::Data entry = asset_list[0];

    double value = Model_Asset::value(entry);

    CPPUNIT_ASSERT(entry.VALUECHANGE == "Appreciates");
    CPPUNIT_ASSERT((value > 2488) && (value < 2489));       // values from V 0.9.9.0
    //CPPUNIT_ASSERT((value > 2000) && (value < 2001));     // values from v 0.9.9.2
}

void Test_Asset::test_depreciate()
{
    Model_Asset::Data_Set asset_list = Model_Asset::instance().all();
    Model_Asset::Data entry = asset_list[1];

    double value = Model_Asset::value(entry);

    CPPUNIT_ASSERT(entry.VALUECHANGE == "Depreciates");
    CPPUNIT_ASSERT((value > 327) && (value < 328));     // values from V 0.9.9.0         
    //CPPUNIT_ASSERT(value == 0);                       // values from v 0.9.9.2

    // Entry older than 5 years
    entry = asset_list[4];
    value = Model_Asset::value(entry);

    CPPUNIT_ASSERT(entry.VALUECHANGE == "Depreciates");
    CPPUNIT_ASSERT((value >= 0) && (value < 110));     // values from V 0.9.9.0         
    //CPPUNIT_ASSERT(value == 0);                       // values from v 0.9.9.2
}

void Test_Asset::test_remove()
{
    Model_Asset::instance().remove(4);
    Model_Asset::Data_Set assets = Model_Asset::instance().all();
    CPPUNIT_ASSERT(assets.size() == 4);
}

void Test_Asset::test_assetpanel()
{
    // Create a new frame anchored to the base frame.
    TestFrameBase* asset_frame = new TestFrameBase(m_base_frame, 670, 400);
    asset_frame->Show();

    // Create the panel under test
    mmAssetsPanel* asset_panel = new mmAssetsPanel(asset_frame);
    asset_frame->SetStatusText(Model_Asset::instance().version());
    asset_panel->Show();

    // Anchor the panel. Otherwise it will disappear.
    wxMessageBox("Please Examine: Asset Panel.\n\nContinue other tests ...",
        "Testing: Asset Panel", wxOK, wxTheApp->GetTopWindow());
}
//--------------------------------------------------------------------------
