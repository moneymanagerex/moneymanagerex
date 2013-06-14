/*******************************************************
Copyright (C) 2010 VaDiM

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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ********************************************************/

//----------------------------------------------------------------------------
#include <UnitTest++.h>
#include "testing_util.h"
#include "util.h"
//----------------------------------------------------------------------------
#ifdef UTIL_TEST_INCLUDED_IN_BUILD

#include <boost/unordered_map.hpp>
#include <wx/log.h>
//----------------------------------------------------------------------------

namespace
{

struct TestData
{
	TestData(int i, wxString n): id(i), name(n) {}
	int id;
	wxString name;
};

void dumpDates(wxString mgs, wxDateTime dateStart, wxDateTime dateEnd)
{
	wxLogDebug(mgs.c_str());
	wxLogDebug((dateStart.FormatISODate() + wxT('-') + dateStart.FormatISOTime()).c_str());
	wxLogDebug(wxT(" - "));
	wxLogDebug((dateEnd.FormatISODate() + wxT('-') + dateEnd.FormatISOTime()).c_str());
	wxLogDebug(wxT("\n"));
}

} // namespace

//----------------------------------------------------------------------------

SUITE(DateTimeProviders)
{

using namespace DateTimeProviders;

TEST(CustomDate)
{
    printf("\nutil_test: START");
    display_STD_IO_separation_line();

    typedef CustomDate<2010, wxDateTime::May, 1> Date_20100501;
	wxDateTime dateStart = Date_20100501::StartRange();
	wxDateTime dateEnd = Date_20100501::EndRange();
	CHECK(2010 == dateStart.GetYear());
	CHECK(2010 == dateEnd.GetYear());
	CHECK(wxDateTime::May == dateStart.GetMonth());
	CHECK(wxDateTime::May == dateEnd.GetMonth());
	CHECK(1 == dateStart.GetDay());
	CHECK(1 == dateEnd.GetDay());
}
//----------------------------------------------------------------------------

TEST(LastDays)
{
	typedef CustomDate<2010, wxDateTime::May, 1> Date_20100501;

	wxDateTime dateStart = LastDays<5, Date_20100501>::StartRange();
	wxDateTime dateEnd = LastDays<5, Date_20100501>::EndRange();
	dumpDates(wxT("LastDays [1]: "), dateStart, dateEnd);

	CHECK(!wxDateTime(26, wxDateTime::Apr, 2010, 23, 59, 59, 999).IsBetween(dateStart, dateEnd));
	CHECK(wxDateTime(27, wxDateTime::Apr, 2010, 0, 0, 0, 0).IsBetween(dateStart, dateEnd));
	CHECK(wxDateTime(1, wxDateTime::May, 2010, 23, 59, 59, 0).IsBetween(dateStart, dateEnd));
	CHECK(!wxDateTime(2, wxDateTime::May, 2010, 0, 0, 0, 0).IsBetween(dateStart, dateEnd));

	CHECK(2010 == dateStart.GetYear());
	CHECK(2010 == dateEnd.GetYear());
	CHECK(wxDateTime::Apr == dateStart.GetMonth());
	CHECK(wxDateTime::May == dateEnd.GetMonth());
	CHECK(27 == dateStart.GetDay());
	CHECK(1 == dateEnd.GetDay());

	dateStart = LastDays<1, Date_20100501>::StartRange();
	dateEnd = LastDays<1, Date_20100501>::EndRange();
	dumpDates(wxT("LastDays [2]: "), dateStart, dateEnd);

	CHECK(!wxDateTime(4, wxDateTime::May, 2010, 23, 59, 59, 999).IsBetween(dateStart, dateEnd));
	CHECK(wxDateTime(1, wxDateTime::May, 2010, 0, 0, 0, 0).IsBetween(dateStart, dateEnd));
	CHECK(wxDateTime(1, wxDateTime::May, 2010, 23, 59, 59, 0).IsBetween(dateStart, dateEnd));
	CHECK(!wxDateTime(6, wxDateTime::May, 2010, 0, 0, 0, 0).IsBetween(dateStart, dateEnd));

	CHECK(2010 == dateStart.GetYear());
	CHECK(2010 == dateEnd.GetYear());
	CHECK(wxDateTime::May == dateStart.GetMonth());
	CHECK(wxDateTime::May == dateEnd.GetMonth());
	CHECK(1 == dateStart.GetDay());
	CHECK(1 == dateEnd.GetDay());

	dateStart = LastDays<30, Date_20100501>::StartRange();
	dateEnd = LastDays<30, Date_20100501>::EndRange();
	dumpDates(wxT("LastDays [3]: "), dateStart, dateEnd);

	CHECK(!wxDateTime(1, wxDateTime::Apr, 2010, 23, 59, 59, 999).IsBetween(dateStart, dateEnd));
	CHECK(wxDateTime(2, wxDateTime::Apr, 2010, 0, 0, 0, 0).IsBetween(dateStart, dateEnd));
	CHECK(wxDateTime(1, wxDateTime::May, 2010, 23, 59, 59, 0).IsBetween(dateStart, dateEnd));
	CHECK(!wxDateTime(2, wxDateTime::May, 2010, 0, 0, 0, 0).IsBetween(dateStart, dateEnd));

	CHECK(2010 == dateStart.GetYear());
	CHECK(2010 == dateEnd.GetYear());
	CHECK(wxDateTime::Apr == dateStart.GetMonth());
	CHECK(wxDateTime::May == dateEnd.GetMonth());
	CHECK(2 == dateStart.GetDay());
	CHECK(1 == dateEnd.GetDay());
}
//----------------------------------------------------------------------------

TEST(CurrentMonth)
{
	typedef CustomDate<2010, wxDateTime::May, 1> Date_20100501;
	typedef CustomDate<2010, wxDateTime::Jun, 12> Date_20100612;
	typedef CustomDate<2010, wxDateTime::Oct, 26> Date_20101026;

	wxDateTime dateStart = CurrentMonth<Date_20100501>::StartRange();
	wxDateTime dateEnd = CurrentMonth<Date_20100501>::EndRange();	
	dumpDates(wxT("Current month [1]: "), dateStart, dateEnd);

	CHECK(!wxDateTime(30, wxDateTime::Apr, 2010, 23, 59, 59, 999).IsBetween(dateStart, dateEnd));
	CHECK(wxDateTime(1, wxDateTime::May, 2010, 0, 0, 0, 0).IsBetween(dateStart, dateEnd));
	CHECK(wxDateTime(31, wxDateTime::May, 2010, 23, 59, 59, 0).IsBetween(dateStart, dateEnd));
	CHECK(!wxDateTime(1, wxDateTime::Jun, 2010, 0, 0, 0, 0).IsBetween(dateStart, dateEnd));

	CHECK(2010 == dateStart.GetYear());
	CHECK(2010 == dateEnd.GetYear());
	CHECK(wxDateTime::May == dateStart.GetMonth());
	CHECK(wxDateTime::May == dateEnd.GetMonth());
	CHECK(1 == dateStart.GetDay());
	CHECK(31 == dateEnd.GetDay());

	dateStart = CurrentMonth<Date_20100612>::StartRange();
	dateEnd = CurrentMonth<Date_20100612>::EndRange();
	dumpDates(wxT("Current month [2]: "), dateStart, dateEnd);

	CHECK(!wxDateTime(31, wxDateTime::May, 2010, 23, 59, 59, 999).IsBetween(dateStart, dateEnd));
	CHECK(wxDateTime(1, wxDateTime::Jun, 2010, 0, 0, 0, 0).IsBetween(dateStart, dateEnd));
	CHECK(wxDateTime(30, wxDateTime::Jun, 2010, 23, 59, 59, 0).IsBetween(dateStart, dateEnd));
	CHECK(!wxDateTime(1, wxDateTime::Jul, 2010, 0, 0, 0, 0).IsBetween(dateStart, dateEnd));

	CHECK(2010 == dateStart.GetYear());
	CHECK(2010 == dateEnd.GetYear());
	CHECK(wxDateTime::Jun == dateStart.GetMonth());
	CHECK(wxDateTime::Jun == dateEnd.GetMonth());
	CHECK(1 == dateStart.GetDay());
	CHECK(30 == dateEnd.GetDay());

	dateStart = CurrentMonth<Date_20101026>::StartRange();
	dateEnd = CurrentMonth<Date_20101026>::EndRange();
	dumpDates(wxT("Current month [3]: "), dateStart, dateEnd);

	CHECK(!wxDateTime(30, wxDateTime::Sep, 2010, 23, 59, 59, 999).IsBetween(dateStart, dateEnd));
	CHECK(wxDateTime(1, wxDateTime::Oct, 2010, 0, 0, 0, 0).IsBetween(dateStart, dateEnd));
	CHECK(wxDateTime(30, wxDateTime::Oct, 2010, 23, 59, 59, 0).IsBetween(dateStart, dateEnd));
	CHECK(!wxDateTime(1, wxDateTime::Nov, 2010, 0, 0, 0, 0).IsBetween(dateStart, dateEnd));

	CHECK(2010 == dateStart.GetYear());
	CHECK(2010 == dateEnd.GetYear());
	CHECK(wxDateTime::Oct == dateStart.GetMonth());
	CHECK(wxDateTime::Oct == dateEnd.GetMonth());
	CHECK(1 == dateStart.GetDay());
	CHECK(31 == dateEnd.GetDay());
}
//----------------------------------------------------------------------------

TEST(LastMonths)
{
	typedef CustomDate<2010, wxDateTime::May, 1> Date_20100501;
	typedef CustomDate<2010, wxDateTime::Jun, 12> Date_20100612;
	typedef CustomDate<2010, wxDateTime::Oct, 26> Date_20101026;

	wxDateTime dateStart = LastMonths<1, 0, Date_20100501>::StartRange();
	wxDateTime dateEnd = LastMonths<1, 0, Date_20100501>::EndRange();	
	dumpDates(wxT("Last months [1]: "), dateStart, dateEnd);

	dateStart = LastMonths<2, 0, Date_20100501>::StartRange();
	dateEnd = LastMonths<2, 0, Date_20100501>::EndRange();	
	dumpDates(wxT("Last months [2]: "), dateStart, dateEnd);

	dateStart = LastMonths<2, 1, Date_20100501>::StartRange();
	dateEnd = LastMonths<2, 1, Date_20100501>::EndRange();	
	dumpDates(wxT("Last months [3]: "), dateStart, dateEnd);

	dateStart = LastMonths<0, 0, Date_20100501>::StartRange();
	dateEnd = LastMonths<0, 0, Date_20100501>::EndRange();	
	dumpDates(wxT("Last months [4]: "), dateStart, dateEnd);
}
//----------------------------------------------------------------------------

}

//----------------------------------------------------------------------------

SUITE(collections)
{

namespace
{

typedef boost::unordered_map<wxString, wxString> TransactionMatchMap;

const wxString s_view_reconciled(wxT("View Reconciled"));
const wxString s_view_void(wxT("View Void"));
const wxString s_view_flagged(wxT("View Flagged"));
const wxString s_view_unreconciled(wxT("View UnReconciled"));
const wxString s_view_not_reconciled(wxT("View Not-Reconciled"));
const wxString s_view_duplicates(wxT("View Duplicates"));

const TransactionMatchMap& initTransactionMatchMap()
{
	static TransactionMatchMap map;

	map[s_view_reconciled] = wxT("R");
	map[s_view_void] = wxT("V");
	map[s_view_flagged] = wxT("F");
	map[s_view_unreconciled] = wxT("");
	map[s_view_not_reconciled] = wxT("R");
	map[s_view_duplicates] = wxT("D");

	return map;
}

const TransactionMatchMap& s_transactionMatchers_Map = initTransactionMatchMap();

} // namespace

//----------------------------------------------------------------------------

TEST(BoostHashMap)
{
	TransactionMatchMap::const_iterator end;

	size_t size = s_transactionMatchers_Map.size();
	CHECK(size == 6);

	TransactionMatchMap::hasher hasher = s_transactionMatchers_Map.hash_function();
	std::size_t h1 = hasher(s_view_reconciled);
	std::size_t h2 = hasher(wxT("View Reconciled"));
	CHECK(h1 == h2);

	TransactionMatchMap::const_iterator result = s_transactionMatchers_Map.find(wxT("View Void"));
	CHECK(result != end);
}

//----------------------------------------------------------------------------

} // SUITE

//----------------------------------------------------------------------------

SUITE(util)
{

TEST(formatDoubleToCurrency1)
{
	mmex::CurrencyFormatter::instance().loadDefaultSettings();

	wxString s;
	
	mmex::formatDoubleToCurrency(0, s);
	CHECK(s == wxT("$ 0.00"));

	mmex::formatDoubleToCurrency(-0, s);
	CHECK(s == wxT("$ 0.00"));

	mmex::formatDoubleToCurrency(1.0, s);
	CHECK(s == wxT("$ 1.00"));

	mmex::formatDoubleToCurrency(-12.3, s);
	CHECK(s == wxT("$ -12.30"));

	mmex::formatDoubleToCurrency(123.4, s);
	CHECK(s == wxT("$ 123.40"));
	
	mmex::formatDoubleToCurrency(-1000.0, s);
	CHECK(s == wxT("$ -1,000.00"));

	mmex::formatDoubleToCurrency(10000.0, s);
	CHECK(s == wxT("$ 10,000.00"));

	mmex::formatDoubleToCurrency(-100000.0, s);
	CHECK(s == wxT("$ -100,000.00"));

	mmex::formatDoubleToCurrency(1000000.0, s);
	CHECK(s == wxT("$ 1,000,000.00"));

	mmex::formatDoubleToCurrency(-10000000.0, s);
	CHECK(s == wxT("$ -10,000,000.00"));

	mmex::formatDoubleToCurrency(100000000.0, s);
	CHECK(s == wxT("$ 100,000,000.00"));

	mmex::formatDoubleToCurrency(1000000000.0, s);
	CHECK(s == wxT("$ 1,000,000,000.00"));

	mmex::formatDoubleToCurrency(-10000000000.0, s);
	CHECK(s == wxT("$ -10,000,000,000.00"));

	mmex::formatDoubleToCurrency(100000000000.0, s);
	CHECK(s == wxT("$ 100,000,000,000.00"));

	mmex::formatDoubleToCurrency(-1234567890123.45, s);
	CHECK(s == wxT("$ -1,234,567,890,123.45"));
}
//----------------------------------------------------------------------------

TEST(formatDoubleToCurrency2)
{
	double val = 1234.1415;
	wxString s;

	mmex::formatDoubleToCurrency(val, s);
	CHECK(s == wxT("$ 1,234.14"));

	mmex::formatDoubleToCurrency(-val, s);
	CHECK(s == wxT("$ -1,234.14"));
}
//----------------------------------------------------------------------------

TEST(formatDoubleToCurrency3)
{
        double val = 0.0099;
	wxString s;

	mmex::formatDoubleToCurrency(val, s);
	CHECK(s == wxT("$ 0.01"));

	mmex::formatDoubleToCurrency(-val, s);
	CHECK(s == wxT("$ -0.01"));

	mmex::formatDoubleToCurrency(0.004, s);
	CHECK(s == wxT("$ 0.00"));

	mmex::formatDoubleToCurrency(0.005, s);
	CHECK(s == wxT("$ 0.01"));
}
//----------------------------------------------------------------------------

TEST(formatDoubleToCurrency4)
{
	wxString pfx = wxT("AAA");
	wxString sfx = wxT("ZZZ");
	wxChar dec = wxT('%');
	wxChar grp = wxT(':');
	wxString unit = wxT("baboses");
	wxString cent = wxT("kop.");
	double scale = 100;

	mmex::CurrencyFormatter::instance().loadSettings(pfx, sfx, dec, grp, unit, cent, scale);

	wxString s;
	mmex::formatDoubleToCurrency(-9123456789012.34, s);
	CHECK(s == wxT("AAA -9:123:456:789:012%34ZZZ"));
}
//----------------------------------------------------------------------------

TEST(formatDoubleToCurrencyEdit)
{
	wxString s;

	mmex::formatDoubleToCurrencyEdit(0, s);
	CHECK(s == wxT("0%00"));

	mmex::formatDoubleToCurrencyEdit(-9123456789012.34, s);
	CHECK(s == wxT("-9:123:456:789:012%34"));
}
//----------------------------------------------------------------------------

TEST(formatDoubleToCurrency5)
{
	wxString es;
	wxChar dec = wxT('\a'); // isprint() -> false
	wxChar grp = wxT('\0');

	mmex::CurrencyFormatter::instance().loadSettings(es, es, dec, grp, es, es, 1000);

	wxString s;
	mmex::formatDoubleToCurrency(12345670.895, s);
	CHECK(s == wxT("12345670.895"));

	// --

	mmex::CurrencyFormatter::instance().loadSettings(es, es, dec, grp, es, es, 10);

	s.clear();
	mmex::formatDoubleToCurrency(12345670.895, s);
	CHECK(s == wxT("12345670.9"));

	// --

	mmex::CurrencyFormatter::instance().loadSettings(es, es, dec, grp, es, es, 10000);

	s.clear();
	mmex::formatDoubleToCurrency(12345670.8954, s);
	CHECK(s == wxT("12345670.8954"));

	// --

	mmex::CurrencyFormatter::instance().loadSettings(es, es, dec, grp, es, es, 0);

	s.clear();
	mmex::formatDoubleToCurrency(12345670.89, s);
	CHECK(s == wxT("12345670.89"));

	// --

	mmex::CurrencyFormatter::instance().loadSettings(es, es, dec, grp, es, es, -100);

	s.clear();
	mmex::formatDoubleToCurrency(12345670.89, s);
	CHECK(s == wxT("12345670.89"));
}
//----------------------------------------------------------------------------

} // SUITE

//----------------------------------------------------------------------------
#endif UTIL_TEST_INCLUDED_IN_BUILD