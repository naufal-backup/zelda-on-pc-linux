#!/usr/bin/env python3
"""Membuat tabel uniform Clarity sesuai tata letak std140 dari Clarity.fx.glsl.

ReShade mengubah Clarity.fx menjadi GLSL dengan urutan uniform yang pasti (lihat blok
"uniform _Globals"). Uniform buffer di sisi C++ harus mengikuti urutan dan perataan std140 itu
(vec3 selalu menempati 16 byte), kalau tidak nilainya salah.

Skrip ini membaca blok uniform dari Clarity.fx.glsl dan menulis ClarityUniforms.h.
"""
import os
import re

HERE = os.path.dirname(os.path.abspath(__file__))
GLSL = os.path.join(HERE, "..", "src", "pcedition", "reshade", "Clarity.fx.glsl")
OUT = os.path.join(HERE, "..", "src", "pcedition", "reshade", "ClarityUniforms.h")

TYPE_SIZE = {"int": 4, "float": 4, "vec2": 8, "vec3": 12, "vec4": 16}
TYPE_ALIGN = {"int": 4, "float": 4, "vec2": 8, "vec3": 16, "vec4": 16}
TYPE_CPP = {"int": "int32", "float": "float", "vec2": "Vector2f", "vec3": "Vector3f", "vec4": "Vector4f"}


def main():
    text = open(GLSL, encoding="utf-8").read()
    block = re.search(r"uniform _Globals \{(.*?)\n\};", text, re.S)
    if not block:
        raise SystemExit("blok uniform _Globals tidak ditemukan di " + GLSL)

    fields = []
    for line in block.group(1).splitlines():
        m = re.match(r"\s*(int|float|vec[234])\s+V_x(\w+);", line)
        if m:
            fields.append((m.group(1), m.group(2)))

    # tata letak std140
    offset = 0
    layout = []
    for glsl_type, name in fields:
        align = TYPE_ALIGN[glsl_type]
        size = TYPE_SIZE[glsl_type]
        offset = (offset + align - 1) // align * align
        layout.append((glsl_type, name, offset))
        offset += size
    total = (offset + 15) // 16 * 16

    out = []
    out.append("// Dibuat oleh tools/make_clarity_uniforms.py - jangan disunting dengan tangan.")
    out.append("// Urutan dan perataan uniform Clarity, mengikuti blok _Globals dari Clarity.fx.glsl (std140).")
    out.append("#pragma once")
    out.append("")
    out.append("#include <array>")
    out.append("")
    out.append("namespace pce::Reshade")
    out.append("{")
    out.append("\t// Ukuran satu blok uniform Clarity (sudah dibulatkan ke 16 byte).")
    out.append(f"\tinline constexpr int kClarityUniformSize = {total};")
    out.append("")
    out.append("\t// Satu uniform: namanya seperti di Clarity.fx, posisinya di dalam blok.")
    out.append("\tstruct ClarityUniformSlot")
    out.append("\t{")
    out.append("\t\tconst char* name;")
    out.append("\t\tint offset;   // byte di dalam blok")
    out.append("\t\tint count;    // jumlah komponen float (1, 2, 3 atau 4)")
    out.append("\t\tbool isInt;   // int (skalar) atau float")
    out.append("\t};")
    out.append("")
    out.append("\tinline constexpr ClarityUniformSlot kClarityUniforms[] = {")
    for glsl_type, name, off in layout:
        count = 1 if glsl_type in ("int", "float") else int(glsl_type[-1])
        is_int = "true" if glsl_type == "int" else "false"
        out.append(f'\t\t{{"{name}", {off}, {count}, {is_int}}},')
    out.append("\t};")
    out.append(f"\tinline constexpr int kClarityUniformCount = {len(layout)};")
    out.append("")
    out.append("\t// Kedudukan uniform di dalam blok, untuk pemeriksaan saat build.")
    out.append("\tinline int ClarityUniformIndex(const char* name)")
    out.append("\t{")
    out.append("\t\tfor (int i = 0; i < kClarityUniformCount; i++)")
    out.append("\t\t\tif (strcmp(kClarityUniforms[i].name, name) == 0)")
    out.append("\t\t\t\treturn i;")
    out.append("\t\treturn -1;")
    out.append("\t}")
    out.append("}")
    out.append("")

    open(OUT, "w", encoding="utf-8").write("\n".join(out))
    print(f"{len(layout)} uniform, blok {total} byte -> {OUT}")
    for glsl_type, name, off in layout:
        print(f"  +{off:3d}  {glsl_type:5s} {name}")


if __name__ == "__main__":
    main()