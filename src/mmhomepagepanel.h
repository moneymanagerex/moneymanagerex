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

#ifndef MM_EX_HOMEPAGEPANEL_H_
#define MM_EX_HOMEPAGEPANEL_H_

#include "mmpanelbase.h"
#include "constants.h"
#include "Model_Account.h"

class mmGUIFrame;
class mmHTMLBuilder;
class mmDateRange;
class wxWebView;
class wxWebViewEvent;

class mmHomePagePanel : public mmPanelBase
{
    wxDECLARE_EVENT_TABLE();

public:
    mmHomePagePanel(wxWindow *parent, mmGUIFrame *frame,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = "mmHomePagePanel");

    bool Create(wxWindow *parent, wxWindowID winid,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = "mmHomePagePanel");

    ~mmHomePagePanel();

    wxString BuildPage() const { return GetHomePageText(); }
    void PrintPage();
    void createHTML();

public:
    mmGUIFrame* m_frame;
private:
    static const std::vector < std::pair <wxString, wxString> > acc_type_str;
    wxString GetHomePageText() const;
    wxWebView* browser_;
    mmDateRange* date_range_;
    void CreateControls();
    void sortTable() {}
    wxString vAccts_;

    wxString m_templateText;
    std::map <wxString, wxString> m_frames;
    void getTemplate();
    void getData();
    void fillData();
    void OnLinkClicked(wxWebViewEvent& event);
    const wxString getAssetsJSON(double& tBalance) const;
    const wxString getIncomeVsExpensesJSON() const;
    const wxString getStatWidget() const;
    const wxString getToggles() const;
    const wxString getGrandTotalsJSON(double& tBalance) const;

    const wxString getAccountsHTML(double& tBalance, std::map<int, std::pair<double, double> > &accountStats
        , enum Model_Account::TYPE type = Model_Account::CHECKING) const;
    void setAccountsData(std::map<int, std::pair<double, double> > &accountStats);
    void setExpensesIncomeStatsData(std::map<int, std::pair<double, double> > &incomeExpensesStats
        , mmDateRange* date_range) const;
    int countFollowUp_, total_transactions_;
};

#endif
