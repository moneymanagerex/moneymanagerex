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

#ifndef _WX_CHARTS_WXCHARTSLABELOPTIONS_H_
#define _WX_CHARTS_WXCHARTSLABELOPTIONS_H_

#include "wxchartfontoptions.h"
#include "wxchartbackgroundoptions.h"

/// The options for the wxChartsLabel element.
class wxChartsLabelOptions : public wxObject
{
public:
    wxChartsLabelOptions(const wxChartFontOptions &fontOptions,
        bool hasBackground, const wxChartBackgroundOptions &backgroundOptions);

    const wxChartFontOptions& GetFontOptions() const;
    bool HasBackground() const;
    const wxChartBackgroundOptions& GetBackgroundOptions() const;

private:
    class RefData : public wxObjectRefData
    {
    public:
        RefData(const wxChartFontOptions &fontOptions, bool hasBackground, 
            const wxChartBackgroundOptions &backgroundOptions);

        const wxChartFontOptions& GetFontOptions() const;
        bool HasBackground() const;
        const wxChartBackgroundOptions& GetBackgroundOptions() const;

    private:
        wxChartFontOptions m_fontOptions;
        bool m_hasBackground;
        wxChartBackgroundOptions m_backgroundOptions;
    };
};

#endif
