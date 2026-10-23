#pragma once

// Zelda on PC - graphics options backed by Cemu graphic packs.

#include <string>
#include <utility>
#include <vector>
#include <iterator>

#include "pcedition/PCSettings.h"

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

	// The presets of the packs that are offered in the settings. The packs have many more of them (the weather has ten,
	// the time of day has the twenty four hours): only a few are shown, so that the menu stays simple.
	inline constexpr const char* const kDrawDistancePresets[] = {
		"Very Low (0.5x)", "Low (0.75x)", "Medium (1x)", "High (1.25x)", "Ultra (1.5x)",
		"Extreme (2x, requires Extended Memory pack!)",
	};
	inline constexpr const char* const kLevelOfDetailPresets[] = {
		"Low (0.75x)", "Medium (1x)", "High (1.25x)", "Ultra (1.5x, requires Extended Memory pack!)",
	};
	inline constexpr const char* const kWeatherPresets[] = {
		"None (Use Individual Settings)", "Blue Sky", "Cloudy", "Rain", "Heavy Rain", "Snow", "Heavy Snow", "Thunderstorm",
	};
	inline constexpr const char* const kDistantFogPresets[] = {
		"Default Fog Levels", "Fog 0%", "Fog 20%", "Fog 40%", "Fog 60%", "Fog 80%",
	};
	inline constexpr const char* const kDistantLightingPresets[] = {
		"Default Distant Lighting 1x", "Distant Lighting x2", "Distant Lighting x4", "Distant Lighting x6",
		"Distant Lighting x8", "Darker Distant Lighting 40%",
	};
	inline constexpr const char* const kCelShadingPresets[] = {
		"Cel-Shading Disabled", "Light Shading", "Painting Shading (World)", "Painting Shading (World & Link)",
	};
	inline constexpr const char* const kDayNightPresets[] = {"Enabled (Default)", "Disabled (Fixed Time Of Day)"};
	inline constexpr const char* const kTimeOfDayPresets[] = {
		"00:00", "04:00", "08:00", "12:00", "16:00", "20:00", "22:00",
	};
	inline constexpr const char* const kCloudSpeedPresets[] = {
		"Normal (No Speedup/Slowdown)", "Scaled with day length (Default)",
	};
	inline constexpr const char* const kHudElementPresets[] = {"Enabled (Default)", "Disabled"};
	inline constexpr const char* const kCameraHudPresets[] = {"Enabled (Default)", "Disabled (messes with text and hearts)"};
	inline constexpr const char* const kCameraSensitivityPresets[] = {
		"0.5x", "0.75x", "1x (Default)", "1.25x", "1.5x", "2x", "3x",
	};
	inline constexpr const char* const kMenuSpeedPresets[] = {"0.7x", "1.0x (Default)", "1.25x", "1.5x", "2.0x", "3x"};
	inline constexpr const char* const kLaserColourPresets[] = {
		"Red (Default)", "Rainbow", "Purple", "Blue", "Cyan", "Green", "Yellow",
	};
	inline constexpr const char* const kLaserPresets[] = {"Yes (Default)", "No"};
	inline constexpr const char* const kGuardianColourPresets[] = {
		"Malice Red (Default)", "Guardian Yellow", "Ancient Orange", "Energy Blue", "Glowing White",
	};
	inline constexpr const char* const kNoSpawnsPresets[] = {"Disabled (Default)", "Enabled"};

	// A setting of a community graphic pack: the pack, the category of its presets, the name in the settings, the
	// explanation and the presets that are offered (all of them when the list is empty).
	struct PackOption
	{
		const char* pack;                 // the folder of the pack
		const char* category;             // the category of its presets
		const char* label;                // the name in the settings
		const char* help;                 // the explanation
		const char* const* presets = nullptr; // the presets of the pack that are offered
	};
	inline constexpr PackOption kPackOptions[] = {
		{"BreathOfTheWild/Mods/DrawDistance", "NPC, Enemies And Other Entities", "Draw Distance",
		 "How far the characters, the enemies and the other things are drawn. A lower distance saves the graphics card, "
		 "the things pop in when they come close.", kDrawDistancePresets},
		{"BreathOfTheWild/Mods/DrawDistance", "Terrain, Buildings, Bushes And Other Objects", "Level of Detail",
		 "How far the terrain, the buildings and the plants are drawn.", kLevelOfDetailPresets},
		{"BreathOfTheWild/Mods/Weather", "Replace All Weather", "Weather",
		 "The weather everywhere at once. Off: the game keeps its own weather.", kWeatherPresets},
		{"BreathOfTheWild/Mods/RemoveFog", "Distant World Fog", "Distant Fog",
		 "The fog in the distance. Less fog: the horizon stays clearer.", kDistantFogPresets},
		{"BreathOfTheWild/Mods/RemoveFog", "Distant Lighting", "Distant Lighting",
		 "How bright the landscape in the distance is drawn.", kDistantLightingPresets},
		{"BreathOfTheWild/Mods/CelShading", "Preset", "Cel Shading",
		 "The picture drawn like a painting instead of like a photograph.", kCelShadingPresets},
		{"BreathOfTheWild/Mods/DayLength", "Day/Night Cycle", "Day/Night Cycle",
		 "Whether the day passes on its own. Off: the time of day stands still.", kDayNightPresets},
		{"BreathOfTheWild/Mods/DayLength", "Fixed Time Of Day", "Time Of Day",
		 "The time of day when the cycle is off.", kTimeOfDayPresets},
		{"BreathOfTheWild/Mods/DayLength", "Cloud Speed", "Cloud Speed",
		 "How fast the clouds move.", kCloudSpeedPresets},
		{"BreathOfTheWild/Mods/HUDElements", "Master Mode Icon", "Master Mode Icon",
		 "The icon of the master mode in the corner of the screen.", kHudElementPresets},
		{"BreathOfTheWild/Mods/HUDElements", "Hearts", "Hearts",
		 "The hearts of the life meter.", kHudElementPresets},
		{"BreathOfTheWild/Mods/HUDElements", "Stamina", "Stamina",
		 "The ring of the stamina.", kHudElementPresets},
		{"BreathOfTheWild/Mods/HUDElements", "Camera HUD (for screenshots)", "Camera HUD",
		 "The texts and the marks of the camera, for the screenshots.", kCameraHudPresets},
		{"BreathOfTheWild/Mods/CameraSensitivity", "Camera Sensitivity", "Game Camera Speed",
		 "How fast the camera of the game turns with the stick. It does not change the speed of the mouse.",
		 kCameraSensitivityPresets},
		{"BreathOfTheWild/Mods/MenuCursorSpeed", "Menu Navigation Speed", "Menu Speed",
		 "How fast the cursor of the game moves through the menus.", kMenuSpeedPresets},
		{"BreathOfTheWild/Mods/DivineLaserBeam", "Laser Color", "Laser Colour",
		 "The colour of the laser of the divine beams.", kLaserColourPresets},
		{"BreathOfTheWild/Mods/DivineLaserBeam", "Show Laser", "Divine Laser",
		 "The laser of the divine beams.", kLaserPresets},
		{"BreathOfTheWild/Mods/GlowingGuardianColor", "Decayed Guardian", "Guardian Colour",
		 "The colour of the guardians that have decayed.", kGuardianColourPresets},
		{"BreathOfTheWild/Mods/GlowingGuardianColor", "Ancient Armor", "Ancient Armour Colour",
		 "The colour of the ancient armour.", kGuardianColourPresets},
		{"BreathOfTheWild/Cheats/PreventRandomSpawns", "Prevent Yiga Clan Spawns", "No Yiga Clan",
		 "The Yiga clan does not appear any more.", kNoSpawnsPresets},
		{"BreathOfTheWild/Cheats/PreventRandomSpawns", "Prevent Skeleton-Type Enemy Spawns", "No Skeletons",
		 "The skeleton-like enemies do not appear any more.", kNoSpawnsPresets},
		{"BreathOfTheWild/Cheats/PreventRandomSpawns", "Prevent Keese Spawns", "No Keese",
		 "The Keese (the flying enemies) do not appear any more.", kNoSpawnsPresets},
		{"BreathOfTheWild/Cheats/PreventRandomSpawns", "Prevent Chuchu Jelly Spawns", "No Chuchu Jelly",
		 "The Chuchu jellies do not appear any more.", kNoSpawnsPresets},
		{"BreathOfTheWild/Cheats/PreventRandomSpawns", "Prevent Octarock Spawns", "No Octarocks",
		 "The Octarocks do not appear any more.", kNoSpawnsPresets},
		{"BreathOfTheWild/Cheats/PreventRandomSpawns", "Prevent Lizalfos Spawns", "No Lizalfos",
		 "The Lizalfos do not appear any more.", kNoSpawnsPresets},
		{"BreathOfTheWild/Cheats/PreventRandomSpawns", "Prevent All Animals, Insects and Fish Spawns", "No Animals",
		 "The animals, the insects and the fish do not appear any more.", kNoSpawnsPresets},
	};
	inline constexpr int kPackOptionCount = (int)std::size(kPackOptions);
	// the settings keep the preset of every option (the array of the settings must be big enough)
	static_assert(kPackOptionCount <= 32, "the settings have no room for the presets of the packs");

	// The names of the presets of the packs are not the names of the settings: the few that read badly are rewritten.
	struct PresetRename
	{
		const char* from;
		const char* to;
	};
	inline constexpr PresetRename kPresetRenames[] = {
		{"None (Use Individual Settings)", "Off"},
		{"Cel-Shading Disabled", "Off"},
		{"Enabled (Default)", "On"},
		{"Disabled (Default)", "Off"},
		{"Default Fog Levels", "Default"},
		{"Default Distant Lighting 1x", "Default"},
		{"Extreme (2x, requires Extended Memory pack!)", "Extreme (2x)"},
		{"Ultra (1.5x, requires Extended Memory pack!)", "Ultra (1.5x)"},
		{"Yes (Default)", "On"},
		{"No", "Off"},
		{"Malice Red (Default)", "Red"},
		{"Ancient Orange (Default)", "Orange"},
		{"1x (Default)", "1x"},
		{"Disabled (Fixed Time Of Day)", "Off"},
		{"Normal (No Speedup/Slowdown)", "Normal"},
		{"Scaled with day length (Default)", "With the day length"},
		{"Disabled (messes with text and hearts)", "Off"},
		{"Enabled (Stalkoblin, Stalmoblin & Stalizalfos)", "On"},
		{"Malice Red", "Red"},
		{"Guardian Yellow", "Yellow"},
		{"Ancient Orange", "Orange"},
		{"Energy Blue", "Blue"},
		{"Glowing White", "White"},
	};
	// the name a preset of a pack is shown with in the settings
	std::string PresetDisplayName(const std::string& preset);
	// the presets of a category of a pack: the name of the pack (the first) and the name shown (the second)
	const std::vector<std::pair<std::string, std::string>>& PackPresetChoices(const std::string& pack, const std::string& category, const char* const* presets = nullptr);
	// the preset of an option of a pack, as it is wanted by the settings (the old settings are used as a fallback)
	std::string PackOptionPreset(const Settings& s, int index);
	// whether the AMD FidelityFX runtime is there: the frame generation needs it
	bool FrameGenerationAvailable();

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
