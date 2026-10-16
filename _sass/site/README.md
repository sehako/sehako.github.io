# Site styles

`_sass/minimal-mistakes/` is the vendored Minimal Mistakes 4.24.0 theme.
Everything that is specific to this site lives here, so the theme files stay
close to upstream and a theme upgrade is a diff, not an archaeology dig.

## Layout

| Path | Purpose |
| --- | --- |
| `assets/css/main.scss` | Entry point. Loads, in order: skin → layout → theme. The skin name comes from `minimal_mistakes_skin` in `_config.yml` (Liquid fills in the `@use` path). |
| `site/skins/_<skin>.scss` | One wrapper per skin. Forwards the upstream skin and carries the site's palette tweaks as module configuration (`dirt`: white background; `dark`: grey primary colour). |
| `site/_layout.scss` | Configures sidebar widths (250px / 300px instead of 300px / 400px). |
| `site/_theme.scss` | Cascade order: each upstream partial is followed by the site partial that adjusts it. |
| `site/_<name>.scss` | Additive overrides for the upstream partial of the same name. |
| `minimal-mistakes/_breakpoint.scss` | `breakpoint($min)` / `breakpoint-max($max)`: native `@media` in ems. Replaces the vendored Breakpoint library. |
| `minimal-mistakes/_grid.scss` | `grid-span(n, of)` / `grid-gutter(of)`: the percentage math Susy produced. Replaces the vendored Susy library. |
| `minimal-mistakes/_tools.scss` | What a partial needs: `@use "tools" as *;` gives variables, layout, mixins, breakpoints, grid. |

### Configuration order

`@use … with (…)` can configure a module only the first time it is loaded, so
`main.scss` loads the skin (which configures `variables`) and `site/layout`
(which configures `layout`) before anything else. Skins keep their palette in
`!default` locals, so a derived colour (e.g. the dark skin's link colour)
follows a site override of its base colour.

## Patches inside upstream files

Removals cannot be expressed as an override, so these three upstream partials
carry a small `SITE PATCH` comment. Re-apply them after upgrading:

* `_base.scss` – the `p > code, a > code, …` inline-code rule is dropped
  (`site/_page.scss` styles `.page__content :not(pre) > code` instead).
* `_search.scss` – `border: none` on `.search-input` is dropped.
* `_notices.scss` – blockquote notices get a tinted `border-left-color`.

Also not loaded: `minimal-mistakes/_tables.scss` is replaced by
`site/_tables.scss`. `.comment__date { @extend .page__meta }` moved from `_page.scss`
to `site/_theme.scss`, because `@extend` only reaches modules the extending
file loads.

## Checks

* `ruby test/sass_test.rb` – zero deprecation warnings for every skin, a build
  under `_config.ci.yml` (deprecations are fatal), no `@import` left, and the
  normalised compiled CSS equals `test/fixtures/main.baseline.css`.
  After an intentional style change: `UPDATE_CSS_BASELINE=1 ruby test/sass_test.rb`.
* `script/visual_snapshot <built-site> <out-dir>` – headless Chrome
  screenshots of key pages at four widths; compare two runs with
  `script/visual_snapshot --compare <dir-a> <dir-b>`.
