// Konversi DXBC tidak dibangun di luar Windows, jadi entry pointnya diganti dengan kosong.
// Dipakai hanya supaya tools/fxc.cpp (compiler efek ReShade) bisa ditautkan di Linux.
// Lihat tools/build_clarity_shader.sh.
#include "effect_codegen.hpp"

namespace reshadefx
{
	codegen* create_codegen_dxbc(unsigned int /*shader_model*/, bool /*debug_info*/,
								 bool /*uniforms_to_spec_constants*/, int /*optimization_level*/)
	{
		return nullptr;
	}
}