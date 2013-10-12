/*
    Copyright (C) 2013 Nikolay Akimov
*/
#ifndef _MM_EX_CALCULATOR_H_
#define _MM_EX_CALCULATOR_H_

#include <wx/string.h>

class mmCalculator
{
public:
    mmCalculator(const wxString& input);
    const virtual bool is_ok();
    const virtual wxString get_result() {return output_;}

protected:
    const wxString input_;
    wxString output_;
    const virtual bool check_syntax() const;
    const virtual bool calculate();
};

#endif 
