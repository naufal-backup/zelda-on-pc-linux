// AMD FSR 1.0 (EASU + RCAS) untuk Zelda on PC. Lihat VulkanFSR1.h.
//
// EASU (Edge Adaptive Spatial Upsampling) memperbesar gambar: untuk setiap piksel keluaran ia
// mengambil 12 piksel di sekitarnya, menentukan arah tepi, lalu memakai lobus Lanczos dengan
// jendela yang menyesuaikan (lebih panjang pada diagonal). RCAS (Robust Contrast Adaptive
// Sharpening) menajamkan hasilnya dengan batas agar tidak muncul lingkaran cahaya.
//
// Matematikanya dari ffx_fsr1.h (AMD FidelityFX SDK, lisensi MIT):
//   https://github.com/GPUOpen-Effects/FidelityFX-FSR
// Kedua shader di bawah adalah terjemahan langsung dari FsrEasuF/FsrRcasF ke GLSL.

#include "Cafe/HW/Latte/Renderer/Vulkan/VulkanFSR1.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>

#include "Cafe/HW/Latte/Renderer/Vulkan/RendererShaderVk.h"
#include "Cemu/Logging/CemuLogging.h"

// RendererShaderVk.cpp: GLSL mandiri -> SPIR-V (tahap 2 = compute)
std::vector<uint32> RendererShaderVk_CompileCompute(const std::string& source, std::string& errorOut);

namespace
{
	// EASU: satu piksel keluaran per invocation.
	//   input  : textureSrc (binding 0) - gambar game yang lebih kecil
	//   output : dstImg (binding 1)     - gambar yang diperbesar
	//   constants: con0..con3 dari FsrEasuCon (dihitung di CPU, dikirim sebagai push constant)
	const char* kEasuShader = R"(
#version 450
layout(local_size_x = 8, local_size_y = 8) in;

layout(set = 0, binding = 0) uniform sampler2D srcTex;
layout(set = 0, binding = 1, rgba16f) uniform writeonly image2D dstImg;

layout(push_constant) uniform Push {
	vec4 con0;   // x,y = skala masuk->keluar, z,w = pergeseran setengah piksel
	vec4 con1;   // xy = 1/ukuran sumber, zw = offset gather pertama
	vec4 con2;   // offset gather kedua dan ketiga
	vec4 con3;   // offset gather terakhir
	ivec4 size;  // xy = ukuran keluaran
} pc;

// pengganti APrxLoRcpF1 (perkiraan 1/x)
float rcp(float v) { return 1.0 / v; }
float rcpSqrt(float v) { return inversesqrt(v); }
float saturate(float v) { return clamp(v, 0.0, 1.0); }

// satu tap dari inti 12 piksel
void easuTap(inout vec3 aC, inout float aW, vec2 off, vec2 dir, vec2 len, float lob, float clp, vec3 c)
{
	vec2 v;
	v.x = off.x * dir.x + off.y * dir.y;
	v.y = off.x * (-dir.y) + off.y * dir.x;
	v *= len;
	float d2 = v.x * v.x + v.y * v.y;
	d2 = min(d2, clp);
	// Lanczos2 tanpa sin(): jendela kali basis
	float wB = (2.0 / 5.0) * d2 - 1.0;
	float wA = lob * d2 - 1.0;
	wB *= wB;
	wA *= wA;
	wB = (25.0 / 16.0) * wB - (25.0 / 16.0 - 1.0);
	float w = wB * wA;
	aC += c * w;
	aW += w;
}

// menambah arah gradien dan panjangnya untuk satu sudut bilinear
void easuSet(inout vec2 dir, inout float len, vec2 uv, bool biS, bool biT, bool biU, bool biV,
			 float lA, float lB, float lC, float lD, float lE)
{
	float w = 0.0;
	if (biS) w = (1.0 - uv.x) * (1.0 - uv.y);
	if (biT) w = uv.x * (1.0 - uv.y);
	if (biU) w = (1.0 - uv.x) * uv.y;
	if (biV) w = uv.x * uv.y;

	float dc = lD - lC;
	float cb = lC - lB;
	float lenX = max(abs(dc), abs(cb));
	lenX = rcp(lenX);
	float dirX = lD - lB;
	dir.x += dirX * w;
	lenX = saturate(abs(dirX) * lenX);
	lenX *= lenX;
	len += lenX * w;

	float ec = lE - lC;
	float ca = lC - lA;
	float lenY = max(abs(ec), abs(ca));
	lenY = rcp(lenY);
	float dirY = lE - lA;
	dir.y += dirY * w;
	lenY = saturate(abs(dirY) * lenY);
	lenY *= lenY;
	len += lenY * w;
}

void main()
{
	ivec2 ip = ivec2(gl_GlobalInvocationID.xy);
	if (ip.x >= pc.size.x || ip.y >= pc.size.y)
		return;

	// posisi di gambar sumber (dalam piksel)
	vec2 pp = vec2(ip) * pc.con0.xy + pc.con0.zw;
	vec2 fp = floor(pp);
	pp -= fp;

	// inti 12 piksel, dikumpulkan dalam 4 tap berisi 4 piksel
	vec2 p0 = fp * pc.con1.xy + pc.con1.zw;
	vec2 p1 = p0 + pc.con2.xy;
	vec2 p2 = p0 + pc.con2.zw;
	vec2 p3 = p0 + pc.con3.xy;

	vec4 bczzR = textureLod(srcTex, p0, 0.0);
	vec4 ijfeR = textureLod(srcTex, p1, 0.0);
	vec4 klhgR = textureLod(srcTex, p2, 0.0);
	vec4 zzonR = textureLod(srcTex, p3, 0.0);

	// luma kali 2 (perkiraan murah, sama seperti aslinya), dihitung per komponen
	vec4 bczzL = bczzR.bbrg * vec4(0.5, 0.5, 0.5, 0.5) + (bczzR.rrgg * vec4(0.5, 0.5, 0.5, 0.5) + bczzR.ggbb);
	vec4 ijfeL = ijfeR.bbrg * vec4(0.5, 0.5, 0.5, 0.5) + (ijfeR.rrgg * vec4(0.5, 0.5, 0.5, 0.5) + ijfeR.ggbb);
	vec4 klhgL = klhgR.bbrg * vec4(0.5, 0.5, 0.5, 0.5) + (klhgR.rrgg * vec4(0.5, 0.5, 0.5, 0.5) + klhgR.ggbb);
	vec4 zzonL = zzonR.bbrg * vec4(0.5, 0.5, 0.5, 0.5) + (zzonR.rrgg * vec4(0.5, 0.5, 0.5, 0.5) + zzonR.ggbb);

	float bL = bczzL.x, cL = bczzL.y;
	float iL = ijfeL.x, jL = ijfeL.y, fL = ijfeL.z, eL = ijfeL.w;
	float kL = klhgL.x, lL = klhgL.y, hL = klhgL.z, gL = klhgL.w;
	float oL = zzonL.z, nL = zzonL.w;

	vec2 dir = vec2(0.0);
	float len = 0.0;
	easuSet(dir, len, pp, true,  false, false, false, bL, eL, fL, gL, jL);
	easuSet(dir, len, pp, false, true,  false, false, cL, fL, gL, hL, kL);
	easuSet(dir, len, pp, false, false, true,  false, fL, iL, jL, kL, nL);
	easuSet(dir, len, pp, false, false, false, true,  gL, jL, kL, lL, oL);

	// normalisasi arah
	vec2 dir2 = dir * dir;
	float dirR = dir2.x + dir2.y;
	bool zro = dirR < (1.0 / 32768.0);
	dirR = rcpSqrt(dirR);
	dirR = zro ? 1.0 : dirR;
	dir.x = zro ? 1.0 : dir.x;
	dir *= dirR;

	// panjang 0..1, dibentuk kuadrat
	len = len * 0.5;
	len *= len;

	// jendela memanjang ke arah diagonal
	float stretch = (dir.x * dir.x + dir.y * dir.y) * rcp(max(abs(dir.x), abs(dir.y)));
	vec2 len2 = vec2(1.0 + (stretch - 1.0) * len, 1.0 + (-0.5) * len);
	float lob = 0.5 + ((1.0 / 4.0 - 0.04) - 0.5) * len;
	float clp = rcp(lob);

	// batas dari 4 piksel terdekat
	vec3 min4 = min(min(vec3(ijfeR.z, ijfeR.y, ijfeR.x), vec3(klhgR.w, klhgR.z, klhgR.y)),
					vec3(ijfeR.x, ijfeR.y, ijfeR.y));
	vec3 max4 = max(max(vec3(ijfeR.z, ijfeR.y, ijfeR.x), vec3(klhgR.w, klhgR.z, klhgR.y)),
					vec3(ijfeR.x, ijfeR.y, ijfeR.y));

	vec3 aC = vec3(0.0);
	float aW = 0.0;
	easuTap(aC, aW, vec2( 0.0, -1.0) - pp, dir, len2, lob, clp, bczzR.xyz); // b
	easuTap(aC, aW, vec2( 1.0, -1.0) - pp, dir, len2, lob, clp, bczzR.yzw); // c
	easuTap(aC, aW, vec2(-1.0,  1.0) - pp, dir, len2, lob, clp, ijfeR.xyz); // i
	easuTap(aC, aW, vec2( 0.0,  1.0) - pp, dir, len2, lob, clp, ijfeR.yzw); // j
	easuTap(aC, aW, vec2( 0.0,  0.0) - pp, dir, len2, lob, clp, ijfeR.zwx); // f
	easuTap(aC, aW, vec2(-1.0,  0.0) - pp, dir, len2, lob, clp, ijfeR.wxy); // e
	easuTap(aC, aW, vec2( 1.0,  1.0) - pp, dir, len2, lob, clp, klhgR.xyz); // k
	easuTap(aC, aW, vec2( 2.0,  1.0) - pp, dir, len2, lob, clp, klhgR.yzw); // l
	easuTap(aC, aW, vec2( 2.0,  0.0) - pp, dir, len2, lob, clp, klhgR.zwx); // h
	easuTap(aC, aW, vec2( 1.0,  0.0) - pp, dir, len2, lob, clp, klhgR.wxy); // g
	easuTap(aC, aW, vec2( 1.0,  2.0) - pp, dir, len2, lob, clp, zzonR.zwx); // o
	easuTap(aC, aW, vec2( 0.0,  2.0) - pp, dir, len2, lob, clp, zzonR.wxy); // n

	// normalisasi, lalu jepit di antara min4 dan max4 (mencegah dering)
	vec3 pix = min(max4, max(min4, aC * rcp(aW)));
	imageStore(dstImg, ip, vec4(pix, 1.0));
}
)";

	// RCAS: penajaman dengan batas kontras.
	const char* kRcasShader = R"(
#version 450
layout(local_size_x = 8, local_size_y = 8) in;

layout(set = 0, binding = 0) uniform sampler2D srcTex;
layout(set = 0, binding = 1, rgba16f) uniform writeonly image2D dstImg;

layout(push_constant) uniform Push {
	vec4 con;    // x = ketajaman linear (2^-sharpness)
	ivec4 size;  // xy = ukuran gambar
} pc;

const float kLimit = 0.25 - 1.0 / 16.0;

float saturate(float v) { return clamp(v, 0.0, 1.0); }

vec3 loadPixel(ivec2 p)
{
	ivec2 sz = ivec2(pc.size.xy);
	p = clamp(p, ivec2(0), sz - ivec2(1));
	return texelFetch(srcTex, p, 0).rgb;
}

void main()
{
	ivec2 ip = ivec2(gl_GlobalInvocationID.xy);
	if (ip.x >= pc.size.x || ip.y >= pc.size.y)
		return;

	// lingkungan 3x3 minimal
	vec3 b = loadPixel(ip + ivec2( 0, -1));
	vec3 d = loadPixel(ip + ivec2(-1,  0));
	vec3 e = loadPixel(ip);
	vec3 f = loadPixel(ip + ivec2( 1,  0));
	vec3 h = loadPixel(ip + ivec2( 0,  1));

	// luma kali 2
	float bL = b.b * 0.5 + (b.r * 0.5 + b.g);
	float dL = d.b * 0.5 + (d.r * 0.5 + d.g);
	float eL = e.b * 0.5 + (e.r * 0.5 + e.g);
	float fL = f.b * 0.5 + (f.r * 0.5 + f.g);
	float hL = h.b * 0.5 + (h.r * 0.5 + h.g);

	// deteksi derau
	float nz = 0.25 * bL + 0.25 * dL + 0.25 * fL + 0.25 * hL - eL;
	float mx = max(max(max(bL, dL), eL), max(fL, hL));
	float mn = min(min(min(bL, dL), eL), min(fL, hL));
	nz = saturate(abs(nz) * (1.0 / max(mx - mn, 1e-6)));
	nz = -0.5 * nz + 1.0;

	// batas bawah dan atas
	float mnR = min(min(b.r, d.r), min(f.r, h.r));
	float mnG = min(min(b.g, d.g), min(f.g, h.g));
	float mnB = min(min(b.b, d.b), min(f.b, h.b));
	float mxR = max(max(b.r, d.r), max(f.r, h.r));
	float mxG = max(max(b.g, d.g), max(f.g, h.g));
	float mxB = max(max(b.b, d.b), max(f.b, h.b));

	// pengatur agar tidak melewati rentang tetangga
	float hitMinR = min(mnR, e.r) / (4.0 * mxR);
	float hitMinG = min(mnG, e.g) / (4.0 * mxG);
	float hitMinB = min(mnB, e.b) / (4.0 * mxB);
	float hitMaxR = (1.0 - max(mxR, e.r)) / (4.0 * mnR - 4.0);
	float hitMaxG = (1.0 - max(mxG, e.g)) / (4.0 * mnG - 4.0);
	float hitMaxB = (1.0 - max(mxB, e.b)) / (4.0 * mnB - 4.0);

	float lobeR = max(-hitMinR, hitMaxR);
	float lobeG = max(-hitMinG, hitMaxG);
	float lobeB = max(-hitMinB, hitMaxB);
	float lobe = max(-kLimit, min(max(lobeR, max(lobeG, lobeB)), 0.0)) * pc.con.x;

	// hilangkan derau
	lobe *= nz;

	// hasil akhir
	float rcpL = 1.0 / (4.0 * lobe + 1.0);
	vec3 pix = (lobe * b + lobe * d + lobe * h + lobe * f + e) * rcpL;
	imageStore(dstImg, ip, vec4(pix, 1.0));
}
)";

	// FsrEasuCon dari ffx_fsr1.h, dihitung di CPU.
	struct EasuConstants
	{
		float con0[4];
		float con1[4];
		float con2[4];
		float con3[4];
	};

	EasuConstants FsrEasuCon(float srcW, float srcH, float dstW, float dstH)
	{
		EasuConstants c{};
		const float rcpDstX = 1.0f / dstW;
		const float rcpDstY = 1.0f / dstH;
		const float rcpSrcX = 1.0f / srcW;
		const float rcpSrcY = 1.0f / srcH;

		c.con0[0] = srcW * rcpDstX;
		c.con0[1] = srcH * rcpDstY;
		c.con0[2] = 0.5f * srcW * rcpDstX - 0.5f;
		c.con0[3] = 0.5f * srcH * rcpDstY - 0.5f;

		c.con1[0] = rcpSrcX;
		c.con1[1] = rcpSrcY;
		c.con1[2] = 1.0f * rcpSrcX;
		c.con1[3] = -1.0f * rcpSrcY;

		c.con2[0] = -1.0f * rcpSrcX;
		c.con2[1] = 2.0f * rcpSrcY;
		c.con2[2] = 1.0f * rcpSrcX;
		c.con2[3] = 2.0f * rcpSrcY;

		c.con3[0] = 0.0f;
		c.con3[1] = 4.0f * rcpSrcY;
		c.con3[2] = 0.0f;
		c.con3[3] = 0.0f;
		return c;
	}
}

VulkanFSR1::VulkanFSR1(VkDevice device, VkPhysicalDevice physicalDevice)
	: m_device(device), m_physicalDevice(physicalDevice)
{
}

VulkanFSR1::~VulkanFSR1()
{
	if (m_device == VK_NULL_HANDLE)
		return;
	vkDeviceWaitIdle(m_device);
	DestroyImage(m_easu);
	DestroyImage(m_output);
	if (m_easuPipeline)
		vkDestroyPipeline(m_device, m_easuPipeline, nullptr);
	if (m_rcasPipeline)
		vkDestroyPipeline(m_device, m_rcasPipeline, nullptr);
	if (m_pipelineLayout)
		vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
	if (m_setLayout)
		vkDestroyDescriptorSetLayout(m_device, m_setLayout, nullptr);
	if (m_pool)
		vkDestroyDescriptorPool(m_device, m_pool, nullptr);
	if (m_sampler)
		vkDestroySampler(m_device, m_sampler, nullptr);
}

bool VulkanFSR1::IsSupported(VkPhysicalDevice physicalDevice)
{
	return physicalDevice != VK_NULL_HANDLE;
}

uint32 VulkanFSR1::FindMemoryType(uint32 bits, VkMemoryPropertyFlags want)
{
	VkPhysicalDeviceMemoryProperties mp{};
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &mp);
	for (uint32 i = 0; i < mp.memoryTypeCount; i++)
	{
		if ((bits & (1u << i)) && (mp.memoryTypes[i].propertyFlags & want) == want)
			return i;
	}
	return UINT32_MAX;
}

VkFormat VulkanFSR1::PickFormat(VkFormat preferred)
{
	if (preferred == VK_FORMAT_UNDEFINED)
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	VkFormatProperties props{};
	vkGetPhysicalDeviceFormatProperties(m_physicalDevice, preferred, &props);
	if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)
		return preferred;
	// fallback: format 8 bit yang selalu bisa dipakai sebagai storage image
	return VK_FORMAT_R8G8B8A8_UNORM;
}

void VulkanFSR1::DestroyImage(Image& img)
{
	if (img.view)
		vkDestroyImageView(m_device, img.view, nullptr);
	if (img.image)
		vkDestroyImage(m_device, img.image, nullptr);
	if (img.memory)
		vkFreeMemory(m_device, img.memory, nullptr);
	img = Image{};
}

bool VulkanFSR1::CreateImage(Image& img, uint32 w, uint32 h, VkFormat format, VkImageUsageFlags usage)
{
	VkImageCreateInfo ii{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
	ii.imageType = VK_IMAGE_TYPE_2D;
	ii.format = format;
	ii.extent = {w, h, 1};
	ii.mipLevels = 1;
	ii.arrayLayers = 1;
	ii.samples = VK_SAMPLE_COUNT_1_BIT;
	ii.tiling = VK_IMAGE_TILING_OPTIMAL;
	ii.usage = usage;
	ii.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	// gambar ditulis oleh shader dan dibaca sebagai tekstur: GENERAL cocok untuk keduanya
	ii.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
	if (vkCreateImage(m_device, &ii, nullptr, &img.image) != VK_SUCCESS)
		return false;

	VkMemoryRequirements req{};
	vkGetImageMemoryRequirements(m_device, img.image, &req);
	const uint32 type = FindMemoryType(req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (type == UINT32_MAX)
		return false;

	VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
	ai.allocationSize = req.size;
	ai.memoryTypeIndex = type;
	if (vkAllocateMemory(m_device, &ai, nullptr, &img.memory) != VK_SUCCESS)
		return false;
	if (vkBindImageMemory(m_device, img.image, img.memory, 0) != VK_SUCCESS)
		return false;

	VkImageViewCreateInfo vi{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
	vi.image = img.image;
	vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
	vi.format = format;
	vi.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
	if (vkCreateImageView(m_device, &vi, nullptr, &img.view) != VK_SUCCESS)
		return false;

	img.width = w;
	img.height = h;
	return true;
}

bool VulkanFSR1::EnsureImages(uint32 width, uint32 height)
{
	if (m_imageFailed)
		return false;
	if (m_output.width == width && m_output.height == height && m_output.image && m_easu.image)
		return true;

	DestroyImage(m_easu);
	DestroyImage(m_output);
	if (m_easuSet)
	{
		vkFreeDescriptorSets(m_device, m_pool, 1, &m_easuSet);
		m_easuSet = VK_NULL_HANDLE;
	}
	if (m_rcasSet)
	{
		vkFreeDescriptorSets(m_device, m_pool, 1, &m_rcasSet);
		m_rcasSet = VK_NULL_HANDLE;
	}

	const VkImageUsageFlags usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	if (!CreateImage(m_output, width, height, m_format, usage) || !CreateImage(m_easu, width, height, m_format, usage))
	{
		cemuLog_log(LogType::Force, "FSR 1.0: gambar tidak bisa dibuat");
		DestroyImage(m_easu);
		DestroyImage(m_output);
		m_imageFailed = true;
		return false;
	}
	return true;
}

bool VulkanFSR1::EnsurePipelines()
{
	if (m_pipelineFailed)
		return false;
	if (m_easuPipeline && m_rcasPipeline)
		return true;

	m_format = PickFormat(VK_FORMAT_R16G16B16A16_SFLOAT);
	const char* formatName = (m_format == VK_FORMAT_R16G16B16A16_SFLOAT) ? "rgba16f" : "rgba8";

	// format gambar adalah bagian dari shader (kualifikasi storage image)
	std::string easuSource(kEasuShader);
	std::string rcasSource(kRcasShader);
	for (auto* src : {&easuSource, &rcasSource})
	{
		const size_t pos = src->find("rgba16f) uniform writeonly image2D dstImg");
		if (pos != std::string::npos)
			src->replace(pos, 6, formatName);
	}

	std::string err;
	auto easuSpirv = RendererShaderVk_CompileCompute(easuSource, err);
	if (easuSpirv.empty())
	{
		cemuLog_log(LogType::Force, "FSR 1.0: shader EASU gagal: {}", err);
		m_pipelineFailed = true;
		return false;
	}
	auto rcasSpirv = RendererShaderVk_CompileCompute(rcasSource, err);
	if (rcasSpirv.empty())
	{
		cemuLog_log(LogType::Force, "FSR 1.0: shader RCAS gagal: {}", err);
		m_pipelineFailed = true;
		return false;
	}

	// deskriptor: 0 = tekstur sumber, 1 = gambar tujuan
	VkDescriptorSetLayoutBinding bindings[2]{};
	bindings[0].binding = 0;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[0].descriptorCount = 1;
	bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	bindings[1].binding = 1;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[1].descriptorCount = 1;
	bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	VkDescriptorSetLayoutCreateInfo li{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
	li.bindingCount = 2;
	li.pBindings = bindings;
	if (vkCreateDescriptorSetLayout(m_device, &li, nullptr, &m_setLayout) != VK_SUCCESS)
	{
		m_pipelineFailed = true;
		return false;
	}

	// push constant: 4 vec4 (konstanta) + ivec4 (ukuran) untuk EASU; RCAS memakai 5 vec4
	VkPushConstantRange range{};
	range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	range.offset = 0;
	range.size = 4 * 4 * sizeof(float) + 4 * sizeof(int32_t);

	VkPipelineLayoutCreateInfo pli{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
	pli.setLayoutCount = 1;
	pli.pSetLayouts = &m_setLayout;
	pli.pushConstantRangeCount = 1;
	pli.pPushConstantRanges = &range;
	if (vkCreatePipelineLayout(m_device, &pli, nullptr, &m_pipelineLayout) != VK_SUCCESS)
	{
		m_pipelineFailed = true;
		return false;
	}

	auto createPipeline = [&](const std::vector<uint32>& spirv, VkPipeline& out) {
		VkShaderModuleCreateInfo smi{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
		smi.codeSize = spirv.size() * sizeof(uint32);
		smi.pCode = spirv.data();
		VkShaderModule module = VK_NULL_HANDLE;
		if (vkCreateShaderModule(m_device, &smi, nullptr, &module) != VK_SUCCESS)
			return false;

		VkComputePipelineCreateInfo ci{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
		ci.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		ci.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		ci.stage.module = module;
		ci.stage.pName = "main";
		ci.layout = m_pipelineLayout;
		const VkResult r = vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &ci, nullptr, &out);
		vkDestroyShaderModule(m_device, module, nullptr);
		return r == VK_SUCCESS;
	};

	if (!createPipeline(easuSpirv, m_easuPipeline) || !createPipeline(rcasSpirv, m_rcasPipeline))
	{
		cemuLog_log(LogType::Force, "FSR 1.0: pipeline tidak bisa dibuat");
		m_pipelineFailed = true;
		return false;
	}

	// descriptor pool (2 set)
	VkDescriptorPoolSize sizes[2]{};
	sizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sizes[0].descriptorCount = 2;
	sizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	sizes[1].descriptorCount = 2;
	VkDescriptorPoolCreateInfo dpi{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
	dpi.maxSets = 2;
	dpi.poolSizeCount = 2;
	dpi.pPoolSizes = sizes;
	if (vkCreateDescriptorPool(m_device, &dpi, nullptr, &m_pool) != VK_SUCCESS)
	{
		m_pipelineFailed = true;
		return false;
	}

	// sampler linear, tanpa mip
	VkSamplerCreateInfo sci{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
	sci.magFilter = VK_FILTER_LINEAR;
	sci.minFilter = VK_FILTER_LINEAR;
	sci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	sci.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sci.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sci.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sci.maxLod = 0.0f;
	if (vkCreateSampler(m_device, &sci, nullptr, &m_sampler) != VK_SUCCESS)
	{
		m_pipelineFailed = true;
		return false;
	}
	return true;
}

VkImageView VulkanFSR1::Upscale(VkCommandBuffer cmd, VkImageView srcView, VkImageLayout srcLayout, uint32 srcW, uint32 srcH,
								uint32 dstW, uint32 dstH, float sharpness, bool sharpen)
{
	if (srcView == VK_NULL_HANDLE || srcW == 0 || srcH == 0 || dstW == 0 || dstH == 0)
		return VK_NULL_HANDLE;
	// tidak ada yang diperbesar
	if (srcW >= dstW && srcH >= dstH)
		return VK_NULL_HANDLE;
	if (!EnsurePipelines() || !EnsureImages(dstW, dstH))
		return VK_NULL_HANDLE;

	VkDescriptorSetAllocateInfo ai{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
	ai.descriptorPool = m_pool;
	ai.descriptorSetCount = 1;
	ai.pSetLayouts = &m_setLayout;
	if (!m_easuSet)
		vkAllocateDescriptorSets(m_device, &ai, &m_easuSet);
	if (!m_rcasSet)
		vkAllocateDescriptorSets(m_device, &ai, &m_rcasSet);
	if (!m_easuSet || !m_rcasSet)
		return VK_NULL_HANDLE;

	const bool withRcas = sharpen;
	Image& easuTarget = withRcas ? m_easu : m_output;

	// ---- tahap 1: EASU (sumber -> gambar besar) ----
	{
		VkDescriptorImageInfo ii[2]{};
		ii[0] = {m_sampler, srcView, srcLayout};
		ii[1] = {VK_NULL_HANDLE, easuTarget.view, VK_IMAGE_LAYOUT_GENERAL};

		VkWriteDescriptorSet w[2]{};
		w[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		w[0].dstSet = m_easuSet;
		w[0].dstBinding = 0;
		w[0].descriptorCount = 1;
		w[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		w[0].pImageInfo = &ii[0];
		w[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		w[1].dstSet = m_easuSet;
		w[1].dstBinding = 1;
		w[1].descriptorCount = 1;
		w[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		w[1].pImageInfo = &ii[1];
		vkUpdateDescriptorSets(m_device, 2, w, 0, nullptr);

		// gambar tujuan harus dalam layout GENERAL
		VkImageMemoryBarrier b{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
		b.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
		b.newLayout = VK_IMAGE_LAYOUT_GENERAL;
		b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		b.image = easuTarget.image;
		b.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
		b.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		b.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
							 0, nullptr, 0, nullptr, 1, &b);

		const EasuConstants c = FsrEasuCon((float)srcW, (float)srcH, (float)dstW, (float)dstH);
		struct
		{
			float con0[4], con1[4], con2[4], con3[4];
			int32_t size[4];
		} push{};
		std::memcpy(push.con0, c.con0, sizeof(push.con0));
		std::memcpy(push.con1, c.con1, sizeof(push.con1));
		std::memcpy(push.con2, c.con2, sizeof(push.con2));
		std::memcpy(push.con3, c.con3, sizeof(push.con3));
		push.size[0] = (int32_t)dstW;
		push.size[1] = (int32_t)dstH;

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_easuPipeline);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 0, 1, &m_easuSet, 0, nullptr);
		vkCmdPushConstants(cmd, m_pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(push), &push);
		vkCmdDispatch(cmd, (dstW + 7) / 8, (dstH + 7) / 8, 1);
	}

	if (!withRcas)
		return m_output.view;

	// ---- tahap 2: RCAS (gambar besar -> gambar besar yang tajam) ----
	{
		VkImageMemoryBarrier b{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
		b.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
		b.newLayout = VK_IMAGE_LAYOUT_GENERAL;
		b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		b.image = m_easu.image;
		b.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
		b.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		b.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
							 0, nullptr, 0, nullptr, 1, &b);

		VkDescriptorImageInfo ii[2]{};
		ii[0] = {m_sampler, m_easu.view, VK_IMAGE_LAYOUT_GENERAL};
		ii[1] = {VK_NULL_HANDLE, m_output.view, VK_IMAGE_LAYOUT_GENERAL};

		VkWriteDescriptorSet w[2]{};
		w[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		w[0].dstSet = m_rcasSet;
		w[0].dstBinding = 0;
		w[0].descriptorCount = 1;
		w[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		w[0].pImageInfo = &ii[0];
		w[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		w[1].dstSet = m_rcasSet;
		w[1].dstBinding = 1;
		w[1].descriptorCount = 1;
		w[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		w[1].pImageInfo = &ii[1];
		vkUpdateDescriptorSets(m_device, 2, w, 0, nullptr);

		struct
		{
			float con[4];
			int32_t size[4];
		} push{};
		push.con[0] = std::exp2(-sharpness); // stop -> linear
		push.size[0] = (int32_t)dstW;
		push.size[1] = (int32_t)dstH;

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_rcasPipeline);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 0, 1, &m_rcasSet, 0, nullptr);
		vkCmdPushConstants(cmd, m_pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(push), &push);
		vkCmdDispatch(cmd, (dstW + 7) / 8, (dstH + 7) / 8, 1);
	}

	// hasil dibaca oleh tahap blit
	VkImageMemoryBarrier b{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
	b.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
	b.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	b.image = m_output.image;
	b.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
	b.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	b.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
						 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0,
						 0, nullptr, 0, nullptr, 1, &b);

	return m_output.view;
}