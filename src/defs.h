/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

#ifndef MM_EX_DEFS_H_
#define MM_EX_DEFS_H_

// For compilers that support precompilation, includes "wx/wx.h>.
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/splitter.h>
#include <wx/treectrl.h>
#include <wx/listctrl.h>
#include <wx/fileconf.h>
#include <wx/filename.h>
#include <wx/gdicmn.h>
#include <wx/string.h>
#include <wx/datectrl.h>
#include <wx/datetime.h>
#include <wx/wxsqlite3.h>
#include <wx/progdlg.h>
#include <wx/utils.h>
#include <wx/splash.h>
#include <wx/imagpng.h>
#include <wx/calctrl.h>
#include <wx/dateevt.h>
#include <wx/tokenzr.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/url.h>
#include <wx/html/htmlwin.h>
#include <wx/html/htmlproc.h>
#include <wx/html/htmprint.h>
#include <wx/fs_inet.h>
#include <wx/listbook.h>
#include <wx/imaglist.h>
#include <wx/popupwin.h>
#include <wx/dir.h>
#include <wx/stdpaths.h>
#include <wx/app.h>
#include <wx/choice.h>
#include <wx/statline.h>
#include <wx/dialog.h>
#include <wx/numformatter.h>

#ifdef _MSC_VER
#pragma warning (disable:4100)
#endif

#define _n(string) wxString(wxTRANSLATE(string))
#define _t(string) _(string)
#define _nu(string_utf8) wxString::FromUTF8(wxTRANSLATE(string_utf8))
#define _tu(string_utf8) wxGetTranslation(wxString::FromUTF8(wxTRANSLATE(string_utf8)))
#define wxPLURAL_U8(singular, plural, n) wxPLURAL(wxString::FromUTF8(singular), wxString::FromUTF8(plural), n)

#endif // MM_EX_DEFS_H_

