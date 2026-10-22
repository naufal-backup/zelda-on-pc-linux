#!/usr/bin/env bash
# Mengubah Clarity.fx (efek ReShade) menjadi GLSL yang bisa dipakai renderer Vulkan Zelda on PC.
#
# Proses ini berat (membangun compiler efek ReShade), jadi hanya dijalankan di CI. Keluarannya
# (Clarity.fx.glsl) di-commit, sehingga build biasa tidak perlu menjalankan skrip ini.
#
# Pakai: tools/build_clarity_shader.sh [folder-kerja]
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
root="$(cd "$here/.." && pwd)"
work="${1:-$root/build/reshade}"
fx_src="$root/src/pcedition/reshade/Clarity.fx"
out="$root/src/pcedition/reshade/Clarity.fx.glsl"

reshade_version="v6.8.0"
reshade_url="https://github.com/crosire/reshade.git"
shaders_url="https://raw.githubusercontent.com/crosire/reshade-shaders/master/Shaders/ReShade.fxh"

mkdir -p "$work"

# ---- sumber ReShade (compiler efek: bagian DirectX dan Windows tidak dibangun) ----
if [ ! -d "$work/reshade/.git" ]; then
  echo "Mengambil ReShade $reshade_version"
  git clone --depth 1 --branch "$reshade_version" "$reshade_url" "$work/reshade"
fi
cd "$work/reshade"
git submodule update --init --depth 1 \
  deps/stb deps/utfcpp deps/glad deps/khronos deps/fpng deps/imgui deps/jxl_simple_lossless deps/spirv

# version.h biasanya dibuat oleh sebuah skrip PowerShell
if [ ! -f res/version.h ]; then
  printf '#pragma once\n#define VERSION_FULL 6.8.0.0\n#define VERSION_MAJOR 6\n#define VERSION_MINOR 8\n#define VERSION_REVISION 0\n#define VERSION_BUILD 0\n#define VERSION_STRING_FILE "6.8.0.0"\n#define VERSION_STRING_PRODUCT "6.8.0"\n#define VERSION_STRING "6.8.0"\n' > res/version.h
fi

# glad di-generate dari spesifikasi Khronos
if [ ! -f deps/glad/target/src/gl.c ]; then
  python3 -m pip install --quiet --break-system-packages glad jinja2 2>/dev/null \
    || python3 -m pip install --quiet glad jinja2
  ( cd deps/glad && mkdir -p target && cp ../khronos/*.xml ./ \
    && python3 -m glad --out-path ./target --api gl:compatibility=4.6,vulkan=1.4 \
         --extensions GL_EXT_texture,GL_EXT_texture_storage,VK_EXT_debug_utils,VK_KHR_surface,VK_KHR_swapchain c --mx )
fi

# ---- compiler efek (ReShadeFX + tools/fxc.cpp) ----
cmake -S . -B build-reshadefx -DCMAKE_BUILD_TYPE=Release >/dev/null
cmake --build build-reshadefx --target ReShadeFX -j"$(nproc)"
g++ -std=c++17 -O2 -I source -I res -I build-reshadefx \
    -o "$work/reshadefxc" tools/fxc.cpp "$root/tools/reshade_dxbc_stub.cpp" build-reshadefx/libReShadeFX.a

# ---- ReShade.fxh (definisi standar efek ReShade) ----
if [ ! -f "$work/ReShade.fxh" ]; then
  curl -fsSL -o "$work/ReShade.fxh" "$shaders_url"
fi

# ---- Clarity.fx -> GLSL ----
echo "Mengubah $fx_src"
"$work/reshadefxc" "$fx_src" -I "$work" -I "$(dirname "$fx_src")" --glsl -Fo "$out"

echo "Selesai: $out ($(wc -l < "$out") baris)"