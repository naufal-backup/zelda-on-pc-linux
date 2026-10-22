// Preset Clarity sebagai shader keluaran (lihat ClarityEffect.h).
//
// Rantai efeknya ada di src/pcedition/reshade/Clarity.fx dan sudah diubah menjadi GLSL oleh
// tools/build_clarity_shader.sh (di CI). Di sini GLSL itu dipakai sebagai shader keluaran, dengan
// parameter preset disisipkan ke blok uniform. Karena itu ganti preset tidak perlu membuat shader
// baru - cukup mengganti isi uniform buffer, jadi bisa berlaku saat game berjalan.

#include "Cafe/HW/Latte/Renderer/Vulkan/ClarityEffect.h"

#include <cstring>
#include <vector>

#include "Cafe/HW/Latte/Renderer/RendererOuputShader.h"
#include "Cemu/Logging/CemuLogging.h"
#include "pcedition/reshade/ClarityUniforms.h"
#include "pcedition/reshade/Clarity.fx.glsl.h"

namespace pce::Reshade
{
	namespace
	{
		// Shader keluaran Clarity: blok uniformnya adalah blok standar (resolusi, gamma) ditambah
		// uniform preset Clarity. Badan shader-nya dari kClarityFragmentBody.
		class ClarityOutputShader : public RendererOutputShader
		{
		public:
			ClarityOutputShader(const std::vector<uint8>& presetValues)
				: RendererOutputShader(RendererOutputShader::GetVulkanVertexSource(false),
									   RendererOutputShader::PrependFragmentPreamble(
										   std::string(kClarityUniformFields) + "\n" + std::string(kClarityFragmentBody))),
				  m_presetValues(presetValues)
			{
			}

			bool HasExtendedUniforms() const override { return true; }

			void BuildExtendedUniforms(const OutputUniformVariables& standard, std::vector<uint8>& out) const override
			{
				// blok standar, lalu uniform Clarity
				out.resize(sizeof(standard) + m_presetValues.size());
				std::memcpy(out.data(), &standard, sizeof(standard));
				if (!m_presetValues.empty())
					std::memcpy(out.data() + sizeof(standard), m_presetValues.data(), m_presetValues.size());
			}

		private:
			std::vector<uint8> m_presetValues;
		};
	}

	ClarityEffect& ClarityEffect::Get()
	{
		static ClarityEffect s_instance;
		return s_instance;
	}

	bool ClarityEffect::IsEnabledPreset(int preset)
	{
		// Preset 0 adalah "Disabled": tidak ada efek, graphic pack yang menangani (kalau dipakai)
		return preset >= 1 && preset <= kClarityPresetCount;
	}

	int ClarityEffect::PresetCount()
	{
		return kClarityPresetCount;
	}

	void ClarityEffect::SetPreset(int preset)
	{
		m_preset.store(preset);
	}

	int ClarityEffect::GetPreset() const
	{
		return m_preset.load();
	}

	const std::vector<uint8>& ClarityEffect::GetUniformData(int preset)
	{
		const int index = preset - 1;
		static const std::vector<uint8> s_empty;
		if (index < 0 || index >= kClarityPresetCount)
			return s_empty;

		Entry& entry = m_entries[index];
		if (!entry.uniformData.empty())
			return entry.uniformData;

		entry.uniformData.assign(kClarityUniformSize, 0);
		const ClarityPreset& p = kClarityPresets[index];
		for (int i = 0; i < p.count; i++)
		{
			const ClarityValue& v = p.values[i];
			const int slot = ClarityUniformIndex(v.name);
			if (slot < 0)
				continue;
			const ClarityUniformSlot& u = kClarityUniforms[slot];
			if (u.isInt)
			{
				const int32 value = (int32)v.value;
				std::memcpy(entry.uniformData.data() + u.offset, &value, sizeof(value));
			}
			else
			{
				// vec2/vec3/vec4: semua komponen diberi nilai yang sama
				float comps[4] = {v.value, v.value, v.value, v.value};
				std::memcpy(entry.uniformData.data() + u.offset, comps, sizeof(float) * u.count);
			}
		}
		return entry.uniformData;
	}

	RendererOutputShader* ClarityEffect::GetShader(int preset)
	{
		const int index = preset - 1;
		if (index < 0 || index >= (int)m_entries.size())
			return nullptr;
		Entry& entry = m_entries[index];
		if (entry.shader)
			return entry.shader.get();
		if (entry.failed)
			return nullptr;
		if (!IsEnabledPreset(preset))
			return nullptr;

		try
		{
			const std::vector<uint8>& values = GetUniformData(preset);
			entry.shader = std::make_unique<ClarityOutputShader>(values);
		}
		catch (const std::exception& e)
		{
			cemuLog_log(LogType::Force, "Clarity: preset {} tidak bisa dipakai: {}", preset, e.what());
			entry.failed = true;
			return nullptr;
		}
		return entry.shader.get();
	}
}