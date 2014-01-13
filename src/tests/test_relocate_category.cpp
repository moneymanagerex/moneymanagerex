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
//----------------------------------------------------------------------------
#include "test_relocate_category.h"
#include "framebase_tests.h"
#include "relocatecategorydialog.h"
#include "model/Model_Infotable.h"
#include "model/Model_Billsdeposits.h"
#include "model/Model_Checking.h"
#include "model/Model_Payee.h"
#include "model/Model_Setting.h"
#include "model/Model_Category.h"
#include "model/Model_Budget.h"
#include "mmOption.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( Test_Relocate_Category );

static int s_instance_count = 0;
//----------------------------------------------------------------------------
Test_Relocate_Category::Test_Relocate_Category()
{
    s_instance_count++;
    m_this_instance = s_instance_count;
    m_test_db_filename = "test_db_relocate_category.mmb";
}

Test_Relocate_Category::~Test_Relocate_Category()
{
    s_instance_count--;
    if (s_instance_count < 1)
    {
        wxRemoveFile(m_test_db_filename);
    }
}

void Test_Relocate_Category::setUp()
{
    CpuTimer time("Startup");
    m_frame = new TestFrameBase(m_this_instance);
    m_frame->Show(true);
    m_test_db.Open(m_test_db_filename);

    // Initialise the required tables
    Model_Infotable::instance(&m_test_db);
    Model_Billsdeposits::instance(&m_test_db);
    Model_Budgetsplittransaction::instance(&m_test_db);
    // subcategory must be initialized before category
    Model_Subcategory::instance(&m_test_db);
    Model_Category::instance(&m_test_db);
    Model_Payee::instance(&m_test_db);
    Model_Checking::instance(&m_test_db);
    Model_Splittransaction::instance(&m_test_db);
    Model_Budget::instance(&m_test_db);

    // For the purpose of this test, we will create the
    // settings table in the main database.
    Model_Setting::instance(&m_test_db);
    mmIniOptions::instance().loadOptions();

    int cat_id_insurance = Model_Category::instance().get("Insurance")->id();
    int subcat_id_auto = Model_Subcategory::instance().get("Auto", cat_id_insurance)->id();

    Model_Checking::instance().Begin();
    {
        // Set up the categories in the other tables.
        Model_Checking::Data* checking_entry = Model_Checking::instance().create();
        checking_entry->CATEGID = cat_id_insurance;
        checking_entry->SUBCATEGID = subcat_id_auto;
        Model_Checking::instance().save(checking_entry);

        Model_Splittransaction::Data* split_trans_entry = Model_Splittransaction::instance().create();
        split_trans_entry->CATEGID = cat_id_insurance;
        split_trans_entry->SUBCATEGID = subcat_id_auto;
        Model_Splittransaction::instance().save(split_trans_entry);

        Model_Budgetsplittransaction::Data* budget_split_trans_entry = Model_Budgetsplittransaction::instance().create();
        budget_split_trans_entry->CATEGID = cat_id_insurance;
        budget_split_trans_entry->SUBCATEGID = subcat_id_auto;
        Model_Budgetsplittransaction::instance().save(budget_split_trans_entry);

        Model_Billsdeposits::Data* bill_entry = Model_Billsdeposits::instance().create();
        bill_entry->CATEGID = cat_id_insurance;
        bill_entry->SUBCATEGID = subcat_id_auto;
        Model_Billsdeposits::instance().save(bill_entry);

        Model_Budget::Data* budget_entry = Model_Budget::instance().create();
        budget_entry->CATEGID = cat_id_insurance;
        budget_entry->SUBCATEGID = subcat_id_auto;
        Model_Budget::instance().save(budget_entry);

        Model_Payee::Data* payee_entry = Model_Payee::instance().create();
        payee_entry->PAYEENAME = "Aldi";
        payee_entry->CATEGID = cat_id_insurance;
        payee_entry->SUBCATEGID = subcat_id_auto;
        Model_Payee::instance().save(payee_entry);
    }
    Model_Checking::instance().Commit();
}

void Test_Relocate_Category::tearDown()
{
    m_test_db.Close();
    delete m_frame;
}

void Test_Relocate_Category::ShowMessage(wxString msg)
{
    msg = msg << "\nInstance # " << m_this_instance;
    wxMessageBox(msg, "Test: Relocate Category Dialog", wxOK, wxTheApp->GetTopWindow());
}

void Test_Relocate_Category::test_dialog()
{
    ShowMessage("Please relocate Insurance/Auto to Automobile/Registration\n\nThis should result in 6 records being changed.\n");
    relocateCategoryDialog dlg(m_frame);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxString msg = "Test Complete: ";
        msg << dlg.updatedCategoriesCount() << " records changed.";
        ShowMessage(msg);
    }

    int cat_id_auto = Model_Category::instance().get("Automobile")->id();
    int subcat_id_reg = Model_Subcategory::instance().get("Registration", cat_id_auto)->id();

    CPPUNIT_ASSERT(dlg.updatedCategoriesCount() == 6);
    CPPUNIT_ASSERT(Model_Checking::instance().get(1)->CATEGID == cat_id_auto);
    CPPUNIT_ASSERT(Model_Checking::instance().get(1)->SUBCATEGID == subcat_id_reg);

    CPPUNIT_ASSERT(Model_Splittransaction::instance().get(1)->CATEGID == cat_id_auto);
    CPPUNIT_ASSERT(Model_Splittransaction::instance().get(1)->SUBCATEGID == subcat_id_reg);

    CPPUNIT_ASSERT(Model_Budgetsplittransaction::instance().get(1)->CATEGID == cat_id_auto);
    CPPUNIT_ASSERT(Model_Budgetsplittransaction::instance().get(1)->SUBCATEGID == subcat_id_reg);

    CPPUNIT_ASSERT(Model_Billsdeposits::instance().get(1)->CATEGID == cat_id_auto);
    CPPUNIT_ASSERT(Model_Billsdeposits::instance().get(1)->SUBCATEGID == subcat_id_reg);

    CPPUNIT_ASSERT(Model_Budget::instance().get(1)->CATEGID == cat_id_auto);
    CPPUNIT_ASSERT(Model_Budget::instance().get(1)->SUBCATEGID == subcat_id_reg);

    CPPUNIT_ASSERT(Model_Payee::instance().get("Aldi")->CATEGID == cat_id_auto);
    CPPUNIT_ASSERT(Model_Payee::instance().get("Aldi")->SUBCATEGID == subcat_id_reg);
}
