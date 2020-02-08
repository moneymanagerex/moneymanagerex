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

#ifndef _WX_CHARTS_WXCHARTSPOINTOPTIONS_H_
#define _WX_CHARTS_WXCHARTSPOINTOPTIONS_H_

#include <wx/colour.h>

/// The options for the wxChartPoint element.

/// \ingroup elementclasses
class wxChartsPointOptions
{
public:
    /// Constructs a wxChartPointOptions instance.
    /// @param strokeWidth The size of the pen used to draw the 
    /// outline of the circle.
    /// @param strokeColor The color of the pen used to draw the
    /// outline of the circle.
    /// @param fillColor The color of the point.
    wxChartsPointOptions(unsigned int strokeWidth, const wxColor &strokeColor,
        const wxColor &fillColor);

    unsigned int GetStrokeWidth() const;
    const wxColor& GetStrokeColor() const;
    const wxColor& GetFillColor() const;

private:
    unsigned int m_strokeWidth;
    wxColor m_strokeColor;
    wxColor m_fillColor;
};

#endif
