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
#include "model\Model_Currency.h"
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
}

void CurrencyTest::tearDown()
{
}

void CurrencyTest::formatDoubleToCurrency()
{
	double value = 0.0099;

    wxString s = Model_Currency::toString(value, 0);
    CPPUNIT_ASSERT(s == wxT("0.01"));

    s = Model_Currency::toString(-value, 0);
    CPPUNIT_ASSERT(s == wxT("-0.01"));
}
