#pragma once

// AMD FSR 1.0 (EASU membuat gambar lebih besar, RCAS menajamkannya) untuk Zelda on PC.
//
// Gambar game dibuat dalam resolusi lebih kecil (resolusi pack "Graphics" dibagi rasio FSR) lalu
// diperbesar oleh kartu grafis dengan EASU, bukan dengan penskalaan biasa. Ini memberi detail lebih
// banyak daripada penskalaan bilinear. Hasilnya kemudian ditajamkan dengan RCAS.
//
// Matematikanya dari SDK FidelityFX milik AMD (ffx_fsr1.h, lisensi MIT), ditulis ulang sebagai dua
// shader compute Vulkan. Tidak ada pustaka AMD yang dibutuhkan saat berjalan.

#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <vector>

class VulkanFSR1
{
public:
	VulkanFSR1(VkDevice device, VkPhysicalDevice physicalDevice);
	~VulkanFSR1();

	// Apakah perangkat ini bisa memakai FSR 1.0 (compute shader + storage image).
	static bool IsSupported(VkPhysicalDevice physicalDevice);

	// Memperbesar gambar sumber menjadi dstW x dstH dan mengembalikan image view hasilnya, atau
	// VK_NULL_HANDLE kalau tidak bisa. sharpness adalah jumlah "stop" penurunan ketajaman RCAS
	// (0 = paling tajam).
	VkImageView Upscale(VkCommandBuffer cmd, VkImageView srcView, VkImageLayout srcLayout, uint32 srcW, uint32 srcH,
						uint32 dstW, uint32 dstH, float sharpness, bool sharpen);

	// Ukuran gambar hasil (untuk dipakai sebagai sumber blit).
	uint32 GetOutputWidth() const { return m_output.width; }
	uint32 GetOutputHeight() const { return m_output.height; }

private:
	struct Image
	{
		VkImage image = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		uint32 width = 0;
		uint32 height = 0;
	};

	bool EnsurePipelines();
	bool EnsureImages(uint32 width, uint32 height);
	bool CreateImage(Image& img, uint32 width, uint32 height, VkFormat format, VkImageUsageFlags usage);
	void DestroyImage(Image& img);
	VkFormat PickFormat(VkFormat preferred);
	uint32 FindMemoryType(uint32 bits, VkMemoryPropertyFlags want);

	VkDevice m_device = VK_NULL_HANDLE;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

	VkDescriptorSetLayout m_setLayout = VK_NULL_HANDLE;
	VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
	VkPipeline m_easuPipeline = VK_NULL_HANDLE;
	VkPipeline m_rcasPipeline = VK_NULL_HANDLE;
	VkDescriptorPool m_pool = VK_NULL_HANDLE;
	VkDescriptorSet m_easuSet = VK_NULL_HANDLE;
	VkDescriptorSet m_rcasSet = VK_NULL_HANDLE;
	VkSampler m_sampler = VK_NULL_HANDLE;
	bool m_pipelineFailed = false;

	// gambar antara (hasil EASU) dan gambar akhir (hasil RCAS)
	Image m_easu;
	Image m_output;
	bool m_imageFailed = false;

	VkFormat m_format = VK_FORMAT_UNDEFINED;
};