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
//----------------------------------------------------------------------------
#include "test_relocate_payee.h"
#include "framebase_tests.h"
#include "relocatepayeedialog.h"
#include "model/Model_Payee.h"
#include "model/Model_Category.h"
#include "model/Model_Checking.h"
#include "model/Model_Billsdeposits.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( Test_Relocate_Payee );

static int s_instance_count = 0;
//----------------------------------------------------------------------------
Test_Relocate_Payee::Test_Relocate_Payee()
{
    s_instance_count++;
    m_this_instance = s_instance_count;
    m_test_db_filename = "test_db_relocate_payee.mmb";
}

Test_Relocate_Payee::~Test_Relocate_Payee()
{
    s_instance_count--;
    if (s_instance_count < 1)
    {
        wxRemoveFile(m_test_db_filename);
    }
}

void Test_Relocate_Payee::setUp()
{
    CpuTimer time("Startup");
    m_frame = new TestFrameBase(m_this_instance);
    m_frame->Show(true);
    m_test_db.Open(m_test_db_filename);
    m_dbmodel = new DB_Init_Model();
    m_dbmodel->Init_Model_Tables(&m_test_db);
    m_dbmodel->Init_BaseCurrency();


    // Initialise some payees
    Model_Payee::instance().Begin();
    {
        Model_Payee::Data* payee = Model_Payee::instance().create();
        payee->PAYEENAME = "Workshop";
        payee->CATEGID = Model_Category::instance().get("Income")->id();
        payee->SUBCATEGID = Model_Subcategory::instance().get("Salary", payee->CATEGID)->id();
        Model_Payee::instance().save(payee);

        Model_Payee::Data* supermarket = Model_Payee::instance().clone(payee);
        supermarket->PAYEENAME = "Supermarket";
        supermarket->CATEGID = Model_Category::instance().get("Food")->id();
        supermarket->SUBCATEGID = Model_Subcategory::instance().get("Groceries", supermarket->CATEGID)->id();
        Model_Payee::instance().save(supermarket);

        payee = Model_Payee::instance().create();
        payee->PAYEENAME = "Aldi";
        Model_Payee::instance().save(payee);

        payee = Model_Payee::instance().create();
        payee->PAYEENAME = "Coles";
        Model_Payee::instance().save(payee);

        payee = Model_Payee::instance().create();
        payee->PAYEENAME = "Woolworths";
        Model_Payee::instance().save(payee);

        // Set up payees inthe other tables.
        Model_Checking::Data* checking_entry = Model_Checking::instance().create();
        checking_entry->PAYEEID = supermarket->id();
        Model_Checking::instance().save(checking_entry);

        Model_Billsdeposits::Data* bill_entry = Model_Billsdeposits::instance().create();
        bill_entry->PAYEEID = supermarket->id();
        Model_Billsdeposits::instance().save(bill_entry);
    }
    Model_Payee::instance().Commit();
}

void Test_Relocate_Payee::tearDown()
{
    m_test_db.Close();
    delete m_frame;
    delete m_dbmodel;
}

void Test_Relocate_Payee::ShowMessage(wxString msg)
{
    msg = msg << "\nInstance # " << m_this_instance;
    wxMessageBox(msg, "Test: Relocate Payee Dialog", wxOK, wxTheApp->GetTopWindow());
}

void Test_Relocate_Payee::test_dialog()
{
    int supermarket_id = Model_Payee::instance().get("Supermarket")->id();
    int coles_id = Model_Payee::instance().get("Coles")->id();

    CPPUNIT_ASSERT(Model_Checking::instance().get(1)->PAYEEID == supermarket_id);
    CPPUNIT_ASSERT(Model_Billsdeposits::instance().get(1)->PAYEEID == supermarket_id);


    ShowMessage("Please relocate Supermarket to Coles\n\nThis should result in 2 records being changed.\n");
    relocatePayeeDialog dlg(m_frame);
    if (dlg.ShowModal() == wxID_OK)
    {
        wxString msg = "Test Complete: ";
        msg << dlg.updatedPayeesCount() << " records changed.";
        ShowMessage(msg);
    }

    CPPUNIT_ASSERT(dlg.updatedPayeesCount() == 2);
    CPPUNIT_ASSERT(Model_Checking::instance().get(1)->PAYEEID == coles_id);
    CPPUNIT_ASSERT(Model_Billsdeposits::instance().get(1)->PAYEEID == coles_id);
}
