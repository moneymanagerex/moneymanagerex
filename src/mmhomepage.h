/*******************************************************
Copyright (C) 2014 - 2020 Nikolay Akimov
Copyright (C) 2022 Mark Whalley (mark@ipx.co.uk)

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

#pragma once

#include "reports/mmDateRange.h"
#include <map>
#include <vector>

class htmlWidgetStocks
{
public:
    ~htmlWidgetStocks();
    htmlWidgetStocks();
    double get_total();
    double get_total_gein_lost();

    const wxString getHTMLText();

protected:

    wxString title_;
    double grand_total_;
    double grand_gain_lost_;
    void calculate_stats(std::map<int, std::pair<double, double> > &stockStats);
};

class htmlWidgetTop7Categories
{
public:
    explicit htmlWidgetTop7Categories();
    ~htmlWidgetTop7Categories();
    const wxString getHTMLText();

protected:
    mmDateRange* date_range_;
    wxString title_;
    void getTopCategoryStats(
        std::vector<std::pair<wxString, double> > &categoryStats
        , const mmDateRange* date_range) const;
};


class htmlWidgetBillsAndDeposits
{
public:

    explicit htmlWidgetBillsAndDeposits(const wxString& title
        , mmDateRange* date_range = new mmAllTime());
    ~htmlWidgetBillsAndDeposits();
    const wxString getHTMLText();

protected:
    mmDateRange* date_range_;
    wxString title_;
};

class htmlWidgetIncomeVsExpenses
{
public:
    ~htmlWidgetIncomeVsExpenses();
    const wxString getHTMLText();
};

class htmlWidgetStatistics
{
public:
    ~htmlWidgetStatistics();
    const wxString getHTMLText();
};

class htmlWidgetGrandTotals
{
public:
    ~htmlWidgetGrandTotals();
    const wxString getHTMLText(double tBalance, double tReconciled, double tAssets, double tStocks);
};

class htmlWidgetAssets
{
public:
    ~htmlWidgetAssets();
    const wxString getHTMLText();
};

class htmlWidgetAccounts
{
public:
    htmlWidgetAccounts();
    const wxString displayAccounts(double& tBalance, double& tReconciled, int type);
    ~htmlWidgetAccounts();
private:
    std::map<int, std::pair<double, double> > accountStats_;
    void get_account_stats();
};


class htmlWidgetCurrency
{
public:
    ~htmlWidgetCurrency();
    const wxString getHtmlText();
};
