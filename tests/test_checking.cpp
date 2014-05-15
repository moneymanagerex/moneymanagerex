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
#include "test_checking.h"
#include "transdialog.h"
#include "mmcheckingpanel.h"

/*****************************************************************************
Turn test ON or OFF in file: defined_test_selection.h
*****************************************************************************/
#include "defined_test_selection.h"

#ifdef MMEX_TESTS_CHECKING
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(Test_Checking);
#endif

static int s_instance_count = 0;
//----------------------------------------------------------------------------
Test_Checking::Test_Checking()
{
    s_instance_count++;
    m_this_instance = s_instance_count;
    m_test_db_filename = "test_db_model_checking.mmb";
}

Test_Checking::~Test_Checking()
{
    s_instance_count--;
    if (s_instance_count < 1)
    {
        wxRemoveFile(m_test_db_filename);
    }
}

void Test_Checking::setUp()
{
    CpuTimer time("Startup");
    

    // Only set up the base frame if test requires it.
    // wxWidgets will clean up the test frames on completion.
    if (m_this_instance > 2)
    {
        m_base_frame = new TestFrameBase(m_this_instance);
        m_base_frame->Show(true);
    }

    m_commit_hook = new CommitCallbackHook();
    m_test_db.Open(m_test_db_filename);
    m_test_db.SetCommitHook(m_commit_hook);

    m_dbmodel = new DB_Init_Model();
    m_dbmodel->Init_Model_Tables(&m_test_db);
    m_dbmodel->Init_BaseCurrency();
}

void Test_Checking::tearDown()
{
    delete m_dbmodel;

    m_test_db.SetCommitHook(0);
    m_test_db.Close();
    delete m_commit_hook;

    if (m_this_instance > 2)
    {
        delete m_base_frame;
    }
}

void Test_Checking::Set_UP_Database_conditions()
{
    CpuTimer Start("Set Up Database Conditions");
    m_test_db.Begin();
    // Add Accounts
    m_dbmodel->Add_Bank_Account("NAB - Savings", 1000, "Joint - General Account");
    m_dbmodel->Add_Term_Account("ANZ - Cash Manager", 0, "Savings for special ocasions, Higher Interest Account", false);
    m_dbmodel->Add_Term_Account("ANZ - Personal Loan", 0, "$10,000 @ 20% pa", false);

    m_dbmodel->Add_Bank_Account("ANZ - Cheque", 0, "", false);
    m_dbmodel->Add_Term_Account("Home Loan", -500000, "House Mortgage", false);

    m_dbmodel->Add_Bank_Account("ANZ - Mastercard", 0, "Credit Card");
    m_dbmodel->Add_Bank_Account("Wallet - Peter", 0, "Cash Money - Daily Expenses");
    m_dbmodel->Add_Bank_Account("Wallet - Mary", 0, "Cash Money - Daily Expenses");

    m_dbmodel->Add_Investment_Account("ABC Corporation", 0, "Shares");
    m_dbmodel->Add_Investment_Account("Acme Corporation", 0, "Shares");
    m_dbmodel->Add_Term_Account("Insurance Policies");

    // Add Payees
    m_dbmodel->Add_Payee("Supermarket", "Food", "Groceries");
    m_dbmodel->Add_Payee("Aldi", "Food", "Groceries");
    m_dbmodel->Add_Payee("Coles", "Food", "Groceries");
    m_dbmodel->Add_Payee("Woolworths", "Food", "Groceries");
    m_dbmodel->Add_Payee("Peter", "Income", "Salary");
    m_dbmodel->Add_Payee("Mary", "Income", "Salary");
    m_dbmodel->Add_Payee("Bank - NAB");
    m_dbmodel->Add_Payee("Bank - ANZ");
    m_dbmodel->Add_Payee("Government Authorities");
    m_dbmodel->Add_Payee("Utility Provider");
    m_dbmodel->Add_Payee("Cash - Miscellaneous");

    // Category structure: "Family Home"
    int cat_id = m_dbmodel->Add_Category("Home");
    m_dbmodel->Add_Subcategory(cat_id, "General Rates");
    m_dbmodel->Add_Subcategory(cat_id, "Water Rates");
    m_dbmodel->Add_Subcategory(cat_id, "Electricity");
    m_dbmodel->Add_Subcategory(cat_id, "Gas");
    m_dbmodel->Add_Subcategory(cat_id, "Phone/Internet");
    m_dbmodel->Add_Subcategory(cat_id, "Insurance");
    m_dbmodel->Add_Subcategory(cat_id, "Loan Repayments");
    m_dbmodel->Add_Subcategory(cat_id, "Loan Offset");
    m_dbmodel->Add_Subcategory(cat_id, "Loan Interest");

    // Category structure: "Mastercard"
    cat_id = m_dbmodel->Add_Category("Mastercard");
    m_dbmodel->Add_Subcategory(cat_id, "Repayment");
    m_dbmodel->Add_Subcategory(cat_id, "Annual Fee");

    // Category structure: "Personal Loan"
    cat_id = m_dbmodel->Add_Category("Personal Loan");
    m_dbmodel->Add_Subcategory(cat_id, "Repayments");
    m_dbmodel->Add_Subcategory(cat_id, "Offset");
    m_dbmodel->Add_Subcategory(cat_id, "Interest");

    cat_id = m_dbmodel->Get_category_id("Income");
    m_dbmodel->Add_Subcategory(cat_id, "Tax");
    m_dbmodel->Add_Subcategory(cat_id, "Bank Interest");

    m_test_db.Commit();
}

void Test_Checking::Add_Transactions()
{
    CpuTimer Start("Add_Transactions");
    // Set date 3 years ago from today.
    wxDateTime starting_date(wxDateTime::Today().Subtract(wxDateSpan::Years(3)));

    // Advance or retard the date to the beginning of that financial year.
    int month = starting_date.GetMonth();
    if (month > 6)
    {
        starting_date.Subtract(wxDateSpan::Months(month - 6));
    }
    else starting_date.Subtract(wxDateSpan::Year()).Add(wxDateSpan::Months(6 - month));
    starting_date.Subtract(wxDateSpan::Days(starting_date.GetDay() - 1));

    // Set all data to memory first, then save to database at end.
    m_test_db.Begin();

    wxDateTime trans_date = starting_date;

    // Setting up a personal loan
    int personal_loan_id = m_dbmodel->Add_Trans_Transfer("ANZ - Personal Loan", trans_date, "ANZ - Cheque", 10000, "Transfer");
    Model_Checking::Data* personal_loan = Model_Checking::instance().get(personal_loan_id);
    personal_loan->NOTES = "Initialise $10,000 Personal loan from ANZ -Bank";
    Model_Checking::instance().save(personal_loan);

    // Create transactions for a single month. These are repeated untill current month.
    int month_count = 0;
    while (starting_date < wxDateTime::Today())
    {
        month_count++;
        trans_date = starting_date;
        m_dbmodel->Add_Trans_Deposit("NAB - Savings", trans_date.Add(wxDateSpan::Days(8)), "Peter", 1200.0, "Income", "Salary");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date, "Peter", 300.0, "Income", "Tax");
        m_dbmodel->Add_Trans_Deposit("NAB - Savings", trans_date.Add(wxDateSpan::Days(7)), "Peter", 1500.0, "Income", "Salary");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date, "Peter", 375.0, "Income", "Tax");
        m_dbmodel->Add_Trans_Deposit("NAB - Savings", trans_date.Add(wxDateSpan::Days(7)), "Peter", 1200.0, "Income", "Salary");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date, "Peter", 300.0, "Income", "Tax");
        m_dbmodel->Add_Trans_Deposit("NAB - Savings", trans_date.Add(wxDateSpan::Days(7)), "Peter", 1200.0, "Income", "Salary");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date, "Peter", 300.0, "Income", "Tax");
        //--------------------------------------------------------------------

        trans_date = starting_date;
        m_dbmodel->Add_Trans_Deposit("NAB - Savings", trans_date.Add(wxDateSpan::Days(14)), "Mary", 1600.0, "Income", "Salary");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date, "Mary", 375.0, "Income", "Tax");
        m_dbmodel->Add_Trans_Deposit("NAB - Savings", trans_date.Add(wxDateSpan::Days(14)), "Mary", 1700.0, "Income", "Salary");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date, "Mary", 375.0, "Income", "Tax");
        //--------------------------------------------------------------------

        trans_date = starting_date;

        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date.Add(wxDateSpan::Days(1)), "Aldi", 50, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date.Add(wxDateSpan::Days(2)), "Coles", 30, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date.Add(wxDateSpan::Days(3)), "Woolworths", 40, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date.Add(wxDateSpan::Days(3)), "Supermarket", 100, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date.Add(wxDateSpan::Days(2)), "Coles", 30, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date.Add(wxDateSpan::Days(3)), "Aldi", 40, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date.Add(wxDateSpan::Days(2)), "Coles", 60, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date.Add(wxDateSpan::Days(3)), "Supermarket", 80, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date.Add(wxDateSpan::Days(2)), "Woolworths", 30, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date.Add(wxDateSpan::Days(3)), "Coles", 40, "Food", "Groceries");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date.Add(wxDateSpan::Days(3)), "Supermarket", 50, "Food", "Groceries");
        //--------------------------------------------------------------------

        trans_date = starting_date;
        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date.Add(wxDateSpan::Days(1)), "Wallet - Peter", 70, "Transfer");
        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date, "Wallet - Mary", 80, "Transfer");

        m_dbmodel->Add_Trans_Withdrawal("Wallet - Peter", trans_date.Add(wxDateSpan::Days(7)), "Cash - Miscellaneous", 70, "Miscellaneous");
        m_dbmodel->Add_Trans_Withdrawal("Wallet - Mary", trans_date, "Cash - Miscellaneous", 80, "Miscellaneous");

        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date, "Wallet - Peter", 70, "Transfer");
        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date, "Wallet - Mary", 100, "Transfer");

        m_dbmodel->Add_Trans_Withdrawal("Wallet - Peter", trans_date.Add(wxDateSpan::Days(7)), "Cash - Miscellaneous", 70, "Miscellaneous");
        m_dbmodel->Add_Trans_Withdrawal("Wallet - Mary", trans_date, "Cash - Miscellaneous", 100, "Miscellaneous");

        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date, "Wallet - Peter", 60, "Transfer");
        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date, "Wallet - Mary", 90, "Transfer");

        // Add split Transactions
        int split_id = m_dbmodel->Add_Trans_Withdrawal("Wallet - Peter", trans_date.Add(wxDateSpan::Days(8)), "Cash - Miscellaneous", 60);
        m_dbmodel->Add_Trans_Split(split_id, 20, "Leisure", "Magazines");
        m_dbmodel->Add_Trans_Split(split_id, 40, "Miscellaneous");

        split_id = m_dbmodel->Add_Trans_Withdrawal("Wallet - Mary", trans_date, "Cash - Miscellaneous", 90);
        m_dbmodel->Add_Trans_Split(split_id, 30, "Leisure", "Video Rental");
        m_dbmodel->Add_Trans_Split(split_id, 20, "Miscellaneous");
        m_dbmodel->Add_Trans_Split(split_id, 40, "Healthcare", "Health");

        trans_date = starting_date;
        // Quarterley Transactions
        if ((trans_date.GetMonth() == 3) || (trans_date.GetMonth() == 6) || (trans_date.GetMonth() == 9) || (trans_date.GetMonth() == 12))  // December
        {
            m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date, "Government Authorities", 250, "Home", "Water Rates");
        }

        trans_date = starting_date;
        // Yearly Transactions - 1
        if ((trans_date.GetMonth() == 6))
        {
            m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date.Add(wxDateSpan::Days(14)), "Utility Provider", 400, "Home", "Electricity");
            m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date, "Utility Provider", 700, "Home", "Gas");
        }

        trans_date = starting_date;
        // Yearly Transactions - 2
        if ((trans_date.GetMonth() == 12))
        {
            m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date.Add(wxDateSpan::Days(14)), "Utility Provider", 200, "Home", "Electricity");
            m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date, "Utility Provider", 50, "Home", "Gas");
        }

        trans_date = starting_date;
        // Yearly Transactions - 3
        if ((trans_date.GetMonth() == 8))   // August
        {
            m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date.Add(wxDateSpan::Days(12)), "Government Authorities", 2200, "Home", "General Rates");
        }

        trans_date = starting_date;
        // Six Monthly Transactions - 1
        if ((trans_date.GetMonth() == 3) || (trans_date.GetMonth() == 9)) // March & September
        {
            m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date.Add(wxDateSpan::Days(14)), "Utility Provider", 300, "Home", "Electricity");
            m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date, "Utility Provider", 400, "Home", "Gas");
        }
        //--------------------------------------------------------------------

        // End of Month Transactions
        trans_date = starting_date;
        trans_date.SetToLastMonthDay();

        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date, "ANZ - Mastercard", 625.0, "Mastercard", "Repayment");
        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date, "Bank - NAB", 3500, "Home", "Loan Repayments");
        m_dbmodel->Add_Trans_Deposit("Home Loan", trans_date, "Bank - NAB", 3500, "Home", "Loan Offset");
        m_dbmodel->Add_Trans_Withdrawal("Home Loan", trans_date, "Bank - NAB", 500, "Home", "Loan Interest");

        m_dbmodel->Add_Trans_Withdrawal("ANZ - Mastercard", trans_date, "Utility Provider", 150, "Home", "Phone/Internet");

        m_dbmodel->Add_Trans_Withdrawal("NAB - Savings", trans_date, "Bank - ANZ", 180, "Personal Loan", "Repayments");
        m_dbmodel->Add_Trans_Deposit("ANZ - Personal Loan", trans_date, "Bank - ANZ", 180, "Personal Loan", "Offset");
        m_dbmodel->Add_Trans_Withdrawal("ANZ - Personal Loan", trans_date, "Bank - ANZ", ((10000 - (180 * month_count)) * 0.20) / 12, "Personal Loan", "Interest");

        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date, "ANZ - Cheque", 100, "Transfer");
        m_dbmodel->Add_Trans_Transfer("NAB - Savings", trans_date, "ANZ - Cash Manager", 250, "Transfer");
        m_dbmodel->Add_Trans_Deposit("ANZ - Cash Manager", trans_date, "Bank - ANZ", (250 * month_count * .05) / 12, "Income", "Bank Interest");
        // -------------------------------------------------------------------

        // Set start of next month transactions
        starting_date.Add(wxDateSpan::Month());
    }
    // Finalise the database entries.
    m_test_db.Commit();
}

void Test_Checking::Transaction_Dialog_Messages()
{
    TestFrameBase* user_request = new TestFrameBase(m_base_frame);
    user_request->Show();

    int account_id = m_dbmodel->Get_account_id("NAB - Savings");
    bool testing = true;
    int test_count = 0;
    while (testing)
    {
        test_count++;
        switch (test_count)
        {
            case 1:
            {
                user_request->Show_InfoBarMessage(
                    "1. Use the OK button to observe error messages.\n"
                    "2. Add appropriate details as required.\n\n"
                    "Use Cancel to exit dialog.");
                mmTransDialog test_dialog(m_base_frame, account_id, 0);
                if (wxID_CANCEL == test_dialog.ShowModal())
                {
                    testing = false;
                }

                break;
            }

            case 2:
            {
                user_request->Show_InfoBarMessage(
                    "Set the Type to: Transfer\n\n"
                    "1. Use the OK button to observe error messages.\n"
                    "2. Add appropriate details as required.\n\n"
                    "Use Cancel to exit dialog.");
                mmTransDialog test_dialog(m_base_frame, account_id, 0);
                if (wxID_CANCEL == test_dialog.ShowModal())
                {
                    testing = false;
                }
                break;
            }
            default:
            {
                Model_Checking::Data_Set table = Model_Checking::instance().all();
                Model_Checking::Data trans_entry = table.at(table.size() - 1);

                user_request->Show_InfoBarMessage("This Test Completed\n\n Use Cancel to proceed to next test.");
                mmTransDialog test_dialog(m_base_frame, account_id, trans_entry.id());
                if (wxID_CANCEL == test_dialog.ShowModal())
                {
                    testing = false;
                }
            }
        }
    }
}

void Test_Checking::Transaction_New_Edit()
{
    TestFrameBase* user_request = new TestFrameBase(m_base_frame);
    user_request->Show();

    int account_id = m_dbmodel->Get_account_id("NAB - Savings");
    bool testing = true;
    int test_count = 0;
    while (testing)
    {
        test_count++;
        switch (test_count)
        {
            case 1:
            {
                user_request->Show_InfoBarMessage(
                      "Create a new 'Transfer' transaction.: \n\n"
                      "1. Set the category to Mastercard/Repayment\n"
                      "   This transaction will be used for editing.\n"
                      "2. Add appropriate details as required.\n\n"
                      "Use Cancel to exit dialog.\n");
                mmTransDialog test_dialog(m_base_frame, account_id, 0);
                if (wxID_CANCEL == test_dialog.ShowModal())
                {
                    user_request->Show_InfoBarMessage("Test Completed\n\n Refer back to console.");
                    testing = false;
                }
                break;
            }
            default:
            {
                Model_Checking::Data_Set table = Model_Checking::instance().all();
                Model_Checking::Data trans_entry = table.at(table.size() - 1);
                user_request->Show_InfoBarMessage(
                    "Edit the 'Transfer' transaction.: \n\n"
                    "1. Check that the category is Mastercard/Repayment\n"
                    "2. Add appropriate details as required.\n\n"
                    "Use Cancel to exit dialog.\n");
                mmTransDialog test_dialog(m_base_frame, account_id, trans_entry.id());
                if (wxID_CANCEL == test_dialog.ShowModal())
                {
                    user_request->Show_InfoBarMessage("Test Completed\n\n Refer back to console.");
                    testing = false;
                }
            }
        }
    }
}

void Test_Checking::Account_View_Savings()
{
    TestFrameBase* user_request = new TestFrameBase(m_base_frame);
    user_request->Show();

    // Create a new frame anchored to the base frame.
    TestFrameBase* account_frame = new TestFrameBase(m_base_frame, 670, 400);
    account_frame->Show();

    int account_id = m_dbmodel->Get_account_id("NAB - Savings");

    CpuTimer* panel_timer = new CpuTimer("Account_View: NAB - Savings");
    // Create the panel under test
    mmCheckingPanel* account_panel = new mmCheckingPanel(account_id, account_frame);
    account_panel->Show();
    delete panel_timer; // Kill the timer to determine speed of activation.

    // Anchor the panel. Otherwise it will disappear.
    wxMessageBox("Please Examine: Account View."
        "\n\nNAB - Savings\n\nContinue other tests ...",
        "Testing Account: Savings", wxOK, wxTheApp->GetTopWindow());
}

void Test_Checking::Account_View_Mary()
{
    TestFrameBase* user_request = new TestFrameBase(m_base_frame);
    user_request->Show();

    // Create a new frame anchored to the base frame.
    TestFrameBase* account_frame = new TestFrameBase(m_base_frame, 670, 400);
    account_frame->Show();

    int account_id = m_dbmodel->Get_account_id("Wallet - Mary");

    CpuTimer* panel_timer = new CpuTimer("Account_View: Wallet - Mary");
    // Create the panel under test
    mmCheckingPanel* account_panel = new mmCheckingPanel(account_id, account_frame);
    account_panel->Show();
    delete panel_timer; // Kill the timer to determine speed of activation.

    // Anchor the panel. Otherwise it will disappear.
    wxMessageBox("Please Examine: Account View."
        "\n\nWallet - Mary.\n\nContinue other tests ...",
        "Testing Account: Mary", wxOK, wxTheApp->GetTopWindow());
}
//--------------------------------------------------------------------------
