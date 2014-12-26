#pragma once

#include <wx/panel.h>
#include <wx/msgdlg.h> 

class mmOptionSettingsBase : public wxPanel
{
public:
    virtual void SaveSettings() = 0;
};
