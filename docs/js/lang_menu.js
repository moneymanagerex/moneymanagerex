/*
 * Dynamic language menu script
 * by Tomasz Słodkowicz
 */
"use strict";

/* Sorted by language code (id) */
function generateLangMenu(nav) {
    var list = [
        { id: "be_BY", flag: "be", lang: "be-BY Беларуская (Беларусь)"  },
        { id: "de_DE", flag: "de", lang: "de-DE Deutsch (Deutschland)"  },
/*        { id: "en_GB", flag: "en", lang: "en-GB English (British)"      },*/
        { id: "en_US", flag: "us", lang: "en-US English (US)"           },
        { id: "es_ES", flag: "es", lang: "es-ES Español (españa)"       },
        { id: "fr_FR", flag: "fr", lang: "fr-FR Français (France)"      },
        { id: "he_IL", flag: "he", lang: "he-IL עברית (ישראל)"          },
        { id: "hr_HR", flag: "hr", lang: "hr-HR hrvatski (Hrvatska)"    },
        { id: "hu_HU", flag: "hu", lang: "hu-HU magyar (Magyarország)"  },
        { id: "it_IT", flag: "it", lang: "it-IT Italiano (Italia)"      },
        { id: "ko_KR", flag: "ko", lang: "ko-KR 한국어(한국)"             },
        { id: "nl_NL", flag: "nl", lang: "nl-NL Nederlands (Nederland)" },
        { id: "pl_PL", flag: "pl", lang: "pl-PL polski (Polska)"        },
        { id: "ru_RU", flag: "ru", lang: "ru-RU Русский (Россия)"       },
        { id: "zh_CN", flag: "cn", lang: "zh-CN 中文（中国）"             }
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
