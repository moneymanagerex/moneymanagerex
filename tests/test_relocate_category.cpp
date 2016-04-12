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
#include "test_relocate_category.h"
#include "relocatecategorydialog.h"
#include "model/Model_Infotable.h"
#include "model/Model_Setting.h"

#include "model/Model_Checking.h"
#include "model/Model_Payee.h"
#include "model/Model_Category.h"
#include "model/Model_Billsdeposits.h"
#include "model/Model_Budget.h"
#include "option.h"

/*****************************************************************************
Turn test ON or OFF in file: defined_test_selection.h
*****************************************************************************/
#include "defined_test_selection.h"

#ifdef MMEX_TESTS_RELOCATE_CATEGORY
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( Test_Relocate_Category );
#endif

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
    m_base_frame = new TestFrameBase(m_this_instance);
    m_base_frame->Show(true);
    m_test_db.Open(m_test_db_filename);

    m_dbmodel = new DB_Init_Model();
    m_dbmodel->Init_Model_Tables(&m_test_db);
    m_dbmodel->Init_BaseCurrency();

    int cat_id_insurance = Model_Category::instance().get("Insurance")->id();
    int subcat_id_auto = Model_Subcategory::instance().get("Auto", cat_id_insurance)->id();

    Model_Checking::instance().Savepoint();
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
    Model_Checking::instance().ReleaseSavepoint();
}

void Test_Relocate_Category::tearDown()
{
    delete m_dbmodel;
    m_test_db.Close();
    delete m_base_frame;
}

void Test_Relocate_Category::test_dialog()
{
    TestFrameBase* user_request = new TestFrameBase(m_base_frame);
    user_request->Show();

    wxString info_message = "Please relocate:\n";
    info_message << "Insurance/Auto to Automobile/Registration\n\n";
    info_message << "Use Cancel to ignore test results.";
    user_request->Show_InfoBarMessage(info_message);
    relocateCategoryDialog dlg(m_base_frame);
    if (dlg.ShowModal() == wxID_OK)
    {
        CPPUNIT_ASSERT(dlg.updatedCategoriesCount() == 6);
        int cat_id_auto = Model_Category::instance().get("Automobile")->id();
        int subcat_id_reg = Model_Subcategory::instance().get("Registration", cat_id_auto)->id();

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
}
