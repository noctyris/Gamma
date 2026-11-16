<p align="center"><img src="https://github.com/Omega-Numworks/Omega-Design/blob/master/Omega-Themes.png" /></p>

# Gamma themes

Gamma uses the theme engine inherited from Omega. A theme is a JSON palette
compiled into the firmware or simulator; the default is `gamma_dark`.

## Included themes

- Gamma Dark (`gamma_dark`, default)
- Gamma Light (`gamma_light`)
- Omega Dark (`omega_dark`)
- Omega Light (`omega_light`)
- Upsilon Light (`upsilon_light`)
- Epsilon Dark (`epsilon_dark`)
- Epsilon Light (`epsilon_light`)

The Omega, Upsilon and Epsilon themes are kept for compatibility with the
lineage. Gamma-specific colours are defined in the two `gamma_*` files.

## Usage

```bash
make THEME_NAME=gamma_light -j"$(nproc)"
```

List the local themes:

```bash
python3 themes/themes_manager.py --list
```

## Third-party themes

Third-party themes can be loaded from a git repository:

```bash
make THEME_REPO={your repository url} THEME_NAME={your theme name}
```

Example:

```bash
make THEME_REPO=https://github.com/Omega-Numworks/Omega-Theme-Example.git THEME_NAME=omega_blue
```

To build a theme's icons from the colours in `themes/logocolors.json`, run:

```bash
./themes/script.sh your_theme_name
```

## License

The theme engine and the inherited themes are distributed under the
[CC BY-NC-SA 4.0](https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode)
license. NumWorks is a registered trademark.
