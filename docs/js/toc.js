/*
 * Dynamic Table of Contents script
 * by Matt Whitlock <http://www.whitsoftdev.com/>
 * modified by Tomasz Słodkowicz
 */

function createLink(href, innerHTML) {
    var a = document.createElement("a");
    a.setAttribute("href", href);
    a.innerHTML = innerHTML;
    return a;
}

function addId(section, node) {
    node.insertBefore(document.createTextNode(section + ". "), node.firstChild);
    node.id = "section" + section;
}

function addTOC(list, node) {
    list.appendChild(document.createElement("li"))
        .appendChild(createLink("#" + node.id, node.innerHTML));
}

function generateTOC(nav) {
    var i2 = 0, i3 = 0;
    nav = nav.appendChild(document.createElement("ul"));
    Array.prototype.slice.apply(document.querySelectorAll("h2, h3"))
        .forEach(function(node) {
            var tagName = node.nodeName.toLowerCase();
            if (tagName === "h3") {
                if (++i3 === 1) nav.lastChild.appendChild(document.createElement("ul"));
                addId(i2 + "." + i3, node);
                addTOC(nav.lastChild.lastChild, node);
            }
            else if (tagName === "h2") {
                i3 = 0;
                addId(++i2, node);
                addTOC(nav, node);
            }
        });
}
