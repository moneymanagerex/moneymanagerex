/*
 * Dynamic page translation script
 * by Nikolay Akimov <https://github.com/vomikan/>
 */

/* https://eslint.org/docs/user-guide/configuring#specifying-globals */
/* global localization:false */

function getParameterValue(parameter) {
    parameter = parameter.replace(/[\[]/, "\\\[").replace(/[\]]/, "\\\]");
    var regexS = "[\\?&]" + parameter + "=([^&#]*)";
    var regex = new RegExp(regexS);
    var results = regex.exec(window.location.href);
    if (results == null)
        return null;
    else
        return results[1];
}
