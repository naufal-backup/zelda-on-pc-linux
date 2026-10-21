#!/usr/bin/env bash
# Installs Zelda on PC (the Linux release) on this system.
#
# The program goes to /opt/zelda-on-pc, a launcher in /usr/local/bin and a menu entry in /usr/share/applications.
#
# The saves, the settings and the caches live in the data folder of the user (~/.local/share/zelda-on-pc), because a
# program that is installed for the whole system cannot write next to itself. The folder of the game is remembered in
# the settings file there ([Game] Path=...).
#
# Usage: sudo ./install.sh [--prefix /opt/zelda-on-pc] [--game <folder with the game>]
set -euo pipefail

prefix="/opt/zelda-on-pc"
game=""
user="${SUDO_USER:-$(id -un)}"

while [ $# -gt 0 ]; do
  case "$1" in
    --prefix) prefix="$2"; shift 2 ;;
    --game) game="$2"; shift 2 ;;
    -h|--help) sed -n '2,12p' "$0"; exit 0 ;;
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

# the data folder of the user (the saves, the settings, the caches)
data="$(eval echo ~"$user")/.local/share/zelda-on-pc/data"
mkdir -p "$data"
chown -R "$user" "$data"

if [ -n "$game" ]; then
  settings="$data/settings.ini"
  if [ ! -f "$settings" ]; then
    printf '[Game]\nPath=%s\n' "$game" > "$settings"
    chown "$user" "$settings"
    echo "The game folder was written to $settings"
  else
    echo "$settings exists already, it was not changed."
  fi
fi

echo
echo "Zelda on PC is installed. Start it with 'zelda-on-pc'."
echo "Put the folder of your copy of the game into $data/settings.ini ([Game] Path=...) or next to the program"
echo "in a folder named 'game'."
