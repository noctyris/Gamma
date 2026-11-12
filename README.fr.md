<p align="center">
  <strong>Gamma</strong><br/>
  Un système d'exploitation communautaire pour les calculatrices NumWorks.
</p>

<p align="center">
  <a href="https://github.com/noctyris/Gamma/issues"><img alt="Issues" src="https://img.shields.io/github/issues/noctyris/Gamma?style=for-the-badge" /></a>
  <a href="https://github.com/noctyris/Gamma/commits/gamma-dev"><img alt="Dernier commit" src="https://img.shields.io/github/last-commit/noctyris/Gamma/gamma-dev?style=for-the-badge" /></a>
  <a href="./LICENSE.md"><img alt="Licence : CC BY-NC-SA 4.0" src="https://img.shields.io/badge/License-CC%20BY--NC--SA%204.0-525252.svg?labelColor=292929&logo=creative%20commons&style=for-the-badge" /></a>
</p>

> 🇬🇧 You prefer English? Read the [English README](./README.md).

## À propos

Gamma est un système d'exploitation communautaire pour les calculatrices
NumWorks. Il prolonge la lignée **Epsilon 15.5.0 → Omega → Upsilon → Gamma** et
conserve les fonctionnalités des projets précédents tout en ajoutant les siennes.

Gamma est basé sur les sources publiques d'**Epsilon 15.5.0**. Epsilon 16.0.0 et
les versions suivantes sont publiées par NumWorks sans la licence CC BY-NC-SA
(le README amont indique « All rights reserved ») : Gamma n'importe donc
**aucun** code de ces versions. Voir [NOTICE.md](./NOTICE.md).

## Fonctionnalités

- Thème Gamma et moteur de thèmes, avec thèmes clair et sombre
- Fonds d'écran et applications externes (NWA)
- Applications RPN et Atomic
- Application Reader
- Extensions Python : ulab, surcharge d'opérateurs, `os.uname`, `getlogin`
- Accessibilité : couleurs inversées, loupe, correction gamma RVB
- Application Finance (TVM)
- Améliorations du Grapheur : tracés implicites, inéquations, types de fonctions
- Huit langues : anglais, français, néerlandais, portugais, italien, allemand, espagnol, hongrois
- Support des NumWorks n0100 et n0110, avec les slots de bootloader A/B/Khi

## Version

`GAMMA_VERSION` est dérivé de l'historique git, sans fichier de version à
modifier à la main :

- un commit normal incrémente le numéro de patch (`1.0.0` → `1.0.1`) ;
- un merge d'intégration incrémente le numéro mineur et remet le patch à zéro
  (`1.0.1` → `1.1.0`) ;
- le numéro majeur se fixe avec `GAMMA_VERSION_MAJOR` ou un `GAMMA_VERSION`
  complet pour une release.

La version s'affiche dans le menu About du bootloader et à côté des slots Gamma.
`make info` affiche la valeur courante.

## Compilation

### Dépendances

Gamma utilise la même chaîne d'outils qu'Epsilon 15.5.0. Sur Debian ou Ubuntu :

```bash
sudo apt-get install build-essential git imagemagick libx11-dev libxext-dev \
  libfreetype6-dev libpng-dev libjpeg-dev pkg-config \
  gcc-arm-none-eabi binutils-arm-none-eabi
```

Sur macOS avec Homebrew :

```bash
brew install freetype imagemagick libpng libusb pkg-config python3
python3 -m venv .venv
.venv/bin/pip3 install setuptools lz4 pypng stringcase
```

Sous Windows, utilisez MSYS2 ou WSL 2. Le compilateur croisé ARM n'est nécessaire
que pour la calculatrice.

### Clonage

```bash
git clone --recursive https://github.com/noctyris/Gamma.git
cd Gamma
git checkout gamma-dev
```

### Simulateur

```bash
make PLATFORM=simulator TARGET=linux -j$(nproc)
```

Le simulateur est produit dans `output/release/simulator/linux/`. Pour compiler
et lancer les tests :

```bash
make PLATFORM=simulator TARGET=linux test -j$(nproc)
./output/release/simulator/linux/test.bin --headless
```

### Simulateur web

Installez emsdk 4.0.22, activez-le, puis :

```bash
make PLATFORM=simulator TARGET=web htmlpack.zip -j$(nproc)
```

L'archive est produite dans `output/release/simulator/web/htmlpack.zip`.

### NumWorks n0100

```bash
make MODEL=n0100 EPSILON_I18N=fr GAMMA_USERNAME="Votre nom" -j$(nproc)
make MODEL=n0100 epsilon_flash
```

Une seule langue est supportée sur n0100 ; le build utilise l'anglais par défaut
et retire Reader et ulab.

### NumWorks n0110

Avec le bootloader Gamma/Omega/Upsilon :

```bash
make MODEL=bootloader GAMMA_USERNAME="Votre nom" -j$(nproc)
make epsilon.A_flash
```

Sans bootloader (ancien mode, déconseillé) :

```bash
make MODEL=n0110 GAMMA_USERNAME="Votre nom" -j$(nproc)
make MODEL=n0110 epsilon_flash
```

`GAMMA_USERNAME` est l'orthographe recommandée. `OMEGA_USERNAME` reste accepté
pour compatibilité avec les anciens scripts.

### Autres cibles

```bash
make PLATFORM=simulator TARGET=android -j$(nproc)   # epsilon.apk
make PLATFORM=simulator TARGET=3ds -j$(nproc)       # epsilon.3dsx
make PLATFORM=simulator TARGET=fxcg -j$(nproc)      # epsilon.g3a
make PLATFORM=simulator TARGET=macos -j$(nproc)
make PLATFORM=simulator TARGET=windows -j$(nproc)
```

Voir `make help` pour la liste complète.

## Installation

Il n'existe pas encore d'installateur Gamma officiel ni de build hébergé.
Compilez le firmware vous-même, vérifiez l'artefact, puis flashez-le avec l'outil
adapté. Ne flashez jamais un binaire provenant d'une source non fiable : un
firmware a un accès complet à la calculatrice.

Le bootloader conserve deux slots. Un build compatible bootloader peut être
flashé dans le slot A ou B sans toucher à l'autre slot. L'ancien mode n0110 sans
bootloader n'offre pas cette protection.

## Contribution

Les contributions sont bienvenues. Lisez [CONTRIBUTING.md](./CONTRIBUTING.md)
avant d'ouvrir une pull request. Pour un bug, utilisez les modèles d'issue. Pour
un problème de sécurité, suivez [SECURITY.md](./.github/SECURITY.md).

La branche de développement principale est `gamma-dev` ; `gamma` est la branche
stable. N'ouvrez pas de pull request directement sur un tag de release.

## Licence

Gamma est distribué sous
[CC BY-NC-SA 4.0](https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode.fr),
comme Epsilon 15.5.0, Omega et Upsilon.

- **Epsilon 15.5.0** : CC BY-NC-SA 4.0. C'est la base de Gamma.
- **Epsilon 16.0.0 et suivants** : pas de fichier `LICENSE` ; le README amont
  indique « All rights reserved ». Gamma n'inclut pas de code de ces versions.
- **Omega** et **Upsilon** : CC BY-NC-SA 4.0.

Voir [NOTICE.md](./NOTICE.md) pour la notice complète, les marques et les
composants tiers.

## Liens

- [Suivi des issues](https://github.com/noctyris/Gamma/issues)
- [Discussions](https://github.com/noctyris/Gamma/discussions)
- [Documentation de compilation](./docs/)
- [Guide de contribution](./CONTRIBUTING.md)

NumWorks est une marque déposée de NumWorks SAS. Nintendo et Nintendo 3DS sont
des marques déposées de Nintendo of America Inc. Casio est une marque déposée de
Casio Computer Co., Ltd. Aucune de ces entreprises n'est associée à Gamma.
