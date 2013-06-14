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

#ifndef _MM_EX_PLATFDEP_H_
#define _MM_EX_PLATFDEP_H_
//----------------------------------------------------------------------------
/*
        Platform-dependent API.
*/
//----------------------------------------------------------------------------

class wxFileName;
class wxString;

namespace mmex
{
wxFileName GetDocDir();
wxFileName GetResourceDir();
wxFileName GetSharedDir();
wxFileName GetUserDir(bool create);
wxFileName GetLogDir(bool create);

/*
    wxStandardPaths uses wxApp::GetAppName(), so you should
    call wxApp::SetAppName(mmex::GetAppName()) in wxApp::OnInit().

    Use mmex::getProgramName() for others purposes.
*/
wxString GetAppName();
} // namespace mmex

//----------------------------------------------------------------------------
#endif // _MM_EX_PLATFDEP_H_
//----------------------------------------------------------------------------
