#!/usr/bin/env python3
"""Membuat preset Clarity untuk Zelda on PC dari parameter shader graphic pack.

Sumber: src/pcedition/reshade/clarity_presets.json (hasil ekstraksi
37040a485a29d54e_00000000000003c9_ps.txt, shader Clarity milik graphic pack "Enhancements").

Keluaran: src/pcedition/reshade/ClarityPresets.h - tabel C++ berisi 20 preset + Disabled,
siap dipakai oleh menu Filters. Nama uniform di sini harus sama dengan yang ada di Clarity.fx.
"""
import json
import os

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, "..", "src", "pcedition", "reshade", "clarity_presets.json")
OUT = os.path.join(HERE, "..", "src", "pcedition", "reshade", "ClarityPresets.h")

# nama preset seperti di menu (nomor shader -> nama pack)
NAMES = {
    0: "User-Defined Preset",
    1: "Bruz's Preset",
    2: "t3nka Neutral+",
    3: "Jamie's Preset",
    4: "Contrasty Preset",
    5: "The Complaining Gamer's Preset",
    6: "Filmic Preset",
    7: "CellHunter's Preset",
    8: "Serfrost's Preset (Bright Display)",
    9: "Serfrost's Preset (Dim Display)",
    10: "Serfrost's Preset (Recommended)",
    11: "ViskClarity Preset",
    12: "Survival of the Wild Preset (Warmer)",
    13: "Survival of the Wild Preset (Colder)",
    14: "Godot's Mexican Preset",
    15: "Hexaae's Clear Preset",
    16: "MaranBr Preset (Little More Intense)",
    17: "YeMaoWuXin's Preset (Bright colors)",
    18: "YeMaoWuXin's Preset (Vivid and clear)",
    19: "Cobalt's Preset (Saturated & Bright)",
}

# nama uniform ReShade -> nama di shader asli
UNIFORM = {
    "adjust_bloom": "adjust_bloom",
    "bloomFactor": "bloomFactor",
    "radius1": "radius1",
    "radius2": "radius2",
    "HDRpassing": "HDRpassing",
    "HDRPower": "HDRPower",
    "lumapassing": "lumapassing",
    "sharp_strength": "sharp_strength",
    "sharp_clamp": "sharp_clamp",
    "offset_bias": "offset_bias",
    "Tone_map": "Tone_map",
    "Exposure": "Exposure",
    "Gamma": "Gamma",
    "Bleach": "Bleach",
    "sat": "sat",
    "defog": "defog",
    "blacknwhitepass": "blacknwhitepass",
    "cmatrix": "cmatrix",
    "Tech": "Tech",
    "Strength": "Strength",
    "Power": "Power",
    "Techine": "Techine",
    "Technicolor2_Red_Strength": "Technicolor2_Red_Strength",
    "Technicolor2_Green_Strength": "Technicolor2_Green_Strength",
    "Technicolor2_Blue_Strength": "Technicolor2_Blue_Strength",
    "Technicolor2_Brightness": "Technicolor2_Brightness",
    "Technicolor2_Saturation": "Technicolor2_Saturation",
    "Technicolor2_Strength": "Technicolor2_Strength",
    "post_process": "post_process",
    "Vibrance": "Vibrance",
    "satFactor": "satFactor",
    "crushContrast": "crushContrast",
    "dpxpass": "dpxpass",
    "Filmicpass": "Filmicpass",
    "Filmic_Strength": "Filmic_Strength",
    "Filmic_Contrast": "Filmic_Contrast",
    "Filmic_Bleach": "Filmic_Bleach",
    "Linearization": "Linearization",
    "Contrast": "Contrast",
    "BaseGamma": "BaseGamma",
    "EffectGamma": "EffectGamma",
    "Fade": "Fade",
    "Saturation": "Saturation",
    "BaseCurve": "BaseCurve",
    "lggpass": "lggpass",
    "CurvesPss": "CurvesPss",
    "vibpass": "vibpass",
}

INT_KEYS = {"adjust_bloom", "HDRpassing", "lumapassing", "Tone_map", "blacknwhitepass",
            "cmatrix", "Tech", "Techine", "post_process", "dpxpass", "Filmicpass",
            "lggpass", "CurvesPss", "vibpass"}

# nilai default saat shader asli tidak menyetelnya (dari blok [Default] pack)
DEFAULTS = {
    "Saturation": 0.5,
    "Strength": 0.20,
    "Power": 4.0,
    "VibranceRGBBalance": (1.0, 1.0, 1.0),
}


def main():
    data = json.load(open(SRC, encoding="utf-8"))
    out = []
    out.append("// Dibuat oleh tools/make_clarity_presets.py - jangan disunting dengan tangan.")
    out.append("// Parameter preset Clarity, hasil ekstraksi shader graphic pack \"Enhancements\".")
    out.append("#pragma once")
    out.append("")
    out.append("namespace pce::Reshade")
    out.append("{")
    out.append("\t// Satu nilai uniform dari sebuah preset.")
    out.append("\tstruct ClarityValue")
    out.append("\t{")
    out.append("\t\tconst char* name;")
    out.append("\t\tbool isInt;")
    out.append("\t\tfloat value;")
    out.append("\t};")
    out.append("")
    out.append("\tstruct ClarityPreset")
    out.append("\t{")
    out.append("\t\tconst char* name;")
    out.append("\t\tconst ClarityValue* values;")
    out.append("\t\tint count;")
    out.append("\t};")
    out.append("")

    presets = []
    for num in sorted(int(k) for k in data):
        block = data[str(num)]
        vals = []
        for key, value in sorted(block["floats"].items()):
            if key not in UNIFORM:
                continue
            vals.append((UNIFORM[key], key in INT_KEYS, float(value)))
        for key, value in sorted(block["defines"].items()):
            if key not in UNIFORM:
                continue
            vals.append((UNIFORM[key], True, float(value)))
        # lengkapi nilai yang tidak disetel shader dengan default pack
        have = {v[0] for v in vals}
        for key, value in DEFAULTS.items():
            if key in have or key not in UNIFORM or isinstance(value, tuple):
                continue
            vals.append((UNIFORM[key], key in INT_KEYS, float(value)))
        vals.sort(key=lambda v: v[0])
        name = NAMES.get(num, f"Preset {num}")
        presets.append((name, vals))

    for i, (name, vals) in enumerate(presets):
        out.append(f"\t// {name}")
        out.append(f"\tinline constexpr ClarityValue kClarityValues{i}[] = {{")
        for uname, is_int, value in vals:
            if is_int:
                v = f"{int(value)}"
            else:
                # C++ needs a digit on both sides of the point: 1.0f, not 1f
                t = f"{value:g}"
                if "." not in t and "e" not in t and "E" not in t:
                    t += ".0"
                v = t + "f"
            out.append(f'\t\t{{"{uname}", {"true" if is_int else "false"}, {v}}},')
        out.append("\t};")
        out.append("")

    out.append("\tinline constexpr ClarityPreset kClarityPresets[] = {")
    for i, (name, _) in enumerate(presets):
        out.append(f'\t\t{{"{name}", kClarityValues{i}, (int)std::size(kClarityValues{i})}},')
    out.append("\t};")
    out.append("\tinline constexpr int kClarityPresetCount = (int)std::size(kClarityPresets);")
    out.append("}")
    out.append("")

    open(OUT, "w", encoding="utf-8").write("\n".join(out))
    print(f"{len(presets)} preset ditulis ke {OUT}")
    for name, vals in presets:
        print(f"  {name}: {len(vals)} uniform")


if __name__ == "__main__":
    main()