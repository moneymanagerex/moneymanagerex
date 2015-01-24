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
#include "assetdialog.h"
#include "../src/constants.h"
#include "mmcheckingpanel.h"
#include "model/Model_TransferTrans.h"

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
        m_user_request = new TestFrameBase(m_base_frame);
        m_user_request->Show();
    }

    if (m_this_instance == 6)
    {
        m_base_frame = new TestFrameBase(m_this_instance);
        m_base_frame->Show(true);
    }

    m_dbmodel = new DB_Init_Model();
    m_dbmodel->Init_Model_Tables(&m_test_db);
    m_dbmodel->Init_Model_Assets(&m_test_db);
}

void Test_Asset::tearDown()
{
    delete m_dbmodel;
    m_test_db.Close();

    if (m_this_instance > 4)
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

void Test_Asset::test_dialog()
{
    m_user_request->Show_InfoBarMessage(
        "Assets Dialog Test:\n\n"
        "\n\n"
        "Use Cancel to ignore test results.");

    mmAssetDialog* dlg = new mmAssetDialog(m_base_frame, 0);
    if (dlg->ShowModal() == wxID_OK)
    {
        Model_Asset::Data_Set asset_table = Model_Asset::instance().all();
        CPPUNIT_ASSERT(asset_table.size() > 0);

        Model_Asset::Data bill_entry = asset_table.at(asset_table.size() - 1);
    }
}

void Test_Asset::test_assetpanel()
{
    // Set up some data in table
    m_test_db.Begin();
    m_dbmodel->Add_Bank_Account("Cheque", 0);
    m_dbmodel->Add_Bank_Account("Savings", 0);

    m_dbmodel->Add_Payee("Peter", "Income", "Salary");
    m_dbmodel->Add_Payee("Supermarket");

    wxDateTime trans_date(wxDateTime::Today().Subtract(wxDateSpan::Days(7)));
    m_dbmodel->Add_Trans_Deposit("Savings", trans_date, "Peter", 1500.0, "Income", "Salary");
    m_dbmodel->Add_Trans_Withdrawal("Savings", trans_date, "Supermarket", 300.0, "Food", "Groceries");
    m_test_db.Commit();

    // Need to have user get the account
    int savings_account_id = m_dbmodel->Get_account_id("Savings");

    double purchase_value = 1400.0;

    // Asset is created from assets, specifying Buy/withdrawal, Sell/income,
    Model_Asset::Data* asset_entry = Model_Asset::instance().create();
    asset_entry->ASSETNAME = "Bed";
    asset_entry->STARTDATE = trans_date.FormatISODate();
    asset_entry->VALUE = purchase_value;
    asset_entry->VALUECHANGE = Model_Asset::all_rate()[Model_Asset::RATE_DEPRECIATE];
    asset_entry->VALUECHANGERATE = 10.0;
    asset_entry->ASSETTYPE = Model_Asset::all_type()[Model_Asset::TYPE_HOUSE];
    asset_entry->NOTES = "Purchase of New Bed";
    Model_Asset::instance().save(asset_entry);

    Model_TransferTrans::Data* trans_entry = Model_TransferTrans::instance().create();
    Model_Checking::Data* checking_entry = Model_Checking::instance().create();

    /*  Set tp the transfer table.
        
        This allows
        1. Asset record to be located via knowing the transaction
        2. Checking record to be found from knowing the asset
    */
    trans_entry->TABLE_TYPE = Model_TransferTrans::all_table_type()[Model_TransferTrans::ASSETS];
    trans_entry->ID_TABLE = asset_entry->id();
    trans_entry->ID_CHECKINGACCOUNT = checking_entry->id();
    trans_entry->ID_CURRENCY = Model_Currency::instance().GetBaseCurrency()->id();

    /* Set tp the transaction

       Purchase of an asset will be a withdrawal from the checking account
       Sale of an asset will be a deposit in the checking account.
    */
    checking_entry->ACCOUNTID = savings_account_id;
    checking_entry->TOACCOUNTID = trans_entry->id();
    checking_entry->PAYEEID = m_dbmodel->Get_Payee_id("Supermarket");
    checking_entry->TRANSCODE = Model_Checking::instance().all_type()[Model_Checking::WITHDRAWAL];
    checking_entry->TRANSAMOUNT = purchase_value;
    checking_entry->STATUS = Model_Checking::all_status()[Model_Checking::RECONCILED].Mid(0, 1);
    checking_entry->CATEGID = m_dbmodel->Get_category_id("Homeneeds");
    checking_entry->SUBCATEGID = m_dbmodel->Get_subcategory_id(checking_entry->CATEGID, "Furnishing");
    checking_entry->TRANSDATE = trans_date.FormatISODate();
    checking_entry->FOLLOWUPID = 0;
    checking_entry->NOTES = "Asset: New Bed";
    checking_entry->TOTRANSAMOUNT = purchase_value;
    Model_Checking::instance().save(checking_entry);

    /*
        Now to display the two rcords.
    */

    // Create a new frame anchored to the base frame.
    TestFrameBase* asset_frame = new TestFrameBase(m_base_frame, 670, 400);
    asset_frame->Show();

    // Create the panel under test
    mmAssetsPanel* asset_panel = new mmAssetsPanel(asset_frame, mmID_ASSETS);
    asset_panel->Show();

    TestFrameBase* account_frame = new TestFrameBase(m_base_frame, 670, 400);
    account_frame->Show();

    mmCheckingPanel* account_panel = new mmCheckingPanel(account_frame, 0, savings_account_id);
    account_panel->Show();

    /*
        Code to identify this transaction type is required in the checking panel
        TRANSCODE == Model_Checking::WITHDRAWAL or Model_Checking::DEPOSIT
        TOACCOUNTID != -1   == holds ID to Model_TransferTrans record
    */

    // Anchor the panels. Otherwise they will disappear.
    wxMessageBox(
        "Please Examine: Asset Panel and Account panel."
        "\n\nContinue other tests ...",
        "Testing: Asset Panel to Account Panel", wxOK, wxTheApp->GetTopWindow());

    delete(account_panel);
}
//--------------------------------------------------------------------------
