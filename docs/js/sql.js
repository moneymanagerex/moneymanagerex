/*
 * SQL Formatting script
 */
// numerate rows automatically
document.querySelectorAll(".sqlCode").forEach(code => {

  // Zeilen auslesen
  let lines = code.textContent.replace(/^\n|\n$/g, "").split("\n");

  // HTML mit Zeilen erstellen
  code.innerHTML = lines
    .map(line => `<span>${line || "&nbsp;"}</span>`)
    .join("");

});
// Add copy function
document.querySelectorAll(".sql-codebox").forEach(box => {
  const code = box.querySelector(".sqlCode");
  const btn = box.querySelector(".copy-btn");

  if (!btn) return;

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
