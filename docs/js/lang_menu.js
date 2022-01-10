/*
 * Dynamic language menu script
 * by Tomasz Słodkowicz
 */
"use strict";

function generateLangMenu(nav) {
    var list = [
        { id: "en_GB", flag: "en", lang: "English"   },
        { id: "be_BY", flag: "be", lang: "Belarusian"},
        { id: "zh_CN", flag: "cn", lang: "Chinese"},
        { id: "de_DE", flag: "de", lang: "German"    },
        { id: "fr_FR", flag: "fr", lang: "French"    },
        { id: "hu_HU", flag: "hu", lang: "Hungarian" },
        { id: "he_IL", flag: "he", lang: "Hebrew"    },
        { id: "hr_HR", flag: "hr", lang: "Croatian"  },
        { id: "it_IT", flag: "it", lang: "Italian"   },
        { id: "nl_NL", flag: "nl", lang: "Dutch"     },
        { id: "pl_PL", flag: "pl", lang: "Polish"    },
        { id: "ru_RU", flag: "ru", lang: "Russian"   },
        { id: "es_ES", flag: "es", lang: "Spanish"   }
    ];
    var docLang = document.getElementsByTagName("html")[0].lang;
    nav = nav.appendChild(document.createElement("small"));
    list.forEach(function(d) {
        var a;
        if (d.id !== docLang) {
            a = document.createElement("a");
            a.setAttribute("href", "../index.html?lang=" + d.id);
        } else
            a = nav;
        var f = document.createElement("span");
        f.setAttribute("class", "flag-icon flag-icon-" + d.flag);
        a.appendChild(f);
        a.appendChild(document.createTextNode(d.lang));
        if (d.id !== docLang)
            nav.appendChild(a);
        nav.appendChild(document.createTextNode(" "));
    });

    var tr = document.getElementById("translator");
    if (docLang === "en" && tr)
        tr.parentNode.removeChild(tr);
}
