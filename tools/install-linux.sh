#!/usr/bin/env bash
# Installs Zelda on PC (the Linux release) on this system.
#
# The program goes to /opt/zelda-on-pc, a launcher in /usr/local/bin and a menu entry in /usr/share/applications.
# The saves, the settings and the caches live in the folder of the program (data and settings.ini), so the
# installation can be removed by deleting that folder.
#
# Usage: sudo ./install.sh [--prefix /opt/zelda-on-pc] [--game <folder with the game>]
set -euo pipefail

prefix="/opt/zelda-on-pc"
game=""

while [ $# -gt 0 ]; do
  case "$1" in
    --prefix) prefix="$2"; shift 2 ;;
    --game) game="$2"; shift 2 ;;
    -h|--help) sed -n '2,10p' "$0"; exit 0 ;;
    *) echo "unknown argument: $1" >&2; exit 1 ;;
  esac
done

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [ "$(id -u)" -ne 0 ]; then
  echo "The installation needs root rights: run it with sudo." >&2
  exit 1
fi

echo "Installing Zelda on PC into $prefix"
mkdir -p "$prefix"
cp -r "$here/." "$prefix/"
chmod +x "$prefix/ZeldaOnPC"

# the launcher
cat > /usr/local/bin/zelda-on-pc <<EOF
#!/usr/bin/env bash
exec "$prefix/ZeldaOnPC" "\$@"
EOF
chmod +x /usr/local/bin/zelda-on-pc

# the menu entry
if [ -f "$prefix/zelda-on-pc.desktop" ]; then
  sed "s|^Exec=zelda-on-pc %f|Exec=$prefix/ZeldaOnPC %f|" "$prefix/zelda-on-pc.desktop" > /usr/share/applications/zelda-on-pc.desktop
  chmod +x /usr/share/applications/zelda-on-pc.desktop
fi

# the icon of the window (the one of the game, made from the dump at the first start)
if [ -f "$prefix/game.ico" ]; then
  mkdir -p /usr/share/icons/hicolor/256x256/apps
  cp "$prefix/game.ico" /usr/share/icons/hicolor/256x256/apps/zelda-on-pc.ico
  (command -v gtk-update-icon-cache > /dev/null && gtk-update-icon-cache -f /usr/share/icons/hicolor || true)
fi

if [ -n "$game" ]; then
  if [ ! -f "$prefix/settings.ini" ]; then
    printf '[Game]\nPath=%s\n' "$game" > "$prefix/settings.ini"
    echo "The game folder was written to $prefix/settings.ini"
  else
    echo "$prefix/settings.ini exists already, it was not changed."
  fi
fi

echo
echo "Zelda on PC is installed. Start it with 'zelda-on-pc'."
echo "Put the folder of your copy of the game into $prefix/settings.ini ([Game] Path=...) or next to the program"
echo "in a folder named 'game'."
