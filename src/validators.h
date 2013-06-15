// Validators -----------------------------------------------------
class calcValidator : public wxTextValidator {
public:
calcValidator() : wxTextValidator(wxFILTER_INCLUDE_CHAR_LIST)
{
	wxArrayString list;
	for (const auto& c : "1234567890.,(/+-*)") {
	    list.Add(c);
    }
	SetIncludes(list);
}

};

//    wxFloatingPointValidator<double> validator(4, NULL , wxNUM_VAL_NO_TRAILING_ZEROES );

