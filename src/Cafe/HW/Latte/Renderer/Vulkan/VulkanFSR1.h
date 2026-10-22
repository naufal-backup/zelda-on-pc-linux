#pragma once

// Zelda on PC - AMD FSR 1.0 (EASU + RCAS) as a compute pass (Vulkan).
//
// The picture of the game is made bigger by the GPU before it is drawn in the window: EASU (edge
// adaptive spatial upsampling) makes the picture bigger, RCAS (robust contrast adaptive
// sharpening) sharpens it again. The algorithm is the one of the FidelityFX SDK of AMD (the
// headers of the SDK are in dependencies/FidelityFX-SDK, the shaders here are the same maths in
// one piece of GLSL, as the SDK needs its own shader compiler).

#include "Cafe/HW/Latte/Renderer/Vulkan/VulkanAPI.h"

class VulkanFSR1
{
public:
	VulkanFSR1(VkDevice device, VkPhysicalDevice physicalDevice);
	~VulkanFSR1();

	struct Image
	{
		VkImage image = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
		VkFormat format = VK_FORMAT_UNDEFINED;
		uint32 width = 0, height = 0;
	};

	// Makes the picture of the game bigger (the size of the window) and sharpens it. The view of
	// the picture that was made is returned; it is null when the pass was not possible (the
	// picture is then drawn as usual).
	//   sharpness: 0 = the most sharpening, 1 = no sharpening (the scale of RCAS)
	//   sharpen:   false = EASU only (no RCAS)
	VkImageView Upscale(VkCommandBuffer cmd, VkImage src, VkFormat srcFormat, VkImageLayout srcLayout, uint32 srcW, uint32 srcH, uint32 dstW, uint32 dstH,
						float sharpness, bool sharpen);

	// the picture that was made last (the view that the blit draws)
	VkImageView GetResultView() const { return m_result.view; }

private:
	bool EnsurePipelines();
	bool EnsureImages(uint32 dstW, uint32 dstH);
	bool CreateImage(Image& img, uint32 w, uint32 h, VkFormat format, VkImageUsageFlags usage);
	void DestroyImage(Image& img);
	VkFormat PickFormat(VkFormat wanted);

	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice;

	Image m_easu;    // the picture made by EASU (only when RCAS sharpens it)
	Image m_result;  // the picture that is drawn in the window

	VkImage m_srcImage = VK_NULL_HANDLE;  // the picture of the game (the view of it is made once)
	VkImageView m_srcView = VK_NULL_HANDLE;

	VkSampler m_sampler = VK_NULL_HANDLE;
	VkDescriptorSetLayout m_setLayout = VK_NULL_HANDLE;
	VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
	VkPipeline m_easuPipeline = VK_NULL_HANDLE;
	VkPipeline m_rcasPipeline = VK_NULL_HANDLE;
	VkDescriptorPool m_pool = VK_NULL_HANDLE;
	VkDescriptorSet m_easuSet = VK_NULL_HANDLE;
	VkDescriptorSet m_rcasSet = VK_NULL_HANDLE;
	bool m_pipelineFailed = false;
	bool m_imageFailed = false;
};
