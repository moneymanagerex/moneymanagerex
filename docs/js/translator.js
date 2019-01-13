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

function translate() {
    var lang = getParameterValue("lang");
    if (lang != null) String.locale = lang;

    if (Object.keys(localization).indexOf(lang) > -1) {
        // console.log(lang);
        var elements = document.querySelectorAll(".i18n");
        var success = 0, all = elements.length;
        elements.forEach(function(element) {
            var elementStr = element.innerHTML;
            var id = element.id;
            if (id) elementStr = id;
            var translationStr = localization[lang][0][elementStr];
            if (translationStr) {
                element.innerHTML = translationStr;
                ++success;
            }
            else
                element.style.color = "#aa0000";
        });
        if (success !== all)
            document.getElementById("percentage")
                .innerHTML = "Translated " + success + " of " + all;
    }
}
