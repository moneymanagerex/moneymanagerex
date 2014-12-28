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
#include "test_options_dialog.h"
#include "optionsdialog.h"
#include "mmOptionGeneralSettings.h"
#include "mmOptionMiscSettings.h"
#include "mmOptionNetSettings.h"
#include "mmOptionViewSettings.h"
#include "mmOptionAttachmentSettings.h"

/*****************************************************************************
Turn test ON or OFF in file: defined_test_selection.h
*****************************************************************************/
#include "defined_test_selection.h"

#ifdef MMEX_TESTS_OPTIONS_DIALOG
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( Test_Options_Dialog );
#endif

static int s_instance_count = 0;
//----------------------------------------------------------------------------
Test_Options_Dialog::Test_Options_Dialog()
{
    s_instance_count++;
    m_this_instance = s_instance_count;
    m_test_db_filename = "test_db_options.mmb";
}

Test_Options_Dialog::~Test_Options_Dialog()
{
    s_instance_count--;
    if (s_instance_count < 1)
    {
        wxRemoveFile(m_test_db_filename);
    }
}

void Test_Options_Dialog::setUp()
{
    CpuTimer time("Startup");
    m_base_frame = new TestFrameBase(m_this_instance);
    m_base_frame->Show(true);
    m_test_db.Open(m_test_db_filename);

    m_dbmodel = new DB_Init_Model();
    m_dbmodel->Init_Model_Tables(&m_test_db);
    m_dbmodel->Init_BaseCurrency();
}

void Test_Options_Dialog::tearDown()
{
    delete m_dbmodel;
    m_test_db.Close();
    delete m_base_frame;
}

void Test_Options_Dialog::test_dialog()
{
    TestFrameBase* user_request = new TestFrameBase(m_base_frame);
    user_request->Show();

    wxString info_message = wxString()
        << "Options Dialog tests:\n"
        << "\n\n"
        << "Use Cancel to ignore test results.";
    user_request->Show_InfoBarMessage(info_message);
    mmOptionsDialog dlg(m_base_frame, 0);
    if (dlg.ShowModal() == wxID_OK)
    {
        CPPUNIT_ASSERT(mmOptions::instance().language_ == "english");
        CPPUNIT_ASSERT(mmOptions::instance().userNameString_ == "Test Database");

        CPPUNIT_ASSERT(mmOptions::instance().financialYearStartDayString_ == "1");
        CPPUNIT_ASSERT(mmOptions::instance().financialYearStartMonthString_ == "7");
    }
}

void Test_Options_Dialog::test_general_panel()
{
    // Create a new frame anchored to the base frame.
    TestFrameBase* settings_frame = new TestFrameBase(m_base_frame, 400, 550);
    settings_frame->Show();

    // Create the panel under test
    mmOptionSettingsBase* settings_panel = new mmOptionGeneralSettings(settings_frame, wxID_ANY);
    settings_panel->Show();

    // Anchor the panel. Otherwise it will disappear.
    wxMessageBox("Please Examine: mmOptionGeneralSettings Panel.\n\nContinue other tests ...",
        "Testing: Settings Panel", wxOK, wxTheApp->GetTopWindow());

    settings_panel->SaveSettings();
    wxMessageBox("General Settings saved", "Option Settings Panel");
}

void Test_Options_Dialog::test_view_panel()
{
    // Create a new frame anchored to the base frame.
    TestFrameBase* settings_frame = new TestFrameBase(m_base_frame, 400, 550);
    settings_frame->Show();

    // Create the panel under test
    mmOptionSettingsBase* settings_panel = new mmOptionViewSettings(settings_frame, wxID_ANY);
    settings_panel->Show();

    // Anchor the panel. Otherwise it will disappear.
    wxMessageBox("Please Examine: mmOptionViewSettings Panel.\n\nContinue other tests ...",
        "Testing: Settings Panel", wxOK, wxTheApp->GetTopWindow());
    settings_panel->SaveSettings();
    wxMessageBox("View Settings saved", "Option Settings Panel");
}

void Test_Options_Dialog::test_network_panel()
{
    // Create a new frame anchored to the base frame.
    TestFrameBase* settings_frame = new TestFrameBase(m_base_frame, 400, 550);
    settings_frame->Show();

    // Create the panel under test
    mmOptionSettingsBase* settings_panel = new mmOptionNetSettings(settings_frame, wxID_ANY);
    settings_panel->Show();

    // Anchor the panel. Otherwise it will disappear.
    wxMessageBox("Please Examine: mmOptionNetSettings Panel.\n\nContinue other tests ...",
        "Testing: Settings Panel", wxOK, wxTheApp->GetTopWindow());
    settings_panel->SaveSettings();
    //wxMessageBox("Network Settings saved", "Option Settings Panel");
}

void Test_Options_Dialog::test_misc_panel()
{
    // Create a new frame anchored to the base frame.
    TestFrameBase* settings_frame = new TestFrameBase(m_base_frame, 400, 550);
    settings_frame->Show();

    // Create the panel under test
    mmOptionSettingsBase* settings_panel = new mmOptionMiscSettings(settings_frame, wxID_ANY);
    settings_panel->Show();

    // Anchor the panel. Otherwise it will disappear.
    wxMessageBox("Please Examine: mmOptionMiscSettings Panel.\n\nContinue other tests ...",
        "Testing: Settings Panel", wxOK, wxTheApp->GetTopWindow());
    settings_panel->SaveSettings();
    wxMessageBox("Misc Settings", "Option Settings Panel");
}

void Test_Options_Dialog::test_attachment_panel()
{
    // Create a new frame anchored to the base frame.
    TestFrameBase* settings_frame = new TestFrameBase(m_base_frame, 400, 550);
    settings_frame->Show();

    // Create the panel under test
    mmOptionSettingsBase* settings_panel = new mmOptionAttachmentSettings(settings_frame, wxID_ANY);
    settings_panel->Show();

    // Anchor the panel. Otherwise it will disappear.
    wxMessageBox("Please Examine: mmOptionAttachmentSettings Panel.\n\nContinue other tests ...",
        "Testing: Settings Panel", wxOK, wxTheApp->GetTopWindow());
    settings_panel->SaveSettings();
    //wxMessageBox("Attachment Settings", "Option Settings Panel");
}
