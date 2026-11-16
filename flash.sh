#!/usr/bin/env bash
#
# Build and flash Gamma on a NumWorks calculator.
#
# Unlike the old one-liners, this script:
#   - always sets MODEL explicitly, so the build and the flash target use the
#     same output directory;
#   - builds as the current user; sudo is only used for USB access and upload;
#   - detects the DFU device type instead of flashing blindly;
#   - stops after a timeout instead of waiting forever;
#   - refuses to build on top of root-owned files left by an old `sudo make`.
#
# Usage:
#   ./flash.sh [A|B|n0110|n0100] [options]

set -euo pipefail

readonly SCRIPT_NAME="$(basename "$0")"
readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$REPO_ROOT"

die() {
  printf 'erreur : %s\n' "$*" >&2
  exit 1
}

info() {
  printf '\n==> %s\n' "$*"
}

warn() {
  printf 'attention : %s\n' "$*" >&2
}

usage() {
  cat <<EOF
Usage : ./$SCRIPT_NAME [cible] [options]

Cibles :
  A           slot A via le bootloader Gamma (défaut)
  B           slot B via le bootloader Gamma
  n0110       ancien mode n0110, sans bootloader
  n0100       ancien mode n0100, sans bootloader

Options :
  -c, --clean             nettoie le dossier de build du modèle choisi
  -u, --username NOM      nom affiché dans À propos
                          (défaut : \$GAMMA_USERNAME, sinon Noctyris)
  -j, --jobs N            jobs de compilation (défaut : nproc)
  -t, --timeout SECONDES  attente maximale du mode DFU (défaut : 300)
  -h, --help              affiche cette aide

Exemples :
  ./$SCRIPT_NAME                 # construit et flashe le slot A
  ./$SCRIPT_NAME B               # construit et flashe le slot B
  ./$SCRIPT_NAME A --clean       # repart de zéro pour le slot A
  ./$SCRIPT_NAME n0110           # ancien mode, sans bootloader

Le build se fait sans sudo. Le script ne demande sudo qu'après la compilation,
pour détecter la calculatrice et écrire l'image DFU.
EOF
}

target=A
clean=0
username="${GAMMA_USERNAME:-Noctyris}"
jobs="$(nproc 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)"
timeout=300

while (( $# > 0 )); do
  case "$1" in
    A|a)
      target=A
      ;;
    B|b)
      target=B
      ;;
    n0110|N0110|legacy)
      target=n0110
      ;;
    n0100|N0100)
      target=n0100
      ;;
    -c|--clean)
      clean=1
      ;;
    -u|--username)
      [[ $# -ge 2 ]] || die "il manque la valeur de $1"
      username="$2"
      shift
      ;;
    -j|--jobs)
      [[ $# -ge 2 ]] || die "il manque la valeur de $1"
      jobs="$2"
      shift
      ;;
    -t|--timeout)
      [[ $# -ge 2 ]] || die "il manque la valeur de $1"
      timeout="$2"
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      die "argument inconnu : $1 (voir ./$SCRIPT_NAME --help)"
      ;;
  esac
  shift
done

[[ "$jobs" =~ ^[1-9][0-9]*$ ]] || die "--jobs doit être un entier positif"
[[ "$timeout" =~ ^[1-9][0-9]*$ ]] || die "--timeout doit être un entier positif"

mass_erase=0
case "$target" in
  A)
    model=bootloader
    dfu_name=epsilon.A.dfu
    expected_pid=a291
    slot_label="slot A (bootloader)"
    ;;
  B)
    model=bootloader
    dfu_name=epsilon.B.dfu
    expected_pid=a291
    slot_label="slot B (bootloader)"
    ;;
  n0110)
    model=n0110
    dfu_name=epsilon.dfu
    expected_pid=df11
    slot_label="n0110 legacy (sans bootloader)"
    ;;
  n0100)
    model=n0100
    dfu_name=epsilon.dfu
    expected_pid=df11
    slot_label="n0100 legacy (sans bootloader)"
    mass_erase=1
    ;;
esac

# Read the real build directory and version from make instead of guessing them.
make_info="$(make --no-print-directory MODEL="$model" info 2>/dev/null || true)"
version="$(printf '%s\n' "$make_info" | sed -n 's/^GAMMA_VERSION = //p' | head -n 1)"
build_dir="$(printf '%s\n' "$make_info" | sed -n 's/^BUILD_DIR = //p' | head -n 1)"
[[ -n "$build_dir" ]] || build_dir="output/release/device/$model"

info "Configuration Gamma"
printf '    cible   : %s\n' "$slot_label"
printf '    modèle  : %s\n' "$model"
printf '    version : %s\n' "${version:-inconnue}"
printf '    build   : %s\n' "$build_dir"

# A previous `sudo make` can leave root-owned objects that a user build cannot
# overwrite. Fix them explicitly instead of silently building as root.
if [[ -d "$build_dir" ]]; then
  if [[ ! -w "$build_dir" ]]; then
    die "$build_dir n'est pas accessible en écriture.
       Corrige avec : sudo chown -R $(id -u):$(id -g) $build_dir"
  fi
  root_owned="$(find "$build_dir" ! -user "$(id -u)" -print -quit 2>/dev/null || true)"
  if [[ -n "$root_owned" ]]; then
    warn "des fichiers de $build_dir appartiennent à root (ancien « sudo make »)."
    printf '    exemple : %s\n' "$root_owned" >&2
    if [[ -t 0 ]]; then
      answer=""
      read -r -p "Corriger les droits avec sudo chown ? [Y/n] " answer || true
      if [[ -z "$answer" || "$answer" =~ ^[yY]$ ]]; then
        command -v sudo >/dev/null 2>&1 || die "sudo est introuvable."
        sudo chown -R "$(id -u):$(id -g)" "$build_dir"
      else
        die "corrige les droits puis relance ./$SCRIPT_NAME"
      fi
    else
      die "corrige les droits avec : sudo chown -R $(id -u):$(id -g) $build_dir"
    fi
  fi
fi

if (( clean )); then
  info "Nettoyage de $model"
  make MODEL="$model" clean
fi

info "Compilation ($jobs jobs)"
make MODEL="$model" GAMMA_USERNAME="$username" -j"$jobs" "$dfu_name"

dfu_file="$build_dir/$dfu_name"
elf_file="${dfu_file%.dfu}.elf"
[[ -f "$dfu_file" ]] || die "l'artefact $dfu_file n'a pas été produit."

if [[ -n "$version" && -f "$elf_file" ]] &&
   ! grep -aqF -- "$version" "$elf_file"; then
  warn "l'ELF ne contient pas la chaîne de version $version ; l'artefact est peut-être périmé."
fi

python_bin="$(command -v "${PYTHON:-python3}" 2>/dev/null || true)"
[[ -n "$python_bin" ]] || die "python3 est introuvable."
"$python_bin" -c 'import usb.core' >/dev/null 2>&1 ||
  die "PyUSB n'est pas installé pour $python_bin.
       Installe-le avec : $python_bin -m pip install pyusb"

if (( EUID != 0 )); then
  command -v sudo >/dev/null 2>&1 || die "sudo est requis pour accéder à l'USB."
  info "Autorisation d'accès à l'USB (sudo)"
  sudo -v || die "autorisation sudo refusée."
  sudo "$python_bin" -c 'import usb.core' >/dev/null 2>&1 ||
    die "PyUSB n'est pas disponible pour root.
         Installe-le avec : sudo $python_bin -m pip install pyusb"
fi

list_dfu() {
  if (( EUID == 0 )); then
    "$python_bin" build/device/dfu.py -l 2>/dev/null
  else
    sudo "$python_bin" build/device/dfu.py -l 2>/dev/null
  fi
}

info "En attente de la calculatrice en mode DFU (${timeout}s max)"
case "$target" in
  A|B)
    cat <<'EOF'
    Branche l'USB, maintiens 6 puis appuie sur RESET.
    Dans le menu du bootloader : 4 - Installer Mode, puis 1 - Flash Slots.
EOF
    ;;
  n0110)
    cat <<'EOF'
    Branche l'USB, maintiens 6 puis appuie sur RESET.
EOF
    ;;
  n0100)
    cat <<'EOF'
    Branche l'USB puis appuie sur RESET (à l'arrière de la calculatrice).
EOF
    ;;
esac

deadline=$((SECONDS + timeout))
devices=""
while (( SECONDS < deadline )); do
  devices="$(list_dfu | grep -E 'ID 0483:(a291|df11)' || true)"
  if [[ -n "$devices" ]]; then
    break
  fi
  sleep 2
done

if [[ -z "$devices" ]]; then
  die "aucune calculatrice DFU détectée après ${timeout}s.
       Vérifie le câble, le mode DFU et l'accès USB (PyUSB/udev)."
fi

device_count="$(printf '%s\n' "$devices" | grep -c . || true)"
if (( device_count > 1 )); then
  printf '%s\n' "$devices" >&2
  die "plusieurs calculatrices DFU détectées ; débranche les autres."
fi

pid="$(printf '%s\n' "$devices" | sed -n '1s/.*ID 0483:\([0-9a-fA-F]*\).*/\1/p')"
if [[ "$pid" != "$expected_pid" ]]; then
  warn "calculatrice détectée en DFU $pid, mais la cible $target attend $expected_pid."
  case "$target:$pid" in
    A:df11|B:df11)
      warn "DFU ROM STM32 actif : le bootloader Gamma n'est pas lancé."
      warn "un slot A/B écrit ainsi ne démarrera pas sans bootloader."
      ;;
    n0110:a291|n0100:a291)
      warn "bootloader Gamma actif : une image legacy peut écraser le bootloader."
      ;;
  esac
  answer=""
  read -r -p "Continuer quand même ? [y/N] " answer || true
  [[ "$answer" =~ ^[yY]$ ]] || die "flash annulé."
fi

size="$(stat -c%s "$dfu_file" 2>/dev/null || stat -f%z "$dfu_file")"
info "Écriture de $dfu_name"
printf '    fichier : %s\n' "$dfu_file"
printf '    taille  : %s octets\n' "$size"
if command -v sha256sum >/dev/null 2>&1; then
  printf '    sha256  : %s\n' "$(sha256sum "$dfu_file" | cut -d' ' -f1)"
elif command -v shasum >/dev/null 2>&1; then
  printf '    sha256  : %s\n' "$(shasum -a 256 "$dfu_file" | cut -d' ' -f1)"
fi

dfu_args=(-u "$dfu_file")
if (( mass_erase )); then
  dfu_args=(-m "${dfu_args[@]}")
fi

if (( EUID == 0 )); then
  "$python_bin" build/device/dfu.py "${dfu_args[@]}"
else
  sudo "$python_bin" build/device/dfu.py "${dfu_args[@]}"
fi

info "Flash terminé"
printf '    Vérifie : Réglages → À propos → Version de Gamma (%s).\n' "${version:-?}"
if [[ "$model" == bootloader ]]; then
  printf '    Le menu du bootloader doit afficher : 1 - Slot %s -- Gamma v%s\n' \
    "$target" "${version:-?}"
  printf '    (« 5 - About » affiche la version du bootloader, pas celle du slot.)\n'
fi
