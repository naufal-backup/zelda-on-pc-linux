#pragma once

// AMD FSR 1.0 RCAS: penajaman gambar setelah diperbesar ke ukuran jendela.
//
// Ini hanya separuh penajaman dari FSR 1.0 (RCAS). Berjalan sebagai pass tersendiri pada gambar akhir,
// dengan parameter berupa uniform, sehingga perubahan langsung terlihat pada frame berikutnya (tanpa
// restart). Separuh pembesaran (EASU) tidak dipakai: ia menuntut game menggambar dalam resolusi lebih
// kecil, dan itu tidak bisa diubah saat game berjalan.
//
// Matematikanya dari ffx_fsr1.h milik AMD (lisensi MIT).

#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <vector>

class VulkanFSRSharpen
{
public:
	VulkanFSRSharpen(VkDevice device, VkPhysicalDevice physicalDevice);
	~VulkanFSRSharpen();

	// Mempertajam gambar sumber dan mengembalikan image view hasilnya, atau VK_NULL_HANDLE kalau tidak
	// bisa. sharpness adalah jumlah "stop" (0 = paling tajam).
	VkImageView Sharpen(VkCommandBuffer cmd, VkImageView srcView, VkImageLayout srcLayout, uint32 width, uint32 height,
						float sharpness);

private:
	struct Image
	{
		VkImage image = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		uint32 width = 0;
		uint32 height = 0;
	};

	bool EnsurePipeline();
	bool EnsureImage(uint32 width, uint32 height);
	bool CreateImage(Image& img, uint32 width, uint32 height, VkFormat format);
	void DestroyImage(Image& img);
	VkFormat PickFormat(VkFormat preferred);
	uint32 FindMemoryType(uint32 bits, VkMemoryPropertyFlags want);

	VkDevice m_device = VK_NULL_HANDLE;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

	VkDescriptorSetLayout m_setLayout = VK_NULL_HANDLE;
	VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
	VkPipeline m_pipeline = VK_NULL_HANDLE;
	VkDescriptorPool m_pool = VK_NULL_HANDLE;
	VkDescriptorSet m_set = VK_NULL_HANDLE;
	VkSampler m_sampler = VK_NULL_HANDLE;
	bool m_pipelineFailed = false;

	Image m_output;
	bool m_imageFailed = false;
	VkFormat m_format = VK_FORMAT_UNDEFINED;
};