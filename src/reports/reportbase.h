/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2017 James Higley

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
    mmPrintableBase(const wxString& title);
    virtual ~mmPrintableBase();
    virtual wxString getHTMLText() = 0;
    virtual void RefreshData() {}
    virtual wxString title() const;
    virtual wxString file_name() const;
    virtual bool has_date_range() { return false; }
    virtual bool has_budget_dates() { return false; }
    virtual bool has_only_years() { return false; }
    virtual bool has_accounts() { return false; }
    virtual void date_range(const mmDateRange* date_range, int selection);
    void accounts(int selection, wxString& name);
    int getDateSelection() { return this->m_date_selection; }
    int getAccountSelection() { return this->m_account_selection; }
    void initial_report(bool initial) { m_initial = initial; }
    void setSettings(const wxString& settings);
    void getDates(wxDateTime &begin, wxDateTime &end);
protected:
    wxString m_title;
    const mmDateRange* m_date_range;
    bool m_initial;
    int m_date_selection;
    int m_account_selection;
    const wxArrayString* accountArray_;
    bool m_only_active;
    wxString m_settings;
    wxDateTime m_begin_date;
    wxDateTime m_end_date;

public:
    static const char * m_template;
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

#include "html_template.h"
class mm_html_template: public html_template
{
public:
    explicit mm_html_template(const wxString & arg_template);

private:
    void load_context();
};



//----------------------------------------------------------------------------
#endif // MM_EX_REPORTBASE_H_
