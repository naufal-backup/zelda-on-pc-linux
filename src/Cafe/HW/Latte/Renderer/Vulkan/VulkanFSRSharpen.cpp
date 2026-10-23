// AMD FSR 1.0 RCAS (penajaman) untuk Zelda on PC. Lihat VulkanFSRSharpen.h.
//
// RCAS (Robust Contrast Adaptive Sharpening) menajamkan gambar dengan batas kontras: banyaknya
// penajaman menyesuaikan diri dengan lingkungan tiap piksel, sehingga tepi tidak memunculkan lingkaran
// cahaya. Matematikanya terjemahan langsung dari FsrRcasF di ffx_fsr1.h milik AMD (lisensi MIT):
//   https://github.com/GPUOpen-Effects/FidelityFX-FSR

#include "Cafe/HW/Latte/Renderer/Vulkan/VulkanFSRSharpen.h"

#include <cmath>
#include <cstring>
#include <string>

#include "Cafe/HW/Latte/Renderer/Vulkan/VulkanAPI.h"
#include "Cafe/HW/Latte/Renderer/Vulkan/RendererShaderVk.h"
#include "Cemu/Logging/CemuLogging.h"

std::vector<uint32> RendererShaderVk_CompileCompute(const std::string& source, std::string& errorOut); // RendererShaderVk.cpp

namespace
{
	const char* kRcasShader = R"(
#version 450
layout(local_size_x = 8, local_size_y = 8) in;

layout(set = 0, binding = 0) uniform sampler2D srcTex;
layout(set = 0, binding = 1, rgba16f) uniform writeonly image2D dstImg;

layout(push_constant) uniform Push {
	vec4 con;    // x = ketajaman linear (2^-stop)
	ivec4 size;  // xy = ukuran gambar
} pc;

// batas penajaman yang masih wajar
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

	// deteksi derau: pada tempat yang ramai detail penajaman dikurangi
	float nz = 0.25 * bL + 0.25 * dL + 0.25 * fL + 0.25 * hL - eL;
	float mx = max(max(max(bL, dL), eL), max(fL, hL));
	float mn = min(min(min(bL, dL), eL), min(fL, hL));
	nz = saturate(abs(nz) * (1.0 / max(mx - mn, 1e-6)));
	nz = -0.5 * nz + 1.0;

	// batas bawah dan atas dari lingkungan
	float mnR = min(min(b.r, d.r), min(f.r, h.r));
	float mnG = min(min(b.g, d.g), min(f.g, h.g));
	float mnB = min(min(b.b, d.b), min(f.b, h.b));
	float mxR = max(max(b.r, d.r), max(f.r, h.r));
	float mxG = max(max(b.g, d.g), max(f.g, h.g));
	float mxB = max(max(b.b, d.b), max(f.b, h.b));

	// pengatur agar hasil tidak keluar dari rentang tetangganya
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

	// kurangi pada tempat yang ramai detail
	lobe *= nz;

	// hasil akhir
	float rcpL = 1.0 / (4.0 * lobe + 1.0);
	vec3 pix = (lobe * b + lobe * d + lobe * h + lobe * f + e) * rcpL;
	imageStore(dstImg, ip, vec4(pix, 1.0));
}
)";
}

VulkanFSRSharpen::VulkanFSRSharpen(VkDevice device, VkPhysicalDevice physicalDevice)
	: m_device(device), m_physicalDevice(physicalDevice)
{
}

VulkanFSRSharpen::~VulkanFSRSharpen()
{
	if (m_device == VK_NULL_HANDLE)
		return;
	vkDeviceWaitIdle(m_device);
	DestroyImage(m_output);
	if (m_pipeline)
		vkDestroyPipeline(m_device, m_pipeline, nullptr);
	if (m_pipelineLayout)
		vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
	if (m_setLayout)
		vkDestroyDescriptorSetLayout(m_device, m_setLayout, nullptr);
	if (m_pool)
		vkDestroyDescriptorPool(m_device, m_pool, nullptr);
	if (m_sampler)
		vkDestroySampler(m_device, m_sampler, nullptr);
}

uint32 VulkanFSRSharpen::FindMemoryType(uint32 bits, VkMemoryPropertyFlags want)
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

VkFormat VulkanFSRSharpen::PickFormat(VkFormat preferred)
{
	VkFormatProperties props{};
	vkGetPhysicalDeviceFormatProperties(m_physicalDevice, preferred, &props);
	if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT)
		return preferred;
	return VK_FORMAT_R8G8B8A8_UNORM;
}

void VulkanFSRSharpen::DestroyImage(Image& img)
{
	if (img.view)
		vkDestroyImageView(m_device, img.view, nullptr);
	if (img.image)
		vkDestroyImage(m_device, img.image, nullptr);
	if (img.memory)
		vkFreeMemory(m_device, img.memory, nullptr);
	img = Image{};
}

bool VulkanFSRSharpen::CreateImage(Image& img, uint32 w, uint32 h, VkFormat format)
{
	VkImageCreateInfo ii{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
	ii.imageType = VK_IMAGE_TYPE_2D;
	ii.format = format;
	ii.extent = {w, h, 1};
	ii.mipLevels = 1;
	ii.arrayLayers = 1;
	ii.samples = VK_SAMPLE_COUNT_1_BIT;
	ii.tiling = VK_IMAGE_TILING_OPTIMAL;
	ii.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	ii.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	// ditulis oleh shader dan dibaca sebagai tekstur: GENERAL cocok untuk keduanya
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

bool VulkanFSRSharpen::EnsureImage(uint32 width, uint32 height)
{
	if (m_imageFailed)
		return false;
	if (m_output.width == width && m_output.height == height && m_output.image)
		return true;

	DestroyImage(m_output);
	if (m_set)
	{
		vkFreeDescriptorSets(m_device, m_pool, 1, &m_set);
		m_set = VK_NULL_HANDLE;
	}
	if (!CreateImage(m_output, width, height, m_format))
	{
		cemuLog_log(LogType::Force, "FSR Sharpening: gambar tidak bisa dibuat");
		DestroyImage(m_output);
		m_imageFailed = true;
		return false;
	}
	return true;
}

bool VulkanFSRSharpen::EnsurePipeline()
{
	if (m_pipelineFailed)
		return false;
	if (m_pipeline)
		return true;

	m_format = PickFormat(VK_FORMAT_R16G16B16A16_SFLOAT);
	const char* formatName = (m_format == VK_FORMAT_R16G16B16A16_SFLOAT) ? "rgba16f" : "rgba8";

	// format gambar adalah bagian dari shader (kualifikasi storage image). Panjang yang diganti harus
	// panjang nama lama (7 untuk "rgba16f"), bukan ditebak: kalau salah, hasilnya "rgba16ff" dan shader
	// tidak bisa dikompilasi.
	std::string source(kRcasShader);
	static const std::string kOldFormat = "rgba16f";
	const size_t pos = source.find(kOldFormat + ") uniform writeonly image2D dstImg");
	if (pos != std::string::npos)
		source.replace(pos, kOldFormat.size(), formatName);

	std::string err;
	auto spirv = RendererShaderVk_CompileCompute(source, err);
	if (spirv.empty())
	{
		cemuLog_log(LogType::Force, "FSR Sharpening: shader RCAS gagal: {}", err);
		m_pipelineFailed = true;
		return false;
	}

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

	VkPushConstantRange range{};
	range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	range.offset = 0;
	range.size = 4 * sizeof(float) + 4 * sizeof(int32_t);

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

	VkShaderModuleCreateInfo smi{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
	smi.codeSize = spirv.size() * sizeof(uint32);
	smi.pCode = spirv.data();
	VkShaderModule module = VK_NULL_HANDLE;
	if (vkCreateShaderModule(m_device, &smi, nullptr, &module) != VK_SUCCESS)
	{
		m_pipelineFailed = true;
		return false;
	}

	VkComputePipelineCreateInfo ci{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
	ci.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	ci.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	ci.stage.module = module;
	ci.stage.pName = "main";
	ci.layout = m_pipelineLayout;
	const VkResult r = vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &ci, nullptr, &m_pipeline);
	vkDestroyShaderModule(m_device, module, nullptr);
	if (r != VK_SUCCESS)
	{
		cemuLog_log(LogType::Force, "FSR Sharpening: pipeline tidak bisa dibuat");
		m_pipelineFailed = true;
		return false;
	}

	VkDescriptorPoolSize sizes[2]{};
	sizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sizes[0].descriptorCount = 1;
	sizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	sizes[1].descriptorCount = 1;
	VkDescriptorPoolCreateInfo dpi{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
	dpi.maxSets = 1;
	dpi.poolSizeCount = 2;
	dpi.pPoolSizes = sizes;
	if (vkCreateDescriptorPool(m_device, &dpi, nullptr, &m_pool) != VK_SUCCESS)
	{
		m_pipelineFailed = true;
		return false;
	}

	VkSamplerCreateInfo sci{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
	sci.magFilter = VK_FILTER_NEAREST;
	sci.minFilter = VK_FILTER_NEAREST;
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

VkImageView VulkanFSRSharpen::Sharpen(VkCommandBuffer cmd, VkImageView srcView, VkImageLayout srcLayout, uint32 width, uint32 height, float sharpness)
{
	if (srcView == VK_NULL_HANDLE || width == 0 || height == 0)
		return VK_NULL_HANDLE;
	if (!EnsurePipeline() || !EnsureImage(width, height))
		return VK_NULL_HANDLE;

	if (!m_set)
	{
		VkDescriptorSetAllocateInfo ai{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
		ai.descriptorPool = m_pool;
		ai.descriptorSetCount = 1;
		ai.pSetLayouts = &m_setLayout;
		if (vkAllocateDescriptorSets(m_device, &ai, &m_set) != VK_SUCCESS)
			return VK_NULL_HANDLE;
	}

	VkDescriptorImageInfo ii[2]{};
	ii[0] = {m_sampler, srcView, srcLayout};
	ii[1] = {VK_NULL_HANDLE, m_output.view, VK_IMAGE_LAYOUT_GENERAL};

	VkWriteDescriptorSet w[2]{};
	w[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	w[0].dstSet = m_set;
	w[0].dstBinding = 0;
	w[0].descriptorCount = 1;
	w[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	w[0].pImageInfo = &ii[0];
	w[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	w[1].dstSet = m_set;
	w[1].dstBinding = 1;
	w[1].descriptorCount = 1;
	w[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	w[1].pImageInfo = &ii[1];
	vkUpdateDescriptorSets(m_device, 2, w, 0, nullptr);

	// gambar tujuan dalam layout GENERAL
	VkImageMemoryBarrier b{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
	b.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
	b.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	b.image = m_output.image;
	b.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
	b.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	b.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
						 0, nullptr, 0, nullptr, 1, &b);

	struct
	{
		float con[4];
		int32_t size[4];
	} push{};
	push.con[0] = std::exp2(-sharpness); // stop -> linear
	push.size[0] = (int32_t)width;
	push.size[1] = (int32_t)height;

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 0, 1, &m_set, 0, nullptr);
	vkCmdPushConstants(cmd, m_pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(push), &push);
	vkCmdDispatch(cmd, (width + 7) / 8, (height + 7) / 8, 1);

	// hasil dibaca tahap blit
	b.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	b.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
						 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, 0,
						 0, nullptr, 0, nullptr, 1, &b);

	return m_output.view;
}