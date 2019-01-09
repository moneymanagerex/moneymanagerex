/*
 * Dynamic Table of Contents script
 * by Matt Whitlock <http://www.whitsoftdev.com/>
 */

function createLink(href, innerHTML) {
    var a = document.createElement("a");
    a.setAttribute("href", href);
    a.innerHTML = innerHTML;
    return a;
}

function generateTOC(toc) {
    var i2 = 0, i3 = 0;
    toc = toc.appendChild(document.createElement("ul"));
    for (var i = 0; i < document.body.childNodes.length; ++i) {
        var node = document.body.childNodes[i];
        var tagName = node.nodeName.toLowerCase();
        if (tagName == "h3") {
            ++i3;
            if (i3 == 1) toc.lastChild.appendChild(document.createElement("ul"));
            var section = i2 + "." + i3;
            node.insertBefore(document.createTextNode(section + ". "), node.firstChild);
            node.id = "section" + section;
            toc.lastChild.lastChild
                .appendChild(document.createElement("li"))
                .appendChild(createLink("#section" + section, node.innerHTML));
        }
        else if (tagName == "h2") {
            ++i2, i3 = 0;
            var section = i2;
            node.insertBefore(document.createTextNode(section + ". "), node.firstChild);
            node.id = "section" + section;
            toc.appendChild(h2item = document.createElement("li"))
                .appendChild(createLink("#section" + section, node.innerHTML));
        }
    }
}
