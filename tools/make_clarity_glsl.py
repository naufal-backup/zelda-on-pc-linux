#!/usr/bin/env python3
"""Menyesuaikan GLSL keluaran ReShade agar bisa dipakai sebagai shader keluaran Zelda on PC.

ReShade menghasilkan program lengkap (vertex + fragment) dengan nama dan binding sendiri. Renderer
Zelda on PC memakai satu shader keluaran dengan aturan tetap:
  - binding 0 = sampler2D textureSrc (gambar game)
  - binding 1 = blok uniform "parameters" (std140) berisi resolusi/gamma, dan uniform Clarity
  - masukan  = vec2 passUV
  - keluaran = vec4 colorOut0 (RGBA)
  - fungsi   = void outputShader()

Karena binding 1 sudah dipakai blok "parameters" milik shader keluaran, uniform Clarity tidak bisa
membuat blok sendiri: ia disisipkan sebagai anggota blok yang sama. Karena itu skrip ini menghasilkan
dua bagian: kClarityUniformFields (anggota blok) dan kClarityFragmentSource (badan shader).
"""
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, "..", "src", "pcedition", "reshade", "Clarity.fx.glsl")
OUT_H = os.path.join(HERE, "..", "src", "pcedition", "reshade", "Clarity.fx.glsl.h")


def uniform_fields(glsl):
    """(tipe, nama) setiap uniform Clarity, sesuai urutan di GLSL keluaran ReShade."""
    block = re.search(r"uniform _Globals \{(.*?)\n\};", glsl, re.S)
    if not block:
        return []
    fields = []
    for line in block.group(1).splitlines():
        m = re.match(r"\s*(int|float|vec[234])\s+V_x(\w+);", line)
        if m:
            fields.append((m.group(1), m.group(2)))
    return fields


def main():
    original = open(SRC, encoding="utf-8").read()

    fields = uniform_fields(original)
    if not fields:
        print("MASALAH: tidak ada uniform di blok _Globals", file=sys.stderr)
        return 1

    text = original

    # bagian vertex dibuang: Zelda on PC memakai vertex shader sendiri
    text = re.sub(r"#ifdef E_xPostProcessVS.*?#endif\n", "", text, flags=re.S)

    # sampler gambar game: preamble shader keluaran sudah mendeklarasikan textureSrc, jadi
    # deklarasi ReShade dibuang dan namanya cukup dipakai langsung
    text = text.replace("layout(binding = 0) uniform sampler2D V_xReShade_xBackBuffer;", "")
    text = text.replace("V_xReShade_xBackBuffer", "textureSrc")

    # depth buffer: Clarity tidak memakainya
    text = re.sub(r"layout\(binding = 1\) uniform sampler2D V_xReShade_xDepthBuffer;", "", text)
    text = re.sub(r"(float|vec3) F_xReShade_xGetLinearizedDepth\(.*?\n\}\n", "", text, flags=re.S)
    text = text.replace("V_xReShade_xDepthBuffer", "textureSrc")

    # blok uniform ReShade dibuang; anggotanya disisipkan ke blok "parameters"
    text = re.sub(r"layout\(std140, column_major, binding = 0\) uniform _Globals \{.*?\n\};\n", "", text, flags=re.S)

    # nama uniform: V_xFoo -> clarity_Foo
    text = re.sub(r"\bV_x(\w+)", r"clarity_\1", text)

    # blok main() ReShade diganti dengan outputShader() milik Zelda on PC
    main_pattern = re.compile(
        r"#ifdef E_xPS_Clarity\s*\n"
        r".*?void main\(\)\s*\n\{\s*\n"
        r".*?= F_xPS_Clarity\(pos, uv\);\s*\n"
        r"\s*return;\s*\n\}\s*\n#endif[^\n]*\n",
        re.S)
    replacement = ("void outputShader()\n"
                   "{\n"
                   "\tcolorOut0 = vec4(F_xPS_Clarity(vec4(gl_FragCoord), passUV), 1.0);\n"
                   "}\n")
    text, count = main_pattern.subn(replacement, text)
    if count != 1:
        print(f"MASALAH: blok main() fragment tidak tergantikan (ditemukan {count})", file=sys.stderr)
        return 1

    # masukan dan keluaran: preamble yang mendeklarasikannya, jadi deklarasi di badan dibuang
    text = text.replace("layout(location = 0) in vec2 _in_uv;", "")
    text = text.replace("layout(location = 0) out vec3 _return;", "")
    text = text.replace("_in_uv", "passUV")

    # ukuran piksel: ReShade menyediakannya, di sini dihitung dari resolusi keluaran
    text = re.sub(r"const (vec2|float) clarity_PixelSize = .*?;", "", text)

    # #version disediakan oleh preamble shader keluaran
    text = re.sub(r"#version \d+\n", "", text, count=1)

    field_lines = "\n".join(f"\tuniform {t} clarity_{n};" for t, n in fields)

    out = []
    out.append("// Dibuat oleh tools/make_clarity_glsl.py dari Clarity.fx.glsl - jangan disunting dengan tangan.")
    out.append("// Uniform Clarity dan badan shader untuk shader keluaran Zelda on PC.")
    out.append("#pragma once")
    out.append("")
    out.append("namespace pce::Reshade")
    out.append("{")
    out.append("\t// Anggota tambahan blok uniform \"parameters\" (binding 1), setelah milik shader keluaran.")
    out.append("\tinline constexpr const char* kClarityUniformFields = R\"GLSL(")
    out.append(field_lines)
    out.append(")GLSL\";")
    out.append("")
    out.append("\t// Badan shader: dipakai bersama PrependFragmentPreamble.")
    out.append("\tinline constexpr const char* kClarityFragmentBody = R\"GLSL(")
    out.append(text)
    out.append(")GLSL\";")
    out.append("}")
    out.append("")

    open(OUT_H, "w", encoding="utf-8").write("\n".join(out))

    problems = []
    # teksturSrc, passUV dan colorOut0 datang dari preamble shader keluaran, jadi tidak diperiksa di badan
    for needle, what in [("void outputShader()", "fungsi outputShader"),
                         ("textureSrc", "pemakaian textureSrc"),
                         ("colorOut0", "keluaran colorOut0"),
                         ("passUV", "masukan passUV")]:
        if needle not in text:
            problems.append(f"tidak ada {what}")
    if "V_xReShade" in text:
        problems.append("masih ada nama ReShade")
    if "_Globals" in text:
        problems.append("masih ada blok _Globals")
    if "#version" in text:
        problems.append("masih ada #version")

    print(f"{len(fields)} uniform, {len(text.splitlines())} baris GLSL -> {OUT_H}")
    for p in problems:
        print("MASALAH:", p, file=sys.stderr)
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())