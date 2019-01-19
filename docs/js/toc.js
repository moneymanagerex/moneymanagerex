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

function generateTOC(toc) {
    var i2 = 0, i3 = 0;
    toc = toc.appendChild(document.createElement("ul"));
	let nodeList = document.querySelectorAll("h2, h3");
	for(var i = 0; i < nodeList.length; i++)
	{
        var tagName = nodeList[i].nodeName.toLowerCase();
        if (tagName === "h3") {
            if (++i3 === 1) toc.lastChild.appendChild(document.createElement("ul"));
            addId(i2 + "." + i3, nodeList[i]);
            addTOC(toc.lastChild.lastChild, nodeList[i]);
        }
        else if (tagName === "h2") {
            i3 = 0;
            addId(++i2, nodeList[i]);
            addTOC(toc, nodeList[i]);
        }
    }
}
