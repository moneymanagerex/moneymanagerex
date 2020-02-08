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

/// @file

#ifndef _WX_CHARTS_WXPOLARAREACHARTOPTIONS_H_
#define _WX_CHARTS_WXPOLARAREACHARTOPTIONS_H_

#include "wxchartoptions.h"
#include "wxchartsradialgridoptions.h"

/// The options for the wxPolarAreaChartCtrl control.

/// \ingroup chartclasses
class wxPolarAreaChartOptions : public wxChartOptions
{
public:
    /// Constructs a wxPolarAreaChartOptions instance.
    wxPolarAreaChartOptions();

    /// Gets the options for the grid.
    /// @return The options for the grid.
    const wxChartsRadialGridOptions& GetGridOptions() const;

    wxDouble GetStartAngle() const;
    void SetStartAngle(wxDouble startAngle);

private:
    wxChartsRadialGridOptions m_gridOptions;
    wxDouble m_startAngle;
};

#endif
