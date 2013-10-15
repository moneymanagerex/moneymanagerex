/*******************************************************
 Copyright (C) 2013 Nikolay

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

#include "reportbase.h"
#include <map>

#ifndef HTML_WIDGET_STOCKS_H
#define HTML_WIDGET_STOCKS_H

class htmlWidgetStocks : public mmPrintableBase
{
public:
    ~htmlWidgetStocks();
    htmlWidgetStocks();
    double get_total();
    double get_total_gein_lost();
    void enable_detailes(bool enable);

    wxString getHTMLText();

protected:

    wxString title_;
    bool enable_details_;
    double grand_total_;
    double grand_gain_lost_;
    void calculate_stats(std::map<int, std::pair<double, double> > &stockStats);
};

#endif //
