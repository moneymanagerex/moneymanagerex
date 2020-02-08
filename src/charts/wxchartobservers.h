/*
    Copyright (c) 2018 Xavier Leclercq and the wxCharts contributors.

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

#ifndef _WX_CHARTS_WXCHARTOBSERVERS_H_
#define _WX_CHARTS_WXCHARTOBSERVERS_H_

#include <wx/vector.h>

template<typename T>
class wxChartValueObserver
{
public:
    virtual void OnUpdate(const T &value) = 0;
};

template<typename T>
class wxChartObservableValue
{
public:
    wxChartObservableValue() {};
    virtual ~wxChartObservableValue() {};

    void AddObserver(wxChartValueObserver<T> *observer)
    {
        m_observers.push_back(observer);
    }

    void RemoveObserver(wxChartValueObserver<T> *observer)
    {
        m_observers.erase(
            std::remove(m_observers.begin(), m_observers.end(), observer),
            m_observers.end());
    }

    void Notify()
    {
        for(auto &observer : m_observers)
            observer->OnUpdate(m_value);
    }

    T GetValue() const
    {
        return m_value;
    }

    void SetValue(const T &newValue)
    {
        m_value = newValue;
        Notify();
    }

protected:
    T m_value;

private:
    wxVector<wxChartValueObserver<T>*> m_observers;
};

#endif
