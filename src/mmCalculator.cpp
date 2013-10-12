/*
    Copyright (C) 2013 Nikolay Akimov
*/
#include "mmCalculator.h"
#include <wx/tokenzr.h>

mmCalculator::mmCalculator()
{
    output_ = 0;
}

const bool mmCalculator::is_ok(const wxString& input)
{
    bool ok = check_syntax(input);
    if (ok) ok = calculate(input);
    return ok;
}

const bool mmCalculator::calculate(const wxString& input)
{
    bool ok = true;
    wxString temp = input;
    temp = temp.Trim().Prepend("(").Append(")");
    wxString string_between_brackets, string_to_calc;
    double amount = 0;

    while (temp.Contains("(") && ok)
    {
        amount = 0;
        size_t left_pos = temp.Find('(', true);
        size_t right_pos = temp.find(")", left_pos);
        string_between_brackets = temp.SubString(left_pos, right_pos);
        string_between_brackets.Replace("(-", "(N");
        string_to_calc = string_between_brackets.SubString(1, string_between_brackets.Len()-2);
        if (string_to_calc.IsEmpty()) ok = false;
        double temp_amount;
        string_to_calc.Replace("*-", "M");
        string_to_calc.Replace("/-", "D");
        string_to_calc.Replace("+-", "-");
        string_to_calc.Replace("-+", "-");
        string_to_calc.Replace("+", "|");
        string_to_calc.Replace("-", "|-");
        string_between_brackets.Replace("(N", "(-");

        wxStringTokenizer token(string_to_calc, "|");

        while (token.HasMoreTokens() && ok)
        {
            double dSubtotal = 1;
            wxString sToken = token.GetNextToken();
            sToken.Replace("M", "|M");
            sToken.Replace("D", "|D");
            sToken.Replace("*", "|*");
            sToken.Replace("/", "|/");
            sToken.Replace("N", "-");
            sToken.Prepend("*");

            wxStringTokenizer token2(sToken, "|");
            while (token2.HasMoreTokens() && ok)
            {
                wxString sElement = token2.GetNextToken();
                wxString sSign = sElement.Mid(0,1);
                sElement.Remove(0,1);

                if (sElement.ToDouble(&temp_amount))
                {
                    if (sSign == "*") dSubtotal = dSubtotal*temp_amount;
                    else if (sSign == "M") dSubtotal = -dSubtotal*temp_amount;
                    else if (sSign == "/" && temp_amount != 0) dSubtotal = dSubtotal/temp_amount;
                    else if (sSign == "D" && temp_amount != 0) dSubtotal = -dSubtotal/temp_amount;
                    else ok = false;
                }
                else
                    ok = false;
            }
            amount += dSubtotal;
        }
        temp.Replace(string_between_brackets, wxString()<<amount);
    }
    if (ok)
    {
        if (temp.Contains("(")||temp.Contains(")"))
            ok = false;
        else
            output_ = amount;
    }
    return ok;
}

const bool mmCalculator::check_syntax(const wxString& input) const
{
    bool ok = false;
    wxString temp = input;
    int a = temp.Replace("(", "(");
    int b = temp.Replace(")", ")");
    ok = (a == b);
    if (ok && a > 0)
    {
        for (size_t i = 0; i < input.Len(); i++)
        {
            if (input[i] == '(') a += i;
            else if (input[i] == ')') b += i;
            if (input[i] == '(' && i > 0) ok = (ok && (wxString("(+-*/").Contains(input[i-1])));
            if (input[i] == ')' && i < input.Len()-1) ok = (ok && wxString(")+-*/").Contains(input[i+1]));
        }
        ok = (a < b);
    }
    return ok;
}
