/*******************************************************
Copyright (C) 2013 James Higley

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
#include "currency_test.h"
#include "mmCurrency.h"
#include "mmCurrencyFormatter.h"
#include <wx/string.h>
#include <cppunit/config/SourcePrefix.h>

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( CurrencyTest );

CurrencyTest::CurrencyTest()
{
}


CurrencyTest::~CurrencyTest()
{
}

void CurrencyTest::setUp()
{
	locale_.Init(wxLANGUAGE_ENGLISH);

	mmCurrency currency("USD", "United States dollar", L"$", L"", L"", L"", 100, 1);
	CurrencyFormatter::instance().loadSettings(currency);
}

void CurrencyTest::tearDown()
{
}

void CurrencyTest::formatDoubleToCurrency()
{
	double value1 = 1234.1415;
	double value2 = 0.0099;

	wxString s = CurrencyFormatter::float2Money(0);
	CPPUNIT_ASSERT(s == wxT("$0.00"));

	s = CurrencyFormatter::float2Money(-0);
	CPPUNIT_ASSERT(s == wxT("$0.00"));

	s = CurrencyFormatter::float2Money(1.0);
	CPPUNIT_ASSERT(s == wxT("$1.00"));

	s = CurrencyFormatter::float2Money(-12.3);
	CPPUNIT_ASSERT(s == wxT("$-12.30"));

	s = CurrencyFormatter::float2Money(123.4);
	CPPUNIT_ASSERT(s == wxT("$123.40"));

	s = CurrencyFormatter::float2Money(-1000.0);
	CPPUNIT_ASSERT(s == wxT("$-1,000.00"));

	s = CurrencyFormatter::float2Money(10000.0);
	CPPUNIT_ASSERT(s == wxT("$10,000.00"));

	s = CurrencyFormatter::float2Money(-100000.0);
	CPPUNIT_ASSERT(s == wxT("$-100,000.00"));

	s = CurrencyFormatter::float2Money(1000000.0);
	CPPUNIT_ASSERT(s == wxT("$1,000,000.00"));

	s = CurrencyFormatter::float2Money(-10000000.0);
	CPPUNIT_ASSERT(s == wxT("$-10,000,000.00"));

	s = CurrencyFormatter::float2Money(100000000.0);
	CPPUNIT_ASSERT(s == wxT("$100,000,000.00"));

	s = CurrencyFormatter::float2Money(1000000000.0);
	CPPUNIT_ASSERT(s == wxT("$1,000,000,000.00"));

	s = CurrencyFormatter::float2Money(-10000000000.0);
	CPPUNIT_ASSERT(s == wxT("$-10,000,000,000.00"));

	s = CurrencyFormatter::float2Money(100000000000.0);
	CPPUNIT_ASSERT(s == wxT("$100,000,000,000.00"));

	s = CurrencyFormatter::float2Money(-1234567890123.45);
	CPPUNIT_ASSERT(s == wxT("$-1,234,567,890,123.45"));

	s = CurrencyFormatter::float2Money(value1);
	CPPUNIT_ASSERT(s == wxT("$1,234.14"));

	s = CurrencyFormatter::float2Money(-value1);
	CPPUNIT_ASSERT(s == wxT("$-1,234.14"));

	s = CurrencyFormatter::float2Money(value2);
	CPPUNIT_ASSERT(s == wxT("$0.01"));

	s = CurrencyFormatter::float2Money(-value2);
	CPPUNIT_ASSERT(s == wxT("$-0.01"));

	s = CurrencyFormatter::float2Money(0.004);
	CPPUNIT_ASSERT(s == wxT("$0.00"));

	s = CurrencyFormatter::float2Money(0.005);
	CPPUNIT_ASSERT(s == wxT("$0.01"));
}
