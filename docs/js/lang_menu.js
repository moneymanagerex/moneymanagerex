/*
 * Dynamic language menu script
 * by Tomasz Słodkowicz
 */
"use strict";

function generateLangMenu(nav) {
    var list = [
        { id: "en", flag: "🇬🇧", lang: "English"   },
        { id: "de", flag: "🇩🇪", lang: "German"    },
        { id: "fr", flag: "🇫🇷", lang: "French"    },
        { id: "hu", flag: "🇭🇺", lang: "Hungarian" },
        { id: "it", flag: "🇮🇹", lang: "Italian"   },
        { id: "pl", flag: "🇵🇱", lang: "Polish"    },
        { id: "ru", flag: "🇷🇺", lang: "Russian"   },
        { id: "es", flag: "🇪🇸", lang: "Spanish"   }
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
        f.setAttribute("class", "flag-icon");
        f.appendChild(document.createTextNode(d.flag));
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
