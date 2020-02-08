/*
    Copyright (c) 2016-2019 Xavier Leclercq

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

/*
    Part of this file were copied from the Chart.js project (http://chartjs.org/)
    and translated into C++.

    The files of the Chart.js project have the following copyright and license.

    Copyright (c) 2013-2016 Nick Downie
    Released under the MIT license
    https://github.com/nnnick/Chart.js/blob/master/LICENSE.md
*/

/// @file

#ifndef _WX_CHARTS_WXCHARTSGRIDMAPPING_H_
#define _WX_CHARTS_WXCHARTSGRIDMAPPING_H_

#include "wxchartsaxis.h"
#include <wx/geometry.h>

/// This class maintains a mapping between points on the chart and pixels on the screen.
class wxChartsGridMapping
{
public:
    wxChartsGridMapping(const wxSize &size,
        const wxChartsAxis::ptr xAxis, const wxChartsAxis::ptr yAxis);

    const wxSize& GetSize() const;
    void SetSize(const wxSize &size);

    wxPoint2DDouble GetWindowPosition(wxDouble x, wxDouble y) const;
    wxPoint2DDouble GetWindowPositionAtTickMark(size_t index, wxDouble value) const;
    
    const wxChartsAxis& GetXAxis() const;
    
private:
    wxSize m_size;
    wxChartsAxis::ptr m_XAxis;
    wxChartsAxis::ptr m_YAxis;
};

#endif
