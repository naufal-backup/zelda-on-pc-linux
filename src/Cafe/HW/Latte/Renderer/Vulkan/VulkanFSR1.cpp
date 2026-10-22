#include "Cafe/HW/Latte/Renderer/Vulkan/VulkanFSR1.h"

#include <cmath>
#include <cstring>
#include <string>

#include "Cafe/HW/Latte/Renderer/Vulkan/RendererShaderVk.h"
#include "Cemu/Logging/CemuLogging.h"

std::vector<uint32> RendererShaderVk_CompileCompute(const std::string& source, std::string& errorOut); // RendererShaderVk.cpp

namespace
{
	// The maths of AMD FSR 1.0 (the FidelityFX SDK, MIT licence). The SDK needs its own shader
	// compiler and its own headers for the constants, so the two passes are here in one piece of
	// GLSL each: the algorithm is the same, the taps are read one by one instead of with the
	// gathers of the SDK.
	const char* kEasuShader = R"(#version 450
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
layout(set = 0, binding = 0) uniform sampler2D srcTex;
layout(set = 0, binding = 1, FSR_FORMAT) uniform writeonly image2D dstImg;
layout(push_constant) uniform PC {
	vec4 srcSize;    // the size of the picture of the game (xy), the size of the picture that is made (zw)
	ivec4 dstOffset; // where in the picture the result is written (xy)
} pc;

// the reciprocal of AMD (a bit trick: no division, and no infinity for zero)
float rcpApprox(float v) { return float(0x7ef07ebb - floatBitsToUint(v)); }
float rcpSqrtApprox(float v) { return float(0x5f347d74 - (floatBitsToUint(v) >> 1u)); }

float saturate(float v) { return clamp(v, 0.0, 1.0); }


vec3 fetch(ivec2 p)
{
	p = clamp(p, ivec2(0), ivec2(pc.srcSize.xy) - ivec2(1));
	return texelFetch(srcTex, p, 0).rgb;
}

// the weight of a tap (the lanczos like window of FSR)
void easuTap(inout vec3 acc, inout float w, vec2 offset, vec2 dir, vec2 len, float lob, float clp, vec3 color)
{
	vec2 rotated;
	rotated.x = offset.x * dir.x + offset.y * dir.y;
	rotated.y = offset.x * (-dir.y) + offset.y * dir.x;
	rotated *= len;
	float d2 = rotated.x * rotated.x + rotated.y * rotated.y;
	d2 = min(d2, clp);
	float weightB = (2.0 / 5.0) * d2 - 1.0;
	float weightA = lob * d2 - 1.0;
	weightB *= weightB;
	weightA *= weightA;
	weightB = (25.0 / 16.0) * weightB - (25.0 / 16.0 - 1.0);
	float weight = weightB * weightA;
	acc += color * weight;
	w += weight;
}

// the direction and the length of the edge at the place of the pixel
void easuSet(inout vec2 dir, inout float len, vec2 pp, int which, float lA, float lB, float lC, float lD, float lE)
{
	float weight = 0.0;
	if (which == 0)
		weight = (1.0 - pp.x) * (1.0 - pp.y);
	else if (which == 1)
		weight = pp.x * (1.0 - pp.y);
	else if (which == 2)
		weight = (1.0 - pp.x) * pp.y;
	else
		weight = pp.x * pp.y;

	float dc = lD - lC;
	float cb = lC - lB;
	float lengthX = max(abs(dc), abs(cb));
	lengthX = rcpApprox(lengthX);
	float directionX = lD - lB;
	dir.x += directionX * weight;
	lengthX = saturate(abs(directionX) * lengthX);
	lengthX *= lengthX;
	len += lengthX * weight;

	float ec = lE - lC;
	float ca = lC - lA;
	float lengthY = max(abs(ec), abs(ca));
	lengthY = rcpApprox(lengthY);
	float directionY = lE - lA;
	dir.y += directionY * weight;
	lengthY = saturate(abs(directionY) * lengthY);
	lengthY *= lengthY;
	len += lengthY * weight;
}

void main()
{
	ivec2 dst = ivec2(gl_GlobalInvocationID.xy);
	if (dst.x >= int(pc.srcSize.z) || dst.y >= int(pc.srcSize.w))
		return;

	// the place of the pixel in the picture of the game
	vec2 scale = pc.srcSize.xy / pc.srcSize.zw;
	vec2 pp = vec2(dst) * scale + (0.5 * scale - 0.5);
	vec2 fp = floor(pp);
	pp -= fp;
	ivec2 ip = ivec2(fp);

	// the twelve taps around it
	vec3 b = fetch(ip + ivec2(0, -1));
	vec3 c = fetch(ip + ivec2(1, -1));
	vec3 i = fetch(ip + ivec2(-1, 1));
	vec3 j = fetch(ip + ivec2(0, 1));
	vec3 f = fetch(ip + ivec2(0, 0));
	vec3 e = fetch(ip + ivec2(-1, 0));
	vec3 k = fetch(ip + ivec2(1, 1));
	vec3 l = fetch(ip + ivec2(2, 1));
	vec3 h = fetch(ip + ivec2(2, 0));
	vec3 g = fetch(ip + ivec2(1, 0));
	vec3 o = fetch(ip + ivec2(1, 2));
	vec3 n = fetch(ip + ivec2(0, 2));

	// the brightness of the taps (twice the luma)
	float bL = dot(b, vec3(0.5, 1.0, 0.5));
	float cL = dot(c, vec3(0.5, 1.0, 0.5));
	float iL = dot(i, vec3(0.5, 1.0, 0.5));
	float jL = dot(j, vec3(0.5, 1.0, 0.5));
	float fL = dot(f, vec3(0.5, 1.0, 0.5));
	float eL = dot(e, vec3(0.5, 1.0, 0.5));
	float kL = dot(k, vec3(0.5, 1.0, 0.5));
	float lL = dot(l, vec3(0.5, 1.0, 0.5));
	float hL = dot(h, vec3(0.5, 1.0, 0.5));
	float gL = dot(g, vec3(0.5, 1.0, 0.5));
	float oL = dot(o, vec3(0.5, 1.0, 0.5));
	float nL = dot(n, vec3(0.5, 1.0, 0.5));

	vec2 dir = vec2(0.0);
	float len = 0.0;
	easuSet(dir, len, pp, 0, bL, eL, fL, gL, jL);
	easuSet(dir, len, pp, 1, cL, fL, gL, hL, kL);
	easuSet(dir, len, pp, 2, fL, iL, jL, kL, nL);
	easuSet(dir, len, pp, 3, gL, jL, kL, lL, oL);

	vec2 dir2 = dir * dir;
	float dirR = dir2.x + dir2.y;
	bool zro = dirR < (1.0 / 32768.0);
	dirR = zro ? 1.0 : rcpSqrtApprox(dirR);
	dirR = zro ? 1.0 : dirR;
	dir.x = zro ? 1.0 : dir.x;
	dir *= dirR;

	len = len * 0.5;
	len *= len;

	float stretch = (dir.x * dir.x + dir.y * dir.y) * rcpApprox(max(abs(dir.x), abs(dir.y)));
	vec2 len2 = vec2(1.0 + (stretch - 1.0) * len, 1.0 + (-0.5) * len);
	float lob = 0.5 + ((1.0 / 4.0 - 0.04) - 0.5) * len;
	float clp = rcpApprox(lob);

	vec3 min4 = min(min(f, g), min(j, k));
	vec3 max4 = max(max(f, g), max(j, k));

	vec3 acc = vec3(0.0);
	float weight = 0.0;
	easuTap(acc, weight, vec2(0.0, -1.0) - pp, dir, len2, lob, clp, b);
	easuTap(acc, weight, vec2(1.0, -1.0) - pp, dir, len2, lob, clp, c);
	easuTap(acc, weight, vec2(-1.0, 1.0) - pp, dir, len2, lob, clp, i);
	easuTap(acc, weight, vec2(0.0, 1.0) - pp, dir, len2, lob, clp, j);
	easuTap(acc, weight, vec2(0.0, 0.0) - pp, dir, len2, lob, clp, f);
	easuTap(acc, weight, vec2(-1.0, 0.0) - pp, dir, len2, lob, clp, e);
	easuTap(acc, weight, vec2(1.0, 1.0) - pp, dir, len2, lob, clp, k);
	easuTap(acc, weight, vec2(2.0, 1.0) - pp, dir, len2, lob, clp, l);
	easuTap(acc, weight, vec2(2.0, 0.0) - pp, dir, len2, lob, clp, h);
	easuTap(acc, weight, vec2(1.0, 0.0) - pp, dir, len2, lob, clp, g);
	easuTap(acc, weight, vec2(1.0, 2.0) - pp, dir, len2, lob, clp, o);
	easuTap(acc, weight, vec2(0.0, 2.0) - pp, dir, len2, lob, clp, n);

	vec3 pix = clamp(acc * (weight > 0.0 ? 1.0 / weight : 0.0), min4, max4);
	imageStore(dstImg, dst + pc.dstOffset.xy, vec4(pix, 1.0));
}
)";

	// The second pass of FSR 1.0: the sharpening that follows the making bigger.
	const char* kRcasShader = R"(#version 450
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
layout(set = 0, binding = 0) uniform sampler2D srcTex;
layout(set = 0, binding = 1, FSR_FORMAT) uniform writeonly image2D dstImg;
layout(push_constant) uniform PC {
	vec4 srcSize;    // the size of the picture that was made bigger (xy), the sharpness (z)
	ivec4 dstOffset; // where in the picture the result is written (xy)
} pc;

float saturate(float v) { return clamp(v, 0.0, 1.0); }

vec3 fetch(ivec2 p)
{
	p = clamp(p, ivec2(0), ivec2(pc.srcSize.xy) - ivec2(1));
	return texelFetch(srcTex, p, 0).rgb;
}

void main()
{
	ivec2 dst = ivec2(gl_GlobalInvocationID.xy);
	if (dst.x >= int(pc.srcSize.x) || dst.y >= int(pc.srcSize.y))
		return;

	// the sharpness: 0 = the most, 1 = none
	float sharpness = exp2(-pc.srcSize.z);
	float limit = 0.25 - (1.0 / 16.0);

	ivec2 sp = dst;
	vec3 b = fetch(sp + ivec2(0, -1));
	vec3 d = fetch(sp + ivec2(-1, 0));
	vec3 e = fetch(sp);
	vec3 f = fetch(sp + ivec2(1, 0));
	vec3 h = fetch(sp + ivec2(0, 1));

	float bL = dot(b, vec3(0.5, 1.0, 0.5));
	float dL = dot(d, vec3(0.5, 1.0, 0.5));
	float eL = dot(e, vec3(0.5, 1.0, 0.5));
	float fL = dot(f, vec3(0.5, 1.0, 0.5));
	float hL = dot(h, vec3(0.5, 1.0, 0.5));

	// the noise around (the sharpening is softer where there is noise)
	float nz = 0.25 * bL + 0.25 * dL + 0.25 * fL + 0.25 * hL - eL;
	float hi = max(max(max(bL, dL), eL), max(fL, hL));
	float lo = min(min(min(bL, dL), eL), min(fL, hL));
	nz = saturate(abs(nz) * (1.0 / max(hi - lo, 1e-6)));
	nz = -0.5 * nz + 1.0;

	vec3 mn4 = min(min(b, d), min(f, h));
	vec3 mx4 = max(max(b, d), max(f, h));

	vec2 peakC = vec2(1.0, -4.0);
	vec3 hitMin = mn4 / (4.0 * mx4);
	vec3 hitMax = (peakC.x - mx4) / (4.0 * mn4 + peakC.y);
	vec3 lobe = max(-hitMin, hitMax);
	float lobeMax = max(-limit, min(max(lobe.r, max(lobe.g, lobe.b)), 0.0));
	lobeMax *= sharpness;
	lobeMax *= nz;

	float rcpL = 1.0 / (4.0 * lobeMax + 1.0);
	vec3 pix = (lobeMax * b + lobeMax * d + lobeMax * h + lobeMax * f + e) * rcpL;
	imageStore(dstImg, dst + pc.dstOffset.xy, vec4(pix, 1.0));
}
)";

	uint32 FindMemory(VkPhysicalDevice pd, uint32 bits, VkMemoryPropertyFlags want)
	{
		VkPhysicalDeviceMemoryProperties mp;
		vkGetPhysicalDeviceMemoryProperties(pd, &mp);
		for (uint32 i = 0; i < mp.memoryTypeCount; i++)
			if ((bits & (1u << i)) && (mp.memoryTypes[i].propertyFlags & want) == want)
				return i;
		return UINT32_MAX;
	}

}

VulkanFSR1::VulkanFSR1(VkDevice device, VkPhysicalDevice physicalDevice) : m_device(device), m_physicalDevice(physicalDevice) {}

VulkanFSR1::~VulkanFSR1()
{
	vkDeviceWaitIdle(m_device);
	DestroyImage(m_easu);
	DestroyImage(m_result);
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
	ii.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	if (vkCreateImage(m_device, &ii, nullptr, &img.image) != VK_SUCCESS)
		return false;
	VkMemoryRequirements req;
	vkGetImageMemoryRequirements(m_device, img.image, &req);
	VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
	ai.allocationSize = req.size;
	ai.memoryTypeIndex = FindMemory(m_physicalDevice, req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (ai.memoryTypeIndex == UINT32_MAX || vkAllocateMemory(m_device, &ai, nullptr, &img.memory) != VK_SUCCESS)
		return false;
	vkBindImageMemory(m_device, img.image, img.memory, 0);
	VkImageViewCreateInfo vi{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
	vi.image = img.image;
	vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
	vi.format = format;
	vi.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
	if (vkCreateImageView(m_device, &vi, nullptr, &img.view) != VK_SUCCESS)
		return false;
	img.format = format;
	img.width = w;
	img.height = h;
	return true;
}

VkFormat VulkanFSR1::PickFormat(VkFormat wanted)
{
	// the format of the picture that is made bigger: 16 bits per channel when the card can write
	// it in a picture (it keeps the quality of the sharpening), else the usual 8 bits
	VkFormatProperties props;
	vkGetPhysicalDeviceFormatProperties(m_physicalDevice, wanted, &props);
	if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)
		return wanted;
	return VK_FORMAT_R8G8B8A8_UNORM;
}

bool VulkanFSR1::EnsurePipelines()
{
	if (m_easuPipeline && m_rcasPipeline)
		return true;
	if (m_pipelineFailed)
		return false;
	m_pipelineFailed = true;

	VkSamplerCreateInfo si{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
	si.magFilter = si.minFilter = VK_FILTER_NEAREST;
	si.addressModeU = si.addressModeV = si.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	si.maxLod = 0.25f;
	if (vkCreateSampler(m_device, &si, nullptr, &m_sampler) != VK_SUCCESS)
		return false;

	VkDescriptorSetLayoutBinding b[2]{};
	b[0] = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
	b[1] = {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr};
	VkDescriptorSetLayoutCreateInfo li{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
	li.bindingCount = 2;
	li.pBindings = b;
	if (vkCreateDescriptorSetLayout(m_device, &li, nullptr, &m_setLayout) != VK_SUCCESS)
		return false;

	VkPushConstantRange pr{VK_SHADER_STAGE_COMPUTE_BIT, 0, 32};
	VkPipelineLayoutCreateInfo pli{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
	pli.setLayoutCount = 1;
	pli.pSetLayouts = &m_setLayout;
	pli.pushConstantRangeCount = 1;
	pli.pPushConstantRanges = &pr;
	if (vkCreatePipelineLayout(m_device, &pli, nullptr, &m_pipelineLayout) != VK_SUCCESS)
		return false;

	// the qualifier of the picture in the shader has to be the format of the picture
	const VkFormat format = PickFormat(VK_FORMAT_R16G16B16A16_SFLOAT);
	const char* formatName = (format == VK_FORMAT_R16G16B16A16_SFLOAT) ? "rgba16f" : "rgba8";
	std::string easuSource = kEasuShader;
	std::string rcasSource = kRcasShader;
	{
		const size_t pos = easuSource.find("FSR_FORMAT");
		if (pos != std::string::npos)
			easuSource = easuSource.substr(0, pos) + formatName + easuSource.substr(pos + 10);
		const size_t pos2 = rcasSource.find("FSR_FORMAT");
		if (pos2 != std::string::npos)
			rcasSource = rcasSource.substr(0, pos2) + formatName + rcasSource.substr(pos2 + 10);
	}

	std::string err;
	auto spirv = RendererShaderVk_CompileCompute(easuSource, err);
	if (spirv.empty())
	{
		cemuLog_log(LogType::Force, "FSR 1.0: the EASU shader does not compile: {}", err);
		return false;
	}
	VkShaderModuleCreateInfo mi{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
	mi.codeSize = spirv.size() * 4;
	mi.pCode = spirv.data();
	VkShaderModule module;
	if (vkCreateShaderModule(m_device, &mi, nullptr, &module) != VK_SUCCESS)
		return false;
	VkComputePipelineCreateInfo ci{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
	ci.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	ci.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	ci.stage.module = module;
	ci.stage.pName = "main";
	ci.layout = m_pipelineLayout;
	const VkResult r = vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &ci, nullptr, &m_easuPipeline);
	vkDestroyShaderModule(m_device, module, nullptr);
	if (r != VK_SUCCESS)
		return false;

	spirv = RendererShaderVk_CompileCompute(rcasSource, err);
	if (spirv.empty())
	{
		cemuLog_log(LogType::Force, "FSR 1.0: the RCAS shader does not compile: {}", err);
		return false;
	}
	mi.codeSize = spirv.size() * 4;
	mi.pCode = spirv.data();
	if (vkCreateShaderModule(m_device, &mi, nullptr, &module) != VK_SUCCESS)
		return false;
	ci.stage.module = module;
	const VkResult r2 = vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &ci, nullptr, &m_rcasPipeline);
	vkDestroyShaderModule(m_device, module, nullptr);
	if (r2 != VK_SUCCESS)
		return false;

	VkDescriptorPoolSize ps[2] = {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2}, {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2}};
	VkDescriptorPoolCreateInfo pi{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
	pi.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pi.maxSets = 2;
	pi.poolSizeCount = 2;
	pi.pPoolSizes = ps;
	if (vkCreateDescriptorPool(m_device, &pi, nullptr, &m_pool) != VK_SUCCESS)
		return false;

	m_pipelineFailed = false;
	return true;
}

bool VulkanFSR1::EnsureImages(uint32 dstW, uint32 dstH)
{
	if (m_imageFailed)
		return false;
	if (m_result.width == dstW && m_result.height == dstH && m_result.image && m_easu.image)
		return true;
	vkDeviceWaitIdle(m_device);
	DestroyImage(m_easu);
	DestroyImage(m_result);
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
	const VkFormat format = PickFormat(VK_FORMAT_R16G16B16A16_SFLOAT);
	const VkImageUsageFlags usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	if (!CreateImage(m_result, dstW, dstH, format, usage) || !CreateImage(m_easu, dstW, dstH, format, usage))
	{
		cemuLog_log(LogType::Force, "FSR 1.0: the picture could not be made");
		DestroyImage(m_easu);
		DestroyImage(m_result);
		m_imageFailed = true;
		return false;
	}
	return true;
}

VkImageView VulkanFSR1::Upscale(VkCommandBuffer cmd, VkImageView srcView, VkImageLayout srcLayout, uint32 srcW, uint32 srcH, uint32 dstW, uint32 dstH,
								 float sharpness, bool sharpen)
{
	if (srcView == VK_NULL_HANDLE || srcW == 0 || srcH == 0 || dstW == 0 || dstH == 0)
		return VK_NULL_HANDLE;
	if (srcW >= dstW && srcH >= dstH)
		return VK_NULL_HANDLE; // the picture is not made bigger: nothing to do
	if (!EnsurePipelines() || !EnsureImages(dstW, dstH))
		return VK_NULL_HANDLE;

	// the view of the picture of the game (it is sampled by the first pass)
	// the descriptor sets of the two passes
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

	// the first pass (EASU) writes in the picture of the sharpening, the second one (RCAS) in the
	// picture that is drawn in the window
	const bool withRcas = sharpen;
	VkImageView firstDst = withRcas ? m_easu.view : m_result.view;

	VkDescriptorImageInfo ii[2]{};
	ii[0] = {m_sampler, srcView, srcLayout};
	ii[1] = {VK_NULL_HANDLE, firstDst, VK_IMAGE_LAYOUT_GENERAL};
	VkWriteDescriptorSet w[2]{};
	for (uint32 i = 0; i < 2; i++)
	{
		w[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		w[i].dstSet = m_easuSet;
		w[i].dstBinding = i;
		w[i].descriptorCount = 1;
		w[i].descriptorType = i == 0 ? VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER : VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		w[i].pImageInfo = &ii[i];
	}
	vkUpdateDescriptorSets(m_device, 2, w, 0, nullptr);

	if (withRcas)
	{
		ii[0] = {m_sampler, m_easu.view, VK_IMAGE_LAYOUT_GENERAL};
		ii[1] = {VK_NULL_HANDLE, m_result.view, VK_IMAGE_LAYOUT_GENERAL};
		for (uint32 i = 0; i < 2; i++)
		{
			w[i].dstSet = m_rcasSet;
			w[i].pImageInfo = &ii[i];
		}
		vkUpdateDescriptorSets(m_device, 2, w, 0, nullptr);
	}

	// the pictures are in the layout GENERAL: they are written by the shader and read by it

	// the picture of the game is read by the first pass
	{
		VkMemoryBarrier mb{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
		mb.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_WRITE_BIT;
		mb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
							 VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &mb, 0, nullptr, 0, nullptr);
	}

	// the first pass (EASU): the picture of the game is made bigger
	{
		struct Push
		{
			float srcWidth, srcHeight, dstWidth, dstHeight;
			int32 dstOffsetX, dstOffsetY, pad0, pad1;
		} push;
		push.srcWidth = (float)srcW;
		push.srcHeight = (float)srcH;
		push.dstWidth = (float)dstW;
		push.dstHeight = (float)dstH;
		push.dstOffsetX = 0;
		push.dstOffsetY = 0;
		push.pad0 = push.pad1 = 0;
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_easuPipeline);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 0, 1, &m_easuSet, 0, nullptr);
		vkCmdPushConstants(cmd, m_pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(push), &push);
		vkCmdDispatch(cmd, (dstW + 7) / 8, (dstH + 7) / 8, 1);
	}

	// the second pass (RCAS): the picture is sharpened
	if (withRcas)
	{
		struct Push
		{
			float srcWidth, srcHeight, sharpness, pad0;
			int32 dstOffsetX, dstOffsetY, pad1, pad2;
		} push;
		push.srcWidth = (float)dstW;
		push.srcHeight = (float)dstH;
		push.sharpness = sharpness;
		push.pad0 = 0;
		push.dstOffsetX = 0;
		push.dstOffsetY = 0;
		push.pad1 = push.pad2 = 0;
		// the picture of EASU is read: it has to be ready
		VkMemoryBarrier mb{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
		mb.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		mb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &mb, 0, nullptr, 0, nullptr);
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_rcasPipeline);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 0, 1, &m_rcasSet, 0, nullptr);
		vkCmdPushConstants(cmd, m_pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(push), &push);
		vkCmdDispatch(cmd, (dstW + 7) / 8, (dstH + 7) / 8, 1);
	}

	// the picture is read by the drawing of the window
	{
		VkMemoryBarrier mb{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
		mb.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		mb.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
							 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0, 1, &mb, 0, nullptr, 0, nullptr);
	}

	return m_result.view;
}
