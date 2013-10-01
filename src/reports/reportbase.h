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

//----------------------------------------------------------------------------
#ifndef _MM_EX_REPORTBASE_H_
#define _MM_EX_REPORTBASE_H_
//----------------------------------------------------------------------------
#include "mmcoredb.h"
#include "mmOption.h"
class wxString;
//----------------------------------------------------------------------------

class mmPrintableBase
{
public:
    virtual ~mmPrintableBase() {}
    virtual wxString getHTMLText() = 0;
	void setSortColumn(const int sort_by) { sortColumn_ = sort_by; }

public:
    mmPrintableBase(mmCoreDB* core, int sort_column = 0): core_(core), sortColumn_(sort_column) {}
    virtual wxString version() { return "$Rev$"; }

protected:
    const mmCoreDB* core_;
	int sortColumn_;
};
//----------------------------------------------------------------------------
#endif // _MM_EX_REPORTBASE_H_
