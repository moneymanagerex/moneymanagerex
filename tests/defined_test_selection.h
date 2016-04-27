/*****************************************************************************
File:           defined_test_selection.h

Author:         Stefano Giorgio ( Jan 2014)
Description:    This identifies all the tests in this test environment.
				It simplifies the selection of any desired test.

Purpose:        To enable or disable tests from a single location.
Action:         Comment out any unwanted test.
******************************************************************************/
#pragma once
//#define MMEX_TESTS_DISPLAY_TIMING

#define MMEX_TESTS_TEST_DATE_RANGE
#define MMEX_TESTS_TEST_DATABASE_INITIALISATION

#define MMEX_TEST_ABOUT_DIALOG
#define MMEX_TEST_ASSETS
#define MMEX_TEST_BILLS_DEPOSITS
#define MMEX_TEST_CALLBACK_HOOKS
#define MMEX_TEST_CHECKING
#define MMEX_TEST_CURRENCY
#define MMEX_TEST_FILTER_TRANS_DIALOG
#define MMEX_TEST_RELOCATE_CATEGORY
#define MMEX_TEST_RELOCATE_PAYEE
#define MMEX_TEST_OPTIONS_DIALOG
#define MMEX_TEST_STOCKS
#define MMEX_TEST_TRANSLINK
