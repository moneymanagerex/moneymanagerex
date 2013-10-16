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

#ifndef _MM_EX_HOMEPAGEPANEL_H_
#define _MM_EX_HOMEPAGEPANEL_H_

#include "mmpanelbase.h"
#include "constants.h"
#include "reports/mmDateRange.h"
#include "model/Model_Account.h"

class mmGUIFrame;
class mmHTMLBuilder;
class mmDateRange;

class mmHtmlWindow: public wxHtmlWindow
{
    DECLARE_NO_COPY_CLASS(mmHtmlWindow)
    DECLARE_EVENT_TABLE()

public:
    mmHtmlWindow(wxWindow *parent,
                 const wxWindowID id, const wxPoint& pos,
                 const wxSize& size, long style)
        : wxHtmlWindow(parent, id, pos, size, style)
    {}

public:
    void OnLinkClicked(const wxHtmlLinkInfo& link);

};

class mmHomePagePanel : public mmPanelBase
{
    DECLARE_EVENT_TABLE()

public:
    mmHomePagePanel( wxWindow *parent,
                     wxWindowID winid = wxID_ANY,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     long style = wxTAB_TRAVERSAL | wxNO_BORDER,
                     const wxString& name = wxPanelNameStr );

    bool Create( wxWindow *parent, wxWindowID winid,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxTAB_TRAVERSAL | wxNO_BORDER,
                 const wxString& name = wxPanelNameStr);

    ~mmHomePagePanel();

    wxString GetHomePageText();

private:
    mmGUIFrame* frame_;
    mmHtmlWindow* htmlWindow_;
    mmDateRange* date_range_;
    void CreateControls();
    void sortTable();
    void createFrames();
    wxString prepareTemplate(const wxString& left, const wxString& right);
    wxString html_text_;
    wxString vAccts_;

    wxString displaySummaryHeader(const wxString& summaryTitle );
    wxString displaySectionTotal(const wxString& totalsTitle, double tRecBalance, double& tBalance);

    wxString displayAccounts(double& tBalance, std::map<int, std::pair<double, double> > &accountStats, int type = Model_Account::CHECKING);
    void get_account_stats(std::map<int, std::pair<double, double> > &accountStats);
    wxString displayAssets(double& tBalance);
    wxString displayIncomeVsExpenses();
    wxString getStatWidget();
    wxString getCalendarWidget();
    wxString displayGrandTotals(double& tBalance);
};

#endif
