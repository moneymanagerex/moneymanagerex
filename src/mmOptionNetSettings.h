#pragma once

#include "mmOptionBaseSettings.h"

class mmOptionNetSettings : public mmOptionSettingsBase
{
    wxDECLARE_EVENT_TABLE();

public:
    mmOptionNetSettings();

    mmOptionNetSettings(wxWindow *parent
        , wxWindowID id = wxID_ANY
        , const wxPoint &pos = wxDefaultPosition
        , const wxSize &size = wxDefaultSize
        , long style = wxTAB_TRAVERSAL
        , const wxString &name = wxPanelNameStr);

    ~mmOptionNetSettings();

public:
    virtual void SaveSettings();

private:
    void Create(wxWindow *parent
        , wxWindowID id
        , const wxPoint& pos
        , const wxSize& size
        , long style
        , const wxString &name);
};
