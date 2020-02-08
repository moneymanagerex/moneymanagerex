/*
    Copyright (c) 2017-2019 Xavier Leclercq

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

#include "wxchartsthemefactory.h"

std::map<wxChartsThemeId, wxSharedPtr<wxChartsTheme>>* wxChartsThemeFactory::m_themes = 0;

wxChartsTheme& wxChartsThemeFactory::Get(const wxChartsThemeId& id)
{
    std::map<wxChartsThemeId, wxSharedPtr<wxChartsTheme>>& themes = GetMap();
    std::map<wxChartsThemeId, wxSharedPtr<wxChartsTheme>>::const_iterator it = themes.find(id);
    if (it != themes.end())
    {
        return *it->second;
    }
    else
    {
        return *themes[wxChartsThemeId()];
    }
}

void wxChartsThemeFactory::Register(const wxChartsThemeId& id, wxSharedPtr<wxChartsTheme> theme)
{
    GetMap()[id] = theme;
}

std::map<wxChartsThemeId, wxSharedPtr<wxChartsTheme>>& wxChartsThemeFactory::GetMap()
{
    if (m_themes == 0)
    {
        m_themes = new std::map<wxChartsThemeId, wxSharedPtr<wxChartsTheme>>();
        (*m_themes)[wxChartsThemeId()] = wxChartsDefaultTheme;
    }
    return *m_themes;
}
