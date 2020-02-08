/*
    Copyright (c) 2016-2018 Xavier Leclercq

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

#ifndef _WX_CHARTS_WXCHARTSLICEDATA_H_
#define _WX_CHARTS_WXCHARTSLICEDATA_H_

/// @file

#include <wx/string.h>
#include <wx/colour.h>

/// The data associated with a slice in some charts like the pie and doughnut charts.

/// \ingroup dataclasses
class wxChartSliceData
{
public:
    /// Constructs a wxChartSliceData instance.
    /// @param value The value of the slice.
    /// @param color The slice will be filled with this color.
    /// @param label The label of the slice. By default this will
    /// be used in the legend and the tooltips.
    wxChartSliceData(wxDouble value, const wxColor &color,
        const wxString &label);

    wxDouble GetValue() const;
    void SetValue(wxDouble value);
    const wxColor& GetColor() const;
    const wxString& GetLabel() const;

    wxString GetTooltipText() const;

private:
    wxDouble m_value;
    wxColor m_color;
    wxString m_label;
};

#endif
