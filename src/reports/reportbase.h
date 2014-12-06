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
#include "mmOption.h"
#include "model/Model_Report.h"
class wxString;
class wxArrayString;
class mmDateRange;
//----------------------------------------------------------------------------

class mmPrintableBase
{
public:
    mmPrintableBase(): m_title("mmPrintableBase"), m_local_title(_("mmPrintableBase")), m_date_range(nullptr) {}
    mmPrintableBase(const wxString& title, const wxString& local_title): m_title(title), m_local_title(local_title), m_date_range(nullptr) {}
    virtual ~mmPrintableBase() {}
    virtual wxString getHTMLText() = 0;
    virtual void RefreshData() {}
    virtual wxString title() const;
    virtual wxString local_title() const;
    virtual bool has_date_range() { return false;}
public:
    void date_range(const mmDateRange* date_range) { this->m_date_range = date_range; }
protected:
    wxString m_title;
    wxString m_local_title; // after wxGetTranslation or _()
    const mmDateRange* m_date_range;
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
    wxString reportName_;

    void getSpecificAccounts();
};

//----------------------------------------------------------------------------
#endif // MM_EX_REPORTBASE_H_
