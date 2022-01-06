/*
 * Dynamic language menu script
 * by Tomasz Słodkowicz
 */
"use strict";

function generateLangMenu(nav) {
    var list = [
        { id: "en", flag: "en", lang: "English"   },
        { id: "be", flag: "be", lang: "Belarusian"},
        { id: "cn", flag: "cn", lang: "Chinese"},
        { id: "de", flag: "de", lang: "German"    },
        { id: "fr", flag: "fr", lang: "French"    },
        { id: "hu", flag: "hu", lang: "Hungarian" },
        { id: "he", flag: "he", lang: "Hebrew"    },
        { id: "hr", flag: "hr", lang: "Croatian"  },
        { id: "it", flag: "it", lang: "Italian"   },
        { id: "nl", flag: "nl", lang: "Dutch"     },
        { id: "pl", flag: "pl", lang: "Polish"    },
        { id: "ru", flag: "ru", lang: "Russian"   },
        { id: "es", flag: "es", lang: "Spanish"   }
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
