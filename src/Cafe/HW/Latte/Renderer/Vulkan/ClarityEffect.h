#pragma once

// Preset Clarity untuk Zelda on PC.
//
// Preset shader Clarity dari graphic pack "Enhancements" berjalan di dalam pipeline game: presetnya
// tertanam di shader yang dipatch ke pipeline, sehingga ganti preset butuh restart. Di sini rantai efek
// yang sama dijalankan sebagai shader keluaran terpisah dan seluruh parameter preset menjadi uniform.
// Akibatnya ganti preset cukup mengganti nilai uniform - berlaku saat game berjalan.
//
// Shader-nya sendiri ditulis sebagai efek ReShade (src/pcedition/reshade/Clarity.fx) dan diubah menjadi
// GLSL oleh tools/build_clarity_shader.sh di CI.

#include <string>
#include <vector>
#include <array>
#include <cstdint>
#include <memory>
#include <atomic>
#include <cstring>

#include "pcedition/reshade/ClarityPresets.h"

class RendererOutputShader;
class RendererShader;

namespace pce::Reshade
{
	// Jumlah uniform yang dipakai Clarity.fx. Harus sama dengan urutan di shader.
	inline constexpr int kClarityUniformCount = 56;

	class ClarityEffect
	{
	public:
		static ClarityEffect& Get();

		// Apakah preset dengan nomor ini ada (0 = "Disabled": efek tidak dipakai).
		static bool IsEnabledPreset(int preset);
		static int PresetCount();

		// Nomor preset yang sedang dipakai (dibaca render thread, ditulis menu).
		void SetPreset(int preset);
		int GetPreset() const;

		// Shader keluaran untuk preset yang aktif, atau nullptr kalau tidak ada.
		// Dibuat sekali untuk setiap preset, lalu dipakai ulang.
		RendererOutputShader* GetShader(int preset);

		// Uniform buffer untuk preset yang aktif (urut sesuai Clarity.fx).
		const std::vector<uint8>& GetUniformData(int preset);

	private:
		ClarityEffect() = default;

		struct Entry
		{
			std::unique_ptr<RendererOutputShader> shader;
			std::vector<uint8> uniformData;
			bool failed = false;
		};

		Entry& GetEntry(int preset);

		std::array<Entry, 21> m_entries;
		std::atomic<int> m_preset{0};
	};
}