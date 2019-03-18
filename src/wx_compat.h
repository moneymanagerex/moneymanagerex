/*
 * Compatibility macros for older wxWidgets versions
 */

#include <wx/intl.h>

/* CONTEXT support since 3.1.1 */
#ifndef wxGETTEXT_IN_CONTEXT
    #define wxGETTEXT_IN_CONTEXT(c, s) _((s))
#endif
#ifndef wxGETTEXT_IN_CONTEXT_PLURAL
    #define wxGETTEXT_IN_CONTEXT_PLURAL(c, sing, plur, n) \
        wxPLURAL((sing), (plur), n)
#endif
