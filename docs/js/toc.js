/*
 * Dynamic Table of Contents script
 * by Matt Whitlock <http://www.whitsoftdev.com/>
 * modified by Tomasz Słodkowicz
 */
"use strict";

function createLink(href, innerHTML) {
    var a = document.createElement("a");
    a.setAttribute("href", href);
    a.innerHTML = innerHTML;
    return a;
}

function addTOC(list, node) {
    list.appendChild(document.createElement("li"))
        .appendChild(createLink("#" + node.id, node.innerHTML));
}

function generateTOC(nav) {
    var i2 = 0, i3 = 0;
    nav = nav.appendChild(document.createElement("ol"));
    [].forEach.call(document.querySelectorAll("h2, h3"), function(node) {
        if (node.nodeName === "H3") {
            if (i3 === 0)
                nav.lastChild.appendChild(document.createElement("ol"));
            node.id = "section" + i2 + "." + (++i3);
            addTOC(nav.lastChild.lastChild, node);
        }
        else if (node.nodeName === "H2") {
            i3 = 0;
            node.id = "section" + (++i2);
            addTOC(nav, node);
        }
    });
}

function collapseNode(n, bool) {
    [].forEach.call(n.childNodes, function(n) {
        if (n.nodeName === "OL")
            n.style.display = bool ? "none" : "";
    });
    n.firstChild.textContent = bool ? "+" : "-";
    n.firstChild.style.cursor = bool ? "zoom-in" : "zoom-out";
    // n.firstChild.title = bool ? "expand" : "collapse";
    n.firstChild.onclick = function(){collapseNode(n, !bool);};
}

function collapseTOC(nav, bool) {
    [].forEach.call(nav.firstChild.childNodes, function(n) {
        var icon = document.createElement("span");
        icon.innerHTML = "&nbsp;";
        icon.className = "icon";
        n.insertBefore(icon, n.firstChild);
        if (n.lastChild.nodeName === "OL")
            collapseNode(n, bool);
    });
}
