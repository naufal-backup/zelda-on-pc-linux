#pragma once

// Zelda on PC - the FidelityFX API headers.
//
// The headers of the SDK version this project uses declare their entry points with __declspec(dllexport), which only
// the compilers of Windows know. The entry points are never called directly (they are loaded at runtime), so the
// attribute is not needed: on the other platforms it is removed before the headers are read.

#ifdef _WIN32
#include <ffx_api/ffx_api.h>
#include <ffx_api/ffx_framegeneration.h>
#include <ffx_api/vk/ffx_api_vk.h>
#else
#ifndef __declspec
#define __declspec(x)
#define CEMU_FFX_UNDEF_DECLSPEC
#endif
#include <ffx_api/ffx_api.h>
#include <ffx_api/ffx_framegeneration.h>
#include <ffx_api/vk/ffx_api_vk.h>
#ifdef CEMU_FFX_UNDEF_DECLSPEC
#undef __declspec
#undef CEMU_FFX_UNDEF_DECLSPEC
#endif
#endif
