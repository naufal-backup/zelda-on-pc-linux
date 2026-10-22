# Changelog

All notable changes of Zelda on PC are listed here. The project follows
[Semantic Versioning](https://semver.org/spec/v2.0.0.html); the Linux builds carry the suffix `-linux`.

## v1.3.3-linux

The release of the PC Edition for Linux: the mouse is now the mouse everywhere, the camera turns all the way
around, and the settings of the game grew.

### The mouse

- **The mouse cursor works in the menus of the game.** The selection follows the pointer and a click opens the entry:
  the title screen, the pause menu, the inventory, the map, the dialogs.
- **The PC settings are a menu of the game too:** the pages are opened, scrolled and changed with the mouse.
- **The mouse cursor of the desktop was free in the menus on Wayland.** The events of the window did not reach the
  canvas there, so the position of the pointer and the state of its buttons are now taken from the desktop and from
  the raw input of the device.
- **The scope and the camera rune are aimed with the mouse**, like the bow: the cursor of the desktop stays out of
  the way there. The hand of the game in the weapon and rune selector is hidden while the selector is open.

### The camera

- **The camera turns all the way around.** While Link stands still it keeps the direction the mouse gave it, it can
  be spun around him any number of times and it does not swing back on its own. While he runs it swings back behind
  him as usual.
- **`Esc` → Camera → Aim Limit** puts a hard limit on the turn (the camera stops at the limit instead of spinning
  around). By default there is no limit.
- The scope, the camera rune and the bow are aimed with the mouse, in every camera of the game.

### The settings

- **`Esc` → Graphics**: the frame rate from 30 to 244 FPS (the mod has two sets of presets, the mode is switched to
  the one that has the value that is wanted), the frame rate of the cutscenes, the upscale and the downscale filter of
  the renderer, the draw distance of the objects and of the terrain.
- **`Esc` → Filters** (new): 22 presets of the picture (Clarity), the depth of field, the reflections and the
  anisotropic filtering of the textures.
- **`Esc` → Cheats** (new): the cheats of the graphic pack - infinite hearts, stamina, arrows, amiibo, the divine
  protections, the bow draw speed, the durability and the options of the master mode. The page is only there when it
  is wanted (`Esc` → Game → Enable Cheats).
- **The lock on has priority over the sprint**: pressing it stops the running, and while it is held Link walks.
- **The attack key attacks while the lock on is held.** The perfect guard with the attack button is an option now
  (off by default): the attack key attacks, and the perfect guard happens when it is pressed at the right moment.

### Fixed

- **The keys of the keyboard were mixed up** in the raw input on Linux: the codes of the kernel follow the QWERTY
  layout, not the alphabet, so `D` was the crouch and `W` threw the weapon.
- **The assertions that stop the game now say what failed** (they broke silently before).

### Linux

- The install puts everything into `~/.local/share/zelda-on-pc` (no root needed for the program itself).
- The rule of udev that gives the user access to the input devices is named `69-` on purpose: it has to be read
  before `71-seat.rules`, or systemd-logind does not hand the devices to the session and the mouse falls back to the
  pointer of the desktop.
- The version of the release is taken from its tag, so the names of the files always match it.
