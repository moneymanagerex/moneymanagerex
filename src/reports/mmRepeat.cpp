/*
    Author: Lisheng Guan (guanlisheng@gmail.com) 
*/

#include "mmRepeat.h"

template <int N> wxDateSpan mmRepeat(int num)
{
    ++num;  // Unused at this stage: remove compiler warning
    return wxDateSpan::Day();
}

template<> wxDateSpan mmRepeat<0>(int num)
{
    ++num;  // Unused at this stage: remove compiler warning
    return wxDateSpan::Days(0);
}

template<> wxDateSpan mmRepeat<1>(int num)
{
    ++num;  // Unused at this stage: remove compiler warning
    return wxDateSpan::Week();
}

template<> wxDateSpan mmRepeat<2>(int num)
{
    ++num;  // Unused at this stage: remove compiler warning
    return wxDateSpan::Weeks(2);
}

template<> wxDateSpan mmRepeat<3>(int num)
{
    ++num;  // Unused at this stage: remove compiler warning
    return wxDateSpan::Month();
}

template<> wxDateSpan mmRepeat<4>(int num)
{
    ++num;  // Unused at this stage: remove compiler warning
    return wxDateSpan::Months(2);
}

template<> wxDateSpan mmRepeat<5>(int num)
{
    ++num;  // Unused at this stage: remove compiler warning
    return wxDateSpan::Months(3);
}

template<> wxDateSpan mmRepeat<6>(int num)
{
    ++num;  // Unused at this stage: remove compiler warning
    return wxDateSpan::Months(6);
}

template<> wxDateSpan mmRepeat<7>(int num)
{
    ++num;  // Unused at this stage: remove compiler warning
    return wxDateSpan::Year();
}

template<> wxDateSpan mmRepeat<8>(int num)
{
    ++num;  // Unused at this stage: remove compiler warning
    return wxDateSpan::Months(4);
}

template<> wxDateSpan mmRepeat<9>(int num)
{
    ++num;  // Unused at this stage: remove compiler warning
    return wxDateSpan::Weeks(4);
}

template<> wxDateSpan mmRepeat<10>(int num)
{
    ++num;  // Unused at this stage: remove compiler warning
    return wxDateSpan::Day();
}

template<> wxDateSpan mmRepeat<11>(int num)
{
    ++num;  // Unused at this stage: remove compiler warning
    return wxDateSpan::Days(num);
}

template<> wxDateSpan mmRepeat<12>(int num)
{
    ++num;  // Unused at this stage: remove compiler warning
    return wxDateSpan::Months(num);
}

wxDateSpan mmRepeat(int period, int num)
{
    switch (period)
    {
        case 1: return mmRepeat<1>(num);
        case 2: return mmRepeat<2>(num);
        case 3: return mmRepeat<3>(num);
        case 4: return mmRepeat<4>(num);
        case 5: return mmRepeat<5>(num);
        case 6: return mmRepeat<6>(num);
        case 7: return mmRepeat<7>(num);
        case 8: return mmRepeat<8>(num);
        case 9: return mmRepeat<9>(num);
        case 10: return mmRepeat<10>(num);
        case 11: return mmRepeat<11>(num);
        default: return mmRepeat<0>(num);
    }
}
