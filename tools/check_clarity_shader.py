#!/usr/bin/env python3
"""Memeriksa bahwa shader Clarity benar-benar bisa dikompilasi seperti yang dilakukan renderer.

Ini menirukan apa yang dilakukan LatteRenderTarget/ClarityEffect saat runtime:
  1. preamble standar shader keluaran (lihat RendererOutputShader::PrependFragmentPreamble)
  2. anggota blok uniform Clarity
  3. badan shader Clarity
lalu mengompilasinya dengan glslangValidator.

Kesalahan seperti "#version bukan yang pertama" atau uniform ganda baru terlihat saat game
berjalan (layarnya menjadi hitam), jadi pemeriksaan ini penting - dijalankan di CI.
"""
import os
import re
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
HEADER = os.path.join(HERE, "..", "src", "pcedition", "reshade", "Clarity.fx.glsl.h")

# Sama dengan RendererOutputShader::PrependFragmentPreamble (jalur Vulkan).
PREAMBLE = """#version 430
layout(location = 0) smooth in vec2 passUV;
layout(binding = 0) uniform sampler2D textureSrc;
layout(location = 0) out vec4 colorOut0;

#ifdef VULKAN
layout (binding = 1, std140)
#else
layout (binding = 0, std140)
#endif
uniform parameters {
uniform vec2 textureSrcResolution;
uniform vec2 nativeResolution;
uniform vec2 outputResolution;
uniform bool applySRGBEncoding;
uniform float targetGamma;
uniform float displayGamma;
"""


def extract(name, text):
    m = re.search(rf'{name} = R"GLSL\(\n(.*?)\n\)GLSL"', text, re.S)
    if not m:
        raise SystemExit(f"{name} tidak ditemukan di {HEADER}")
    return m.group(1)


def main():
    if not shutil.which("glslangValidator"):
        print("glslangValidator tidak ada, pemeriksaan dilewati")
        return 0

    text = open(HEADER, encoding="utf-8").read()
    fields = extract("kClarityUniformFields", text)
    body = extract("kClarityFragmentBody", text)

    # renderer mengompilasi sebagai #version 450 di tahap fragment
    source = PREAMBLE + fields + "\n};\n" + body

    problems = []
    if body.count("#version") > 0:
        problems.append("badan shader masih punya #version (preamble sudah menyediakannya)")
    if "uniform parameters {" in body:
        problems.append("badan shader punya blok parameters sendiri (bentrok dengan preamble)")
    if "uniform sampler2D textureSrc" in body:
        problems.append("badan shader mendeklarasikan textureSrc lagi")

    with tempfile.TemporaryDirectory() as tmp:
        path = os.path.join(tmp, "clarity.frag")
        open(path, "w", encoding="utf-8").write(source)
        result = subprocess.run(["glslangValidator", "-V", "--target-env", "vulkan1.1", "-S", "frag", path],
                                capture_output=True, text=True)
        output = (result.stdout or "") + (result.stderr or "")
        # The renderer names its entry point outputShader (it is compiled as a library shader),
        # so glslang's "missing entry point" note is expected and not a failure.
        real_errors = [line for line in output.splitlines()
                       if "ERROR:" in line and "Missing entry point" not in line]
        if real_errors:
            problems.append("glslang menolak shader:\n" + "\n".join(real_errors))

    if problems:
        for p in problems:
            print("MASALAH:", p, file=sys.stderr)
        return 1
    print(f"Shader Clarity dapat dikompilasi ({len(source.splitlines())} baris)")
    return 0


if __name__ == "__main__":
    sys.exit(main())