# Portfolio audit — 11 September 2026

Scope: all 25 HTML pages, the dynamically rendered catalogue of 108 parts, local media and page links, and 407 unique HTTP URLs found in the pages and catalogue.

## Repairs

- Removed literal tool-output truncation messages from the home page. Rebuilt the damaged quadruped release copy and restored the bench-test and first-steps video markup from commit `555ca914e3064917d898fa57cd4569563c7c69a7`. The gallery again contains four correctly nested videos.
- Added the missing photograph of the ECSiNG six-switch set from the exact linked Amazon listing, ASIN B0CXSLG6MY.
- All 108 catalogue image references now resolve to files in this repository. Extracted the existing 63 embedded photographs and downloaded 44 product photographs including the newly added switch set. The blocked external laser photograph was replaced with the existing owner's photograph of the laser in use on the CNC.
- The page loads a 5,823-byte image map instead of 1,697,236 bytes of legacy image JavaScript. Image files load separately and lazily. Original photographs retain their bytes and dimensions. Source information is in `parts/image-sources.json`; the old image-data scripts remain available for recovery but are no longer loaded by the catalogue.
- Reused the local catalogue images in the hardware-gotchas panel.
- Fixed the `#gotchas` target and same-page navigation. Category buttons restore the normal catalogue.
- Added visible Product / Search store labels. 85 catalogue URLs are store searches, so they are no longer presented as exact product pages. Affiliate URLs and tags were preserved.
- Enlarged catalogue action text and category controls, and enabled keyboard activation of home-page gallery images.

- Final live verification exposed failed YouTube thumbnails on the home page. Saved all five YouTube thumbnails used by the HTML pages locally, including the radio and aluminium update thumbnails. Downloaded the original images from `https://img.youtube.com/vi/VIDEO_ID/hqdefault.jpg`; video links remain unchanged.

## Validation

- Loaded all 25 pages and their local JavaScript in a DOM test harness: no application script exceptions or missing referenced local files.
- Checked all 108 catalogue image files by decoding them. No missing catalogue images or image placeholders remain.
- Exercised English/Polish switching, search and clear, product/search labels, the gotchas link, return to the catalogue, and direct `#gotchas` entry.
- Verified all four quadruped videos, their poster/source files, restored download links and their DOM nesting.
- Inspected the existing home page and 11 distinct content pages in the cloud browser at desktop width. No horizontal document overflow was found. Mobile media rules were reviewed in source; a separate mobile viewport/device test was not available.
- Confirmed successful GitHub Pages publication of the main repair, then checked the live catalogue image count (108), switch photograph loading, PL/EN controls, same-page gotchas navigation and the restored wiring anchor.
- Historical unused corrupt image files remain in backups. They are not referenced by active pages and were not deleted.

## External-link limitations

HTTP checks were attempted for 407 unique URLs. Some first-pass requests timed out; those results do not establish broken links. Targeted retries confirmed the previously timed-out YouTube page links, PPD instructions, Pi Hut search and the linked GOV.UK page with HTTP 200.

The following could not be certified during this audit:

- Many Amazon search requests returned HTTP 503 or timed out. Product availability and exact search matches are not guaranteed by a successful HTTP response.
- Ko-fi and the Geekworm X1203 wiki returned HTTP 403.
- Two AliExpress item pages timed out.
- The external Edgars Auto redesign demo returned HTTP 502. The local Edgars Auto project page itself loaded successfully.

These links were retained because restricted or temporary responses do not prove that the destinations are permanently unavailable. No replacement product IDs or affiliate links were guessed.
