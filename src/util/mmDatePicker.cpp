/*******************************************************
 Copyright (C) 2014 Gabriele-V
 Copyright (C) 2015, 2016, 2020, 2022 Nikolay Akimov
 Copyright (C) 2022, 2023 Mark Whalley (mark@ipx.co.uk)
 Copyright (C) 2025 Klaus Wich

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

#include "mmDatePicker.h"
#include "base/_constants.h"
#include "model/PrefModel.h"

// -- constructor

mmDatePicker::mmDatePicker(
    wxWindow* parent_win,
    wxWindowID win_id,
    wxDateTime dateTime,
    wxPoint pos,
    wxSize size,
    long style
) :
    wxPanel(parent_win, win_id, pos, size, style),
    m_dateTime(dateTime),
    w_parent(parent_win)
{
    if (!m_dateTime.IsValid())
        m_dateTime = wxDateTime::Now();

    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    SetSizer(sizer);
    w_date_picker = new wxDatePickerCtrl(this, win_id,
        dateTime,
        wxDefaultPosition, wxDefaultSize,
        style
    );
    w_date_picker->SetRange(wxDateTime(), mmDate::max().dateTime());
    setValue(m_dateTime);
    w_date_picker->Bind(wxEVT_DATE_CHANGED, &mmDatePicker::onDateChanged, this);
    sizer->Add(w_date_picker, g_flagsH);
}

mmDatePicker::~mmDatePicker()
{
    // CHECK: wrong condition?
    if (!w_weekday_text)
        delete w_weekday_text;
    if (!w_spin_btn)
        delete w_spin_btn;
}

// -- override

bool mmDatePicker::Enable(bool state)
{
    bool response = w_date_picker->Enable(state);
    if (w_time_picker)
        w_time_picker->Enable(state);
    if (w_weekday_text)
        w_weekday_text->Enable(state);
    if (w_spin_btn)
        w_spin_btn->Enable(state);
    return response;
}

bool mmDatePicker::Show(bool state)
{
    bool response = w_date_picker->Show(state);
    if (w_time_picker) {
        w_time_picker->Show(state);
    }
    if (w_weekday_text) {
        w_weekday_text->Show(state);
    }
    if (w_spin_btn) {
        w_spin_btn->Show(state);
    }
    return response;
}

// -- methods

void mmDatePicker::setValue(const wxDateTime& dateTime)
{
    wxDateTime dt_max = mmDate::max().dateTime();
    wxDateTime dt = (dateTime <= dt_max) ? dateTime : dt_max;

    w_date_picker->SetValue(dt);

    if (w_time_picker)
        w_time_picker->SetValue(dt);

    // trigger date change event
    wxDateEvent dateEvent(this, dt, wxEVT_DATE_CHANGED);
    onDateChanged(dateEvent);
}

bool mmDatePicker::isItMyDateControl(wxObject *obj)
{
    return obj && static_cast <wxDatePickerCtrl*>(obj) == w_date_picker;
}

// Gets the full layout including spin buttons, time picker, and day of week
wxBoxSizer* mmDatePicker::mmGetLayout(bool showTimeCtrl)
{
    wxBoxSizer* date_sizer = new wxBoxSizer(wxHORIZONTAL);
    date_sizer->Add(this, g_flagsH);
#if defined(__WXMSW__) || defined(__WXGTK__)
    date_sizer->Add(this->getSpinButton(), g_flagsH);
#endif
    // If time picker is requested and turned on in Options, add it to the layout
    if (showTimeCtrl && PrefModel::instance().getUseTransDateTime()) {
        w_time_picker = new wxTimePickerCtrl(
            w_parent, GetId(), m_dateTime,
            wxDefaultPosition, wxDefaultSize
        );
        w_time_picker->Bind(wxEVT_TIME_CHANGED,
            &mmDatePicker::onDateChanged, this
        );
        date_sizer->Add(w_time_picker, g_flagsH);
    }
    date_sizer->Add(this->getTextWeek(), g_flagsH);

    return date_sizer;
}

// Adds only the time picker to the layout if "Use time" is turned on in Options
wxBoxSizer* mmDatePicker::mmGetLayoutWithTime()
{
    wxBoxSizer* date_sizer = new wxBoxSizer(wxHORIZONTAL);
    date_sizer->Add(this, g_flagsH);
    if (PrefModel::instance().getUseTransDateTime()) {
        w_time_picker = new wxTimePickerCtrl(
            w_parent, GetId(), m_dateTime,
            wxDefaultPosition, wxDefaultSize
        );
        w_time_picker->Bind(wxEVT_TIME_CHANGED,
            &mmDatePicker::onDateChanged, this
        );
        date_sizer->Add(w_time_picker, g_flagsH);
    }

    return date_sizer;
}

wxStaticText* mmDatePicker::getTextWeek()
{
    if (w_weekday_text)
        return w_weekday_text;

    //Text field for name of day of the week
    wxSize WeekDayNameMaxSize(wxDefaultSize);
    for (wxDateTime::WeekDay d = wxDateTime::Sun;
        d != wxDateTime::Inv_WeekDay;
        d = wxDateTime::WeekDay(d+1)
    )
        WeekDayNameMaxSize.IncTo(GetTextExtent(wxGetTranslation(
            wxDateTime::GetEnglishWeekDayName(d)
        ) + " "));

    WeekDayNameMaxSize.SetHeight(-1);
    w_weekday_text = new wxStaticText(
        w_parent, wxID_ANY, "",
        wxDefaultPosition, WeekDayNameMaxSize, wxST_NO_AUTORESIZE
    );

    // Force update
    wxDateEvent dateEvent(this, w_date_picker->GetValue(), wxEVT_DATE_CHANGED);
    onDateChanged(dateEvent);

    return w_weekday_text;
}

wxSpinButton* mmDatePicker::getSpinButton()
{
    if (w_spin_btn)
        return w_spin_btn;

    w_spin_btn = new wxSpinButton(w_parent, wxID_ANY,
        wxDefaultPosition, wxSize(-1, GetBestSize().GetHeight()),
        wxSP_VERTICAL | wxSP_ARROW_KEYS | wxSP_WRAP
    );
    w_spin_btn->Bind(wxEVT_SPIN, &mmDatePicker::onDateSpin, this);
    w_spin_btn->SetRange(-32768, 32768);

    return w_spin_btn;
}

// -- event handlers

void mmDatePicker::onDateChanged(wxDateEvent& event)
{
#ifdef __WXMAC__  // https://github.com/moneymanagerex/moneymanagerex/issues/7821
    if (w_date_picker->GetValue().GetCentury() < 1)
        w_date_picker->SetValue(w_date_picker->GetValue().Add(wxDateSpan::Years(2000)));
#endif
    if (w_time_picker)
        m_dateTime.ParseISOCombined(
            w_date_picker->GetValue().FormatISODate() + "T" +
            w_time_picker->GetValue().FormatISOTime()
        );
    else
        m_dateTime = w_date_picker->GetValue();

    if (w_weekday_text) {
        w_weekday_text->SetLabelText(wxGetTranslation(
            m_dateTime.GetEnglishWeekDayName(m_dateTime.GetWeekDay())
        ));
    }

    event.SetDate(m_dateTime);
    event.Skip();
}

void mmDatePicker::onDateSpin(wxSpinEvent&)
{
    if (!w_spin_btn)
        return;

    wxDateTime date = GetValue();
    date = date.Add(wxDateSpan::Days(w_spin_btn->GetValue()));
    setValue(date);
    wxDateEvent evt(this, GetValue(), wxEVT_DATE_CHANGED);
    GetEventHandler()->AddPendingEvent(evt);
    w_spin_btn->SetValue(0);
}
