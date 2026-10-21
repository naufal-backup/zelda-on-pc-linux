add_library(CemuPCEdition
	PCEdition.cpp
	PCEdition.h
	PCPlatform.cpp
	PCPlatform.h
	PCSettings.cpp
	PCSettings.h
	PCKeys.cpp
	PCKeys.h
	PCVK.h
	PCInput.cpp
	PCInput.h
	PCInputBackend.h
	PCGraphics.cpp
	PCGraphics.h
	PCCamera.cpp
	PCCamera.h
	PCOverlay.cpp
	PCOverlay.h
	PCDevBridge.cpp
	PCDevBridge.h
	PCUI.cpp
	PCUI.h
	PCMenus.cpp
	PCMenus.h
	PCPrompts.cpp
	PCPrompts.h
	PCProfiler.cpp
	PCProfiler.h
	PCIcon.cpp
	PCIcon.h
	PCMapIcons.cpp
	PCMapIcons.h
)

if (WIN32)
	target_sources(CemuPCEdition PRIVATE
		PCInputWindows.cpp
	)
else()
	target_sources(CemuPCEdition PRIVATE
		PCInputLinux.cpp
	)
endif()

set_property(TARGET CemuPCEdition PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

cemu_use_precompiled_header(CemuPCEdition)

target_link_libraries(CemuPCEdition PRIVATE
	CemuCafe
	CemuCommon
	CemuConfig
	CemuInput
	CemuUtil
	imguiImpl
	pugixml::pugixml
	ZArchive::zarchive
	Boost::headers
)

if (WIN32)
	target_link_libraries(CemuPCEdition PRIVATE ws2_32)
elseif (UNIX AND NOT APPLE)
	# the X11 input backend (raw keyboard and pointer events of the X server)
	if (TARGET PkgConfig::XI)
		target_link_libraries(CemuPCEdition PRIVATE PkgConfig::XI)
	endif()
	target_link_libraries(CemuPCEdition PRIVATE ${CMAKE_DL_LIBS})
endif()
