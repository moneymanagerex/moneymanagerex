/*
 * Dynamic page translation script
 * by Nikolay Akimov <https://github.com/vomikan/>
 */

function translate() {
    var lang = getParameterValue("lang");
    if (lang != "") String.locale = lang;

    if (Object.keys(localization).indexOf(lang) > -1) {
        console.log(lang)
        var success = 0;
        var elements = document.getElementsByClassName('i18n');
        for (var i = 0; i < elements.length; i++) {
            var element = elements[i];
            var element_str = element.innerHTML;
            var id = element.getAttribute('id');
            if (id != null) element_str = id;
            var translation_str = localization[lang][0][element_str];
            if (translation_str != "" & translation_str !== undefined) {
                element.innerHTML = translation_str;
                success = success + 1;
            }
            else
                element.style.color = "#aa0000";
        }
        if (i != success) {
            var perc = document.getElementById("percentage");
            perc.innerHTML = "Translated " + success + " of " + i;
        }
    }
}

function getParameterValue(parameter) {
    parameter = parameter.replace(/[\[]/, "\\\[").replace(/[\]]/, "\\\]");
    var regexS = "[\\?&]" + parameter + "=([^&#]*)";
    var regex = new RegExp(regexS);
    var results = regex.exec(window.location.href);
    if (results == null)
        return "";
    else
        return results[1];
}
