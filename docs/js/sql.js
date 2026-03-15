/*
 * SQL Formatting script
 */

  document.querySelectorAll(".sqlCode").forEach(code => {
    const lines = code.textContent.replace(/^\n|\n$/g, "").split("\n");
    code.textContent = "";
    lines.forEach(line => {
      const span = document.createElement("span");
      if (line) {
        span.textContent = line;
      }
      else {
        span.innerHTML = "&nbsp;";
      }
      code.appendChild(span);
    });
  });

// Add copy function
document.querySelectorAll(".sql-codebox").forEach(box => {
  const code = box.querySelector(".sqlCode");
  const btn = box.querySelector(".copy-btn");

  if (!btn) {
    return;
  }

  btn.addEventListener("click", () => {
    const lines = [...code.querySelectorAll("span")]
      .map(line => line.textContent);
    const text = lines.join("\n");

    navigator.clipboard.writeText(text).then(() => {
      btn.textContent = "Copied!";
      setTimeout(() => {
        btn.textContent = "Copy";
      }, 1500);
    });
  });
});
