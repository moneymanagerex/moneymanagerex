/*******************************************************
Copyright (C) 2009 VaDiM

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

#include "constants.h"
#include <wx/string.h>
#include <wx/filefn.h>
#include "paths.h"

/*************************************************************************
 Revision of last commit: $Revision$
 Author of last commit:   $Author$

 When creating a build:
 Update the MMEX_REVISION_ID string to the latest SVN Revision first,
 OR
 Commit a change to the SVN repository, then create the build.
 *************************************************************************/
const wxString MMEX_REVISION_ID = "$Rev$";
//---------------------------------------------------------------------------
const int mmex::MIN_DATAVERSION = 2;
const wxChar * const mmex::DATAVERSION = wxT("2");
const wxChar *const mmex::DEFDATEFORMAT = wxT("%m/%d/%y");
const wxChar *const mmex::DEFDELIMTER = wxT(",");

// Using google: To specify the exchange, use exch:code
// Using yahoo: To specify the exchange, use code.exch

// const wxChar *const mmex::DEFSTOCKURL = "http://www.google.com/finance?q=%s";

// Will display the stock page when using Looks up the current value
const wxString mmex::DEFSTOCKURL = "http://finance.yahoo.com/echarts?s=%s";

// Looks up the current value
// const wxChar *const mmex::DEFSTOCKURL = "http://finance.yahoo.com/lookup?s=%s";

//US Dollar (USD) in Euro (EUR) Chart
//http://www.google.com/finance?q=CURRENCY%3AUSD

//----------------------------------------------------------------------------

wxString mmex::getProgramName()
{
    return wxString("MoneyManagerEx");
}
//----------------------------------------------------------------------------

wxString mmex::getProgramVersion()
{
    wxString revision(MMEX_REVISION_ID);
    revision.Replace("$", wxEmptyString);

/**************************************************
 Refer to comments in the file: constants.h
 **************************************************/
#ifndef _MM_EX_BUILD_TYPE_RELEASE
    revision.Replace("Rev: ", "DEV:SVN-");
#endif

    revision.Trim();
    return wxString::Format("0.9.9.2  %s", revision);
}
//----------------------------------------------------------------------------
wxString mmex::getProgramCopyright()
{
    return "(c) 2005-2012 Madhan Kanagavel";
}

wxString mmex::getProgramWebSite()
{
    return "http://codelathe.com/mmex";
}

wxString mmex::getProgramForum()
{
    return "http://www.codelathe.com/forum";
}
wxString mmex::getProgramFacebookSite()
{
    return "http://www.facebook.com/pages/Money-Manager-Ex/242286559144586";
}

wxString mmex::getProgramDescription()
{
    wxString description;
    description << _("MMEX is using the following support products") << ":\n"
                << "======================================\n"
                << wxVERSION_STRING << "\n"
                << "SQLite3 " << wxSQLite3Database::GetVersion() << "\n"
                << "wxSQLite 3.0.3"<< "\n"
                << "Lua 5.2.2";
    return description;
}

