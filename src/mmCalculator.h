/*
    Copyright (C) 2013 Nikolay Akimov
*/
#ifndef _MM_EX_CALCULATOR_H_
#define _MM_EX_CALCULATOR_H_

#include <wx/string.h>

class mmCalculator
{
public:
    mmCalculator();
    const virtual bool is_ok(const wxString& input);
    const virtual double get_result() {return output_;}

protected:

private:
    double output_;
    const virtual bool check_syntax(const wxString& input) const;
    const virtual bool calculate(const wxString& input);
};

#endif 
