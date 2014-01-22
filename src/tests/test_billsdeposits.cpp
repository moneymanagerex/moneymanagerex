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
    wxMessageBox(msg, "MMEX Bill Deposits Dialog Test", wxOK, wxTheApp->GetTopWindow());
}

//----------------------------------------------------------------------------
void Test_BillsDeposits::test_dialog_freeform_new()
{
    m_user_request->Show_InfoBarMessage(
        "Free form New Repeating Transaction\n\n"
        "Use Cancel to progress to next dialog.\n"
        "Note: Enter and Entry dialogs will only appear\n"
        "      if an entry has been created\n");

    mmBDDialog* dlg = new mmBDDialog(m_base_frame, 0, false, false);

    bool testing_dialog = true;
    while (testing_dialog)
    {
        if (dlg->ShowModal() == wxID_CANCEL)
        {
            testing_dialog = false;
        }
        else ShowMessage("Contents saved to database,\n\nSame dialog will be displayed.");
    }
}

void Test_BillsDeposits::test_dialog_freeform_edit()
{
    Model_Billsdeposits::Data_Set bill_table = Model_Billsdeposits::instance().all();
    if (bill_table.size() < 1) return;

    m_user_request->Show_InfoBarMessage(
        "Free form Edit Repeating Transaction\n\n"
        "Editing last repeat transaction in the list.\n\n"
        "Use Cancel to exit dialog.");

    Model_Billsdeposits::Data bill_entry = bill_table.at(bill_table.size() - 1);
    mmBDDialog* dlg = new mmBDDialog(m_base_frame, bill_entry.BDID, true, false);

    bool testing_dialog = true;
    while (testing_dialog)
    {
        if (dlg->ShowModal() == wxID_CANCEL)
        {
            testing_dialog = false;
        }
        else ShowMessage("Contents saved to database,\n\nSame dialog will be displayed.");
    }
}

void Test_BillsDeposits::test_dialog_freeform_enter()
{
    Model_Billsdeposits::Data_Set bill_table = Model_Billsdeposits::instance().all();
    if (bill_table.size() < 1) return;

    m_user_request->Show_InfoBarMessage(
        "Free form Entry Repeating Transaction\n\n"
        "Editing last repeat transaction in the list.\n\n"
        "Use Cancel to exit dialog.");

    Model_Billsdeposits::Data bill_entry = bill_table.at(bill_table.size() - 1);
    mmBDDialog* dlg = new mmBDDialog(m_base_frame, bill_entry.BDID, false, true);

    bool testing_dialog = true;
    while (testing_dialog)
    {
        if (dlg->ShowModal() == wxID_CANCEL)
        {
            testing_dialog = false;
        }
        else ShowMessage("Contents saved to database,\n\nSame dialog will be displayed.");
    }
}
//----------------------------------------------------------------------------

void Test_BillsDeposits::test_new_simple_entry()
{
    m_user_request->Show_InfoBarMessage(
        "Please create a new Simple entry:\n\n"
        "Account Name: Savings,   Repeats: Monthly\n"
        "Amount: 100,   Payee: Supermarket\n");
    
    mmBDDialog* dlg = new mmBDDialog(m_base_frame, 0, false, false);
    if (dlg->ShowModal() == wxID_OK)
    {
        Model_Billsdeposits::Data_Set bill_table = Model_Billsdeposits::instance().all();
        CPPUNIT_ASSERT(bill_table.size() > 0);

        Model_Billsdeposits::Data bill_entry = bill_table.at(bill_table.size() - 1);
        CPPUNIT_ASSERT(bill_entry.ACCOUNTID == Model_Account::instance().get("Savings")->ACCOUNTID);
        CPPUNIT_ASSERT(bill_entry.NEXTOCCURRENCEDATE == wxDateTime(wxDateTime::Today()).FormatISODate());
        CPPUNIT_ASSERT(bill_entry.TRANSAMOUNT == 100);
        CPPUNIT_ASSERT(bill_entry.TOTRANSAMOUNT == 100);
        CPPUNIT_ASSERT(bill_entry.REPEATS == Model_Billsdeposits::REPEAT_MONTHLY);
    }
}

void Test_BillsDeposits::test_edit_simple_entry()
{
    Model_Billsdeposits::Data_Set bill_table = Model_Billsdeposits::instance().all();
    if (bill_table.size() < 1) return;

    m_user_request->Show_InfoBarMessage(
        "Please confirm Simple entry - Edit\n\n"
        "Account Name: Savings,   Repeats: Monthly\n"
        "Amount: 100,   Payee: Supermarket\n"
        "Category: Food/Groceries\n\n"
        "Modify Amount: 200\n");

    Model_Billsdeposits::Data bill_entry = bill_table.at(bill_table.size() - 1);
    mmBDDialog* dlg = new mmBDDialog(m_base_frame, bill_entry.BDID, true, false);
    if (dlg->ShowModal() == wxID_OK)
    {
        bill_table = Model_Billsdeposits::instance().all();
        bill_entry = bill_table.at(bill_table.size() - 1);
        CPPUNIT_ASSERT(bill_entry.ACCOUNTID == Model_Account::instance().get("Savings")->ACCOUNTID);
        CPPUNIT_ASSERT(bill_entry.NEXTOCCURRENCEDATE == wxDateTime(wxDateTime::Today()).FormatISODate());
        CPPUNIT_ASSERT(bill_entry.TRANSAMOUNT == 200);
        CPPUNIT_ASSERT(bill_entry.TOTRANSAMOUNT == 200);
        CPPUNIT_ASSERT(bill_entry.REPEATS == Model_Billsdeposits::REPEAT_MONTHLY);
    }
}

void Test_BillsDeposits::test_enter_simple_entry()
{
    Model_Billsdeposits::Data_Set bill_table = Model_Billsdeposits::instance().all();
    if (bill_table.size() < 1) return;

    m_user_request->Show_InfoBarMessage(
        "Please confirm Simple entry - Enter\n\n"
        "Account Name: Savings,   Repeats: Monthly\n"
        "Amount: 200,   Payee: Supermarket\n"
        "Category: Food/Groceries\n");

    Model_Billsdeposits::Data bill_entry = bill_table.at(bill_table.size() - 1);
    mmBDDialog* dlg = new mmBDDialog(m_base_frame, bill_entry.BDID, false, true);
    if (dlg->ShowModal() == wxID_OK)
    {
        bill_table = Model_Billsdeposits::instance().all();
        bill_entry = bill_table.at(bill_table.size() - 1);
        CPPUNIT_ASSERT(bill_entry.REPEATS == Model_Billsdeposits::REPEAT_MONTHLY);
        wxDateTime next_date = wxDateTime(wxDateTime::Today()).Add(wxDateSpan::Month());
        CPPUNIT_ASSERT(bill_entry.NEXTOCCURRENCEDATE == next_date.FormatISODate());

        Model_Checking::Data_Set checking_table = Model_Checking::instance().all();
        CPPUNIT_ASSERT(checking_table.size() > 0);

        Model_Checking::Data checking_entry = checking_table.at(checking_table.size() - 1);
        CPPUNIT_ASSERT(checking_entry.ACCOUNTID == Model_Account::instance().get("Savings")->ACCOUNTID);
        CPPUNIT_ASSERT(checking_entry.TRANSAMOUNT == 200);
        CPPUNIT_ASSERT(checking_entry.TOTRANSAMOUNT == 200);
    }
}

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
        Model_Billsdeposits::Data_Set bill_table = Model_Billsdeposits::instance().all();
        CPPUNIT_ASSERT(bill_table.size() > 0);

        Model_Billsdeposits::Data bill_entry = bill_table.at(bill_table.size() - 1);
        CPPUNIT_ASSERT(bill_entry.ACCOUNTID == Model_Account::instance().get("Savings")->ACCOUNTID);
        CPPUNIT_ASSERT(bill_entry.NEXTOCCURRENCEDATE == wxDateTime(wxDateTime::Today()).FormatISODate());
        CPPUNIT_ASSERT(bill_entry.TRANSAMOUNT == 100);
        CPPUNIT_ASSERT(bill_entry.TOTRANSAMOUNT == 50);
        CPPUNIT_ASSERT(bill_entry.REPEATS == Model_Billsdeposits::REPEAT_QUARTERLY);
    }
}

void Test_BillsDeposits::test_edit_transfer_entry()
{
    Model_Billsdeposits::Data_Set bill_table = Model_Billsdeposits::instance().all();
    if (bill_table.size() < 1) return;

    m_user_request->Show_InfoBarMessage(
        "Please confirm transfer entry - Edit\n\n"
        "Account Name: Savings,   Repeats: Quarterly\n"
        "Type: Transfer,   Advanced,   Amount: 100, 50\n"
        "To: Cheque        Category: Gifts\n");

    Model_Billsdeposits::Data bill_entry = bill_table.at(bill_table.size() - 1);
    mmBDDialog* dlg = new mmBDDialog(m_base_frame, bill_entry.BDID, true, false);
    if (dlg->ShowModal() == wxID_OK)
    {
        bill_table = Model_Billsdeposits::instance().all();
        bill_entry = bill_table.at(bill_table.size() - 1);
        CPPUNIT_ASSERT(bill_entry.ACCOUNTID == Model_Account::instance().get("Savings")->ACCOUNTID);
        CPPUNIT_ASSERT(bill_entry.NEXTOCCURRENCEDATE == wxDateTime(wxDateTime::Today()).FormatISODate());
        CPPUNIT_ASSERT(bill_entry.TRANSAMOUNT == 100);
        CPPUNIT_ASSERT(bill_entry.TOTRANSAMOUNT == 50);
        CPPUNIT_ASSERT(bill_entry.REPEATS == Model_Billsdeposits::REPEAT_QUARTERLY);
    }
}

void Test_BillsDeposits::test_enter_transfer_entry()
{
    Model_Billsdeposits::Data_Set bill_table = Model_Billsdeposits::instance().all();
    if (bill_table.size() < 1) return;

    m_user_request->Show_InfoBarMessage(
        "Please confirm transfer entry - Enter\n\n"
        "Account Name: Savings,   Repeats: Quarterly\n"
        "Type: Transfer,   Advanced,   Amount: 100, 50\n"
        "To Account: Cheque   Category: Gifts\n");

    Model_Billsdeposits::Data bill_entry = bill_table.at(bill_table.size() - 1);
    mmBDDialog* dlg = new mmBDDialog(m_base_frame, bill_entry.BDID, false, true);
    if (dlg->ShowModal() == wxID_OK)
    {
        bill_table = Model_Billsdeposits::instance().all();
        bill_entry = bill_table.at(bill_table.size() - 1);
        CPPUNIT_ASSERT(bill_entry.REPEATS == Model_Billsdeposits::REPEAT_QUARTERLY);
        wxDateTime next_date = wxDateTime(wxDateTime::Today()).Add(wxDateSpan::Months(3));
        CPPUNIT_ASSERT(bill_entry.NEXTOCCURRENCEDATE == next_date.FormatISODate());

        Model_Checking::Data_Set checking_table = Model_Checking::instance().all();
        CPPUNIT_ASSERT(checking_table.size() > 0);

        Model_Checking::Data checking_entry = checking_table.at(checking_table.size() - 1);
        CPPUNIT_ASSERT(checking_entry.ACCOUNTID == Model_Account::instance().get("Savings")->ACCOUNTID);
        CPPUNIT_ASSERT(checking_entry.TRANSAMOUNT == 100);
        CPPUNIT_ASSERT(checking_entry.TOTRANSAMOUNT == 50);
    }
}

void Test_BillsDeposits::test_new_split_entry()
{
    m_user_request->Show_InfoBarMessage(
        "Please create a new Split entry\n\n"
        "Account Name: Savings,   Repeats: Monthly\n"
        "Amount: 150,             Payee: Supermarket,   Split\n"
        "Add Deposit 50,          Category: Bills/Water\n");

    mmBDDialog* dlg = new mmBDDialog(m_base_frame, 0, false, false);
    if (dlg->ShowModal() == wxID_OK)
    {
        Model_Billsdeposits::Data_Set bill_table = Model_Billsdeposits::instance().all();
        CPPUNIT_ASSERT(bill_table.size() > 0);

        // check the data in the bill table
        Model_Billsdeposits::Data bill_entry = bill_table.at(bill_table.size() - 1);
        CPPUNIT_ASSERT(bill_entry.ACCOUNTID == Model_Account::instance().get("Savings")->ACCOUNTID);
        CPPUNIT_ASSERT(bill_entry.NEXTOCCURRENCEDATE == wxDateTime(wxDateTime::Today()).FormatISODate());
        CPPUNIT_ASSERT(bill_entry.TRANSAMOUNT == 100);
        CPPUNIT_ASSERT(bill_entry.TOTRANSAMOUNT == 100);
        CPPUNIT_ASSERT(bill_entry.REPEATS == Model_Billsdeposits::REPEAT_MONTHLY);
        CPPUNIT_ASSERT(bill_entry.CATEGID == -1);
        CPPUNIT_ASSERT(bill_entry.SUBCATEGID == -1);

        Model_Budgetsplittransaction::Data_Set bill_split_subtable = Model_Budgetsplittransaction::instance().find(Model_Budgetsplittransaction::TRANSID(bill_entry.id()));
        CPPUNIT_ASSERT(bill_split_subtable.size() == 2);

        CPPUNIT_ASSERT(bill_split_subtable[0].SPLITTRANSAMOUNT == 150);
        CPPUNIT_ASSERT(bill_split_subtable[1].SPLITTRANSAMOUNT == -50);
        CPPUNIT_ASSERT(Model_Budgetsplittransaction::instance().get_total(bill_split_subtable) == 100);
    }
}

void Test_BillsDeposits::test_edit_split_entry()
{
    Model_Billsdeposits::Data_Set bill_table = Model_Billsdeposits::instance().all();
    if (bill_table.size() < 1) return;

    m_user_request->Show_InfoBarMessage(
        "Please confirm Split entry - Edit\n\n"
        "Account Name: Savings,   Repeats: Monthly\n"
        "Amount: 100 (Grey)   Payee: Supermarket,   Split\n"
        "Split Entries:"
        "Type: Withdrawal 150,   Category: Food/Groceries\n"
        "Type: Deposit 50,       Category: Bills/Water\n\n"
        "Modify Split Deposit to Split Withdrawal.");

    Model_Billsdeposits::Data bill_entry = bill_table.at(bill_table.size() - 1);
    mmBDDialog* dlg = new mmBDDialog(m_base_frame, bill_entry.BDID, true, false);
    if (dlg->ShowModal() == wxID_OK)
    {
        bill_table = Model_Billsdeposits::instance().all();
        bill_entry = bill_table.at(bill_table.size() - 1);
        CPPUNIT_ASSERT(bill_entry.ACCOUNTID == Model_Account::instance().get("Savings")->ACCOUNTID);
        CPPUNIT_ASSERT(bill_entry.NEXTOCCURRENCEDATE == wxDateTime(wxDateTime::Today()).FormatISODate());
        CPPUNIT_ASSERT(bill_entry.TRANSAMOUNT == 200);
        CPPUNIT_ASSERT(bill_entry.TOTRANSAMOUNT == 200);
        CPPUNIT_ASSERT(bill_entry.REPEATS == Model_Billsdeposits::REPEAT_MONTHLY);

        Model_Budgetsplittransaction::Data_Set bill_split_subtable = Model_Budgetsplittransaction::instance().find(Model_Budgetsplittransaction::TRANSID(bill_entry.id()));
        CPPUNIT_ASSERT(bill_split_subtable.size() == 2);

        CPPUNIT_ASSERT(bill_split_subtable[0].SPLITTRANSAMOUNT == 150);
        CPPUNIT_ASSERT(bill_split_subtable[1].SPLITTRANSAMOUNT == 50);
        CPPUNIT_ASSERT(Model_Budgetsplittransaction::instance().get_total(bill_split_subtable) == 200);

        Model_Splittransaction::Data_Set checking_split_subtable = Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(bill_entry.id()));
        CPPUNIT_ASSERT(checking_split_subtable.size() == 0);
    }
}

void Test_BillsDeposits::test_enter_split_entry()
{
    Model_Billsdeposits::Data_Set bill_table = Model_Billsdeposits::instance().all();
    if (bill_table.size() < 1) return;

    m_user_request->Show_InfoBarMessage(
        "Please confirm Split entry - Enter\n\n"
        "Account Name: Savings,   Repeats: Monthly\n"
        "Amount: 200 (Grey)       Payee: Supermarket,   Split\n"
        "Split Entries:\n"
        "Type: Withdrawal 150,    Category: Food/Groceries\n"
        "Type: Withdrawal 50,     Category: Bills/Water\n");

    Model_Billsdeposits::Data bill_entry = bill_table.at(bill_table.size() - 1);
    mmBDDialog* dlg = new mmBDDialog(m_base_frame, bill_entry.BDID, false, true);
    if (dlg->ShowModal() == wxID_OK)
    {
        bill_table = Model_Billsdeposits::instance().all();
        bill_entry = bill_table.at(bill_table.size() - 1);
        CPPUNIT_ASSERT(bill_entry.REPEATS == Model_Billsdeposits::REPEAT_MONTHLY);
        wxDateTime next_date = wxDateTime(wxDateTime::Today()).Add(wxDateSpan::Month());
        CPPUNIT_ASSERT(bill_entry.NEXTOCCURRENCEDATE == next_date.FormatISODate());

        Model_Checking::Data_Set checking_table = Model_Checking::instance().all();
        CPPUNIT_ASSERT(checking_table.size() > 0);

        Model_Checking::Data checking_entry = checking_table.at(checking_table.size() - 1);
        CPPUNIT_ASSERT(checking_entry.ACCOUNTID == Model_Account::instance().get("Savings")->ACCOUNTID);
        CPPUNIT_ASSERT(checking_entry.TRANSAMOUNT == 200);
        CPPUNIT_ASSERT(checking_entry.TOTRANSAMOUNT == 200);
        CPPUNIT_ASSERT(checking_entry.CATEGID == -1);
        CPPUNIT_ASSERT(checking_entry.SUBCATEGID == -1);

        Model_Splittransaction::Data_Set checking_split_subtable = Model_Splittransaction::instance().find(Model_Splittransaction::TRANSID(checking_entry.id()));
        CPPUNIT_ASSERT(checking_split_subtable.size() == 2);

        CPPUNIT_ASSERT(checking_split_subtable[0].SPLITTRANSAMOUNT == 150);
        CPPUNIT_ASSERT(checking_split_subtable[1].SPLITTRANSAMOUNT == 50);
        CPPUNIT_ASSERT(Model_Splittransaction::instance().get_total(checking_split_subtable) == 200);
        // Now check the split entries.

    }
}
//--------------------------------------------------------------------------
