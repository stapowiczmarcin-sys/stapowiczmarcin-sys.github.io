"use strict";

const parts = window.PARTS_CATALOG || [];
const state = {
  query: "",
  category: new URLSearchParams(location.search).get("category") || "Wszystkie",
  supplier: new URLSearchParams(location.search).get("supplier") || "Wszystkie sklepy",
  project: new URLSearchParams(location.search).get("project") || "Wszystkie projekty"
};

const $ = (selector) => document.querySelector(selector);
const categories = ["Wszystkie", ...new Set(parts.map((part) => part.category))];
const projects = ["Wszystkie projekty", ...new Set(parts.map((part) => part.project))];
const suppliers = ["Wszystkie sklepy", ...new Set(parts.map((part) => part.supplier))];

function escapeHtml(value) {
  return String(value).replace(/[&<>"']/g, (character) => ({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#039;"})[character]);
}

function copyText(part, mode) {
  if (mode === "short") return part.name + " — " + part.spec;
  return part.youtubePl + "\n" + part.youtubeEn + "\n\nElement / Component: " + part.name + "\nSklep / Store: " + part.supplier + "\n" + part.url;
}

async function copyToClipboard(text) {
  if (navigator.clipboard?.writeText) return navigator.clipboard.writeText(text);
  const field = document.createElement("textarea");
  field.value = text;
  field.setAttribute("readonly", "");
  field.style.position = "fixed";
  field.style.opacity = "0";
  document.body.appendChild(field);
  field.select();
  document.execCommand("copy");
  field.remove();
}

function filteredParts() {
  const query = state.query.trim().toLocaleLowerCase("pl");
  return parts.filter((part) => {
    const text = [part.name, part.short, part.spec, part.project, part.supplier].join(" ").toLocaleLowerCase("pl");
    return (!query || text.includes(query)) &&
      (state.category === "Wszystkie" || part.category === state.category) &&
      (state.supplier === "Wszystkie sklepy" || part.supplier === state.supplier) &&
      (state.project === "Wszystkie projekty" || part.project === state.project);
  });
}

function renderCategories() {
  $("#categoryTabs").innerHTML = categories.map((category) =>
    '<button type="button" class="' + (state.category === category ? "active" : "") + '" data-category="' + escapeHtml(category) + '">' + escapeHtml(category) + "</button>"
  ).join("");
}

function renderCards() {
  const filtered = filteredParts();
  $("#resultCount").textContent = String(filtered.length).padStart(2, "0") + " WYNIKÓW";
  $("#emptyState").hidden = filtered.length !== 0;
  $("#partsGrid").innerHTML = filtered.map((part, index) =>     '<article class="part-card">' +
      '<div class="part-visual ' + escapeHtml(part.accent) + '">' +
        '<img src="' + escapeHtml(window.PART_IMAGES[part.id]) + '" alt="' + escapeHtml(part.name) + '" loading="lazy">' +
        '<span class="card-number">' + String(index + 1).padStart(2, "0") + '</span>' +
        '<span class="status" data-status="' + escapeHtml(part.status) + '">' + escapeHtml(part.status) + '</span>' +
      '</div>' +
      '<div class="part-body">' +
        '<div class="meta-row"><span>' + escapeHtml(part.category) + '</span><span>' + escapeHtml(part.supplier) + '</span></div>' +
        '<h3>' + escapeHtml(part.name) + '</h3><p class="part-short">' + escapeHtml(part.short) + '</p><p class="spec">' + escapeHtml(part.spec) + '</p>' +
        '<div class="project-row"><span>Projekt</span><strong>' + escapeHtml(part.project) + '</strong></div>' +
        '<div class="price-row"><span>Cena zakupu</span><strong>' + escapeHtml(part.price || "—") + '</strong></div>' +
        '<div class="card-actions"><button type="button" class="copy-primary" data-copy="full" data-id="' + escapeHtml(part.id) + '">Opis na YouTube</button>' +
        '<button type="button" data-copy="short" data-id="' + escapeHtml(part.id) + '" aria-label="Kopiuj krótki opis ' + escapeHtml(part.name) + '" title="Kopiuj krótki opis">⧉</button>' +
        '<a href="' + escapeHtml(part.url) + '" target="_blank" rel="noreferrer" aria-label="Otwórz produkt ' + escapeHtml(part.name) + '" title="Otwórz produkt">↗</a></div>' +
      '</div>' +
    '</article>'
  ).join("");
}

function render() {
  renderCategories();
  renderCards();
}

function fillSelect(selector, values, selected) {
  const select = $(selector);
  select.innerHTML = values.map((value) => '<option value="' + escapeHtml(value) + '">' + escapeHtml(value) + '</option>').join("");
  select.value = values.includes(selected) ? selected : values[0];
}

const activeCount = parts.filter((part) => part.status === "Używam").length;
const incomingCount = parts.filter((part) => part.status === "W drodze").length;
$("#stats").innerHTML = '<div><strong>' + parts.length + '</strong><span>elementów</span></div><div><strong>' + activeCount + '</strong><span>w użyciu</span></div><div><strong>' + new Set(parts.map((part) => part.supplier)).size + '</strong><span>sklepy</span></div><div><strong>' + incomingCount + '</strong><span>w drodze</span></div>';

fillSelect("#projectFilter", projects, state.project);
fillSelect("#supplierFilter", suppliers, state.supplier);
state.project = $("#projectFilter").value;
state.supplier = $("#supplierFilter").value;
render();

$("#search").addEventListener("input", (event) => { state.query = event.target.value; $("#clearSearch").hidden = !state.query; renderCards(); });
$("#clearSearch").addEventListener("click", () => { state.query = ""; $("#search").value = ""; $("#clearSearch").hidden = true; renderCards(); });
$("#projectFilter").addEventListener("change", (event) => { state.project = event.target.value; renderCards(); });
$("#supplierFilter").addEventListener("change", (event) => { state.supplier = event.target.value; renderCards(); });
$("#categoryTabs").addEventListener("click", (event) => { const button = event.target.closest("[data-category]"); if (!button) return; state.category = button.dataset.category; render(); });
$("#partsGrid").addEventListener("click", async (event) => {
  const button = event.target.closest("[data-copy]");
  if (!button) return;
  const part = parts.find((item) => item.id === button.dataset.id);
  if (!part) return;
  await copyToClipboard(copyText(part, button.dataset.copy));
  const previous = button.textContent;
  button.textContent = button.dataset.copy === "full" ? "Skopiowano ✓" : "✓";
  setTimeout(() => { button.textContent = previous; }, 1600);
});
