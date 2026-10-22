#!/usr/bin/env bash
# Installs Zelda on PC (the Linux release) on this system.
#
# Without arguments the program goes to /opt/zelda-on-pc, a launcher in /usr/local/bin and a menu entry in
# /usr/share/applications (this needs root rights). With --user everything goes into the home of the user
# (~/.local/zelda-on-pc, ~/.local/bin, ~/.local/share/applications) and no root rights are needed.
#
# The saves, the settings and the caches live in the data folder of the user (~/.local/share/zelda-on-pc), because a
# program that is installed for the whole system cannot write next to itself. The folder of the game is remembered in
# the settings file there ([Game] Path=...).
#
# Usage: ./install.sh [--user] [--prefix <folder>] [--game <folder with the game>]
set -euo pipefail

prefix="/opt/zelda-on-pc"
game=""
mode="system"
user="${SUDO_USER:-$(id -un)}"

while [ $# -gt 0 ]; do
  case "$1" in
    --user) mode="user"; shift ;;
    --prefix) prefix="$2"; shift 2 ;;
    --game) game="$2"; shift 2 ;;
    -h|--help) sed -n '2,12p' "$0"; exit 0 ;;
    *) echo "unknown argument: $1" >&2; exit 1 ;;
  esac
done

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [ "$mode" = "user" ]; then
  home="$(eval echo ~"$user")"
  prefix="$home/.local/share/zelda-on-pc"
  bindir="$home/.local/bin"
  datadir="$home/.local/share"
  launcher="$bindir/zelda-on-pc"
  desktop="$datadir/applications/zelda-on-pc.desktop"
  icon="$datadir/icons/hicolor/256x256/apps/zelda-on-pc.ico"
  mkdir -p "$prefix" "$bindir" "$datadir/applications" "$datadir/icons/hicolor/256x256/apps"
else
  if [ "$(id -u)" -ne 0 ]; then
    echo "The installation into $prefix needs root rights: run it with sudo, or with --user." >&2
    exit 1
  fi
  bindir="/usr/local/bin"
  datadir="/usr/share"
  launcher="$bindir/zelda-on-pc"
  desktop="$datadir/applications/zelda-on-pc.desktop"
  icon="$datadir/icons/hicolor/256x256/apps/zelda-on-pc.ico"
  mkdir -p "$prefix" "$bindir" "$datadir/applications" "$datadir/icons/hicolor/256x256/apps"
fi

echo "Installing Zelda on PC into $prefix"
cp -r "$here/." "$prefix/"
chmod +x "$prefix/ZeldaOnPC"

# the launcher
cat > "$launcher" <<EOF
#!/usr/bin/env bash
exec "$prefix/ZeldaOnPC" "\$@"
EOF
chmod +x "$launcher"

# the menu entry
if [ -f "$prefix/zelda-on-pc.desktop" ]; then
  sed "s|^Exec=zelda-on-pc %f|Exec=$prefix/ZeldaOnPC %f|" "$prefix/zelda-on-pc.desktop" > "$desktop"
  chmod +x "$desktop"
fi

# the icon of the window (the one of the game, made from the dump at the first start)
if [ -f "$prefix/game.ico" ]; then
  cp "$prefix/game.ico" "$icon"
  (command -v gtk-update-icon-cache > /dev/null && gtk-update-icon-cache -f "$datadir/icons/hicolor" || true)
fi

# the data folder of the user (the saves, the settings, the caches)
data="$(eval echo ~"$user")/.local/share/zelda-on-pc/data"
mkdir -p "$data"
[ "$(id -u)" -eq 0 ] && chown -R "$user" "$data"

if [ -n "$game" ]; then
  settings="$data/settings.ini"
  if [ ! -f "$settings" ]; then
    printf '[Game]\nPath=%s\n' "$game" > "$settings"
    [ "$(id -u)" -eq 0 ] && chown "$user" "$settings"
    echo "The game folder was written to $settings"
  else
    echo "$settings exists already, it was not changed."
  fi
fi

# the rule that gives the user access to the input devices (the game reads them directly)
if [ "$(id -u)" -eq 0 ] && [ -f "$here/69-zelda-on-pc-input.rules" ]; then
  cp "$here/69-zelda-on-pc-input.rules" /etc/udev/rules.d/
  udevadm control --reload-rules
  # "add" so that systemd-logind hands the devices of the seat to the session right away
  udevadm trigger --action=add --subsystem-match=input
  echo "The rule for the input devices was installed."
fi

echo
echo "Zelda on PC is installed. Start it with 'zelda-on-pc'."
echo "Put the folder of your copy of the game into $data/settings.ini ([Game] Path=...) or next to the program"
echo "in a folder named 'game'."
