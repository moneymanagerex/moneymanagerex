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
#include "framebase_tests.h"
#include "cpu_timer.h"
//----------------------------------------------------------------------------
#include "test_filtertrans_dialog.h"
#include "filtertransdialog.h"
//----------------------------------------------------------------------------
#include "model/Model_Infotable.h"
#include "model/Model_Setting.h"

#include "model/Model_Checking.h"
#include "model/Model_Payee.h"
#include "model/Model_Category.h"
#include "model/Model_Billsdeposits.h"
#include "mmOption.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( Test_FilterTrans_Dialog );

static int s_instance_count = 0;
//----------------------------------------------------------------------------
Test_FilterTrans_Dialog::Test_FilterTrans_Dialog()
{
    s_instance_count++;
    m_this_instance = s_instance_count;
    m_test_db_filename = "test_db_filtertrans.mmb";
}

Test_FilterTrans_Dialog::~Test_FilterTrans_Dialog()
{
    s_instance_count--;
    if (s_instance_count < 1)
    {
        wxRemoveFile(m_test_db_filename);
    }
}

void Test_FilterTrans_Dialog::setUp()
{
    CpuTimer time("Startup");
    m_frame = new TestFrameBase(m_this_instance);
    m_frame->Show(true);
    m_test_db.Open(m_test_db_filename);

    // Initialise the required tables
    Model_Infotable::instance(&m_test_db);

    // For the purpose of this test, we will create the
    // settings table in the main database.
    Model_Setting::instance(&m_test_db);
    mmIniOptions::instance().loadOptions();

    Model_Account::instance(&m_test_db);
    Model_Payee::instance(&m_test_db);
    Model_Subcategory::instance(&m_test_db);
    Model_Category::instance(&m_test_db);
    Model_Checking::instance(&m_test_db);

    Model_Billsdeposits::instance(&m_test_db);

    int cat_id_insurance = Model_Category::instance().get("Insurance")->id();
    int subcat_id_auto = Model_Subcategory::instance().get("Auto", cat_id_insurance)->id();

    Model_Checking::instance().Begin();
    {
        // Set up the categories in the other tables.
        Model_Checking::Data* checking_entry = Model_Checking::instance().create();
        checking_entry->CATEGID = cat_id_insurance;
        checking_entry->SUBCATEGID = subcat_id_auto;
        Model_Checking::instance().save(checking_entry);

        Model_Billsdeposits::Data* bill_entry = Model_Billsdeposits::instance().create();
        bill_entry->CATEGID = cat_id_insurance;
        bill_entry->SUBCATEGID = subcat_id_auto;
        Model_Billsdeposits::instance().save(bill_entry);
        Model_Payee::Data* payee_entry = Model_Payee::instance().create();
        payee_entry->PAYEENAME = "Aldi";
        payee_entry->CATEGID = cat_id_insurance;
        payee_entry->SUBCATEGID = subcat_id_auto;
        Model_Payee::instance().save(payee_entry);

        payee_entry = Model_Payee::instance().clone(payee_entry);
        payee_entry->PAYEENAME = "Supermarket";
        Model_Payee::instance().save(payee_entry);
        payee_entry = Model_Payee::instance().clone(payee_entry);

        payee_entry = Model_Payee::instance().clone(payee_entry);
        payee_entry->PAYEENAME = "Coles";
        Model_Payee::instance().save(payee_entry);
        
        payee_entry = Model_Payee::instance().clone(payee_entry);
        payee_entry->PAYEENAME = "Woolworths";
        Model_Payee::instance().save(payee_entry);
    }
    Model_Checking::instance().Commit();
}

void Test_FilterTrans_Dialog::tearDown()
{
    m_test_db.Close();
    delete m_frame;
}

void Test_FilterTrans_Dialog::ShowMessage(wxString msg)
{
    msg = msg << "\nInstance # " << m_this_instance;
    wxMessageBox(msg, "Test: FilterTrans Dialog", wxOK, wxTheApp->GetTopWindow());
}

void Test_FilterTrans_Dialog::test_dialog()
{
    //ShowMessage("Please relocate Insurance/Auto to Automobile/Registration\n\nThis should result in 6 records being changed.\n");
    ShowMessage("No data in tables yet.\n\nStill under construction.\n");
    mmFilterTransactionsDialog dlg(m_frame);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxString msg = "Test Complete: ";
        //msg << dlg.updatedCategoriesCount() << " records changed.";
        ShowMessage(msg);
    }
}
