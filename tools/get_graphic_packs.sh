#!/usr/bin/env bash
# Fetches the two community graphic packs Zelda on PC builds on ("Graphics" and "FPS++" for Breath of the Wild) from
# the official Cemu graphic pack release this version was tested with, and puts them next to the executable's resources.
#
# The packs are made by the Cemu community (https://github.com/cemu-project/cemu_graphic_packs, CC0-1.0). They are not
# stored in this repository; the release ships the copy fetched here.
#
# Usage: tools/get_graphic_packs.sh [destination folder]
set -euo pipefail

version=987
sha256="FFF8DF5D8122B18871C0485FE266873644ED5254C15EDE727BAD1EF5F383951D"
url="https://github.com/cemu-project/cemu_graphic_packs/releases/download/Github$version/graphicPacks$version.zip"

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
destination="${1:-$root/bin/resources/pcedition/graphicPacks}"
cache="$root/build/downloads"
zip="$cache/graphicPacks$version.zip"

mkdir -p "$cache"
if [ ! -f "$zip" ]; then
  echo "Downloading $url"
  curl -fL --retry 3 -o "$zip.part" "$url"
  mv -f "$zip.part" "$zip"
fi

hash="$(sha256sum "$zip" | cut -d' ' -f1 | tr '[:lower:]' '[:upper:]')"
if [ "$hash" != "$sha256" ]; then
  rm -f "$zip"
  echo "graphicPacks$version.zip has an unexpected checksum ($hash). The download was removed; run the script again." >&2
  exit 1
fi

mkdir -p "$destination"
extract="$cache/extract"
rm -rf "$extract"
mkdir -p "$extract"
unzip -q -o "$zip" -d "$extract"

for pack in \
            "BreathOfTheWild/Graphics" \
            "BreathOfTheWild/Mods/FPS++" \
            "BreathOfTheWild/Mods/DrawDistance" \
            "BreathOfTheWild/Mods/Weather" \
            "BreathOfTheWild/Mods/RemoveFog" \
            "BreathOfTheWild/Mods/CelShading" \
            "BreathOfTheWild/Mods/DayLength" \
            "BreathOfTheWild/Mods/HUDElements" \
            "BreathOfTheWild/Mods/CameraSensitivity" \
            "BreathOfTheWild/Mods/MenuCursorSpeed" \
            "BreathOfTheWild/Mods/DivineLaserBeam" \
            "BreathOfTheWild/Mods/GlowingGuardianColor" \
            "BreathOfTheWild/Enhancements" \
            "BreathOfTheWild/Cheats/ArrowDrawSpeed" \
            "BreathOfTheWild/Cheats/Durability" \
            "BreathOfTheWild/Cheats/MasterModeOptions" \
            "BreathOfTheWild/Cheats/9999StampsLimit" \
            "BreathOfTheWild/Cheats/PreventRandomSpawns" \
            "BreathOfTheWild/Cheats/InfiniteAmiibo" \
            "BreathOfTheWild/Cheats/InfiniteArrows" \
            "BreathOfTheWild/Cheats/InfiniteDaruk" \
            "BreathOfTheWild/Cheats/InfiniteHearts" \
            "BreathOfTheWild/Cheats/InfiniteMipha" \
            "BreathOfTheWild/Cheats/InfiniteMotorcycleEnergy" \
            "BreathOfTheWild/Cheats/InfiniteRevali" \
            "BreathOfTheWild/Cheats/InfiniteStamina" \
            "BreathOfTheWild/Cheats/InfiniteUrbosa"; do
  if [ ! -d "$extract/$pack" ]; then
    echo "$pack was not found in graphicPacks$version.zip (the game looks for it in the packs of Cemu)" >&2
    continue
  fi
  rm -rf "${destination:?}/$pack"
  mkdir -p "$(dirname "$destination/$pack")"
  cp -r "$extract/$pack" "$destination/$pack"
  count="$(find "$destination/$pack" -type f | wc -l)"
  echo "$pack ($count files)"
done

cat > "$destination/README.txt" <<EOF
Cemu community graphic packs v$version (Breath of the Wild: Graphics, FPS++, DrawDistance, the cheats)
Source:  https://github.com/cemu-project/cemu_graphic_packs
License: CC0-1.0
EOF
echo "Graphic packs v$version are in $destination"
