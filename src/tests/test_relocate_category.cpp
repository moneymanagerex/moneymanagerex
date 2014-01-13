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
    ShowMessage("Not complete:\n\nNo data in tables.\n");
    relocateCategoryDialog dlg(m_frame);
    if (dlg.ShowModal() == wxID_OK)
    {
        ShowMessage("To be completed.");
    }
}
