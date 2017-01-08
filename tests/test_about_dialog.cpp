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
#include "framebase_tests.h"
//---------------------------------------------------------------------------
#include "test_about_dialog.h"
#include "aboutdialog.h"
#include "paths.h"

/*****************************************************************************
Turn test ON or OFF in file: defined_test_selection.h
*****************************************************************************/
#include "defined_test_selection.h"

#ifdef MMEX_TESTS_ABOUT_DIALOG
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( Test_About_Dialog );
#endif

static int s_instance_count = 0;
//----------------------------------------------------------------------------
Test_About_Dialog::Test_About_Dialog()
{
    s_instance_count++;
    m_this_instance = s_instance_count;
}

Test_About_Dialog::~Test_About_Dialog()
{
    s_instance_count--;
}

void Test_About_Dialog::setUp()
{
    m_base_frame = new TestFrameBase(m_this_instance);
    m_base_frame->Show(true);
}

void Test_About_Dialog::tearDown()
{
    /**
    * Note: If the frame is not deleted here, it will
    * remain active until the console window closes.
    */    
    delete m_base_frame;
}

void Test_About_Dialog::test_about_dialog()
{
    wxString file_name = mmex::getPathDoc(mmex::F_CONTRIB);
    if (wxFileExists(file_name))
    {
        mmAboutDialog* dlg = new mmAboutDialog(m_base_frame, 4);
        int id = dlg->ShowModal();
        if (id == wxID_OK)
        {
            wxMessageBox("About is OK", "MMEX Dialog Test", wxOK, wxTheApp->GetTopWindow());
        }
    }
    else
    {
        wxString msg = wxString() << "Cannot find file: " << file_name << "\n";
        wxMessageBox(msg, "MMEX Dialog Test", wxOK, wxTheApp->GetTopWindow());
    }
}
