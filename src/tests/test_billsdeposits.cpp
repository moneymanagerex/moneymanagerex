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

#include "defined_test_selection.h"
#include "defs.h"
#include <cppunit/config/SourcePrefix.h>
#include "cpu_timer.h"
#include "db_init_model.h"
#include "framebase_tests.h"
//----------------------------------------------------------------------------
#include "test_billsdeposits.h"
#include "billsdepositsdialog.h"
#include "model/Model_Payee.h"
#include "model/Model_Setting.h"
#include "model/Model_Category.h"
#include "mmOption.h"

#ifdef __MMEX_TESTS__BILLS_DEPOSITS
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(Test_BillsDeposits);
#endif

static int s_instance_count = 0;
//----------------------------------------------------------------------------
Test_BillsDeposits::Test_BillsDeposits()
{
    s_instance_count++;
    m_this_instance = s_instance_count;
    m_test_db_filename = "test_db_model_billsdeposits.mmb";
}

Test_BillsDeposits::~Test_BillsDeposits()
{
    s_instance_count--;
    if (s_instance_count < 1)
    {
        wxRemoveFile(m_test_db_filename);
    }
}

void Test_BillsDeposits::setUp()
{
    CpuTimer time("Setup");
    m_base_frame = new TestFrameBase(m_this_instance);
    m_base_frame->Show(true);

    m_user_request = new TestFrameBase(m_base_frame);
    m_user_request->Show();

    m_test_db.Open(m_test_db_filename);

    m_dbmodel = new DB_Init_Model();
    m_dbmodel->Init_Model_Tables(&m_test_db);

    // Only need to add data to database once.
    if (m_this_instance == 1)
    {
        m_dbmodel->Init_BaseCurrency();

        m_test_db.Begin();
        {
            // initialise some accounts
            m_dbmodel->Add_Account("Savings", Model_Account::TYPE::CHECKING);
            m_dbmodel->Add_Account("Cheque", Model_Account::TYPE::CHECKING);
            m_dbmodel->Add_Account("Mastercard", Model_Account::TYPE::CHECKING);

            // Initialise some payees
            m_dbmodel->Add_Payee("Workshop");
            m_dbmodel->Add_Payee("Supermarket", "Food", "Groceries");
            m_dbmodel->Add_Payee("Restaurant", "Food", "Dining Out");
        }
        m_test_db.Commit();
    }
}

void Test_BillsDeposits::tearDown()
{
    delete m_dbmodel;
    m_test_db.Close();
    delete m_user_request;
    delete m_base_frame;
}

void Test_BillsDeposits::ShowMessage(wxString msg)
{
    msg = msg << "   # " << m_this_instance;
    wxMessageBox(msg, "MMEX Bill Deposits Dialog Test", wxOK, wxTheApp->GetTopWindow());
}

void Test_BillsDeposits::test_new_simple_entry()
{
    m_user_request->Show_InfoBarMessage(
        "Please create a new Simple entry:\n\n"
        "Account Name: Savings,   Repeats: Monthly\n"
        "Amount: 100,   Payee: Supermarket\n");

    mmBDDialog* dlg = new mmBDDialog(m_base_frame, 0, false, false);
    if (dlg->ShowModal() == wxID_OK)
    {
        Model_Billsdeposits::Data_Set table_entries = Model_Billsdeposits::instance().all();
        if (table_entries.size() == 1)
        {
            Model_Billsdeposits::Data entry = table_entries[0];
            CPPUNIT_ASSERT(entry.ACCOUNTID == Model_Account::instance().get("Savings")->ACCOUNTID);
            CPPUNIT_ASSERT(entry.NEXTOCCURRENCEDATE == wxDateTime(wxDateTime::Today()).FormatISODate());
            CPPUNIT_ASSERT(entry.TRANSAMOUNT == 100);
            CPPUNIT_ASSERT(entry.TOTRANSAMOUNT == 100);
            CPPUNIT_ASSERT(entry.REPEATS == Model_Billsdeposits::REPEAT_MONTHLY);
        }
    }
}

void Test_BillsDeposits::test_edit_simple_entry()
{
    Model_Billsdeposits::Data_Set entries = Model_Billsdeposits::instance().all();
    if (entries.size() < 1) return;
    CPPUNIT_ASSERT(entries.size() == 1);

    m_user_request->Show_InfoBarMessage(
        "Please confirm Simple entry - Edit\n\n"
        "Account Name: Savings,   Repeats: Monthly\n"
        "Amount: 100,   Payee: Supermarket\n"
        "Category: Food/Groceries\n\n"
        "Modify Amount: 200\n");

    mmBDDialog* dlg = new mmBDDialog(m_base_frame, 1, true, false);
    if (dlg->ShowModal() == wxID_OK)
    {
        Model_Billsdeposits::Data_Set table_entries = Model_Billsdeposits::instance().all();
        if (table_entries.size() == 1)
        {
            Model_Billsdeposits::Data entry = table_entries[0];
            CPPUNIT_ASSERT(entry.ACCOUNTID == Model_Account::instance().get("Savings")->ACCOUNTID);
            CPPUNIT_ASSERT(entry.NEXTOCCURRENCEDATE == wxDateTime(wxDateTime::Today()).FormatISODate());
            CPPUNIT_ASSERT(entry.TRANSAMOUNT == 200);
            CPPUNIT_ASSERT(entry.TOTRANSAMOUNT == 200);
            CPPUNIT_ASSERT(entry.REPEATS == Model_Billsdeposits::REPEAT_MONTHLY);
        }
    }
}

void Test_BillsDeposits::test_enter_simple_entry()
{
    Model_Billsdeposits::Data_Set entries = Model_Billsdeposits::instance().all();
    if (entries.size() < 1) return;
    CPPUNIT_ASSERT(entries.size() == 1);

    m_user_request->Show_InfoBarMessage(
        "Please confirm Simple entry - Enter\n\n"
        "Account Name: Savings,   Repeats: Monthly\n"
        "Amount: 200,   Payee: Supermarket\n"
        "Category: Food/Groceries\n");

    mmBDDialog* dlg = new mmBDDialog(m_base_frame, 1, false, true);
    if (dlg->ShowModal() == wxID_OK)
    {
        Model_Checking::Data_Set table_entries = Model_Checking::instance().all();
        if (table_entries.size() == 1)
        {
            Model_Checking::Data entry = table_entries[0];
            CPPUNIT_ASSERT(entry.ACCOUNTID == Model_Account::instance().get("Savings")->ACCOUNTID);
            CPPUNIT_ASSERT(entry.TRANSAMOUNT == 200);
            CPPUNIT_ASSERT(entry.TOTRANSAMOUNT == 200);

            Model_Billsdeposits::Data* bill = Model_Billsdeposits::instance().get(1);
            CPPUNIT_ASSERT(bill->REPEATS == Model_Billsdeposits::REPEAT_MONTHLY);
            wxDateTime next_date = wxDateTime(wxDateTime::Today()).Add(wxDateSpan::Month());
            CPPUNIT_ASSERT(bill->NEXTOCCURRENCEDATE == next_date.FormatISODate());
        }
    }
}
//-----------------------------------------------------------------------------

void Test_BillsDeposits::test_new_transfer_entry()
{
    m_user_request->Show_InfoBarMessage(
        "Please create a new Transfer entry:\n\n"
        "Account Name: Savings,   Repeats: Quarterly\n"
        "Type: Transfer,    Advanced,   Amount: 100, 50\n"
        "To: Cheque         Category: Gifts\n");

    mmBDDialog* dlg = new mmBDDialog(m_base_frame, 0, false, false);
    if (dlg->ShowModal() == wxID_OK)
    {
        Model_Billsdeposits::Data_Set table_entries = Model_Billsdeposits::instance().all();
        if (table_entries.size() == 2)
        {
            Model_Billsdeposits::Data entry = table_entries[1];
            CPPUNIT_ASSERT(entry.ACCOUNTID == Model_Account::instance().get("Savings")->ACCOUNTID);
            CPPUNIT_ASSERT(entry.NEXTOCCURRENCEDATE == wxDateTime(wxDateTime::Today()).FormatISODate());
            CPPUNIT_ASSERT(entry.TRANSAMOUNT == 100);
            CPPUNIT_ASSERT(entry.TOTRANSAMOUNT == 50);
            CPPUNIT_ASSERT(entry.REPEATS == Model_Billsdeposits::REPEAT_QUARTERLY);
        }
    }
}

void Test_BillsDeposits::test_edit_transfer_entry()
{
    Model_Billsdeposits::Data_Set entries = Model_Billsdeposits::instance().all();
    if (entries.size() < 2) return;
    CPPUNIT_ASSERT(entries.size() == 2);

    m_user_request->Show_InfoBarMessage(
        "Please confirm transfer entry - Edit\n\n"
        "Account Name: Savings,   Repeats: Quarterly\n"
        "Type: Transfer,   Advanced,   Amount: 100, 50\n"
        "To: Cheque        Category: Gifts\n");

    mmBDDialog* dlg = new mmBDDialog(m_base_frame, 2, true, false);
    if (dlg->ShowModal() == wxID_OK)
    {
        Model_Billsdeposits::Data_Set table_entries = Model_Billsdeposits::instance().all();
        if (table_entries.size() == 1)
        {
            Model_Billsdeposits::Data entry = table_entries[1];
            CPPUNIT_ASSERT(entry.ACCOUNTID == Model_Account::instance().get("Savings")->ACCOUNTID);
            CPPUNIT_ASSERT(entry.NEXTOCCURRENCEDATE == wxDateTime(wxDateTime::Today()).FormatISODate());
            CPPUNIT_ASSERT(entry.TRANSAMOUNT == 100);
            CPPUNIT_ASSERT(entry.TOTRANSAMOUNT == 50);
            CPPUNIT_ASSERT(entry.REPEATS == Model_Billsdeposits::REPEAT_QUARTERLY);
        }
    }
}

void Test_BillsDeposits::test_enter_transfer_entry()
{
    Model_Billsdeposits::Data_Set entries = Model_Billsdeposits::instance().all();
    if (entries.size() < 2) return;
    CPPUNIT_ASSERT(entries.size() == 2);

    m_user_request->Show_InfoBarMessage(
        "Please confirm transfer entry - Enter\n\n"
        "Account Name: Savings,   Repeats: Quarterly\n"
        "Type: Transfer,   Advanced,   Amount: 100, 50\n"
        "To Account: Cheque   Category: Gifts\n");

    mmBDDialog* dlg = new mmBDDialog(m_base_frame, 2, false, true);
    if (dlg->ShowModal() == wxID_OK)
    {
        Model_Checking::Data_Set table_entries = Model_Checking::instance().all();
        if (table_entries.size() == 1)
        {
            Model_Checking::Data entry = table_entries[1];
            CPPUNIT_ASSERT(entry.ACCOUNTID == Model_Account::instance().get("Savings")->ACCOUNTID);
            CPPUNIT_ASSERT(entry.TRANSAMOUNT == 100);
            CPPUNIT_ASSERT(entry.TOTRANSAMOUNT == 50);

            Model_Billsdeposits::Data* bill = Model_Billsdeposits::instance().get(1);
            CPPUNIT_ASSERT(bill->REPEATS == Model_Billsdeposits::REPEAT_QUARTERLY);
            wxDateTime next_date = wxDateTime(wxDateTime::Today()).Add(wxDateSpan::Months(3));
            CPPUNIT_ASSERT(bill->NEXTOCCURRENCEDATE == next_date.FormatISODate());
        }
    }
}
//-----------------------------------------------------------------------------

void Test_BillsDeposits::test_new_split_entry()
{
    m_user_request->Show_InfoBarMessage(
        "Please create a new Split entry\n\n"
        "Account Name: Savings,   Repeats: Monthly\n"
        "Amount: 150,   Payee: Supermarket,   Split\n"
        "Add Deposit 50, Category: Bills/Water\n");

    mmBDDialog* dlg = new mmBDDialog(m_base_frame, 0, false, false);
    if (dlg->ShowModal() == wxID_OK)
    {
        Model_Billsdeposits::Data_Set table_entries = Model_Billsdeposits::instance().all();
        if (table_entries.size() == 1)
        {
            Model_Billsdeposits::Data entry = table_entries[2];
            CPPUNIT_ASSERT(entry.ACCOUNTID == Model_Account::instance().get("Savings")->ACCOUNTID);
            CPPUNIT_ASSERT(entry.NEXTOCCURRENCEDATE == wxDateTime(wxDateTime::Today()).FormatISODate());
            CPPUNIT_ASSERT(entry.TRANSAMOUNT == 100);
            CPPUNIT_ASSERT(entry.TOTRANSAMOUNT == 100);
            CPPUNIT_ASSERT(entry.REPEATS == Model_Billsdeposits::REPEAT_MONTHLY);
        }
    }
}

void Test_BillsDeposits::test_edit_split_entry()
{
    Model_Billsdeposits::Data_Set entries = Model_Billsdeposits::instance().all();
    if (entries.size() < 3) return;
    CPPUNIT_ASSERT(entries.size() == 3);

    m_user_request->Show_InfoBarMessage(
        "Please confirm Split entry - Edit\n\n"
        "Account Name: Savings,   Repeats: Monthly\n"
        "Amount: 100 (Grey)   Payee: Supermarket,   Split\n"
        "Split Entries:"
        "Type: Withdrawal 150,   Category: Food/Groceries\n"
        "Type: Deposit 50,       Category: Bills/Water\n");

    mmBDDialog* dlg = new mmBDDialog(m_base_frame, 3, true, false);
    if (dlg->ShowModal() == wxID_OK)
    {
        Model_Billsdeposits::Data_Set table_entries = Model_Billsdeposits::instance().all();
        if (table_entries.size() == 1)
        {
            Model_Billsdeposits::Data entry = table_entries[2];
            CPPUNIT_ASSERT(entry.ACCOUNTID == Model_Account::instance().get("Savings")->ACCOUNTID);
            CPPUNIT_ASSERT(entry.NEXTOCCURRENCEDATE == wxDateTime(wxDateTime::Today()).FormatISODate());
            CPPUNIT_ASSERT(entry.TRANSAMOUNT == 200);
            CPPUNIT_ASSERT(entry.TOTRANSAMOUNT == 200);
            CPPUNIT_ASSERT(entry.REPEATS == Model_Billsdeposits::REPEAT_MONTHLY);
        }
    }
}

void Test_BillsDeposits::test_enter_split_entry()
{
    Model_Billsdeposits::Data_Set entries = Model_Billsdeposits::instance().all();
    if (entries.size() < 3) return;
    CPPUNIT_ASSERT(entries.size() == 3);

    m_user_request->Show_InfoBarMessage(
        "Please confirm Split entry - Enter\n\n"
        "Account Name: Savings,   Repeats: Monthly\n"
        "Amount: 100 (Grey)   Payee: Supermarket,   Split\n"
        "Split Entries:"
        "Type: Withdrawal 150,   Category: Food/Groceries\n"
        "Type: Deposit 50,       Category: Bills/Water\n");

    mmBDDialog* dlg = new mmBDDialog(m_base_frame, 3, false, true);
    if (dlg->ShowModal() == wxID_OK)
    {
        Model_Checking::Data_Set table_entries = Model_Checking::instance().all();
        if (table_entries.size() == 1)
        {
            Model_Checking::Data entry = table_entries[2];
            CPPUNIT_ASSERT(entry.ACCOUNTID == Model_Account::instance().get("Savings")->ACCOUNTID);
            CPPUNIT_ASSERT(entry.TRANSAMOUNT == 200);
            CPPUNIT_ASSERT(entry.TOTRANSAMOUNT == 200);

            Model_Billsdeposits::Data* bill = Model_Billsdeposits::instance().get(1);
            CPPUNIT_ASSERT(bill->REPEATS == Model_Billsdeposits::REPEAT_MONTHLY);
            wxDateTime next_date = wxDateTime(wxDateTime::Today()).Add(wxDateSpan::Month());
            CPPUNIT_ASSERT(bill->NEXTOCCURRENCEDATE == next_date.FormatISODate());
        }
    }
}

void Test_BillsDeposits::test_dialog_freeform()
{
    m_user_request->Show_InfoBarMessage("Freeform Test\nUse Cancel to exit dialog.");
    mmBDDialog* dlg = new mmBDDialog(m_base_frame, 0, false, false);

    bool testing_dialog = true;
    while (testing_dialog)
    {
        if (dlg->ShowModal() == wxID_CANCEL)
        {
            testing_dialog = false;
        }
    }
}

//--------------------------------------------------------------------------
