// Validators -----------------------------------------------------
class mmCalcValidator : public wxTextValidator
{
public:
    mmCalcValidator() : wxTextValidator(wxFILTER_INCLUDE_CHAR_LIST)
    {
        wxArrayString list;
        for (const auto& c : "1234567890.,(/+-*)")
        {
            list.Add(c);
        }
        SetIncludes(list);
    }

};

class mmDoubleValidator2: public wxFloatingPointValidator<double>
{
public:
    mmDoubleValidator2(): wxFloatingPointValidator<double>(2, NULL, wxNUM_VAL_NO_TRAILING_ZEROES)
    {
        this->SetRange(0, 100000000);
    }
};

class mmDoubleValidator4: public wxFloatingPointValidator<double>
{
public:
    mmDoubleValidator4(): wxFloatingPointValidator<double>(4, NULL, wxNUM_VAL_NO_TRAILING_ZEROES)
    {
        this->SetRange(0, 100000000);
    }
};

