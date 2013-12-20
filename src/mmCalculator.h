/*******************************************************
Copyright (C) 2013 Nikolay Akimov

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
};

#endif 
