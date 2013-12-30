/*******************************************************
Copyright (C) 2009 VaDiM
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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ********************************************************/

#include "../platfdep.h" // GetAppName
#include <wx/app.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
//----------------------------------------------------------------------------

/*
        I must create an instance of application because some 
        classes require one (for example, wxStandardPaths).
*/
int main(int /*argc*/, char const* /*argv*/[])
{
    wxAppInitializerFunction f = wxAppConsole::GetInitializerFunction();
    f(); // creates instance of application

    wxApp::GetInstance()->SetAppName(mmex::GetAppName());

	// Create the event manager and test controller
	CPPUNIT_NS::TestResult controller;

	// Add a listener that colllects test result
	CPPUNIT_NS::TestResultCollector result;
	controller.addListener( &result );        

	// Add a listener that print dots as test run.
	CPPUNIT_NS::BriefTestProgressListener progress;
	controller.addListener( &progress );      

	// Add the top suite to the test runner
	CPPUNIT_NS::TestRunner runner;
	runner.addTest( CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest() );
	try
	{
		runner.run( controller );

		// Print test in a compiler compatible format.
		CPPUNIT_NS::CompilerOutputter outputter( &result, CPPUNIT_NS::stdCOut() );
		outputter.write();

        // Pause command window. Allow user to review results from Windows and/or IDE
        std::cout << "\nPress ENTER to continue... ";
        std::cin.get();
	}
	catch ( std::invalid_argument &e )  // Test path not resolved
	{
		CPPUNIT_NS::stdCOut()	<<  "\n"  
								<<  "ERROR: "  <<  e.what()
								<< "\n";
		return 0;
	}

	return result.wasSuccessful() ? 0 : 1;
}
//----------------------------------------------------------------------------
