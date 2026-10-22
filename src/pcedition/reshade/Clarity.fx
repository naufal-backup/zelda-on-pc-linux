/*
 * Clarity for Zelda on PC - a port of the Clarity shader of the community graphic pack
 * ("Enhancements", shader 37040a485a29d54e) to a ReShade effect.
 *
 * The original shader patches a game pixel shader: the preset is a compile time #define, so a change of
 * preset needs a restart of the game. Here every preset is a set of uniform values instead, so a preset
 * can be changed while the game runs.
 *
 * Ported from the graphic pack made by Jamie, Kiri, Crementif, Serfrost and the other creators of the
 * alternative presets. Only the effect chain itself (the part that draws the picture) is ported; the
 * Bloom and the HDR of the original take a second picture of the game that is not there at this point of
 * the pipeline, so they are done from the picture itself.
 */

#include "ReShade.fxh"

// ---------------------------------------------------------------------------------------------------
// the parameters of a preset (the names follow the original shader)
// ---------------------------------------------------------------------------------------------------

uniform int adjust_bloom <
	ui_label = "Bloom";
	ui_tooltip = "A soft glow around the bright parts of the picture.";
	ui_category = "Bloom";
	ui_type = "combo";
	ui_items = "Off\0On\0";
> = 1;
uniform float bloomFactor < ui_category = "Bloom"; ui_min = 0.0; ui_max = 3.0; > = 1.0;
uniform float radius1 < ui_category = "Bloom"; ui_min = 0.0; ui_max = 8.0; > = 0.793;
uniform float radius2 < ui_category = "Bloom"; ui_min = 0.0; ui_max = 8.0; > = 0.87;

uniform int HDRpassing <
	ui_label = "Fake HDR";
	ui_category = "HDR";
	ui_type = "combo";
	ui_items = "Off\0On\0";
> = 0;
uniform float HDRPower < ui_category = "HDR"; ui_min = 0.0; ui_max = 8.0; > = 1.30;

uniform int lumapassing <
	ui_label = "Sharpening";
	ui_category = "Sharpening";
	ui_type = "combo";
	ui_items = "Off\0On\0";
> = 0;
uniform float sharp_strength < ui_category = "Sharpening"; ui_min = 0.10; ui_max = 3.00; > = 0.65;
uniform float sharp_clamp < ui_category = "Sharpening"; ui_min = 0.0; ui_max = 1.0; > = 0.035;
uniform float offset_bias < ui_category = "Sharpening"; ui_min = 0.0; ui_max = 6.0; > = 1.0;

uniform int Tone_map <
	ui_label = "Tone mapping";
	ui_category = "Tone mapping";
	ui_tooltip = "How the bright parts of the picture are brought into the range of the screen.";
	ui_type = "combo";
	ui_items = "Game (BotW)\0ReShade\0Linear\0Reinhard\0Reinhard (luma)\0Reinhard (white)\0RomBinDaHouse\0Filmic\0Uncharted 2\0ACES\0";
> = 0;
uniform float Exposure < ui_category = "Tone mapping"; ui_min = 0.0; ui_max = 4.0; > = 1.0;
uniform float Gamma < ui_category = "Tone mapping"; ui_min = 0.5; ui_max = 3.0; > = 1.0;
uniform float Bleach < ui_category = "Tone mapping"; ui_min = 0.0; ui_max = 1.0; > = 0.1;
uniform float sat < ui_category = "Tone mapping"; ui_min = 0.0; ui_max = 1.0; > = 0.0;
uniform float defog < ui_category = "Tone mapping"; ui_min = 0.0; ui_max = 1.0; > = 0.0;
uniform float3 FogColor < ui_category = "Tone mapping"; > = float3(0.5, 0.6, 0.7);

uniform int blacknwhitepass < ui_label = "Levels"; ui_category = "Levels"; ui_type = "combo"; ui_items = "Off\0On\0"; > = 0;
uniform int BlackPoint < ui_category = "Levels"; ui_min = 0; ui_max = 255; > = 0;
uniform int WhitePoint < ui_category = "Levels"; ui_min = 0; ui_max = 255; > = 255;

uniform int cmatrix < ui_label = "Color matrix"; ui_category = "Color matrix"; ui_type = "combo"; ui_items = "Off\0On\0"; > = 0;
uniform float CM_Strength < ui_category = "Color matrix"; ui_min = 0.0; ui_max = 1.0; > = 0.0;
uniform float3 ColorMatrix_Red < ui_category = "Color matrix"; > = float3(0.817, 0.183, 0.0);
uniform float3 ColorMatrix_Green < ui_category = "Color matrix"; > = float3(0.333, 0.667, 0.0);
uniform float3 ColorMatrix_Blue < ui_category = "Color matrix"; > = float3(0.0, 0.125, 0.875);

uniform int Tech < ui_label = "Technicolor"; ui_category = "Technicolor"; ui_type = "combo"; ui_items = "Off\0On\0"; > = 0;
uniform float Strength < ui_category = "Technicolor"; ui_min = 0.0; ui_max = 1.0; > = 0.20;
uniform float Power < ui_category = "Technicolor"; ui_min = 0.0; ui_max = 8.0; > = 4.0;
uniform float3 RGBNegativeAmount < ui_category = "Technicolor"; > = float3(0.88, 0.88, 0.88);

uniform int Techine < ui_label = "Technicolor 2"; ui_category = "Technicolor 2"; ui_type = "combo"; ui_items = "Off\0On\0"; > = 0;
uniform float Technicolor2_Red_Strength < ui_category = "Technicolor 2"; ui_min = 0.0; ui_max = 1.0; > = 0.02;
uniform float Technicolor2_Green_Strength < ui_category = "Technicolor 2"; ui_min = 0.0; ui_max = 1.0; > = 0.02;
uniform float Technicolor2_Blue_Strength < ui_category = "Technicolor 2"; ui_min = 0.0; ui_max = 1.0; > = 0.02;
uniform float Technicolor2_Brightness < ui_category = "Technicolor 2"; ui_min = 0.0; ui_max = 2.0; > = 1.0;
uniform float Technicolor2_Saturation < ui_category = "Technicolor 2"; ui_min = 0.0; ui_max = 2.0; > = 1.0;
uniform float Technicolor2_Strength < ui_category = "Technicolor 2"; ui_min = 0.0; ui_max = 1.0; > = 1.0;

uniform int post_process <
	ui_label = "Saturation and contrast";
	ui_category = "Saturation and contrast";
	ui_type = "combo";
	ui_items = "Vibrance (BotW)\0Contrasty (ReShade)\0";
> = 0;
uniform float Vibrance < ui_category = "Saturation and contrast"; ui_min = 0.0; ui_max = 1.0; > = 0.15;
uniform float3 VibranceRGBBalance < ui_category = "Saturation and contrast"; > = float3(1.0, 1.0, 1.0);
uniform float satFactor < ui_category = "Saturation and contrast"; ui_min = 0.0; ui_max = 1.0; > = 0.18;
uniform float crushContrast < ui_category = "Saturation and contrast"; ui_min = 0.0; ui_max = 1.0; > = 0.0;

uniform int dpxpass < ui_label = "DPX"; ui_category = "DPX"; ui_type = "combo"; ui_items = "Off\0On\0"; > = 0;
uniform float DPX_Contrast < ui_category = "DPX"; ui_min = 0.0; ui_max = 2.0; > = 1.0;
uniform float DPX_Saturation < ui_category = "DPX"; ui_min = 0.0; ui_max = 2.0; > = 1.0;
uniform float DPX_Strength < ui_category = "DPX"; ui_min = 0.0; ui_max = 1.0; > = 0.2;
uniform float RGB_Curve < ui_category = "DPX"; ui_min = 0.0; ui_max = 8.0; > = 1.0;
uniform float RGB_C < ui_category = "DPX"; ui_min = 0.0; ui_max = 1.0; > = 0.5;
uniform float Colorfulness < ui_category = "DPX"; ui_min = 0.0; ui_max = 4.0; > = 1.0;

uniform int Filmicpass < ui_label = "Filmic"; ui_category = "Filmic"; ui_type = "combo"; ui_items = "Off\0On\0"; > = 0;
uniform float Filmic_Strength < ui_category = "Filmic"; ui_min = 0.0; ui_max = 1.0; > = 0.85;
uniform float Filmic_Contrast < ui_category = "Filmic"; ui_min = 0.0; ui_max = 2.0; > = 1.0;
uniform float Filmic_Bleach < ui_category = "Filmic"; ui_min = 0.0; ui_max = 1.0; > = 0.0;
uniform float Linearization < ui_category = "Filmic"; ui_min = 0.0; ui_max = 2.0; > = 0.5;
uniform float Contrast < ui_category = "Filmic"; ui_min = 0.0; ui_max = 2.0; > = 0.65;
uniform float BaseGamma < ui_category = "Filmic"; ui_min = 0.0; ui_max = 3.0; > = 1.0;
uniform float EffectGamma < ui_category = "Filmic"; ui_min = 0.0; ui_max = 3.0; > = 0.68;
uniform float Fade < ui_category = "Filmic"; ui_min = 0.0; ui_max = 1.0; > = 0.4;
uniform float Saturation < ui_category = "Filmic"; ui_min = 0.0; ui_max = 2.0; > = 0.5;
uniform float BaseCurve < ui_category = "Filmic"; ui_min = 0.0; ui_max = 8.0; > = 1.5;

uniform int lggpass < ui_label = "Lift Gamma Gain"; ui_category = "Lift Gamma Gain"; ui_type = "combo"; ui_items = "Off\0On\0"; > = 0;
uniform float3 RGB_Lift < ui_category = "Lift Gamma Gain"; > = float3(1.0, 1.0, 1.0);
uniform float3 RGB_Gamma < ui_category = "Lift Gamma Gain"; > = float3(1.0, 1.0, 1.0);
uniform float3 RGB_Gain < ui_category = "Lift Gamma Gain"; > = float3(1.0, 1.0, 1.0);

uniform int CurvesPss < ui_label = "Curves"; ui_category = "Curves"; ui_type = "combo"; ui_items = "Off\0On\0"; > = 0;

uniform int vibpass < ui_label = "Vibrance (ReShade)"; ui_category = "Vibrance"; ui_type = "combo"; ui_items = "Off\0On\0"; > = 0;

// ---------------------------------------------------------------------------------------------------
// the effect chain
// ---------------------------------------------------------------------------------------------------

static const float3 kLumCoeff601 = float3(0.2989, 0.5866, 0.1145);
static const float3 kLumCoeff709 = float3(0.2126, 0.7152, 0.0722);

float getL601(float3 rgb) { return dot(rgb, kLumCoeff601); }
float getL709(float3 rgb) { return dot(rgb, kLumCoeff709); }

// --- tone mapping ---

float3 LinearToneMapping(float3 color)
{
	color = clamp(Exposure * color, 0.0, 1.0);
	return pow(color, 1.0 / Gamma);
}

float3 SimpleReinhardToneMapping(float3 color)
{
	const float exposure = 1.5;
	color *= exposure / (1.0 + color / exposure);
	return pow(color, 1.0 / Gamma);
}

float3 LumaBasedReinhardToneMapping(float3 color)
{
	const float luma = getL709(color);
	const float toneMappedLuma = luma / (1.0 + luma);
	color *= toneMappedLuma / luma;
	return pow(color, 1.0 / Gamma);
}

float3 WhitePreservingLumaBasedReinhardToneMapping(float3 color)
{
	const float white = 2.0;
	const float luma = getL709(color);
	const float toneMappedLuma = luma * (1.0 + luma / (white * white)) / (1.0 + luma);
	color *= toneMappedLuma / luma;
	return pow(color, 1.0 / Gamma);
}

float3 RomBinDaHouseToneMapping(float3 color)
{
	color = exp(-1.0 / (2.72 * color + 0.15));
	return pow(color, 1.0 / Gamma);
}

float3 FilmicToneMapping(float3 color)
{
	color = max(0.0, color - 0.004);
	return (color * (6.2 * color + 0.5)) / (color * (6.2 * color + 1.7) + 0.06);
}

float3 Uncharted2ToneMapping(float3 color)
{
	const float A = 0.15, B = 0.50, C = 0.10, D = 0.20, E = 0.02, F = 0.30, W = 11.2;
	color *= 2.0;
	color = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
	const float white = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
	color /= white;
	return pow(color, 1.0 / Gamma);
}

float3 ReshadeToneMap(float3 inputColor)
{
	float3 color = inputColor;
	color = clamp(color - defog * FogColor * 2.55, 0.0, 1.0);
	color *= Exposure / (1.0 + color / Exposure);
	color = pow(color, 1.0 / Gamma);

	const float lum = getL709(color);
	const float L = clamp(10.0 * (lum - 0.45), 0.0, 1.0);
	float3 A2 = Bleach * color;

	float3 result1 = 2.0 * color * lum;
	float3 result2 = 1.0 - 2.0 * (1.0 - lum) * (1.0 - color);

	float3 newColor = lerp(result1, result2, L);
	float3 mixRGB = A2 * newColor;
	color += ((1.0 - A2) * mixRGB);

	float3 middlegray = dot(color, float3(1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0));
	float3 diffcolor = color - middlegray;
	color = (color + diffcolor * sat) / (1.0 + diffcolor * sat);
	return color;
}

float3 BotWToneMap(float3 color)
{
	const float Lumn = getL601(color);
	const float4 exptm = 1.0 - exp(-float4(color, Lumn));
	const float3 cpre = exptm.w / Lumn * color;
	const float3 colorldr = lerp(cpre, exptm.rgb, pow(exptm.w, 2.0));
	return colorldr;
}

float3 ACESFilm(float3 color)
{
	color *= Exposure;
	const float Lumn = getL709(color);
	float4 tm = float4(color, Lumn);
	tm = (tm * (2.51 * tm + 0.03)) / (tm * (2.43 * tm + 0.59) + 0.14);
	const float3 cpre = tm.w / Lumn * color;
	const float3 colorldr = lerp(cpre, tm.rgb, pow(tm.w, 2.0));
	return colorldr;
}

// --- grading ---

float3 LevelsPass(float3 inputColor)
{
	const float black_point_float = BlackPoint / 255.0;
	const float white_point_float = (WhitePoint == BlackPoint) ? (255.0 / 0.00025) : (255.0 / (WhitePoint - BlackPoint));
	return inputColor * white_point_float - (black_point_float * white_point_float);
}

float3 ColorMatrixPass(float3 inputColor)
{
	const float3x3 ColorMatrix = float3x3(ColorMatrix_Red, ColorMatrix_Green, ColorMatrix_Blue);
	float3 color = inputColor;
	color *= lerp(color, mul(ColorMatrix, color), CM_Strength);
	return saturate(color);
}

float3 TechnicolorPass(float3 color)
{
	const float3 cyanfilter = float3(0.0, 1.30, 1.0);
	const float3 magentafilter = float3(1.0, 0.0, 1.05);
	const float3 yellowfilter = float3(1.6, 1.6, 0.05);
	const float2 redorangefilter = float2(1.05, 0.620);
	const float2 greenfilter = float2(0.30, 1.0);
	const float2 magentafilter2 = magentafilter.rb;

	float3 tcol = color;

	float2 negative_mul_r = tcol.rg * (1.0 / (RGBNegativeAmount.r * Power));
	float2 negative_mul_g = tcol.rg * (1.0 / (RGBNegativeAmount.g * Power));
	float2 negative_mul_b = tcol.rb * (1.0 / (RGBNegativeAmount.b * Power));
	float3 output_r = dot(redorangefilter, negative_mul_r) + cyanfilter;
	float3 output_g = dot(greenfilter, negative_mul_g) + magentafilter;
	float3 output_b = dot(magentafilter2, negative_mul_b) + yellowfilter;

	return lerp(tcol, output_r * output_g * output_b, Strength);
}

float3 Technicolor2Pass(float3 inputColor)
{
	float3 color = inputColor;
	const float3 Color_Strength = float3(Technicolor2_Red_Strength, Technicolor2_Green_Strength, Technicolor2_Blue_Strength);
	const float3 source = color;
	float3 temp = 1.0 - source;
	float3 target = temp.grg;
	float3 target2 = temp.bbr;
	float3 temp2 = source * target;
	temp2 *= target2;

	temp = temp2 * Color_Strength;
	temp2 *= Technicolor2_Brightness;

	target = temp.grg;
	target2 = temp.bbr;

	temp = source - target;
	temp += temp2;
	temp2 = temp - target2;

	color = lerp(source, temp2, Technicolor2_Strength);
	color = lerp(dot(color, float3(0.333, 0.333, 0.333)), color, Technicolor2_Saturation);
	return color;
}

float3 BotWVibrance(float3 color)
{
	const float avg = (color.r + color.g + color.b) / 3.0;
	const float maxc = max(color.r, max(color.g, color.b));
	const float w = 1.0 - pow(1.0 - 2.0 * avg, 2.0);
	const float weight = 1.0 + w * satFactor;
	return lerp(maxc, color, weight);
}

float3 ReshadeVibrance(float3 color)
{
	const float max_color = max(color.r, max(color.g, color.b));
	const float min_color = min(color.r, min(color.g, color.b));
	const float luma = getL709(color);
	const float color_saturation = max_color - min_color;
	const float3 coeffVibrance = VibranceRGBBalance * Vibrance;
	return lerp(luma, color, 1.0 + (coeffVibrance * (1.0 - (sign(coeffVibrance) * color_saturation))));
}

float3 ContrastyPass(float3 fColour)
{
	fColour = max(0.0, fColour - crushContrast);
	fColour = clamp(Exposure * fColour, 0.0, 1.0);
	fColour = pow(fColour, 1.0 / Gamma);
	return ReshadeVibrance(fColour);
}

float3 FilmPass(float3 inputColor)
{
	float3 B = inputColor;
	const float3 H = 0.0;

	B = saturate(B);
	B = pow(B, Linearization);
	B = lerp(H, B, Filmic_Contrast);

	const float3 LumCoeff = float3(0.2126, 0.7152, 0.0722);
	const float A = dot(B, LumCoeff);
	float3 D = A;

	B = pow(B, 1.0 / BaseGamma);

	const float a = 1.0, b = 1.0, c = 1.0, d = BaseCurve;

	const float y = 1.0 / (1.0 + exp(a / 2.0));
	const float z = 1.0 / (1.0 + exp(b / 2.0));
	const float w = 1.0 / (1.0 + exp(c / 2.0));
	const float v = 1.0 / (1.0 + exp(d / 2.0));

	float3 C = B;

	D.r = (1.0 / (1.0 + exp(-a * (D.r - 0.5))) - y) / (1.0 - 2.0 * y);
	D.g = (1.0 / (1.0 + exp(-b * (D.g - 0.5))) - z) / (1.0 - 2.0 * z);
	D.b = (1.0 / (1.0 + exp(-c * (D.b - 0.5))) - w) / (1.0 - 2.0 * w);

	D = pow(D, 1.0 / EffectGamma);

	const float3 Di = 1.0 - D;
	D = lerp(D, Di, Filmic_Bleach);

	D.r = pow(abs(D.r), 1.0 / 1.0);
	D.g = pow(abs(D.g), 1.0 / 1.0);
	D.b = pow(abs(D.b), 1.0 / 1.0);

	C.r = (D.r < 0.5) ? ((2.0 * D.r - 1.0) * (B.r - B.r * B.r) + B.r) : ((2.0 * D.r - 1.0) * (sqrt(B.r) - B.r) + B.r);
	C.g = (D.g < 0.5) ? ((2.0 * D.g - 1.0) * (B.g - B.g * B.g) + B.g) : ((2.0 * D.g - 1.0) * (sqrt(B.g) - B.g) + B.g);
	C.b = (D.b < 0.5) ? ((2.0 * D.b - 1.0) * (B.b - B.b * B.b) + B.b) : ((2.0 * D.b - 1.0) * (sqrt(B.b) - B.b) + B.b);

	float3 F = lerp(B, C, Filmic_Strength);

	F = (1.0 / (1.0 + exp(-d * (F - 0.5))) - v) / (1.0 - 2.0 * v);

	const float r2R = 1.0 - Saturation;
	const float g2R = 0.0 + Saturation;
	const float b2R = 0.0 + Saturation;
	const float r2G = 0.0 + Saturation;
	const float g2G = (1.0 - Fade) - Saturation;
	const float b2G = (0.0 + Fade) + Saturation;
	const float r2B = 0.0 + Saturation;
	const float g2B = (0.0 + Fade) + Saturation;
	const float b2B = (1.0 - Fade) - Saturation;

	const float3 iF = F;
	F.r = (iF.r * r2R + iF.g * g2R + iF.b * b2R);
	F.g = (iF.r * r2G + iF.g * g2G + iF.b * b2G);
	F.b = (iF.r * r2B + iF.g * g2B + iF.b * b2B);

	const float N = dot(F, LumCoeff);
	float3 Cn = F;
	Cn = (N < 0.5) ? ((2.0 * N - 1.0) * (F - F * F) + F) : ((2.0 * N - 1.0) * (sqrt(F) - F) + F);

	Cn = pow(max(Cn, 0.0), 1.0 / Linearization);

	const float3 Fn = lerp(B, Cn, Filmic_Strength);
	return Fn;
}

static const float3x3 kDpxRGB = float3x3(
	2.6714711726599600, -1.2672360578624100, -0.4109956021722270,
	-1.0251070293466400, 1.9840911624108900, 0.0439502493584124,
	0.0610009456429445, -0.2236707508128630, 1.1590210416706100);
static const float3x3 kDpxXYZ = float3x3(
	0.5003033835433160, 0.3380975732227390, 0.1645897795458570,
	0.2579688942747580, 0.6761952591447060, 0.0658358459823868,
	0.0234517888692628, 0.1126992737203000, 0.8668396731242010);

float3 DPXPass(float3 inputColor)
{
	float3 color = inputColor;
	float3 B = color;
	B = B * (1.0 - DPX_Contrast) + (0.5 * DPX_Contrast);
	const float3 Btemp = (1.0 / (1.0 + exp(RGB_Curve / 2.0)));
	B = ((1.0 / (1.0 + exp(-RGB_Curve * (B - RGB_C))) / (-2.0 * Btemp + 1.0)) + (-Btemp / (-2.0 * Btemp + 1.0)));

	const float value = max(max(B.r, B.g), B.b);
	const float3 Ztemp = B / value;
	color = pow(abs(color), 1.0 / Colorfulness);

	float3 c0 = Ztemp * value;
	c0 = mul(kDpxXYZ, c0);
	const float luma = dot(c0, float3(0.30, 0.59, 0.11));
	c0 = (1.0 - DPX_Saturation) * luma + DPX_Saturation * c0;
	c0 = mul(kDpxRGB, c0);
	return lerp(color, c0, DPX_Strength);
}

float3 LiftGammaGainPass(float3 inputColor)
{
	float3 color = inputColor;
	color = color * (1.5 - 0.5 * RGB_Lift) + 0.5 * RGB_Lift - 0.5;
	color = max(0.0, color);
	color = pow(color, 1.0 / RGB_Gamma);
	color = color * RGB_Gain;
	return color;
}

float3 CurvesPass(float3 inputColor)
{
	float3 colorInput = inputColor;
	const float Contrast_blend = Contrast * 1.5;
	float3 x = colorInput;
	x = x - 0.5;
	x = (x / (0.5 + abs(x))) + 0.5;
	colorInput = lerp(colorInput, x, Contrast_blend);
	return colorInput;
}

float3 VibrancePass(float3 color)
{
	const float maxc = max(color.r, max(color.g, color.b));
	const float minc = min(color.r, min(color.g, color.b));
	const float luma = getL709(color);
	const float color_saturation = maxc - minc;
	const float3 coeffVibrance = VibranceRGBBalance * Vibrance;
	return lerp(luma, color, 1.0 + (coeffVibrance * (1.0 - (sign(coeffVibrance) * color_saturation))));
}

// --- bloom and sharpening from the picture itself ---

float3 BloomPass(float3 color, float2 uv)
{
	const float2 px = ReShade::PixelSize;
	float3 bloom_sum1 = 0.0;
	float3 bloom_sum2 = 0.0;

	bloom_sum1 += tex2D(ReShade::BackBuffer, uv + float2(0.0, -1.0) * radius1 * px).rgb;
	bloom_sum1 += tex2D(ReShade::BackBuffer, uv + float2(0.0, 1.0) * radius1 * px).rgb;
	bloom_sum1 += tex2D(ReShade::BackBuffer, uv + float2(-1.0, 0.0) * radius1 * px).rgb;
	bloom_sum1 += tex2D(ReShade::BackBuffer, uv + float2(1.0, 0.0) * radius1 * px).rgb;
	bloom_sum1 *= 0.25;

	bloom_sum2 += tex2D(ReShade::BackBuffer, uv + float2(0.0, -2.5) * radius2 * px).rgb;
	bloom_sum2 += tex2D(ReShade::BackBuffer, uv + float2(0.0, 2.5) * radius2 * px).rgb;
	bloom_sum2 += tex2D(ReShade::BackBuffer, uv + float2(-2.5, 0.0) * radius2 * px).rgb;
	bloom_sum2 += tex2D(ReShade::BackBuffer, uv + float2(2.5, 0.0) * radius2 * px).rgb;
	bloom_sum2 *= 0.010;

	const float dist = radius2 - radius1;
	const float3 HDR = (color + (bloom_sum2 - bloom_sum1)) * dist;

	return HDR + color;
}

float3 LumaSharpenPass(float2 uv)
{
	const float2 px = ReShade::PixelSize;
	const float3 c = tex2D(ReShade::BackBuffer, uv).rgb;
	const float3 north = tex2D(ReShade::BackBuffer, uv + float2(0.0, -offset_bias) * px).rgb;
	const float3 south = tex2D(ReShade::BackBuffer, uv + float2(0.0, offset_bias) * px).rgb;
	const float3 west = tex2D(ReShade::BackBuffer, uv + float2(-offset_bias, 0.0) * px).rgb;
	const float3 east = tex2D(ReShade::BackBuffer, uv + float2(offset_bias, 0.0) * px).rgb;

	const float3 minRGB = min(min(min(north, south), min(west, east)), c);
	const float3 maxRGB = max(max(max(north, south), max(west, east)), c);

	const float3 blur = (north + south + west + east) * 0.25;
	const float3 sharp = c + (c - blur) * sharp_strength;
	return clamp(sharp, minRGB - sharp_clamp, maxRGB + sharp_clamp);
}

// ---------------------------------------------------------------------------------------------------
// the pass
// ---------------------------------------------------------------------------------------------------

float3 PS_Clarity(float4 pos : SV_Position, float2 uv : TEXCOORD) : SV_Target
{
	float3 color = tex2D(ReShade::BackBuffer, uv).rgb;

	if (adjust_bloom == 1)
		color = BloomPass(color, uv);

	if (lumapassing == 1)
		color = LumaSharpenPass(uv);

	// tone mapping
	if (Tone_map == -1)
		color = clamp(color * Exposure, 0.0, 1.0);
	else if (Tone_map == 0)
		color = BotWToneMap(color);
	else if (Tone_map == 1)
		color = ReshadeToneMap(color);
	else if (Tone_map == 2)
		color = LinearToneMapping(color);
	else if (Tone_map == 3)
		color = SimpleReinhardToneMapping(color);
	else if (Tone_map == 4)
		color = LumaBasedReinhardToneMapping(color);
	else if (Tone_map == 5)
		color = WhitePreservingLumaBasedReinhardToneMapping(color);
	else if (Tone_map == 6)
		color = RomBinDaHouseToneMapping(color);
	else if (Tone_map == 7)
		color = FilmicToneMapping(color);
	else if (Tone_map == 8)
		color = Uncharted2ToneMapping(color);
	else if (Tone_map == 9)
		color = ACESFilm(color);

	if (blacknwhitepass == 1)
		color = LevelsPass(color);

	if (cmatrix == 1)
		color = ColorMatrixPass(color);

	if (Tech == 1)
		color = TechnicolorPass(color);

	if (Techine == 1)
		color = Technicolor2Pass(color);

	if (post_process == 0)
		color = BotWVibrance(color);
	else if (post_process == 1)
		color = ContrastyPass(color);

	if (dpxpass == 1)
		color = DPXPass(color);

	if (Filmicpass == 1)
		color = FilmPass(color);

	if (lggpass == 1)
		color = LiftGammaGainPass(color);

	if (CurvesPss == 1)
		color = CurvesPass(color);

	if (vibpass == 1)
		color = VibrancePass(color);

	return saturate(color);
}

technique Clarity <
	ui_tooltip = "The picture is graded like the Clarity presets of the graphic pack, but a change of preset is there at once.";
>
{
	pass
	{
		VertexShader = PostProcessVS;
		PixelShader = PS_Clarity;
	}
}