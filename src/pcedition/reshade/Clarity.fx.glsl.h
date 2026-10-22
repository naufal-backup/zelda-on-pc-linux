// Dibuat oleh tools/make_clarity_glsl.py dari Clarity.fx.glsl - jangan disunting dengan tangan.
// Uniform Clarity dan badan shader untuk shader keluaran Zelda on PC.
#pragma once

namespace pce::Reshade
{
	// Anggota tambahan blok uniform "parameters" (binding 1), setelah milik shader keluaran.
	inline constexpr const char* kClarityUniformFields = R"GLSL(
	uniform int clarity_adjust_bloom;
	uniform float clarity_bloomFactor;
	uniform float clarity_radius1;
	uniform float clarity_radius2;
	uniform int clarity_HDRpassing;
	uniform float clarity_HDRPower;
	uniform int clarity_lumapassing;
	uniform float clarity_sharp_strength;
	uniform float clarity_sharp_clamp;
	uniform float clarity_offset_bias;
	uniform int clarity_Tone_map;
	uniform float clarity_Exposure;
	uniform float clarity_Gamma;
	uniform float clarity_Bleach;
	uniform float clarity_sat;
	uniform float clarity_defog;
	uniform vec3 clarity_FogColor;
	uniform int clarity_blacknwhitepass;
	uniform int clarity_BlackPoint;
	uniform int clarity_WhitePoint;
	uniform int clarity_cmatrix;
	uniform float clarity_CM_Strength;
	uniform vec3 clarity_ColorMatrix_Red;
	uniform vec3 clarity_ColorMatrix_Green;
	uniform vec3 clarity_ColorMatrix_Blue;
	uniform int clarity_Tech;
	uniform float clarity_Strength;
	uniform float clarity_Power;
	uniform vec3 clarity_RGBNegativeAmount;
	uniform int clarity_Techine;
	uniform float clarity_Technicolor2_Red_Strength;
	uniform float clarity_Technicolor2_Green_Strength;
	uniform float clarity_Technicolor2_Blue_Strength;
	uniform float clarity_Technicolor2_Brightness;
	uniform float clarity_Technicolor2_Saturation;
	uniform float clarity_Technicolor2_Strength;
	uniform int clarity_post_process;
	uniform float clarity_Vibrance;
	uniform vec3 clarity_VibranceRGBBalance;
	uniform float clarity_satFactor;
	uniform float clarity_crushContrast;
	uniform int clarity_dpxpass;
	uniform float clarity_DPX_Contrast;
	uniform float clarity_DPX_Saturation;
	uniform float clarity_DPX_Strength;
	uniform float clarity_RGB_Curve;
	uniform float clarity_RGB_C;
	uniform float clarity_Colorfulness;
	uniform int clarity_Filmicpass;
	uniform float clarity_Filmic_Strength;
	uniform float clarity_Filmic_Contrast;
	uniform float clarity_Filmic_Bleach;
	uniform float clarity_Linearization;
	uniform float clarity_Contrast;
	uniform float clarity_BaseGamma;
	uniform float clarity_EffectGamma;
	uniform float clarity_Fade;
	uniform float clarity_Saturation;
	uniform float clarity_BaseCurve;
	uniform int clarity_lggpass;
	uniform vec3 clarity_RGB_Lift;
	uniform vec3 clarity_RGB_Gamma;
	uniform vec3 clarity_RGB_Gain;
	uniform int clarity_CurvesPss;
	uniform int clarity_vibpass;
)GLSL";

	// Badan shader: dipakai bersama PrependFragmentPreamble.
	inline constexpr const char* kClarityFragmentBody = R"GLSL(
vec2 compCond(bvec2 cond, vec2 a, vec2 b) { return vec2(cond.x ? a.x : b.x, cond.y ? a.y : b.y); }
vec3 compCond(bvec3 cond, vec3 a, vec3 b) { return vec3(cond.x ? a.x : b.x, cond.y ? a.y : b.y, cond.z ? a.z : b.z); }
vec4 compCond(bvec4 cond, vec4 a, vec4 b) { return vec4(cond.x ? a.x : b.x, cond.y ? a.y : b.y, cond.z ? a.z : b.z, cond.w ? a.w : b.w); }
bvec2 compCond(bvec2 cond, bvec2 a, bvec2 b) { return bvec2(cond.x ? a.x : b.x, cond.y ? a.y : b.y); }
bvec3 compCond(bvec3 cond, bvec3 a, bvec3 b) { return bvec3(cond.x ? a.x : b.x, cond.y ? a.y : b.y, cond.z ? a.z : b.z); }
bvec4 compCond(bvec4 cond, bvec4 a, bvec4 b) { return bvec4(cond.x ? a.x : b.x, cond.y ? a.y : b.y, cond.z ? a.z : b.z, cond.w ? a.w : b.w); }
ivec2 compCond(bvec2 cond, ivec2 a, ivec2 b) { return ivec2(cond.x ? a.x : b.x, cond.y ? a.y : b.y); }
ivec3 compCond(bvec3 cond, ivec3 a, ivec3 b) { return ivec3(cond.x ? a.x : b.x, cond.y ? a.y : b.y, cond.z ? a.z : b.z); }
ivec4 compCond(bvec4 cond, ivec4 a, ivec4 b) { return ivec4(cond.x ? a.x : b.x, cond.y ? a.y : b.y, cond.z ? a.z : b.z, cond.w ? a.w : b.w); }
uvec2 compCond(bvec2 cond, uvec2 a, uvec2 b) { return uvec2(cond.x ? a.x : b.x, cond.y ? a.y : b.y); }
uvec3 compCond(bvec3 cond, uvec3 a, uvec3 b) { return uvec3(cond.x ? a.x : b.x, cond.y ? a.y : b.y, cond.z ? a.z : b.z); }
uvec4 compCond(bvec4 cond, uvec4 a, uvec4 b) { return uvec4(cond.x ? a.x : b.x, cond.y ? a.y : b.y, cond.z ? a.z : b.z, cond.w ? a.w : b.w); }


void F_xPostProcessVS(
	in uint id,
	out vec4 position,
	out vec2 texcoord)
{
	bool _43 = id == 2u;
	float _46 = _43 ? 2.00000000e+00 : 0.00000000e+00;
	texcoord[0] = _46;
	bool _48 = id == 1u;
	float _51 = _48 ? 2.00000000e+00 : 0.00000000e+00;
	texcoord[1] = _51;
	vec2 _53 = texcoord * vec2(2.00000000e+00, -2.00000000e+00);
	vec2 _55 = _53 + vec2(-1.00000000e+00, 1.00000000e+00);
	vec4 _60 = vec4(_55.x, _55.y, 0.00000000e+00, 1.00000000e+00);
	position = _60;
	return;
}
float F_xgetL601(
	in vec3 rgb)
{
	float _130 = dot(rgb, vec3(2.98900008e-01, 5.86600006e-01, 1.14500001e-01));
	return _130;
}
float F_xgetL709(
	in vec3 rgb)
{
	float _135 = dot(rgb, vec3(2.12599993e-01, 7.15200007e-01, 7.22000003e-02));
	return _135;
}
vec3 F_xLinearToneMapping(
	in vec3 color)
{
	vec3 _140 = clarity_Exposure.xxx * color;
	vec3 _143 = clamp(_140, vec3(0.00000000e+00, 0.00000000e+00, 0.00000000e+00), vec3(1.00000000e+00, 1.00000000e+00, 1.00000000e+00));
	color = _143;
	float _145 = 1.00000000e+00 / clarity_Gamma;
	vec3 _147 = pow(color, _145.xxx);
	return _147;
}
vec3 F_xSimpleReinhardToneMapping(
	in vec3 color)
{
	vec3 _152 = color / vec3(1.50000000e+00, 1.50000000e+00, 1.50000000e+00);
	vec3 _154 = vec3(1.00000000e+00, 1.00000000e+00, 1.00000000e+00) + _152;
	vec3 _156 = vec3(1.50000000e+00, 1.50000000e+00, 1.50000000e+00) / _154;
	vec3 _157 = color * _156;
	color = _157;
	float _159 = 1.00000000e+00 / clarity_Gamma;
	vec3 _161 = pow(color, _159.xxx);
	return _161;
}
vec3 F_xLumaBasedReinhardToneMapping(
	in vec3 color)
{
	vec3 _165;
	_165 = color;
	float _166 = F_xgetL709(_165);
	float luma = _166;
	float _169 = 1.00000000e+00 + luma;
	float _170 = luma / _169;
	float toneMappedLuma = _170;
	float _172 = toneMappedLuma / luma;
	vec3 _174 = color * _172.xxx;
	color = _174;
	float _176 = 1.00000000e+00 / clarity_Gamma;
	vec3 _178 = pow(color, _176.xxx);
	return _178;
}
vec3 F_xWhitePreservingLumaBasedReinhardToneMapping(
	in vec3 color)
{
	vec3 _182;
	_182 = color;
	float _183 = F_xgetL709(_182);
	float luma_184 = _183;
	float _186 = luma_184 / 4.00000000e+00;
	float _188 = 1.00000000e+00 + _186;
	float _189 = luma_184 * _188;
	float _191 = 1.00000000e+00 + luma_184;
	float _192 = _189 / _191;
	float toneMappedLuma_193 = _192;
	float _194 = toneMappedLuma_193 / luma_184;
	vec3 _196 = color * _194.xxx;
	color = _196;
	float _198 = 1.00000000e+00 / clarity_Gamma;
	vec3 _200 = pow(color, _198.xxx);
	return _200;
}
vec3 F_xRomBinDaHouseToneMapping(
	in vec3 color)
{
	vec3 _205 = vec3(2.72000003e+00, 2.72000003e+00, 2.72000003e+00) * color;
	vec3 _207 = _205 + vec3(1.50000006e-01, 1.50000006e-01, 1.50000006e-01);
	vec3 _209 = vec3(-1.00000000e+00, -1.00000000e+00, -1.00000000e+00) / _207;
	vec3 _210 = exp(_209);
	color = _210;
	float _212 = 1.00000000e+00 / clarity_Gamma;
	vec3 _214 = pow(color, _212.xxx);
	return _214;
}
vec3 F_xFilmicToneMapping(
	in vec3 color)
{
	vec3 _219 = color - vec3(4.00000019e-03, 4.00000019e-03, 4.00000019e-03);
	vec3 _221 = max(vec3(0.00000000e+00, 0.00000000e+00, 0.00000000e+00), _219);
	color = _221;
	vec3 _223 = vec3(6.19999981e+00, 6.19999981e+00, 6.19999981e+00) * color;
	vec3 _225 = _223 + vec3(5.00000000e-01, 5.00000000e-01, 5.00000000e-01);
	vec3 _226 = color * _225;
	vec3 _228 = vec3(6.19999981e+00, 6.19999981e+00, 6.19999981e+00) * color;
	vec3 _230 = _228 + vec3(1.70000005e+00, 1.70000005e+00, 1.70000005e+00);
	vec3 _231 = color * _230;
	vec3 _233 = _231 + vec3(5.99999987e-02, 5.99999987e-02, 5.99999987e-02);
	vec3 _234 = _226 / _233;
	return _234;
}
vec3 F_xUncharted2ToneMapping(
	in vec3 color)
{
	vec3 _239 = color * vec3(2.00000000e+00, 2.00000000e+00, 2.00000000e+00);
	color = _239;
	vec3 _241 = vec3(1.50000006e-01, 1.50000006e-01, 1.50000006e-01) * color;
	vec3 _243 = _241 + vec3(5.00000007e-02, 5.00000007e-02, 5.00000007e-02);
	vec3 _244 = color * _243;
	vec3 _246 = _244 + vec3(4.00000019e-03, 4.00000019e-03, 4.00000019e-03);
	vec3 _248 = vec3(1.50000006e-01, 1.50000006e-01, 1.50000006e-01) * color;
	vec3 _250 = _248 + vec3(5.00000000e-01, 5.00000000e-01, 5.00000000e-01);
	vec3 _251 = color * _250;
	vec3 _253 = _251 + vec3(6.00000024e-02, 6.00000024e-02, 6.00000024e-02);
	vec3 _254 = _246 / _253;
	vec3 _256 = _254 - vec3(6.66666627e-02, 6.66666627e-02, 6.66666627e-02);
	color = _256;
	vec3 _258 = color / vec3(7.25129366e-01, 7.25129366e-01, 7.25129366e-01);
	color = _258;
	float _260 = 1.00000000e+00 / clarity_Gamma;
	vec3 _262 = pow(color, _260.xxx);
	return _262;
}
vec3 F_xReshadeToneMap(
	in vec3 inputColor)
{
	vec3 color_266 = inputColor;
	vec3 _268 = clarity_defog.xxx * clarity_FogColor;
	vec3 _270 = _268 * vec3(2.54999995e+00, 2.54999995e+00, 2.54999995e+00);
	vec3 _271 = color_266 - _270;
	vec3 _274 = clamp(_271, vec3(0.00000000e+00, 0.00000000e+00, 0.00000000e+00), vec3(1.00000000e+00, 1.00000000e+00, 1.00000000e+00));
	color_266 = _274;
	vec3 _276 = color_266 / clarity_Exposure.xxx;
	vec3 _278 = vec3(1.00000000e+00, 1.00000000e+00, 1.00000000e+00) + _276;
	vec3 _280 = clarity_Exposure.xxx / _278;
	vec3 _281 = color_266 * _280;
	color_266 = _281;
	float _283 = 1.00000000e+00 / clarity_Gamma;
	vec3 _285 = pow(color_266, _283.xxx);
	color_266 = _285;
	vec3 _286;
	_286 = color_266;
	float _287 = F_xgetL709(_286);
	float lum = _287;
	float _290 = lum - 4.49999988e-01;
	float _292 = 1.00000000e+01 * _290;
	float _295 = clamp(_292, 0.00000000e+00, 1.00000000e+00);
	float L = _295;
	vec3 _298 = clarity_Bleach.xxx * color_266;
	vec3 A2 = _298;
	vec3 _301 = vec3(2.00000000e+00, 2.00000000e+00, 2.00000000e+00) * color_266;
	vec3 _303 = _301 * lum.xxx;
	vec3 result1 = _303;
	float _306 = 1.00000000e+00 - lum;
	float _308 = 2.00000000e+00 * _306;
	vec3 _310 = vec3(1.00000000e+00, 1.00000000e+00, 1.00000000e+00) - color_266;
	vec3 _312 = _308.xxx * _310;
	vec3 _314 = vec3(1.00000000e+00, 1.00000000e+00, 1.00000000e+00) - _312;
	vec3 result2 = _314;
	vec3 _317 = mix(result1, result2, L.xxx);
	vec3 newColor = _317;
	vec3 _319 = A2 * newColor;
	vec3 mixRGB = _319;
	vec3 _322 = vec3(1.00000000e+00, 1.00000000e+00, 1.00000000e+00) - A2;
	vec3 _323 = _322 * mixRGB;
	vec3 _324 = color_266 + _323;
	color_266 = _324;
	float _326 = dot(color_266, vec3(3.33333343e-01, 3.33333343e-01, 3.33333343e-01));
	vec3 middlegray = _326.xxx;
	vec3 _329 = color_266 - middlegray;
	vec3 diffcolor = _329;
	vec3 _332 = diffcolor * clarity_sat.xxx;
	vec3 _333 = color_266 + _332;
	vec3 _335 = diffcolor * clarity_sat.xxx;
	vec3 _337 = vec3(1.00000000e+00, 1.00000000e+00, 1.00000000e+00) + _335;
	vec3 _338 = _333 / _337;
	color_266 = _338;
	return color_266;
}
vec3 F_xBotWToneMap(
	in vec3 color)
{
	vec3 _342;
	_342 = color;
	float _343 = F_xgetL601(_342);
	float Lumn = _343;
	vec4 _348 = vec4(color.x, color.y, color.z, Lumn);
	vec4 _349 = -(_348);
	vec4 _350 = exp(_349);
	vec4 _352 = vec4(1.00000000e+00, 1.00000000e+00, 1.00000000e+00, 1.00000000e+00) - _350;
	vec4 exptm = _352;
	float _355 = exptm.w / Lumn;
	vec3 _357 = _355.xxx * color;
	vec3 cpre = _357;
	float _361 = pow(exptm.w, 2.00000000e+00);
	vec3 _364 = mix(cpre, exptm.xyz, _361.xxx);
	vec3 colorldr = _364;
	return colorldr;
}
vec3 F_xACESFilm(
	in vec3 color)
{
	vec3 _370 = color * clarity_Exposure.xxx;
	color = _370;
	vec3 _371;
	_371 = color;
	float _372 = F_xgetL709(_371);
	float Lumn_373 = _372;
	vec4 _377 = vec4(color.x, color.y, color.z, Lumn_373);
	vec4 tm = _377;
	vec4 _380 = vec4(2.50999999e+00, 2.50999999e+00, 2.50999999e+00, 2.50999999e+00) * tm;
	vec4 _382 = _380 + vec4(2.99999993e-02, 2.99999993e-02, 2.99999993e-02, 2.99999993e-02);
	vec4 _383 = tm * _382;
	vec4 _385 = vec4(2.43000007e+00, 2.43000007e+00, 2.43000007e+00, 2.43000007e+00) * tm;
	vec4 _387 = _385 + vec4(5.89999974e-01, 5.89999974e-01, 5.89999974e-01, 5.89999974e-01);
	vec4 _388 = tm * _387;
	vec4 _390 = _388 + vec4(1.40000001e-01, 1.40000001e-01, 1.40000001e-01, 1.40000001e-01);
	vec4 _391 = _383 / _390;
	tm = _391;
	float _393 = tm.w / Lumn_373;
	vec3 _395 = _393.xxx * color;
	vec3 cpre_396 = _395;
	float _399 = pow(tm.w, 2.00000000e+00);
	vec3 _402 = mix(cpre_396, tm.xyz, _399.xxx);
	vec3 colorldr_403 = _402;
	return colorldr_403;
}
vec3 F_xLevelsPass(
	in vec3 inputColor)
{
	float _409 = float(clarity_BlackPoint) / 2.55000000e+02;
	float black_point_float = _409;
	bool _411 = clarity_WhitePoint == clarity_BlackPoint;
	int _412 = clarity_WhitePoint - clarity_BlackPoint;
	float _415 = 2.55000000e+02 / float(_412);
	float _417 = _411 ? 1.01999994e+06 : _415;
	float white_point_float = _417;
	vec3 _420 = inputColor * white_point_float.xxx;
	float _421 = black_point_float * white_point_float;
	vec3 _423 = _420 - _421.xxx;
	return _423;
}
vec3 F_xColorMatrixPass(
	in vec3 inputColor)
{
	mat3x3 _436 = mat3x3(clarity_ColorMatrix_Red.x, clarity_ColorMatrix_Red.y, clarity_ColorMatrix_Red.z, clarity_ColorMatrix_Green.x, clarity_ColorMatrix_Green.y, clarity_ColorMatrix_Green.z, clarity_ColorMatrix_Blue.x, clarity_ColorMatrix_Blue.y, clarity_ColorMatrix_Blue.z);
	mat3x3 ColorMatrix = _436;
	vec3 color_438 = inputColor;
	vec3 _439 = (color_438 * ColorMatrix);
	vec3 _441 = mix(color_438, _439, clarity_CM_Strength.xxx);
	vec3 _442 = color_438 * _441;
	color_438 = _442;
	vec3 _443 = clamp(color_438, 0.0, 1.0);
	return _443;
}
vec3 F_xTechnicolorPass(
	in vec3 color)
{
	vec3 tcol = color;
	float _449 = clarity_RGBNegativeAmount.x * clarity_Power;
	float _451 = 1.00000000e+00 / _449;
	vec2 _454 = tcol.xy * _451.xx;
	vec2 negative_mul_r = _454;
	float _457 = clarity_RGBNegativeAmount.y * clarity_Power;
	float _459 = 1.00000000e+00 / _457;
	vec2 _462 = tcol.xy * _459.xx;
	vec2 negative_mul_g = _462;
	float _465 = clarity_RGBNegativeAmount.z * clarity_Power;
	float _467 = 1.00000000e+00 / _465;
	vec2 _470 = tcol.xz * _467.xx;
	vec2 negative_mul_b = _470;
	float _473 = dot(vec2(1.04999995e+00, 6.20000005e-01), negative_mul_r);
	vec3 _476 = _473.xxx + vec3(0.00000000e+00, 1.29999995e+00, 1.00000000e+00);
	vec3 output_r = _476;
	float _479 = dot(vec2(3.00000012e-01, 1.00000000e+00), negative_mul_g);
	vec3 _482 = _479.xxx + vec3(1.00000000e+00, 0.00000000e+00, 1.04999995e+00);
	vec3 output_g = _482;
	float _485 = dot(vec2(1.00000000e+00, 1.04999995e+00), negative_mul_b);
	vec3 _488 = _485.xxx + vec3(1.60000002e+00, 1.60000002e+00, 5.00000007e-02);
	vec3 output_b = _488;
	vec3 _490 = output_r * output_g;
	vec3 _491 = _490 * output_b;
	vec3 _493 = mix(tcol, _491, clarity_Strength.xxx);
	return _493;
}
vec3 F_xTechnicolor2Pass(
	in vec3 inputColor)
{
	vec3 color_497 = inputColor;
	vec3 _498 = vec3(clarity_Technicolor2_Red_Strength, clarity_Technicolor2_Green_Strength, clarity_Technicolor2_Blue_Strength);
	vec3 Color_Strength = _498;
	vec3 source = color_497;
	vec3 _502 = vec3(1.00000000e+00, 1.00000000e+00, 1.00000000e+00) - source;
	vec3 temp = _502;
	vec3 target = temp.yxy;
	vec3 target2 = temp.zzx;
	vec3 _508 = source * target;
	vec3 temp2 = _508;
	vec3 _510 = temp2 * target2;
	temp2 = _510;
	vec3 _511 = temp2 * Color_Strength;
	temp = _511;
	vec3 _513 = temp2 * clarity_Technicolor2_Brightness.xxx;
	temp2 = _513;
	target = temp.yxy;
	target2 = temp.zzx;
	vec3 _516 = source - target;
	temp = _516;
	vec3 _517 = temp + temp2;
	temp = _517;
	vec3 _518 = temp - target2;
	temp2 = _518;
	vec3 _520 = mix(source, temp2, clarity_Technicolor2_Strength.xxx);
	color_497 = _520;
	float _522 = dot(color_497, vec3(3.33000004e-01, 3.33000004e-01, 3.33000004e-01));
	vec3 _525 = mix(_522.xxx, color_497, clarity_Technicolor2_Saturation.xxx);
	color_497 = _525;
	return color_497;
}
vec3 F_xBotWVibrance(
	in vec3 color)
{
	float _531 = color.x + color.y;
	float _533 = _531 + color.z;
	float _535 = _533 / 3.00000000e+00;
	float avg = _535;
	float _539 = max(color.y, color.z);
	float _541 = max(color.x, _539);
	float maxc = _541;
	float _544 = 2.00000000e+00 * avg;
	float _546 = 1.00000000e+00 - _544;
	float _548 = pow(_546, 2.00000000e+00);
	float _550 = 1.00000000e+00 - _548;
	float w = _550;
	float _552 = w * clarity_satFactor;
	float _554 = 1.00000000e+00 + _552;
	float weight = _554;
	vec3 _558 = mix(maxc.xxx, color, weight.xxx);
	return _558;
}
vec3 F_xReshadeVibrance(
	in vec3 color)
{
	float _564 = max(color.y, color.z);
	float _566 = max(color.x, _564);
	float max_color = _566;
	float _570 = min(color.y, color.z);
	float _572 = min(color.x, _570);
	float min_color = _572;
	vec3 _574;
	_574 = color;
	float _575 = F_xgetL709(_574);
	float luma_576 = _575;
	float _577 = max_color - min_color;
	float color_saturation = _577;
	vec3 _580 = clarity_VibranceRGBBalance * clarity_Vibrance.xxx;
	vec3 coeffVibrance = _580;
	vec3 _582 = sign(coeffVibrance);
	vec3 _584 = _582 * color_saturation.xxx;
	vec3 _586 = vec3(1.00000000e+00, 1.00000000e+00, 1.00000000e+00) - _584;
	vec3 _587 = coeffVibrance * _586;
	vec3 _589 = vec3(1.00000000e+00, 1.00000000e+00, 1.00000000e+00) + _587;
	vec3 _591 = mix(luma_576.xxx, color, _589);
	return _591;
}
vec3 F_xContrastyPass(
	in vec3 fColour)
{
	vec3 _596 = fColour - clarity_crushContrast.xxx;
	vec3 _598 = max(vec3(0.00000000e+00, 0.00000000e+00, 0.00000000e+00), _596);
	fColour = _598;
	vec3 _600 = clarity_Exposure.xxx * fColour;
	vec3 _603 = clamp(_600, vec3(0.00000000e+00, 0.00000000e+00, 0.00000000e+00), vec3(1.00000000e+00, 1.00000000e+00, 1.00000000e+00));
	fColour = _603;
	float _605 = 1.00000000e+00 / clarity_Gamma;
	vec3 _607 = pow(fColour, _605.xxx);
	fColour = _607;
	vec3 _608;
	_608 = fColour;
	vec3 _609 = F_xReshadeVibrance(_608);
	return _609;
}
vec3 F_xFilmPass(
	in vec3 inputColor)
{
	vec3 B = inputColor;
	vec3 _614 = clamp(B, 0.0, 1.0);
	B = _614;
	vec3 _616 = pow(B, clarity_Linearization.xxx);
	B = _616;
	vec3 _619 = mix(vec3(0.00000000e+00, 0.00000000e+00, 0.00000000e+00), B, clarity_Filmic_Contrast.xxx);
	B = _619;
	float _621 = dot(B, vec3(2.12599993e-01, 7.15200007e-01, 7.22000003e-02));
	float A = _621;
	vec3 D = A.xxx;
	float _626 = 1.00000000e+00 / clarity_BaseGamma;
	vec3 _628 = pow(B, _626.xxx);
	B = _628;
	float d = clarity_BaseCurve;
	float _631 = exp(5.00000000e-01);
	float _633 = 1.00000000e+00 + _631;
	float _635 = 1.00000000e+00 / _633;
	float y = _635;
	float _638 = exp(5.00000000e-01);
	float _640 = 1.00000000e+00 + _638;
	float _642 = 1.00000000e+00 / _640;
	float z = _642;
	float _645 = exp(5.00000000e-01);
	float _647 = 1.00000000e+00 + _645;
	float _649 = 1.00000000e+00 / _647;
	float w_650 = _649;
	float _652 = d / 2.00000000e+00;
	float _653 = exp(_652);
	float _655 = 1.00000000e+00 + _653;
	float _657 = 1.00000000e+00 / _655;
	float v = _657;
	vec3 C = B;
	float _662 = D.x - 5.00000000e-01;
	float _664 = -1.00000000e+00 * _662;
	float _665 = exp(_664);
	float _667 = 1.00000000e+00 + _665;
	float _669 = 1.00000000e+00 / _667;
	float _670 = _669 - y;
	float _672 = 2.00000000e+00 * y;
	float _674 = 1.00000000e+00 - _672;
	float _675 = _670 / _674;
	D[0] = _675;
	float _678 = D.y - 5.00000000e-01;
	float _680 = -1.00000000e+00 * _678;
	float _681 = exp(_680);
	float _683 = 1.00000000e+00 + _681;
	float _685 = 1.00000000e+00 / _683;
	float _686 = _685 - z;
	float _688 = 2.00000000e+00 * z;
	float _690 = 1.00000000e+00 - _688;
	float _691 = _686 / _690;
	D[1] = _691;
	float _694 = D.z - 5.00000000e-01;
	float _696 = -1.00000000e+00 * _694;
	float _697 = exp(_696);
	float _699 = 1.00000000e+00 + _697;
	float _701 = 1.00000000e+00 / _699;
	float _702 = _701 - w_650;
	float _704 = 2.00000000e+00 * w_650;
	float _706 = 1.00000000e+00 - _704;
	float _707 = _702 / _706;
	D[2] = _707;
	float _709 = 1.00000000e+00 / clarity_EffectGamma;
	vec3 _711 = pow(D, _709.xxx);
	D = _711;
	vec3 _713 = vec3(1.00000000e+00, 1.00000000e+00, 1.00000000e+00) - D;
	vec3 Di = _713;
	vec3 _716 = mix(D, Di, clarity_Filmic_Bleach.xxx);
	D = _716;
	float _718 = abs(D.x);
	float _720 = pow(_718, 1.00000000e+00);
	D[0] = _720;
	float _722 = abs(D.y);
	float _724 = pow(_722, 1.00000000e+00);
	D[1] = _724;
	float _726 = abs(D.z);
	float _728 = pow(_726, 1.00000000e+00);
	D[2] = _728;
	bool _731 = D.x < 5.00000000e-01;
	float _734 = 2.00000000e+00 * D.x;
	float _736 = _734 - 1.00000000e+00;
	float _739 = B.x * B.x;
	float _741 = B.x - _739;
	float _742 = _736 * _741;
	float _744 = _742 + B.x;
	float _747 = 2.00000000e+00 * D.x;
	float _749 = _747 - 1.00000000e+00;
	float _751 = sqrt(B.x);
	float _753 = _751 - B.x;
	float _754 = _749 * _753;
	float _756 = _754 + B.x;
	float _757 = _731 ? _744 : _756;
	C[0] = _757;
	bool _760 = D.y < 5.00000000e-01;
	float _763 = 2.00000000e+00 * D.y;
	float _765 = _763 - 1.00000000e+00;
	float _768 = B.y * B.y;
	float _770 = B.y - _768;
	float _771 = _765 * _770;
	float _773 = _771 + B.y;
	float _776 = 2.00000000e+00 * D.y;
	float _778 = _776 - 1.00000000e+00;
	float _780 = sqrt(B.y);
	float _782 = _780 - B.y;
	float _783 = _778 * _782;
	float _785 = _783 + B.y;
	float _786 = _760 ? _773 : _785;
	C[1] = _786;
	bool _789 = D.z < 5.00000000e-01;
	float _792 = 2.00000000e+00 * D.z;
	float _794 = _792 - 1.00000000e+00;
	float _797 = B.z * B.z;
	float _799 = B.z - _797;
	float _800 = _794 * _799;
	float _802 = _800 + B.z;
	float _805 = 2.00000000e+00 * D.z;
	float _807 = _805 - 1.00000000e+00;
	float _809 = sqrt(B.z);
	float _811 = _809 - B.z;
	float _812 = _807 * _811;
	float _814 = _812 + B.z;
	float _815 = _789 ? _802 : _814;
	C[2] = _815;
	vec3 _817 = mix(B, C, clarity_Filmic_Strength.xxx);
	vec3 F = _817;
	float _819 = -(d);
	vec3 _821 = F - vec3(5.00000000e-01, 5.00000000e-01, 5.00000000e-01);
	vec3 _823 = _819.xxx * _821;
	vec3 _824 = exp(_823);
	vec3 _826 = vec3(1.00000000e+00, 1.00000000e+00, 1.00000000e+00) + _824;
	vec3 _828 = vec3(1.00000000e+00, 1.00000000e+00, 1.00000000e+00) / _826;
	vec3 _830 = _828 - v.xxx;
	float _832 = 2.00000000e+00 * v;
	float _834 = 1.00000000e+00 - _832;
	vec3 _836 = _830 / _834.xxx;
	F = _836;
	float _838 = 1.00000000e+00 - clarity_Saturation;
	float r2R = _838;
	float _841 = 0.00000000e+00 + clarity_Saturation;
	float g2R = _841;
	float _844 = 0.00000000e+00 + clarity_Saturation;
	float b2R = _844;
	float _847 = 0.00000000e+00 + clarity_Saturation;
	float r2G = _847;
	float _850 = 1.00000000e+00 - clarity_Fade;
	float _851 = _850 - clarity_Saturation;
	float g2G = _851;
	float _854 = 0.00000000e+00 + clarity_Fade;
	float _855 = _854 + clarity_Saturation;
	float b2G = _855;
	float _858 = 0.00000000e+00 + clarity_Saturation;
	float r2B = _858;
	float _861 = 0.00000000e+00 + clarity_Fade;
	float _862 = _861 + clarity_Saturation;
	float g2B = _862;
	float _865 = 1.00000000e+00 - clarity_Fade;
	float _866 = _865 - clarity_Saturation;
	float b2B = _866;
	vec3 iF = F;
	float _870 = iF.x * r2R;
	float _872 = iF.y * g2R;
	float _873 = _870 + _872;
	float _875 = iF.z * b2R;
	float _876 = _873 + _875;
	F[0] = _876;
	float _878 = iF.x * r2G;
	float _880 = iF.y * g2G;
	float _881 = _878 + _880;
	float _883 = iF.z * b2G;
	float _884 = _881 + _883;
	F[1] = _884;
	float _886 = iF.x * r2B;
	float _888 = iF.y * g2B;
	float _889 = _886 + _888;
	float _891 = iF.z * b2B;
	float _892 = _889 + _891;
	F[2] = _892;
	float _894 = dot(F, vec3(2.12599993e-01, 7.15200007e-01, 7.22000003e-02));
	float N = _894;
	vec3 Cn = F;
	bool _898 = N < 5.00000000e-01;
	float _900 = 2.00000000e+00 * N;
	float _902 = _900 - 1.00000000e+00;
	vec3 _903 = F * F;
	vec3 _904 = F - _903;
	vec3 _906 = _902.xxx * _904;
	vec3 _907 = _906 + F;
	float _909 = 2.00000000e+00 * N;
	float _911 = _909 - 1.00000000e+00;
	vec3 _912 = sqrt(F);
	vec3 _913 = _912 - F;
	vec3 _915 = _911.xxx * _913;
	vec3 _916 = _915 + F;
	vec3 _918 = compCond(_898.xxx, _907, _916);
	Cn = _918;
	vec3 _920 = max(Cn, vec3(0.00000000e+00, 0.00000000e+00, 0.00000000e+00));
	float _922 = 1.00000000e+00 / clarity_Linearization;
	vec3 _924 = pow(_920, _922.xxx);
	Cn = _924;
	vec3 _926 = mix(B, Cn, clarity_Filmic_Strength.xxx);
	vec3 Fn = _926;
	return Fn;
}
vec3 F_xDPXPass(
	in vec3 inputColor)
{
	vec3 color_931 = inputColor;
	vec3 B_932 = color_931;
	float _934 = 1.00000000e+00 - clarity_DPX_Contrast;
	vec3 _936 = B_932 * _934.xxx;
	float _938 = 5.00000000e-01 * clarity_DPX_Contrast;
	vec3 _940 = _936 + _938.xxx;
	B_932 = _940;
	float _942 = clarity_RGB_Curve / 2.00000000e+00;
	float _943 = exp(_942);
	float _945 = 1.00000000e+00 + _943;
	float _947 = 1.00000000e+00 / _945;
	vec3 Btemp = _947.xxx;
	float _950 = -(clarity_RGB_Curve);
	vec3 _952 = B_932 - clarity_RGB_C.xxx;
	vec3 _954 = _950.xxx * _952;
	vec3 _955 = exp(_954);
	vec3 _957 = vec3(1.00000000e+00, 1.00000000e+00, 1.00000000e+00) + _955;
	vec3 _959 = vec3(1.00000000e+00, 1.00000000e+00, 1.00000000e+00) / _957;
	vec3 _961 = vec3(-2.00000000e+00, -2.00000000e+00, -2.00000000e+00) * Btemp;
	vec3 _963 = _961 + vec3(1.00000000e+00, 1.00000000e+00, 1.00000000e+00);
	vec3 _964 = _959 / _963;
	vec3 _965 = -(Btemp);
	vec3 _967 = vec3(-2.00000000e+00, -2.00000000e+00, -2.00000000e+00) * Btemp;
	vec3 _969 = _967 + vec3(1.00000000e+00, 1.00000000e+00, 1.00000000e+00);
	vec3 _970 = _965 / _969;
	vec3 _971 = _964 + _970;
	B_932 = _971;
	float _974 = max(B_932.x, B_932.y);
	float _976 = max(_974, B_932.z);
	float value = _976;
	vec3 _979 = B_932 / value.xxx;
	vec3 Ztemp = _979;
	vec3 _981 = abs(color_931);
	float _983 = 1.00000000e+00 / clarity_Colorfulness;
	vec3 _985 = pow(_981, _983.xxx);
	color_931 = _985;
	vec3 _987 = Ztemp * value.xxx;
	vec3 c0 = _987;
	vec3 _990 = (c0 * mat3x3(5.00303388e-01, 3.38097572e-01, 1.64589778e-01, 2.57968903e-01, 6.76195264e-01, 6.58358485e-02, 2.34517884e-02, 1.12699270e-01, 8.66839647e-01));
	c0 = _990;
	float _992 = dot(c0, vec3(3.00000012e-01, 5.89999974e-01, 1.09999999e-01));
	float luma_993 = _992;
	float _995 = 1.00000000e+00 - clarity_DPX_Saturation;
	float _996 = _995 * luma_993;
	vec3 _998 = clarity_DPX_Saturation.xxx * c0;
	vec3 _1000 = _996.xxx + _998;
	c0 = _1000;
	vec3 _1002 = (c0 * mat3x3(2.67147112e+00, -1.26723611e+00, -4.10995603e-01, -1.02510703e+00, 1.98409116e+00, 4.39502485e-02, 6.10009469e-02, -2.23670751e-01, 1.15902102e+00));
	c0 = _1002;
	vec3 _1004 = mix(color_931, c0, clarity_DPX_Strength.xxx);
	return _1004;
}
vec3 F_xLiftGammaGainPass(
	in vec3 inputColor)
{
	vec3 color_1008 = inputColor;
	vec3 _1010 = vec3(5.00000000e-01, 5.00000000e-01, 5.00000000e-01) * clarity_RGB_Lift;
	vec3 _1012 = vec3(1.50000000e+00, 1.50000000e+00, 1.50000000e+00) - _1010;
	vec3 _1013 = color_1008 * _1012;
	vec3 _1015 = vec3(5.00000000e-01, 5.00000000e-01, 5.00000000e-01) * clarity_RGB_Lift;
	vec3 _1016 = _1013 + _1015;
	vec3 _1018 = _1016 - vec3(5.00000000e-01, 5.00000000e-01, 5.00000000e-01);
	color_1008 = _1018;
	vec3 _1020 = max(vec3(0.00000000e+00, 0.00000000e+00, 0.00000000e+00), color_1008);
	color_1008 = _1020;
	vec3 _1022 = vec3(1.00000000e+00, 1.00000000e+00, 1.00000000e+00) / clarity_RGB_Gamma;
	vec3 _1023 = pow(color_1008, _1022);
	color_1008 = _1023;
	vec3 _1024 = color_1008 * clarity_RGB_Gain;
	color_1008 = _1024;
	return color_1008;
}
vec3 F_xCurvesPass(
	in vec3 inputColor)
{
	vec3 colorInput = inputColor;
	float _1030 = clarity_Contrast * 1.50000000e+00;
	float Contrast_blend = _1030;
	vec3 x = colorInput;
	vec3 _1034 = x - vec3(5.00000000e-01, 5.00000000e-01, 5.00000000e-01);
	x = _1034;
	vec3 _1035 = abs(x);
	vec3 _1037 = vec3(5.00000000e-01, 5.00000000e-01, 5.00000000e-01) + _1035;
	vec3 _1038 = x / _1037;
	vec3 _1040 = _1038 + vec3(5.00000000e-01, 5.00000000e-01, 5.00000000e-01);
	x = _1040;
	vec3 _1042 = mix(colorInput, x, Contrast_blend.xxx);
	colorInput = _1042;
	return colorInput;
}
vec3 F_xVibrancePass(
	in vec3 color)
{
	float _1048 = max(color.y, color.z);
	float _1050 = max(color.x, _1048);
	float maxc_1051 = _1050;
	float _1054 = min(color.y, color.z);
	float _1056 = min(color.x, _1054);
	float minc = _1056;
	vec3 _1058;
	_1058 = color;
	float _1059 = F_xgetL709(_1058);
	float luma_1060 = _1059;
	float _1061 = maxc_1051 - minc;
	float color_saturation_1062 = _1061;
	vec3 _1064 = clarity_VibranceRGBBalance * clarity_Vibrance.xxx;
	vec3 coeffVibrance_1065 = _1064;
	vec3 _1066 = sign(coeffVibrance_1065);
	vec3 _1068 = _1066 * color_saturation_1062.xxx;
	vec3 _1070 = vec3(1.00000000e+00, 1.00000000e+00, 1.00000000e+00) - _1068;
	vec3 _1071 = coeffVibrance_1065 * _1070;
	vec3 _1073 = vec3(1.00000000e+00, 1.00000000e+00, 1.00000000e+00) + _1071;
	vec3 _1075 = mix(luma_1060.xxx, color, _1073);
	return _1075;
}
vec3 F_xBloomPass(
	in vec3 color,
	in vec2 uv)
{
	vec3 bloom_sum1 = vec3(0.00000000e+00, 0.00000000e+00, 0.00000000e+00);
	vec3 bloom_sum2 = vec3(0.00000000e+00, 0.00000000e+00, 0.00000000e+00);
	vec2 _1086 = vec2(0.00000000e+00, -1.00000000e+00) * clarity_radius1.xx;
	vec2 _1088 = _1086 * vec2(1.24999997e-03, 1.66666671e-03);
	vec2 _1089 = uv + _1088;
	vec4 _1090 = texture(textureSrc, _1089);
	vec3 _1092 = bloom_sum1 + _1090.xyz;
	bloom_sum1 = _1092;
	vec2 _1095 = vec2(0.00000000e+00, 1.00000000e+00) * clarity_radius1.xx;
	vec2 _1097 = _1095 * vec2(1.24999997e-03, 1.66666671e-03);
	vec2 _1098 = uv + _1097;
	vec4 _1099 = texture(textureSrc, _1098);
	vec3 _1101 = bloom_sum1 + _1099.xyz;
	bloom_sum1 = _1101;
	vec2 _1104 = vec2(-1.00000000e+00, 0.00000000e+00) * clarity_radius1.xx;
	vec2 _1106 = _1104 * vec2(1.24999997e-03, 1.66666671e-03);
	vec2 _1107 = uv + _1106;
	vec4 _1108 = texture(textureSrc, _1107);
	vec3 _1110 = bloom_sum1 + _1108.xyz;
	bloom_sum1 = _1110;
	vec2 _1113 = vec2(1.00000000e+00, 0.00000000e+00) * clarity_radius1.xx;
	vec2 _1115 = _1113 * vec2(1.24999997e-03, 1.66666671e-03);
	vec2 _1116 = uv + _1115;
	vec4 _1117 = texture(textureSrc, _1116);
	vec3 _1119 = bloom_sum1 + _1117.xyz;
	bloom_sum1 = _1119;
	vec3 _1121 = bloom_sum1 * vec3(2.50000000e-01, 2.50000000e-01, 2.50000000e-01);
	bloom_sum1 = _1121;
	vec2 _1124 = vec2(0.00000000e+00, -2.50000000e+00) * clarity_radius2.xx;
	vec2 _1126 = _1124 * vec2(1.24999997e-03, 1.66666671e-03);
	vec2 _1127 = uv + _1126;
	vec4 _1128 = texture(textureSrc, _1127);
	vec3 _1130 = bloom_sum2 + _1128.xyz;
	bloom_sum2 = _1130;
	vec2 _1133 = vec2(0.00000000e+00, 2.50000000e+00) * clarity_radius2.xx;
	vec2 _1135 = _1133 * vec2(1.24999997e-03, 1.66666671e-03);
	vec2 _1136 = uv + _1135;
	vec4 _1137 = texture(textureSrc, _1136);
	vec3 _1139 = bloom_sum2 + _1137.xyz;
	bloom_sum2 = _1139;
	vec2 _1142 = vec2(-2.50000000e+00, 0.00000000e+00) * clarity_radius2.xx;
	vec2 _1144 = _1142 * vec2(1.24999997e-03, 1.66666671e-03);
	vec2 _1145 = uv + _1144;
	vec4 _1146 = texture(textureSrc, _1145);
	vec3 _1148 = bloom_sum2 + _1146.xyz;
	bloom_sum2 = _1148;
	vec2 _1151 = vec2(2.50000000e+00, 0.00000000e+00) * clarity_radius2.xx;
	vec2 _1153 = _1151 * vec2(1.24999997e-03, 1.66666671e-03);
	vec2 _1154 = uv + _1153;
	vec4 _1155 = texture(textureSrc, _1154);
	vec3 _1157 = bloom_sum2 + _1155.xyz;
	bloom_sum2 = _1157;
	vec3 _1159 = bloom_sum2 * vec3(9.99999978e-03, 9.99999978e-03, 9.99999978e-03);
	bloom_sum2 = _1159;
	float _1160 = clarity_radius2 - clarity_radius1;
	float dist = _1160;
	vec3 _1162 = bloom_sum2 - bloom_sum1;
	vec3 _1163 = color + _1162;
	vec3 _1165 = _1163 * dist.xxx;
	vec3 HDR = _1165;
	vec3 _1167 = HDR + color;
	return _1167;
}
vec3 F_xLumaSharpenPass(
	in vec2 uv)
{
	vec4 _1171 = texture(textureSrc, uv);
	vec3 c = _1171.xyz;
	float _1174 = -(clarity_offset_bias);
	vec2 _1176 = vec2(0.00000000e+00, _1174);
	vec2 _1178 = _1176 * vec2(1.24999997e-03, 1.66666671e-03);
	vec2 _1179 = uv + _1178;
	vec4 _1180 = texture(textureSrc, _1179);
	vec3 north = _1180.xyz;
	vec2 _1184 = vec2(0.00000000e+00, clarity_offset_bias);
	vec2 _1186 = _1184 * vec2(1.24999997e-03, 1.66666671e-03);
	vec2 _1187 = uv + _1186;
	vec4 _1188 = texture(textureSrc, _1187);
	vec3 south = _1188.xyz;
	float _1191 = -(clarity_offset_bias);
	vec2 _1193 = vec2(_1191, 0.00000000e+00);
	vec2 _1195 = _1193 * vec2(1.24999997e-03, 1.66666671e-03);
	vec2 _1196 = uv + _1195;
	vec4 _1197 = texture(textureSrc, _1196);
	vec3 west = _1197.xyz;
	vec2 _1201 = vec2(clarity_offset_bias, 0.00000000e+00);
	vec2 _1203 = _1201 * vec2(1.24999997e-03, 1.66666671e-03);
	vec2 _1204 = uv + _1203;
	vec4 _1205 = texture(textureSrc, _1204);
	vec3 east = _1205.xyz;
	vec3 _1208 = min(north, south);
	vec3 _1209 = min(west, east);
	vec3 _1210 = min(_1208, _1209);
	vec3 _1211 = min(_1210, c);
	vec3 minRGB = _1211;
	vec3 _1213 = max(north, south);
	vec3 _1214 = max(west, east);
	vec3 _1215 = max(_1213, _1214);
	vec3 _1216 = max(_1215, c);
	vec3 maxRGB = _1216;
	vec3 _1218 = north + south;
	vec3 _1219 = _1218 + west;
	vec3 _1220 = _1219 + east;
	vec3 _1222 = _1220 * vec3(2.50000000e-01, 2.50000000e-01, 2.50000000e-01);
	vec3 blur = _1222;
	vec3 _1224 = c - blur;
	vec3 _1226 = _1224 * clarity_sharp_strength.xxx;
	vec3 _1227 = c + _1226;
	vec3 sharp = _1227;
	vec3 _1230 = minRGB - clarity_sharp_clamp.xxx;
	vec3 _1232 = maxRGB + clarity_sharp_clamp.xxx;
	vec3 _1233 = clamp(sharp, _1230, _1232);
	return _1233;
}
vec3 F_xPS_Clarity(
	in vec4 pos,
	in vec2 uv)
{
	vec4 _1238 = texture(textureSrc, uv);
	vec3 color_1240 = _1238.xyz;
	bool _1245 = clarity_adjust_bloom == 1;
	if (_1245)
	{
		vec3 _1246;
		vec2 _1247;
		_1246 = color_1240;
		_1247 = uv;
		vec3 _1248 = F_xBloomPass(_1246, _1247);
		color_1240 = _1248;
	}
	bool _1253 = clarity_lumapassing == 1;
	if (_1253)
	{
		vec2 _1254;
		_1254 = uv;
		vec3 _1255 = F_xLumaSharpenPass(_1254);
		color_1240 = _1255;
	}
	bool _1260 = clarity_Tone_map == -1;
	if (_1260)
	{
		vec3 _1262 = color_1240 * clarity_Exposure.xxx;
		vec3 _1265 = clamp(_1262, vec3(0.00000000e+00, 0.00000000e+00, 0.00000000e+00), vec3(1.00000000e+00, 1.00000000e+00, 1.00000000e+00));
		color_1240 = _1265;
	}
	else
	{
		bool _1270 = clarity_Tone_map == 0;
		if (_1270)
		{
			vec3 _1271;
			_1271 = color_1240;
			vec3 _1272 = F_xBotWToneMap(_1271);
			color_1240 = _1272;
		}
		else
		{
			bool _1277 = clarity_Tone_map == 1;
			if (_1277)
			{
				vec3 _1278;
				_1278 = color_1240;
				vec3 _1279 = F_xReshadeToneMap(_1278);
				color_1240 = _1279;
			}
			else
			{
				bool _1284 = clarity_Tone_map == 2;
				if (_1284)
				{
					vec3 _1285;
					_1285 = color_1240;
					vec3 _1286 = F_xLinearToneMapping(_1285);
					color_1240 = _1286;
				}
				else
				{
					bool _1291 = clarity_Tone_map == 3;
					if (_1291)
					{
						vec3 _1292;
						_1292 = color_1240;
						vec3 _1293 = F_xSimpleReinhardToneMapping(_1292);
						color_1240 = _1293;
					}
					else
					{
						bool _1298 = clarity_Tone_map == 4;
						if (_1298)
						{
							vec3 _1299;
							_1299 = color_1240;
							vec3 _1300 = F_xLumaBasedReinhardToneMapping(_1299);
							color_1240 = _1300;
						}
						else
						{
							bool _1305 = clarity_Tone_map == 5;
							if (_1305)
							{
								vec3 _1306;
								_1306 = color_1240;
								vec3 _1307 = F_xWhitePreservingLumaBasedReinhardToneMapping(_1306);
								color_1240 = _1307;
							}
							else
							{
								bool _1312 = clarity_Tone_map == 6;
								if (_1312)
								{
									vec3 _1313;
									_1313 = color_1240;
									vec3 _1314 = F_xRomBinDaHouseToneMapping(_1313);
									color_1240 = _1314;
								}
								else
								{
									bool _1319 = clarity_Tone_map == 7;
									if (_1319)
									{
										vec3 _1320;
										_1320 = color_1240;
										vec3 _1321 = F_xFilmicToneMapping(_1320);
										color_1240 = _1321;
									}
									else
									{
										bool _1326 = clarity_Tone_map == 8;
										if (_1326)
										{
											vec3 _1327;
											_1327 = color_1240;
											vec3 _1328 = F_xUncharted2ToneMapping(_1327);
											color_1240 = _1328;
										}
										else
										{
											bool _1333 = clarity_Tone_map == 9;
											if (_1333)
											{
												vec3 _1334;
												_1334 = color_1240;
												vec3 _1335 = F_xACESFilm(_1334);
												color_1240 = _1335;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	bool _1340 = clarity_blacknwhitepass == 1;
	if (_1340)
	{
		vec3 _1341;
		_1341 = color_1240;
		vec3 _1342 = F_xLevelsPass(_1341);
		color_1240 = _1342;
	}
	bool _1347 = clarity_cmatrix == 1;
	if (_1347)
	{
		vec3 _1348;
		_1348 = color_1240;
		vec3 _1349 = F_xColorMatrixPass(_1348);
		color_1240 = _1349;
	}
	bool _1354 = clarity_Tech == 1;
	if (_1354)
	{
		vec3 _1355;
		_1355 = color_1240;
		vec3 _1356 = F_xTechnicolorPass(_1355);
		color_1240 = _1356;
	}
	bool _1361 = clarity_Techine == 1;
	if (_1361)
	{
		vec3 _1362;
		_1362 = color_1240;
		vec3 _1363 = F_xTechnicolor2Pass(_1362);
		color_1240 = _1363;
	}
	bool _1368 = clarity_post_process == 0;
	if (_1368)
	{
		vec3 _1369;
		_1369 = color_1240;
		vec3 _1370 = F_xBotWVibrance(_1369);
		color_1240 = _1370;
	}
	else
	{
		bool _1375 = clarity_post_process == 1;
		if (_1375)
		{
			vec3 _1376;
			_1376 = color_1240;
			vec3 _1377 = F_xContrastyPass(_1376);
			color_1240 = _1377;
		}
	}
	bool _1382 = clarity_dpxpass == 1;
	if (_1382)
	{
		vec3 _1383;
		_1383 = color_1240;
		vec3 _1384 = F_xDPXPass(_1383);
		color_1240 = _1384;
	}
	bool _1389 = clarity_Filmicpass == 1;
	if (_1389)
	{
		vec3 _1390;
		_1390 = color_1240;
		vec3 _1391 = F_xFilmPass(_1390);
		color_1240 = _1391;
	}
	bool _1396 = clarity_lggpass == 1;
	if (_1396)
	{
		vec3 _1397;
		_1397 = color_1240;
		vec3 _1398 = F_xLiftGammaGainPass(_1397);
		color_1240 = _1398;
	}
	bool _1403 = clarity_CurvesPss == 1;
	if (_1403)
	{
		vec3 _1404;
		_1404 = color_1240;
		vec3 _1405 = F_xCurvesPass(_1404);
		color_1240 = _1405;
	}
	bool _1410 = clarity_vibpass == 1;
	if (_1410)
	{
		vec3 _1411;
		_1411 = color_1240;
		vec3 _1412 = F_xVibrancePass(_1411);
		color_1240 = _1412;
	}
	vec3 _1413 = clamp(color_1240, 0.0, 1.0);
	return _1413;
}
void outputShader()
{
	colorOut0 = vec4(F_xPS_Clarity(vec4(gl_FragCoord), passUV), 1.0);
}

)GLSL";
}
