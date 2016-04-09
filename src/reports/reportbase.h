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
#ifndef MM_EX_REPORTBASE_H_
#define MM_EX_REPORTBASE_H_
//----------------------------------------------------------------------------
#include "mmDateRange.h"
#include "option.h"
#include "model/Model_Report.h"
class wxString;
class wxArrayString;
//----------------------------------------------------------------------------

class mmPrintableBase
{
public:
    mmPrintableBase(const wxString& title): m_title(title), m_date_range(nullptr), m_initial(true), m_date_selection(0) {}
    virtual ~mmPrintableBase() {}
    virtual wxString getHTMLText() = 0;
    virtual void RefreshData() {}
    virtual wxString title() const;
    virtual bool has_date_range() { return false;}
    void date_range(const mmDateRange* date_range, int selection) { this->m_date_range = date_range; this->m_date_selection = selection; }
    int getDateSelection() { return this->m_date_selection; }
    void initial_report(bool initial) { m_initial = initial; }
protected:
    wxString m_title;
    const mmDateRange* m_date_range;
    bool m_initial;
    int m_date_selection;
};

class mmGeneralReport : public mmPrintableBase
{
public:
    explicit mmGeneralReport(const Model_Report::Data* report);

public:
    wxString getHTMLText();

private:
    const Model_Report::Data* m_report;
};

class mmPrintableBaseSpecificAccounts : public mmPrintableBase
{
public:
    explicit mmPrintableBaseSpecificAccounts(const wxString& report_name, int sort_column = 0);
    virtual ~mmPrintableBaseSpecificAccounts();

protected:
    const wxArrayString* accountArray_;

    void getSpecificAccounts();
};

//----------------------------------------------------------------------------
#endif // MM_EX_REPORTBASE_H_
