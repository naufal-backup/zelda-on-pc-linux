#!/usr/bin/env bash
# Stages the files of the Linux release of Zelda on PC (the same set the Windows installer puts on the disk).
#
# The staged folder (build/stage) is a complete portable copy: add a settings.ini with the game folder
# ([Game] Path=...) or a "game" folder next to the executable and it runs from anywhere.
#
# Usage: tools/package-linux.sh [-Version 1.3.1]
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="$ROOT/bin"
STAGE="$ROOT/build/stage"
VERSION="1.3.1"

while [ $# -gt 0 ]; do
  case "$1" in
    -Version|--version) VERSION="$2"; shift 2 ;;
    *) echo "unknown argument: $1" >&2; exit 1 ;;
  esac
done

EXE="$BIN/ZeldaOnPC_release"
if [ ! -x "$EXE" ]; then
  echo "$EXE was not found. Run tools/build-linux.sh first." >&2
  exit 1
fi

# ---- the community graphic packs (not stored in the repository) ----
PACKS="$BIN/resources/pcedition/graphicPacks"
if [ ! -f "$PACKS/BreathOfTheWild/Graphics/rules.txt" ]; then
  "$ROOT/tools/get_graphic_packs.sh" "$PACKS"
fi

# ---- stage: exactly the files that are installed (nothing written at run time, no game data) ----
rm -rf "$STAGE"
mkdir -p "$STAGE"

# the executable
cp "$EXE" "$STAGE/ZeldaOnPC"
chmod +x "$STAGE/ZeldaOnPC"

# the game profiles of the three regions
if [ -d "$BIN/gameProfiles/default" ]; then
  mkdir -p "$STAGE/gameProfiles/default"
  for profile in "$BIN"/gameProfiles/default/00050000101c9[345]00.ini; do
    [ -e "$profile" ] || continue
    cp "$profile" "$STAGE/gameProfiles/default/$(basename "$profile" | tr '[:upper:]' '[:lower:]')"
  done
fi

# the replacement system fonts of Cemu, the key prompt font and the community graphic packs
mkdir -p "$STAGE/resources/pcedition"
if [ -d "$BIN/resources/sharedFonts" ]; then
  cp -r "$BIN/resources/sharedFonts" "$STAGE/resources/"
fi
if [ -f "$BIN/resources/pcedition/External_00.bffnt" ]; then
  cp "$BIN/resources/pcedition/External_00.bffnt" "$STAGE/resources/pcedition/"
fi
cp -r "$PACKS" "$STAGE/resources/pcedition/graphicPacks"

# the licences
cp "$ROOT/LICENSE.txt" "$STAGE/LICENSE.txt"
cp "$ROOT/installer/THIRD-PARTY-NOTICES.txt" "$STAGE/THIRD-PARTY-NOTICES.txt"
mkdir -p "$STAGE/licenses"
if [ -f "$ROOT/dependencies/imgui/LICENSE.txt" ]; then
  cp "$ROOT/dependencies/imgui/LICENSE.txt" "$STAGE/licenses/imgui.txt"
fi
if [ -f "$ROOT/dependencies/FidelityFX-SDK/LICENSE.txt" ]; then
  cp "$ROOT/dependencies/FidelityFX-SDK/LICENSE.txt" "$STAGE/licenses/FidelityFX-SDK.txt"
fi

# the launcher of the desktop and the install script
cp "$ROOT/dist/linux/zelda-on-pc.desktop" "$STAGE/zelda-on-pc.desktop"
cp "$ROOT/dist/linux/99-zelda-on-pc-input.rules" "$STAGE/99-zelda-on-pc-input.rules"
cp "$ROOT/tools/install-linux.sh" "$STAGE/install.sh"
chmod +x "$STAGE/install.sh"

# the archive
ARCHIVE="$ROOT/build/zelda-on-pc-linux-$VERSION-linux-x64.tar.gz"
mkdir -p "$(dirname "$ARCHIVE")"
tar -czf "$ARCHIVE" -C "$STAGE" .
cd "$(dirname "$ARCHIVE")" && sha256sum "$(basename "$ARCHIVE")" > "$(basename "$ARCHIVE").sha256"

echo "Staged $(find "$STAGE" -type f | wc -l) files in $STAGE"
echo "Archive: $ARCHIVE"
