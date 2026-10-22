<p align="center"><img src="docs/icon.png" width="96" alt=""></p>

<h1 align="center">Zelda on PC</h1>

<p align="center">
<b>The Legend of Zelda: Breath of the Wild, set up as a PC game.</b><br>
Mouse and keyboard · <b>1:1 mouse camera that turns all the way around</b> · PC menus and key prompts<br>
<b>AMD FSR 3 frame generation</b> · shader filters · cheats<br>
<i>Runs your own copy of the game on the <a href="https://github.com/cemu-project/Cemu">Cemu</a> emulator.</i>
</p>

---

## It runs on Cemu

Zelda on PC is **not a port and not a recompilation**. It is a fork of [Cemu](https://github.com/cemu-project/Cemu),
the open source Wii U emulator, built to run exactly one game and to make it feel like a native PC release. The
original game code runs inside the emulator, unchanged on disk; everything this project adds sits around it.

**You need your own dump of the game.** No game files, keys or Nintendo code are in this repository or in the
installer, and none are downloaded.

| | Cemu | Zelda on PC |
|---|---|---|
| Start | Emulator window, game list, per-game setup | Double-click, straight into the game |
| Controls | Controller, or keys mapped to controller buttons | Mouse and keyboard made for the game, 1:1 mouse camera |
| Camera | The right stick, limited by the game | The mouse, patched into the camera code of the game, turns all the way around |
| On-screen prompts | The buttons of the Wii U GamePad | Your own keys and mouse buttons |
| Menus | Stick and buttons | Mouse: hover, click, drag, wheel |
| Map | Controller pan and zoom | Drag to pan, wheel to zoom, **interactive map companion** with live completion tracking |
| Frame rate | FPS++ graphic pack, set up by hand | Built in, **plus AMD FSR 3 frame generation** |
| Picture | The shaders of the game | The shaders of the game **plus 22 presets of the picture**, depth of field, reflections, filters |
| Settings | Emulator dialogs and graphic packs | In the game's own pause menu (`Esc`) |

The changes of every release are in the [changelog](CHANGELOG.md).

## Features

### AMD FSR 3 frame generation

`Esc` → **Graphics** → **Frame Generation** → **AMD FSR 3**.

A generated frame is shown between every two real ones, so 60 rendered frames become 120 displayed ones. This is AMD's
own frame generation (FidelityFX SDK, FSR 3.1) integrated into Cemu's Vulkan renderer. Every frame the emulator hands
it the game's **depth buffer**, **motion vectors** reconstructed from the game camera, and the image **without the
HUD**, so the interface is drawn on top at full sharpness instead of being interpolated.

- The motion vectors cover the movement of the camera only. Things that move on their own (Link, enemies, grass in
  the wind) have no motion vectors: they are left to FSR's optical flow and can show ghosting that a game with
  per-object motion vectors would not have.
- The frame rate limit counts the frames you see (limit 120 = 60 real + 60 generated).
- Not tied to a GPU vendor: it runs on the Vulkan renderer.
- Game logic and physics never run on generated frames.
- Like all frame generation it adds about one game frame of input latency. It feels best from 60 real frames upwards.

### Mouse and keyboard, made for this game

- **1:1 mouse camera.** Raw input, no acceleration, no smoothing. The mouse does not pretend to be an analog stick: the
  game's camera code is patched to take the rotation directly, in the third person camera, with a target locked on, on
  horseback, while aiming the bow, in the scope, in the camera rune, while climbing, in conversations, while shield
  surfing, behind a sand seal and swimming up a waterfall.
- **The camera turns all the way around.** While Link stands still the camera keeps the direction the mouse gave it: it
  can be spun around him any number of times, it does not swing back on its own. While he runs, the camera swings back
  behind him as usual. `Esc` → **Camera** → **Aim Limit** puts a hard limit on the turn (the camera stops at the limit
  instead of spinning around); by default there is no limit.
- **Every action rebindable**, two keys per action, with conflict highlighting.
- **Weapons on the mouse wheel**, arrows while aiming. Hold `1` – `4` for the weapon, bow, shield and rune selectors.
  The mouse aims the camera while a selector is open, the selection is stepped with the wheel. As an option
  (`Esc` → **Camera**), the movement keys step through an open selector and move the cursor in the inventory.
- **The scope and the camera rune are aimed with the mouse**, like the bow: the cursor of the desktop stays out of the
  way there.
- **Toggle modes** for sprint and for lock on / shield, for everyone who cannot or does not want to hold a key down.
- Perfect guard with the attack button while the shield is up (an option: off, the attack key attacks and the perfect
  guard happens when it is pressed at the right moment), flurry rush with mouse clicks, the jump key closes and
  reopens the paraglider.
- **The lock on has priority over the sprint**: pressing it stops the running, and while it is held Link walks.
- **Motion control shrines with the mouse**: `T` tilts the virtual GamePad.
- As an option, remote bombs keep the bow on the right mouse button (Use Rune detonates them).

### Your keys on screen

The game's button prompts show the key or mouse button that is bound to the action, and follow your rebinding. The
glyphs are drawn for this project in the style of the game. The pages of **Ability Controls** (pause menu, System) and
the game's tutorial tips show your keys where the game has pictures of the GamePad.

<p align="center"><img src="docs/prompts.png" width="560" alt="Key and mouse prompt glyphs"></p>

### A mouse pointer in the game's own menus

- **Every menu of the game:** the selection follows the pointer and a click opens the entry, in the title screen, in the
  pause menu, in the inventory, on the map and in the dialogs. The mouse is the game's own while a menu is open, the
  camera does not turn.
- **The PC settings are a menu of the game too:** the pages of the settings (camera, controls, motion, graphics,
  filters, audio, game, cheats) are opened, scrolled and changed with the mouse, like the rest of the game's menus.
- **Inventory:** the selection follows the pointer, click to use an item, click the category icons and the page
  arrows, wheel to change category, drag to rotate Link.
- **Map:** drag to pan, wheel to zoom, click a spot to move the cursor there and confirm (pins, fast travel).
- **Interactive World Map:** an interactive companion built directly into the Sheikah Slate map (`M`). Shows over 20,000 markers for Shrines, Towers, Korok Seeds, Treasure Chests, Hinox, Stone Talus, Molduga, Lynels, and more. Filter categories, search for specific items, auto-hide collected items from your live save, and press `Enter` to center the map on any marker. Toggle on/off anytime with `H` or in `Esc` → **Game**.
- **Conversations:** click the answer.
- **Adventure Log, title screen, save / load, system menus:** click what you want.

### Plays like a PC game

- Starts directly into the game: no emulator window, no game list, no first-run wizard.
- Display mode: windowed, borderless or exclusive fullscreen (`Esc` → **Graphics**, `Alt+Enter` toggles a window),
  cursor captured while playing and released in menus.
- **Settings in the game's own pause menu.** `Esc` opens it on its System page (save, load, options), where the PC
  settings take the place of the game's picture of the GamePad: camera, controls, motion, graphics, filters, audio, game
  and cheats. Mouse or keyboard (`→` to go in, `Esc` to come out). On the title screen `Esc` opens them on their own.
  They are stored in a readable `settings.ini`.
- Resolution from 720p to 4K, frame rate limits from 30 to 244 FPS with corrected game speed and physics, shadows,
  anti-aliasing, V-Sync.
- **The cutscenes have their own frame rate limit** (`Esc` → **Graphics** → **Cutscene FPS**): only the few that need
  it, all of them at 60 FPS, all of them at 30 FPS or none.
- **The filters of the renderer** (`Esc` → **Graphics**): the upscale and the downscale filter (linear, bicubic, bicubic
  hermite, nearest neighbour) and the **draw distance** of the objects and of the terrain.
- **The Vulkan driver** (`Esc` → **Graphics** → **Vulkan Driver**): a machine can have more than one driver for the same
  card (the proprietary driver of AMD and RADV, for example). The loader then offers all of them and the game takes
  the first one; here the driver is picked by hand.
- **The shader filters** (`Esc` → **Filters**): 22 presets of the picture (Clarity), the depth of field, the reflections
  and the anisotropic filtering of the textures.
- **The cheats of the graphic pack** (`Esc` → **Cheats**): infinite hearts, stamina, arrows, amiibo, the divine
  protections, the bow draw speed, the durability and the options of the master mode. The page is only there when it is
  wanted (`Esc` → **Game** → **Enable Cheats**).
- Audio: volume (applied at once), speakers (mono, stereo, 5.1 surround) and the output device.
- **Aspect ratio:** 16:9, 16:10, 21:9 and 32:9, with the HUD at the edges of the screen or centred on ultrawide. An
  ultrawide main display is detected. New, and so far tried in a window only, not on an ultrawide display.
- Inventory, map and Adventure Log open directly on their page (`Tab` / `I`, `M`, `J`).
- **amiibo from files.** When the amiibo rune looks for an amiibo, the game's own window lists your amiibo files
  (`.bin` dumps of your figures, also the decrypted kind amiibo tools write) where it shows a GamePad on a console;
  choose one and it is scanned. Files are added with **Add amiibo File...** in that window or under `Esc` → **Game**.
  The rune itself is turned on in the game's Options.
- **Camera rune with the mouse:** click takes the picture, the wheel zooms, `Esc` (or right click) leaves; "Save in
  album?" is answered with a click.
- Loading screen tips that make no sense on a PC (aiming by tilting the GamePad, amiibo, Wolf Link) are left out.
- **Language and units:** the text language is picked from the ones your copy of the game has, and the temperature
  can be shown in Celsius or Fahrenheit whatever the region of the game is.
- Your existing Cemu save is **copied** on the first start. Cemu itself is never changed.
- The window and the shortcuts carry the game's own icon. It is made on your PC from the icon inside your dump; this
  project ships none of the game's artwork.

## Requirements

- Windows 10 or 11, 64-bit (this fork also builds and runs on Linux, see below).
- A graphics card with Vulkan support. What runs Breath of the Wild well in Cemu runs it well here.
- Your own dump of **The Legend of Zelda: Breath of the Wild** for Wii U, either as folders that contain `code`,
  `content` and `meta` (the format [Dumpling](https://github.com/emiyl/dumpling) writes) or as a `.wua` archive (the
  format Cemu converts to):
  - the base game: Europe, Americas or Japan
  - the **update 1.5.0** (required)
  - the DLC (optional)

**Experimental:** the base game can also be a disc image (`.wud`, `.wux`). This is new and has seen little testing;
if it does not work for you, please open an issue, and use folders or a `.wua` in the meantime. Disc images are
encrypted: the key file of the disc (same name as the image, extension `.key`, 16 bytes) has to be next to it, or its
key in `data\keys.txt`; a `keys.txt` of a Cemu installation is read as well. Update and DLC are not part of a disc
image: they come as folders or a `.wua`.

## Install

1. Download `ZeldaOnPC-Setup-x.y.z.exe` from the [Releases](../../releases) page, or build it yourself (below).
2. Run it. It asks where to install and **where your game dump is**. Select the folder that holds the base game, the
   update and the DLC (or your `.wua` file), or the base game folder itself; the update and DLC are found next to it
   or in an existing Cemu installation.
3. Start **Zelda on PC** from the Start menu, or from the desktop shortcut if you kept it ticked.

The installer needs no administrator rights and copies nothing from your game folder. Saves and settings live next to
the program (`data` and `settings.ini` in the installation folder); the uninstaller asks before it removes them.

The first minutes after the first start stutter while the emulator compiles the game's shaders. They are cached.

### Install on Linux

This fork builds and runs on Linux as well (the GitHub Action `Build Linux release` makes the package
`zelda-on-pc-linux-<version>-linux-x64.tar.gz`).

1. Download the archive from the [Releases](../../releases) page (or from the artifacts of the Action) and unpack it.
2. Install it: `./install.sh --user` puts everything into your home directory (`~/.local/zelda-on-pc`,
   `~/.local/bin/zelda-on-pc`, `~/.local/share/applications`) and needs no root rights. `./install.sh` (with `sudo`)
   installs it for the whole system (`/opt/zelda-on-pc`).
3. Put the folder of your copy of the game into `~/.local/share/zelda-on-pc/data/settings.ini`:

   ```ini
   [Game]
   Path=/home/you/Games/Breath of the Wild
   ```

   The saves, the settings and the caches live in `~/.local/share/zelda-on-pc/data`.

The keyboard and the mouse are read directly from the devices of the machine (the same way Windows Raw Input does):
the movement of the mouse is the movement of the device, without the acceleration of the desktop. That needs access to
the input devices, which the installation with `sudo` gives you (the rule of udev). Without it the events of the window
are used instead: everything works, but the movement of the mouse is the movement of the pointer of the desktop.

To give the access by hand:

```bash
sudo cp 69-zelda-on-pc-input.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
sudo udevadm trigger --action=add --subsystem-match=input
```

The name of the rule file matters: it has to be read before `71-seat.rules`, which tags the devices of the seat. A rule
that runs later adds the tag `uaccess` too late, and systemd-logind then does not hand the devices to the session.

## Controls

All of these are defaults. Change them in `Esc` → **Controls**.

| Action | Key | | Action | Key |
|---|---|---|---|---|
| Move | `W` `A` `S` `D` | | Inventory | `Tab` / `I` |
| Camera | Mouse | | Map | `M` |
| Jump / paraglider | `Space` | | Adventure Log | `J` |
| Sprint / cancel | `Shift` | | Pause menu, PC settings | `Esc` / `F1` |
| Walk | `Ctrl` | | Weapon / bow / shield / rune selector | hold `1` `2` `3` `4` |
| Crouch | `C` | | Next / previous weapon | Mouse wheel |
| Attack | Left mouse | | Menu navigation | Arrow keys, `Enter`, `Backspace` |
| Draw bow | Right mouse | | Motion control (tilt) | `T` |
| Lock on / shield | `Q` / Mouse 4 | | Scope | Middle mouse / `Z` |
| Action / interact | `E` | | Whistle | `X` |
| Use rune | `F` | | Throw weapon | `R` |
| Inventory: previous / next page | `Page Up` / `Page Down`, or `A` / `D` | | Interactive map toggle | `H` (on map) |

Controllers are not handled by this build. For controller play, the game runs just as well in regular Cemu.

## Building from source

You need Windows, [Visual Studio 2022](https://visualstudio.microsoft.com/) or its Build Tools with the *Desktop
development with C++* workload (it brings CMake and Ninja), and Git. For the installer, also
[Inno Setup 6.3+](https://jrsoftware.org/isinfo.php).

```bat
git clone --recursive https://github.com/mircea32000/Zelda-on-PC.git
cd Zelda-on-PC
tools\build.cmd
powershell -ExecutionPolicy Bypass -File tools\package.ps1
```

- `tools\build.cmd` configures and builds `bin\ZeldaOnPC_release.exe`. The first run builds the dependencies through
  vcpkg and takes a while.
- `tools\package.ps1` fetches the two community graphic packs the graphics settings build on, stages the files into
  `build\stage` and compiles `installer\output\ZeldaOnPC-Setup-<version>.exe`. `build\stage` is also a complete portable
  copy.

To run straight from the build folder, tell it where the game is (a folder or a `.wua` file) in `bin\settings.ini`:

```ini
[Game]
Path=D:\Games\Breath of the Wild
```

### Building on Linux

The same tree builds on Linux with CMake, Ninja and vcpkg (the dependencies are the ones of the project: wxWidgets,
GTK3, Vulkan, ...).

```bash
git clone --recursive https://github.com/naufal-backup/zelda-on-pc-linux.git
cd zelda-on-pc-linux
tools/build-linux.sh          # builds bin/ZeldaOnPC_release
tools/package-linux.sh        # stages the files and makes build/zelda-on-pc-linux-<version>-linux-x64.tar.gz
```

The GitHub Action `Build Linux release` (`.github/workflows/build-linux.yml`) does the same on every push and puts the
package into the artifacts of the run (on a tag it also publishes a release).

More in [docs/DEVELOPMENT.md](docs/DEVELOPMENT.md): how the code is laid out, how the game is patched, the developer
mode and its tools. Cemu's own build notes are in [BUILD.md](BUILD.md).

## How it works

- **A Cemu fork in kiosk mode.** `src/pcedition` is the PC layer; the rest of the tree is Cemu with a few hooks. The
  emulator's interface is still there for debugging: start with `--cemu-ui`.
- **Patches generated at startup.** The game executable (update 1.5.0) is patched in memory through Cemu's graphic
  pack mechanism: camera hooks, menu hooks, prompt hooks. Nothing on disk is modified.
- **The game's interface is read live.** The layouts the game draws are walked every frame, which is how the pointer
  knows what is clickable and how the prompts know which keys to show.
- **Input goes straight to the game.** Raw Input feeds the emulated GamePad directly; Cemu's controller profiles are
  bypassed.
- **Community graphic packs** provide resolution, shadows and the FPS++ frame rate unlock; the settings menu drives
  them.

## Questions

**Is this the game running natively?** No. It is the Wii U game running in Cemu, with a PC layer built around it.

**Does it work with other versions of the game?** Only with update 1.5.0, the last one. The patches are written for
that executable. All three regions use it.

**Can I use my Cemu save?** Yes, it is copied automatically on the first start. Afterwards the two are independent.

**Where is the Cemu window?** Hidden. `ZeldaOnPC.exe --cemu-ui` starts the plain emulator interface.

## Credits and licence

- [Cemu](https://github.com/cemu-project/Cemu) by Team Cemu and its contributors. This project is a fork of it and
  would not exist without it.
- [AMD FidelityFX SDK](https://github.com/GPUOpen-LibrariesAndSDKs/FidelityFX-SDK) for FSR 3 frame generation.
- The [Cemu graphic pack community](https://github.com/cemu-project/cemu_graphic_packs) for the *Graphics* and *FPS++*
  packs.

Licensed under the [Mozilla Public License 2.0](LICENSE.txt), like Cemu. Third-party notices are in
[installer/THIRD-PARTY-NOTICES.txt](installer/THIRD-PARTY-NOTICES.txt).

Zelda on PC is an unofficial fan project. It is not affiliated with, endorsed or sponsored by Nintendo, the Cemu
project or AMD. *The Legend of Zelda* and *Breath of the Wild* are trademarks of Nintendo. Dump your own games.
