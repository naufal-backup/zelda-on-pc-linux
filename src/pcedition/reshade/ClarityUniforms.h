// Dibuat oleh tools/make_clarity_uniforms.py - jangan disunting dengan tangan.
// Urutan dan perataan uniform Clarity, mengikuti blok _Globals dari Clarity.fx.glsl (std140).
#pragma once

#include <array>

namespace pce::Reshade
{
	// Ukuran satu blok uniform Clarity (sudah dibulatkan ke 16 byte).
	inline constexpr int kClarityUniformSize = 368;

	// Satu uniform: namanya seperti di Clarity.fx, posisinya di dalam blok.
	struct ClarityUniformSlot
	{
		const char* name;
		int offset;   // byte di dalam blok
		int count;    // jumlah komponen float (1, 2, 3 atau 4)
		bool isInt;   // int (skalar) atau float
	};

	inline constexpr ClarityUniformSlot kClarityUniforms[] = {
		{"adjust_bloom", 0, 1, true},
		{"bloomFactor", 4, 1, false},
		{"radius1", 8, 1, false},
		{"radius2", 12, 1, false},
		{"HDRpassing", 16, 1, true},
		{"HDRPower", 20, 1, false},
		{"lumapassing", 24, 1, true},
		{"sharp_strength", 28, 1, false},
		{"sharp_clamp", 32, 1, false},
		{"offset_bias", 36, 1, false},
		{"Tone_map", 40, 1, true},
		{"Exposure", 44, 1, false},
		{"Gamma", 48, 1, false},
		{"Bleach", 52, 1, false},
		{"sat", 56, 1, false},
		{"defog", 60, 1, false},
		{"FogColor", 64, 3, false},
		{"blacknwhitepass", 76, 1, true},
		{"BlackPoint", 80, 1, true},
		{"WhitePoint", 84, 1, true},
		{"cmatrix", 88, 1, true},
		{"CM_Strength", 92, 1, false},
		{"ColorMatrix_Red", 96, 3, false},
		{"ColorMatrix_Green", 112, 3, false},
		{"ColorMatrix_Blue", 128, 3, false},
		{"Tech", 140, 1, true},
		{"Strength", 144, 1, false},
		{"Power", 148, 1, false},
		{"RGBNegativeAmount", 160, 3, false},
		{"Techine", 172, 1, true},
		{"Technicolor2_Red_Strength", 176, 1, false},
		{"Technicolor2_Green_Strength", 180, 1, false},
		{"Technicolor2_Blue_Strength", 184, 1, false},
		{"Technicolor2_Brightness", 188, 1, false},
		{"Technicolor2_Saturation", 192, 1, false},
		{"Technicolor2_Strength", 196, 1, false},
		{"post_process", 200, 1, true},
		{"Vibrance", 204, 1, false},
		{"VibranceRGBBalance", 208, 3, false},
		{"satFactor", 220, 1, false},
		{"crushContrast", 224, 1, false},
		{"dpxpass", 228, 1, true},
		{"DPX_Contrast", 232, 1, false},
		{"DPX_Saturation", 236, 1, false},
		{"DPX_Strength", 240, 1, false},
		{"RGB_Curve", 244, 1, false},
		{"RGB_C", 248, 1, false},
		{"Colorfulness", 252, 1, false},
		{"Filmicpass", 256, 1, true},
		{"Filmic_Strength", 260, 1, false},
		{"Filmic_Contrast", 264, 1, false},
		{"Filmic_Bleach", 268, 1, false},
		{"Linearization", 272, 1, false},
		{"Contrast", 276, 1, false},
		{"BaseGamma", 280, 1, false},
		{"EffectGamma", 284, 1, false},
		{"Fade", 288, 1, false},
		{"Saturation", 292, 1, false},
		{"BaseCurve", 296, 1, false},
		{"lggpass", 300, 1, true},
		{"RGB_Lift", 304, 3, false},
		{"RGB_Gamma", 320, 3, false},
		{"RGB_Gain", 336, 3, false},
		{"CurvesPss", 348, 1, true},
		{"vibpass", 352, 1, true},
	};
	inline constexpr int kClarityUniformCount = 65;

	// Kedudukan uniform di dalam blok, untuk pemeriksaan saat build.
	inline int ClarityUniformIndex(const char* name)
	{
		for (int i = 0; i < kClarityUniformCount; i++)
			if (strcmp(kClarityUniforms[i].name, name) == 0)
				return i;
		return -1;
	}
}
