#pragma once

// Zelda on PC - graphics options backed by Cemu graphic packs.

#include <string>
#include <vector>

namespace pce::Graphics
{
	// The presets of the "Enhancements" pack of the community (the shader filters and the texture detail). They are
	// used by the settings (the labels) and by the pack itself (the names of the presets).
	inline constexpr const char* const kClarityPresets[] = {
		"Disabled (Default)", "Serfrost's Preset (Recommended)", "Serfrost's Preset (Dim Display)",
		"Serfrost's Preset (Bright Display)", "Bruz's Preset", "CellHunter's Preset", "Jamie's Preset",
		"t3nka Neutral+", "The Complaining Gamer's Preset", "ViskClarity Preset", "Contrasty Preset",
		"Filmic Preset", "Silentverge's Survival of the Wild Preset (Warmer)",
		"Silentverge's Survival of the Wild Preset (Colder)", "Godot's Mexican Preset", "Hexaae's Clear Preset",
		"MaranBr Preset (Little More Intense)", "YeMaoWuXin's Preset (Bright colors)",
		"YeMaoWuXin's Preset (Vivid and clear)", "Cobalt's Preset (Saturated & Bright)", "User-Defined Preset",
	};
	inline constexpr const char* const kReflectionRangePresets[] = {
		"Disabled", "Extra Low (2 samples)", "Low (4 samples, Default)", "Medium (8 samples)",
		"High (16 samples)", "Extra High (32 samples)", "Extreme (64 samples)",
	};
	inline constexpr const char* const kAnisotropyPresets[] = {
		"Low (1x)", "Medium (2x)", "High (4x)", "Ultra (8x, Default)", "Extreme (16x)",
	};

	struct ResolutionOption
	{
		int width, height;
		std::string label;
	};
	// A Vulkan driver (the ICD manifests the loader reads). A machine can have several of them for the same card
	// (the proprietary driver of AMD and RADV, for example), and then the loader offers both.
	struct VulkanDriver
	{
		std::string manifest;  // the path of the ICD manifest
		std::string name;      // the name shown in the settings
	};
	// the drivers installed on this machine (empty when the list cannot be read)
	const std::vector<VulkanDriver>& GetVulkanDrivers();
	// tells the Vulkan loader which driver to use (the setting with the manifest of the driver; empty: all of them)
	void ApplyVulkanDriver();
	// Aspect ratio the game renders in: 1 16:9, 2 16:10, 3 21:9, 4 32:9. For Settings::aspectRatio = 0 it follows the
	// main display, for the ultrawide formats only (a 16:10 display keeps 16:9 unless 16:10 is chosen).
	int EffectiveAspect(int setting);
	const std::vector<std::string>& GetAspectOptions();
	// the resolutions of an aspect ratio (same number of entries and the same heights for all of them)
	const std::vector<ResolutionOption>& GetResolutionOptions(int aspect = 1);
	// The picture inside the game window (it has bars when the window has another shape), in pixels of the window's
	// client area. false while there is no picture yet.
	bool GetPictureArea(int& x, int& y, int& width, int& height, int& windowWidth, int& windowHeight);
	const std::vector<std::string>& GetShadowOptions();
	const std::vector<std::string>& GetAAOptions();
	const std::vector<int>& GetFpsOptions();

	// copies community packs (read-only source: the user's Cemu install) and writes the PC Edition pack
	void PrepareGraphicPacks();
	// after GraphicPack2::LoadAll(): enable packs and select presets from settings.ini
	void ApplySettingsToPacks();
	// at runtime from the settings menu: re-apply live-capable options (FPS limit, AA, vsync)
	// returns true if some change requires a restart (resolution/shadows)
	bool ApplyLive();
	bool RestartPending();
	// directory of the PC Edition graphic pack (patches + content replacements)
	fs::path GetEditionPackDir();

	// display mode the main window is in (Settings::displayMode: 0 windowed, 1 borderless, 2 exclusive fullscreen); set by
	// the window, read by the swapchain (exclusive fullscreen is a property of the swapchain)
	void SetDisplayModeApplied(int mode);
	int GetDisplayModeApplied();
}
