"use strict";

const parts = window.PARTS_CATALOG || [];
const i18n = window.CATALOG_I18N || { categories: {}, projects: {}, statuses: {}, parts: {} };
const state = {
  language: "en",
  query: "",
  category: new URLSearchParams(location.search).get("category") || "Wszystkie",
  supplier: new URLSearchParams(location.search).get("supplier") || "Wszystkie sklepy",
  project: new URLSearchParams(location.search).get("project") || "Wszystkie projekty",
  status: new URLSearchParams(location.search).get("status") || "Wszystkie statusy"
};

const $ = (selector) => document.querySelector(selector);
const categories = ["Wszystkie", ...new Set(parts.map((part) => part.category))];
const projects = ["Wszystkie projekty", ...new Set(parts.map((part) => part.project))];
const suppliers = ["Wszystkie sklepy", ...new Set(parts.map((part) => part.supplier))];
const statuses = ["Wszystkie statusy", ...new Set(parts.map((part) => part.status))];
const ui = {
  pl: {
    allStores: "Wszystkie sklepy", results: "wyników", elements: "elementów", inUse: "w użyciu", stores: "sklepy", incoming: "w drodze",
    project: "Projekt", price: "Cena zakupu", youtube: "Opis na YouTube", copied: "Skopiowano ✓", copyShort: "Kopiuj krótki opis",
    open: "Otwórz produkt", noPhoto: "Zdjęcie jeszcze nie dodane", knownVisible: "Widoczne — znane ceny", knownPrices: "znanych cen", search: "Szukaj: lidar, ESP32, audio…",
    searchLabel: "Szukaj części", clearLabel: "Wyczyść wyszukiwanie", projectLabel: "Filtruj według projektu", supplierLabel: "Filtruj według sklepu",
    statusLabel: "Filtruj według statusu", categoriesLabel: "Kategorie", languageLabel: "Zmień język na angielski",
    pageTitle: "Marcin — katalog części projektowych", pageDescription: "Katalog części używanych przez Marcina w projektach CNC, Kora, robotyce, elektronice i audio."
  },
  en: {
    allStores: "All stores", results: "results", elements: "components", inUse: "in use", stores: "stores", incoming: "in transit",
    project: "Project", price: "Purchase price", youtube: "YouTube copy", copied: "Copied ✓", copyShort: "Copy short description",
    open: "Open product", noPhoto: "Photo not added yet", knownVisible: "Visible — known prices", knownPrices: "known prices", search: "Search: lidar, ESP32, audio…",
    searchLabel: "Search parts", clearLabel: "Clear search", projectLabel: "Filter by project", supplierLabel: "Filter by store",
    statusLabel: "Filter by status", categoriesLabel: "Categories", languageLabel: "Switch language to Polish",
    pageTitle: "Marcin — project parts catalogue", pageDescription: "Tested project parts used in Kora, CNC, robotics and electronics — with real build notes and hardware gotchas."
  }
};

function escapeHtml(value) {
  return String(value).replace(/[&<>"']/g, (character) => ({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#039;"})[character]);
}

function translate(mapping, value) {
  return state.language === "en" ? (mapping[value] || value) : value;
}

function viewPart(part) {
  const translated = i18n.parts[part.id] || {};
  return {
    ...part,
    name: state.language === "en" ? (translated.name || part.name) : part.name,
    short: state.language === "en" ? (translated.short || part.youtubeEn || part.short) : part.short,
    spec: state.language === "en" ? (translated.spec || part.spec) : part.spec,
    categoryLabel: translate(i18n.categories, part.category),
    projectLabel: translate(i18n.projects, part.project),
    statusLabel: translate(i18n.statuses, part.status)
  };
}

function copyText(part, mode) {
  const view = viewPart(part);
  if (mode === "short") return view.name + " — " + view.spec;
  const affiliateNotice = part.supplier === "Amazon" ? "\n\nLink afiliacyjny / Affiliate link\nAs an Amazon Associate I earn from qualifying purchases." : "";
  return part.youtubePl + "\n" + part.youtubeEn + "\n\nElement / Component: " + part.name + "\nSklep / Store: " + part.supplier + "\n" + part.url + affiliateNotice;
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
  const query = state.query.trim().toLocaleLowerCase(state.language === "pl" ? "pl" : "en");
  return parts.filter((part) => {
    const english = i18n.parts[part.id] || {};
    const text = [part.name, part.short, part.spec, part.project, part.supplier, english.name, english.short, english.spec].join(" ").toLocaleLowerCase();
    return (!query || text.includes(query)) &&
      (state.category === "Wszystkie" || part.category === state.category) &&
      (state.supplier === "Wszystkie sklepy" || part.supplier === state.supplier) &&
      (state.project === "Wszystkie projekty" || part.project === state.project) &&
      (state.status === "Wszystkie statusy" || part.status === state.status);
  });
}

function renderCategories() {
  $("#categoryTabs").innerHTML = categories.map((category) =>
    '<button type="button" class="' + (state.category === category ? "active" : "") + '" data-category="' + escapeHtml(category) + '">' + escapeHtml(translate(i18n.categories, category)) + "</button>"
  ).join("");
}

function parseKnownGbp(price) {
  const match = String(price || "").match(/£\s*([0-9]+(?:\.[0-9]{1,2})?)/);
  return match ? Number(match[1]) : null;
}

function budgetFor(list) {
  const values = list.map((part) => parseKnownGbp(part.price)).filter((value) => value !== null);
  return { count: list.length, known: values.length, total: values.reduce((sum, value) => sum + value, 0) };
}

function formatGbp(total, known) {
  if (!known) return "—";
  return new Intl.NumberFormat(state.language === "pl" ? "pl-PL" : "en-GB", { style: "currency", currency: "GBP" }).format(total);
}

function renderBudget(filtered) {
  const groups = [
    { label: ui[state.language].knownVisible, data: budgetFor(filtered) },
    { label: translate(i18n.statuses, "Używam"), data: budgetFor(filtered.filter((part) => part.status === "Używam")) },
    { label: translate(i18n.statuses, "Mam"), data: budgetFor(filtered.filter((part) => part.status === "Mam")) },
    { label: translate(i18n.statuses, "W drodze"), data: budgetFor(filtered.filter((part) => part.status === "W drodze")) }
  ];
  $("#budgetSummary").innerHTML = groups.map(({ label, data }) =>
    "<div><span>" + escapeHtml(label) + "</span><strong>" + escapeHtml(formatGbp(data.total, data.known)) + "</strong><small>" + data.known + " / " + data.count + " " + escapeHtml(ui[state.language].knownPrices) + "</small></div>"
  ).join("");
}

function renderCards() {
  const filtered = filteredParts();
  $("#resultCount").textContent = String(filtered.length).padStart(2, "0") + " " + ui[state.language].results.toLocaleUpperCase(state.language === "pl" ? "pl" : "en");
  $("#emptyState").hidden = filtered.length !== 0;
  renderBudget(filtered);
  $("#partsGrid").innerHTML = filtered.map((part, index) => {
    const view = viewPart(part);
    const imageSrc = window.PART_IMAGES && window.PART_IMAGES[part.id];
    const imageMarkup = imageSrc
      ? '<img src="' + escapeHtml(imageSrc) + '" alt="' + escapeHtml(view.name) + '" loading="lazy" decoding="async" referrerpolicy="no-referrer">'
      : '<div class="part-image-placeholder" role="img" aria-label="' + escapeHtml(ui[state.language].noPhoto + ": " + view.name) + '"><span>◇</span><small>' + escapeHtml(ui[state.language].noPhoto) + '</small></div>';
    return '<article class="part-card">' +
      '<div class="part-visual ' + escapeHtml(part.accent) + '">' +
        imageMarkup +
        '<span class="card-number">' + String(index + 1).padStart(2, "0") + '</span>' +
        '<span class="status" data-status="' + escapeHtml(part.status) + '">' + escapeHtml(view.statusLabel) + '</span>' +
      '</div>' +
      '<div class="part-body">' +
        '<div class="meta-row"><span>' + escapeHtml(view.categoryLabel) + '</span><span>' + escapeHtml(part.supplier === "Amazon" ? (state.language === "en" ? "Amazon · affiliate link" : "Amazon · link afiliacyjny") : part.supplier) + '</span></div>' +
        '<h3>' + escapeHtml(view.name) + '</h3><p class="part-short">' + escapeHtml(view.short) + '</p><p class="spec">' + escapeHtml(view.spec) + '</p>' +
        '<div class="project-row"><span>' + escapeHtml(ui[state.language].project) + '</span><strong>' + escapeHtml(view.projectLabel) + '</strong></div>' +
        '<div class="price-row"><span>' + escapeHtml(ui[state.language].price) + '</span><strong>' + escapeHtml(part.price || "—") + '</strong></div>' +
        '<div class="card-actions"><button type="button" class="copy-primary" data-copy="full" data-id="' + escapeHtml(part.id) + '">' + escapeHtml(ui[state.language].youtube) + '</button>' +
        '<button type="button" data-copy="short" data-id="' + escapeHtml(part.id) + '" aria-label="' + escapeHtml(ui[state.language].copyShort + " " + view.name) + '" title="' + escapeHtml(ui[state.language].copyShort) + '">⧉</button>' +
        '<a href="' + escapeHtml(part.url) + '" target="_blank" rel="noreferrer" aria-label="' + escapeHtml(ui[state.language].open + " " + view.name) + '" title="' + escapeHtml(ui[state.language].open) + '">↗</a></div>' +
      '</div>' +
    '</article>';
  }).join("");

  document.querySelectorAll("#partsGrid img").forEach((image) => {
    image.addEventListener("error", () => {
      const placeholder = document.createElement("div");
      placeholder.className = "part-image-placeholder";
      placeholder.setAttribute("role", "img");
      placeholder.setAttribute("aria-label", ui[state.language].noPhoto + ": " + image.alt);
      placeholder.innerHTML = "<span>◇</span><small>" + escapeHtml(ui[state.language].noPhoto) + "</small>";
      image.replaceWith(placeholder);
    }, { once: true });
  });
}

function fillSelect(selector, values, selected, label) {
  const select = $(selector);
  select.innerHTML = values.map((value) => '<option value="' + escapeHtml(value) + '">' + escapeHtml(label(value)) + '</option>').join("");
  select.value = values.includes(selected) ? selected : values[0];
}

function renderStats() {
  const activeCount = parts.filter((part) => part.status === "Używam").length;
  const incomingCount = parts.filter((part) => part.status === "W drodze").length;
  $("#stats").innerHTML = '<div><strong>' + parts.length + '</strong><span>' + ui[state.language].elements + '</span></div><div><strong>' + activeCount + '</strong><span>' + ui[state.language].inUse + '</span></div><div><strong>' + new Set(parts.map((part) => part.supplier)).size + '</strong><span>' + ui[state.language].stores + '</span></div><div><strong>' + incomingCount + '</strong><span>' + ui[state.language].incoming + '</span></div>';
}

function translateStatic() {
  document.documentElement.lang = state.language;
  document.querySelectorAll("[data-pl][data-en]").forEach((element) => { element.textContent = element.dataset[state.language]; });
  const languageButton = $("#catalogLanguage");
  languageButton.textContent = state.language === "pl" ? "PL / EN" : "EN / PL";
  languageButton.setAttribute("aria-label", ui[state.language].languageLabel);
  $("#search").placeholder = ui[state.language].search;
  $("#search").setAttribute("aria-label", ui[state.language].searchLabel);
  $("#clearSearch").setAttribute("aria-label", ui[state.language].clearLabel);
  $("#projectFilter").setAttribute("aria-label", ui[state.language].projectLabel);
  $("#supplierFilter").setAttribute("aria-label", ui[state.language].supplierLabel);
  $("#statusFilter").setAttribute("aria-label", ui[state.language].statusLabel);
  $("#categoryTabs").setAttribute("aria-label", ui[state.language].categoriesLabel);
  document.title = ui[state.language].pageTitle;
  document.querySelector('meta[name="description"]').content = ui[state.language].pageDescription;
}

function render() {
  translateStatic();
  fillSelect("#projectFilter", projects, state.project, (value) => translate(i18n.projects, value));
  fillSelect("#supplierFilter", suppliers, state.supplier, (value) => value === "Wszystkie sklepy" && state.language === "en" ? ui.en.allStores : value);
  fillSelect("#statusFilter", statuses, state.status, (value) => translate(i18n.statuses, value));
  state.project = $("#projectFilter").value;
  state.supplier = $("#supplierFilter").value;
  state.status = $("#statusFilter").value;
  renderStats();
  renderCategories();
  renderCards();
}

render();

$("#catalogLanguage").addEventListener("click", () => { state.language = state.language === "pl" ? "en" : "pl"; render(); });
$("#search").addEventListener("input", (event) => { state.query = event.target.value; $("#clearSearch").hidden = !state.query; renderCards(); });
$("#clearSearch").addEventListener("click", () => { state.query = ""; $("#search").value = ""; $("#clearSearch").hidden = true; renderCards(); });
$("#projectFilter").addEventListener("change", (event) => { state.project = event.target.value; renderCards(); });
$("#supplierFilter").addEventListener("change", (event) => { state.supplier = event.target.value; renderCards(); });
$("#statusFilter").addEventListener("change", (event) => { state.status = event.target.value; renderCards(); });
$("#categoryTabs").addEventListener("click", (event) => { const button = event.target.closest("[data-category]"); if (!button) return; state.category = button.dataset.category; renderCategories(); renderCards(); });
$("#partsGrid").addEventListener("click", async (event) => {
  const button = event.target.closest("[data-copy]");
  if (!button) return;
  const part = parts.find((item) => item.id === button.dataset.id);
  if (!part) return;
  await copyToClipboard(copyText(part, button.dataset.copy));
  const previous = button.textContent;
  button.textContent = button.dataset.copy === "full" ? ui[state.language].copied : "✓";
  setTimeout(() => { button.textContent = previous; }, 1600);
});
